/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/RulesDriven/PresentationManager.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define DISPLAY_LABEL_RULESET_ID    "RulesDrivenECPresentationManager_RulesetId_DisplayLabel"

struct NodesCache;
struct ContentCache;
struct ContentProviderKey;
struct JsonNavNodesFactory;
struct CustomFunctionsInjector;
typedef RefCountedPtr<struct INavNodesDataSource> INavNodesDataSourcePtr;
typedef RefCountedPtr<struct SpecificationContentProvider> SpecificationContentProviderPtr;
typedef RefCountedPtr<struct SpecificationContentProvider const> SpecificationContentProviderCPtr;

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                11/2017
//=======================================================================================
struct IRulesDrivenECPresentationManagerDependenciesFactory
    {
    virtual ~IRulesDrivenECPresentationManagerDependenciesFactory() {}
    virtual IRulesetLocaterManager* _CreateRulesetLocaterManager(IConnectionManagerCR) const = 0;
    virtual IUserSettingsManager* _CreateUserSettingsManager(BeFileNameCR temporaryDirectory) const = 0;
    };

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                11/2017
//=======================================================================================
struct RulesDrivenECPresentationManagerDependenciesFactory : IRulesDrivenECPresentationManagerDependenciesFactory
    {
    ECPRESENTATION_EXPORT virtual IRulesetLocaterManager* _CreateRulesetLocaterManager(IConnectionManagerCR) const override;
    ECPRESENTATION_EXPORT virtual IUserSettingsManager* _CreateUserSettingsManager(BeFileNameCR) const override;
    };

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                11/2017
//=======================================================================================
struct RulesDrivenECPresentationManager::Impl
{
    typedef RulesDrivenECPresentationManager::Paths Paths;
    typedef RulesDrivenECPresentationManager::Params Params;
    typedef RulesDrivenECPresentationManager::NavigationOptions NavigationOptions;
    typedef RulesDrivenECPresentationManager::ContentOptions ContentOptions;
    struct CompositeUpdateRecordsHandler;

private:
    IRulesetLocaterManager* m_locaters;
    IUserSettingsManager* m_userSettings;
    IJsonLocalState* m_localState;
    IECPropertyFormatter const* m_ecPropertyFormatter;
    IPropertyCategorySupplier* m_categorySupplier;
    ILocalizationProvider const* m_localizationProvider;
    bset<IECInstanceChangeHandlerPtr, IECInstanceChangeHandlerPtrComparer> m_ecInstanceChangeHandlers;
    bvector<ECInstanceChangeEventSourcePtr> m_ecInstanceChangeEventSources;
    CompositeUpdateRecordsHandler* m_compositeUpdateRecordsHandler;

protected:
/** @name Rules */
    virtual IRulesPreprocessorPtr _GetRulesPreprocessor(IConnectionCR connection, Utf8StringCR rulesetId, Utf8StringCR locale, IUsedUserSettingsListener* usedSettingsListener) const = 0;
/** @} */

/** @name IECPresentationManager: Navigation */
    virtual INavNodesDataSourcePtr _GetRootNodes(IConnectionCR, PageOptionsCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual size_t _GetRootNodesCount(IConnectionCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual INavNodesDataSourcePtr _GetChildren(IConnectionCR, NavNodeCR, PageOptionsCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual size_t _GetChildrenCount(IConnectionCR, NavNodeCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual bool _HasChild(IConnectionCR, NavNodeCR, ECInstanceKeyCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual NavNodeCPtr _GetParent(IConnectionCR, NavNodeCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual NavNodeCPtr _GetNode(IConnectionCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual bvector<NavNodeCPtr> _GetFilteredNodes(IConnectionCR, Utf8CP, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual void _OnNodeChecked(IConnectionCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual void _OnNodeUnchecked(IConnectionCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual void _OnNodeExpanded(IConnectionCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual void _OnNodeCollapsed(IConnectionCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual void _OnAllNodesCollapsed(IConnectionCR, NavigationOptions const&, ICancelationTokenCR) = 0;
/** @} */

/** @name IECPresentationManager: Content */
    virtual bvector<SelectClassInfo> _GetContentClasses(IConnectionCR, Utf8CP, bvector<ECClassCP> const&, ContentOptions const&, ICancelationTokenCR) = 0;
    virtual ContentDescriptorCPtr _GetContentDescriptor(IConnectionCR, Utf8CP, KeySetCR, SelectionInfo const*, ContentOptions const&, ICancelationTokenCR) = 0;
    virtual ContentCPtr _GetContent(ContentDescriptorCR, PageOptionsCR, ICancelationTokenCR) = 0;
    virtual size_t _GetContentSetSize(ContentDescriptorCR, ICancelationTokenCR) = 0;
    virtual Utf8String _GetDisplayLabel(IConnectionCR, KeySetCR, ICancelationTokenCR) = 0;
/** @} */

/** @name IECPresentationManager: Updating */
    virtual bvector<ECInstanceChangeResult> _SaveValueChange(IConnectionCR, bvector<ChangedECInstanceInfo> const&, Utf8CP, ECValueCR) = 0;
/**/

/** @name Misc callbacks */
    virtual void _OnUpdateRecordsHandlerChanged() {}
    virtual void _OnECInstanceChangeEventSourceRegistered(ECInstanceChangeEventSource&) {}
    virtual void _OnECInstanceChangeEventSourceUnregister(ECInstanceChangeEventSource&) {}
    virtual void _OnCategoriesChanged() {}
/**/

public:
    ECPRESENTATION_EXPORT Impl(IRulesDrivenECPresentationManagerDependenciesFactory const&, Params const&);
    ECPRESENTATION_EXPORT virtual ~Impl();

    IUserSettingsManager& GetUserSettingsManager() const {return *m_userSettings;}
    bvector<ECInstanceChangeEventSourcePtr> const& GetECInstanceChangeEventSources() const {return m_ecInstanceChangeEventSources;}
    bset<IECInstanceChangeHandlerPtr, IECInstanceChangeHandlerPtrComparer> const& GetECInstanceChangeHandlers() const {return m_ecInstanceChangeHandlers;}
    ECPRESENTATION_EXPORT void SetLocalizationProvider(ILocalizationProvider const* provider);
    ILocalizationProvider const* GetLocalizationProvider() const { return m_localizationProvider; }

/** @name General */
/** @{ */
    IRulesetLocaterManager& GetLocaters() const {return *m_locaters;}
    IUserSettings& GetUserSettings(Utf8CP rulesetId) const {return m_userSettings->GetSettings(rulesetId);}
    ECPRESENTATION_EXPORT void SetLocalState(IJsonLocalState* localState);
    IJsonLocalState* GetLocalState() const {return m_localState;}
/** @} */

/** @name Rules */
/** @{ */
    IRulesPreprocessorPtr GetRulesPreprocessor(IConnectionCR connection, Utf8StringCR rulesetId, Utf8StringCR locale, IUsedUserSettingsListener* usedSettingsListener) const {return _GetRulesPreprocessor(connection, rulesetId, locale, usedSettingsListener);}
/** @} */

/** @name Property Formatting */
/** @{ */
    void SetECPropertyFormatter(IECPropertyFormatter const* formatter) {m_ecPropertyFormatter = formatter;}
    ECPRESENTATION_EXPORT IECPropertyFormatter const& GetECPropertyFormatter() const;
/** @} */

/** @name Property Categories */
/** @{ */
    void SetCategorySupplier(IPropertyCategorySupplier* supplier) {m_categorySupplier = supplier;}
    ECPRESENTATION_EXPORT IPropertyCategorySupplier& GetCategorySupplier() const;
    void NotifyCategoriesChanged() {_OnCategoriesChanged();}
/** @} */

/** @name Changing ECInstances */
/** @{ */
    void RegisterECInstanceChangeHandler(IECInstanceChangeHandler& handler) {m_ecInstanceChangeHandlers.insert(&handler);}
    void UnregisterECInstanceChangeHandler(IECInstanceChangeHandler& handler) {m_ecInstanceChangeHandlers.erase(&handler);}
/** @} */

/** @name Notifying about ECInstance Changes */
/** @{ */
    ECPRESENTATION_EXPORT void RegisterECInstanceChangeEventSource(ECInstanceChangeEventSource&);
    ECPRESENTATION_EXPORT void UnregisterECInstanceChangeEventSource(ECInstanceChangeEventSource&);
/** @} */

/** @name Reacting to Content / Hierarchy Changes */
/** @{ */
    ECPRESENTATION_EXPORT void RegisterUpdateRecordsHandler(IUpdateRecordsHandler&);
    ECPRESENTATION_EXPORT void UnregisterUpdateRecordsHandler(IUpdateRecordsHandler&);
    ECPRESENTATION_EXPORT IUpdateRecordsHandler& GetCompositeUpdateRecordsHandler() const;
/** @} */

/** @name IECPresentationManager: Navigation */
    INavNodesDataSourcePtr GetRootNodes(IConnectionCR connection, PageOptionsCR pageOptions, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetRootNodes(connection, pageOptions, options, cancelationToken);}
    size_t GetRootNodesCount(IConnectionCR connection, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetRootNodesCount(connection, options, cancelationToken);}
    INavNodesDataSourcePtr GetChildren(IConnectionCR connection, NavNodeCR parentNode, PageOptionsCR pageOptions, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetChildren(connection, parentNode, pageOptions, options, cancelationToken);}
    size_t GetChildrenCount(IConnectionCR connection, NavNodeCR parentNode, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetChildrenCount(connection, parentNode, options, cancelationToken);}
    bool HasChild(IConnectionCR connection, NavNodeCR parentNode, ECInstanceKeyCR childKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _HasChild(connection, parentNode, childKey, options, cancelationToken);}
    NavNodeCPtr GetParent(IConnectionCR connection, NavNodeCR childNode, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetParent(connection, childNode, options, cancelationToken);}
    NavNodeCPtr GetNode(IConnectionCR connection, NavNodeKeyCR nodeKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetNode(connection, nodeKey, options, cancelationToken);}
    bvector<NavNodeCPtr> GetFilteredNodes(IConnectionCR connection, Utf8CP filterText, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetFilteredNodes(connection, filterText, options, cancelationToken);}
    void NotifyNodeChecked(IConnectionCR connection, NavNodeKeyCR nodeKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {_OnNodeChecked(connection, nodeKey, options, cancelationToken);}
    void NotifyNodeUnchecked(IConnectionCR connection, NavNodeKeyCR nodeKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {_OnNodeUnchecked(connection, nodeKey, options, cancelationToken);}
    void NotifyNodeExpanded(IConnectionCR connection, NavNodeKeyCR nodeKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {_OnNodeExpanded(connection, nodeKey, options, cancelationToken);}
    void NotifyNodeCollapsed(IConnectionCR connection, NavNodeKeyCR nodeKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {_OnNodeCollapsed(connection, nodeKey, options, cancelationToken);}
    void NotifyAllNodesCollapsed(IConnectionCR connection, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {_OnAllNodesCollapsed(connection, options, cancelationToken);}
/** @} */

/** @name IECPresentationManager: Content */
    bvector<SelectClassInfo> GetContentClasses(IConnectionCR connection, Utf8CP preferredDisplayType, bvector<ECClassCP> const& inputClasses, ContentOptions const& options, ICancelationTokenCR cancelationToken) {return _GetContentClasses(connection, preferredDisplayType, inputClasses, options, cancelationToken);}
    ContentDescriptorCPtr GetContentDescriptor(IConnectionCR connection, Utf8CP preferredDisplayType, KeySetCR inputKeys, SelectionInfo const* selectionInfo, ContentOptions const& options, ICancelationTokenCR cancelationToken) {return _GetContentDescriptor(connection, preferredDisplayType, inputKeys, selectionInfo, options, cancelationToken);}
    ContentCPtr GetContent(ContentDescriptorCR descriptor, PageOptionsCR pageOptions, ICancelationTokenCR cancelationToken) {return _GetContent(descriptor, pageOptions, cancelationToken);}
    size_t GetContentSetSize(ContentDescriptorCR descriptor, ICancelationTokenCR cancelationToken) {return _GetContentSetSize(descriptor, cancelationToken);}
    Utf8String GetDisplayLabel(IConnectionCR connection, KeySetCR keys, ICancelationTokenCR cancelationToken) {return _GetDisplayLabel(connection, keys, cancelationToken);}
/** @} */

/** @name IECPresentationManager: Updating */
    bvector<ECInstanceChangeResult> SaveValueChange(IConnectionCR connection, bvector<ChangedECInstanceInfo> const& instanceInfos, Utf8CP propertyAccessor, ECValueCR value) {return _SaveValueChange(connection, instanceInfos, propertyAccessor, value);}
/**/
};

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
struct RulesDrivenECPresentationManagerImpl : RulesDrivenECPresentationManager::Impl, ECInstanceChangeEventSource::IEventHandler,
    IRulesetCallbacksHandler, IUserSettingsChangeListener, IConnectionsListener
{
    struct ECDbCaches;
    struct RulesetECExpressionsCache;
    struct UsedClassesListener;
    struct NodesProviderContextFactory;
    struct NodesProviderFactory;

private:
    IConnectionManagerCR m_connections;
    JsonNavNodesFactory const* m_nodesFactory;
    NodesProviderContextFactory const* m_nodesProviderContextFactory;
    NodesProviderFactory const* m_nodesProviderFactory;
    CustomFunctionsInjector* m_customFunctions;
    mutable NodesCache* m_nodesCache;
    mutable ContentCache* m_contentCache;
    ECDbCaches* m_ecdbCaches;
    RulesetECExpressionsCache* m_rulesetECExpressionsCache;
    UpdateHandler* m_updateHandler;
    UsedClassesListener* m_usedClassesListener;
    bmap<Utf8String, bvector<RuleSetLocaterPtr>> m_embeddedRuleSetLocaters;

private:
    INavNodesDataSourcePtr GetCachedDataSource(IConnectionCR, ICancelationTokenCR, NavigationOptions const&);
    INavNodesDataSourcePtr GetCachedDataSource(IConnectionCR, ICancelationTokenCR, NavNodeCR parent, NavigationOptions const&);
    SpecificationContentProviderCPtr GetContentProvider(IConnectionCR, ICancelationTokenCR, ContentProviderKey const&, INavNodeKeysContainerCR, SelectionInfo const*, ContentOptions const&);
    SpecificationContentProviderPtr GetContentProvider(IConnectionCR, ICancelationTokenCR, ContentDescriptorCR, INavNodeKeysContainerCR, SelectionInfo const*, ContentOptions const&);

protected:
    // RulesDrivenECPresentationManager::Impl: Rules
    IRulesPreprocessorPtr _GetRulesPreprocessor(IConnectionCR connection, Utf8StringCR rulesetId, Utf8StringCR locale, IUsedUserSettingsListener* usedSettingsListener) const override;

    // IRulesetCallbacksHandler
    ECPRESENTATION_EXPORT void _OnRulesetDispose(RuleSetLocaterCR, PresentationRuleSetCR) override;
    ECPRESENTATION_EXPORT void _OnRulesetCreated(RuleSetLocaterCR, PresentationRuleSetR) override;

    // IUserSettingsChangeListener
    ECPRESENTATION_EXPORT void _OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const override;

    // ECInstanceChangeEventSource::IEventHandler
    ECPRESENTATION_EXPORT void _OnECInstancesChanged(ECDbCR, bvector<ECInstanceChangeEventSource::ChangedECInstance>) override;

    // IConnectionListener
    ECPRESENTATION_EXPORT void _OnConnectionEvent(ConnectionEvent const&) override;

    // RulesDrivenECPresentationManager::Impl: Navigation
    ECPRESENTATION_EXPORT INavNodesDataSourcePtr _GetRootNodes(IConnectionCR, PageOptionsCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT size_t _GetRootNodesCount(IConnectionCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT INavNodesDataSourcePtr _GetChildren(IConnectionCR, NavNodeCR, PageOptionsCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT size_t _GetChildrenCount(IConnectionCR, NavNodeCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT bool _HasChild(IConnectionCR, NavNodeCR, ECInstanceKeyCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT NavNodeCPtr _GetParent(IConnectionCR, NavNodeCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT NavNodeCPtr _GetNode(IConnectionCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT bvector<NavNodeCPtr> _GetFilteredNodes(IConnectionCR, Utf8CP, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT void _OnNodeChecked(IConnectionCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT void _OnNodeUnchecked(IConnectionCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT void _OnNodeExpanded(IConnectionCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT void _OnNodeCollapsed(IConnectionCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT void _OnAllNodesCollapsed(IConnectionCR, NavigationOptions const&, ICancelationTokenCR) override;

    // RulesDrivenECPresentationManager::Impl: Content
    ECPRESENTATION_EXPORT bvector<SelectClassInfo> _GetContentClasses(IConnectionCR, Utf8CP, bvector<ECClassCP> const&, ContentOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT ContentDescriptorCPtr _GetContentDescriptor(IConnectionCR, Utf8CP, KeySetCR, SelectionInfo const*, ContentOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT ContentCPtr _GetContent(ContentDescriptorCR, PageOptionsCR, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT size_t _GetContentSetSize(ContentDescriptorCR, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT Utf8String _GetDisplayLabel(IConnectionCR, KeySetCR, ICancelationTokenCR) override;

    // RulesDrivenECPresentationManager::Impl: Updating
    ECPRESENTATION_EXPORT bvector<ECInstanceChangeResult> _SaveValueChange(IConnectionCR, bvector<ChangedECInstanceInfo> const&, Utf8CP, ECValueCR) override;

    // RulesDrivenECPresentationManager::Impl: Misc
    ECPRESENTATION_EXPORT void _OnECInstanceChangeEventSourceRegistered(ECInstanceChangeEventSource&) override;
    ECPRESENTATION_EXPORT void _OnECInstanceChangeEventSourceUnregister(ECInstanceChangeEventSource&) override;
    ECPRESENTATION_EXPORT void _OnUpdateRecordsHandlerChanged() override;
    ECPRESENTATION_EXPORT void _OnCategoriesChanged() override;

public:
    ECPRESENTATION_EXPORT RulesDrivenECPresentationManagerImpl(IRulesDrivenECPresentationManagerDependenciesFactory const&, Params const&);
    ECPRESENTATION_EXPORT ~RulesDrivenECPresentationManagerImpl();
    NodesCache& GetNodesCache() {return *m_nodesCache;}
    UpdateHandler& GetUpdateHandler() const {return *m_updateHandler;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
