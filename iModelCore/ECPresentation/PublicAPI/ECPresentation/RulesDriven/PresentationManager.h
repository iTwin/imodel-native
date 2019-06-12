/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/IECPresentationManager.h>
#include <ECPresentation/Localization.h>
#include <ECPresentation/RulesDriven/IRulesPreprocessor.h>
#include <ECPresentation/RulesDriven/RuleSetLocater.h>
#include <ECPresentation/RulesDriven/UserSettings.h>
#include <ECPresentation/RulesDriven/ECInstanceChangeHandlers.h>
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

//__PUBLISH_SECTION_END__
//#define RULES_ENGINE_FORCE_SINGLE_THREAD
struct IRulesDrivenECPresentationManagerDependenciesFactory;
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
    struct CancelableTasksStore;
    struct ConnectionManagerWrapper;
    struct RulesetLocaterManagerWrapper;
    struct UserSettingsManagerWrapper;
    struct ECInstanceChangeEventSourceWrapper;

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
    private:
        IConnectionManagerR m_connections;
        Paths m_paths;
        bool m_disableDiskCache;
        uint64_t m_diskCacheFileSizeLimit;
    public:
        //! Constructor.
        //! @param[in] connections Connection manager used by the presentation manager
        //! @param[in] paths Known directory paths required by the presentation manager
        Params(IConnectionManagerR connections, Paths paths)
            : m_connections(connections), m_paths(paths)
            {
            m_disableDiskCache = false;
            m_diskCacheFileSizeLimit = DEFAULT_DISK_CACHE_SIZE_LIMIT;
            }
        IConnectionManagerR GetConnections() const {return m_connections;}
        Paths const& GetPaths() const {return m_paths;}
        //! Is hierarchy caching on disk disabled
        bool ShouldDisableDiskCache() const {return m_disableDiskCache;}
        void SetDisableDiskCache(bool value) {m_disableDiskCache = value;}
        //! Maximum allowed size (in bytes) of cache that's stored on disk by presentation manager.
        //! 0 means infinite size. Defaults to DEFAULT_DISK_CACHE_SIZE_LIMIT.
        uint64_t GetDiskCacheFileSizeLimit() const {return m_diskCacheFileSizeLimit;}
        void SetDiskCacheFileSizeLimit(uint64_t value) {m_diskCacheFileSizeLimit = value;}
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

    protected:
        //! Constructor. Creates a read-only accessor.
        CommonOptions(JsonValueCR data) : JsonCppAccessor(data) {}
        //! Constructor. Creates a read-write accessor.
        CommonOptions(JsonValueR data) : JsonCppAccessor(data) {}
        //! Copy constructor.
        CommonOptions(CommonOptions const& other) : JsonCppAccessor(other) {}
        //! Constructor.
        //! @param[in] rulesetId The ID of the ruleset.
        //! @param[in] locale Locale identifier
        CommonOptions(Utf8CP rulesetId, Utf8CP locale = nullptr) : JsonCppAccessor() {SetRulesetId(rulesetId); SetLocale(locale);}

    public:
        //! Is ruleset ID defined.
        bool HasRulesetId() const {return GetJson().isMember(OPTION_NAME_RulesetId);}
        //! Get the ruleset ID.
        Utf8CP GetRulesetId() const {return GetJson().isMember(OPTION_NAME_RulesetId) ? GetJson()[OPTION_NAME_RulesetId].asCString() : "";}
        //! Set the ruleset ID.
        void SetRulesetId(Utf8CP rulesetId) {AddMember(OPTION_NAME_RulesetId, rulesetId);}

        //! Get locale identifier.
        Utf8CP GetLocale() const {return GetJson().isMember(OPTION_NAME_Locale) ? GetJson()[OPTION_NAME_Locale].asCString() : "";}
        //! Set locale identifier.
        void SetLocale(Utf8CP locale) {AddMember(OPTION_NAME_Locale, locale);}
    };

    //===================================================================================
    //! A helper class to help create the extended options JSON object for rules-driven
    //! presentation manager's navigation-related request functions.
    // @bsiclass                                    Grigas.Petraitis            03/2015
    //===================================================================================
    struct NavigationOptions : CommonOptions
        {
        ECPRESENTATION_EXPORT static const Utf8CP OPTION_NAME_RuleTargetTree;
        ECPRESENTATION_EXPORT static const Utf8CP OPTION_NAME_DisableUpdates;

        //! Constructor. Creates a read-only accessor.
        NavigationOptions(JsonValueCR data) : CommonOptions(data) {}
        //! Constructor. Creates a read-write accessor.
        NavigationOptions(JsonValueR data) : CommonOptions(data) {}
        //! Copy constructor.
        NavigationOptions(NavigationOptions const& other) : CommonOptions(other) {}
        //! Constructor.
        //! @param[in] rulesetId The ID of the ruleset to use for requesting nodes.
        //! @param[in] ruleTargetTree The target tree.
        //! @param[in] disableUpdates True if hierarchy should not be auto-updating. (User knows that hierarchy won't change)
        //! @param[in] locale Locale identifier
        NavigationOptions(Utf8CP rulesetId, RuleTargetTree ruleTargetTree = TargetTree_Both, bool disableUpdates = false, Utf8CP locale = nullptr)
            : CommonOptions(rulesetId, locale) {SetRuleTargetTree(ruleTargetTree); SetDisableUpdates(disableUpdates);}
        //! Constructor.
        //! @param[in] rulesetId The ID of the ruleset to use for requesting nodes.
        //! @param[in] ruleTargetTree The target tree.
        //! @param[in] disableUpdates True if hierarchy should not be auto-updating. (User knows that hierarchy won't change)
        //! @param[in] locale Locale identifier
        NavigationOptions(Utf8StringCR rulesetId, RuleTargetTree ruleTargetTree = TargetTree_Both, bool disableUpdates = false, Utf8CP locale = nullptr)
            : NavigationOptions(rulesetId.c_str(), ruleTargetTree, disableUpdates, locale) {}

        //! Get disable updates.
        bool GetDisableUpdates() const {return GetJson().isMember(OPTION_NAME_DisableUpdates) ? GetJson()[OPTION_NAME_DisableUpdates].asBool() : false;}
        //! Set disable updates.
        void SetDisableUpdates(bool disableUpdates) {AddMember(OPTION_NAME_DisableUpdates, disableUpdates);}

        //! Is rule target tree defined.
        bool HasRuleTargetTree() const {return GetJson().isMember(OPTION_NAME_RuleTargetTree);}
        //! Get the rule target tree.
        RuleTargetTree GetRuleTargetTree() const {return GetJson().isMember(OPTION_NAME_RuleTargetTree) ? (RuleTargetTree)GetJson()[OPTION_NAME_RuleTargetTree].asInt() : RuleTargetTree::TargetTree_MainTree;}
        //! Set the rule target tree.
        void SetRuleTargetTree(RuleTargetTree ruleTargetTree) {AddMember(OPTION_NAME_RuleTargetTree, (int)ruleTargetTree);}
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
        //! Copy constructor.
        ContentOptions(ContentOptions const& other) : CommonOptions(other) {}
        //! Constructor.
        //! @param[in] rulesetId The ID of the ruleset to use for requesting content.
        //! @param[in] locale Locale identifier
        ContentOptions(Utf8CP rulesetId, Utf8CP locale = nullptr) : CommonOptions(rulesetId, locale) {}
        //! Constructor.
        //! @param[in] rulesetId The ID of the ruleset to use for requesting content.
        //! @param[in] locale Locale identifier
        ContentOptions(Utf8StringCR rulesetId, Utf8StringCR locale = "") : CommonOptions(rulesetId.c_str(), locale.empty() ? nullptr : locale.c_str()) {}
        };

