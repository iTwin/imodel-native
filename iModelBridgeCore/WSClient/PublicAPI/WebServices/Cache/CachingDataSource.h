/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/CachingDataSource.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
struct CachingDataSource :
    public ICachingDataSource,
    public std::enable_shared_from_this<CachingDataSource>
    {
    public:
        friend struct CacheNavigationTask;
        friend struct DownloadFilesTask;
        friend struct SyncLocalChangesTask;
        friend struct SyncCachedDataTask;

        typedef AsyncResult<CachingDataSourcePtr, Error> OpenResult;

    private:
        typedef AsyncResult<DataOrigin, Error> DataOriginResult;

    private:
        IWSRepositoryClientPtr                      m_client;
        std::shared_ptr<ICacheTransactionManager>   m_cacheTransactionManager;
        std::shared_ptr<IRepositoryInfoStore>       m_infoStore;

        WorkerThreadPtr                             m_cacheAccessThread;
        SimpleCancellationTokenPtr                  m_cancellationToken;

        BeFileName                                  m_temporaryDir;
        NavigationCachingOptions                    m_cachingOptions;

        bvector<std::shared_ptr<AsyncTask>>         m_syncLocalChangesQueue;

    private:
        CachingDataSource
            (
            IWSRepositoryClientPtr client,
            std::shared_ptr<ICacheTransactionManager> cacheTransactionManager,
            std::shared_ptr<IRepositoryInfoStore> infoStore,
            WorkerThreadPtr cacheAccessThread,
            BeFileNameCR temporaryDir
            );

        void ExecuteNextSyncLocalChangesTask();
        ICancellationTokenPtr CreateCancellationToken(ICancellationTokenPtr ct);

        SchemaKey ReadSchemaKey(CacheTransactionCR txn, ObjectIdCR schemaId);

        TempFilePtr GetTempFile(Utf8StringCR fileName, ObjectIdCR objectId); // TODO: create mockable file manager
        TempFilePtr GetTempFileForSchema(SchemaKeyCR schemaKey);

        Utf8String GetObjectLabel(CacheTransactionCR txn, ObjectIdCR objectId);
        Utf8String GetObjectLabel(CacheTransactionCR txn, ECInstanceKeyCR instanceKey);

        AsyncTaskPtr<Result> CacheObject
            (
            ObjectIdCR objectId,
            ICancellationTokenPtr ct
            );

        BentleyStatus LoadSchemas
            (
            const std::vector<BeFileName>& schemaFilePaths,
            std::vector<ECSchemaPtr>& schemasOut
            );

        void NotifyOnCacheSchemaChangedListeners();
        CachedResponseKey CreateSchemaListResponseKey(CacheTransactionCR txn);

        static BeFileName GetMetaSchemaPath();

        AsyncTaskPtr<DataOriginResult> CacheObjects
            (
            CachedResponseKeyCR responseKey,
            WSQueryCR query,
            DataOrigin origin,
            Utf8StringCR skipToken,
            uint64_t page,
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
            SyncProgressCallback onProgress,
            ICancellationTokenPtr ct,
            SyncOptions options
            );

    public:
        virtual ~CachingDataSource() override;

        //! Be sure to initialize APIs before executing caching code:
        //!     [Required] BeSQLite::EC::ECDb::Initialize()
        //!     [Required] MobileDgn {platform specific} ::Initialize()
        //!     [Required] MobileDgnL10N::Initialize() (error message localization needs SQLang file built from HttpError.xliff.h)
        //!     [Optional] HttpClient::InitializeNetworkActivityCallback()
        WSCACHE_EXPORT static AsyncTaskPtr<OpenResult> OpenOrCreate
            (
            IWSRepositoryClientPtr client,
            BeFileNameCR cacheFilePath,
            CacheEnvironmentCR cacheEnvironment,
            WorkerThreadPtr cacheAccessThread = nullptr
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

        WSCACHE_EXPORT IWSRepositoryClientPtr GetClient() const override;
        WSCACHE_EXPORT void SetClient(IWSRepositoryClientPtr client) override;

        WSCACHE_EXPORT void CancelAllTasksAndWait() override;

        WSCACHE_EXPORT AsyncTaskPtr<Result> UpdateSchemas(ICancellationTokenPtr ct) override;

        WSCACHE_EXPORT CacheTransaction StartCacheTransaction() override;
        WSCACHE_EXPORT WorkerThreadPtr GetCacheAccessThread() override;

        WSCACHE_EXPORT ECSchemaList GetRepositorySchemas(CacheTransactionCR txn) override;
        WSCACHE_EXPORT bvector<SchemaKey> GetRepositorySchemaKeys(CacheTransactionCR txn) override;
        WSCACHE_EXPORT WSInfo GetServerInfo(CacheTransactionCR txn) override;

        WSCACHE_EXPORT void SetClassesToAlwaysCacheChildren(const bset<Utf8String>& classesToAlwaysCacheChildren) override;

        WSCACHE_EXPORT CachedResponseKey GetNavigationResponseKey(CacheTransactionCR txn, ObjectIdCR parentId) override;
        WSCACHE_EXPORT CachedResponseKey GetNavigationResponseKey(CacheTransactionCR txn, ECInstanceKeyCR parentKey) override;
        WSCACHE_EXPORT WSQueryPtr GetNavigationQuery(CacheTransactionCR txn, ObjectIdCR parentId, ISelectProviderPtr selectProvider = nullptr) override;

        WSCACHE_EXPORT AsyncTaskPtr<ObjectsResult> GetObject
            (
            ObjectIdCR objectId,
            DataOrigin origin,
            IDataSourceCache::JsonFormat format,
            ICancellationTokenPtr ct = nullptr
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<ObjectsResult> GetObjects
            (
            CachedResponseKeyCR responseKey,
            WSQueryCR query,
            DataOrigin origin,
            std::shared_ptr<const ISelectProvider> cacheReadOptions,
            ICancellationTokenPtr ct
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<KeysResult> GetObjectsKeys
            (
            CachedResponseKeyCR responseKey,
            WSQueryCR query,
            DataOrigin origin,
            ICancellationTokenPtr ct
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<ObjectsResult> GetNavigationChildren
            (
            ObjectIdCR parentId,
            DataOrigin origin,
            std::shared_ptr<const SelectProvider> readOptions,
            ICancellationTokenPtr ct = nullptr
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<KeysResult> GetNavigationChildrenKeys
            (
            ObjectIdCR parentId,
            DataOrigin origin,
            std::shared_ptr<const ISelectProvider> serverReadOptions,
            ICancellationTokenPtr ct = nullptr
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<FileResult> GetFile
            (
            ObjectIdCR fileId,
            DataOrigin origin,
            LabeledProgressCallback onProgress,
            ICancellationTokenPtr ct
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<BatchResult> CacheFiles
            (
            const bvector<ObjectId>& filesIds,
            bool skipCachedFiles,
            FileCache fileCacheLocation,
            LabeledProgressCallback onProgress,
            ICancellationTokenPtr ct
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<Result> DownloadAndCacheChildren
            (
            const bvector<ObjectId>& parentIds,
            ICancellationTokenPtr ct
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<BatchResult> SyncLocalChanges
            (
            SyncProgressCallback onProgress,
            ICancellationTokenPtr ct,
            SyncOptions options = SyncOptions()
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<BatchResult> SyncLocalChanges
            (
            const bset<ECInstanceKey>& objectsToSync,
            SyncProgressCallback onProgress,
            ICancellationTokenPtr ct,
            SyncOptions options = SyncOptions()
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<BatchResult> SyncCachedData
            (
            bvector<ECInstanceKey> initialInstances,
            bvector<IQueryProvider::Query> initialQueries,
            bvector<IQueryProviderPtr> queryProviders,
            ProgressCallback onProgress,
            ICancellationTokenPtr ct
            ) override;

        WSCACHE_EXPORT AsyncTaskPtr<BatchResult> CacheNavigation
            (
            const bvector<ObjectId>& persistenceNavigationTrees,
            const bvector<ObjectId>& temporaryNavigationTrees,
            std::shared_ptr<const ISelectProvider> temporaryNavigationTreesServerReadOptions,
            LabeledProgressCallback onProgress,
            ICancellationTokenPtr ct
            ) override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
