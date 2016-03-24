/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMeshCache.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "..\ThreeMxSchemaInternal.h"
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
        Render::SystemP m_renderSys;
        RequestOptions(Render::SystemP system) : RealityDataCacheOptions(true, false, true, true), m_renderSys(system) {}
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
    MRMeshNodePtr m_node;
    mutable MxStreamBuffer m_nodeBytes;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus InitFromSelf(IRealityDataBase const& self, RequestOptions const& options)
        {
        MRMeshData const& other = dynamic_cast<MRMeshData const&>(self);
        m_node = new MRMeshNode(S3NodeInfo(), nullptr, options.m_renderSys);
        m_node->Clone(*other.GetNode());
        m_filename = other.GetFilename();
        return SUCCESS;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus InitFromSource(Utf8CP filename, ByteStream const& data, RequestOptions const& options)
        {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
        m_filename = filename;
        m_node = MRMeshNode::Create();
        
        m_nodeBytes = data;
        BVectorStreamBuffer buff(m_nodeBytes);
        std::istream stream(&buff);
    
        BeAssert(m_node->m_children.empty());
        if (SUCCESS != m_node->Read3MXB(stream))
            {
            // Needs work... .Error message.
            BeAssert(false && error.c_str());
            m_nodeBytes.clear();
            m_node = nullptr;
            return ERROR;
            }

        if (!options.UseStorage())
            m_nodeBytes.Clear();

        return SUCCESS;
#else
        return ERROR;
#endif
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

        m_node = new MRMeshNode(S3NodeInfo(), nullptr, options.m_renderSys);

        BeAssert(m_node->GetChildren().empty());
        BentleyStatus result = m_node->Read3MXB(m_nodeBytes);
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
    MRMeshNodePtr GetNode() const {return m_node;}
    Utf8StringCR GetFilename() const {return m_filename;}
};

BeAtomic<bool> MRMeshData::DatabasePrepareAndCleanupHandler::s_isPrepared;

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct MRMeshHttpData : MRMeshData, IRealityData<MRMeshHttpData, BeSQLiteRealityDataStorage, HttpRealityDataSource>
{
    //=======================================================================================
    // @bsiclass                                        Grigas.Petraitis            03/2015
    //=======================================================================================
    struct RequestOptions : MRMeshData::RequestOptions, IRealityData::RequestOptions
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS
    public:
        RequestOptions(SystemP target, bool synchronous) : MRMeshData::RequestOptions(target)
            {
            BeSQLiteRealityDataStorage::SelectOptions::SetForceSynchronousRequest(synchronous);
            HttpRealityDataSource::RequestOptions::SetForceSynchronousRequest(synchronous);
            SetUseStorage(true);
            }
//        static RefCountedPtr<RequestOptions> Create(bool synchronous) {return new RequestOptions(synchronous);}
    };

private:
protected:
    virtual Utf8CP _GetId() const override {return GetFilename().c_str();}
    virtual bool _IsExpired() const override {return false;}
    virtual BentleyStatus _InitFrom(IRealityDataBase const& self, RealityDataCacheOptions const& options) override {return InitFromSelf(self, (RequestOptions const&)options);}
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    virtual BentleyStatus _InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, bvector<Byte> const& body, HttpRealityDataSource::RequestOptions const& options) override {return InitFromSource(url, body, (RequestOptions const&)options);}
#else
    virtual BentleyStatus _InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, bvector<Byte> const& body, HttpRealityDataSource::RequestOptions const& options) override {return ERROR;}
#endif
    virtual BentleyStatus _InitFrom(Db& db, BeMutex& cs, Utf8CP id, BeSQLiteRealityDataStorage::SelectOptions const& options) override {return InitFromStorage(db, cs, id, (RequestOptions const&)options);}
    virtual BentleyStatus _Persist(Db& db, BeMutex& cs) const override {return PersistToStorage(db, cs);}
    virtual BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandlerPtr _GetDatabasePrepareAndCleanupHandler() const override {return DatabasePrepareAndCleanupHandler::Create();}
public:
    static RefCountedPtr<MRMeshHttpData> Create() {return new MRMeshHttpData();}
};
typedef RefCountedPtr<MRMeshHttpData> MRMeshHttpDataPtr;

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct MRMeshFileData : MRMeshData, IRealityData<MRMeshFileData, BeSQLiteRealityDataStorage, FileRealityDataSource>
{
    //=======================================================================================
    // @bsiclass                                        Grigas.Petraitis            03/2015
    //=======================================================================================
    struct RequestOptions : MRMeshData::RequestOptions, IRealityData::RequestOptions
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS
    public:
        RequestOptions(SystemP target, bool synchronous) : MRMeshData::RequestOptions(target)
            {
            BeSQLiteRealityDataStorage::SelectOptions::SetForceSynchronousRequest(synchronous);
            FileRealityDataSource::RequestOptions::SetForceSynchronousRequest(synchronous);
            SetUseStorage(false);
            }
//        static RefCountedPtr<RequestOptions> Create( bool synchronous) {return new RequestOptions(synchronous);}
    };

