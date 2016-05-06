/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxCache.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ThreeMxInternal.h"
#include <DgnPlatform/HttpHandler.h>

#define ONE_GB (1024 * 1024 * 1024)

int s_debugCacheLevel = 0;

#define TABLE_NAME_ThreeMx "ThreeMx"

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct ThreeMxData
{
    //=======================================================================================
    // @bsiclass                                        Grigas.Petraitis            03/2015
    //=======================================================================================
    struct DatabasePrepareAndCleanupHandler : BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandler
        {
        mutable BeAtomic<bool> m_isPrepared;

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Grigas.Petraitis                04/2015
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual bool _IsPrepared() const {return m_isPrepared;}

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Grigas.Petraitis                03/2015
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual BentleyStatus _PrepareDatabase(BeSQLite::Db& db) const override
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
        virtual BentleyStatus _CleanupDatabase(BeSQLite::Db& db) const override
            {
            CachedStatementPtr sumStatement;
            if (BE_SQLITE_OK != db.GetCachedStatement(sumStatement, "SELECT SUM(DataSize) FROM " TABLE_NAME_ThreeMx))
                return ERROR;

            if (BE_SQLITE_ROW != sumStatement->Step())
                return ERROR;

            static uint64_t allowedSize = ONE_GB; // 1 GB

            CachedStatementPtr selectStatement;
            if (BE_SQLITE_OK != db.GetCachedStatement(selectStatement, "SELECT DataSize, Created FROM " TABLE_NAME_ThreeMx " ORDER BY Created ASC"))
                return ERROR;

            uint64_t runningSum = 0;
            while ((runningSum < allowedSize) && (BE_SQLITE_ROW == selectStatement->Step()))
                runningSum += selectStatement->GetValueInt64(0);

            uint64_t creationDate = selectStatement->GetValueInt64(1);
            CachedStatementPtr deleteStatement;
            if (BE_SQLITE_OK != db.GetCachedStatement(deleteStatement, "DELETE FROM " TABLE_NAME_ThreeMx " WHERE Created <= ?"))
                return ERROR;

            deleteStatement->BindInt64(1, creationDate);
            if (BE_SQLITE_DONE != deleteStatement->Step())
                return ERROR;

            return SUCCESS;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Grigas.Petraitis                03/2015
        +---------------+---------------+---------------+---------------+---------------+------*/
        static RefCountedPtr<DatabasePrepareAndCleanupHandler> Create() {return new DatabasePrepareAndCleanupHandler();}
        };

protected:
    Scene& m_scene;
    NodePtr m_node;
    Utf8String m_filename;
    mutable MxStreamBuffer m_nodeBytes;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                05/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    ThreeMxData(NodeP node, Scene& scene)
        : m_scene(scene), m_node(node)
        {
        /*BeAssert(m_node.IsValid() || nullptr!=m_output);*/
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus InitFromSource(Utf8CP filename, ByteStream const& data)
        {
        m_filename = filename;
        m_nodeBytes = data;

        /*if (m_output)
            {
            *m_output = m_nodeBytes; // copy, don't move - we need the data in this object to cache it.
            return SUCCESS;
            }*/

        printf ("initfromsource node=%p\n", m_node.get());
        BeAssert(m_node->IsQueued());
        if (SUCCESS != m_node->Read3MXB(m_nodeBytes, m_scene))
            {
            m_nodeBytes.Clear();
            m_nodeBytes.SetPos(0);    
            return ERROR;
            }

        /*if (!options.UseStorage())
            {
            m_nodeBytes.Clear();
            m_nodeBytes.SetPos(0);    
            }*/

        return SUCCESS;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus InitFromStorage(Db& db, BeMutex& cs, Utf8CP filename)
        {
        if (true)
            {
            BeMutexHolder lock(cs);

            CachedStatementPtr stmt;
            if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT Data,DataSize FROM " TABLE_NAME_ThreeMx " WHERE Filename=?"))
                return ERROR;

            stmt->ClearBindings();
            stmt->BindText(1, filename, Statement::MakeCopy::No);
            if (BE_SQLITE_ROW != stmt->Step())
                return ERROR;

            m_filename = filename;

            const void* data = stmt->GetValueBlob(0);
            int dataSize = stmt->GetValueInt(1);
            m_nodeBytes.SaveData((Byte*)data, dataSize);
            m_nodeBytes.SetPos(0);
            }

        /*if (options.m_output)
            {
            *options.m_output = std::move(m_nodeBytes);
            return SUCCESS;
            }*/

        BeAssert(m_node->IsQueued());
        BentleyStatus result = m_node->Read3MXB(m_nodeBytes, m_scene);
        if (SUCCESS != result)
            {
            BeAssert(false);
            return ERROR;
            }

        m_nodeBytes.Clear();    
        m_nodeBytes.SetPos(0);    
        return SUCCESS;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus PersistToStorage(Db& db, BeMutex& cs) const
        {
        BeMutexHolder lock(cs);
        BeAssert(m_nodeBytes.HasData());

        CachedStatementPtr selectStatement;
        if (BE_SQLITE_OK != db.GetCachedStatement(selectStatement, "SELECT Filename FROM " TABLE_NAME_ThreeMx " WHERE Filename=?"))
            return ERROR;

        selectStatement->ClearBindings();
        selectStatement->BindText(1, m_filename.c_str(), Statement::MakeCopy::No);
        if (BE_SQLITE_ROW == selectStatement->Step())
            {
            // update
            }
        else
            {

            // insert
            CachedStatementPtr stmt;
            if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "INSERT INTO " TABLE_NAME_ThreeMx " (Filename,Data,DataSize,Created) VALUES (?,?,?,?)"))
                return ERROR;

            stmt->ClearBindings();
            stmt->BindText(1, m_filename.c_str(), Statement::MakeCopy::No);
            stmt->BindBlob(2, m_nodeBytes.GetData(), (int)m_nodeBytes.GetSize(), Statement::MakeCopy::No);
            stmt->BindInt64(3, (int64_t)m_nodeBytes.GetSize());
            stmt->BindInt64(4, BeTimeUtilities::GetCurrentTimeAsUnixMillis());
            if (BE_SQLITE_DONE != stmt->Step())
                return ERROR;
            }

        m_nodeBytes.Clear();
        m_nodeBytes.SetPos(0);
        return SUCCESS;
        }
public:
    virtual ~ThreeMxData() {}
    Utf8StringCR GetFilename() const {return m_filename;}
    MxStreamBuffer& GetOutput() const {return m_nodeBytes;}
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct HttpData : ThreeMxData, IRealityData<HttpData, BeSQLiteRealityDataStorage, HttpRealityDataSource>
{
    //=======================================================================================
    // @bsiclass                                        Grigas.Petraitis            03/2015
    //=======================================================================================
    struct RequestOptions : IRealityData::RequestOptions
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS
    public:
        RequestOptions(bool synchronous)
            {
            BeSQLiteRealityDataStorage::SelectOptions::SetForceSynchronousRequest(synchronous);
            HttpRealityDataSource::RequestOptions::SetForceSynchronousRequest(synchronous);
            SetUseStorage(true);
            SetRequestFromSource(true);
            }
    };

private:
    HttpData(NodeP node, Scene& scene) : ThreeMxData(node, scene) {}

protected:
    virtual Utf8CP _GetId() const override {return GetFilename().c_str();}
    virtual bool _IsExpired() const override {return false;}
    virtual BentleyStatus _InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, ByteStream const& body) override {return InitFromSource(url, body);}
    virtual BentleyStatus _InitFrom(Db& db, BeMutex& cs, Utf8CP id) override {return InitFromStorage(db, cs, id);}
    virtual BentleyStatus _Persist(Db& db, BeMutex& cs) const override {return PersistToStorage(db, cs);}
    virtual BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandlerPtr _GetDatabasePrepareAndCleanupHandler() const override {return DatabasePrepareAndCleanupHandler::Create();}
public:
    static RefCountedPtr<HttpData> Create(NodeP node, Scene& scene) {return new HttpData(node, scene);}
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct FileData : ThreeMxData, IRealityData<FileData, BeSQLiteRealityDataStorage, FileRealityDataSource>
{
    //=======================================================================================
    // @bsiclass                                        Grigas.Petraitis            03/2015
    //=======================================================================================
    struct RequestOptions : IRealityData::RequestOptions
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS
    public:
        RequestOptions(bool synchronous)
            {
            FileRealityDataSource::RequestOptions::SetForceSynchronousRequest(synchronous);
            SetUseStorage(false);
            SetRequestFromSource(true);
            }
    };

private:
    FileData(NodeP node, Scene& scene) : ThreeMxData(node, scene) {}

protected:
    virtual Utf8CP _GetId() const override {return GetFilename().c_str();}
    virtual bool _IsExpired() const override {return false;}
    virtual BentleyStatus _InitFrom(Utf8CP filepath, ByteStream const& data) override {return InitFromSource(filepath, data);}
    virtual BentleyStatus _InitFrom(Db& db, BeMutex& cs, Utf8CP id) override {return InitFromStorage(db, cs, id);} 
    virtual BentleyStatus _Persist(Db& db, BeMutex& cs) const override {return PersistToStorage(db, cs);}
    virtual BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandlerPtr _GetDatabasePrepareAndCleanupHandler() const override {return DatabasePrepareAndCleanupHandler::Create();}
public:
    static RefCountedPtr<FileData> Create(NodeP node, Scene& scene) {return new FileData(node, scene);}    
};

DEFINE_REF_COUNTED_PTR(ThreeMxData)
DEFINE_REF_COUNTED_PTR(FileData)
DEFINE_REF_COUNTED_PTR(HttpData)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Scene::RemoveRequest(NodeR node)
    {
    auto found = m_requests.find(&node);

    if (found != m_requests.end())
        m_requests.erase(found);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataCacheResult Scene::RequestData(NodeP node, bool synchronous, MxStreamBuffer** output)
    {
    DgnDb::VerifyClientThread();
    BeAssert(output || node);

    Utf8String filePath ;
    if (nullptr != node)
        {
        if (node->IsQueued())
            return RealityDataCacheResult::RequestQueued;

        if (node->AreChildrenValid())
            return RealityDataCacheResult::Success;

        node->m_childLoad.store(Node::ChildLoad::Queued);
        filePath = ConstructNodeName(*node);
        }
    else
        {
        filePath = m_rootUrl;
        }

    ThreeMxData const* data = nullptr;
    RealityDataCacheResult result;
    if (IsUrl())
        {
        HttpDataPtr meshData = HttpData::Create(node, *this);
        data = meshData.get();
        result = m_cache->Get(*meshData, filePath.c_str(), HttpData::RequestOptions(synchronous));
        }
    else
        {
        FileDataPtr meshData = FileData::Create(node, *this);
        data = meshData.get();
        result = m_cache->Get(*meshData, filePath.c_str(), FileData::RequestOptions(synchronous));
        }

    if (nullptr != output)
        *output = &data->GetOutput();

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Scene::RequestStatus Scene::ProcessRequests()
    {
    if (m_requests.empty())
        return  RequestStatus::Finished;

    uint64_t startTime = BeTimeUtilities::QueryMillisecondsCounter(), endTime = startTime + 200;

    for (auto curr = m_requests.begin(); curr != m_requests.end(); )
        {
        if (BeTimeUtilities::QueryMillisecondsCounter() > endTime)
            break;

        BeAssert(curr->IsValid());
        RealityDataCacheResult cacheStatus = RequestData(curr->get(), false, nullptr);
        switch (cacheStatus)
            {
            case RealityDataCacheResult::Success:
                {
                curr = m_requests.erase(curr);
                break;
                }

            case RealityDataCacheResult::NotFound:
                {
                (*curr)->SetNotFound();
                curr = m_requests.erase(curr);
                break;
                }

            case RealityDataCacheResult::RequestQueued:
                curr++;
                break;

            default:
                BeAssert(false);
                break;
            }
        }

    return m_requests.empty() ? RequestStatus::Finished : RequestStatus::Pending;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Scene::QueueLoadChildren(Node& node)
    {
    BeAssert(node.NeedLoadChildren());
    RequestData(&node, false, nullptr);
    m_requests.insert(&node);
    }
