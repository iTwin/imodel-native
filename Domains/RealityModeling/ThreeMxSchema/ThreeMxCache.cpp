/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxCache.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ThreeMxInternal.h"
#include <DgnPlatform/HttpHandler.h>

#define TABLE_NAME_ThreeMx "ThreeMx"

BEGIN_UNNAMED_NAMESPACE

//=======================================================================================
// Manage the creation and cleanup of the local ThreeMxTileCache used by ThreeMxFileData
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct ThreeMxTileCache : RealityData::Storage
{
    uint64_t m_allowedSize = (1024*1024*1024); // 1 Gb
    using Storage::Storage;
    virtual BentleyStatus _PrepareDatabase(Db& db) const override;
    virtual BentleyStatus _CleanupDatabase(Db& db) const override;
};

//=======================================================================================
// This object is created to load a single 3mx file asynchronously. Its virtual methods are called on other threads.
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct ThreeMxFileData : RealityData::Payload
{
protected:
    Scene& m_scene;
    NodePtr m_node;
    Utf8String m_shortName;
    mutable MxStreamBuffer m_nodeBytes;
    MxStreamBuffer* m_output;

public:
    struct RequestOptions : RealityData::Options
    {
        RequestOptions(bool synchronous) {m_forceSynchronous=synchronous;}
    };

    ThreeMxFileData(Utf8CP filename, NodeP node, Scene& scene, MxStreamBuffer* output) : Payload(filename), m_scene(scene), m_node(node), m_output(output) 
        {
        if (node)
            m_shortName = node->GetChildFile(); // Note: we must save this in the ctor, since it is not safe to call this on other threads.
        }

    MxStreamBuffer& GetOutput() const {return m_nodeBytes;}

    virtual bool _IsExpired() const override {return false;}
    virtual void _OnError() override {_OnNotFound();}
    virtual void _OnNotFound() override {BeAssert(false); if (m_node.IsValid()) m_node->SetNotFound();}
    virtual BentleyStatus _InitFrom(bmap<Utf8String, Utf8String> const& header, ByteStream const& body) override {return _InitFrom(body);}
    virtual BentleyStatus _InitFrom(Db& db) override;
    virtual BentleyStatus _InitFrom(ByteStream const& data) override;
    virtual BentleyStatus _Persist(Db& db) const override;
};