private:
    MRMeshFileData() : MRMeshData() {}

protected:
    virtual Utf8CP _GetId() const override {return GetFilename().c_str();}
    virtual bool _IsExpired() const override {return false;}
    virtual BentleyStatus _InitFrom(IRealityDataBase const& self, RealityDataCacheOptions const& options) override {return InitFromSelf(self, (RequestOptions const&)options);}
    virtual BentleyStatus _InitFrom(Utf8CP filepath, ByteStream const& data, FileRealityDataSource::RequestOptions const& options) override {return InitFromSource(filepath, data, (RequestOptions const&)options);}
    virtual BentleyStatus _InitFrom(Db& db, BeMutex& cs, Utf8CP id, BeSQLiteRealityDataStorage::SelectOptions const& options) override {return InitFromStorage(db, cs, id, (RequestOptions const&)options);} 
    virtual BentleyStatus _Persist(Db& db, BeMutex& cs) const override {return PersistToStorage(db, cs);}
    virtual BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandlerPtr _GetDatabasePrepareAndCleanupHandler() const override {return DatabasePrepareAndCleanupHandler::Create();}
public:
    static RefCountedPtr<MRMeshFileData> Create() {return new MRMeshFileData();}    
};
typedef RefCountedPtr<MRMeshFileData> MRMeshFileDataPtr;

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct   NodeRequest
{
    Transform  m_transform;
    bset<DgnViewportP> m_viewports;

    NodeRequest() {}
    NodeRequest(TransformCR transform, DgnViewportP viewport) : m_transform(transform) { m_viewports.insert(viewport); }
};

