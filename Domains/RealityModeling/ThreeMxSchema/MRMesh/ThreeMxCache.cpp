/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/ThreeMxCache.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ThreeMxSchemaInternal.h"
#include    <DgnPlatform/HttpHandler.h>

#define ONE_GB (1024 * 1024 * 1024)

int s_debugCacheLevel = 0;

static bool isUrl(Utf8CP str){return (0 == strncmp("http:", str, 5) || 0 == strncmp("https:", str, 6));}

#define TABLE_NAME_ThreeMx "ThreeMx"

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct ThreeMxData
{
    //=======================================================================================
    // @bsiclass                                        Grigas.Petraitis            03/2015
    //=======================================================================================
    struct RequestOptions : RealityDataCacheOptions
        {
        Scene& m_scene;
        MxStreamBuffer* m_output;
        RequestOptions(MxStreamBuffer* output, Scene& scene) : RealityDataCacheOptions(true, false, true, true), m_scene(scene), m_output(output) {}
        };

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
    bool m_isRoot;
    Utf8String m_filename;
    NodePtr m_node;
    mutable MxStreamBuffer m_nodeBytes;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus InitFromSelf(IRealityDataBase const& self, RequestOptions const& options)
        {
        ThreeMxData const& other = dynamic_cast<ThreeMxData const&>(self);
        m_node = new Node(NodeInfo(), nullptr);
        m_node->Clone(*other.GetNode());
        m_filename = other.GetFilename();
        return SUCCESS;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus InitFromSource(Utf8CP filename, ByteStream const& data, RequestOptions const& options)
        {
        m_filename = filename;
        m_nodeBytes = data;

        if (options.m_output)
            {
            *options.m_output = m_nodeBytes; // copy, don't move - we need the data in this object to cache it.
            return SUCCESS;
            }

        m_node = new Node(NodeInfo(), nullptr);
    
        BeAssert(m_node->m_children.empty());
        if (SUCCESS != m_node->Read3MXB(m_nodeBytes, options.m_scene))
            {
            m_nodeBytes.Clear();
            m_nodeBytes.SetPos(0);    
            m_node = nullptr;
            return ERROR;
            }

        if (!options.UseStorage())
            {
            m_nodeBytes.Clear();
            m_nodeBytes.SetPos(0);    
            }

        return SUCCESS;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus InitFromStorage(Db& db, BeMutex& cs, Utf8CP id, RequestOptions const& options)
        {
        if (true)
            {
            BeMutexHolder lock(cs);

            CachedStatementPtr stmt;
            if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT Data, DataSize FROM " TABLE_NAME_ThreeMx " WHERE Filename=?"))
                return ERROR;

            stmt->ClearBindings();
            stmt->BindText(1, id, Statement::MakeCopy::No);
            if (BE_SQLITE_ROW != stmt->Step())
                return ERROR;

            m_filename = id;

            const void* data = stmt->GetValueBlob(0);
            int dataSize = stmt->GetValueInt(1);
            m_nodeBytes.SaveData((Byte*)data, dataSize);
            m_nodeBytes.SetPos(0);
            }

        if (options.m_output)
            {
            *options.m_output = std::move(m_nodeBytes);
            return SUCCESS;
            }

        m_node = new Node(NodeInfo(), nullptr);

        BeAssert(m_node->GetChildren().empty());
        BentleyStatus result = m_node->Read3MXB(m_nodeBytes, options.m_scene);
        if (SUCCESS != result)
            {
            BeAssert(false);
            m_node = nullptr;
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
            if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "INSERT INTO " TABLE_NAME_ThreeMx " (Filename, Data, DataSize, Created) VALUES (?,?,?,?)"))
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
    NodePtr GetNode() const {return m_node;}
    Utf8StringCR GetFilename() const {return m_filename;}
    void CloneNode(Node& node);
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct HttpData : ThreeMxData, IRealityData<HttpData, BeSQLiteRealityDataStorage, HttpRealityDataSource>
{
    //=======================================================================================
    // @bsiclass                                        Grigas.Petraitis            03/2015
    //=======================================================================================
    struct RequestOptions : ThreeMxData::RequestOptions, IRealityData::RequestOptions
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS
    public:
        RequestOptions(MxStreamBuffer* output, Scene& scene, bool synchronous) : ThreeMxData::RequestOptions(output, scene)
            {
            BeSQLiteRealityDataStorage::SelectOptions::SetForceSynchronousRequest(synchronous);
            HttpRealityDataSource::RequestOptions::SetForceSynchronousRequest(synchronous);
            SetUseStorage(true);
            }
    };

private:

protected:
    virtual Utf8CP _GetId() const override {return GetFilename().c_str();}
    virtual bool _IsExpired() const override {return false;}
    virtual BentleyStatus _InitFrom(IRealityDataBase const& self, RealityDataCacheOptions const& options) override {return InitFromSelf(self, (RequestOptions const&) options);}
    virtual BentleyStatus _InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, ByteStream const& body, HttpRealityDataSource::RequestOptions const& options) override {return InitFromSource(url, body, (RequestOptions const&)options);}
    virtual BentleyStatus _InitFrom(Db& db, BeMutex& cs, Utf8CP id, BeSQLiteRealityDataStorage::SelectOptions const& options) override {return InitFromStorage(db, cs, id, (RequestOptions const&)options);}
    virtual BentleyStatus _Persist(Db& db, BeMutex& cs) const override {return PersistToStorage(db, cs);}
    virtual BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandlerPtr _GetDatabasePrepareAndCleanupHandler() const override {return DatabasePrepareAndCleanupHandler::Create();}
public:
    static RefCountedPtr<HttpData> Create() {return new HttpData();}
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct FileData : ThreeMxData, IRealityData<FileData, BeSQLiteRealityDataStorage, FileRealityDataSource>
{
    //=======================================================================================
    // @bsiclass                                        Grigas.Petraitis            03/2015
    //=======================================================================================
    struct RequestOptions : ThreeMxData::RequestOptions, IRealityData::RequestOptions
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS
    public:
        RequestOptions(MxStreamBuffer* output, Scene& scene, bool synchronous) : ThreeMxData::RequestOptions(output, scene)
            {
            BeSQLiteRealityDataStorage::SelectOptions::SetForceSynchronousRequest(synchronous);
            FileRealityDataSource::RequestOptions::SetForceSynchronousRequest(synchronous);
            SetUseStorage(false);
            }
    };

private:
    FileData() : ThreeMxData() {}

protected:
    virtual Utf8CP _GetId() const override {return GetFilename().c_str();}
    virtual bool _IsExpired() const override {return false;}
    virtual BentleyStatus _InitFrom(IRealityDataBase const& self, RealityDataCacheOptions const& options) override {return InitFromSelf(self, (RequestOptions const&)options);}
    virtual BentleyStatus _InitFrom(Utf8CP filepath, ByteStream const& data, FileRealityDataSource::RequestOptions const& options) override {return InitFromSource(filepath, data, (RequestOptions const&)options);}
    virtual BentleyStatus _InitFrom(Db& db, BeMutex& cs, Utf8CP id, BeSQLiteRealityDataStorage::SelectOptions const& options) override {return InitFromStorage(db, cs, id, (RequestOptions const&)options);} 
    virtual BentleyStatus _Persist(Db& db, BeMutex& cs) const override {return PersistToStorage(db, cs);}
    virtual BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandlerPtr _GetDatabasePrepareAndCleanupHandler() const override {return DatabasePrepareAndCleanupHandler::Create();}
public:
    static RefCountedPtr<FileData> Create() {return new FileData();}    
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

    for (auto const& child : node.GetChildren())
        RemoveRequest(*child);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxData::CloneNode(Node& node)
    {
    BeAssert(GetNode().IsValid());

    // clone the node but keep the parent and the m_info structure the same
    NodeP parent = node.m_parent;
    NodeInfo  info = node.m_info;
    node.Clone(*GetNode());
    node.m_parent = parent;
    node.m_info = info;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataCacheResult Scene::RequestData(Node* node, Utf8StringCR path, bool synchronous, MxStreamBuffer* output)
    {
    RealityDataCacheResult result;

    if (isUrl(path.c_str()))
        {
        HttpDataPtr meshData;
        result = m_cache->Get(meshData, path.c_str(), *new HttpData::RequestOptions(output, *this, synchronous));
        if (RealityDataCacheResult::Success == result && nullptr != node)
            meshData->CloneNode(*node);
        }
    else
        {
        FileDataPtr meshData;
        result = m_cache->Get(meshData, path.c_str(), *new FileData::RequestOptions(output, *this, synchronous));
        if (RealityDataCacheResult::Success == result && nullptr != node)
            meshData->CloneNode(*node);
        }

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
    size_t requestsProcessed = 0;

    for (auto curr = m_requests.begin(); curr != m_requests.end(); )
        {
        if (BeTimeUtilities::QueryMillisecondsCounter() > endTime)
            break;

        Utf8StringCR filePath = curr->first->GetFilePath(*this);
        RealityDataCacheResult cacheStatus = RequestData(curr->first.get(), filePath.c_str(), false, nullptr);
        switch (cacheStatus)
            {
            case RealityDataCacheResult::Success:
                {
                curr->first->SetNodePath(filePath);

                requestsProcessed++;
                curr = m_requests.erase(curr);
                break;
                }

            case RealityDataCacheResult::NotFound:
                {
                DisplayNodeFailureWarning(filePath);
                curr->first->m_parent->RemoveChild(curr->first.get());
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

    return (0 == requestsProcessed) ? RequestStatus::None : RequestStatus::Processed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::SynchronousRead(NodeR node, Utf8StringCR filePath)
    {
    RealityDataCacheResult status = RequestData(&node, filePath.c_str(), true, nullptr);
    if (RealityDataCacheResult::Success != status)
        return ERROR;

    node.SetNodePath(filePath);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Scene::QueueLoadChildren(Node& node, DgnViewportP viewport)
    {
    int requestsMade = 0;

    node.m_childrenRequested = true;

    auto found = m_requests.find(&node);
    if (found == m_requests.end())
        {
        Utf8StringCR fileName = node.GetFilePath(*this);
        BeAssert(node.m_children.empty());
        RequestData(nullptr, fileName.c_str(), false, nullptr);
        m_requests[&node] = NodeRequest(viewport);
        ++requestsMade;
        }
    else
        {
        found->second.m_viewports.insert(viewport);
        }

    if (s_debugCacheLevel > 3 && requestsMade)
        printf("%d Cache Requests Initiated, %d Exist\n", requestsMade, (int) m_requests.size());

#ifdef WIP
    if (0 != requestsMade && !m_progressiveStarted)
        {
        m_progressiveStarted = true;
        ProgressiveDisplayManager::GetManager().BeginProgressive(*this);
        }
#endif
    }

