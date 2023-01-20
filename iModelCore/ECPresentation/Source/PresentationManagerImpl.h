/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationManager.h>
#include "Hierarchies/NavNodesDataSource.h"
#include "Hierarchies/NavNodeProviders.h"
#include "Hierarchies/NavNodesCache.h"
#include "Content/ContentProviders.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define DISPLAY_LABEL_RULESET_ID    "RulesDrivenECPresentationManager_RulesetId_DisplayLabel"

struct NodesCache;
struct ContentCache;
struct ContentProviderKey;
struct NavNodesFactory;
struct CustomFunctionsInjector;
struct UpdateHandler;

//=======================================================================================
// @bsiclass
//=======================================================================================
template<typename TBase>
struct ImplTaskParams : TBase
{
private:
    IConnectionCR m_connection;
    ICancelationTokenCPtr m_cancellationToken;

private:
    ImplTaskParams(TBase const& source, IConnectionCR connection, ICancelationTokenCP cancellationToken)
        : TBase(source), m_connection(connection), m_cancellationToken(cancellationToken)
        {}
    ImplTaskParams(TBase&& source, IConnectionCR connection, ICancelationTokenCP cancellationToken)
        : TBase(std::move(source)), m_connection(connection), m_cancellationToken(cancellationToken)
        {}

public:
    template<typename ...TBaseArgs>
    static ImplTaskParams<TBase> Create(IConnectionCR connection, ICancelationTokenCP cancellationToken, TBaseArgs&&... rest)
        {
        return ImplTaskParams(TBase(std::forward<TBaseArgs>(rest)...), connection, cancellationToken);
        }

    template<typename TDerivedBase>
    static ImplTaskParams<TBase> Create(ImplTaskParams<TDerivedBase> const& source)
        {
        return ImplTaskParams(TBase(source), source.GetConnection(), source.GetCancellationToken());
        }

    template<typename TImplParams>
    static ImplTaskParams<TBase> Create(TBase&& baseParams, TImplParams const& implParamsSource)
        {
        return ImplTaskParams<TBase>(baseParams, implParamsSource.GetConnection(), implParamsSource.GetCancellationToken());
        }

