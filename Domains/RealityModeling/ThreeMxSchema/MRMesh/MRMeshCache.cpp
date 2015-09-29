/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMeshCache.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "..\ThreeMxSchemaInternal.h"

#include    <DgnPlatform/DgnCore/HttpHandler.h>

#include    <windows.h>


USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

#define     ONE_GB   (1024 * 1024 * 1024)

int s_debugCacheLevel = 0;



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2005
+---------------+---------------+---------------+---------------+---------------+------*/
double   getTimeResolution ()
    {
    // Thanks to http://www.codeproject.com/cpp/duration.asp for pointing put how to use QueryPerformanceCounter, etc.
    LARGE_INTEGER li;
    QueryPerformanceFrequency (&li);
    return (double)li.QuadPart;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2005
+---------------+---------------+---------------+---------------+---------------+------*/
double   getTime ()
    {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return (double)li.QuadPart;
    }

static double   s_timerResolution = getTimeResolution();

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            04/2015
//=======================================================================================
struct BVectorStreamBuffer : public std::basic_streambuf<Utf8Char>
    {
    bvector<Byte> const& m_vector;
    BVectorStreamBuffer(bvector<Byte> const& vec) : m_vector(vec) {setg((Utf8P)vec.data(), (Utf8P)vec.data(), (Utf8P)(vec.data() + vec.size()));}

    virtual pos_type __CLR_OR_THIS_CALL seekoff(off_type offset, std::ios_base::seekdir dir, std::ios_base::openmode = std::ios_base::in | std::ios_base::out) override
        {
        switch (dir)
            {
            case std::ios_base::beg:
                setg((Utf8P)m_vector.data(), (Utf8P)m_vector.data(), (Utf8P)(m_vector.data() + m_vector.size()));
                gbump((int) offset);
                break;
            case std::ios_base::cur:
                gbump((int)offset);
                break;
            case std::ios_base::end:
                setg((Utf8P)m_vector.data(), (Utf8P)(m_vector.data() + m_vector.size()), (Utf8P)(m_vector.data() + m_vector.size()));
                gbump(-1 * (int) offset);
                break;
            }
        return 1;
	}

    virtual pos_type __CLR_OR_THIS_CALL seekpos(pos_type position, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) override
        {
        return seekoff(position, std::ios_base::beg, mode);
        }
    };

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
        RequestOptions() : RealityDataCacheOptions(true, false, true, true) {}
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
            if (db.TableExists (TABLE_NAME_Acute3dMesh))
                {
                s_isPrepared = true;
                return SUCCESS;
                }

            Utf8CP ddl = "Filename CHAR PRIMARY KEY, \
                          Data BLOB,                 \
                          DataSize BIGINT,           \
                          Created BIGINT";

            if (BeSQLite::BE_SQLITE_OK == db.CreateTable(TABLE_NAME_Acute3dMesh, ddl))
                {
                s_isPrepared = true;
                return SUCCESS;
                }
            return ERROR;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Grigas.Petraitis                03/2015
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual BentleyStatus _CleanupDatabase(BeSQLite::Db& db, double percentage) const override
            {
            CachedStatementPtr sumStatement;
            if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement (sumStatement, "SELECT SUM(DataSize) FROM " TABLE_NAME_Acute3dMesh))
                return ERROR;

            if (BeSQLite::BE_SQLITE_ROW != sumStatement->Step ())
                return ERROR;

            uint64_t rasterSize = sumStatement->GetValueInt64 (0);
            uint64_t overhead = (uint64_t) (rasterSize * percentage) / 100;

            CachedStatementPtr selectStatement;
            if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement (selectStatement, "SELECT DataSize, Created FROM " TABLE_NAME_Acute3dMesh " ORDER BY Created ASC"))
                return ERROR;

            uint64_t runningSum = 0;
            while ((runningSum < overhead) && (BeSQLite::BE_SQLITE_ROW == selectStatement->Step ()))
                runningSum += selectStatement->GetValueInt64 (0);

            uint64_t creationDate = selectStatement->GetValueInt64 (1);
            CachedStatementPtr deleteStatement;
            if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement (deleteStatement, "DELETE FROM " TABLE_NAME_Acute3dMesh " WHERE Created <= ?"))
                return ERROR;

            deleteStatement->BindInt64 (1, creationDate);
            if (BeSQLite::BE_SQLITE_DONE != deleteStatement->Step ())
                return ERROR;

            return SUCCESS;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Grigas.Petraitis                03/2015
        +---------------+---------------+---------------+---------------+---------------+------*/
        static RefCountedPtr<DatabasePrepareAndCleanupHandler> Create() {return new DatabasePrepareAndCleanupHandler();}
        };
