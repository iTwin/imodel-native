/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// THIS IS NOT FINALIZED CODE, MAJOR API & CACHE STRUCTURE CHANGES ARE POSSIBLE WITHOUT MUCH CONVERSION SUPPORT //

#include <WebServices/Cache/WebServicesCache.h>

#include <WebServices/Cache/ICachingDataSource.h>
#include <WebServices/Cache/NavigationCachingOptions.h>
#include <WebServices/Cache/Persistence/IDataSourceCache.h>
#include <WebServices/Cache/Persistence/IRepositoryInfoStore.h>
#include <WebServices/Cache/Transactions/ICacheTransactionManager.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>
#include <WebServices/Cache/Util/TempFile.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

typedef std::shared_ptr<struct IDataSourceCache> IDataSourceCachePtr;
typedef std::shared_ptr<struct CachingDataSource> CachingDataSourcePtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma   01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE CachingDataSource :
    public ICachingDataSource,
    public std::enable_shared_from_this<CachingDataSource>
    {
    public:
        friend struct CacheNavigationTask;
        friend struct DownloadFilesTask;
        friend struct SyncLocalChangesTask;
        friend struct SyncCachedDataTask;
        friend struct FileDownloadManager;

        typedef AsyncResult<CachingDataSourcePtr, Error> OpenResult;

    private:
        struct DataOriginStatus
            {
            DataOrigin origin;
            SyncStatus syncStatus;
            };

    private:
        typedef AsyncResult<DataOriginStatus, Error> DataOriginResult;

    private:
        bool                                        m_isOpen = false;
        IWSRepositoryClientPtr                      m_client;
        std::shared_ptr<ICacheTransactionManager>   m_cacheTransactionManager;
        std::shared_ptr<IRepositoryInfoStore>       m_infoStore;
        std::unique_ptr<struct SessionInfo>         m_sessionInfo;
        std::shared_ptr<struct FileDownloadManager> m_fileDownloadManager;

        WorkerThreadPtr                             m_cacheAccessThread;
        SimpleCancellationTokenPtr                  m_cancellationToken;

        BeFileName                                  m_temporaryDir;
        NavigationCachingOptions                    m_cachingOptions;

        std::deque<std::shared_ptr<AsyncTask>>      m_syncLocalChangesQueue;
        bool                                        m_enableSkipTokens = false;

        size_t                                      m_maxParalelDownloads = 10;
        uint64_t                                    m_minTimeBetweenProgressCallsMs = 250;

    private:
        CachingDataSource
            (
            IWSRepositoryClientPtr client,
            std::shared_ptr<ICacheTransactionManager> cacheTransactionManager,
            std::shared_ptr<IRepositoryInfoStore> infoStore,
            WorkerThreadPtr cacheAccessThread,
            BeFileNameCR temporaryDir
            );

        BentleyStatus FinalizeOpen(CacheTransactionCR txn);
        ICancellationTokenPtr CreateCancellationToken(ICancellationTokenPtr ct);

        static bool IsServerSchemaSupported(CacheTransactionCR txn, JsonValueCR schemaDef);
        static bool IsServerSchemaDeprecated(JsonValueCR schemaDef);
        static SchemaKey ExtractSchemaKey(JsonValueCR schemaDef);

        TempFilePtr GetTempFile(Utf8StringCR fileName, ObjectIdCR objectId); // TODO: create mockable file manager
        TempFilePtr GetTempFileForSchema(SchemaKeyCR schemaKey);

        Utf8String GetObjectLabel(CacheTransactionCR txn, ObjectIdCR objectId);
        Utf8String GetObjectLabel(CacheTransactionCR txn, ECInstanceKeyCR instanceKey);

        AsyncTaskPtr<Result> CacheObject
            (
            ObjectIdCR objectId,
            ICancellationTokenPtr ct
            );

        CachedResponseKey CreateSchemaListResponseKey(CacheTransactionCR txn);

        AsyncTaskPtr<DataOriginResult> CacheObjects
            (
            CachedResponseKeyCR responseKey,
            WSQueryCR query,
            DataOrigin origin,
            Utf8StringCR skipToken,
            uint64_t page,
            ICancellationTokenPtr ct
            );

        void CacheObjectsInBackgroundIfNeeded
            (
            CachedResponseKeyCR responseKey,
            WSQueryCR query,
            RetrieveOptions retrieveOptions,
            DataOriginResult result,
            ICancellationTokenPtr ct
            );

        AsyncTaskPtr<ObjectsResult> GetObjectInternal
            (
            ObjectIdCR objectId,
            DataOrigin origin,
            ICancellationTokenPtr ct = nullptr
            );

        void GetObjectInBackgroundIfNeeded
            (
            ObjectIdCR objectId,
            RetrieveOptions retrieveOptions,
            CachingDataSource::ObjectsResult result,
            ICancellationTokenPtr ct
            );

        AsyncTaskPtr<DataOriginResult> CacheNavigationChildren
            (
            ObjectIdCR parentId,
            DataOrigin origin,
            std::shared_ptr<const ISelectProvider> serverReadOptions,
            ICancellationTokenPtr ct
            );

        AsyncTaskPtr<BatchResult> SyncLocalChanges
            (
            std::shared_ptr<bset<ECInstanceKey>> instancesToSync,
            ProgressCallback onProgress,
            ICancellationTokenPtr ct,
            SyncOptions options
            );

        AsyncTaskPtr<BatchResult> DownloadAndCacheFiles
            (
            bset<ObjectId> filesToDownload,
            FileCache fileCacheLocation,
            CachingDataSource::ProgressCallback onProgress,
            ICancellationTokenPtr ct
            );

        AsyncTaskPtr<BatchResult> DownloadAndCacheFiles
        (
            bmap<ObjectId, ICancellationTokenPtr> filesToDownload,
            FileCache fileCacheLocation,
            CachingDataSource::ProgressCallback onProgress,
            ICancellationTokenPtr ct
        );

        Utf8String GetInitialSkipToken() const;

    public:
        virtual ~CachingDataSource() override;

        //! Host should call to establish search paths for caching.
        //! @param[in] hostAssetsDirectory Directory to where the application has deployed assets that come with the API,
        //!            e.g. standard ECSchemas.
        WSCACHE_EXPORT static void Initialize (BeFileNameCR hostAssetsDirectory);

        //! Be sure to initialize APIs before executing caching code:
        //!     [Required] BeSQLite::EC::ECDb::Initialize()
        //!     [Required] BeSQLite::L10N::Initialize ()
        //!     [Required] CachingDataSource::Initialize ()
        //!     [Optional] HttpClient::InitializeNetworkActivityCallback()
        //! @param client - used for communicating to server
        //! @param cacheFilePath - cache database file path to store or open
        //! @param cacheEnvironment - cache environment to use for file caching
        //! @param cacheAccessThread - thread to do all I/O on cache
        //! @param ct - cancellation token to cancel open/creation 
        //! @return pointer to valid CachingDataSource or error that occurred
        WSCACHE_EXPORT static AsyncTaskPtr<OpenResult> OpenOrCreate
            (
            IWSRepositoryClientPtr client,
            BeFileNameCR cacheFilePath,
            CacheEnvironmentCR cacheEnvironment,
            WorkerThreadPtr cacheAccessThread = nullptr,
            ICancellationTokenPtr ct = nullptr
            );

        //! DO NOT USE for production! Used only for internal testing and may be removed in future.
        WSCACHE_EXPORT static CachingDataSourcePtr Create
            (
            IWSRepositoryClientPtr client,
            std::shared_ptr<ICacheTransactionManager> cacheTransactionManager,
            std::shared_ptr<IRepositoryInfoStore> infoStore,
            WorkerThreadPtr cacheAccessThread,
            BeFileNameCR temporaryDir
            );

        //! Force close connection to server and local database. Makes object unusable after this.
        WSCACHE_EXPORT void Close();
        
        //! Gets the host assets directory to where the application deploys assets that come with the API, e.g. standard ECSchemas.
        //! Must have been set via CachingDataSource::Initialize.
        //! @return Host assets directory
        WSCACHE_EXPORT static BeFileNameCR GetHostAssetsDirectory ();

        WSCACHE_EXPORT IWSRepositoryClientPtr GetClient() const override;
        WSCACHE_EXPORT void SetClient(IWSRepositoryClientPtr client) override;

        WSCACHE_EXPORT AsyncTaskPtr<void> CancelAllTasks() override;

        WSCACHE_EXPORT void EnableSkipTokens(bool enable) override;
        //! Configure maximum paralel file download limit in one operation. Default is 10.
        WSCACHE_EXPORT void SetMaxParalelFileDownloadLimit(size_t maxParalelDownloads);
        //! Configure minimum time limit between progress callbacks in milliseconds. Default is 250 ms.
        WSCACHE_EXPORT void SetMinTimeBetweenProgressCalls(uint64_t minTimeBetweenProgressCallsMs);

        WSCACHE_EXPORT AsyncTaskPtr<Result> UpdateSchemas(ICancellationTokenPtr ct = nullptr) override;

        WSCACHE_EXPORT CacheTransaction StartCacheTransaction() override;
        WSCACHE_EXPORT WorkerThreadPtr GetCacheAccessThread() override;

        WSCACHE_EXPORT bvector<ECN::ECSchemaCP> GetRepositorySchemas(CacheTransactionCR txn) override;
        WSCACHE_EXPORT bvector<SchemaKey> GetRepositorySchemaKeys(CacheTransactionCR txn) override;

        //! DEPRECATED, use GetServerInfo()
        WSCACHE_EXPORT WSInfo GetServerInfo(CacheTransactionCR txn) override;
        WSCACHE_EXPORT WSInfo GetServerInfo() override;
        WSCACHE_EXPORT WSRepository GetRepositoryInfo() override;

        WSCACHE_EXPORT void SetClassesToAlwaysCacheChildren(const bset<Utf8String>& classesToAlwaysCacheChildren) override;

        WSCACHE_EXPORT CachedResponseKey GetNavigationResponseKey(CacheTransactionCR txn, ObjectIdCR parentId) override;
        WSCACHE_EXPORT CachedResponseKey GetNavigationResponseKey(CacheTransactionCR txn, ECInstanceKeyCR parentKey) override;
        WSCACHE_EXPORT WSQueryPtr GetNavigationQuery(CacheTransactionCR txn, ObjectIdCR parentId, ISelectProviderPtr selectProvider = nullptr) override;

        WSCACHE_EXPORT AsyncTaskPtr<ObjectsResult> GetObject
            (
            ObjectIdCR objectId,
            RetrieveOptions retrieveOptions,
            ICancellationTokenPtr ct = nullptr
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<ObjectsResult> GetObjects
            (
            CachedResponseKeyCR responseKey,
            WSQueryCR query,
            RetrieveOptions retrieveOptions,
            std::shared_ptr<const ISelectProvider> cachedSelectProvider = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<KeysResult> GetObjectsKeys
            (
            CachedResponseKeyCR responseKey,
            WSQueryCR query,
            RetrieveOptions retrieveOptions,
            ICancellationTokenPtr ct = nullptr
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<ObjectsResult> GetNavigationChildren
            (
            ObjectIdCR parentId,
            DataOrigin origin,
            std::shared_ptr<const SelectProvider> readOptions = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<KeysResult> GetNavigationChildrenKeys
            (
            ObjectIdCR parentId,
            DataOrigin origin,
            std::shared_ptr<const ISelectProvider> remoteSelectProvider = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<FileResult> GetFile
            (
            ObjectIdCR fileId,
            DataOrigin origin,
            ProgressCallback onProgress = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<BatchResult> CacheFiles
            (
            const bvector<ObjectId>& filesIds,
            bool skipCachedFiles = false,
            FileCache fileCacheLocation = FileCache::Auto,
            ProgressCallback onProgress = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<Result> DownloadAndCacheChildren
            (
            const bvector<ObjectId>& parentIds,
            ICancellationTokenPtr ct = nullptr
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<BatchResult> SyncLocalChanges
            (
            ProgressCallback onProgress = nullptr,
            ICancellationTokenPtr ct = nullptr,
            SyncOptions options = SyncOptions()
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<BatchResult> SyncLocalChanges
            (
            const bset<ECInstanceKey>& instancesToSync,
            ProgressCallback onProgress = nullptr,
            ICancellationTokenPtr ct = nullptr,
            SyncOptions options = SyncOptions()
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<BatchResult> SyncCachedData
            (
            bvector<ECInstanceKey> initialInstances,
            bvector<IQueryProvider::Query> initialQueries,
            bvector<IQueryProviderPtr> queryProviders,
            ProgressHandler progressHandler = ProgressHandler(),
            ICancellationTokenPtr ct = nullptr
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<BatchResult> CacheNavigation
            (
            const bvector<ObjectId>& persistenceNavigationTrees,
            const bvector<ObjectId>& temporaryNavigationTrees,
            std::shared_ptr<const ISelectProvider> temporaryNavigationTreesServerSelectProvider,
            ProgressCallback onProgress = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
