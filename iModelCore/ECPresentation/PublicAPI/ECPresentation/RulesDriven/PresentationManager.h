/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/IECPresentationManager.h>
#include <ECPresentation/Localization.h>
#include <ECPresentation/RulesDriven/IRulesPreprocessor.h>
#include <ECPresentation/RulesDriven/RuleSetLocater.h>
#include <ECPresentation/RulesDriven/UserSettings.h>
#include <ECPresentation/RulesDriven/ECInstanceChangeEvents.h>
#include <ECPresentation/Update.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

// Logger Namespaces
#define LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE                    LOGGER_NAMESPACE_ECPRESENTATION ".RulesEngine"
#define LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_NAVIGATION         LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE ".Navigation"
#define LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_NAVIGATION_CACHE   LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_NAVIGATION ".Cache"
#define LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_CONTENT            LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE ".Content"
#define LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_USERSETTINGS       LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE ".RulesetVariables"
#define LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_LOCALIZATION       LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE ".Localization"
#define LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_UPDATE             LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE ".Update"
#define LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_THREADS            LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE ".Threads"

#if defined(BENTLEYCONFIG_OS_APPLE_IOS) || defined(BENTLEYCONFIG_OS_WINRT) || defined(BENTLEYCONFIG_OS_ANDROID)
    // 50 MB on mobile platforms
    #define DEFAULT_DISK_CACHE_SIZE_LIMIT   50 * 1024 * 1024
#else
    // 1 GB on desktop
    #define DEFAULT_DISK_CACHE_SIZE_LIMIT   1024 * 1024 * 1024
#endif

#define DEFAULT_BACKGROUND_THREADS_COUNT    1
#define DEFAULT_REQUEST_PRIORITY            1000

//__PUBLISH_SECTION_END__
struct ECPresentationTasksManager;
struct IECPresentationTask;
//__PUBLISH_SECTION_START__