private:
    Utf8String m_filename;
    MRMeshNodePtr m_node;
    mutable bvector<Byte> m_nodeBytes;

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus InitFromSelf(IRealityDataBase const& self, RequestOptions const& options)
        {
        MRMeshData const& other = dynamic_cast<MRMeshData const&>(self);
        m_node = MRMeshNode::Create();
        m_node->Clone(*other.GetNode());
        m_filename = other.GetFilename();
        return SUCCESS;
        }
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus InitFromSource(Utf8CP filename, bvector<Byte> const& data, RequestOptions const& options)
        {
        m_filename = filename;
        m_node = MRMeshNode::Create();
        
        m_nodeBytes = data;
        BVectorStreamBuffer buff(m_nodeBytes);
        std::istream stream(&buff);
        std::string error;
    
        BeAssert (m_node->m_children.empty());
        if (SUCCESS != m_node->Read3MXB (stream, error))
            {
            // Needs work... .Error message.
            BeAssert (false && error.c_str());
            m_nodeBytes.clear();
            m_node = nullptr;
            return ERROR;
            }
        if (!options.UseStorage())
            m_nodeBytes.clear();

        return SUCCESS;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus InitFromStorage(BeSQLite::Db& db, BeMutex& cs, Utf8CP id, RequestOptions const& options)
        {
        if (true)
            {
            BeMutexHolder lock(cs);

            CachedStatementPtr stmt;
            if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement (stmt, "SELECT Data, DataSize FROM " TABLE_NAME_Acute3dMesh " WHERE Filename=?"))
                return ERROR;

            stmt->ClearBindings();
            stmt->BindText (1, id, BeSQLite::Statement::MakeCopy::No);
            if (BeSQLite::BE_SQLITE_ROW != stmt->Step ())
                return ERROR;

            m_filename = id;

            const void* data = stmt->GetValueBlob (0);
            int dataSize = stmt->GetValueInt (1);
            m_nodeBytes.assign((Byte*)data, (Byte*)data + dataSize);
            }

        m_node = MRMeshNode::Create();

        BVectorStreamBuffer buff(m_nodeBytes);
        std::istream stream(&buff);
        std::string error;
        BeAssert (m_node->m_children.empty());
        BentleyStatus result = m_node->Read3MXB(stream, error);
        if (SUCCESS != result)
            {
            BeAssert (false && error.c_str());
            m_node = nullptr;
            return ERROR;
            }
        m_nodeBytes.clear();
        return SUCCESS;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus PersistToStorage(BeSQLite::Db& db, BeMutex& cs) const
        {
        BeMutexHolder lock(cs);

        CachedStatementPtr selectStatement;
        if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement (selectStatement, "SELECT Filename FROM " TABLE_NAME_Acute3dMesh " WHERE Filename=?"))
            return ERROR;

        selectStatement->ClearBindings();
        selectStatement->BindText (1, m_filename.c_str(), BeSQLite::Statement::MakeCopy::No);
        if (BeSQLite::BE_SQLITE_ROW == selectStatement->Step ())
            {
            // update
            }
        else
            {
            BeAssert(!m_nodeBytes.empty());

            // insert
            CachedStatementPtr stmt;
            if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement (stmt, "INSERT INTO " TABLE_NAME_Acute3dMesh " (Filename, Data, DataSize, Created) VALUES (?,?,?,?)"))
                return ERROR;

            stmt->ClearBindings ();
            stmt->BindText(1, m_filename.c_str(), BeSQLite::Statement::MakeCopy::No);
            stmt->BindBlob(2, m_nodeBytes.data(), (int)m_nodeBytes.size(), BeSQLite::Statement::MakeCopy::No);
            stmt->BindInt64(3, (int64_t)m_nodeBytes.size());
            stmt->BindInt64 (4, BeTimeUtilities::GetCurrentTimeAsUnixMillis());
            if (BeSQLite::BE_SQLITE_DONE != stmt->Step ())
                return ERROR;
            }

        m_nodeBytes.clear();
        return SUCCESS;
        }