private:
    Impl* m_impl;
    folly::Executor* m_executor;
    CancelableTasksStore* m_cancelableTasks;
    ConnectionManagerWrapper* m_connectionsWrapper;
    bvector<ECInstanceChangeEventSourceWrapper*> m_ecInstanceChangeEventSourceWrappers;

protected:
    // IECPresentationManager: Navigation
    ECPRESENTATION_EXPORT virtual folly::Future<NavNodesContainer> _GetRootNodes(IConnectionCR, PageOptionsCR, JsonValueCR, PresentationTaskNotificationsContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<size_t> _GetRootNodesCount(IConnectionCR, JsonValueCR, PresentationTaskNotificationsContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<NavNodesContainer> _GetChildren(IConnectionCR, NavNodeCR, PageOptionsCR, JsonValueCR, PresentationTaskNotificationsContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<size_t> _GetChildrenCount(IConnectionCR, NavNodeCR, JsonValueCR, PresentationTaskNotificationsContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<bool> _HasChild(IConnectionCR, NavNodeCR, ECInstanceKeyCR, JsonValueCR, PresentationTaskNotificationsContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<NavNodeCPtr> _GetParent(IConnectionCR, NavNodeCR, JsonValueCR, PresentationTaskNotificationsContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<NavNodeCPtr> _GetNode(IConnectionCR, NavNodeKeyCR, JsonValueCR, PresentationTaskNotificationsContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<bvector<NavNodeCPtr>> _GetFilteredNodes(IConnectionCR, Utf8CP filterText, JsonValueCR, PresentationTaskNotificationsContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<folly::Unit> _OnNodeChecked(IConnectionCR, NavNodeKeyCR, JsonValueCR, PresentationTaskNotificationsContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<folly::Unit> _OnNodeUnchecked(IConnectionCR, NavNodeKeyCR, JsonValueCR, PresentationTaskNotificationsContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<folly::Unit> _OnNodeExpanded(IConnectionCR, NavNodeKeyCR, JsonValueCR, PresentationTaskNotificationsContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<folly::Unit> _OnNodeCollapsed(IConnectionCR, NavNodeKeyCR, JsonValueCR, PresentationTaskNotificationsContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<folly::Unit> _OnAllNodesCollapsed(IConnectionCR, JsonValueCR, PresentationTaskNotificationsContextCR) override;

    // IECPresentationManager: Content
    ECPRESENTATION_EXPORT virtual folly::Future<bvector<SelectClassInfo>> _GetContentClasses(IConnectionCR, Utf8CP, int, bvector<ECClassCP> const&, JsonValueCR, PresentationTaskNotificationsContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<ContentDescriptorCPtr> _GetContentDescriptor(IConnectionCR, Utf8CP, int, KeySetCR, SelectionInfo const*, JsonValueCR, PresentationTaskNotificationsContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<ContentCPtr> _GetContent(ContentDescriptorCR, PageOptionsCR, PresentationTaskNotificationsContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<size_t> _GetContentSetSize(ContentDescriptorCR, PresentationTaskNotificationsContextCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<Utf8String> _GetDisplayLabel(IConnectionCR, KeySetCR, PresentationTaskNotificationsContextCR) override;

    // IECPresentationManager: Updating
    ECPRESENTATION_EXPORT virtual folly::Future<bvector<ECInstanceChangeResult>> _SaveValueChange(IConnectionCR, bvector<ChangedECInstanceInfo> const&, Utf8CP, ECValueCR, JsonValueCR, PresentationTaskNotificationsContextCR) override;
    
    ECPRESENTATION_EXPORT virtual void _OnLocalizationProviderChanged() override;

//__PUBLISH_SECTION_END__
public:
    folly::Executor& GetExecutor() const {return *m_executor;}
    ECPRESENTATION_EXPORT IUserSettingsManager& GetUserSettings() const;
    Impl& GetImpl() const {return *m_impl;}
    ECPRESENTATION_EXPORT void SetImpl(Impl&);
    ECPRESENTATION_EXPORT std::unique_ptr<IRulesDrivenECPresentationManagerDependenciesFactory> CreateDependenciesFactory();

//__PUBLISH_SECTION_START__
public:
    //! Constructor.
    //! @param[in] connections Connection manager used by this presentation manager.
    //! @param[in] paths Application paths provider.
    //! @param[in] disableDiskCache Is hierarchy caching on disk disabled. It's recommended to keep
    //! this enabled unless being used for testing.
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

    //! Set the local state to use for storing user settings.
    ECPRESENTATION_EXPORT void SetLocalState(IJsonLocalState*);

    //! Get a rules preprocessor for a specific ruleset
    //! @param[in] connection Connection that is used for locating the ruleset.
    //! @param[in] rulesetId The ruleset identifier.
    //! @param[in] locale Locale to use.
    //! @param[in] usedSettingsListener (optional) Listener that is passed to the rules preprocessor.
    ECPRESENTATION_EXPORT IRulesPreprocessorPtr GetRulesPreprocessor(IConnectionCR connection, Utf8StringCR rulesetId, Utf8StringCR locale, IUsedUserSettingsListener* usedSettingsListener = nullptr) const;

    //! Get a navigation nodes factory that can be used for navigation node manual creation.
    ECPRESENTATION_EXPORT INavNodesFactoryPtr GetNavNodesFactory() const;

/** @name Property Formatting */
/** @{ */
    //! Set the property formatter.
    //! @details Property formatter is used to format content display values.
    ECPRESENTATION_EXPORT void SetECPropertyFormatter(IECPropertyFormatter const* formatter);

    //! Get the property formatter.
    ECPRESENTATION_EXPORT IECPropertyFormatter const& GetECPropertyFormatter() const;
/** @} */

/** @name Property Categories */
/** @{ */
    //! Set the property category supplier. If null, the @ref DefaultCategorySupplier is used.
    //! @details The category supplier is responsible for determining category for an ECProperty.
    ECPRESENTATION_EXPORT void SetCategorySupplier(IPropertyCategorySupplier* supplier);

    //! Get the property category supplier used by this presentation manager.
    ECPRESENTATION_EXPORT IPropertyCategorySupplier& GetCategorySupplier() const;

    //! Should be called to force content controls request new categories for displayed content.
    ECPRESENTATION_EXPORT void NotifyCategoriesChanged();
/** @} */

/** @name ECInstance Changes */
/** @{ */
    //! Register an IECInstanceChangeHandler.
    //! @details IECInstanceChangeHandlers are responsible for changing ECInstances in the data
    //! sources. E.g. the DgnDbECInstanceChangeHandler can change ECInstances in DgbDb-based
    //! data sources.
    ECPRESENTATION_EXPORT void RegisterECInstanceChangeHandler(IECInstanceChangeHandler&);

    //! Unregister an IECInstanceChangeHandler.
    //! @see RegisterECInstanceChangeHandler
    ECPRESENTATION_EXPORT void UnregisterECInstanceChangeHandler(IECInstanceChangeHandler&);

    //! Register an ECInstanceChange events source.
    //! @details ECInstanceChange events sources are responsible for notifying the presentation
    //! manager when some ECInstance-related changes happen in the database.
    ECPRESENTATION_EXPORT void RegisterECInstanceChangeEventSource(ECInstanceChangeEventSource&);

    //! Unregister an ECInstanceChange events source.
    //! @see RegisterECInstanceChangeEventSource
    ECPRESENTATION_EXPORT void UnregisterECInstanceChangeEventSource(ECInstanceChangeEventSource&);

    //! Register an update records handler.
    ECPRESENTATION_EXPORT void RegisterUpdateRecordsHandler(IUpdateRecordsHandler&);

    //! Unregister an update records handler.
    ECPRESENTATION_EXPORT void UnregisterUpdateRecordsHandler(IUpdateRecordsHandler&);
/** @} */
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
