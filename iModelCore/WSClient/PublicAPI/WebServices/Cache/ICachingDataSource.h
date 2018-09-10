/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/ICachingDataSource.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// THIS IS NOT FINALIZED CODE, MAJOR API & CACHE STRUCTURE CHANGES ARE POSSIBLE WITHOUT MUCH CONVERSION SUPPORT //

#include <WebServices/Cache/WebServicesCache.h>

#include <Bentley/bset.h>
#include <MobileDgn/Utils/Threading/AsyncResult.h>
#include <MobileDgn/Utils/Threading/WorkerThread.h>
#include <WebServices/Cache/Persistence/IDataSourceCache.h>
#include <WebServices/Cache/Transactions/CacheTransaction.h>
#include <WebServices/Cache/IQueryProvider.h>
#include <WebServices/Cache/SyncOptions.h>
#include <WebServices/Client/WSRepositoryClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS
typedef std::shared_ptr<struct ICachingDataSource> ICachingDataSourcePtr;
typedef std::shared_ptr<const Utf8String> Utf8StringCPtr;
typedef std::shared_ptr<struct SyncNotifier> SyncNotifierPtr;

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
            ApplicationError, //! Used by application logic to pass user-readable error messages
            SchemaError,
            FileLocked,
            FileCancelled
            };

        enum DataOrigin
            {
            CachedData = 1,         // Only return if data is cached, do not query server 
            RemoteData = 2,         // Always query server to update the cache
            CachedOrRemoteData = 3, // Return data if it's cached or query server if it's not
            RemoteOrCachedData = 4  // Query server first to update the cache or return cached data if query failed 
            };

        enum class SyncStatus
            {
            Synced,          // Synchronized succesfully.
            NotModified,     // Synchronized succesfully data not changed on server.
            NotSynced,       // There was no attempt to synchronize data from server
            SyncError        // There was an error during synchronization with server.
            };

        struct RetrieveOptions;

        struct SelectProvider;

        struct Progress;

        struct Error;

        struct KeysData;
        struct ObjectsData;
        struct FileData;

        struct FailedObject;
        struct FailedObjects;

        typedef Progress& ProgressR;
        typedef const Progress& ProgressCR;

        typedef Error& ErrorR;
        typedef const Error& ErrorCR;

        typedef FailedObject& FailedObjectR;
        typedef const FailedObject& FailedObjectCR;

        typedef FailedObjects& FailedObjectsR;
        typedef const FailedObjects& FailedObjectsCR;             

        typedef std::function<void(ProgressCR progress)> ProgressCallback;

        struct ProgressHandler
            {
            public:
                ProgressCallback progressCallback;
                std::function<bool(const IQueryProvider::Query& query)> shouldReportQueryProgress;

            public:
                ProgressHandler() : progressCallback([] (ICachingDataSource::ProgressCR) {}), shouldReportQueryProgress([] (const IQueryProvider::Query& query) { return true; }) {};
                ProgressHandler(std::nullptr_t) : progressCallback([] (ICachingDataSource::ProgressCR) {}), shouldReportQueryProgress([] (const IQueryProvider::Query& query) { return true; }) {};
                ProgressHandler(ProgressCallback callback) : progressCallback(callback), shouldReportQueryProgress([] (const IQueryProvider::Query& query) { return true; }) {};
            };

        typedef std::shared_ptr<ProgressHandler> ProgressHandlerPtr;

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

        virtual AsyncTaskPtr<void> CancelAllTasks() = 0;

        //! Enable experimental skip token support while doing queries and caching data. Disabled by default.
        //! If server-side supported, will do queries in pages.
        //! WARNING: unfinished and non-tested functionality:
        //!     * Data changes, page size changes, additions or removal of instances while pulling might corrupt/distort data or break caching. 
        //!     * Connection loss while pulling data will leave response in state "NotCached" with partially new and old data in cache.
        virtual void EnableSkipTokens(bool enable) = 0;

        //! Check server for schema changes and update if needed.
        virtual AsyncTaskPtr<Result> UpdateSchemas(ICancellationTokenPtr ct = nullptr) = 0;

        //! Get read/write transaction for local data cache storage. Must be called in cache access thread.
        //! CAUTION:
        //!     Do not mix transactions by passing them to different APIs that might cache references when using multiple ICachingDataSource objects.
        //!     TODO: possible redesign to use transaction object for getting cache, instead of it deliverting cache to protect against mixups.
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

        //! DEPRECATED, use GetServerInfo()
        //! Get latest cached server info. WSInfo is refreshed for each new session.
        virtual WSInfo GetServerInfo(CacheTransactionCR txn) = 0;

        //! Get latest cached server info. WSInfo is refreshed for each new session.
        virtual WSInfo GetServerInfo() = 0;

        //! Get latest cached reposiroty info. WSRepository is refreshed for each new session.
        virtual WSRepository GetRepositoryInfo() = 0;

        //! Included class instances will always get their children cached when calling CacheNavigation
        virtual void SetClassesToAlwaysCacheChildren(const bset<Utf8String>& classesToAlwaysCacheChildren) = 0;

        //! Get key for cached navigation query results.
        //! NOTE: use GetNavigationChildren for getting children and use this only for specific cases
        virtual CachedResponseKey GetNavigationResponseKey(CacheTransactionCR txn, ObjectIdCR parentId) = 0;
        virtual CachedResponseKey GetNavigationResponseKey(CacheTransactionCR txn, ECInstanceKeyCR parentKey) = 0;
        //! Get NavigationQuery for any server version.
        //! Specify select provider to fill WSG 1.x navigation query properties parameter or WSG 2.0 select.
        virtual WSQueryPtr GetNavigationQuery(CacheTransactionCR txn, ObjectIdCR parentId, ISelectProviderPtr selectProvider = nullptr) = 0;

        virtual AsyncTaskPtr<ObjectsResult> GetObject
            (
            ObjectIdCR objectId,
            RetrieveOptions retrieveOptions,
            IDataSourceCache::JsonFormat format = IDataSourceCache::JsonFormat::Raw,
            ICancellationTokenPtr ct = nullptr
            ) = 0;

        //! Do objects query to server or cache (depending on DataOrigin) and cache results with responseKey. Return flat list of instances cached.
        //! @param[in] responseKey - identifier for holding cached data
        //! @param[in] query - server query
        //! @param[in] origin - specify what data to try returning
        //! @param[in] cachedSelectProvider - (optional) provider for reading instances from cache. Note that query parameter specifies property selection from server.
        //! @param[in] ct - (optional)
        virtual AsyncTaskPtr<ObjectsResult> GetObjects
            (
            CachedResponseKeyCR responseKey,
            WSQueryCR query,
            DataOrigin origin,
            std::shared_ptr<const ISelectProvider> cachedSelectProvider = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) = 0;

        //! Do objects query to server or cache (depending on DataOrigin) and cache results with responseKey. Return ECInstanceKeys of instances cached.
        //! @param[in] responseKey - identifier for holding cached data
        //! @param[in] query - server query
        //! @param[in] RetrieveOptions - specify what data to try returning
        //! @param[in] ct - (optional)
        virtual AsyncTaskPtr<KeysResult> GetObjectsKeys
            (
            CachedResponseKeyCR responseKey,
            WSQueryCR query,
            RetrieveOptions retrieveOptions,
            ICancellationTokenPtr ct = nullptr
            ) = 0;

        //! Get navigation instances.
        //! @param[in] parentId - parent ObjectId or empty for repository root. Each parent (inluding root) must be cached or linked to cache root first.
        //! @param[in] origin
        //! @param[in] readOptions
        //! @param[in] ct
        virtual AsyncTaskPtr<ObjectsResult> GetNavigationChildren
            (
            ObjectIdCR parentId,
            DataOrigin origin,
            std::shared_ptr<const SelectProvider> readOptions = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) = 0;

        virtual AsyncTaskPtr<KeysResult> GetNavigationChildrenKeys
            (
            ObjectIdCR parentId,
            DataOrigin origin,
            std::shared_ptr<const ISelectProvider> remoteSelectProvider = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) = 0;

        virtual AsyncTaskPtr<FileResult> GetFile
            (
            ObjectIdCR fileId,
            DataOrigin origin,
            ProgressCallback onProgress = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) = 0;

        virtual AsyncTaskPtr<BatchResult> CacheFiles
            (
            const bvector<ObjectId>& filesIds,
            bool skipCachedFiles = false,
            FileCache fileCacheLocation = FileCache::Auto,
            ProgressCallback onProgress = nullptr,
            ICancellationTokenPtr ct = nullptr
            ) = 0;

        virtual AsyncTaskPtr<Result> DownloadAndCacheChildren
            (
            const bvector<ObjectId>& parentIds,
            ICancellationTokenPtr ct = nullptr
            ) = 0;

        //! Push all local changes to server with SyncStatus::Ready.
        //! Note that created instances could change ECInstanceKey if class changes after sync.
        //! @param onProgress - callback to track progress. Will report object labels that are being synced and progress value if any files are being uploaded.
        //! @param ct - cancelling sync task
        //! @param options - additonal configuration for sync
        //! @return fatal error (like server error or connection is lost) - sync is stopped and error is returned. 
        //! If server returns error specific to instances being synced (forbidden, conflict, etc) – it is put into return value list and returned as “FailedObject” 
        //! after everything is synced.
        virtual AsyncTaskPtr<BatchResult> SyncLocalChanges
            (
            ProgressCallback onProgress = nullptr,
            ICancellationTokenPtr ct = nullptr,
            SyncOptions options = SyncOptions()
            ) = 0;

        //! Push specific local changes to server. See SyncLocalChages() for more.
        //! @param instancesToSync - locally changed instances to sync. Changes with SyncStatus::NotReady are also synced if specified.
        //! @param onProgress - callback to track progress. Will report object labels that are being synced and progress value if any files are being uploaded.
        //! @param ct - cancelling sync task
        //! @param options - additonal configuration for sync
        //! @return fatal error (like server error or connection is lost) - sync is stopped and error is returned. 
        //! If server returns error specific to instances being synced (forbidden, conflict, etc) – it is put into return value list and returned as “FailedObject” 
        //! after everything is synced.
        virtual AsyncTaskPtr<BatchResult> SyncLocalChanges
            (
            const bset<ECInstanceKey>& instancesToSync,
            ProgressCallback onProgress = nullptr,
            ICancellationTokenPtr ct = nullptr,
            SyncOptions options = SyncOptions()
            ) = 0;

        //! Pull changes from server to update cached data.
        //! @param initialInstances - list of instances that should be used to start syncing
        //! @param initialQueries - list of queries that should be used to start syncing
        //! @param queryProviders - list of query providers to get queries and file download option for each instance that is being synced
        //! @param onProgress - progress callback
        //! Synced parameter - reports instances + queries synced. File sync represented by bytes synced.
        //! Note that synced value will fluctuate if query providers return more queries to sync.
        //! Label parameter can be empty or show file label being synced. 
        //! @param ct - cancelling sync task
        //! @return fatal error (like server error or connection is lost) - sync is stopped and error is returned. 
        //! If server returns error specific to instances being synced (forbidden, conflict, etc) – it is put into return value list and returned as “FailedObject” 
        //! after everything is synced.
        virtual AsyncTaskPtr<BatchResult> SyncCachedData
            (
            bvector<ECInstanceKey> initialInstances,
            bvector<IQueryProvider::Query> initialQueries,
            bvector<IQueryProviderPtr> queryProviders,
            ProgressHandler progressHandler = ProgressHandler(),
            ICancellationTokenPtr ct = nullptr
            ) = 0;

        //! DEPRECATED - Use SyncCachedData ().
        //! Pull latest changes from server.
        //! Notes:
        //!  Call SyncLocalChanges first to avoid local changes being overriden
        //!
        //! @param persistenceNavigationTrees - Update all items under specified objects including files
        //! @param temporaryNavigationTrees - Update only previously cached items under specified object and don't update any files
        //! @param temporaryNavigationTreesServerSelectProvider - Selected properties will be used to retrieve from server for "Update only" temporary navigation trees.
        //! @param ct - cancelling sync task
        //! @return fatal error (like server error or connection is lost) - sync is stopped and error is returned. 
        //! If server returns error specific to instances being synced (forbidden, conflict, etc) – it is put into return value list and returned as “FailedObject” 
        //! after everything is synced.
        virtual AsyncTaskPtr<BatchResult> CacheNavigation
            (
            const bvector<ObjectId>& persistenceNavigationTrees,
            const bvector<ObjectId>& temporaryNavigationTrees,
            std::shared_ptr<const ISelectProvider> temporaryNavigationTreesServerSelectProvider,
            ProgressCallback onProgress = nullptr,
            ICancellationTokenPtr ct = nullptr
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
        static Utf8String GetLocalizedMessage(ICachingDataSource::Status status);
        void HandleStatusCanceled(ICancellationTokenPtr ct);

    public:
        //! Constructs error with Status::Success.
        WSCACHE_EXPORT Error();
        //! Constructs error with specified status and localized message.
        WSCACHE_EXPORT Error(ICachingDataSource::Status status);
        //! Constructs error with matching status and localized message.
        WSCACHE_EXPORT Error(CacheStatus status);
        //! Constructs error with matching status and specified error message or localized message if empty.
        WSCACHE_EXPORT Error(CacheStatus status, AsyncError error);
        //! Constructs error with matching status and or status Canceled if cancellation token is non null and already canceled.
        //! Used when operation might have been canceled but status does not indicate that.
        WSCACHE_EXPORT Error(CacheStatus status, ICancellationTokenPtr ct);
        //! Constructs error with specificed server error with status NetworkErrorsOccured or Canceled.
        WSCACHE_EXPORT Error(WSError error);
        //! Constructs error with status InternalCacheError and message with desription from specified error.
        WSCACHE_EXPORT Error(AsyncError error);
        //! Constructs error with status and message with desription from specified error.
        //! Can be used with Status::ApplicationError to pass localized error message to user.
        WSCACHE_EXPORT Error(ICachingDataSource::Status status, AsyncError error);
        //! Constructs error with status InternalCacheError and specified message.
        WSCACHE_EXPORT Error(Utf8String message);
        //! Constructs error with supplied status or status Canceled if cancellation token is non null and already canceled.
        //! Used when operation might have been canceled but status does not indicate that.
        WSCACHE_EXPORT Error(ICachingDataSource::Status status, ICancellationTokenPtr ct);
        WSCACHE_EXPORT ~Error();

        WSCACHE_EXPORT ICachingDataSource::Status GetStatus() const;
        WSCACHE_EXPORT WSErrorCR GetWSError() const;
    };
    
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                  
+---------------+---------------+---------------+---------------+---------------+------*/
struct ICachingDataSource::RetrieveOptions
    {
    private:
        DataOrigin m_origin = DataOrigin::CachedData;
        SyncNotifierPtr m_syncNotifier;

    public:
        WSCACHE_EXPORT RetrieveOptions() {}

        //! @param[in] DataOrigin origin - specify what data to try returning
        //! @param[in] backgroundSyncNotifier - (optional) if not null and there was no attempt to pull data from server - repeats the call with DataOrigin::RemoteData in the background
        WSCACHE_EXPORT RetrieveOptions(DataOrigin origin, SyncNotifierPtr syncNotifier = nullptr) : 
            m_origin(origin), m_syncNotifier(syncNotifier) {}

        WSCACHE_EXPORT DataOrigin GetOrigin() const { return m_origin; }
        WSCACHE_EXPORT SyncNotifierPtr GetSyncNotifier() const { return m_syncNotifier; }
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
        SyncStatus m_syncStatus;

    public:
        WSCACHE_EXPORT KeysData();
        WSCACHE_EXPORT KeysData(std::shared_ptr<ECInstanceKeyMultiMap> data, DataOrigin origin, SyncStatus syncStatus);

        WSCACHE_EXPORT std::shared_ptr<ECInstanceKeyMultiMap> GetKeysPtr();
        WSCACHE_EXPORT const ECInstanceKeyMultiMap& GetKeys() const;
        WSCACHE_EXPORT ECInstanceKeyMultiMap& GetKeys();
        WSCACHE_EXPORT DataOrigin GetOrigin() const;
        WSCACHE_EXPORT SyncStatus GetSyncStatus() const;
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

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ICachingDataSource::FailedObjects : public bvector<FailedObject>
    {
    public:
        FailedObjects& AppendFailures(FailedObjectsCR failedObjects)
            {
            if (!failedObjects.empty())
                this->insert(this->end(), failedObjects.begin(), failedObjects.end());
            return *this;
            }
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                Vilius.Kazlauskas    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct ICachingDataSource::Progress
    {
    public:
        struct State
            {
            double current = 0;
            double total = 0;

            State() {};
            State(double current, double total) : current(current), total(total) {};
            bool operator==(const State& other) const { return current == other.current && total == other.total; };
            bool operator!=(const State& other) const { return !this->operator==(other); }
            bool IsFinished() const { return total != 0 && current == total; };
            bool IsUndefined() const { return current == 0 && total == 0; };
            };
        
        typedef State& StateR;
        typedef const State& StateCR;
        
    private:        
        double m_synced = 0;
        State m_bytes;
        State m_instances;
        Utf8StringCPtr m_label;
        State m_currentFileBytes;
        ECInstanceKey m_currentFileKey;
    
    public:
        Progress() {};
            
        // Create prgress for file transfers
        Progress(State bytes, Utf8StringCPtr label = nullptr, double synced = 0, ECInstanceKeyCR currentFileECInstanceKey = ECInstanceKey(), State currentBytes = {0, 0}) :
            m_bytes(bytes), m_label(label), m_synced(synced), m_currentFileKey(currentFileECInstanceKey), m_currentFileBytes(currentBytes)
            {};
        // Create progress for sync and file transfers
        Progress(double synced, State instances = State(), State bytes = State(), Utf8StringCPtr label = nullptr) :
            m_synced(synced), m_instances(instances), m_bytes(bytes), m_label(label)
            {};

        //! GetBytes().current - file bytes already synced
        //! GetBytes().total - total file bytes to sync. 0 if no files are being synced.
        StateCR GetBytes() const { return m_bytes; };
        //! GetInstances().current - already synced instances
        //! GetInstances().toal - total instances to sync
        StateCR GetInstances() const { return m_instances; };
        //! Get percentage (0.0 -> 1.0) of total sync done based on instances and queries count
        double GetSynced() const { return m_synced; };
        //! Get ECInstanceKey of current file being synchronized/uploaded
        ECInstanceKeyCR GetCurrentFileKey() const { return m_currentFileKey; };
        //! Get progress state of single (current) file which is in progress of syncing/uploading
        StateCR GetCurrentFileBytes() const { return m_currentFileBytes; };
        
        //! Get label of instance being synced
        WSCACHE_EXPORT Utf8StringCR GetLabel() const;
        //! Get label of instance being synced
        Utf8StringCPtr GetLabelPtr() const { return m_label; };

        bool operator==(const Progress& other) const 
            {
            return
                m_bytes == other.m_bytes &&
                m_instances == other.m_instances &&
                m_synced == other.m_synced &&
                GetLabel() == other.GetLabel() &&
                m_currentFileKey == other.m_currentFileKey &&
                m_currentFileBytes == other.m_currentFileBytes;
            };

        bool operator!=(const Progress& other) const
            {
            return !operator==(other);
            };
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