DEFINE_REF_COUNTED_PTR(ThreeMxFileData)
DEFINE_REF_COUNTED_PTR(ThreeMxTileCache)

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ThreeMxFileData::_InitFrom(ByteStream const& data)
    {
    if (m_node.IsValid() && !m_node->IsQueued())
        return SUCCESS; // this node was abandoned.

    m_nodeBytes = data;

    if (m_output)
        {
        *m_output = data;
        return SUCCESS;
        }

    BeAssert(m_node->IsQueued());
    if (SUCCESS != m_node->Read3MXB(m_nodeBytes, m_scene))
        {
        m_nodeBytes.Clear();
        m_nodeBytes.SetPos(0);
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ThreeMxFileData::_InitFrom(Db& db)
    {
    if (m_node.IsValid() && !m_node->IsQueued())
        return SUCCESS; // this node was abandoned.

    if (true)
        {
        CachedStatementPtr stmt;
        if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT Data,DataSize FROM " TABLE_NAME_ThreeMx " WHERE Filename=?"))
            return ERROR;

        Utf8StringCR name = m_shortName.empty() ? m_payloadId : m_shortName;
        stmt->ClearBindings();
        stmt->BindText(1, name, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW != stmt->Step())
            return ERROR;

        const void* data = stmt->GetValueBlob(0);
        int dataSize = stmt->GetValueInt(1);
        m_nodeBytes.SaveData((Byte*)data, dataSize);
        m_nodeBytes.SetPos(0);
        }

    if (nullptr != m_output) // is this is the scene file?
        {
        *m_output = m_nodeBytes;  // yes, just save its data. We're going to load it synchronously
        return SUCCESS;
        }

    BeAssert(m_node->IsQueued());
    BentleyStatus result = m_node->Read3MXB(m_nodeBytes, m_scene);
    if (SUCCESS != result)
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* save the data for a 3mx file into the tile cache. Note that this is also called for the scene file.
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ThreeMxFileData::_Persist(Db& db) const
    {
    if (m_node.IsValid() && m_node->IsAbandoned())
        return SUCCESS;

    BeAssert(m_nodeBytes.HasData());

    Utf8StringCR name = m_shortName.empty() ? m_payloadId : m_shortName;
    CachedStatementPtr stmt;
    db.GetCachedStatement(stmt, "INSERT INTO " TABLE_NAME_ThreeMx " (Filename,Data,DataSize,Created) VALUES (?,?,?,?)");

    stmt->ClearBindings();
    stmt->BindText(1, name, Statement::MakeCopy::No);
    stmt->BindBlob(2, m_nodeBytes.GetData(), (int)m_nodeBytes.GetSize(), Statement::MakeCopy::No);
    stmt->BindInt64(3, (int64_t)m_nodeBytes.GetSize());

    if (m_node.IsValid()) // for the root, store NULL for time. That way it will never get purged.
        stmt->BindInt64(4, BeTimeUtilities::GetCurrentTimeAsUnixMillis());

    return BE_SQLITE_DONE == stmt->Step() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ThreeMxTileCache::_PrepareDatabase(BeSQLite::Db& db) const 
    {
    if (db.TableExists(TABLE_NAME_ThreeMx))
        return SUCCESS;

    Utf8CP ddl = "Filename CHAR PRIMARY KEY,Data BLOB,DataSize BIGINT,Created BIGINT";
    if (BE_SQLITE_OK == db.CreateTable(TABLE_NAME_ThreeMx, ddl))
        return SUCCESS;

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ThreeMxTileCache::_CleanupDatabase(BeSQLite::Db& db) const 
    {
    CachedStatementPtr sumStatement;
    db.GetCachedStatement(sumStatement, "SELECT SUM(DataSize) FROM " TABLE_NAME_ThreeMx);

    if (BE_SQLITE_ROW != sumStatement->Step())
        return ERROR;

    uint64_t sum = sumStatement->GetValueInt64(0);
    if (sum <= m_allowedSize)
        return SUCCESS;

    uint64_t garbageSize = sum - m_allowedSize;

    CachedStatementPtr selectStatement;
    db.GetCachedStatement(selectStatement, "SELECT DataSize,Created FROM " TABLE_NAME_ThreeMx " ORDER BY Created ASC");

    uint64_t runningSum=0;
    while (runningSum < garbageSize)
        {
        if (BE_SQLITE_ROW != selectStatement->Step())
            {
            BeAssert(false);
            return ERROR;
            }

        runningSum += selectStatement->GetValueInt64(0);
        }

    BeAssert (runningSum >= garbageSize);
    uint64_t creationDate = selectStatement->GetValueInt64(1);
    BeAssert (creationDate > 0);

    CachedStatementPtr deleteStatement;
    db.GetCachedStatement(deleteStatement, "DELETE FROM " TABLE_NAME_ThreeMx " WHERE Created <= ?");
    deleteStatement->BindInt64(1, creationDate);

    return BE_SQLITE_DONE == deleteStatement->Step() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityData::CacheResult Scene::RequestData(NodeP node, bool synchronous, MxStreamBuffer* output)
    {
    DgnDb::VerifyClientThread();
    BeAssert(output || node);

    Utf8String filePath;
    if (nullptr != node)
        {
        if (!node->AreChildrenNotLoaded())
            {
            BeAssert(false);
            return RealityData::CacheResult::Error;
            }

        node->m_childLoad.store(Node::ChildLoad::Queued);
        filePath = ConstructNodeName(*node);
        }
    else
        {
        filePath = m_rootUrl;
        }

    return m_cache->RequestData(*new ThreeMxFileData(filePath.c_str(), node, *this, output), ThreeMxFileData::RequestOptions(synchronous));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Scene::CreateCache()
    {
    m_cache = new RealityData::Cache();

    uint32_t threadCount = std::max((uint32_t) 2,BeThreadUtilities::GetHardwareConcurrency() / 2);

    if (!IsHttp()) 
        { // Note: local files do not need a tile cache
        m_cache->SetSource(*new RealityData::FileSource(threadCount, RealityData::SchedulingMethod::FIFO));
        return;
        }

    ThreeMxTileCachePtr cache = new ThreeMxTileCache(threadCount);
    if (SUCCESS == cache->OpenAndPrepare(m_localCacheName))
        m_cache->SetStorage(*cache);

    m_cache->SetSource(*new RealityData::HttpSource(threadCount, RealityData::SchedulingMethod::FIFO));
    }