public:
    virtual ~MRMeshData() {}
    MRMeshNodePtr GetNode() const {return m_node;}
    Utf8StringCR GetFilename() const {return m_filename;}
};
BeAtomic<bool> MRMeshData::DatabasePrepareAndCleanupHandler::s_isPrepared = false;

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
    private:
        RequestOptions(bool synchronous) : MRMeshData::RequestOptions()
            {
            DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT
            BeSQLiteRealityDataStorage::SelectOptions::SetForceSynchronousRequest(synchronous);
            HttpRealityDataSource::RequestOptions::SetForceSynchronousRequest(synchronous);
            SetUseStorage(true);
            }
    public:
        static RefCountedPtr<RequestOptions> Create(bool synchronous) {return new RequestOptions(synchronous);}
    };

private:
    MRMeshHttpData () {}
protected:
    virtual Utf8CP _GetId() const override {return GetFilename().c_str();}
    virtual bool _IsExpired() const override {return false;}
    virtual BentleyStatus _InitFrom(IRealityDataBase const& self, RealityDataCacheOptions const& options) override {return InitFromSelf(self, (RequestOptions const&)options);}
    virtual BentleyStatus _InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, bvector<Byte> const& body, HttpRealityDataSource::RequestOptions const& options) override {return InitFromSource(url, body, (RequestOptions const&)options);}
    virtual BentleyStatus _InitFrom(BeSQLite::Db& db, BeMutex& cs, Utf8CP id, BeSQLiteRealityDataStorage::SelectOptions const& options) override {return InitFromStorage(db, cs, id, (RequestOptions const&)options);}
    virtual BentleyStatus _Persist(BeSQLite::Db& db, BeMutex& cs) const override {return PersistToStorage(db, cs);}
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
    private:
        RequestOptions(bool synchronous) : MRMeshData::RequestOptions()
            {
            DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT
            BeSQLiteRealityDataStorage::SelectOptions::SetForceSynchronousRequest(synchronous);
            FileRealityDataSource::RequestOptions::SetForceSynchronousRequest(synchronous);
            SetUseStorage(false);
            }
    public:
        static RefCountedPtr<RequestOptions> Create( bool synchronous) {return new RequestOptions(synchronous);}
    };

private:
    MRMeshFileData () {}
protected:
    virtual Utf8CP _GetId() const override {return GetFilename().c_str();}
    virtual bool _IsExpired() const override {return false;}
    virtual BentleyStatus _InitFrom(IRealityDataBase const& self, RealityDataCacheOptions const& options) override {return InitFromSelf(self, (RequestOptions const&)options);}
    virtual BentleyStatus _InitFrom(Utf8CP filepath, bvector<Byte> const& data, FileRealityDataSource::RequestOptions const& options) override {return InitFromSource(filepath, data, (RequestOptions const&)options);}
    virtual BentleyStatus _InitFrom(BeSQLite::Db& db, BeMutex& cs, Utf8CP id, BeSQLiteRealityDataStorage::SelectOptions const& options) override {return InitFromStorage(db, cs, id, (RequestOptions const&)options);}
    virtual BentleyStatus _Persist(BeSQLite::Db& db, BeMutex& cs) const override {return PersistToStorage(db, cs);}
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
    Transform                   m_transform;
    bset<DgnViewportP>          m_viewports;

    NodeRequest () { }
    NodeRequest (TransformCR transform, DgnViewportP viewport) : m_transform (transform) { m_viewports.insert (viewport); }

};