    IConnectionCR GetConnection() const {return m_connection;}
    void SetCancellationToken(ICancelationTokenCP value) {m_cancellationToken = value;}
    ICancelationTokenCP GetCancellationToken() const {return m_cancellationToken.get();}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct NodeInstanceKeysRequestParams : RequestWithRulesetParams
{
private:
    std::shared_ptr<InstanceFilterDefinition const> m_instanceFilter;
public:
    NodeInstanceKeysRequestParams(RequestWithRulesetParams const& rulesetParams, std::shared_ptr<InstanceFilterDefinition const> instanceFilter)
        : RequestWithRulesetParams(rulesetParams), m_instanceFilter(instanceFilter)
        {}
    NodeInstanceKeysRequestParams(RequestWithRulesetParams&& rulesetParams, std::shared_ptr<InstanceFilterDefinition const> instanceFilter)
        : RequestWithRulesetParams(std::move(rulesetParams)), m_instanceFilter(instanceFilter)
        {}
    NodeInstanceKeysRequestParams(Utf8String rulesetId, RulesetVariables rulesetVariables, std::shared_ptr<InstanceFilterDefinition const> instanceFilter)
        : RequestWithRulesetParams(rulesetId, rulesetVariables), m_instanceFilter(instanceFilter)
        {}
    std::shared_ptr<InstanceFilterDefinition const> GetInstanceFilter() const {return m_instanceFilter;}
    void SetInstanceFilter(std::shared_ptr<InstanceFilterDefinition const> value) {m_instanceFilter = value;}
};
typedef ImplTaskParams<NodeInstanceKeysRequestParams> NodeInstanceKeysRequestImplParams;

typedef ImplTaskParams<RequestWithRulesetParams> RequestWithRulesetImplParams;

typedef ImplTaskParams<HierarchyRequestParams> HierarchyRequestImplParams;
typedef ImplTaskParams<HierarchyLevelDescriptorRequestParams> HierarchyLevelDescriptorRequestImplParams;
typedef ImplTaskParams<NodeByInstanceKeyRequestParams> NodeByInstanceKeyRequestImplParams;
typedef ImplTaskParams<NodeParentRequestParams> NodeParentRequestImplParams;
typedef ImplTaskParams<NodePathFromInstanceKeyPathRequestParams> NodePathFromInstanceKeyPathRequestImplParams;
typedef ImplTaskParams<NodePathsFromInstanceKeyPathsRequestParams> NodePathsFromInstanceKeyPathsRequestImplParams;
typedef ImplTaskParams<NodePathsFromFilterTextRequestParams> NodePathsFromFilterTextRequestImplParams;
typedef ImplTaskParams<HierarchyCompareRequestParams> HierarchyCompareRequestImplParams;

typedef ImplTaskParams<ContentClassesRequestParams> ContentClassesRequestImplParams;
typedef ImplTaskParams<ContentDescriptorRequestParams> ContentDescriptorRequestImplParams;
typedef ImplTaskParams<ContentRequestParams> ContentRequestImplParams;
typedef ImplTaskParams<DistinctValuesRequestParams> DistinctValuesRequestImplParams;
typedef ImplTaskParams<KeySetDisplayLabelRequestParams> KeySetDisplayLabelRequestImplParams;

//===================================================================================
// @bsiclass
//===================================================================================
struct ECPresentationManager::ImplParams : ECPresentationManager::Params
{
private:
    std::shared_ptr<IRulesetLocaterManager> m_rulesetLocaters;
    std::shared_ptr<IUserSettingsManager> m_userSettings;
public:
    ImplParams(ECPresentationManager::Params const& other) : ECPresentationManager::Params(other) {}
    ImplParams(ImplParams const& other) : ECPresentationManager::Params(other), m_rulesetLocaters(other.m_rulesetLocaters), m_userSettings(other.m_userSettings) {}
    void SetRulesetLocaters(std::shared_ptr<IRulesetLocaterManager> locaters) { m_rulesetLocaters = locaters; }
    std::shared_ptr<IRulesetLocaterManager> GetRulesetLocaters() const { return m_rulesetLocaters; }
    void SetUserSettings(std::shared_ptr<IUserSettingsManager> settings) { m_userSettings = settings; }
    std::shared_ptr<IUserSettingsManager> GetUserSettings() const { return m_userSettings; }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ECPresentationManager::Impl
{
    typedef ECPresentationManager::Paths Paths;
    typedef ECPresentationManager::ImplParams Params;

protected:
/** @name Misc */
    virtual IPropertyCategorySupplier const& _GetCategorySupplier() const = 0;
    virtual IECPropertyFormatter const& _GetECPropertyFormatter() const = 0;
    virtual IUserSettingsManager& _GetUserSettingsManager() const = 0;
    virtual bvector<std::shared_ptr<ECInstanceChangeEventSource>> const& _GetECInstanceChangeEventSources() const = 0;
    virtual IJsonLocalState* _GetLocalState() const = 0;
    virtual IRulesetLocaterManager& _GetLocaters() const = 0;
    virtual IConnectionManagerR _GetConnections() = 0;
    virtual std::shared_ptr<INavNodesCache> _GetHierarchyCache(Utf8StringCR) const = 0;
/** @} */

/** @name Navigation */
    virtual std::unique_ptr<INodeInstanceKeysProvider> _CreateNodeInstanceKeysProvider(NodeInstanceKeysRequestImplParams const&) const = 0;
    virtual NavNodesDataSourcePtr _GetNodes(WithPageOptions<HierarchyRequestImplParams> const&) = 0;
    virtual size_t _GetNodesCount(HierarchyRequestImplParams const&) = 0;
    virtual ContentDescriptorCPtr _GetNodesDescriptor(HierarchyLevelDescriptorRequestImplParams const&) = 0;
    virtual NavNodeCPtr _GetParent(NodeParentRequestImplParams const&) = 0;
    virtual bvector<NavNodeCPtr> _GetFilteredNodes(NodePathsFromFilterTextRequestImplParams const&) = 0;
    virtual HierarchyComparePositionPtr _CompareHierarchies(HierarchyCompareRequestImplParams const&) = 0;
/** @} */

/** @name Content */
    virtual bvector<SelectClassInfo> _GetContentClasses(ContentClassesRequestImplParams const&) = 0;
    virtual ContentDescriptorCPtr _GetContentDescriptor(ContentDescriptorRequestImplParams const&) = 0;
    virtual ContentCPtr _GetContent(WithPageOptions<ContentRequestImplParams> const&) = 0;
    virtual size_t _GetContentSetSize(ContentRequestImplParams const&) = 0;
    virtual LabelDefinitionCPtr _GetDisplayLabel(KeySetDisplayLabelRequestImplParams const&) = 0;
    virtual PagingDataSourcePtr<DisplayValueGroupCPtr> _GetDistinctValues(WithPageOptions<DistinctValuesRequestImplParams> const&) = 0;
/** @} */

public:
/** @name General */
/** @{ */
    virtual ~Impl() {}
    IUserSettingsManager& GetUserSettingsManager() const {return _GetUserSettingsManager();}
    IUserSettings& GetUserSettings(Utf8CP rulesetId) const {return GetUserSettingsManager().GetSettings(rulesetId);}
    bvector<std::shared_ptr<ECInstanceChangeEventSource>> const& GetECInstanceChangeEventSources() const {return _GetECInstanceChangeEventSources();}
    IPropertyCategorySupplier const& GetCategorySupplier() const {return _GetCategorySupplier();}
    IECPropertyFormatter const& GetECPropertyFormatter() const {return _GetECPropertyFormatter();}
    IJsonLocalState* GetLocalState() const {return _GetLocalState();}
    IRulesetLocaterManager& GetLocaters() const {return _GetLocaters();}
    IConnectionManagerR GetConnections() {return _GetConnections();}
    std::shared_ptr<INavNodesCache> GetHierarchyCache(Utf8StringCR connectionId) const {return _GetHierarchyCache(connectionId);}
/** @} */

/** @name Navigation */
    std::unique_ptr<INodeInstanceKeysProvider> CreateNodeInstanceKeysProvider(NodeInstanceKeysRequestImplParams const& params) const {return _CreateNodeInstanceKeysProvider(params);}
    NavNodesDataSourcePtr GetNodes(WithPageOptions<HierarchyRequestImplParams> const& params) {return _GetNodes(params);}
    size_t GetNodesCount(HierarchyRequestImplParams const& params) {return _GetNodesCount(params);}
    ContentDescriptorCPtr GetNodesDescriptor(HierarchyLevelDescriptorRequestImplParams const& params) {return _GetNodesDescriptor(params);}
    NavNodeCPtr GetParent(NodeParentRequestImplParams const& params) {return _GetParent(params);}
    bvector<NavNodeCPtr> GetFilteredNodes(NodePathsFromFilterTextRequestImplParams const& params) {return _GetFilteredNodes(params);}
    HierarchyComparePositionPtr CompareHierarchies(HierarchyCompareRequestImplParams const& params) {return _CompareHierarchies(params);}
/** @} */

/** @name Content */
    bvector<SelectClassInfo> GetContentClasses(ContentClassesRequestImplParams const& params) {return _GetContentClasses(params);}
    ContentDescriptorCPtr GetContentDescriptor(ContentDescriptorRequestImplParams const& params) {return _GetContentDescriptor(params);}
    ContentCPtr GetContent(WithPageOptions<ContentRequestImplParams> const& params) {return _GetContent(params);}
    size_t GetContentSetSize(ContentRequestImplParams const& params) {return _GetContentSetSize(params);}
    LabelDefinitionCPtr GetDisplayLabel(KeySetDisplayLabelRequestImplParams const& params) {return _GetDisplayLabel(params);}
    PagingDataSourcePtr<DisplayValueGroupCPtr> GetDistinctValues(WithPageOptions<DistinctValuesRequestImplParams> const& params) {return _GetDistinctValues(params);}
/** @} */
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RulesDrivenECPresentationManagerImpl : ECPresentationManager::Impl, ECInstanceChangeEventSource::IEventHandler,
    IRulesetCallbacksHandler, IUserSettingsChangeListener, IConnectionsListener
{
    struct ECDbCaches;
    struct RulesetECExpressionsCache;
    struct UsedClassesListener;
    struct NodesProviderContextFactory;
    struct NodesProviderFactory;
    struct NavNodeLocater;

private:
    std::shared_ptr<IConnectionManager> m_connections;
    NavNodesFactory const* m_nodesFactory;
    NodesProviderContextFactory const* m_nodesProviderContextFactory;
    NodesProviderFactory const* m_nodesProviderFactory;
    CustomFunctionsInjector* m_customFunctions;
    std::unique_ptr<INodesCacheManager> m_nodesCachesManager;
    ContentCache* m_contentCache;
    ECDbCaches* m_ecdbCaches;
    RulesetECExpressionsCache* m_rulesetECExpressionsCache;
    UpdateHandler* m_updateHandler;
    UsedClassesListener* m_usedClassesListener;
    bmap<Utf8String, bvector<RuleSetLocaterPtr>> m_embeddedRuleSetLocaters;
    std::shared_ptr<IRulesetLocaterManager> m_locaters;
    std::shared_ptr<IUserSettingsManager> m_userSettings;
    IJsonLocalState* m_localState;
    IECPropertyFormatter const* m_ecPropertyFormatter;
    IPropertyCategorySupplier const* m_categorySupplier;
    bvector<std::shared_ptr<ECInstanceChangeEventSource>> m_ecInstanceChangeEventSources;
    mutable BeMutex m_mutex;

private:
    NavNodesProviderContextPtr CreateNodesProviderContext(HierarchyRequestImplParams const&, std::shared_ptr<INavNodesCache> = nullptr) const;
    RefCountedPtr<ProviderBasedNodesDataSource> GetCachedDataSource(WithPageOptions<HierarchyRequestImplParams> const&) const;
    RefCountedPtr<ProviderBasedNodesDataSource> GetCachedDataSource(NavNodesProviderContextR, PageOptionsCP) const;
    void TraverseHierarchy(HierarchyRequestImplParams const&, std::shared_ptr<INavNodesCache>) const;
    ContentProviderContextPtr CreateContentProviderContext(IConnectionCR, ContentProviderKey const&, std::unique_ptr<RulesetVariables>, ICancelationTokenCP) const;
    SpecificationContentProviderPtr GetContentProvider(IConnectionCR, ICancelationTokenCP, ContentProviderKey const&, RulesetVariables const& variables) const;
    SpecificationContentProviderPtr GetContentProvider(ContentRequestImplParams const&) const;
    void FinalizeNode(RequestWithRulesetImplParams const&, NavNodeR) const;
    NavNodePtr FinalizeNode(RequestWithRulesetImplParams const&, NavNodeCR) const;

protected:
    // ECPresentationManager::Impl: General
    IPropertyCategorySupplier const& _GetCategorySupplier() const override;
    IECPropertyFormatter const& _GetECPropertyFormatter() const override;
    IUserSettingsManager& _GetUserSettingsManager() const override {return *m_userSettings;}
    bvector<std::shared_ptr<ECInstanceChangeEventSource>> const& _GetECInstanceChangeEventSources() const override {return m_ecInstanceChangeEventSources;}
    IJsonLocalState* _GetLocalState() const override {return m_localState;}
    IRulesetLocaterManager& _GetLocaters() const override {return *m_locaters;}
    IConnectionManagerR _GetConnections() override {return *m_connections;}
    ECPRESENTATION_EXPORT std::shared_ptr<INavNodesCache> _GetHierarchyCache(Utf8StringCR connectionId) const override;

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

    // ECPresentationManager::Impl: Navigation
    ECPRESENTATION_EXPORT std::unique_ptr<INodeInstanceKeysProvider> _CreateNodeInstanceKeysProvider(NodeInstanceKeysRequestImplParams const&) const override;
    ECPRESENTATION_EXPORT NavNodesDataSourcePtr _GetNodes(WithPageOptions<HierarchyRequestImplParams> const&) override;
    ECPRESENTATION_EXPORT size_t _GetNodesCount(HierarchyRequestImplParams const&) override;
    ECPRESENTATION_EXPORT ContentDescriptorCPtr _GetNodesDescriptor(HierarchyLevelDescriptorRequestImplParams const&) override;
    ECPRESENTATION_EXPORT NavNodeCPtr _GetParent(NodeParentRequestImplParams const&) override;
    ECPRESENTATION_EXPORT bvector<NavNodeCPtr> _GetFilteredNodes(NodePathsFromFilterTextRequestImplParams const&) override;
    ECPRESENTATION_EXPORT HierarchyComparePositionPtr _CompareHierarchies(HierarchyCompareRequestImplParams const&) override;

    // ECPresentationManager::Impl: Content
    ECPRESENTATION_EXPORT bvector<SelectClassInfo> _GetContentClasses(ContentClassesRequestImplParams const&) override;
    ECPRESENTATION_EXPORT ContentDescriptorCPtr _GetContentDescriptor(ContentDescriptorRequestImplParams const&) override;
    ECPRESENTATION_EXPORT ContentCPtr _GetContent(WithPageOptions<ContentRequestImplParams> const&) override;
    ECPRESENTATION_EXPORT size_t _GetContentSetSize(ContentRequestImplParams const&) override;
    ECPRESENTATION_EXPORT LabelDefinitionCPtr _GetDisplayLabel(KeySetDisplayLabelRequestImplParams const&) override;
    ECPRESENTATION_EXPORT PagingDataSourcePtr<DisplayValueGroupCPtr> _GetDistinctValues(WithPageOptions<DistinctValuesRequestImplParams> const&) override;

public:
    ECPRESENTATION_EXPORT RulesDrivenECPresentationManagerImpl(Params const&);
    ECPRESENTATION_EXPORT ~RulesDrivenECPresentationManagerImpl();
    ECPRESENTATION_EXPORT void Initialize();
    ECPRESENTATION_EXPORT std::shared_ptr<NodesCache> GetNodesCache(IConnectionCR connection);
    ContentCache& GetContentCache() {return *m_contentCache;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
