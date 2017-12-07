/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/PresentationManagerImpl.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/RulesDriven/PresentationManager.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

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
    typedef RulesDrivenECPresentationManager::NavigationOptions NavigationOptions;
    typedef RulesDrivenECPresentationManager::ContentOptions ContentOptions;

private:
    IRulesetLocaterManager* m_locaters;
    IUserSettingsManager* m_userSettings;
    IJsonLocalState* m_localState;
    ISelectionManager* m_selectionManager;
    ILocalizationProvider const* m_localizationProvider;
    IECPropertyFormatter const* m_ecPropertyFormatter;
    IPropertyCategorySupplier* m_categorySupplier;
    bset<IECInstanceChangeHandlerPtr, IECInstanceChangeHandlerPtrComparer> m_ecInstanceChangeHandlers;
    bvector<ECInstanceChangeEventSourcePtr> m_ecInstanceChangeEventSources;
    RefCountedPtr<IUpdateRecordsHandler> m_updateRecordsHandler;

protected:
/** @name IECPresentationManager: Navigation */
    virtual INavNodesDataSourcePtr _GetRootNodes(IConnectionCR, PageOptionsCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual size_t _GetRootNodesCount(IConnectionCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual INavNodesDataSourcePtr _GetChildren(IConnectionCR, NavNodeCR, PageOptionsCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual size_t _GetChildrenCount(IConnectionCR, NavNodeCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual bool _HasChild(IConnectionCR, NavNodeCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual NavNodeCPtr _GetParent(IConnectionCR, NavNodeCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual NavNodeCPtr _GetNode(IConnectionCR, uint64_t nodeId, ICancelationTokenCR) = 0;
    virtual bvector<NavNodeCPtr> _GetFilteredNodes(IConnectionCR, Utf8CP, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual void _OnNodeChecked(IConnectionCR, uint64_t nodeId, ICancelationTokenCR) = 0;
    virtual void _OnNodeUnchecked(IConnectionCR, uint64_t nodeId, ICancelationTokenCR) = 0;
    virtual void _OnNodeExpanded(IConnectionCR, uint64_t nodeId, ICancelationTokenCR) = 0;
    virtual void _OnNodeCollapsed(IConnectionCR, uint64_t nodeId, ICancelationTokenCR) = 0;
    virtual void _OnAllNodesCollapsed(IConnectionCR, NavigationOptions const&, ICancelationTokenCR) = 0;
/** @} */

/** @name IECPresentationManager: Content */
    virtual bvector<SelectClassInfo> _GetContentClasses(IConnectionCR, Utf8CP, bvector<ECClassCP> const&, ContentOptions const&, ICancelationTokenCR) = 0;
    virtual ContentDescriptorCPtr _GetContentDescriptor(IConnectionCR, Utf8CP, SelectionInfo const&, ContentOptions const&, ICancelationTokenCR) = 0;
    virtual ContentCPtr _GetContent(IConnectionCR, ContentDescriptorCR, SelectionInfo const&, PageOptionsCR, ContentOptions const&, ICancelationTokenCR) = 0;
    virtual size_t _GetContentSetSize(IConnectionCR, ContentDescriptorCR, SelectionInfo const&, ContentOptions const&, ICancelationTokenCR) = 0;
/** @} */

/** @name IECPresentationManager: Updating */
    virtual bvector<ECInstanceChangeResult> _SaveValueChange(IConnectionCR, bvector<ChangedECInstanceInfo> const&, Utf8CP, ECValueCR) = 0;
/**/

/** @name Misc callbacks */
    virtual void _OnUpdateRecordsHandlerChanged() {}
    virtual void _OnECInstanceChangeEventSourceRegistered(ECInstanceChangeEventSource&) {}
    virtual void _OnECInstanceChangeEventSourceUnregister(ECInstanceChangeEventSource&) {}
    virtual void _OnCategoriesChanged() {}
    virtual void _OnSelectionManagerChanged(ISelectionManager* before, ISelectionManager* after) {}
/**/

public:
    ECPRESENTATION_EXPORT Impl(IRulesDrivenECPresentationManagerDependenciesFactory const&, IConnectionManagerCR, Paths const&);
    ECPRESENTATION_EXPORT virtual ~Impl();

    IUserSettingsManager& GetUserSettingsManager() const {return *m_userSettings;}
    bvector<ECInstanceChangeEventSourcePtr> const& GetECInstanceChangeEventSources() const {return m_ecInstanceChangeEventSources;}
    bset<IECInstanceChangeHandlerPtr, IECInstanceChangeHandlerPtrComparer> const& GetECInstanceChangeHandlers() const {return m_ecInstanceChangeHandlers;}

/** @name General */
/** @{ */
    IRulesetLocaterManager& GetLocaters() const {return *m_locaters;}
    IUserSettings& GetUserSettings(Utf8CP rulesetId) const {return m_userSettings->GetSettings(rulesetId);}
    ECPRESENTATION_EXPORT void SetLocalState(IJsonLocalState* localState);
    IJsonLocalState* GetLocalState() const {return m_localState;}    
    ECPRESENTATION_EXPORT void SetSelectionManager(ISelectionManager*);
    ISelectionManager* GetSelectionManager() const {return m_selectionManager;}
/** @} */

/** @name Localization */
/** @{ */
    ECPRESENTATION_EXPORT void SetLocalizationProvider(ILocalizationProvider const*);
    ECPRESENTATION_EXPORT ILocalizationProvider const& GetLocalizationProvider() const;
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

/** @name Reacting to ECInstance Changes */
/** @{ */
    ECPRESENTATION_EXPORT void RegisterECInstanceChangeEventSource(ECInstanceChangeEventSource&);
    ECPRESENTATION_EXPORT void UnregisterECInstanceChangeEventSource(ECInstanceChangeEventSource&);
    void SetUpdateRecordsHandler(IUpdateRecordsHandler* handler) {m_updateRecordsHandler = handler; _OnUpdateRecordsHandlerChanged();}
    IUpdateRecordsHandler* GetUpdateRecordsHandler() const {return m_updateRecordsHandler.get();}
/** @} */

/** @name IECPresentationManager: Navigation */
    INavNodesDataSourcePtr GetRootNodes(IConnectionCR connection, PageOptionsCR pageOptions, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetRootNodes(connection, pageOptions, options, cancelationToken);}
    size_t GetRootNodesCount(IConnectionCR connection, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetRootNodesCount(connection, options, cancelationToken);}
    INavNodesDataSourcePtr GetChildren(IConnectionCR connection, NavNodeCR parentNode, PageOptionsCR pageOptions, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetChildren(connection, parentNode, pageOptions, options, cancelationToken);}
    size_t GetChildrenCount(IConnectionCR connection, NavNodeCR parentNode, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetChildrenCount(connection, parentNode, options, cancelationToken);}
    bool HasChild(IConnectionCR connection, NavNodeCR parentNode, NavNodeKeyCR childNodeKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _HasChild(connection, parentNode, childNodeKey, options, cancelationToken);}
    NavNodeCPtr GetParent(IConnectionCR connection, NavNodeCR childNode, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetParent(connection, childNode, options, cancelationToken);}
    NavNodeCPtr GetNode(IConnectionCR connection, uint64_t nodeId, ICancelationTokenCR cancelationToken) {return _GetNode(connection, nodeId, cancelationToken);}
    bvector<NavNodeCPtr> GetFilteredNodes(IConnectionCR connection, Utf8CP filterText, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetFilteredNodes(connection, filterText, options, cancelationToken);}
    void NotifyNodeChecked(IConnectionCR connection, uint64_t nodeId, ICancelationTokenCR cancelationToken) {_OnNodeChecked(connection, nodeId, cancelationToken);}
    void NotifyNodeUnchecked(IConnectionCR connection, uint64_t nodeId, ICancelationTokenCR cancelationToken) {_OnNodeUnchecked(connection, nodeId, cancelationToken);}
    void NotifyNodeExpanded(IConnectionCR connection, uint64_t nodeId, ICancelationTokenCR cancelationToken) {_OnNodeExpanded(connection, nodeId, cancelationToken);}
    void NotifyNodeCollapsed(IConnectionCR connection, uint64_t nodeId, ICancelationTokenCR cancelationToken) {_OnNodeCollapsed(connection, nodeId, cancelationToken);}
    void NotifyAllNodesCollapsed(IConnectionCR connection, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {_OnAllNodesCollapsed(connection, options, cancelationToken);}
/** @} */

/** @name IECPresentationManager: Content */
    bvector<SelectClassInfo> GetContentClasses(IConnectionCR connection, Utf8CP preferredDisplayType, bvector<ECClassCP> const& inputClasses, ContentOptions const& options, ICancelationTokenCR cancelationToken) {return _GetContentClasses(connection, preferredDisplayType, inputClasses, options, cancelationToken);}
    ContentDescriptorCPtr GetContentDescriptor(IConnectionCR connection, Utf8CP preferredDisplayType, SelectionInfo const& selectionInfo, ContentOptions const& options, ICancelationTokenCR cancelationToken) {return _GetContentDescriptor(connection, preferredDisplayType, selectionInfo, options, cancelationToken);}
    ContentCPtr GetContent(IConnectionCR connection, ContentDescriptorCR descriptor, SelectionInfo const& selectionInfo, PageOptionsCR pageOptions, ContentOptions const& options, ICancelationTokenCR cancelationToken) {return _GetContent(connection, descriptor, selectionInfo, pageOptions, options, cancelationToken);}
    size_t GetContentSetSize(IConnectionCR connection, ContentDescriptorCR descriptor, SelectionInfo const& selectionInfo, ContentOptions const& options, ICancelationTokenCR cancelationToken) {return _GetContentSetSize(connection, descriptor, selectionInfo, options, cancelationToken);}
/** @} */

/** @name IECPresentationManager: Updating */
    bvector<ECInstanceChangeResult> SaveValueChange(IConnectionCR connection, bvector<ChangedECInstanceInfo> const& instanceInfos, Utf8CP propertyAccessor, ECValueCR value) {return _SaveValueChange(connection, instanceInfos, propertyAccessor, value);}
/**/
};

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
struct RulesDrivenECPresentationManagerImpl : RulesDrivenECPresentationManager::Impl, ECInstanceChangeEventSource::IEventHandler, 
    ISelectionChangesListener, IRulesetCallbacksHandler, IUserSettingsChangeListener, IConnectionsListener
{
    struct ECDbStatementsCache;
    struct ECDbRelatedPathsCache;
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
    ECDbStatementsCache* m_statementCache;
    ECDbRelatedPathsCache* m_relatedPathsCache;
    RulesetECExpressionsCache* m_rulesetECExpressionsCache;    
    UpdateHandler* m_updateHandler;
    UsedClassesListener* m_usedClassesListener;
    bmap<Utf8String, RuleSetLocaterPtr> m_embeddedRuleSetLocaters;
    
private:
    INavNodesDataSourcePtr GetCachedDataSource(IConnectionCR, ICancelationTokenCR, NavigationOptions const&);
    INavNodesDataSourcePtr GetCachedDataSource(IConnectionCR, ICancelationTokenCR, NavNodeCR parent, NavigationOptions const&);
    SpecificationContentProviderCPtr GetContentProvider(IConnectionCR, ICancelationTokenCR, ContentProviderKey const&, SelectionInfo const&, ContentOptions const&);
    SpecificationContentProviderPtr GetContentProvider(IConnectionCR, ICancelationTokenCR, ContentDescriptorCR, SelectionInfo const&, ContentOptions const&);
        
protected:
    // IRulesetCallbacksHandler
    ECPRESENTATION_EXPORT void _OnRulesetDispose(PresentationRuleSetCR) override;
    ECPRESENTATION_EXPORT void _OnRulesetCreated(PresentationRuleSetCR) override;

    // ISelectionChangesListener
    ECPRESENTATION_EXPORT void _OnSelectionChanged(SelectionChangedEventCR) override;

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
    ECPRESENTATION_EXPORT bool _HasChild(IConnectionCR, NavNodeCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT NavNodeCPtr _GetParent(IConnectionCR, NavNodeCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT NavNodeCPtr _GetNode(IConnectionCR, uint64_t nodeId, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT bvector<NavNodeCPtr> _GetFilteredNodes(IConnectionCR, Utf8CP, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT void _OnNodeChecked(IConnectionCR, uint64_t nodeId, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT void _OnNodeUnchecked(IConnectionCR, uint64_t nodeId, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT void _OnNodeExpanded(IConnectionCR, uint64_t nodeId, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT void _OnNodeCollapsed(IConnectionCR, uint64_t nodeId, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT void _OnAllNodesCollapsed(IConnectionCR, NavigationOptions const&, ICancelationTokenCR) override;

    // RulesDrivenECPresentationManager::Impl: Content
    ECPRESENTATION_EXPORT bvector<SelectClassInfo> _GetContentClasses(IConnectionCR, Utf8CP, bvector<ECClassCP> const&, ContentOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT ContentDescriptorCPtr _GetContentDescriptor(IConnectionCR, Utf8CP, SelectionInfo const&, ContentOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT ContentCPtr _GetContent(IConnectionCR, ContentDescriptorCR, SelectionInfo const&, PageOptionsCR, ContentOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT size_t _GetContentSetSize(IConnectionCR, ContentDescriptorCR, SelectionInfo const&, ContentOptions const&, ICancelationTokenCR) override;

    // RulesDrivenECPresentationManager::Impl: Updating
    ECPRESENTATION_EXPORT bvector<ECInstanceChangeResult> _SaveValueChange(IConnectionCR, bvector<ChangedECInstanceInfo> const&, Utf8CP, ECValueCR) override;
    
    // RulesDrivenECPresentationManager::Impl: Misc
    ECPRESENTATION_EXPORT void _OnECInstanceChangeEventSourceRegistered(ECInstanceChangeEventSource&) override;
    ECPRESENTATION_EXPORT void _OnECInstanceChangeEventSourceUnregister(ECInstanceChangeEventSource&) override;    
    ECPRESENTATION_EXPORT void _OnUpdateRecordsHandlerChanged() override;
    ECPRESENTATION_EXPORT void _OnCategoriesChanged() override;
    ECPRESENTATION_EXPORT void _OnSelectionManagerChanged(ISelectionManager* before, ISelectionManager* after) override;

public:
    ECPRESENTATION_EXPORT RulesDrivenECPresentationManagerImpl(IRulesDrivenECPresentationManagerDependenciesFactory const&, IConnectionManagerCR, Paths const&, bool disableDiskCache = false);
    ECPRESENTATION_EXPORT ~RulesDrivenECPresentationManagerImpl();
    NodesCache& GetNodesCache() {return *m_nodesCache;}
    UpdateHandler& GetUpdateHandler() const {return *m_updateHandler;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