typedef bmap <MRMeshNodeP, NodeRequest>   T_RequestMap;
/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct  BentleyApi::ThreeMxSchema::MRMeshCache
{
    RealityDataCache*               m_cache;
    T_RequestMap                    m_requests;
    bset <MRMeshNodeP>              m_roots;

    bool                            m_progressiveStarted;

    MRMeshCache () : m_progressiveStarted (false)   { Initialize();  }

    void            AddRoot (MRMeshNodeR node)      { m_roots.insert (&node); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Initialize ()
    {
    static int          s_fileThreadCount = 4, s_httpThreadCount = 8;

    m_cache = &T_HOST.GetRealityDataAdmin().GetCache();
    m_cache->RegisterSource(*FileRealityDataSource::Create(s_fileThreadCount));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RemoveCacheRequests (MRMeshNodeR node)
    {
    T_RequestMap::iterator found = m_requests.find(&node);

    if (found != m_requests.end())
        m_requests.erase (found);

    for (auto& child : node.m_children)
        RemoveCacheRequests (*child);
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
RealityDataCacheResult RequestData(MRMeshNode* node, BeFileNameCR path, bool synchronous = false)
    {
    RealityDataCacheResult result;

    if (path.IsUrl())
        {
        MRMeshHttpDataPtr meshData;
        result = m_cache->Get (meshData, path.GetNameUtf8().c_str(), *MRMeshHttpData::RequestOptions::Create(synchronous));
        if (RealityDataCacheResult::Success == result && nullptr != node)
            CloneNodeFromMRMeshData(*node, *meshData);
        }
    else
        {
        MRMeshFileDataPtr meshData;
        result = m_cache->Get (meshData, path.GetNameUtf8().c_str(), *MRMeshFileData::RequestOptions::Create(synchronous));
        if (RealityDataCacheResult::Success == result && nullptr != node)
            CloneNodeFromMRMeshData(*node, *meshData);
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshCacheManager::RequestStatus ProcessRequests ()
    {
    if (m_requests.empty())
        return  MRMeshCacheManager::RequestStatus::Finished;

    static double       s_timeoutDelta = .2 * s_timerResolution;
    double              startTime = getTime(), endTime = startTime + s_timeoutDelta;
    size_t              requestsProcessed = 0;

    for (T_RequestMap::iterator curr = m_requests.begin(); curr != m_requests.end(); )
        {
        if (getTime() > endTime)
            break;

        if (!curr->first->m_parent->IsLoaded())
            continue;

        BeFileName             fileName = curr->first->GetFileName();
        RealityDataCacheResult cacheStatus = RequestData(curr->first, fileName, false);
        switch (cacheStatus)
            {
            case RealityDataCacheResult::Success:
                {
                curr->first->_SetDirectory (BeFileName (BeFileName::DevAndDir, fileName));

                requestsProcessed++;
                curr = m_requests.erase (curr);
                break;
                }

            case RealityDataCacheResult::NotFound:
                {
                MRMeshUtil::DisplayNodeFailureWarning (fileName.c_str());
                curr->first->m_parent->RemoveChild (curr->first);
                curr = m_requests.erase (curr);
                break;
                }

            case RealityDataCacheResult::RequestQueued:
                curr++;
                break;

            default:
                BeAssert (false);
                break;
            }
        }

    return (0 == requestsProcessed) ? MRMeshCacheManager::RequestStatus::None : MRMeshCacheManager::RequestStatus::Processed;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    Debug ()
    {
    size_t memoryUsage = GetMemoryUsage(), textureMemoryUsage = GetTextureMemoryUsage(), nodeCount = GetNodeCount();

    printf ("Node Count: %ld, Memory Usage: %lf, Texture Memory: %lf \n",  nodeCount, (double) memoryUsage / (double) ONE_GB, (double) textureMemoryUsage / (double) ONE_GB);
    printf ("Memory/Node: %lf Texture/Node: %lf, Mesh Count: %ld, Max Depth: %d\n", (double) memoryUsage / (double) (nodeCount * 1024), (double) textureMemoryUsage / (double) (nodeCount * 1024), GetMeshCount(), GetMaxDepth());
    printf ("Resolution Ratio: %lf\n", MRMeshUtil::CalculateResolutionRatio());

    size_t      load, total, available;

    MRMeshUtil::GetMemoryStatistics (load, total, available);
    printf ("Memory Load: %ld, Total Memory: %lf, Available Memory: %lf\n", load, (double) total/(double) ONE_GB, (double) available/(double) ONE_GB);
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GetMemoryUsage ()
    {
    size_t      memoryUsage = 0;
    for (auto& root : m_roots)
        memoryUsage += root->GetMemorySize();

    return memoryUsage;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GetTextureMemoryUsage ()
    {
    size_t      memoryUsage = 0;
    for (auto& root : m_roots)
        memoryUsage += root->GetTextureMemorySize();

    return memoryUsage;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GetNodeCount ()
    {
    size_t      nodeCount = 0;

    for (auto& root : m_roots)
        nodeCount += root->GetNodeCount ();

    return nodeCount;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GetMeshCount ()
    {
    size_t      count = 0;

    for (auto& root : m_roots)
        count += root->GetMeshCount();

    return count;
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GetMaxDepth ()
    {
    size_t      maxDepth = 0;

    for (auto& root : m_roots)
        {
        size_t      depth = root->GetMaxDepth();

        if (depth > maxDepth)
            maxDepth = depth;
        }

    return maxDepth;
    }


               
  /*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   FlushStale (uint64_t staleTime)
    {
    if (!m_requests.empty())
        return ERROR;

    for (auto& root : m_roots)
        root->FlushStale (staleTime);

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SynchronousRead (MRMeshNodeR node, BeFileNameCR fileName)
    {
    RealityDataCacheResult status = RequestData(&node, fileName, true);
    if (RealityDataCacheResult::Success != status)
        return ERROR;

    node._SetDirectory (BeFileName (BeFileName::DevAndDir, fileName));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void QueueChildLoad (T_MeshNodeArray const& children, DgnViewportP viewport, TransformCR transform)
    {
    size_t                  requestsMade = 0;
    static long             s_timerDuration = 10;
    DgnViewportP        indexedViewport = dynamic_cast <DgnViewportP> (viewport);

    for (auto const& child : children)
        {
        BeFileName                  fileName = child->GetFileName();
        T_RequestMap::iterator      found = m_requests.find (child.get());

        
        if (found == m_requests.end())
            {
            BeAssert (child->m_children.empty());
            RequestData(nullptr, fileName, false);
            m_requests[child.get()] = NodeRequest (transform, indexedViewport);
            requestsMade++;
            }
        else
            {
            found->second.m_viewports.insert (indexedViewport);
            }
        }

    if (s_debugCacheLevel > 3 && requestsMade)
        printf ("%d Cache Requests Initiated, %d Exist\n", (int) requestsMade, (int) m_requests.size());

#ifdef WIP
    if (0 != requestsMade && !m_progressiveStarted)
        {
        m_progressiveStarted = true;
        ProgressiveDisplayManager::GetManager().BeginProgressive (*this);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    RemoveRoot (MRMeshNodeR node)
    {
    m_cache->Cleanup();
    RemoveCacheRequests (node);

    auto found = m_roots.find (&node);
    if (found != m_roots.end())
        m_roots.erase (found);
    }

 
};  //  MRMeshCache


MRMeshCacheManager MRMeshCacheManager::s_manager;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshCacheManager::QueueChildLoad (T_MeshNodeArray const& children, DgnViewportP viewport, TransformCR transform)
    {
    if (NULL == m_cache)
        m_cache = new MRMeshCache ();

    m_cache->QueueChildLoad (children, viewport, transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshCacheManager::AddRoot (MRMeshNodeR node)
    {
    if (NULL == m_cache)
        m_cache = new MRMeshCache ();

    m_cache->AddRoot (node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MRMeshCacheManager::SynchronousRead (MRMeshNodeR node, BeFileNameCR fileName)
    {
    if (NULL == m_cache)
        m_cache = new MRMeshCache ();

    return m_cache->SynchronousRead (node, fileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void            MRMeshCacheManager::RemoveRoot (MRMeshNodeR node)
    {
    m_cache->RemoveRoot (node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Grigas.Petraitis    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshCacheManager::MRMeshCacheManager()
    {
    }

MRMeshCacheManager::RequestStatus MRMeshCacheManager::ProcessRequests () { return (NULL == m_cache) ? MRMeshCacheManager::RequestStatus::Finished :  m_cache->ProcessRequests(); }
void MRMeshCacheManager::Debug ()                           { if (NULL != m_cache) m_cache->Debug(); }
void MRMeshCacheManager::Flush (uint64_t staleTime)         { if (NULL != m_cache) m_cache->FlushStale (staleTime); }
void MRMeshCacheManager::RemoveRequest (MRMeshNodeR node)   { if (NULL != m_cache) m_cache->RemoveCacheRequests (node); }

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

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  MRMeshUtil::ReadSceneFile (S3SceneInfo& sceneInfo, WCharCP  fileName)
    {
    std::string     err;
    BeFileName      beFileName (fileName);

    if (!beFileName.IsUrl())
        return BaseSceneNode::Read3MX (beFileName, sceneInfo, err);

    Utf8String                      url;
    bmap<Utf8String, Utf8String>    header;

    HttpRequest         request (beFileName.GetNameUtf8().c_str(), header);
    HttpResponsePtr     response;

    if (HttpRequestStatus::Success != HttpHandler::Instance().Request (response, request))
        return ERROR;


    BVectorStreamBuffer buff (response->GetBody());
    std::istream        stream (&buff);

    return (SUCCESS == BaseSceneNode::Read3MX (stream, sceneInfo, err)) ? SUCCESS : ERROR;
    }


