typedef bmap<MRMeshNodeP, NodeRequest>   T_RequestMap;
/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct  ThreeMxSchema::MRMeshCache
{
    RealityDataCache* m_cache;
    T_RequestMap  m_requests;
    MRMeshNodeP m_root;
    SystemP m_target;
    bool m_progressiveStarted;

    MRMeshCache(SystemP target) : m_progressiveStarted(false), m_target(target) {Initialize();}
    void SetRoot(MRMeshNodeR node) {m_root = &node;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Initialize()
    {
    static int s_fileThreadCount = 4, s_httpThreadCount = 8;

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    m_cache = &T_HOST.GetRealityDataAdmin().GetCache();
    m_cache->RegisterSource(*FileRealityDataSource::Create(s_fileThreadCount));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RemoveCacheRequests(MRMeshNodeR node)
    {
    T_RequestMap::iterator found = m_requests.find(&node);

    if (found != m_requests.end())
        m_requests.erase(found);

    for (auto const& child : node.GetChildren())
        RemoveCacheRequests(*child);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CloneNodeFromMRMeshData(MRMeshNode& node, MRMeshData const& meshData)
    {
    BeAssert(meshData.GetNode().IsValid());

    // clone the node but keep the parent and the m_info structure the same
    MRMeshNodeP parent = node.m_parent;
    S3NodeInfo  info = node.m_info;
    node.Clone(*meshData.GetNode());
    node.m_parent = parent;
    node.m_info = info;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataCacheResult RequestData(SystemP target, MRMeshNode* node, BeFileNameCR path, bool synchronous = false)
    {
    RealityDataCacheResult result;

    if (path.IsUrl())
        {
        MRMeshHttpDataPtr meshData;
        result = m_cache->Get(meshData, path.GetNameUtf8().c_str(), * new MRMeshHttpData::RequestOptions(target, synchronous));     
        if (RealityDataCacheResult::Success == result && nullptr != node)
            CloneNodeFromMRMeshData(*node, *meshData);
        }
    else
        {
        MRMeshFileDataPtr meshData;
        result = m_cache->Get(meshData, path.GetNameUtf8().c_str(), *new MRMeshFileData::RequestOptions(target, synchronous));
        if (RealityDataCacheResult::Success == result && nullptr != node)
            CloneNodeFromMRMeshData(*node, *meshData);
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshCacheManager::RequestStatus ProcessRequests()
    {
    if (m_requests.empty())
        return  MRMeshCacheManager::RequestStatus::Finished;

    uint64_t startTime = BeTimeUtilities::QueryMillisecondsCounter(), endTime = startTime + 200;
    size_t requestsProcessed = 0;

    for (T_RequestMap::iterator curr = m_requests.begin(); curr != m_requests.end(); )
        {
        if (BeTimeUtilities::QueryMillisecondsCounter() > endTime)
            break;

        if (!curr->first->m_parent->IsLoaded())
            continue;

        BeFileName             fileName = curr->first->GetFileName();
        RealityDataCacheResult cacheStatus = RequestData(m_target, curr->first, fileName, false);
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
                MRMeshUtil::DisplayNodeFailureWarning(fileName.c_str());
                curr->first->m_parent->RemoveChild(curr->first);
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

    return (0 == requestsProcessed) ? MRMeshCacheManager::RequestStatus::None : MRMeshCacheManager::RequestStatus::Processed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    Debug()
    {
    size_t memoryUsage = GetMemoryUsage(), textureMemoryUsage = GetTextureMemoryUsage(), nodeCount = GetNodeCount();
    printf("Node Count: %zu, Memory Usage: %lf, Texture Memory: %lf \n", nodeCount, (double)memoryUsage / (double)ONE_GB, (double)textureMemoryUsage / (double)ONE_GB);
    printf("Memory/Node: %lf Texture/Node: %lf, Mesh Count: %zu, Max Depth: %zu\n", (double)memoryUsage / (double)(nodeCount * 1024), (double)textureMemoryUsage / (double)(nodeCount * 1024), GetMeshCount(), GetMaxDepth());
    printf("Resolution Ratio: %lf\n", MRMeshUtil::CalculateResolutionRatio());

    size_t  load, total, available;

    MRMeshUtil::GetMemoryStatistics(load, total, available);
    printf("Memory Load: %zu, Total Memory: %lf, Available Memory: %lf\n", load, (double)total / (double)ONE_GB, (double)available / (double)ONE_GB);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GetMemoryUsage()
    {
    return m_root->GetMemorySize();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GetTextureMemoryUsage()
    {
    return m_root->GetTextureMemorySize();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GetNodeCount()
    {
    return m_root->GetNodeCount();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GetMeshCount()
    {
    return m_root->GetMeshCount();
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GetMaxDepth()
    {
    return m_root->GetMaxDepth();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FlushStale(uint64_t staleTime)
    {
    if (!m_requests.empty())
        return ERROR;

    m_root->FlushStale(staleTime);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SynchronousRead(MRMeshNodeR node, BeFileNameCR fileName)
    {
    RealityDataCacheResult status = RequestData(m_target, &node, fileName, true);
    if (RealityDataCacheResult::Success != status)
        return ERROR;

    node.SetDirectory(BeFileName(BeFileName::DevAndDir, fileName));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void QueueChildLoad(MRMeshNode::MeshNodes const& children, DgnViewportP viewport, TransformCR transform)
    {
    size_t                  requestsMade = 0;
    static long             s_timerDuration = 10;
    DgnViewportP        indexedViewport = dynamic_cast <DgnViewportP> (viewport);

    for (auto const& child : children)
        {
        BeFileName                  fileName = child->GetFileName();
        T_RequestMap::iterator      found = m_requests.find(child.get());

        
        if (found == m_requests.end())
            {
            BeAssert(child->m_children.empty());
            RequestData(m_target, nullptr, fileName, false);
            m_requests[child.get()] = NodeRequest(transform, indexedViewport);
            requestsMade++;
            }
        else
            {
            found->second.m_viewports.insert(indexedViewport);
            }
        }

    if (s_debugCacheLevel > 3 && requestsMade)
        printf("%d Cache Requests Initiated, %d Exist\n", (int) requestsMade, (int) m_requests.size());

#ifdef WIP
    if (0 != requestsMade && !m_progressiveStarted)
        {
        m_progressiveStarted = true;
        ProgressiveDisplayManager::GetManager().BeginProgressive(*this);
        }
#endif
    }

};  //  MRMeshCache

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshCacheManagerR MRMeshCacheManager::GetManager()
    {
    static MRMeshCacheManager s_manager;
    return s_manager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshCacheManager::QueueChildLoad(MRMeshNode::MeshNodes const& children, DgnViewportP viewport, TransformCR transform, SystemP system)
    {
    if (NULL == m_cache)
        m_cache = new MRMeshCache(system);

    m_cache->QueueChildLoad(children, viewport, transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshCacheManager::SetRoot(MRMeshNodeR node, SystemP system)
    {
    if (NULL == m_cache)
        m_cache = new MRMeshCache(system);

    m_cache->SetRoot(node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MRMeshCacheManager::SynchronousRead(MRMeshNodeR node, BeFileNameCR fileName, SystemP system)
    {
    if (NULL == m_cache)
        m_cache = new MRMeshCache(system);

    return m_cache->SynchronousRead(node, fileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Grigas.Petraitis    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshCacheManager::MRMeshCacheManager()
    {
    }

MRMeshCacheManager::RequestStatus MRMeshCacheManager::ProcessRequests() { return (NULL == m_cache) ? MRMeshCacheManager::RequestStatus::Finished :  m_cache->ProcessRequests(); }
void MRMeshCacheManager::Debug() { if (NULL != m_cache) m_cache->Debug(); }
void MRMeshCacheManager::Flush(uint64_t staleTime) { if (NULL != m_cache) m_cache->FlushStale(staleTime); }
void MRMeshCacheManager::RemoveRequest(MRMeshNodeR node) { if (NULL != m_cache) m_cache->RemoveCacheRequests(node); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Grigas.Petraitis    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshCacheManager::~MRMeshCacheManager()
    {
    if (nullptr != m_cache)
        {
        delete m_cache;
        m_cache = nullptr;
        }
    }