//=======================================================================================
//! Rules-driven presentation manager implementation which uses presentation rules for
//! determining the hierarchies and content.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RulesDrivenECPresentationManager : IECPresentationManager
{
    struct Impl;
//__PUBLISH_SECTION_END__
    struct ConnectionManagerWrapper;
    struct RulesetLocaterManagerWrapper;
    struct UserSettingsManagerWrapper;
    struct ECInstanceChangeEventSourceWrapper;
//__PUBLISH_SECTION_START__

    //===================================================================================
    // @bsiclass                                    Grigas.Petraitis            07/2018
    //===================================================================================
    enum class Mode
        {
        ReadOnly,
        ReadWrite,
        };

    //===================================================================================
    //! An object that stores paths used by RulesDrivenECPresentationManager
    // @bsiclass                                    Grigas.Petraitis            08/2017
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
    //! Parameters for RulesDrivenECPresentationManager
    // @bsiclass                                    Grigas.Petraitis            07/2018
    //===================================================================================
    struct Params
    {
        //===================================================================================
        // @bsiclass                                    Grigas.Petraitis            09/2019
        //===================================================================================
        struct CachingParams
        {
        private:
            bool m_disableDiskCache;
            uint64_t m_diskCacheFileSizeLimit;
            BeFileName m_cacheDirectory;
        public:
            CachingParams() : m_disableDiskCache(false), m_diskCacheFileSizeLimit(DEFAULT_DISK_CACHE_SIZE_LIMIT), m_cacheDirectory(L"") {}
            CachingParams(uint64_t diskCacheFileSizeLimit): m_disableDiskCache(false), m_diskCacheFileSizeLimit(diskCacheFileSizeLimit), m_cacheDirectory(L"") {}
            CachingParams(Utf8StringCR cacheDirectory) : m_disableDiskCache(false), m_diskCacheFileSizeLimit(DEFAULT_DISK_CACHE_SIZE_LIMIT), m_cacheDirectory(cacheDirectory) {}
            //! Is hierarchy caching on disk disabled
            bool ShouldDisableDiskCache() const {return m_disableDiskCache;}
            void SetDisableDiskCache(bool value) {m_disableDiskCache = value;}
            //! Maximum allowed size (in bytes) of cache that's stored on disk by presentation manager.
            //! 0 means infinite size. Defaults to DEFAULT_DISK_CACHE_SIZE_LIMIT.
            uint64_t GetDiskCacheFileSizeLimit() const {return m_diskCacheFileSizeLimit;}
            void SetDiskCacheFileSizeLimit(uint64_t value) {m_diskCacheFileSizeLimit = value;}
            //! Path to the directory for storing hierarchy caches.
            //! Empty path means cache is stored alongside db used to create hierarchy
            BeFileNameCR GetCacheDirectoryPath() const { return m_cacheDirectory; }
            void SetCacheDirectoryPath(BeFileNameCR value) { m_cacheDirectory = value; }
        };

        //===================================================================================
        // @bsiclass                                    Grigas.Petraitis            09/2019
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
        IConnectionManagerP m_connections;
        Paths m_paths;
        Mode m_mode;
        CachingParams m_cachingParams;
        MultiThreadingParams m_multiThreadingParams;
        IJsonLocalState* m_localState;
        ILocalizationProvider const* m_localizationProvider;
        IECPropertyFormatter const* m_propertyFormatter;
        IPropertyCategorySupplier const* m_categorySupplier;
        IRulesetLocaterManager* m_rulesetLocaters;
        IUserSettingsManager* m_userSettings;
        bvector<std::shared_ptr<ECInstanceChangeEventSource>> m_ecInstanceChangeEventSources;
        bvector<std::shared_ptr<IUpdateRecordsHandler>> m_updateRecordsHandlers;
    public:
        //! Constructor.
        //! @param[in] paths Known directory paths required by the presentation manager
        Params(Paths paths)
            : m_paths(paths), m_connections(nullptr), m_localState(nullptr), m_localizationProvider(nullptr),
            m_propertyFormatter(nullptr), m_categorySupplier(nullptr), m_rulesetLocaters(nullptr),
            m_userSettings(nullptr), m_mode(Mode::ReadWrite)
            {}

        Paths const& GetPaths() const {return m_paths;}

        Mode GetMode() const {return m_mode;}
        void SetMode(Mode mode) {m_mode = mode;}

        CachingParams const& GetCachingParams() const { return m_cachingParams; }
        void SetCachingParams(CachingParams params) { m_cachingParams = params; }

        MultiThreadingParams const& GetMultiThreadingParams() const { return m_multiThreadingParams; }
        void SetMultiThreadingParams(MultiThreadingParams params) { m_multiThreadingParams = params; }

        //! @note: Manager takes ownership of the supplied `IConnectionManager` object
        void SetConnections(IConnectionManagerP connections) {m_connections = connections;}
        IConnectionManagerP GetConnections() const {return m_connections;}
        //! @note: Manager takes ownership of the supplied `IRulesetLocaterManager` object
        void SetRulesetLocaters(IRulesetLocaterManager* locaters) { m_rulesetLocaters = locaters; }
        IRulesetLocaterManager* GetRulesetLocaters() const { return m_rulesetLocaters; }
        //! @note: Manager takes ownership of the supplied `IUserSettingsManager` object
        void SetUserSettings(IUserSettingsManager* settings) { m_userSettings = settings; }
        IUserSettingsManager* GetUserSettings() const { return m_userSettings; }

        IJsonLocalState* GetLocalState() const {return m_localState;}
        void SetLocalState(IJsonLocalState* localState) {m_localState = localState;}
        ILocalizationProvider const* GetLocalizationProvider() const {return m_localizationProvider;}
        void SetLocalizationProvider(ILocalizationProvider const* provider) {m_localizationProvider = provider;}
        IECPropertyFormatter const* GetECPropertyFormatter() const {return m_propertyFormatter;}
        void SetECPropertyFormatter(IECPropertyFormatter const* formatter) {m_propertyFormatter = formatter;}
        IPropertyCategorySupplier const* GetCategorySupplier() const {return m_categorySupplier;}
        void SetCategorySupplier(IPropertyCategorySupplier const* supplier) {m_categorySupplier = supplier;}
        bvector<std::shared_ptr<ECInstanceChangeEventSource>> const& GetECInstanceChangeEventSources() const {return m_ecInstanceChangeEventSources;}
        void SetECInstanceChangeEventSources(bvector<std::shared_ptr<ECInstanceChangeEventSource>> sources) {m_ecInstanceChangeEventSources = sources;}
        bvector<std::shared_ptr<IUpdateRecordsHandler>> const& GetUpdateRecordsHandlers() const {return m_updateRecordsHandlers;}
        void SetUpdateRecordsHandlers(bvector<std::shared_ptr<IUpdateRecordsHandler>> handlers) {m_updateRecordsHandlers = handlers;}
    };

    //===================================================================================
    //! A helper class to help create the extended options JSON object for rules-driven
    //! presentation manager's request functions.
    // @bsiclass                                    Grigas.Petraitis            07/2018
    //===================================================================================
    struct CommonOptions : JsonCppAccessor
        {
        ECPRESENTATION_EXPORT static const Utf8CP OPTION_NAME_RulesetId;
        ECPRESENTATION_EXPORT static const Utf8CP OPTION_NAME_Locale;
        ECPRESENTATION_EXPORT static const Utf8CP OPTION_NAME_UnitSystem;
        ECPRESENTATION_EXPORT static const Utf8CP OPTION_NAME_Priority;

        //! Constructor. Creates a read-only accessor.
        CommonOptions(JsonValueCR data) : JsonCppAccessor(data) {}
        //! Constructor. Creates a read-write accessor.
        CommonOptions(JsonValueR data) : JsonCppAccessor(data) {}
        //! Constructor. Creates a read-write accessor.
        CommonOptions(Json::Value&& data) : JsonCppAccessor(std::move(data)) {}
        //! Copy constructor.
        CommonOptions(CommonOptions const& other) : JsonCppAccessor(other) {}
        //! Constructor.
        //! @param[in] rulesetId The ID of the ruleset.
        //! @param[in] locale Locale identifier
        //! @param[in] unitSystem Unit system to use for formatting property values
        CommonOptions(Utf8CP rulesetId, Utf8CP locale = nullptr, UnitSystem unitSystem = UnitSystem::Undefined)
            : JsonCppAccessor()
            {
            SetRulesetId(rulesetId);
            SetLocale(locale);
            SetUnitSystem(unitSystem);
            }

        //! Is ruleset ID defined.
        bool HasRulesetId() const {return GetJson().isMember(OPTION_NAME_RulesetId);}
        //! Get the ruleset ID.
        Utf8CP GetRulesetId() const {return GetJson().isMember(OPTION_NAME_RulesetId) ? GetJson()[OPTION_NAME_RulesetId].asCString() : "";}
        //! Set the ruleset ID.
        void SetRulesetId(Utf8CP rulesetId) {rulesetId ? AddMember(OPTION_NAME_RulesetId, rulesetId) : RemoveMember(OPTION_NAME_RulesetId);}

        //! Get locale identifier.
        Utf8CP GetLocale() const {return GetJson().isMember(OPTION_NAME_Locale) ? GetJson()[OPTION_NAME_Locale].asCString() : "";}
        //! Set locale identifier.
        void SetLocale(Utf8CP locale) {locale ? AddMember(OPTION_NAME_Locale, locale) : RemoveMember(OPTION_NAME_Locale);}

        //! Get unit system.
        UnitSystem GetUnitSystem() const {return (UnitSystem)GetJson()[OPTION_NAME_UnitSystem].asInt(0);}
        //! Set unit system.
        void SetUnitSystem(UnitSystem unitSystem) {ECPresentation::UnitSystem::Undefined != unitSystem ? AddMember(OPTION_NAME_UnitSystem, (int)unitSystem) : RemoveMember(OPTION_NAME_UnitSystem);}

        //! Get priority.
        int GetPriority() const {return GetJson().isMember(OPTION_NAME_Priority) ? GetJson()[OPTION_NAME_Priority].asInt() : DEFAULT_REQUEST_PRIORITY;}
        //! Set priority.
        void SetPriority(int priority) {AddMember(OPTION_NAME_Priority, priority);}
        };

    //===================================================================================
    //! A helper class to help create the extended options JSON object for rules-driven
    //! presentation manager's navigation-related request functions.
    // @bsiclass                                    Grigas.Petraitis            03/2015
    //===================================================================================
    struct NavigationOptions : CommonOptions
        {
        //! Constructor. Creates a read-only accessor.
        NavigationOptions(JsonValueCR data) : CommonOptions(data) {}
        //! Constructor. Creates a read-write accessor.
        NavigationOptions(JsonValueR data) : CommonOptions(data) {}
        //! Constructor. Creates a read-write accessor.
        NavigationOptions(Json::Value&& data) : CommonOptions(std::move(data)) {}
        //! Copy constructor.
        NavigationOptions(NavigationOptions const& other) : CommonOptions(other) {}
        //! Constructor.
        //! @param[in] rulesetId The ID of the ruleset to use for requesting nodes.
        //! @param[in] locale Locale identifier
        //! @param[in] unitSystem Unit system to use for formatting property values
        NavigationOptions(Utf8CP rulesetId, Utf8CP locale = nullptr, UnitSystem unitSystem = UnitSystem::Undefined) 
            : CommonOptions(rulesetId, locale, unitSystem) 
            {}
        //! Constructor.
        //! @param[in] rulesetId The ID of the ruleset to use for requesting nodes.
        //! @param[in] locale Locale identifier
        //! @param[in] unitSystem Unit system to use for formatting property values
        NavigationOptions(Utf8StringCR rulesetId, Utf8StringCR locale = "", UnitSystem unitSystem = UnitSystem::Undefined) 
            : CommonOptions(rulesetId.c_str(), locale.empty() ? nullptr : locale.c_str(), unitSystem) 
            {}
        };

    //===================================================================================
    //! A helper class to help create the extended options JSON object for rules-driven
    //! presentation manager's content-related request functions.
    // @bsiclass                                    Grigas.Petraitis            03/2015
    //===================================================================================
    struct ContentOptions : CommonOptions
        {
        //! Constructor. Creates a read-only accessor.
        ContentOptions(JsonValueCR data) : CommonOptions(data) {}
        //! Constructor. Creates a read-write accessor.
        ContentOptions(JsonValueR data) : CommonOptions(data) {}
        //! Constructor. Creates a read-write accessor.
        ContentOptions(Json::Value&& data) : CommonOptions(std::move(data)) {}
        //! Copy constructor.
        ContentOptions(ContentOptions const& other) : CommonOptions(other) {}
        //! Constructor.
        //! @param[in] rulesetId The ID of the ruleset to use for requesting content.
        //! @param[in] locale Locale identifier
        //! @param[in] unitSystem Unit system to use for formatting property values
        ContentOptions(Utf8CP rulesetId, Utf8CP locale = nullptr, UnitSystem unitSystem = UnitSystem::Undefined) 
            : CommonOptions(rulesetId, locale, unitSystem)
            {}
        //! Constructor.
        //! @param[in] rulesetId The ID of the ruleset to use for requesting content.
        //! @param[in] locale Locale identifier
        //! @param[in] unitSystem Unit system to use for formatting property values
        ContentOptions(Utf8StringCR rulesetId, Utf8StringCR locale = "", UnitSystem unitSystem = UnitSystem::Undefined) 
            : CommonOptions(rulesetId.c_str(), locale.empty() ? nullptr : locale.c_str(), unitSystem)
            {}
        };

private:
    Impl* m_impl;
    ECPresentationTasksManager* m_tasksManager;

private:
    Utf8CP GetConnectionId(ECDbCR) const;
    IConnectionCPtr GetTaskConnection(IECPresentationTask const&) const;

protected:
    // IECPresentationManager: General
    ECPRESENTATION_EXPORT IConnectionManagerR _GetConnections() override;

    // IECPresentationManager: Navigation
    ECPRESENTATION_EXPORT virtual folly::Future<NavNodesContainer> _GetRootNodes(ECDbCR, PageOptionsCR, JsonValueCR, PresentationRequestContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<size_t> _GetRootNodesCount(ECDbCR, JsonValueCR, PresentationRequestContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<NavNodesContainer> _GetChildren(ECDbCR, NavNodeCR, PageOptionsCR, JsonValueCR, PresentationRequestContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<size_t> _GetChildrenCount(ECDbCR, NavNodeCR, JsonValueCR, PresentationRequestContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<NavNodeCPtr> _GetParent(ECDbCR, NavNodeCR, JsonValueCR, PresentationRequestContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<NavNodeCPtr> _GetNode(ECDbCR, NavNodeKeyCR, JsonValueCR, PresentationRequestContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<bvector<NavNodeCPtr>> _GetFilteredNodes(ECDbCR, Utf8CP filterText, JsonValueCR, PresentationRequestContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<bvector<NodesPathElement>> _GetFilteredNodePaths(ECDbCR, Utf8CP filterText, JsonValueCR, PresentationRequestContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<bvector<NodesPathElement>> _GetNodePaths(ECDbCR, bvector<bvector<ECInstanceKey>> const&, int64_t, JsonValueCR, PresentationRequestContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<NodesPathElement> _GetNodePath(ECDbCR, bvector<ECInstanceKey> const&, JsonValueCR, PresentationRequestContextCR) override;

    // IECPresentationManager: Content
    ECPRESENTATION_EXPORT virtual folly::Future<bvector<SelectClassInfo>> _GetContentClasses(ECDbCR, Utf8CP, int, bvector<ECClassCP> const&, JsonValueCR, PresentationRequestContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<ContentDescriptorCPtr> _GetContentDescriptor(ECDbCR, Utf8CP, int, KeySetCR, SelectionInfo const*, JsonValueCR, PresentationRequestContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<ContentCPtr> _GetContent(ContentDescriptorCR, PageOptionsCR, PresentationRequestContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<size_t> _GetContentSetSize(ContentDescriptorCR, PresentationRequestContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<LabelDefinitionCPtr> _GetDisplayLabel(ECDbCR, KeySetCR, JsonValueCR, PresentationRequestContextCR) override;

//__PUBLISH_SECTION_END__
public:
    ECPresentationTasksManager& GetTasksManager() const {return *m_tasksManager;}
    ECPRESENTATION_EXPORT IUserSettingsManager& GetUserSettings() const;
    ECPRESENTATION_EXPORT Params CreateImplParams(Params const& managerParams);
    Impl& GetImpl() const {return *m_impl;}
    ECPRESENTATION_EXPORT void SetImpl(Impl&);
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> GetTasksCompletion() const;
    ECPRESENTATION_EXPORT unsigned GetBackgroundThreadsCount() const;

//__PUBLISH_SECTION_START__
public:
    //! Constructor.
    //! @param[in] connections Connection manager used by this presentation manager.
    //! @param[in] paths Application paths provider.
    //! @param[in] disableDiskCache Is hierarchy caching on disk disabled. It's recommended to keep
    //!            this enabled unless being used for testing.
    //! @deprecated Use RulesDrivenECPresentationManager::RulesDrivenECPresentationManager(Params const&)
    ECPRESENTATION_EXPORT RulesDrivenECPresentationManager(IConnectionManagerR connections, Paths const& paths, bool disableDiskCache = false);

    //! Constructor.
    //! @param[in] params A object that contains various configuration parameters for the presentation manager
    ECPRESENTATION_EXPORT RulesDrivenECPresentationManager(Params const& params);

    //! Destructor.
    ECPRESENTATION_EXPORT ~RulesDrivenECPresentationManager();

    //! Get the ruleset locater manager.
    ECPRESENTATION_EXPORT IRulesetLocaterManager& GetLocaters();

    //! Get the user settings manager.
    //! @note Local state must be set for user settings to work.
    //! @see SetLocalState
    ECPRESENTATION_EXPORT IUserSettings& GetUserSettings(Utf8CP rulesetId) const;
    
    //! Get the connection manager used by this presentation manager.
    ECPRESENTATION_EXPORT IConnectionManagerCR GetConnectionsCR() const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
