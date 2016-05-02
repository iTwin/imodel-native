/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMeshCache.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ThreeMxSchemaInternal.h"
#include    <DgnPlatform/HttpHandler.h>

#define ONE_GB (1024 * 1024 * 1024)

int s_debugCacheLevel = 0;

#define TABLE_NAME_Acute3dMesh "Acute3dMesh"

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct MRMeshData
{
    //=======================================================================================
    // @bsiclass                                        Grigas.Petraitis            03/2015
    //=======================================================================================
    struct RequestOptions : RealityDataCacheOptions
        {
        Scene& m_scene;
        RequestOptions(Scene& scene) : RealityDataCacheOptions(true, false, true, true), m_scene(scene) {}
        };

    //=======================================================================================
    // @bsiclass                                        Grigas.Petraitis            03/2015
    //=======================================================================================
    struct DatabasePrepareAndCleanupHandler : BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandler
        {
        static BeAtomic<bool> s_isPrepared;

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Grigas.Petraitis                04/2015
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual bool _IsPrepared() const {return s_isPrepared;}

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Grigas.Petraitis                03/2015
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual BentleyStatus _PrepareDatabase(BeSQLite::Db& db) const override
            {
            if (db.TableExists(TABLE_NAME_Acute3dMesh))
                {
                s_isPrepared.store(true);
                return SUCCESS;
                }

            Utf8CP ddl = "Filename CHAR PRIMARY KEY,Data BLOB,DataSize BIGINT,Created BIGINT";
            if (BE_SQLITE_OK == db.CreateTable(TABLE_NAME_Acute3dMesh, ddl))
                {
                s_isPrepared.store(true);
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
            if (BE_SQLITE_OK != db.GetCachedStatement(sumStatement, "SELECT SUM(DataSize) FROM " TABLE_NAME_Acute3dMesh))
                return ERROR;

            if (BE_SQLITE_ROW != sumStatement->Step())
                return ERROR;

            static uint64_t allowedSize = ONE_GB; // 1 GB

            CachedStatementPtr selectStatement;
            if (BE_SQLITE_OK != db.GetCachedStatement(selectStatement, "SELECT DataSize, Created FROM " TABLE_NAME_Acute3dMesh " ORDER BY Created ASC"))
                return ERROR;

            uint64_t runningSum = 0;
            while ((runningSum < allowedSize) && (BE_SQLITE_ROW == selectStatement->Step()))
                runningSum += selectStatement->GetValueInt64(0);

            uint64_t creationDate = selectStatement->GetValueInt64(1);
            CachedStatementPtr deleteStatement;
            if (BE_SQLITE_OK != db.GetCachedStatement(deleteStatement, "DELETE FROM " TABLE_NAME_Acute3dMesh " WHERE Created <= ?"))
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
    Utf8String m_filename;
    NodePtr m_node;
    mutable MxStreamBuffer m_nodeBytes;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus InitFromSelf(IRealityDataBase const& self, RequestOptions const& options)
        {
        MRMeshData const& other = dynamic_cast<MRMeshData const&>(self);
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
        m_node = new Node(NodeInfo(), nullptr);
        
        m_nodeBytes = data;
    
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
            if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT Data, DataSize FROM " TABLE_NAME_Acute3dMesh " WHERE Filename=?"))
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

        CachedStatementPtr selectStatement;
        if (BE_SQLITE_OK != db.GetCachedStatement(selectStatement, "SELECT Filename FROM " TABLE_NAME_Acute3dMesh " WHERE Filename=?"))
            return ERROR;

        selectStatement->ClearBindings();
        selectStatement->BindText(1, m_filename.c_str(), Statement::MakeCopy::No);
        if (BE_SQLITE_ROW == selectStatement->Step())
            {
            // update
            }
        else
            {
            BeAssert(m_nodeBytes.HasData());

            // insert
            CachedStatementPtr stmt;
            if (BE_SQLITE_OK != db.GetCachedStatement(stmt, "INSERT INTO " TABLE_NAME_Acute3dMesh " (Filename, Data, DataSize, Created) VALUES (?,?,?,?)"))
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
    virtual ~MRMeshData() {}
    NodePtr GetNode() const {return m_node;}
    Utf8StringCR GetFilename() const {return m_filename;}
    void CloneNode(Node& node);
};

BeAtomic<bool> MRMeshData::DatabasePrepareAndCleanupHandler::s_isPrepared;

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct HttpData : MRMeshData, IRealityData<HttpData, BeSQLiteRealityDataStorage, HttpRealityDataSource>
{
    //=======================================================================================
    // @bsiclass                                        Grigas.Petraitis            03/2015
    //=======================================================================================
    struct RequestOptions : MRMeshData::RequestOptions, IRealityData::RequestOptions
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS
    public:
        RequestOptions(Scene& scene, bool synchronous) : MRMeshData::RequestOptions(scene)
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
typedef RefCountedPtr<HttpData> HttpDataPtr;

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct FileData : MRMeshData, IRealityData<FileData, BeSQLiteRealityDataStorage, FileRealityDataSource>
{
    //=======================================================================================
    // @bsiclass                                        Grigas.Petraitis            03/2015
    //=======================================================================================
    struct RequestOptions : MRMeshData::RequestOptions, IRealityData::RequestOptions
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS
    public:
        RequestOptions(Scene& scene, bool synchronous) : MRMeshData::RequestOptions(scene)
            {
            BeSQLiteRealityDataStorage::SelectOptions::SetForceSynchronousRequest(synchronous);
            FileRealityDataSource::RequestOptions::SetForceSynchronousRequest(synchronous);
            SetUseStorage(false);
            }
//        static RefCountedPtr<RequestOptions> Create( bool synchronous) {return new RequestOptions(synchronous);}
    };

private:
    FileData() : MRMeshData() {}

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
typedef RefCountedPtr<FileData> FileDataPtr;

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
BEGIN_BENTLEY_THREEMX_NAMESPACE
/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct   NodeRequest
{
    bset<DgnViewportP> m_viewports;
    NodeRequest() {}
    NodeRequest(DgnViewportP viewport) {m_viewports.insert(viewport);}
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct Cache
{
    typedef bmap<NodeP, NodeRequest> RequestMap;
    RealityDataCache* m_cache=nullptr;
    RequestMap m_requests;
    ScenePtr m_scene;
    SystemP m_target;

    Cache(SystemP target) : m_progressiveStarted(false), m_target(target) {}
    void SetRoot(SceneR scene) {m_scene = &scene; Initialize();}
    void Initialize();
    void RemoveCacheRequests(NodeR node);
    void CloneNodeFromData(Node& node, MRMeshData const& meshData);
    RealityDataCacheResult RequestData(SystemP target, Node* node, BeFileNameCR path, bool synchronous = false);
    CacheManager::RequestStatus ProcessRequests();
    void Debug();
    size_t GetMemoryUsage() {return m_scene->GetMemorySize();}
    size_t GetTextureMemoryUsage() {return m_scene->GetTextureMemorySize();}
    size_t GetNodeCount() {return m_scene->GetNodeCount();}
    size_t GetMeshCount() {return m_scene->GetMeshCount();}
    size_t GetMaxDepth() {return m_scene->GetMaxDepth();}
    BentleyStatus FlushStale(uint64_t staleTime);
    BentleyStatus SynchronousRead(NodeR node, BeFileNameCR fileName);
    void QueueChildLoad(Node::MeshNodes const& children, DgnViewportP viewport);
};
END_BENTLEY_THREEMX_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Cache::Initialize()
    {
    RealityDataCachePtr m_cache = RealityDataCache::Create(100);

    BeFileName storageFileName = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName();
    storageFileName.AppendToPath(BeFileName(m_scene->GetSceneName()));

    m_cache->RegisterStorage(*BeSQLiteRealityDataStorage::Create(storageFileName));
    m_cache->RegisterSource(*FileRealityDataSource::Create(4));
    }
#endif

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
void MRMeshData::CloneNode(Node& node)
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
RealityDataCacheResult Scene::RequestData(Node* node, BeFileNameCR path, bool synchronous)
    {
    RealityDataCacheResult result;

    if (path.IsUrl())
        {
        HttpDataPtr meshData;
        result = m_cache->Get(meshData, path.GetNameUtf8().c_str(), *new HttpData::RequestOptions(*this, synchronous));
        if (RealityDataCacheResult::Success == result && nullptr != node)
            meshData->CloneNode(*node);
        }
    else
        {
        FileDataPtr meshData;
        result = m_cache->Get(meshData, path.GetNameUtf8().c_str(), *new FileData::RequestOptions(*this, synchronous));
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

        if (!curr->first->m_parent->IsLoaded())
            continue;

        BeFileName             fileName = curr->first->GetFileName();
        RealityDataCacheResult cacheStatus = RequestData(curr->first, fileName, false);
        switch (cacheStatus)
            {
            case RealityDataCacheResult::Success:
                {
                curr->first->SetDirectory(BeFileName(BeFileName::DevAndDir, fileName));

                requestsProcessed++;
                curr = m_requests.erase(curr);
                break;
                }

            case RealityDataCacheResult::NotFound:
                {
                Util::DisplayNodeFailureWarning(fileName.c_str());
                curr->first->m_parent->RemoveChild(curr->first);
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

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Cache::Debug()
    {
    size_t memoryUsage = GetMemoryUsage(), textureMemoryUsage = GetTextureMemoryUsage(), nodeCount = GetNodeCount();
    printf("Node Count: %zu, Memory Usage: %lf, Texture Memory: %lf \n", nodeCount, (double)memoryUsage / (double)ONE_GB, (double)textureMemoryUsage / (double)ONE_GB);
    printf("Memory/Node: %lf Texture/Node: %lf, Mesh Count: %zu, Max Depth: %zu\n", (double)memoryUsage / (double)(nodeCount * 1024), (double)textureMemoryUsage / (double)(nodeCount * 1024), GetMeshCount(), GetMaxDepth());
#if defined (BENTLEYCONFIG_OS_WINDOWS) && !defined (BENTLEY_WINRT)
    printf("Resolution Ratio: %lf\n", Util::CalculateResolutionRatio());
#endif

#if defined (BENTLEYCONFIG_OS_WINDOWS) && !defined (BENTLEY_WINRT)
    size_t  load, total, available;

    Util::GetMemoryStatistics(load, total, available);
    printf("Memory Load: %zu, Total Memory: %lf, Available Memory: %lf\n", load, (double)total / (double)ONE_GB, (double)available / (double)ONE_GB);
#endif
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Cache::FlushStale(uint64_t staleTime)
    {
    if (!m_requests.empty())
        return ERROR;

    m_scene->FlushStale(staleTime);
    return SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::SynchronousRead(NodeR node, BeFileNameCR fileName)
    {
    RealityDataCacheResult status = RequestData(&node, fileName, true);
    if (RealityDataCacheResult::Success != status)
        return ERROR;

    node.SetDirectory(BeFileName(BeFileName::DevAndDir, fileName));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Scene::QueueChildLoad(Node::MeshNodes const& children, DgnViewportP viewport)
    {
    int requestsMade = 0;

    for (auto const& child : children)
        {
        BeFileName fileName = child->GetFileName();
        auto found = m_requests.find(child.get());
        
        if (found == m_requests.end())
            {
            BeAssert(child->m_children.empty());
            RequestData(nullptr, fileName, false);
            m_requests[child.get()] = NodeRequest(viewport);
            requestsMade++;
            }
        else
            {
            found->second.m_viewports.insert(viewport);
            }
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

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
CacheManagerR CacheManager::GetManager()
    {
    static CacheManager s_manager;
    return s_manager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CacheManager::QueueChildLoad(Node::MeshNodes const& children, DgnViewportP viewport, LoadContextCR loadContext)
    {
    if (nullptr == m_cache)
        m_cache = new Cache(loadContext.m_system);

    m_cache->QueueChildLoad(children, viewport);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CacheManager::SetRoot(SceneR scene, SystemP system)
    {
    if (nullptr == m_cache)
        m_cache = new Cache(system);

    m_cache->SetRoot(scene);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CacheManager::SynchronousRead(NodeR node, BeFileNameCR fileName, LoadContextCR loadContext)
    {
    if (nullptr == m_cache)
        m_cache = new Cache(loadContext.m_system);

    return m_cache->SynchronousRead(node, fileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Grigas.Petraitis    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CacheManager::CacheManager()
    {
    }

CacheManager::RequestStatus CacheManager::ProcessRequests() { return (nullptr == m_cache) ? CacheManager::RequestStatus::Finished :  m_cache->ProcessRequests(); }
void CacheManager::Debug() { if (nullptr != m_cache) m_cache->Debug(); }
void CacheManager::Flush(uint64_t staleTime) { if (nullptr != m_cache) m_cache->FlushStale(staleTime); }
void CacheManager::RemoveRequest(NodeR node) { if (nullptr != m_cache) m_cache->RemoveCacheRequests(node); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Grigas.Petraitis    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CacheManager::~CacheManager()
    {
    if (nullptr != m_cache)
        {
        delete m_cache;
        m_cache = nullptr;
        }
    }
#endif

