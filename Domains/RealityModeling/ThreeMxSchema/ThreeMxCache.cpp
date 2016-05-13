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
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct ThreeMxCache : BeSQLiteRealityDataStorage
{
    uint64_t m_allowedSize = (1024*1024*1024); // 1 Gb

    using BeSQLiteRealityDataStorage::BeSQLiteRealityDataStorage;

    mutable BeAtomic<bool> m_isPrepared;
    virtual bool _IsPrepared() const override {return m_isPrepared;}
    virtual BentleyStatus _PrepareDatabase(BeSQLite::Db& db) const override;
    virtual BentleyStatus _CleanupDatabase(BeSQLite::Db& db) const override;
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct ThreeMxData
{
protected:
    Scene& m_scene;
    NodePtr m_node;
    Utf8String m_filename;
    Utf8String m_shortName;
    mutable MxStreamBuffer m_nodeBytes;
    MxStreamBuffer* m_output;

    BentleyStatus InitFromSource(Utf8CP filename, ByteStream const& data);
    BentleyStatus InitFromStorage(Db& db, BeMutex& cs, Utf8CP filename);
    BentleyStatus PersistToStorage(Db& db, BeMutex& cs) const;
    void OnNotFound() {BeAssert(false); if (m_node.IsValid()) m_node->SetNotFound();}

public:
    ThreeMxData(NodeP node, Scene& scene, MxStreamBuffer* output) : m_scene(scene), m_node(node), m_output(output) 
        {
        if (node)
            m_shortName = node->GetChildFile();
        }

    Utf8StringCR GetFilename() const {return m_filename;}
    MxStreamBuffer& GetOutput() const {return m_nodeBytes;}
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct HttpData : ThreeMxData, IRealityData<HttpData, BeSQLiteRealityDataStorage, HttpRealityDataSource>
{
    using ThreeMxData::ThreeMxData;

    struct RequestOptions : RealityDataOptions
    {
        RequestOptions(bool synchronous)
            {
            m_forceSynchronous = synchronous;
            m_useStorage=true;
            m_requestFromSource=true;
            }
    };


protected:
    virtual Utf8CP _GetId() const override {return GetFilename().c_str();}
    virtual bool _IsExpired() const override {return false;}
    virtual void _OnError() override {OnNotFound();}
    virtual void _OnNotFound() override {OnNotFound();}
    virtual BentleyStatus _InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, ByteStream const& body) override {return InitFromSource(url, body);}
    virtual BentleyStatus _InitFrom(Db& db, BeMutex& cs, Utf8CP id) override {return InitFromStorage(db, cs, id);}
    virtual BentleyStatus _Persist(Db& db, BeMutex& cs) const override {return PersistToStorage(db, cs);}
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct FileData : ThreeMxData, IRealityData<FileData, BeSQLiteRealityDataStorage, FileRealityDataSource>
{
    struct RequestOptions : RealityDataOptions
    {
        RequestOptions(bool synchronous)
            {
            m_forceSynchronous = synchronous;
            m_useStorage=false;
            m_requestFromSource=true;
            }
    };

    using ThreeMxData::ThreeMxData;

protected:
    virtual Utf8CP _GetId() const override {return GetFilename().c_str();}
    virtual bool _IsExpired() const override {return false;}
    virtual void _OnError() override {OnNotFound();}
    virtual void _OnNotFound() override {OnNotFound();}
    virtual BentleyStatus _InitFrom(Utf8CP filepath, ByteStream const& data) override {return InitFromSource(filepath, data);}
    virtual BentleyStatus _InitFrom(Db& db, BeMutex& cs, Utf8CP id) override {return InitFromStorage(db, cs, id);}
    virtual BentleyStatus _Persist(Db& db, BeMutex& cs) const override {return PersistToStorage(db, cs);}
};

DEFINE_REF_COUNTED_PTR(FileData)
DEFINE_REF_COUNTED_PTR(HttpData)

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ThreeMxData::InitFromSource(Utf8CP filename, ByteStream const& data)
    {
    if (m_node.IsValid() && !m_node->IsQueued())
        return SUCCESS; // this node was abandoned.

    m_nodeBytes = data;
    m_filename = filename;

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
BentleyStatus ThreeMxData::InitFromStorage(Db& db, BeMutex& cs, Utf8CP filename)
    {
    if (m_node.IsValid() && !m_node->IsQueued())
        return SUCCESS; // this node was abandoned.

    if (true)
        {
        BeMutexHolder lock(cs);

        CachedStatementPtr stmt;
        if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT Data,DataSize FROM " TABLE_NAME_ThreeMx " WHERE Filename=?"))
            return ERROR;

        Utf8String name = m_shortName.empty() ? filename : m_shortName;
        stmt->ClearBindings();
        stmt->BindText(1, name, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW != stmt->Step())
            return ERROR;

        m_filename = filename;

        const void* data = stmt->GetValueBlob(0);
        int dataSize = stmt->GetValueInt(1);
        m_nodeBytes.SaveData((Byte*)data, dataSize);
        m_nodeBytes.SetPos(0);
        }

    m_filename = filename;
    if (m_output)
        {
        *m_output = m_nodeBytes;
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
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ThreeMxData::PersistToStorage(Db& db, BeMutex& cs) const
    {
    if (m_node.IsValid() && m_node->IsAbandoned())
        return SUCCESS;

    BeMutexHolder lock(cs);
    BeAssert(m_nodeBytes.HasData());

    Utf8String name = m_shortName.empty() ? m_filename : m_shortName;
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
BentleyStatus ThreeMxCache::_PrepareDatabase(BeSQLite::Db& db) const 
    {
    if (db.TableExists(TABLE_NAME_ThreeMx))
        {
        m_isPrepared.store(true);
        return SUCCESS;
        }

    Utf8CP ddl = "Filename CHAR PRIMARY KEY,Data BLOB,DataSize BIGINT,Created BIGINT";
    if (BE_SQLITE_OK == db.CreateTable(TABLE_NAME_ThreeMx, ddl))
        {
        m_isPrepared.store(true);
        return SUCCESS;
        }
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ThreeMxCache::_CleanupDatabase(BeSQLite::Db& db) const 
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
RealityDataCacheResult Scene::RequestData(NodeP node, bool synchronous, MxStreamBuffer* output)
    {
    DgnDb::VerifyClientThread();
    BeAssert(output || node);

    Utf8String filePath ;
    if (nullptr != node)
        {
        if (!node->AreChildrenNotLoaded())
            {
            BeAssert(false);
            return RealityDataCacheResult::Error;
            }

        node->m_childLoad.store(Node::ChildLoad::Queued);
        filePath = ConstructNodeName(*node);
        }
    else
        {
        filePath = m_rootUrl;
        }
    
    if (IsHttp())
        {
        HttpDataPtr httpdata = new HttpData(node, *this, output);
        return m_cache->Get(*httpdata, filePath.c_str(), HttpData::RequestOptions(synchronous));
        }

    FileDataPtr filedata = new FileData(node, *this, output);
    return m_cache->Get(*filedata, filePath.c_str(), FileData::RequestOptions(synchronous));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Scene::CreateCache()
    {
    m_cache = new RealityDataCache();
    m_cache->RegisterStorage(*new ThreeMxCache(m_localCacheName, SchedulingMethod::FIFO));

    uint32_t threadCount = std::max((uint32_t) 2,BeThreadUtilities::GetHardwareConcurrency() / 2);
    m_cache->RegisterSource(IsHttp() ? (IRealityDataSourceBase&) *HttpRealityDataSource::Create(threadCount, SchedulingMethod::FIFO) : *FileRealityDataSource::Create(threadCount, SchedulingMethod::FIFO));
    }
