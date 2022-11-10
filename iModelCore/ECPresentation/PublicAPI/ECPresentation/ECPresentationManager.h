/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/DataSource.h>
#include <ECPresentation/NavNode.h>
#include <ECPresentation/Content.h>
#include <ECPresentation/KeySet.h>
#include <ECPresentation/Connection.h>
#include <ECPresentation/IECPresentationSerializer.h>
#include <ECPresentation/LabelDefinition.h>
#include <ECPresentation/IRulesPreprocessor.h>
#include <ECPresentation/RuleSetLocater.h>
#include <ECPresentation/UserSettings.h>
#include <ECPresentation/RulesetVariables.h>
#include <ECPresentation/ECInstanceChangeEvents.h>
#include <ECPresentation/Rules/PresentationRules.h>
#include <ECPresentation/IUiStateProvider.h>
#include <ECPresentation/Update.h>
#include <ECPresentation/ECPresentationManagerRequestParams.h>
#include <ECPresentation/ECPresentationErrors.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

#if defined(BENTLEYCONFIG_OS_APPLE_IOS) || defined(BENTLEYCONFIG_OS_WINRT) || defined(BENTLEYCONFIG_OS_ANDROID)
    // 50 MB on mobile platforms
    #define DEFAULT_DISK_CACHE_SIZE_LIMIT   50 * 1024 * 1024
#else
    // 1 GB on desktop
    #define DEFAULT_DISK_CACHE_SIZE_LIMIT   1024 * 1024 * 1024
#endif

#define DEFAULT_BACKGROUND_THREADS_COUNT    1
#define DEFAULT_REQUEST_PRIORITY            1000

struct ECPresentationTasksManager;
struct IECPresentationTask;

// WIP
struct ContentCache;

//! A container of refcounted NavNode objects.
typedef DataContainer<NavNodeCPtr> NavNodesContainer;

//=======================================================================================
// @bsiclass
//=======================================================================================
template<typename TBase>
struct WithAsyncTaskParams : TBase
{
private:
    ECDbCR m_db;
    int m_priority;
    std::function<void()> m_taskStartCallback;

private:
    WithAsyncTaskParams(TBase const& source, ECDbCR db) : TBase(source), m_db(db), m_priority(DEFAULT_REQUEST_PRIORITY) {}
    WithAsyncTaskParams(TBase&& source, ECDbCR db) : TBase(std::move(source)), m_db(db), m_priority(DEFAULT_REQUEST_PRIORITY) {}

public:
    template<typename ...TBaseArgs>
    static WithAsyncTaskParams<TBase> Create(ECDbCR db, TBaseArgs&&... rest)
        {
        return WithAsyncTaskParams(TBase(std::forward<TBaseArgs>(rest)...), db);
        }

    template<typename TDerivedFromBase>
    static WithAsyncTaskParams<TBase> Create(WithAsyncTaskParams<TDerivedFromBase> const& source)
        {
        WithAsyncTaskParams asyncParams(TBase(source), source.GetECDb());
        asyncParams.SetRequestPriority(source.GetRequestPriority());
        asyncParams.SetTaskStartCallback(source.GetTaskStartCallback());
        return asyncParams;
        }

    template<typename TAsyncParams>
    static WithAsyncTaskParams<TBase> Create(TBase&& baseParams, TAsyncParams const& asyncParamsSource)
        {
        WithAsyncTaskParams<TBase> asyncParams(baseParams, asyncParamsSource.GetECDb());
        asyncParams.SetRequestPriority(asyncParamsSource.GetRequestPriority());
        asyncParams.SetTaskStartCallback(asyncParamsSource.GetTaskStartCallback());
        return asyncParams;
        }

