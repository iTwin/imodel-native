/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
struct RulesDrivenECPresentationManager::Impl
{
    typedef RulesDrivenECPresentationManager::Mode Mode;
    typedef RulesDrivenECPresentationManager::Paths Paths;
    typedef RulesDrivenECPresentationManager::Params Params;
    typedef RulesDrivenECPresentationManager::NavigationOptions NavigationOptions;
    typedef RulesDrivenECPresentationManager::ContentOptions ContentOptions;
    
protected:
/** @name Misc */
    virtual IRulesPreprocessorPtr _GetRulesPreprocessor(IConnectionCR connection, Utf8StringCR rulesetId, Utf8StringCR locale, IUsedUserSettingsListener* usedSettingsListener) const = 0;
    virtual IPropertyCategorySupplier const& _GetCategorySupplier() const = 0;
    virtual IECPropertyFormatter const& _GetECPropertyFormatter() const = 0;
    virtual IUserSettingsManager& _GetUserSettingsManager() const = 0;
    virtual bvector<std::shared_ptr<ECInstanceChangeEventSource>> const& _GetECInstanceChangeEventSources() const = 0;
    virtual ILocalizationProvider const* _GetLocalizationProvider() const = 0;
    virtual IJsonLocalState* _GetLocalState() const = 0;
    virtual IRulesetLocaterManager& _GetLocaters() const = 0;
    virtual IConnectionManagerR _GetConnections() = 0;
/** @} */

/** @name IECPresentationManager: Navigation */
    virtual INavNodesDataSourcePtr _GetRootNodes(IConnectionCR, PageOptionsCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual size_t _GetRootNodesCount(IConnectionCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual INavNodesDataSourcePtr _GetChildren(IConnectionCR, NavNodeCR, PageOptionsCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual size_t _GetChildrenCount(IConnectionCR, NavNodeCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual NavNodeCPtr _GetParent(IConnectionCR, NavNodeCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual NavNodeCPtr _GetNode(IConnectionCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR) = 0;
    virtual bvector<NavNodeCPtr> _GetFilteredNodes(IConnectionCR, Utf8CP, NavigationOptions const&, ICancelationTokenCR) = 0;
/** @} */

/** @name IECPresentationManager: Content */
    virtual bvector<SelectClassInfo> _GetContentClasses(IConnectionCR, Utf8CP, int, bvector<ECClassCP> const&, ContentOptions const&, ICancelationTokenCR) = 0;
    virtual ContentDescriptorCPtr _GetContentDescriptor(IConnectionCR, Utf8CP, int, KeySetCR, SelectionInfo const*, ContentOptions const&, ICancelationTokenCR) = 0;
    virtual ContentCPtr _GetContent(IConnectionCR, ContentDescriptorCR, PageOptionsCR, ICancelationTokenCR) = 0;
    virtual size_t _GetContentSetSize(IConnectionCR, ContentDescriptorCR, ICancelationTokenCR) = 0;
    virtual LabelDefinitionCPtr _GetDisplayLabel(IConnectionCR, KeySetCR, ICancelationTokenCR) = 0;
/** @} */

public:
/** @name General */
/** @{ */
    virtual ~Impl() {}
    IUserSettingsManager& GetUserSettingsManager() const {return _GetUserSettingsManager();}
    IUserSettings& GetUserSettings(Utf8CP rulesetId) const {return GetUserSettingsManager().GetSettings(rulesetId);}
    bvector<std::shared_ptr<ECInstanceChangeEventSource>> const& GetECInstanceChangeEventSources() const {return _GetECInstanceChangeEventSources();}
    ILocalizationProvider const* GetLocalizationProvider() const {return _GetLocalizationProvider();}
    IRulesPreprocessorPtr GetRulesPreprocessor(IConnectionCR connection, Utf8StringCR rulesetId, Utf8StringCR locale, IUsedUserSettingsListener* usedSettingsListener) const {return _GetRulesPreprocessor(connection, rulesetId, locale, usedSettingsListener);}
    IPropertyCategorySupplier const& GetCategorySupplier() const {return _GetCategorySupplier();}
    IECPropertyFormatter const& GetECPropertyFormatter() const {return _GetECPropertyFormatter();}
    IJsonLocalState* GetLocalState() const {return _GetLocalState();}
    IRulesetLocaterManager& GetLocaters() const {return _GetLocaters();}
    IConnectionManagerR GetConnections() {return _GetConnections();}
/** @} */

/** @name IECPresentationManager: Navigation */
    INavNodesDataSourcePtr GetRootNodes(IConnectionCR connection, PageOptionsCR pageOptions, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetRootNodes(connection, pageOptions, options, cancelationToken);}
    size_t GetRootNodesCount(IConnectionCR connection, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetRootNodesCount(connection, options, cancelationToken);}
    INavNodesDataSourcePtr GetChildren(IConnectionCR connection, NavNodeCR parentNode, PageOptionsCR pageOptions, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetChildren(connection, parentNode, pageOptions, options, cancelationToken);}
    size_t GetChildrenCount(IConnectionCR connection, NavNodeCR parentNode, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetChildrenCount(connection, parentNode, options, cancelationToken);}
    NavNodeCPtr GetParent(IConnectionCR connection, NavNodeCR childNode, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetParent(connection, childNode, options, cancelationToken);}
    NavNodeCPtr GetNode(IConnectionCR connection, NavNodeKeyCR nodeKey, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetNode(connection, nodeKey, options, cancelationToken);}
    bvector<NavNodeCPtr> GetFilteredNodes(IConnectionCR connection, Utf8CP filterText, NavigationOptions const& options, ICancelationTokenCR cancelationToken) {return _GetFilteredNodes(connection, filterText, options, cancelationToken);}
/** @} */

/** @name IECPresentationManager: Content */
    bvector<SelectClassInfo> GetContentClasses(IConnectionCR connection, Utf8CP preferredDisplayType, int contentFlags, bvector<ECClassCP> const& inputClasses, ContentOptions const& options, ICancelationTokenCR cancelationToken) {return _GetContentClasses(connection, preferredDisplayType, contentFlags, inputClasses, options, cancelationToken);}
    ContentDescriptorCPtr GetContentDescriptor(IConnectionCR connection, Utf8CP preferredDisplayType, int contentFlags, KeySetCR inputKeys, SelectionInfo const* selectionInfo, ContentOptions const& options, ICancelationTokenCR cancelationToken) {return _GetContentDescriptor(connection, preferredDisplayType, contentFlags, inputKeys, selectionInfo, options, cancelationToken);}
    ContentCPtr GetContent(IConnectionCR connection, ContentDescriptorCR descriptor, PageOptionsCR pageOptions, ICancelationTokenCR cancelationToken) {return _GetContent(connection, descriptor, pageOptions, cancelationToken);}
    size_t GetContentSetSize(IConnectionCR connection, ContentDescriptorCR descriptor, ICancelationTokenCR cancelationToken) {return _GetContentSetSize(connection, descriptor, cancelationToken);}
    LabelDefinitionCPtr GetDisplayLabel(IConnectionCR connection, KeySetCR keys, ICancelationTokenCR cancelationToken) {return _GetDisplayLabel(connection, keys, cancelationToken);}
/** @} */
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
    struct NodesCacheManager;

private:
    Mode m_mode;
    IConnectionManagerR m_connections;
    JsonNavNodesFactory const* m_nodesFactory;
    NodesProviderContextFactory const* m_nodesProviderContextFactory;
    NodesProviderFactory const* m_nodesProviderFactory;
    CustomFunctionsInjector* m_customFunctions;
    NodesCacheManager* m_nodesCachesManager;
    mutable ContentCache* m_contentCache;
    ECDbCaches* m_ecdbCaches;
    RulesetECExpressionsCache* m_rulesetECExpressionsCache;
    UpdateHandler* m_updateHandler;
    UsedClassesListener* m_usedClassesListener;
    bmap<Utf8String, bvector<RuleSetLocaterPtr>> m_embeddedRuleSetLocaters;
    IRulesetLocaterManager* m_locaters;
    IUserSettingsManager* m_userSettings;
    IJsonLocalState* m_localState;
    IECPropertyFormatter const* m_ecPropertyFormatter;
    IPropertyCategorySupplier const* m_categorySupplier;
    ILocalizationProvider const* m_localizationProvider;
    bvector<std::shared_ptr<ECInstanceChangeEventSource>> m_ecInstanceChangeEventSources;
    mutable BeMutex m_mutex;

private:
    INavNodesDataSourcePtr GetCachedDataSource(IConnectionCR, ICancelationTokenCR, NavigationOptions const&, size_t pageSize = -1);
    INavNodesDataSourcePtr GetCachedDataSource(IConnectionCR, ICancelationTokenCR, NavNodeCR parent, NavigationOptions const&, size_t pageSize = -1);
    SpecificationContentProviderCPtr GetContentProvider(IConnectionCR, ICancelationTokenCR, ContentProviderKey const&, INavNodeKeysContainerCR, SelectionInfo const*, ContentOptions const&);
    SpecificationContentProviderPtr GetContentProvider(IConnectionCR, ICancelationTokenCR, ContentDescriptorCR, INavNodeKeysContainerCR, SelectionInfo const*, ContentOptions const&);

protected:
    // RulesDrivenECPresentationManager::Impl: General
    IRulesPreprocessorPtr _GetRulesPreprocessor(IConnectionCR connection, Utf8StringCR rulesetId, Utf8StringCR locale, IUsedUserSettingsListener* usedSettingsListener) const override;
    IPropertyCategorySupplier const& _GetCategorySupplier() const override;
    IECPropertyFormatter const& _GetECPropertyFormatter() const override;
    IUserSettingsManager& _GetUserSettingsManager() const override {return *m_userSettings;}
    bvector<std::shared_ptr<ECInstanceChangeEventSource>> const& _GetECInstanceChangeEventSources() const override {return m_ecInstanceChangeEventSources;}
    ILocalizationProvider const* _GetLocalizationProvider() const override {return m_localizationProvider;}
    IJsonLocalState* _GetLocalState() const override {return m_localState;}
    IRulesetLocaterManager& _GetLocaters() const override {return *m_locaters;}
    IConnectionManagerR _GetConnections() override {return m_connections;}

    // IRulesetCallbacksHandler
    ECPRESENTATION_EXPORT void _OnRulesetDispose(RuleSetLocaterCR, PresentationRuleSetR) override;
    ECPRESENTATION_EXPORT void _OnRulesetCreated(RuleSetLocaterCR, PresentationRuleSetR) override;

    // IUserSettingsChangeListener
    ECPRESENTATION_EXPORT void _OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const override;

    // ECInstanceChangeEventSource::IEventHandler
    ECPRESENTATION_EXPORT void _OnECInstancesChanged(ECDbCR, bvector<ECInstanceChangeEventSource::ChangedECInstance>) override;

    // IConnectionListener
    int _GetPriority() const override {return INT_MAX;}
    ECPRESENTATION_EXPORT void _OnConnectionEvent(ConnectionEvent const&) override;

    // RulesDrivenECPresentationManager::Impl: Navigation
    ECPRESENTATION_EXPORT INavNodesDataSourcePtr _GetRootNodes(IConnectionCR, PageOptionsCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT size_t _GetRootNodesCount(IConnectionCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT INavNodesDataSourcePtr _GetChildren(IConnectionCR, NavNodeCR, PageOptionsCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT size_t _GetChildrenCount(IConnectionCR, NavNodeCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT NavNodeCPtr _GetParent(IConnectionCR, NavNodeCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT NavNodeCPtr _GetNode(IConnectionCR, NavNodeKeyCR, NavigationOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT bvector<NavNodeCPtr> _GetFilteredNodes(IConnectionCR, Utf8CP, NavigationOptions const&, ICancelationTokenCR) override;

    // RulesDrivenECPresentationManager::Impl: Content
    ECPRESENTATION_EXPORT bvector<SelectClassInfo> _GetContentClasses(IConnectionCR, Utf8CP, int, bvector<ECClassCP> const&, ContentOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT ContentDescriptorCPtr _GetContentDescriptor(IConnectionCR, Utf8CP, int, KeySetCR, SelectionInfo const*, ContentOptions const&, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT ContentCPtr _GetContent(IConnectionCR, ContentDescriptorCR, PageOptionsCR, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT size_t _GetContentSetSize(IConnectionCR, ContentDescriptorCR, ICancelationTokenCR) override;
    ECPRESENTATION_EXPORT LabelDefinitionCPtr _GetDisplayLabel(IConnectionCR, KeySetCR, ICancelationTokenCR) override;
    
public:
    ECPRESENTATION_EXPORT RulesDrivenECPresentationManagerImpl(Params const&);
    ECPRESENTATION_EXPORT ~RulesDrivenECPresentationManagerImpl();
    ECPRESENTATION_EXPORT void Initialize();
    ECPRESENTATION_EXPORT NodesCache* GetNodesCache(IConnectionCR connection);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
