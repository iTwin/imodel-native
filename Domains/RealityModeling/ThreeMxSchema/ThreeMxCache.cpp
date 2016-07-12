/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxCache.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ThreeMxInternal.h"
#include <BeHttp/HttpRequest.h>

#if defined(BENTLEYCONFIG_OS_WINDOWS) || defined(BENTLEYCONFIG_OS_APPLE_IOS) /* || defined(__clang__) WIP_ANDROID_CLANG */
#include <folly/futures/Future.h>
#endif

#define TABLE_NAME_ThreeMx "ThreeMx"

BEGIN_UNNAMED_NAMESPACE

//=======================================================================================
// Manage the creation and cleanup of the local ThreeMxTileCache used by ThreeMxFileData
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct ThreeMxTileCache : RealityData::Cache2
{
    uint64_t m_allowedSize = (1024*1024*1024); // 1 Gb
    virtual BentleyStatus _Prepare() const override;
    virtual BentleyStatus _Cleanup() const override;
};

//=======================================================================================
// This object is created to load a single 3mx file asynchronously. Its virtual methods are called on other threads.
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct ThreeMxFileData
{
protected:
    Scene& m_scene;
    NodePtr m_node;
    Utf8String m_fileName;
    Utf8String m_shortName;
    mutable MxStreamBuffer m_nodeBytes;
    MxStreamBuffer* m_output;

public:
    ThreeMxFileData(Utf8CP filename, NodeP node, Scene& scene, MxStreamBuffer* output) : m_fileName(filename), m_scene(scene), m_node(node), m_output(output) 
        {
        if (node)
            m_shortName = node->GetChildFile(); // Note: we must save this in the ctor, since it is not safe to call this on other threads.
        }

    StatusInt DoRead() const 
        {
        if (m_node.IsValid() && !m_node->IsQueued())
            return SUCCESS; // this node was abandoned.

        return m_scene.IsHttp() ? LoadFromHttp() : ReadFromFile();
        }

    StatusInt ReadFromFile() const;
    StatusInt LoadFromHttp() const;
    StatusInt LoadFromDb() const;
    StatusInt SaveToDb() const;
};