    ECDbCR GetECDb() const {return m_db;}
    int GetRequestPriority() const {return m_priority;}
    void SetRequestPriority(int value) {m_priority = value;}
    std::function<void()> const& GetTaskStartCallback() const {return m_taskStartCallback;}
    void SetTaskStartCallback(std::function<void()> cb) {m_taskStartCallback = cb;}
};
typedef WithAsyncTaskParams<HierarchyRequestParams> AsyncHierarchyRequestParams;
typedef WithAsyncTaskParams<NodeByKeyRequestParams> AsyncNodeByKeyRequestParams;
typedef WithAsyncTaskParams<NodeParentRequestParams> AsyncNodeParentRequestParams;
typedef WithAsyncTaskParams<NodePathFromInstanceKeyPathRequestParams> AsyncNodePathFromInstanceKeyPathRequestParams;
typedef WithAsyncTaskParams<NodePathsFromInstanceKeyPathsRequestParams> AsyncNodePathsFromInstanceKeyPathsRequestParams;
typedef WithAsyncTaskParams<NodePathsFromFilterTextRequestParams> AsyncNodePathsFromFilterTextRequestParams;
typedef WithAsyncTaskParams<HierarchyCompareRequestParams> AsyncHierarchyCompareRequestParams;
typedef WithAsyncTaskParams<ContentClassesRequestParams> AsyncContentClassesRequestParams;
typedef WithAsyncTaskParams<ContentDescriptorRequestParams> AsyncContentDescriptorRequestParams;
typedef WithAsyncTaskParams<ContentRequestParams> AsyncContentRequestParams;
typedef WithAsyncTaskParams<DistinctValuesRequestParams> AsyncDistinctValuesRequestParams;
typedef WithAsyncTaskParams<ECInstanceDisplayLabelRequestParams> AsyncECInstanceDisplayLabelRequestParams;
typedef WithAsyncTaskParams<KeySetDisplayLabelRequestParams> AsyncKeySetDisplayLabelRequestParams;

//=======================================================================================
// @bsiclass
//=======================================================================================
template<typename TResult>
struct ECPresentationResponse
{
    typedef TResult ResultType;

private:
#ifdef TODO_NONCOPYABLE_RESPONSE
    // TODO: This class should eventually become uncopyable by making these unique_ptr instead of shared_ptr, but
    // at this moment it's not possible due to our use of folly::SharedPromise.
    std::unique_ptr<TResult> m_result;
#else
    std::shared_ptr<TResult> m_result;
#endif

private:
    ECPresentationResponse(std::shared_ptr<TResult> result) : m_result(result) {}

public:
    ECPresentationResponse(TResult&& result) : ECPresentationResponse(std::make_unique<TResult>(std::move(result))) {}
    TResult const& GetResult() const {return *m_result;}
    TResult const& operator*() const {return GetResult();}

    // TODO: add telemetry data, diagnostics, etc.
};
typedef ECPresentationResponse<NavNodesContainer> NodesResponse;
typedef ECPresentationResponse<size_t> NodesCountResponse;
typedef ECPresentationResponse<NavNodeCPtr> NodeResponse;
typedef ECPresentationResponse<NodesPathElement> NodePathElementResponse;
typedef ECPresentationResponse<bvector<NodesPathElement>> NodePathsResponse;
typedef ECPresentationResponse<HierarchyComparePositionPtr> HierarchiesCompareResponse;
typedef ECPresentationResponse<bvector<SelectClassInfo>> ContentClassesResponse;
typedef ECPresentationResponse<ContentDescriptorCPtr> ContentDescriptorResponse;
typedef ECPresentationResponse<ContentCPtr> ContentResponse;
typedef ECPresentationResponse<size_t> ContentSetSizeResponse;
typedef ECPresentationResponse<LabelDefinitionCPtr> DisplayLabelResponse;
typedef ECPresentationResponse<PagedDataContainer<DisplayValueGroupCPtr>> DistinctValuesResponse;

//=======================================================================================
//! Rules-driven presentation manager implementation which uses presentation rules for
//! determining the hierarchies and content.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass
//=======================================================================================
struct ECPresentationManager : public NonCopyableClass
{
    struct ImplParams;
    struct Impl;
    struct ConnectionManagerWrapper;
    struct RulesetLocaterManagerWrapper;
    struct UserSettingsManagerWrapper;
    struct ECInstanceChangeEventSourceWrapper;

