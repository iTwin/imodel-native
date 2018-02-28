/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/PresentationManager.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/IECPresentationManager.h>
#include <ECPresentation/SelectionManager.h>
#include <ECPresentation/Localization.h>
#include <ECPresentation/RulesDriven/RuleSetLocater.h>
#include <ECPresentation/RulesDriven/UserSettings.h>
#include <ECPresentation/RulesDriven/ECInstanceChangeHandlers.h>
#include <ECPresentation/RulesDriven/ECInstanceChangeEvents.h>
#include <ECPresentation/RulesDriven/Update.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

// Logger Namespaces
#define LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE                    LOGGER_NAMESPACE_ECPRESENTATION ".RulesEngine"
#define LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_NAVIGATION         LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE ".Navigation"
#define LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_NAVIGATION_CACHE   LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_NAVIGATION ".Cache"
#define LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_CONTENT            LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE ".Content"
#define LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_USERSETTINGS       LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE ".UserSettings"
#define LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_LOCALIZATION       LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE ".Localization"
#define LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_UPDATE             LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE ".Update"
#define LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE_THREADS            LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE ".Threads"

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
    struct SelectionManagerWrapper;
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
    //! A helper class to help create the extended options JSON object for rules-driven
    //! presentation manager's navigation-related request functions.
    // @bsiclass                                    Grigas.Petraitis            03/2015
    //===================================================================================
    struct NavigationOptions : JsonCppAccessor
        {
        ECPRESENTATION_EXPORT static const Utf8CP OPTION_NAME_RulesetId;
        ECPRESENTATION_EXPORT static const Utf8CP OPTION_NAME_RuleTargetTree;
        ECPRESENTATION_EXPORT static const Utf8CP OPTION_NAME_DisableUpdates;

        //! Constructor. Creates a read-only accessor.
        NavigationOptions(JsonValueCR data) : JsonCppAccessor(data) {}
        //! Constructor. Creates a read-write accessor.
        NavigationOptions(JsonValueR data) : JsonCppAccessor(data) {}
        //! Copy constructor.
        NavigationOptions(NavigationOptions const& other) : JsonCppAccessor(other) {}
        //! Constructor.
        //! @param[in] rulesetId The ID of the ruleset to use for requesting nodes.
        //! @param[in] ruleTargetTree The target tree.
        //! @param[in] disableUpdates True if hierarchy should not be auto-updating. (User knows that hierarchy won't change)
        NavigationOptions(Utf8CP rulesetId, RuleTargetTree ruleTargetTree, bool disableUpdates = false) : JsonCppAccessor() {SetRulesetId(rulesetId); SetRuleTargetTree(ruleTargetTree); SetDisableUpdates(disableUpdates);}

        //! Is ruleset ID defined.
        bool HasRulesetId() const {return GetJson().isMember(OPTION_NAME_RulesetId);}
        //! Get the ruleset ID.
        Utf8CP GetRulesetId() const {return GetJson().isMember(OPTION_NAME_RulesetId) ? GetJson()[OPTION_NAME_RulesetId].asCString() : "";}
        //! Set the ruleset ID.
        void SetRulesetId(Utf8CP rulesetId) {AddMember(OPTION_NAME_RulesetId, rulesetId);}

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
    struct ContentOptions : JsonCppAccessor
        {
        ECPRESENTATION_EXPORT static const Utf8CP OPTION_NAME_RulesetId;

        //! Constructor. Creates a read-only accessor.
        ContentOptions(JsonValueCR data) : JsonCppAccessor(data) {}
        //! Constructor. Creates a read-write accessor.
        ContentOptions(JsonValueR data) : JsonCppAccessor(data) {}
        //! Copy constructor.
        ContentOptions(ContentOptions const& other) : JsonCppAccessor(other) {}
        //! Constructor.
        //! @param[in] rulesetId The ID of the ruleset to use for requesting content.
        ContentOptions(Utf8CP rulesetId) : JsonCppAccessor() {SetRulesetId(rulesetId);}
        //! Constructor.
        //! @param[in] rulesetId The ID of the ruleset to use for requesting content.
        ContentOptions(Utf8StringCR rulesetId) : ContentOptions(rulesetId.c_str()) {}

        //! Is ruleset ID defined.
        bool HasRulesetId() const {return GetJson().isMember(OPTION_NAME_RulesetId);}
        //! Get the ruleset ID.
        Utf8CP GetRulesetId() const {return GetJson().isMember(OPTION_NAME_RulesetId) ? GetJson()[OPTION_NAME_RulesetId].asCString() : "";}
        //! Set the ruleset ID.
        void SetRulesetId(Utf8CP rulesetId) {AddMember(OPTION_NAME_RulesetId, rulesetId);}
        };

private:
    Impl* m_impl;
    folly::Executor* m_executor;
    CancelableTasksStore* m_cancelableTasks;
    ConnectionManagerWrapper* m_connectionsWrapper;
    SelectionManagerWrapper* m_selectionProviderWrapper;
    bvector<ECInstanceChangeEventSourceWrapper*> m_ecInstanceChangeEventSourceWrappers;
    
protected:
    // IECPresentationManager: Navigation
    ECPRESENTATION_EXPORT virtual folly::Future<NavNodesContainer> _GetRootNodes(IConnectionCR, PageOptionsCR, JsonValueCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<size_t> _GetRootNodesCount(IConnectionCR, JsonValueCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<NavNodesContainer> _GetChildren(IConnectionCR, NavNodeCR, PageOptionsCR, JsonValueCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<size_t> _GetChildrenCount(IConnectionCR, NavNodeCR, JsonValueCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<bool> _HasChild(IConnectionCR, NavNodeCR, ECInstanceKeyCR, JsonValueCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<NavNodeCPtr> _GetParent(IConnectionCR, NavNodeCR, JsonValueCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<NavNodeCPtr> _GetNode(IConnectionCR, NavNodeKeyCR, JsonValueCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<bvector<NavNodeCPtr>> _GetFilteredNodes(IConnectionCR, Utf8CP filterText, JsonValueCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<folly::Unit> _OnNodeChecked(IConnectionCR, NavNodeKeyCR, JsonValueCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<folly::Unit> _OnNodeUnchecked(IConnectionCR, NavNodeKeyCR, JsonValueCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<folly::Unit> _OnNodeExpanded(IConnectionCR, NavNodeKeyCR, JsonValueCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<folly::Unit> _OnNodeCollapsed(IConnectionCR, NavNodeKeyCR, JsonValueCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<folly::Unit> _OnAllNodesCollapsed(IConnectionCR, JsonValueCR) override;

    // IECPresentationManager: Content
    ECPRESENTATION_EXPORT virtual folly::Future<bvector<SelectClassInfo>> _GetContentClasses(IConnectionCR, Utf8CP, bvector<ECClassCP> const&, JsonValueCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<ContentDescriptorCPtr> _GetContentDescriptor(IConnectionCR, Utf8CP preferredDisplayType, KeySetCR, SelectionInfo const*, JsonValueCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<ContentCPtr> _GetContent(ContentDescriptorCR, PageOptionsCR) override;
    ECPRESENTATION_EXPORT virtual folly::Future<size_t> _GetContentSetSize(ContentDescriptorCR) override;

    // IECPresentationManager: Updating
    ECPRESENTATION_EXPORT virtual folly::Future<bvector<ECInstanceChangeResult>> _SaveValueChange(IConnectionCR, bvector<ChangedECInstanceInfo> const&, Utf8CP, ECValueCR, JsonValueCR) override;

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
    //! @param[in] disableDiskCache Is hierarchy caching on disk disabled. It's recommended to keep this enabled unless being used
    //! for testing.
    ECPRESENTATION_EXPORT RulesDrivenECPresentationManager(IConnectionManagerR connections, Paths const& paths, bool disableDiskCache = false);

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

    //! Set the selection manager.
    //! @details Selection manager is used to listen for selection events and invalidate caches
    //! that depend on selection.
    ECPRESENTATION_EXPORT void SetSelectionManager(ISelectionManager*);
    
    //! Get the selection manager used by presentation manager.
    ECPRESENTATION_EXPORT ISelectionManager* GetSelectionManager() const;

    //! Set custom localization provider. By default BeSQLite::L10N is used.
    ECPRESENTATION_EXPORT void SetLocalizationProvider(ILocalizationProvider const*);

    //! Get localization provider used by this presentation manager.
    ECPRESENTATION_EXPORT ILocalizationProvider const& GetLocalizationProvider() const;

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
    ECPRESENTATION_EXPORT void SetUpdateRecordsHandler(IUpdateRecordsHandler*);
/** @} */
};

//=======================================================================================
//! Selection synchronization handler for rules-driven controls. Basically this is just a
//! helper base class which creates content request options from the selection event
//! and creates rules-driven presentation manager specific selection event extended data.
//! @ingroup GROUP_RulesDrivenPresentation
//! @ingroup GROUP_UnifiedSelection
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct RulesDrivenSelectionSyncHandler : SelectionSyncHandler
{
    //===================================================================================
    //! Helper class to read/write selection change event extended data.
    // @bsiclass                                    Grigas.Petraitis            08/2016
    //===================================================================================
    struct SelectionExtendedData : RapidJsonAccessor
        {
        ECPRESENTATION_EXPORT static const Utf8CP OPTION_NAME_RulesetId;
        ECPRESENTATION_EXPORT static const Utf8CP OPTION_NAME_UseSelectionScope;

        //! Constructor. Creates a read-write accessor.
        SelectionExtendedData() : RapidJsonAccessor() {}
        //! Constructor. Creates a read-only accessor.
        SelectionExtendedData(RapidJsonValueCR data) : RapidJsonAccessor(data) {}
        //! Constructor. Creates a read-write accessor.
        SelectionExtendedData(RapidJsonValueR data, rapidjson::MemoryPoolAllocator<>& allocator) : RapidJsonAccessor(data, allocator) {}
        //! Copy constructor.
        SelectionExtendedData(SelectionExtendedData const& other) : RapidJsonAccessor(other) {}
        //! Constructor.
        //! @param[in] rulesetId The ID of the ruleset which is used by the control that is firing
        //! the selection change event.
        SelectionExtendedData(Utf8CP rulesetId) : RapidJsonAccessor() {SetRulesetId(rulesetId);}
        //! Constructor. Creates a read-only accessor for the specified selection change event.
        SelectionExtendedData(SelectionChangedEventCR evt) : RapidJsonAccessor(evt) {}

        //! Is ruleset ID defined.
        bool HasRulesetId() const {return GetJson().IsObject() && GetJson().HasMember(OPTION_NAME_RulesetId);}
        //! Get the ruleset ID.
        Utf8CP GetRulesetId() const {return GetJson().HasMember(OPTION_NAME_RulesetId) ? GetJson()[OPTION_NAME_RulesetId].GetString() : "";}
        //! Set the ruleset ID.
        void SetRulesetId(Utf8CP rulesetId) {AddMember(OPTION_NAME_RulesetId, rapidjson::Value(rulesetId, GetAllocator()));}

        bool GetUseSelectionScope() const { return GetJson().HasMember(OPTION_NAME_UseSelectionScope) ? GetJson()[OPTION_NAME_UseSelectionScope].GetBool() : false; }
        void SetUseSelectionScope(bool useSelectionScope) { AddMember(OPTION_NAME_UseSelectionScope, rapidjson::Value(useSelectionScope)); }
        };

private:
    Utf8String m_rulesetId;
    bool       m_useSelectionScope;

protected:
    //! Creates rules-driven presentation manager specific content request options
    //! using the supplied selection change event.
    ECPRESENTATION_EXPORT Json::Value _CreateContentOptionsForSelection(SelectionChangedEventCR) const override;

    //! Creates rules-driven presentation manager specific selection event extended
    //! data.
    ECPRESENTATION_EXPORT rapidjson::Document _CreateSelectionEventExtendedData() const override;

protected:
    //! Constructor.
    //! @param[in] rulesetId The ID of the default ruleset to use when requesting for
    //! content in case the selection event doesn't specify a ruleset.
    RulesDrivenSelectionSyncHandler(Utf8CP rulesetId) : m_rulesetId(rulesetId) {}

    //! Sets the default ruleset ID.
    void SetRulesetId(Utf8CP rulesetId) {m_rulesetId = rulesetId;}

    //! Get default ruleset ID.
    Utf8StringCR GetRulesetId() const {return m_rulesetId;}

    void SetUseSelectionScope(bool useSelectionScope) { m_useSelectionScope = useSelectionScope; }
    bool GetUseSelectionScope() const { return m_useSelectionScope; }
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
