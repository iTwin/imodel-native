/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/ICachingDataSource.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// THIS IS NOT FINALIZED CODE, MAJOR API & CACHE STRUCTURE CHANGES ARE POSSIBLE WITHOUT MUCH CONVERSION SUPPORT //

#include <WebServices/Cache/WebServicesCache.h>

#include <Bentley/bset.h>
#include <WebServices/Cache/IQueryProvider.h>
#include <WebServices/Cache/Persistence/IDataSourceCache.h>
#include <WebServices/Cache/Transactions/CacheTransaction.h>
#include <MobileDgn/Utils/Threading/AsyncResult.h>
#include <MobileDgn/Utils/Threading/WorkerThread.h>
#include <WebServices/Client/WSRepositoryClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS
typedef std::shared_ptr<struct ICachingDataSource> ICachingDataSourcePtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ICachingDataSource
    {
    public:
        enum class Status
            {
            Success,
            Canceled,
            InternalCacheError,
            DataNotCached,
            NetworkErrorsOccured,
            FunctionalityNotSupported,
            DependencyNotSynced,
            ApplicationError        // Used by application logic to pass user-readable error messages
            };

        enum DataOrigin
            {
            CachedData = 1,         // Only Cached needed, no cached data will result in error
            RemoteData = 2,         // Download and cache, connection issues will result in error
            CachedOrRemoteData = 3, // Use cached if exists, else - download, connection issues will result in an error.
            RemoteOrCachedData = 4  // If online, download, else - get from cache
            };

        struct SelectProvider;

        struct Error;

        struct KeysData;
        struct ObjectsData;
        struct FileData;

        struct FailedObject;
        typedef bvector<FailedObject> FailedObjects;

        typedef Error& ErrorR;
        typedef const Error& ErrorCR;

        typedef FailedObject& FailedObjectR;
        typedef const FailedObject& FailedObjectCR;

        typedef FailedObjects& FailedObjectsR;
        typedef const FailedObjects& FailedObjectsCR;

        typedef std::function<void(double bytesTransfered, double bytesTotal)> ProgressCallback;
        typedef std::function<void(double bytesTransfered, double bytesTotal, Utf8StringCR taskLabel)> LabeledProgressCallback;

        //! synced - percentage (0.0 -> 1.0) of total sync done based on instances count
        //! currentLabel - label of instance being synced
        //! fileBytesTransfered - files bytes already synced to server. 0 if no files are being synced
        //! fileBytesTotal - total files bytes to sync. 0 if no files are being synced
        typedef std::function<void(double synced, Utf8StringCR currentLabel, double fileBytesTransfered, double fileBytesTotal)> SyncProgressCallback;

        typedef AsyncResult<void, Error>                    Result;
        typedef AsyncResult<KeysData, Error>                KeysResult;
        typedef AsyncResult<ObjectsData, Error>             ObjectsResult;
        typedef AsyncResult<FileData, Error>                FileResult;
        typedef AsyncResult<FailedObjects, Error>           BatchResult;

    public:
        virtual ~ICachingDataSource()
            {};

        virtual IWSRepositoryClientPtr GetClient() const = 0;
        virtual void SetClient(IWSRepositoryClientPtr client) = 0;

        virtual void CancelAllTasksAndWait() = 0;

        virtual AsyncTaskPtr<Result> UpdateSchemas(ICancellationTokenPtr cancellationToken) = 0;

        //! Get read/write transaction for local data cache storage. Must be called in cache access thread.
        //! READ/WRITE:
        //!     CacheTransaction will automatically Rollback() when destructor is called, that is especially helpful when dealing with errors.
        //!     Any sucessful write operation should call CacheTransaction::Commit() to save changes to disk.
        //! SAFE RESOURCE ACCESS:
        //!     Note that ECSchema, ECDb and other related resources are only safe to obtain and use within active transaction. Storing any
        //!     of such runtime resources outside of transaction is forbidden unless there is specific need for it (for example ECSqlStatement caching)
        //!     and if they are cleaned or reloaded when ECDb schema changes.
        //!     Schema changes are detected by registering listener with transaction.GetCache().RegisterCacheSchemaChangeListener(listener).
        //! CONCURRENCY:
        //!     Multiple processes can access same cache database. However, it should be expected that local data can be changed by other process.
        //!     Starting cache transaction ensures that schema change events (if any) are broadcasted and database file is locked for access.
        virtual CacheTransaction StartCacheTransaction() = 0;

        //! Retrieve access thread for accessing local resources directly.
        virtual WorkerThreadPtr GetCacheAccessThread() = 0;

        //! Load whole schemas that were synced from remote repository. Will be slow if schemas are large.
        //! If classes are not needed, consider using GetRepositorySchemaKeys
        virtual ECSchemaList GetRepositorySchemas(CacheTransactionCR txn) = 0;

        //! Get schema keys for schemas that were synced from remote repository.
        virtual bvector<SchemaKey> GetRepositorySchemaKeys(CacheTransactionCR txn) = 0;

        // Get latest cached server info. Info is refreshed on each server session.
        virtual WSInfo GetServerInfo(CacheTransactionCR txn) = 0;

        // Included class instances will always get their children cached when calling CacheNavigation
        virtual void SetClassesToAlwaysCacheChildren(const bset<Utf8String>& classesToAlwaysCacheChildren) = 0;

        // Get key for cached navigation query results.
        // NOTE: use GetNavigationChildren for getting children and use this only for specific cases
        virtual CachedResponseKey GetNavigationResponseKey(CacheTransactionCR txn, ObjectIdCR parentId) = 0;
        virtual CachedResponseKey GetNavigationResponseKey(CacheTransactionCR txn, ECInstanceKeyCR parentKey) = 0;
        //! Get NavigationQuery for any server version.
        //! Specify select provider to fill WSG 1.x navigation query properties parameter or WSG 2.0 select.
        virtual WSQueryPtr GetNavigationQuery(CacheTransactionCR txn, ObjectIdCR parentId, ISelectProviderPtr selectProvider = nullptr) = 0;

        virtual AsyncTaskPtr<ObjectsResult> GetObject
            (
            ObjectIdCR objectId,
            DataOrigin origin,
            IDataSourceCache::JsonFormat format,
            ICancellationTokenPtr cancellationToken = nullptr
            ) = 0;

        //! Do objects query to server or cache (depending on DataOrigin) and cache results with responseKey. Return flat list of instances cached.
        //! @param[in] responseKey - identifier for holding cached data
        //! @param[in] query - server query
        //! @param[in] origin - specify what data to try returning
        //! @param[in] cachedSelectProvider - (optional) provider for reading instances from cache. Note that query parameter specifies property selection from server.
        //! @param[in] cancellationToken - (optional)
        virtual AsyncTaskPtr<ObjectsResult> GetObjects
            (
            CachedResponseKeyCR responseKey,
            WSQueryCR query,
            DataOrigin origin,
            std::shared_ptr<const ISelectProvider> cachedSelectProvider,
            ICancellationTokenPtr cancellationToken
            ) = 0;

        //! Do objects query to server or cache (depending on DataOrigin) and cache results with responseKey. Return ECInstanceKeys of instances cached.
        //! @param[in] responseKey - identifier for holding cached data
        //! @param[in] query - server query
        //! @param[in] origin - specify what data to try returning
        //! @param[in] cancellationToken - (optional)
        virtual AsyncTaskPtr<KeysResult> GetObjectsKeys
            (
            CachedResponseKeyCR responseKey,
            WSQueryCR query,
            DataOrigin origin,
            ICancellationTokenPtr cancellationToken
            ) = 0;

        //! Get navigation instances.
        //! @param[in] parentId - parent ObjectId or empty for repository root. Each parent (inluding root) must be cached or linked to cache root first.
        //! @param[in] origin
        //! @param[in] readOptions
        //! @param[in] cancellationToken
        virtual AsyncTaskPtr<ObjectsResult> GetNavigationChildren
            (
            ObjectIdCR parentId,
            DataOrigin origin,
            std::shared_ptr<const SelectProvider> readOptions,
            ICancellationTokenPtr cancellationToken = nullptr
            ) = 0;

        virtual AsyncTaskPtr<KeysResult> GetNavigationChildrenKeys
            (
            ObjectIdCR parentId,
            DataOrigin origin,
            std::shared_ptr<const ISelectProvider> remoteSelectProvider,
            ICancellationTokenPtr cancellationToken = nullptr
            ) = 0;

        virtual AsyncTaskPtr<FileResult> GetFile
            (
            ObjectIdCR fileId,
            DataOrigin origin,
            LabeledProgressCallback onProgress,
            ICancellationTokenPtr cancellationToken
            ) = 0;

        virtual AsyncTaskPtr<BatchResult> CacheFiles
            (
            const bvector<ObjectId>& filesIds,
            bool skipCachedFiles,
            FileCache fileCacheLocation,
            LabeledProgressCallback onProgress,
            ICancellationTokenPtr cancellationToken
            ) = 0;

        virtual AsyncTaskPtr<Result> DownloadAndCacheChildren
            (
            const bvector<ObjectId>& parentIds,
            ICancellationTokenPtr cancellationToken
            ) = 0;

        //! Push all local changes to server with SyncStatus::Ready.
        //! @param onProgress - callback to track progress. Will report object labels that are being synced and progress value if any files are being uploaded.
        //! @param cancellationToken - for cancelling sync task
        virtual AsyncTaskPtr<BatchResult> SyncLocalChanges
            (
            SyncProgressCallback onProgress,
            ICancellationTokenPtr cancellationToken
            ) = 0;

        //! Push specific local changes to server
        //! @param instancesToSync - locally changed instances to sync. Changes with SyncStatus::NotReady are also synced if specified.
        //! @param onProgress - callback to track progress. Will report object labels that are being synced and progress value if any files are being uploaded.
        //! @param cancellationToken - for cancelling sync task
        virtual AsyncTaskPtr<BatchResult> SyncLocalChanges
            (
            const bset<ECInstanceKey>& instancesToSync,
            SyncProgressCallback onProgress,
            ICancellationTokenPtr cancellationToken
            ) = 0;

        //! Pull changes from server to update cached data.
        //! @param initialInstances - list of instances that should be used to start syncing
        //! @param initialQueries - list of queries that should be used to start syncing
        //! @param queryProviders - list of query providers to get queries for each instance that is being synced
        //! @param onProgress
        //! @param cancellationToken
        //! @return task with completion result
        virtual AsyncTaskPtr<BatchResult> SyncCachedData
            (
            bvector<ECInstanceKey> initialInstances,
            bvector<IQueryProvider::Query> initialQueries,
            bvector<IQueryProviderPtr> queryProviders,
            ProgressCallback onProgress,
            ICancellationTokenPtr cancellationToken
            ) = 0;

        //! DEPRECATED - Use SyncCachedData ().
        //! Pull latest changes from server.
        //! Notes:
        //!  Call SyncLocalChanges first to avoid local changes being overriden
        //!
        //! @param persistenceNavigationTrees - Update all items under specified objects including files
        //! @param temporaryNavigationTrees - Update only previously cached items under specified object and don't update any files
        //! @param temporaryNavigationTreesServerSelectProvider - Selected properties will be used to retrieve from server for "Update only" temporary navigation trees.
        //! @param onProgress
        //! @param cancellationToken
        virtual AsyncTaskPtr<BatchResult> CacheNavigation
            (
            const bvector<ObjectId>& persistenceNavigationTrees,
            const bvector<ObjectId>& temporaryNavigationTrees,
            std::shared_ptr<const ISelectProvider> temporaryNavigationTreesServerSelectProvider,
            LabeledProgressCallback onProgress,
            ICancellationTokenPtr cancellationToken
            ) = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
//! Specify different settings for reading from remote DS if more data is needed to be cached.
struct ICachingDataSource::SelectProvider
    {
    private:
        ISelectProviderPtr  m_remoteSelectProvider;
        ISelectProviderPtr  m_cacheSelectProvider;

    public:
        WSCACHE_EXPORT SelectProvider();
        WSCACHE_EXPORT SelectProvider(ISelectProviderPtr selectProvider);

        WSCACHE_EXPORT void SetForRemote(ISelectProviderPtr selectProvider);
        WSCACHE_EXPORT void SetForCache(ISelectProviderPtr selectProvider);

        //! Folowing methods always return non null value
        WSCACHE_EXPORT std::shared_ptr<const ISelectProvider> GetForRemote() const;
        WSCACHE_EXPORT std::shared_ptr<const ISelectProvider> GetForCache() const;
        WSCACHE_EXPORT ISelectProviderPtr GetForRemote();
        WSCACHE_EXPORT ISelectProviderPtr GetForCache();
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ICachingDataSource::Error : public AsyncError
    {
    private:
        ICachingDataSource::Status  m_status;
        WSError                     m_wsError;

    private:
        static ICachingDataSource::Status ConvertCacheStatus(CacheStatus status);

    public:
        WSCACHE_EXPORT Error();
        WSCACHE_EXPORT Error(ICachingDataSource::Status status);
        WSCACHE_EXPORT Error(CacheStatus status);
        WSCACHE_EXPORT Error(WSErrorCR error);
        //! Constructs error with status InternalCacheError message and desription from error
        WSCACHE_EXPORT Error(AsyncErrorCR error);
        //! Constructs error with status and message with description from error
        WSCACHE_EXPORT Error(ICachingDataSource::Status status, AsyncErrorCR error);
        //! Constructs error with status InternalCacheError and specified message.
        WSCACHE_EXPORT Error(Utf8StringCR message);
        //! Constructs error with supplied status or status Canceled if cancellation token is non null and already canceled.
        //! Used when operation might have been canceled but status does not indicated that.
        WSCACHE_EXPORT Error(ICachingDataSource::Status status, ICancellationTokenPtr cancellationToken);
        WSCACHE_EXPORT ~Error();

        WSCACHE_EXPORT ICachingDataSource::Status GetStatus() const;
        WSCACHE_EXPORT WSErrorCR GetWSError() const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ICachingDataSource::ObjectsData
    {
    private:
        std::shared_ptr<Json::Value> m_data;
        DataOrigin m_origin;

    public:
        WSCACHE_EXPORT ObjectsData();
        WSCACHE_EXPORT ObjectsData(std::shared_ptr<Json::Value> data, DataOrigin origin);

        WSCACHE_EXPORT JsonValueCR GetJson() const;
        WSCACHE_EXPORT JsonValueR GetJson();

        //! Get origin of data - CachedData or RemoteData. Other values are not returned
        WSCACHE_EXPORT DataOrigin GetOrigin() const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ICachingDataSource::KeysData
    {
    private:
        std::shared_ptr<ECInstanceKeyMultiMap> m_data;
        DataOrigin m_origin;

    public:
        WSCACHE_EXPORT KeysData();
        WSCACHE_EXPORT KeysData(std::shared_ptr<ECInstanceKeyMultiMap> data, DataOrigin origin);

        WSCACHE_EXPORT std::shared_ptr<ECInstanceKeyMultiMap> GetKeysPtr();
        WSCACHE_EXPORT const ECInstanceKeyMultiMap& GetKeys() const;
        WSCACHE_EXPORT ECInstanceKeyMultiMap& GetKeys();
        WSCACHE_EXPORT DataOrigin GetOrigin() const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ICachingDataSource::FileData
    {
    private:
        BeFileName m_filePath;
        DataOrigin m_origin;

    public:
        WSCACHE_EXPORT FileData();
        WSCACHE_EXPORT FileData(BeFileNameCR filePath, DataOrigin origin);

        WSCACHE_EXPORT BeFileNameCR GetFilePath() const;
        WSCACHE_EXPORT DataOrigin GetOrigin() const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ICachingDataSource::FailedObject
    {
    private:
        ObjectId    m_objectId;
        Utf8String  m_objectLabel;
        Error       m_error;

    public:
        WSCACHE_EXPORT FailedObject();
        WSCACHE_EXPORT FailedObject(ObjectIdCR objectId, Utf8StringCR objectLabel, ErrorCR error);

        WSCACHE_EXPORT ObjectIdCR   GetObjectId() const;
        WSCACHE_EXPORT Utf8StringCR GetObjectLabel() const;
        WSCACHE_EXPORT ErrorCR      GetError() const;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