    //===================================================================================
    //! An object that stores paths used by ECPresentationManager
    // @bsiclass
    //===================================================================================
    struct Paths
    {
    private:
        BeFileName m_assetsDirectory;
        BeFileName m_tempDirectory;
    public:
        Paths(BeFileName assetsDirectory, BeFileName tempDirectory)
            : m_assetsDirectory(assetsDirectory), m_tempDirectory(tempDirectory)
            {}
        BeFileNameCR GetAssetsDirectory() const {return m_assetsDirectory;}
        BeFileNameCR GetTemporaryDirectory() const {return m_tempDirectory;}
    };

    //===================================================================================
    //! Parameters for constructing ECPresentationManager
    // @bsiclass
    //===================================================================================
    struct Params
    {
        //===================================================================================
        // @bsiclass
        //===================================================================================
        struct CachingParams
        {
        enum class Mode
            {
            Memory,
            Disk,
            Hybrid, // uses memory cache while creating hierarchy and then persists it in disk cache
            };

        private:
            Mode m_mode;
            uint64_t m_diskCacheFileSizeLimit;
            Nullable<uint64_t> m_diskCacheMemoryCacheSize;
            BeFileName m_cacheDirectory;
        public:
            CachingParams() : m_mode(Mode::Disk), m_diskCacheFileSizeLimit(DEFAULT_DISK_CACHE_SIZE_LIMIT), m_cacheDirectory(L"") {}
            CachingParams(uint64_t diskCacheFileSizeLimit): m_mode(Mode::Disk), m_diskCacheFileSizeLimit(diskCacheFileSizeLimit), m_cacheDirectory(L"") {}
            CachingParams(Utf8StringCR cacheDirectory) : m_mode(Mode::Disk), m_diskCacheFileSizeLimit(DEFAULT_DISK_CACHE_SIZE_LIMIT), m_cacheDirectory(cacheDirectory) {}
            //! Is hierarchy caching on disk disabled
            Mode GetCacheMode() const {return m_mode;}
            void SetCacheMode(Mode value) {m_mode = value;}
            //! Maximum allowed size (in bytes) of cache that's stored on disk by presentation manager.
            //! 0 means infinite size. Defaults to DEFAULT_DISK_CACHE_SIZE_LIMIT.
            uint64_t GetDiskCacheFileSizeLimit() const {return m_diskCacheFileSizeLimit;}
            void SetDiskCacheFileSizeLimit(uint64_t value) {m_diskCacheFileSizeLimit = value;}
            //! Path to the directory for storing hierarchy caches.
            //! Empty path means cache is stored alongside db used to create hierarchy
            BeFileNameCR GetCacheDirectoryPath() const {return m_cacheDirectory;}
            void SetCacheDirectoryPath(BeFileNameCR value) {m_cacheDirectory = value;}
            //! Maximum allowed size of the memory cache used by the disk-based hierarchy cache.
            Nullable<uint64_t> const& GetDiskCacheMemoryCacheSize() const {return m_diskCacheMemoryCacheSize;}
            void SetDiskCacheMemoryCacheSize(Nullable<uint64_t> value) {m_diskCacheMemoryCacheSize = value;}
        };

        //===================================================================================
        // @bsiclass
        //===================================================================================
        struct ContentCachingParams
        {
        private:
            size_t m_privateCacheSize;
        public:
            ContentCachingParams() : m_privateCacheSize(100) {}
            size_t GetPrivateCacheSize() const {return m_privateCacheSize;}
            void SetPrivateCacheSize(size_t size) {m_privateCacheSize = size;}
        };

        //===================================================================================
        // @bsiclass
        //===================================================================================
        struct MultiThreadingParams
        {
        private:
            bmap<int, unsigned> m_backgroundThreadAllocations; // max task priority => threads count
        public:
            MultiThreadingParams()
                {
                // allocate 1 thread for tasks of any priority
                m_backgroundThreadAllocations.Insert(INT32_MAX, DEFAULT_BACKGROUND_THREADS_COUNT);
                }
            MultiThreadingParams(bmap<int, unsigned> backgroundThreadAllocations)
                : m_backgroundThreadAllocations(backgroundThreadAllocations)
                {}
            MultiThreadingParams(bpair<int, unsigned> backgroundThreadAllocations)
                {
                m_backgroundThreadAllocations.insert(backgroundThreadAllocations);
                }
            bmap<int, unsigned> const& GetBackgroundThreadAllocations() const {return m_backgroundThreadAllocations;}
        };