DEFINE_REF_COUNTED_PTR(ThreeMxFileData)
DEFINE_REF_COUNTED_PTR(ThreeMxTileCache)

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ThreeMxFileData::ReadFromFile() const
    {
    BeFile dataFile;
    if (BeFileStatus::Success != dataFile.Open(m_fileName.c_str(), BeFileAccess::Read))
        {
        m_node->SetNotFound();
        return ERROR;
        }

    if (BeFileStatus::Success != dataFile.ReadEntireFile(m_nodeBytes))
        {
        m_node->SetNotFound();
        return ERROR;
        }

    if (m_output)
        {
        *m_output = m_nodeBytes;
        return 0;
        }

    if (SUCCESS != m_node->Read3MXB(m_nodeBytes, m_scene))
        {
        m_node->SetNotFound();
        return ERROR;
        }

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* Attempt to load a node from the local cache.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ThreeMxFileData::LoadFromDb() const
    {
    auto cache = m_scene.GetCache();
    if (!cache.IsValid())
        return ERROR;

    if (true)
        {
        CachedStatementPtr stmt;
        if (BE_SQLITE_OK != cache->GetDb().GetCachedStatement(stmt, "SELECT Data,DataSize FROM " TABLE_NAME_ThreeMx " WHERE Filename=?"))
            return ERROR;

        Utf8StringCR name = m_shortName.empty() ? m_fileName : m_shortName;
        stmt->ClearBindings();
        stmt->BindText(1, name, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW != stmt->Step())
            return ERROR;

        m_nodeBytes.SaveData((Byte*)stmt->GetValueBlob(0), stmt->GetValueInt(1));
        m_nodeBytes.SetPos(0);
        }

    if (nullptr != m_output) // is this is the scene file?
        {
        *m_output = std::move(m_nodeBytes); // yes, just save its data. We're going to load it synchronously
        return SUCCESS;
        }

    BeAssert(m_node->IsQueued());
    return m_node->Read3MXB(m_nodeBytes, m_scene);
    }

/*---------------------------------------------------------------------------------**//**
* Load a node from an http source. This method runs on an IOThreadPool thread and waits for the http request to 
* complete or timeout.
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ThreeMxFileData::LoadFromHttp() const
    {
    if (SUCCESS == LoadFromDb())
        return SUCCESS; // node was available from local cache

    Http::HttpByteStreamBodyPtr responseBody = Http::HttpByteStreamBody::Create();
    Http::Request request(m_fileName);
    request.SetResponseBody(responseBody);
    Http::Response response = request.Perform();

    if (Http::ConnectionStatus::OK != response.GetConnectionStatus() || Http::HttpStatus::OK != response.GetHttpStatus())
        {
        m_node->SetNotFound();
        return ERROR;
        }

    if (nullptr != m_output) // is this is the scene file?
        {
        *m_output = std::move(responseBody->GetByteStream());
        return SUCCESS;
        }

    m_nodeBytes = std::move(responseBody->GetByteStream());

    BeAssert(m_node->IsQueued());
    if (SUCCESS != m_node->Read3MXB(m_nodeBytes, m_scene))
        return ERROR;

    SaveToDb();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Save the data for a 3mx file into the tile cache. Note that this is also called for the scene file.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ThreeMxFileData::SaveToDb() const
    {
    auto cache = m_scene.GetCache();
    if (!cache.IsValid())
        {
        BeAssert(false);
        return ERROR;
        }

    BeAssert(m_nodeBytes.HasData());

    Utf8StringCR name = m_shortName.empty() ? m_fileName : m_shortName;
    CachedStatementPtr stmt;
    cache->GetDb().GetCachedStatement(stmt, "INSERT INTO " TABLE_NAME_ThreeMx " (Filename,Data,DataSize,Created) VALUES (?,?,?,?)");

    stmt->ClearBindings();
    stmt->BindText(1, name, Statement::MakeCopy::No);
    stmt->BindBlob(2, m_nodeBytes.GetData(), (int)m_nodeBytes.GetSize(), Statement::MakeCopy::No);
    stmt->BindInt64(3, (int64_t)m_nodeBytes.GetSize());

    if (m_node.IsValid()) // for the root, store NULL for time. That way it will never get purged.
        stmt->BindInt64(4, BeTimeUtilities::GetCurrentTimeAsUnixMillis());

    if (BE_SQLITE_DONE != stmt->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    cache->ScheduleSave();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ThreeMxTileCache::_Prepare() const 
    {
    if (m_db.TableExists(TABLE_NAME_ThreeMx))
        return SUCCESS;
        
    Utf8CP ddl = "Filename CHAR PRIMARY KEY,Data BLOB,DataSize BIGINT,Created BIGINT";
    if (BE_SQLITE_OK == m_db.CreateTable(TABLE_NAME_ThreeMx, ddl))
        return SUCCESS;

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ThreeMxTileCache::_Cleanup() const 
    {
    CachedStatementPtr sumStatement;
    m_db.GetCachedStatement(sumStatement, "SELECT SUM(DataSize) FROM " TABLE_NAME_ThreeMx);

    if (BE_SQLITE_ROW != sumStatement->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    uint64_t sum = sumStatement->GetValueInt64(0);
    if (sum <= m_allowedSize)
        return SUCCESS;

    uint64_t garbageSize = sum - m_allowedSize;

    CachedStatementPtr selectStatement;
    m_db.GetCachedStatement(selectStatement, "SELECT DataSize,Created FROM " TABLE_NAME_ThreeMx " ORDER BY Created ASC");

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
    m_db.GetCachedStatement(deleteStatement, "DELETE FROM " TABLE_NAME_ThreeMx " WHERE Created <= ?");
    deleteStatement->BindInt64(1, creationDate);

    return BE_SQLITE_DONE == deleteStatement->Step() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityData::CacheResult Scene::RequestData(NodeP node, bool synchronous, MxStreamBuffer* output)
    {
#if defined(BENTLEYCONFIG_OS_WINDOWS) || defined(BENTLEYCONFIG_OS_APPLE_IOS)
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

    ThreeMxFileData data(filePath.c_str(), node, *this, output);
    auto result = folly::via(&BeFolly::IOThreadPool::GetPool(), [=](){return data.DoRead();});

    if (synchronous)
        {
        result.wait(std::chrono::seconds(2)); // only wait for 2 seconds
        if (!result.isReady())
            return RealityData::CacheResult::Error;
        }

    return RealityData::CacheResult::Success;
#else
    return RealityData::CacheResult::Error;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Scene::CreateCache()
    {
    if (!IsHttp()) 
        return;

    m_cache = new ThreeMxTileCache();
    if (SUCCESS != m_cache->OpenAndPrepare(m_localCacheName))
        m_cache = nullptr;
    }