    private:
        std::shared_ptr<IConnectionManager> m_connections;
        Paths m_paths;
        CachingParams m_cachingParams;
        MultiThreadingParams m_multiThreadingParams;
        ContentCachingParams m_contentCachingParams;
        IJsonLocalState* m_localState;
        IECPropertyFormatter const* m_propertyFormatter;
        IPropertyCategorySupplier const* m_categorySupplier;
        bvector<std::shared_ptr<ECInstanceChangeEventSource>> m_ecInstanceChangeEventSources;
        bvector<std::shared_ptr<IUpdateRecordsHandler>> m_updateRecordsHandlers;
        std::shared_ptr<IUiStateProvider> m_uiStateProvider;
    public:
        //! Constructor.
        //! @param[in] paths Known directory paths required by the presentation manager
        Params(Paths paths)
            : m_paths(paths), m_localState(nullptr),
            m_propertyFormatter(nullptr), m_categorySupplier(nullptr)
            {}

        Paths const& GetPaths() const {return m_paths;}

        CachingParams const& GetCachingParams() const { return m_cachingParams; }
        void SetCachingParams(CachingParams params) { m_cachingParams = params; }

        ContentCachingParams const& GetContentCachingParams() const {return m_contentCachingParams;}
        void SetContentCachingParams(ContentCachingParams params) {m_contentCachingParams = params;}

        MultiThreadingParams const& GetMultiThreadingParams() const { return m_multiThreadingParams; }
        void SetMultiThreadingParams(MultiThreadingParams params) { m_multiThreadingParams = params; }

        void SetConnections(std::shared_ptr<IConnectionManager> connections) {m_connections = connections;}
        std::shared_ptr<IConnectionManager> GetConnections() const {return m_connections;}

        IJsonLocalState* GetLocalState() const {return m_localState;}
        void SetLocalState(IJsonLocalState* localState) {m_localState = localState;}
        IECPropertyFormatter const* GetECPropertyFormatter() const {return m_propertyFormatter;}
        void SetECPropertyFormatter(IECPropertyFormatter const* formatter) {m_propertyFormatter = formatter;}
        IPropertyCategorySupplier const* GetCategorySupplier() const {return m_categorySupplier;}
        void SetCategorySupplier(IPropertyCategorySupplier const* supplier) {m_categorySupplier = supplier;}
        bvector<std::shared_ptr<ECInstanceChangeEventSource>> const& GetECInstanceChangeEventSources() const {return m_ecInstanceChangeEventSources;}
        void SetECInstanceChangeEventSources(bvector<std::shared_ptr<ECInstanceChangeEventSource>> sources) {m_ecInstanceChangeEventSources = sources;}
        bvector<std::shared_ptr<IUpdateRecordsHandler>> const& GetUpdateRecordsHandlers() const {return m_updateRecordsHandlers;}
        void SetUpdateRecordsHandlers(bvector<std::shared_ptr<IUpdateRecordsHandler>> handlers) {m_updateRecordsHandlers = handlers;}
        std::shared_ptr<IUiStateProvider> GetUiStateProvider() const {return m_uiStateProvider;}
        void SetUiStateProvider(std::shared_ptr<IUiStateProvider> provider) {m_uiStateProvider = provider;}
    };

private:
    static IECPresentationSerializer const* s_serializer;
    Impl* m_impl;
    ECPresentationTasksManager* m_tasksManager;

private:
    Utf8CP GetConnectionId(ECDbCR) const;
    IConnectionCR GetTaskConnection(IECPresentationTask const&) const;

public:
    ECPresentationTasksManager& GetTasksManager() const {return *m_tasksManager;}
    ECPRESENTATION_EXPORT IUserSettingsManager& GetUserSettings() const;
    ECPRESENTATION_EXPORT IECPropertyFormatter const& GetFormatter() const;
    ECPRESENTATION_EXPORT std::unique_ptr<ImplParams> CreateImplParams(Params const& managerParams);
    Impl& GetImpl() const {return *m_impl;}
    ECPRESENTATION_EXPORT void SetImpl(Impl&);
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> GetTasksCompletion() const;
    ECPRESENTATION_EXPORT unsigned GetBackgroundThreadsCount() const;

public:
/** @name General */
/** @{ */
    //! Constructor.
    //! @param[in] params A object that contains various configuration parameters for the presentation manager
    ECPRESENTATION_EXPORT ECPresentationManager(Params const& params);

    //! Destructor.
    ECPRESENTATION_EXPORT ~ECPresentationManager();

    //! Get the ruleset locater manager.
    ECPRESENTATION_EXPORT IRulesetLocaterManager& GetLocaters();

    //! Get the user settings manager.
    //! @note Local state must be set for user settings to work.
    //! @see SetLocalState
    ECPRESENTATION_EXPORT IUserSettings& GetUserSettings(Utf8CP rulesetId) const;

    //! Get the connection manager used by this presentation manager.
    ECPRESENTATION_EXPORT IConnectionManagerCR GetConnections() const;
    ECPRESENTATION_EXPORT IConnectionManagerR GetConnections();

    ECPRESENTATION_EXPORT static void SetSerializer(IECPresentationSerializer const*);
    ECPRESENTATION_EXPORT static IECPresentationSerializer const& GetSerializer();
/** @} */

/** @name Navigation
 *  @{ */
    //! Retrieves the root nodes.
    ECPRESENTATION_EXPORT folly::Future<NodesResponse> GetNodes(WithPageOptions<AsyncHierarchyRequestParams> const&);

    //! Retrieves the number of root nodes.
    ECPRESENTATION_EXPORT folly::Future<NodesCountResponse> GetNodesCount(AsyncHierarchyRequestParams const&);

    //! Retrieves the parent node of the specified node.
    ECPRESENTATION_EXPORT folly::Future<NodeResponse> GetParent(AsyncNodeParentRequestParams const&);

    //! Retrieves the node with the specified node key.
    ECPRESENTATION_EXPORT folly::Future<NodeResponse> GetNode(AsyncNodeByKeyRequestParams const&);

    //! Returns node paths from the provided node key paths.
    ECPRESENTATION_EXPORT folly::Future<NodePathsResponse> GetNodePaths(AsyncNodePathsFromInstanceKeyPathsRequestParams const&);

    //! Returns filtered node paths
    ECPRESENTATION_EXPORT folly::Future<NodePathsResponse> GetNodePaths(AsyncNodePathsFromFilterTextRequestParams const&);

    //! Compare cached presentation data between two rulesets.
    //! @returns String containing hash path from root to node from which comparison should be continued. nullptr if comparsion was completed.
    ECPRESENTATION_EXPORT folly::Future<HierarchiesCompareResponse> CompareHierarchies(AsyncHierarchyCompareRequestParams const&);
/** @} */

/** @name Content
 *  @{ */
    //! Get content classes from the list of supplied input classes.
    ECPRESENTATION_EXPORT folly::Future<ContentClassesResponse> GetContentClasses(AsyncContentClassesRequestParams const&);

    //! Get the content descriptor based on the supplied parameters.
    ECPRESENTATION_EXPORT folly::Future<ContentDescriptorResponse> GetContentDescriptor(AsyncContentDescriptorRequestParams const&);

    //! Get the content.
    ECPRESENTATION_EXPORT folly::Future<ContentResponse> GetContent(WithPageOptions<AsyncContentRequestParams> const&);

    //! Get the content set size.
    ECPRESENTATION_EXPORT folly::Future<ContentSetSizeResponse> GetContentSetSize(AsyncContentRequestParams const&);

    //! Get display label of specific ECInstance
    ECPRESENTATION_EXPORT folly::Future<DisplayLabelResponse> GetDisplayLabel(AsyncECInstanceDisplayLabelRequestParams const&);

    //! Get aggregated display label of multiple ECInstances
    ECPRESENTATION_EXPORT folly::Future<DisplayLabelResponse> GetDisplayLabel(AsyncKeySetDisplayLabelRequestParams const&);

    //! Get distinct values
    ECPRESENTATION_EXPORT folly::Future<DistinctValuesResponse> GetDistinctValues(WithPageOptions<AsyncDistinctValuesRequestParams> const&);
/** @} */
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
