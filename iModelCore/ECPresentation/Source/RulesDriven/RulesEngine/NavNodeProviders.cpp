/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
#include "ECSchemaHelper.h"
#include "NavNodeProviders.h"
#include "NavNodesDataSource.h"
#include "NavNodesCache.h"
#include "ECExpressionContextsProvider.h"
#include "LocalizationHelper.h"
#include "LoggingHelper.h"
#include "CustomizationHelper.h"
#include "QueryContracts.h"
#include "QueryBuilder.h"

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct NavNodesProvider::SpecificationsVisitor : PresentationRuleSpecificationVisitor
{
private:
    bvector<NavNodesProviderPtr> m_nodeProviders;
    NavNodesProviderContext const* m_context;

private:
    void AddQueryBasedNodeProvider(ChildNodeSpecification const& specification)
        {
        NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, specification);
        m_nodeProviders.push_back(provider);
        }
    void AddCustomNodeProvider(CustomNodeSpecification const& specification)
        {
        NavNodesProviderPtr provider = CustomNodesProvider::Create(*m_context, specification);
        m_nodeProviders.push_back(provider);
        }

protected:
    void _Visit(AllInstanceNodesSpecification const& specification) override {AddQueryBasedNodeProvider(specification);}
    void _Visit(AllRelatedInstanceNodesSpecification const& specification) override {AddQueryBasedNodeProvider(specification);}
    void _Visit(RelatedInstanceNodesSpecification const& specification) override {AddQueryBasedNodeProvider(specification);}
    void _Visit(InstanceNodesOfSpecificClassesSpecification const& specification) override {AddQueryBasedNodeProvider(specification);}
    void _Visit(SearchResultInstanceNodesSpecification const& specification) override {AddQueryBasedNodeProvider(specification);}
    void _Visit(CustomNodeSpecification const& specification) override {AddCustomNodeProvider(specification);}

public:
    SpecificationsVisitor() : m_context(nullptr) {}
    void SetContext(NavNodesProviderContextCR context) {m_context = &context;}
    bvector<NavNodesProviderPtr> const& GetNodeProviders() const {return m_nodeProviders;}
};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceRelatedSettingsUpdater::DataSourceRelatedSettingsUpdater(NavNodesProviderContextCR context, JsonNavNodeCP node)
    : m_context(context), m_node(node)
    {
    m_relatedSettingsCountBefore = m_context.GetRelatedSettings().size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceRelatedSettingsUpdater::~DataSourceRelatedSettingsUpdater()
    {
    bvector<UserSettingEntry> settings = m_context.GetRelatedSettings();
    if (settings.size() == m_relatedSettingsCountBefore)
        return;

    DataSourceInfo dsInfo = m_node ? m_context.GetNodesCache().FindDataSource(m_node->GetNodeId()) : m_context.GetDataSourceInfo();
    if (!dsInfo.IsValid())
        {
        BeAssert(false);
        return;
        }

    m_context.GetNodesCache().Update(dsInfo, nullptr, nullptr, &settings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<uint64_t> ProvidersIndexAllocator::CreateIndex(bool createNew)
    {
    bvector<uint64_t> index = m_parentIndex;
    if (createNew)
        {
        index.push_back(m_currIndex++);
        }
    else
        {
        BeAssert(m_currIndex > 0);
        index.push_back(m_currIndex - 1);
        }
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderContext::NavNodesProviderContext(PresentationRuleSetCR ruleset, bool holdRuleset, RuleTargetTree targetTree, Utf8String locale, uint64_t const* physicalParentId,
    IUserSettings const& userSettings, ECExpressionsCache& ecexpressionsCache, RelatedPathsCache& relatedPathsCache, PolymorphicallyRelatedClassesCache& polymorphicallyRelatedClassesCache,
    JsonNavNodesFactory const& nodesFactory, IHierarchyCache& nodesCache, INodesProviderFactoryCR providerFactory, IJsonLocalState const* localState)
    : RulesDrivenProviderContext(ruleset, holdRuleset, locale, userSettings, ecexpressionsCache, relatedPathsCache, polymorphicallyRelatedClassesCache, nodesFactory, localState),
    m_targetTree(targetTree), m_nodesCache(&nodesCache), m_providerFactory(providerFactory)
    {
    Init();
    InitProvidersIndexAllocator(physicalParentId);

    if (nullptr != physicalParentId)
        m_physicalParentNodeId = new uint64_t(*physicalParentId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderContext::NavNodesProviderContext(NavNodesProviderContextCR other)
    : RulesDrivenProviderContext(other), m_nodesCache(other.m_nodesCache), m_targetTree(other.m_targetTree),
    m_physicalParentNodeId(nullptr), m_providerFactory(other.m_providerFactory)
    {
    Init();
    InitProvidersIndexAllocator(other.m_physicalParentNodeId);

    if (nullptr != other.m_physicalParentNodeId)
        m_physicalParentNodeId = new uint64_t(*other.m_physicalParentNodeId);

    if (other.IsQueryContext())
        SetQueryContext(other);

    if (other.IsUpdateContext())
        SetUpdateContext(other);

    if (other.IsUpdatesDisabled())
        SetIsUpdatesDisabled(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderContext::~NavNodesProviderContext()
    {
    DELETE_AND_CLEAR(m_queryBuilder);
    DELETE_AND_CLEAR(m_usedClassesListener);
    DELETE_AND_CLEAR(m_physicalParentNodeId);
    DELETE_AND_CLEAR(m_virtualParentNodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::Init()
    {
    m_isFullLoadDisabled = false;
    m_isRootNodeContext = false;
    m_rootNodeRule = nullptr;
    m_isChildNodeContext = false;
    m_physicalParentNodeId = nullptr;
    m_virtualParentNodeId = nullptr;
    m_childNodeRule = nullptr;
    m_queryBuilder = nullptr;
    m_usedClassesListener = nullptr;
    m_isUpdateContext = false;
    m_isUpdatesDisabled = false;
    m_isCheckingChildren = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::InitProvidersIndexAllocator(uint64_t const* physicalParentNodeId)
    {
    if (nullptr != physicalParentNodeId)
        {
        DataSourceInfo parentDsInfo = m_nodesCache->FindDataSource(*physicalParentNodeId);
        if (parentDsInfo.IsValid())
            m_providersIndexAllocator = new ProvidersIndexAllocator(parentDsInfo.GetIndex());
        }
    if (m_providersIndexAllocator.IsNull())
        m_providersIndexAllocator = new ProvidersIndexAllocator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IHierarchyCacheR NavNodesProviderContext::GetNodesCache() const {return *m_nodesCache;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<UserSettingEntry> NavNodesProviderContext::GetRelatedSettings() const
    {
    bset<Utf8String> ids = RulesDrivenProviderContext::GetRelatedSettingIds();
    bvector<UserSettingEntry> idsWithValues;
    for (Utf8StringCR id : ids)
        idsWithValues.push_back(UserSettingEntry(id, GetUserSettings().GetSettingValueAsJson(id.c_str())));

    if (m_virtualParentNodeId && (!m_physicalParentNodeId || *m_physicalParentNodeId != *m_virtualParentNodeId))
        {
        // note: if virtual parent doesn't match physical one, we need to include virtual parent's settings
        NavNodesProviderPtr provider = GetNodesCache().GetDataSource(*m_virtualParentNodeId, false);
        if (provider.IsValid())
            {
            bvector<UserSettingEntry> virtualParentSettings = provider->GetContext().GetRelatedSettings();
            std::move(virtualParentSettings.begin(), virtualParentSettings.end(), std::inserter(idsWithValues, idsWithValues.end()));
            }
        }

    NavNodeCPtr physicalParentNode = GetPhysicalParentNode();
    if (physicalParentNode.IsValid() && NavNodesHelper::IsGroupingNode(*physicalParentNode))
        {
        // note: if parent node is a grouping node, we want to append all its related settings
        // because it may have derived them from it's virtual parent
        NavNodesProviderPtr provider = GetNodesCache().GetDataSource(physicalParentNode->GetNodeId(), false);
        if (provider.IsValid())
            {
            bvector<UserSettingEntry> physicalParentSettings = provider->GetContext().GetRelatedSettings();
            std::move(physicalParentSettings.begin(), physicalParentSettings.end(), std::inserter(idsWithValues, idsWithValues.end()));
            }
        }

    return idsWithValues;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodeCPtr NavNodesProviderContext::GetPhysicalParentNode() const
    {
    if (nullptr == m_physicalParentNodeId)
        return nullptr;

    JsonNavNodeCPtr node = GetNodesCache().GetNode(*m_physicalParentNodeId);
    BeAssert(node.IsValid());
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodeCPtr NavNodesProviderContext::GetVirtualParentNode() const
    {
    if (nullptr == m_virtualParentNodeId)
        return nullptr;

    JsonNavNodeCPtr node = GetNodesCache().GetNode(*m_virtualParentNodeId);
    BeAssert(node.IsValid());
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NavNodesProviderContext::CreateHierarchyLevelProvider(NavNodesProviderContextR context, JsonNavNodeCP parentNode) const
    {
    return m_providerFactory.Create(context, parentNode, ProviderCacheType::Combined);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetRootNodeContext(RootNodeRuleCR rootNodeRule)
    {
    BeAssert(!IsRootNodeContext());
    BeAssert(!IsChildNodeContext());
    m_isRootNodeContext = true;
    m_rootNodeRule = &rootNodeRule;
    m_virtualParentNodeId = (nullptr != m_physicalParentNodeId) ? new uint64_t(*m_physicalParentNodeId) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetRootNodeContext(NavNodesProviderContextCR other)
    {
    BeAssert(!IsRootNodeContext());
    BeAssert(!IsChildNodeContext());
    m_isRootNodeContext = other.m_isRootNodeContext;
    m_rootNodeRule = other.m_rootNodeRule;
    m_virtualParentNodeId = (nullptr != other.m_virtualParentNodeId) ? new uint64_t(*other.m_virtualParentNodeId) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetChildNodeContext(ChildNodeRuleCP childNodeRule, NavNodeCR virtualParentNode)
    {
    BeAssert(!IsRootNodeContext());
    BeAssert(!IsChildNodeContext());
    m_isChildNodeContext = true;
    m_virtualParentNodeId = new uint64_t(virtualParentNode.GetNodeId());
    m_childNodeRule = childNodeRule;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetChildNodeContext(NavNodesProviderContextCR other)
    {
    BeAssert(!IsRootNodeContext());
    BeAssert(!IsChildNodeContext());
    m_isChildNodeContext = other.m_isChildNodeContext;
    m_virtualParentNodeId = (nullptr != other.m_virtualParentNodeId) ? new uint64_t(*other.m_virtualParentNodeId) : nullptr;
    m_childNodeRule = other.m_childNodeRule;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetQueryContext(IConnectionManagerCR connections, IConnectionCR connection, ECSqlStatementCache const& statementCache, CustomFunctionsInjector& customFunctions, IECDbUsedClassesListener* usedClassesListener)
    {
    RulesDrivenProviderContext::SetQueryContext(connections, connection, statementCache, customFunctions);
    if (nullptr != usedClassesListener)
        m_usedClassesListener = new ECDbUsedClassesListenerWrapper(GetConnection(), *usedClassesListener);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetQueryContext(NavNodesProviderContextCR other)
    {
    RulesDrivenProviderContext::SetQueryContext(other);
    if (nullptr != other.m_usedClassesListener)
        m_usedClassesListener = new ECDbUsedClassesListenerWrapper(*other.m_usedClassesListener);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationQueryBuilder& NavNodesProviderContext::GetQueryBuilder() const
    {
    if (nullptr == m_queryBuilder)
        {
        NavigationQueryBuilderParameters params(GetSchemaHelper(), GetConnections(), GetConnection(), GetRuleset(), GetLocale(),
            GetUserSettings(), &GetUsedSettingsListener(), GetECExpressionsCache(), *m_nodesCache, GetLocalState());

        if (nullptr != m_usedClassesListener)
            params.SetUsedClassesListener(m_usedClassesListener);

        m_queryBuilder = new NavigationQueryBuilder(params);
        }
    return *m_queryBuilder;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IUsedClassesListener* NavNodesProviderContext::GetUsedClassesListener() const {return m_usedClassesListener;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetUpdateContext(bool isUpdateContext)
    {
    m_isUpdateContext = isUpdateContext;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetUpdateContext(NavNodesProviderContextCR other)
    {
    m_isUpdateContext = other.m_isUpdateContext;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
HierarchyLevelInfo const& NavNodesProviderContext::GetHierarchyLevelInfo() const
    {
    if (!m_hierarchyLevelInfo.IsValid())
        {
        m_hierarchyLevelInfo = GetNodesCache().FindHierarchyLevel(GetConnection().GetId().c_str(),
            GetRuleset().GetRuleSetId().c_str(), IsLocalizationContext() ? GetLocale().c_str() : "",
            GetVirtualParentNodeId());
        }
    if (!m_hierarchyLevelInfo.IsValid())
        {
        m_hierarchyLevelInfo = HierarchyLevelInfo(GetConnection().GetId(), GetRuleset().GetRuleSetId(),
            IsLocalizationContext() ? GetLocale() : "",
            GetPhysicalParentNodeId() ? *GetPhysicalParentNodeId() : 0,
            GetVirtualParentNodeId() ? *GetVirtualParentNodeId() : 0);
        GetNodesCache().Cache(m_hierarchyLevelInfo);
        }
    BeAssert(m_hierarchyLevelInfo.IsValid());
    return m_hierarchyLevelInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceInfo const& NavNodesProviderContext::GetDataSourceInfo() const
    {
    if (!m_dataSourceInfo.IsValid())
        {
        bvector<uint64_t> index = GetProvidersIndexAllocator().AllocateIndex();
        m_dataSourceInfo = GetNodesCache().FindDataSource(GetHierarchyLevelInfo().GetId(), index);
        if (!m_dataSourceInfo.IsValid())
            {
            m_dataSourceInfo = DataSourceInfo(GetHierarchyLevelInfo().GetId(), index);
            GetNodesCache().Cache(m_dataSourceInfo, DataSourceFilter(), bmap<ECClassId, bool>(),
                GetRelatedSettings(), IsUpdatesDisabled());
            }
        }
    BeAssert(m_dataSourceInfo.IsValid());
    return m_dataSourceInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetLocale(RulesDrivenProviderContextCR context)
    {
    return context.IsLocalizationContext() ? context.GetLocale() : "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProvider::NavNodesProvider(NavNodesProviderContextCR context)
    : m_context(const_cast<NavNodesProviderContext*>(&context)), m_nodesInitialized(false),
    m_hasCachedHasNodesFlag(false), m_hasCachedNodesCount(false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProvider::Initialize() const
    {
    if (_IsCacheable())
        {
        // call GetDataSourceInfo() to ensure it's cached
        GetContext().GetDataSourceInfo();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProvider::InitializeNodes() const
    {
    if (m_nodesInitialized)
        return;

    if (const_cast<NavNodesProvider*>(this)->_InitializeNodes())
        m_nodesInitialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* Note: ancestorContext doesn't necessarily mean immediate parent context
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static NavNodesProviderContextPtr CreateContextForChildHierarchyLevel(NavNodesProviderContextCR ancestorContext, JsonNavNodeCR parentNode)
    {
    NavNodesProviderContextPtr ctx = NavNodesProviderContext::Create(ancestorContext);
    ctx->SetVirtualParentNode(parentNode);
    if (NodeVisibility::Virtual == ancestorContext.GetNodesCache().GetNodeVisibility(parentNode.GetNodeId()))
        ctx->SetPhysicalParentNodeId(parentNode.GetParentNodeId());
    else
        ctx->SetPhysicalParentNode(parentNode);

    DataSourceInfo parentDataSourceInfo = ancestorContext.GetNodesCache().FindDataSource(parentNode.GetNodeId());
    BeAssert(parentDataSourceInfo.IsValid());
    ctx->SetProvidersIndexAllocator(*new ProvidersIndexAllocator(parentDataSourceInfo.GetIndex()));

    return ctx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static NavNodesProviderContextPtr CreateContextForSameHierarchyLevel(NavNodesProviderContextCR baseContext, bool copyNodesContext)
    {
    NavNodesProviderContextPtr ctx = NavNodesProviderContext::Create(baseContext);
    ctx->SetProvidersIndexAllocator(baseContext.GetProvidersIndexAllocator());
    if (!copyNodesContext)
        return ctx;

    if (baseContext.IsRootNodeContext())
        ctx->SetRootNodeContext(baseContext);
    else if (baseContext.IsChildNodeContext())
        ctx->SetChildNodeContext(baseContext);
    return ctx;
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2019
+===============+===============+===============+===============+===============+======*/
struct NodesChildrenCheckContext
    {
    NavNodesProviderCR m_provider;
    bool m_wasCheckingChildren;
    NodesChildrenCheckContext(NavNodesProviderCR provider)
        : m_provider(provider)
        {
        m_wasCheckingChildren = m_provider.GetContext().IsCheckingChildren();
        m_provider.GetContextR().SetIsCheckingChildren(true);
        }
    ~NodesChildrenCheckContext() { m_provider.GetContextR().SetIsCheckingChildren(m_wasCheckingChildren); }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2019
+===============+===============+===============+===============+===============+======*/
struct OptimizationFlagsCarrier
    {
    private:
        NavNodesProviderR m_childProvider;
        bool m_wasCheckingChildren;
        bool m_wasFullNodesLoadDisabled;
    public:
        OptimizationFlagsCarrier(NavNodesProviderCR parentProvider, NavNodesProviderR childProvider)
            : m_childProvider(childProvider)
            {
            m_wasFullNodesLoadDisabled = m_childProvider.GetContext().IsFullNodesLoadDisabled();
            m_wasCheckingChildren = m_childProvider.GetContext().IsCheckingChildren();
            m_childProvider.GetContextR().SetDisableFullLoad(parentProvider.GetContext().IsFullNodesLoadDisabled());
            m_childProvider.GetContextR().SetIsCheckingChildren(parentProvider.GetContext().IsCheckingChildren());
            }
        ~OptimizationFlagsCarrier()
            {
            m_childProvider.GetContextR().SetDisableFullLoad(m_wasFullNodesLoadDisabled);
            m_childProvider.GetContextR().SetIsCheckingChildren(m_wasCheckingChildren);
            }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodesProvider::GetNode(JsonNavNodePtr& node, size_t index) const
    {
    if (_GetInitializationStrategy() == ProviderNodesInitializationStrategy::Automatic)
        InitializeNodes();

    return _GetNode(node, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodesProvider::HasNodes() const
    {
    if (!m_hasCachedHasNodesFlag)
        {
        if (_GetInitializationStrategy() == ProviderNodesInitializationStrategy::Automatic)
            InitializeNodes();

        NodesChildrenCheckContext checkingNodes(*this);
        m_cachedHasNodesFlag = _HasNodes();
        m_hasCachedHasNodesFlag = true;
        }
    return m_cachedHasNodesFlag;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t NavNodesProvider::GetNodesCount() const
    {
    if (!m_hasCachedNodesCount)
        {
        if (_GetInitializationStrategy() == ProviderNodesInitializationStrategy::Automatic)
            InitializeNodes();

        m_cachedNodesCount = _GetNodesCount();

        if (!GetContext().IsCheckingChildren())
            {
            // note: if we're counting just to check whether provider has any nodes,
            // we should not trust this result to be exact - some providers can
            // return early as soon as they find there's at least one node
            m_hasCachedNodesCount = true;
            }
        }
    return m_cachedNodesCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NodeArtifacts> NavNodesProvider::GetArtifacts() const
    {
    if (_GetInitializationStrategy() == ProviderNodesInitializationStrategy::Automatic)
        InitializeNodes();

    return _GetArtifacts();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProvider::NotifyNodeChanged(JsonNavNodeCR node) const
    {
    GetContext().GetNodesCache().Update(node.GetNodeId(), node);
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2018
+===============+===============+===============+===============+===============+======*/
struct NodeHasChildrenFlagUpdater
    {
    IHierarchyCacheR m_cache;
    JsonNavNodePtr m_node;
    HasChildrenFlag const& m_flag;
    HasChildrenFlag m_initialFlag;

    NodeHasChildrenFlagUpdater(IHierarchyCacheR cache, JsonNavNodePtr node, HasChildrenFlag const& flag)
        : m_cache(cache), m_node(node), m_flag(flag), m_initialFlag(flag)
        {}
    ~NodeHasChildrenFlagUpdater() { Update(); }
    void Update()
        {
        if (m_flag == m_initialFlag)
            return;
        m_node->SetHasChildren(HASCHILDREN_True == m_flag);
        m_cache.Update(m_node->GetNodeId(), *m_node);
        m_initialFlag = m_flag;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
HasChildrenFlag NavNodesProvider::AnyChildSpecificationReturnsNodes(JsonNavNode const& parentNode) const
    {
    NavNodesProviderPtr childrenProvider = GetContext().CreateHierarchyLevelProvider(*CreateContextForChildHierarchyLevel(GetContext(), parentNode), &parentNode);
    return childrenProvider->HasNodes() ? HASCHILDREN_True : HASCHILDREN_False;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProvider::DetermineChildren(JsonNavNodeR node) const
    {
    if (node.DeterminedChildren())
        return;

    NavNodeExtendedData extendedData(node);
    if (ChildrenHint::Unknown != extendedData.GetChildrenHint())
        return;

    node.SetHasChildren(HASCHILDREN_True == AnyChildSpecificationReturnsNodes(node));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProvider::FinalizeNode(JsonNavNodeR node, bool customizeLabel) const
    {
    if (!GetContext().NeedsFullLoad())
        return;

    DataSourceRelatedSettingsUpdater updater(GetContext(), &node);
    bool changed = false;

    // make sure the node is customized
    if (!NavNodeExtendedData(node).IsCustomized())
        {
        CustomizationHelper::Customize(GetContext(), node, customizeLabel);
        changed = true;
        }

    // make sure the node has determined if it has any children
    if (!node.DeterminedChildren())
        {
        DetermineChildren(node);
        changed = true;
        }

    // if any changes mande, update the node in cache
    if (changed)
        NotifyNodeChanged(node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProvider::FinalizeNodes()
    {
    size_t count = GetNodesCount();
    for (size_t i = 0; i < count; ++i)
        {
        if (GetContext().GetCancelationToken().IsCanceled())
            return;

        JsonNavNodePtr node;
        GetNode(node, i);
        FinalizeNode(*node, false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NavNodesProvider::GetCachedProvider() const
    {
    if (!GetContext().GetNodesCache().IsInitialized(GetContext().GetDataSourceInfo()))
        return nullptr;

    NavNodesProviderPtr provider = GetContext().GetNodesCache().GetDataSource(GetContext().GetDataSourceInfo(), false);
    if (!provider.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return provider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodesProvider::HasSimilarNodeInHierarchy(JsonNavNodeCR node, uint64_t parentNodeId) const
    {
    NavNodeCPtr parentNavNode = GetContext().GetNodesCache().GetNode(parentNodeId);
    if (parentNavNode == nullptr)
        return false;

    BeAssert(dynamic_cast<JsonNavNodeCP>(parentNavNode.get()) != nullptr);
    JsonNavNodeCP parentNode = static_cast<JsonNavNodeCP>(parentNavNode.get());

    NavNodeExtendedData thisNodeExtendedData(node);
    NavNodeExtendedData parentNodeExtendedData(*parentNode);

    Utf8String nodeHash = node.GetKey()->GetPathFromRoot().back();
    Utf8String parentHash = parentNode->GetKey()->GetPathFromRoot().back();

    return nodeHash.Equals(parentHash) && 0 == strcmp(thisNodeExtendedData.GetSpecificationHash(), parentNodeExtendedData.GetSpecificationHash())
        || HasSimilarNodeInHierarchy(node, parentNodeExtendedData.HasVirtualParentId() ? parentNodeExtendedData.GetVirtualParentId() : 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NodeArtifacts> NavNodesProvider::GetChildrenArtifacts(JsonNavNodeCR parent) const
    {
    NavNodesProviderPtr childrenProvider = GetContext().CreateHierarchyLevelProvider(*CreateContextForChildHierarchyLevel(GetContext(), parent), &parent);
    return childrenProvider->GetArtifacts();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool NeedsChildrenDetermined(Utf8StringCR expression)
    {
    return expression.Contains(".HasChildren");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ShouldHideNodeBasedOnHideExpression(NavNodesProviderContextCR context, JsonNavNodeCR node, Utf8StringCR expression)
    {
    if (expression.empty())
        return false;

    JsonNavNodeCPtr parentNode = context.GetVirtualParentNode();
    ECExpressionContextsProvider::CustomizationRulesContextParameters params(node, parentNode.get(),
        context.GetConnection(), context.GetLocale(), context.GetUserSettings(), &context.GetUsedSettingsListener());
    ExpressionContextPtr expressionContext = ECExpressionContextsProvider::GetCustomizationRulesContext(params);
    ECValue value;
    if (ECExpressionsHelper(context.GetECExpressionsCache()).EvaluateECExpression(value, expression, *expressionContext) && value.IsPrimitive() && value.ConvertToPrimitiveType(PRIMITIVETYPE_Boolean))
        return value.GetBoolean();
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
EmptyNavNodesProvider::EmptyNavNodesProvider(NavNodesProviderContextR context)
    : NavNodesProvider(context)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool EmptyNavNodesProvider::_InitializeNodes()
    {
    GetContext().GetNodesCache().FinalizeInitialization(GetContext().GetDataSourceInfo());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NodeArtifacts> SingleNavNodeProvider::_GetArtifacts() const
    {
    return { CustomizationHelper::EvaluateArtifacts(GetContext(), *m_node) };
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CustomNodesProvider::CustomNodesProvider(NavNodesProviderContextCR context, CustomNodeSpecificationCR specification)
    : NavNodesProvider(context), m_specification(specification)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomNodesProvider::_InitializeNodes()
    {
    if (!NavNodesProvider::_InitializeNodes())
        return false;

    NavNodesProviderPtr cachedProvider = GetCachedProvider();
    if (cachedProvider.IsValid())
        {
        DisabledFullNodesLoadContext doNotCustomizeCachedNodes(*cachedProvider);
        JsonNavNodePtr cachedNode;
        if (1 != cachedProvider->GetNodesCount() || !cachedProvider->GetNode(cachedNode, 0))
            {
            BeAssert(false);
            return true;
            }
        NavNodeExtendedData cachedNodeExtendedData(*cachedNode);
        if (GetContext().IsChildNodeContext() && HasSimilarNodeInHierarchy(*cachedNode, cachedNodeExtendedData.HasVirtualParentId() ? cachedNodeExtendedData.GetVirtualParentId() : 0))
            return true;
        if (NodeVisibility::Virtual != GetContext().GetNodesCache().GetNodeVisibility(cachedNode->GetNodeId()))
            m_node = cachedNode;
        else
            m_childNodesProvider = GetContext().CreateHierarchyLevelProvider(*CreateContextForChildHierarchyLevel(GetContext(), *cachedNode), cachedNode.get());
        return true;
        }

    if (m_specification.GetNodeType().empty() || m_specification.GetLabel().empty())
        {
        LoggingHelper::LogMessage(Log::Navigation, "Type and Label are required properties for CustomNode specification");
        BeAssert(false);
        return false;
        }

    Utf8String connectionId = GetContext().IsQueryContext() ? GetContext().GetConnection().GetId() : "";
    Utf8String type(m_specification.GetNodeType().c_str());
    Utf8String imageId(m_specification.GetImageId().c_str());
    Utf8String label(m_specification.GetLabel().c_str());
    Utf8String description(m_specification.GetDescription().c_str());
    JsonNavNodeCPtr parent = GetContext().GetVirtualParentNode();
    m_node = GetContext().GetNodesFactory().CreateCustomNode(connectionId, GetLocale(GetContext()), label.c_str(), description.c_str(), imageId.c_str(), type.c_str());

    NavNodeExtendedData extendedData(*m_node);
    extendedData.SetRulesetId(GetContext().GetRuleset().GetRuleSetId().c_str());
    extendedData.SetSpecificationHash(m_specification.GetHash());
    if (ChildrenHint::Unknown != m_specification.GetHasChildren())
        extendedData.SetChildrenHint(m_specification.GetHasChildren());

    if (nullptr != GetContext().GetPhysicalParentNodeId())
        m_node->SetParentNodeId(*GetContext().GetPhysicalParentNodeId());
    if (nullptr != GetContext().GetVirtualParentNodeId())
        extendedData.SetVirtualParentId(*GetContext().GetVirtualParentNodeId());
    if (GetContext().IsRootNodeContext() && GetContext().GetRootNodeRule().GetAutoExpand())
        m_node->SetIsExpanded(true);

    m_node->SetNodeKey(*NavNodesHelper::CreateNodeKey(GetContext().GetConnection(), *m_node, parent.IsValid() ? parent->GetKey().get() : nullptr));

    GetContext().GetNodesCache().Cache(*m_node, GetContext().GetDataSourceInfo(), 0, false);

    if (GetContext().IsChildNodeContext() && HasSimilarNodeInHierarchy(*m_node, extendedData.HasVirtualParentId() ? extendedData.GetVirtualParentId() : 0))
        {
        GetContext().GetNodesCache().MakeVirtual(*m_node);
        GetContext().GetNodesCache().FinalizeInitialization(GetContext().GetDataSourceInfo());
        m_node = nullptr;
        return true;
        }

    // note: determining children articats must be done _after_ caching the parent node
    // but _before_ it's provider is created (because creating children provider might
    // require already having the artifacts)
    if (extendedData.SetChildrenArtifacts(GetChildrenArtifacts(*m_node)))
        GetContext().GetNodesCache().Update(m_node->GetNodeId(), *m_node);

    if (m_specification.GetHideNodesInHierarchy() || ShouldHideNodeBasedOnHideExpression(GetContext(), *m_node, m_specification.GetHideExpression()))
        {
        GetContext().GetNodesCache().MakeVirtual(*m_node);
        GetContext().GetNodesCache().FinalizeInitialization(GetContext().GetDataSourceInfo());
        m_childNodesProvider = GetContext().CreateHierarchyLevelProvider(*CreateContextForChildHierarchyLevel(GetContext(), *m_node), m_node.get());
        return true;
        }

    HasChildrenFlag hasChildren = HASCHILDREN_Unknown;
    NodeHasChildrenFlagUpdater hasChildrenUpdater(GetContext().GetNodesCache(), m_node, hasChildren);

    if (ChildrenHint::Always == m_specification.GetHasChildren())
        hasChildren = HASCHILDREN_True;
    else if (ChildrenHint::Never == m_specification.GetHasChildren())
        hasChildren = HASCHILDREN_False;
    else if (m_specification.GetHideIfNoChildren())
        {
        if (HASCHILDREN_Unknown == hasChildren)
            hasChildren = AnyChildSpecificationReturnsNodes(*m_node);

        if (HASCHILDREN_False == hasChildren)
            {
            // if the node has "hide if no children" flag and none of the child specs return nodes, return false
            GetContext().GetNodesCache().MakeVirtual(*m_node);
            GetContext().GetNodesCache().FinalizeInitialization(GetContext().GetDataSourceInfo());
            m_node = nullptr;
            return true;
            }
         }

    GetContext().GetNodesCache().FinalizeInitialization(GetContext().GetDataSourceInfo());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomNodesProvider::_GetNode(JsonNavNodePtr& node, size_t index) const
    {
    if (m_childNodesProvider.IsValid())
        return m_childNodesProvider->GetNode(node, index);

    if (index > 0)
        return false;

    if (m_node.IsValid())
        FinalizeNode(*m_node, true);

    node = m_node;
    return m_node.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomNodesProvider::_HasNodes() const
    {
    return m_childNodesProvider.IsValid() ? m_childNodesProvider->HasNodes() : m_node.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CustomNodesProvider::_GetNodesCount() const
    {
    if (m_childNodesProvider.IsValid())
        return m_childNodesProvider->GetNodesCount();
    return m_node.IsValid() ? 1 : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NodeArtifacts> CustomNodesProvider::_GetArtifacts() const
    {
    bvector<NodeArtifacts> result;
    if (m_node.IsValid())
        result.push_back(CustomizationHelper::EvaluateArtifacts(GetContext(), *m_node));
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
QueryBasedNodesProvider::QueryBasedNodesProvider(NavNodesProviderContextCR context, NavigationQuery const& query, bmap<ECClassId, bool> const& usedClassIds)
    : MultiNavNodesProvider(context), m_query(&query), m_executor(context.GetNodesFactory(), context.GetConnection(), GetLocale(context), context.GetStatementCache(), query),
    m_executorIndex(0), m_usedClassIds(usedClassIds), m_offset(0)
    { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBasedNodesProvider::SetQuery(NavigationQuery const& query, bmap<ECClassId, bool> const& usedClassIds)
    {
    BeAssert(m_query->GetResultParameters().GetResultType() == query.GetResultParameters().GetResultType());
    m_executor.SetQuery(query, false);
    m_query = &query;
    m_usedClassIds = usedClassIds;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryBasedNodesProvider::ShouldReturnChildNodes(JsonNavNode const& node, HasChildrenFlag& hasChildren) const
    {
    NavNodeExtendedData extendedData(node);

    // when the node is created using a specification with "hide nodes in hierarchy" flag
    if (extendedData.HideNodesInHierarchy())
        return true;

    // when the node is created using a specification with "hide expression" flag
    if (extendedData.HasHideExpression() && ShouldHideNodeBasedOnHideExpression(GetContext(), node, extendedData.GetHideExpression()))
        return true;

    // if the node has "hide if grouping value not specified" flag and is a value property grouping node, grouping null values, show its chilren
    if (extendedData.HideIfGroupingValueNotSpecified() && node.GetType().Equals(NAVNODE_TYPE_ECPropertyGroupingNode) && extendedData.HasPropertyValue() && extendedData.GetPropertyValue()->IsNull())
        return true;

    // if the node has only one child and also has "hide if only one child" flag, we want to display that child
    if (extendedData.HideIfOnlyOneChild())
        {
        if (extendedData.HasGroupingType() && 1 == extendedData.GetGroupedInstanceKeysCount())
            {
            // grouping node having only one grouped instance always means it has only
            // one child - we can skip checking
            return true;
            }

        NavNodesProviderPtr childrenProvider = GetContext().CreateHierarchyLevelProvider(*CreateContextForChildHierarchyLevel(GetContext(), node), &node);
        size_t childrenCount = childrenProvider->GetNodesCount();
        hasChildren = (childrenCount > 0) ? HASCHILDREN_True : HASCHILDREN_False;

        if (childrenCount == 1)
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr QueryBasedNodesProvider::CreateProvider(JsonNavNodeR node) const
    {
    NavNodeExtendedData extendedData(node);

    // avoid returning a node if we already have a similar ancestor (prevent infinite hierarchies)
    NavNodesProviderContextPtr nestedContext = NavNodesProviderContext::Create(GetContext());
    if (GetContext().IsChildNodeContext() && HasSimilarNodeInHierarchy(node, extendedData.HasVirtualParentId() ? extendedData.GetVirtualParentId() : 0))
        {
        GetContext().GetNodesCache().MakeVirtual(node);
        return EmptyNavNodesProvider::Create(*nestedContext);
        }

    HasChildrenFlag hasChildren = HASCHILDREN_Unknown;
    NodeHasChildrenFlagUpdater hasChildrenUpdater(GetContext().GetNodesCache(), &node, hasChildren);

    if (ChildrenHint::Always == extendedData.GetChildrenHint())
        hasChildren = HASCHILDREN_True;
    else if (ChildrenHint::Never == extendedData.GetChildrenHint() && !extendedData.HasGroupingType())
        hasChildren = HASCHILDREN_False;

    // may need to determine children..
    if (HASCHILDREN_Unknown == hasChildren && (extendedData.HideIfNoChildren() || extendedData.HasHideExpression() && NeedsChildrenDetermined(extendedData.GetHideExpression())))
        hasChildren = AnyChildSpecificationReturnsNodes(node);
    hasChildrenUpdater.Update();

    // the specification may want to return node's children instead of the node itself
    if (ShouldReturnChildNodes(node, hasChildren))
        {
        GetContext().GetNodesCache().MakeVirtual(node);
        return GetContext().CreateHierarchyLevelProvider(*CreateContextForChildHierarchyLevel(GetContext(), node), &node);
        }

    // if the node has "hide if no children" flag and none of the child specs return nodes, skip this node
    if (extendedData.HideIfNoChildren() && HASCHILDREN_False == hasChildren)
        {
        GetContext().GetNodesCache().MakeVirtual(node);
        return EmptyNavNodesProvider::Create(*nestedContext);
        }

    // otherwise, make node physical
    return SingleNavNodeProvider::Create(node, *nestedContext);
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct SpecificationFiltersBuilder : PresentationRuleSpecificationVisitor
{
private:
    IHierarchyCache const& m_nodesCache;
    NavNodeCP m_parent;
    NavigationQueryCR m_query;
    DataSourceFilter m_filter;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodeCPtr GetParentInstanceNode() const
        {
        NavNodeCPtr parentInstanceNode = m_parent;
        while (parentInstanceNode.IsValid() && nullptr == parentInstanceNode->GetKey()->AsECInstanceNodeKey())
            parentInstanceNode = m_nodesCache.GetNode(parentInstanceNode->GetParentNodeId());
        return parentInstanceNode;
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _Visit(AllInstanceNodesSpecification const& specification) override {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _Visit(AllRelatedInstanceNodesSpecification const& specification) override
        {
        NavNodeCPtr parent = GetParentInstanceNode();
        if (parent.IsNull())
            return;

        DataSourceFilter::RelatedInstanceInfo relationshipInfo(m_query.GetResultParameters().GetMatchingRelationshipIds(),
            specification.GetRequiredRelationDirection(), parent->GetKey()->AsECInstanceNodeKey()->GetInstanceId());
        m_filter = DataSourceFilter(relationshipInfo, nullptr);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _Visit(CustomNodeSpecification const& specification) override {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _Visit(InstanceNodesOfSpecificClassesSpecification const& specification) override {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _Visit(RelatedInstanceNodesSpecification const& specification) override
        {
        NavNodeCPtr parent = GetParentInstanceNode();
        if (parent.IsNull())
            return;

        DataSourceFilter::RelatedInstanceInfo relationshipInfo(m_query.GetResultParameters().GetMatchingRelationshipIds(),
            specification.GetRequiredRelationDirection(), parent->GetKey()->AsECInstanceNodeKey()->GetInstanceId());
        m_filter = DataSourceFilter(relationshipInfo, nullptr);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _Visit(SearchResultInstanceNodesSpecification const& specification) override {}

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    SpecificationFiltersBuilder(IHierarchyCache const& nodesCache, NavNodeCP parent, NavigationQueryCR query)
        : m_nodesCache(nodesCache), m_parent(parent), m_query(query)
        {}
    DataSourceFilter GetFilter() const {return m_filter;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static DataSourceFilter GetSpecificationFilter(IHierarchyCache const& nodesCache, NavNodeCP parent, NavigationQueryCR query)
    {
    if (nullptr == query.GetResultParameters().GetSpecification())
        return DataSourceFilter();

    SpecificationFiltersBuilder filtersBuilder(nodesCache, parent, query);
    query.GetResultParameters().GetSpecification()->Accept(filtersBuilder);
    return filtersBuilder.GetFilter();
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct QueryBasedNodesProvider::Savepoint
    {
    QueryBasedNodesProvider& m_provider;
    IHierarchyCache::SavepointPtr m_cacheSavepoint;
    Savepoint(QueryBasedNodesProvider& provider)
        : m_provider(provider), m_cacheSavepoint(provider.GetContext().GetNodesCache().CreateSavepoint())
        {}
    void Cancel()
        {
        m_cacheSavepoint->Cancel();
        m_provider.m_executorIndex = 0;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool NodesCountMightChange(IHierarchyCache const& nodesCache, NavNodeCP parent, NavigationQueryCR query)
    {
    // first do the cheapest checks for 'hide nodes' flags
    bool hasHideFlags = query.GetResultParameters().GetNavNodeExtendedData().HideIfGroupingValueNotSpecified()
        || query.GetResultParameters().GetNavNodeExtendedData().HideIfNoChildren()
        || query.GetResultParameters().GetNavNodeExtendedData().HideIfOnlyOneChild()
        || query.GetResultParameters().GetNavNodeExtendedData().HideNodesInHierarchy()
        || query.GetResultParameters().GetNavNodeExtendedData().HasHideExpression();
    if (hasHideFlags)
        return true;

    // we may also need to hide nodes if they're being created by the same specification as
    // one of the parent nodes
    Utf8CP specHash = query.GetResultParameters().GetNavNodeExtendedData().GetSpecificationHash();
    NavNodeCPtr currNode = parent;
    while (currNode.IsValid())
        {
        if (0 == strcmp(NavNodeExtendedData(*currNode).GetSpecificationHash(), specHash))
            return true;
        currNode = (0 != currNode->GetParentNodeId()) ? nodesCache.GetNode(currNode->GetParentNodeId()) : nullptr;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryBasedNodesProvider::InitializeProvidersFromCache()
    {
    if (!GetNodeProviders().empty())
        return true;

    NavNodesProviderPtr cachedProvider = GetCachedProvider();
    if (cachedProvider.IsValid())
        {
        DisabledFullNodesLoadContext doNotCustomizeCachedNodes(*cachedProvider);
        size_t cachedIndex = 0;
        JsonNavNodePtr cachedNode;
        while (cachedProvider->GetNode(cachedNode, cachedIndex++))
            {
            NavNodesProviderContextPtr nestedContext = NavNodesProviderContext::Create(GetContext());
            NavNodeExtendedData cachedNodeExtendedData(*cachedNode);
            if (GetContext().IsChildNodeContext() && HasSimilarNodeInHierarchy(*cachedNode, cachedNodeExtendedData.HasVirtualParentId() ? cachedNodeExtendedData.GetVirtualParentId() : 0))
                AddProvider(*EmptyNavNodesProvider::Create(*nestedContext));
            else if (NodeVisibility::Virtual != GetContext().GetNodesCache().GetNodeVisibility(cachedNode->GetNodeId()))
                AddProvider(*SingleNavNodeProvider::Create(*cachedNode, *nestedContext));
            else
                AddProvider(*GetContext().CreateHierarchyLevelProvider(*CreateContextForChildHierarchyLevel(GetContext(), *cachedNode), cachedNode.get()));
            }
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryBasedNodesProvider::InitializeProvidersForAllNodes()
    {
    RefCountedPtr<PerformanceLogger> _l = LoggingHelper::CreatePerformanceLogger(Log::Navigation, "[QueryBasedNodesProvider] Initialize", NativeLogging::LOG_TRACE);
    if (!GetNodeProviders().empty())
        return true;

    // create a savepoint so we can roll back if canceled
    Savepoint savepoint(*this);

    EnsureQueryRecordsRead();
    if (GetContext().GetCancelationToken().IsCanceled())
        {
        savepoint.Cancel();
        LoggingHelper::LogMessage(Log::Navigation, "[QueryBasedNodesProvider] Initialization canceled", NativeLogging::LOG_DEBUG);
        return false;
        }

    // create providers for each node
    JsonNavNodePtr node;
    GetNodeProvidersR().reserve(m_executor.GetNodesCount());
    while ((node = m_executor.GetNode(m_executorIndex)).IsValid())
        {
        GetContext().GetNodesCache().Cache(*node, GetContext().GetDataSourceInfo(), m_executorIndex + m_offset, false);
        if (GetContext().GetCancelationToken().IsCanceled())
            {
            savepoint.Cancel();
            LoggingHelper::LogMessage(Log::Navigation, "[QueryBasedNodesProvider] Initialization canceled", NativeLogging::LOG_DEBUG);
            return false;
            }

        // note: determining children articats must be done _after_ caching the parent node
        // but _before_ it's provider is created (because creating children provider might
        // require already having the artifacts)
        if (NavNodeExtendedData(*node).SetChildrenArtifacts(GetChildrenArtifacts(*node)))
            GetContext().GetNodesCache().Update(node->GetNodeId(), *node);

        NavNodesProviderPtr provider = CreateProvider(*node);
        AddProvider(*provider);
        ++m_executorIndex;
        }

    GetContext().GetNodesCache().FinalizeInitialization(GetContext().GetDataSourceInfo());

    LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[QueryBasedNodesProvider] Created node providers for %" PRIu64 " nodes", (uint64_t)m_executorIndex).c_str(), NativeLogging::LOG_DEBUG);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryBasedNodesProvider::InitializeProvidersForPagedQueries(size_t nodesCount, size_t pageSize)
    {
    size_t pagesCount = nodesCount / pageSize + 1;
    for (size_t i = 0; i < pagesCount; ++i)
        {
        ComplexNavigationQueryPtr pageQuery = ComplexNavigationQuery::Create();
        pageQuery->SelectAll();
        pageQuery->From(*m_query->Clone());
        pageQuery->Limit(pageSize, pageSize * i);

        NavNodesProviderContextPtr nestedContext = CreateContextForSameHierarchyLevel(GetContext(), true);
        RefCountedPtr<QueryBasedNodesProvider> provider = QueryBasedNodesProvider::Create(*nestedContext, *pageQuery, m_usedClassIds);
        provider->SetNodesCount((i < pagesCount) ? pageSize : (nodesCount % pageSize));
        AddProvider(*provider);
        }
    return true;
    }

#define PAGED_QUERY_THRESHOLD 1000
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryBasedNodesProvider::_InitializeNodes()
    {
    if (!MultiNavNodesProvider::_InitializeNodes())
        return false;

    if (GetContext().GetNodesCache().IsInitialized(GetContext().GetDataSourceInfo()))
        return InitializeProvidersFromCache();

    InitializeDataSource();

    if (NodesCountMightChange(GetContext().GetNodesCache(), GetContext().GetVirtualParentNode().get(),  *m_query) || GetNodesCount() <= PAGED_QUERY_THRESHOLD)
        return InitializeProvidersForAllNodes();

    return InitializeProvidersForPagedQueries(GetNodesCount(), PAGED_QUERY_THRESHOLD);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBasedNodesProvider::InitializeDataSource()
    {
    DataSourceFilter dsFilter(GetSpecificationFilter(GetContext().GetNodesCache(), GetContext().GetVirtualParentNode().get(), *m_query));
    bvector<UserSettingEntry> relatedSettings = GetContext().GetRelatedSettings();
    GetContext().GetNodesCache().Update(GetContext().GetDataSourceInfo(), &dsFilter, &m_usedClassIds, &relatedSettings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBasedNodesProvider::EnsureQueryRecordsRead() const
    {
    if (m_executor.IsReadFinished())
        return;

    JsonNavNodeCPtr virtualParent = GetContext().GetVirtualParentNode();
    DataSourceRelatedSettingsUpdater updater(GetContext(), virtualParent.get());

    // set up the custom functions context
    CustomFunctionsContext fnContext(GetContext().GetSchemaHelper(), GetContext().GetConnections(), GetContext().GetConnection(), GetContext().GetRuleset(),
        GetContext().GetLocale(), GetContext().GetUserSettings(), &GetContext().GetUsedSettingsListener(), GetContext().GetECExpressionsCache(),
        GetContext().GetNodesFactory(), GetContext().GetUsedClassesListener(), virtualParent.get(), &m_query->GetExtendedData());
    if (GetContext().IsLocalizationContext())
        fnContext.SetLocalizationProvider(GetContext().GetLocalizationProvider());

    // read the nodes
    m_executor.ReadRecords(&GetContext().GetCancelationToken());

    // assign keys
    size_t index = 0;
    JsonNavNodePtr node;
    while ((node = m_executor.GetNode(index++)).IsValid())
        {
        if (node->HasKey())
            continue;

        NavNodeExtendedData extendedData(*node);
        extendedData.SetRulesetId(GetContext().GetRuleset().GetRuleSetId().c_str());
        if (nullptr != GetContext().GetPhysicalParentNodeId())
            node->SetParentNodeId(*GetContext().GetPhysicalParentNodeId());
        if (nullptr != GetContext().GetVirtualParentNodeId())
            extendedData.SetVirtualParentId(*GetContext().GetVirtualParentNodeId());
        if (GetContext().IsRootNodeContext() && GetContext().GetRootNodeRule().GetAutoExpand())
            node->SetIsExpanded(true);

        node->SetNodeKey(*NavNodesHelper::CreateNodeKey(GetContext().GetConnection(), *node, virtualParent.IsValid() ? virtualParent->GetKey().get() : nullptr));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryBasedNodesProvider::_GetNode(JsonNavNodePtr& node, size_t index) const
    {
    InitializeNodes();

    if (!MultiNavNodesProvider::_GetNode(node, index))
        return false;

    NavNodeExtendedData extendedData(*node);
    if (extendedData.HideIfOnlyOneChild() && extendedData.HasGroupingType() && 1 == extendedData.GetGroupedInstanceKeysCount())
        {
        // note: for grouping nodes that have the HideIfOnlyOneChild flag and group only one node,
        // we may need to create a children provider and returns its node
        uint64_t nodeId = node->GetNodeId();
        HierarchyLevelInfo hl = GetContext().GetNodesCache().FindHierarchyLevel(GetContext().GetConnection().GetId().c_str(),
            GetContext().GetRuleset().GetRuleSetId().c_str(),
            GetContext().IsLocalizationContext() ? GetContext().GetLocale().c_str() : "", &nodeId);
        if (!hl.IsValid())
            {
            BeAssert(false);
            return false;
            }
        if (!GetContext().GetNodesCache().IsInitialized(hl))
            {
            NavNodesProviderPtr childrenProvider = GetContext().CreateHierarchyLevelProvider(*CreateContextForChildHierarchyLevel(GetContext(), *node), node.get());
            return childrenProvider->GetNode(node, 0);
            }
        }

    FinalizeNode(*node, false);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryBasedNodesProvider::_HasNodes() const
    {
    return GetNodesCount() > 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t QueryBasedNodesProvider::_GetNodesCount() const
    {
    if (GetContext().GetNodesCache().IsInitialized(GetContext().GetDataSourceInfo()))
        {
        const_cast<QueryBasedNodesProvider*>(this)->InitializeProvidersFromCache();
        return MultiNavNodesProvider::_GetNodesCount();
        }

    const_cast<QueryBasedNodesProvider*>(this)->InitializeDataSource();
    JsonNavNodeCPtr virtualParent = GetContext().GetVirtualParentNode();

    if (NodesCountMightChange(GetContext().GetNodesCache(), virtualParent.get(), *m_query))
        {
        const_cast<QueryBasedNodesProvider*>(this)->InitializeProvidersForAllNodes();
        return MultiNavNodesProvider::_GetNodesCount();
        }

    DataSourceRelatedSettingsUpdater updater(GetContext(), virtualParent.get());

    // run a separate query to get the total nodes count (we already know this count
    // won't change during post-processing)
    CustomFunctionsContext fnContext(GetContext().GetSchemaHelper(), GetContext().GetConnections(), GetContext().GetConnection(),
        GetContext().GetRuleset(), GetContext().GetLocale(), GetContext().GetUserSettings(), &GetContext().GetUsedSettingsListener(),
        GetContext().GetECExpressionsCache(), GetContext().GetNodesFactory(), GetContext().GetUsedClassesListener(), virtualParent.get(), &m_query->GetExtendedData());
    if (GetContext().IsLocalizationContext())
        fnContext.SetLocalizationProvider(GetContext().GetLocalizationProvider());

    NavigationQueryPtr queryNotSorted = m_query->Clone();
    QueryBuilderHelpers::Order(*queryNotSorted, "");

    RefCountedPtr<CountQueryContract> contract = CountQueryContract::Create();
    ComplexGenericQueryPtr countQuery = ComplexGenericQuery::Create();
    countQuery->SelectContract(*contract);
    countQuery->GroupByContract(*contract);
    countQuery->From(*StringGenericQuery::Create(queryNotSorted->ToString(), queryNotSorted->GetBoundValues()));

    CountQueryExecutor executor(GetContext().GetConnection(), GetContext().GetStatementCache(), *countQuery);
    executor.ReadRecords(&GetContext().GetCancelationToken());
    return executor.GetResult();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NodeArtifacts> QueryBasedNodesProvider::_GetArtifacts() const
    {
    EnsureQueryRecordsRead();
    bvector<NodeArtifacts> result;
    JsonNavNodePtr node;
    size_t index = 0;
    while ((node = m_executor.GetNode(index++)).IsValid())
        result.push_back(CustomizationHelper::EvaluateArtifacts(GetContext(), *node));
    return result;
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct SpecificationUsedClassesListener : IUsedClassesListener
    {
    NavNodesProviderContextR m_context;
    bmap<ECClassId, bool> m_usedClassIds;

    SpecificationUsedClassesListener(NavNodesProviderContextR ctx)
        : m_context(ctx)
        {
        m_context.GetQueryBuilder().GetParameters().SetUsedClassesListener(this);
        }
    ~SpecificationUsedClassesListener()
        {
        m_context.GetQueryBuilder().GetParameters().SetUsedClassesListener(m_context.GetUsedClassesListener());
        }
    bmap<ECClassId, bool> const& GetUsedClassIds() const {return m_usedClassIds;}

    void _OnClassUsed(ECClassCR ecClass, bool polymorphically) override
        {
        m_usedClassIds[ecClass.GetId()] = polymorphically;

        if (nullptr != m_context.GetUsedClassesListener())
            m_context.GetUsedClassesListener()->_OnClassUsed(ecClass, polymorphically);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
QueryBasedSpecificationNodesProvider::QueryBasedSpecificationNodesProvider(NavNodesProviderContextCR context, ChildNodeSpecificationCR specification)
    : MultiNavNodesProvider(context), m_specification(specification)
    {
    SpecificationUsedClassesListener usedClasses(GetContextR());
    bvector<NavigationQueryPtr> queries = CreateQueries(m_specification);
    for (NavigationQueryPtr const& query : queries)
        {
        NavNodesProviderContextPtr nestedContext = CreateContextForSameHierarchyLevel(GetContext(), true);
        AddProvider(*QueryBasedNodesProvider::Create(*nestedContext, *query, usedClasses.GetUsedClassIds()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavigationQueryPtr> QueryBasedSpecificationNodesProvider::CreateQueries(ChildNodeSpecificationCR specification) const
    {
    BeAssert(GetContext().IsQueryContext());

    RefCountedPtr<PerformanceLogger> _l = LoggingHelper::CreatePerformanceLogger(Log::Navigation, "[QueryBasedSpecificationNodesProvider] Creating queries for specification", NativeLogging::LOG_TRACE);

    if (GetContext().IsRootNodeContext())
        return GetContext().GetQueryBuilder().GetQueries(GetContext().GetRootNodeRule(), specification);

    if (GetContext().IsChildNodeContext())
        {
        if (nullptr == GetContext().GetChildNodeRule())
            {
            BeAssert(false);
            return bvector<NavigationQueryPtr>();
            }
        JsonNavNodeCPtr parent = GetContext().GetVirtualParentNode();
        if (parent.IsNull())
            {
            BeAssert(false);
            return bvector<NavigationQueryPtr>();
            }
        return GetContext().GetQueryBuilder().GetQueries(*GetContext().GetChildNodeRule(), specification, *parent);
        }

    BeAssert(false);
    return bvector<NavigationQueryPtr>();
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct SpecificationChildrenChecker : PresentationRuleSpecificationVisitor
    {
    private:
        NavNodesProviderContext const& m_context;
        bool m_result = false;

    private:
        bool AlwaysHasRelatedInstance(SupportedRelationshipClassInfo const& relationshipInfo, RelatedInstanceNodesSpecification const& spec) const
            {
            ECRelationshipClassCR relationshipClass = relationshipInfo.GetClass();
            if (RequiredRelationDirection::RequiredRelationDirection_Both == spec.GetRequiredRelationDirection())
                {
                // If either source or target has multiplicity higher than 0, return true
                bool bothConstraintsMustMatch = (spec.GetSkipRelatedLevel() > 0);
                ECRelationshipConstraintCR sourceConstraint = relationshipClass.GetSource();
                ECRelationshipConstraintCR targetConstraint = relationshipClass.GetTarget();
                if (bothConstraintsMustMatch && (sourceConstraint.GetMultiplicity().GetLowerLimit() > 0 && targetConstraint.GetMultiplicity().GetLowerLimit() > 0)
                    || !bothConstraintsMustMatch && (sourceConstraint.GetMultiplicity().GetLowerLimit() > 0 || targetConstraint.GetMultiplicity().GetLowerLimit() > 0))
                    return true;
                }
            else
                {
                // If target has multiplicity higher than 0, return true
                // Note. If both relationship and rule are reversed or both are not reversed, target = rel.Target. Otherwise target = rel.Source.
                bool ruleDirectionForward = (spec.GetRequiredRelationDirection() == RequiredRelationDirection::RequiredRelationDirection_Forward);
                bool relDirectionForward = (relationshipClass.GetStrengthDirection() == ECRelatedInstanceDirection::Forward);
                ECRelationshipConstraintCR targetConstraint = (ruleDirectionForward == relDirectionForward) ? relationshipClass.GetTarget() : relationshipClass.GetSource();
                if (targetConstraint.GetMultiplicity().GetLowerLimit() > 0)
                    return true;
                }
            return false;
            }
        void DetermineChildren(RelatedInstanceNodesSpecification const& spec)
            {
            if (!spec.GetInstanceFilter().empty())
                {
                // note: if there's an instance filter then we can't tell if the node always has children..
                return;
                }
            SupportedRelationshipClassInfos relationships = m_context.GetSchemaHelper().GetECRelationshipClasses(spec.GetRelationshipClassNames());
            if (spec.GetSkipRelatedLevel() == 0)
                m_result = std::any_of(relationships.begin(), relationships.end(), [&](SupportedRelationshipClassInfo const& r) {return AlwaysHasRelatedInstance(r, spec); });
            else
                m_result = std::all_of(relationships.begin(), relationships.end(), [&](SupportedRelationshipClassInfo const& r) {return AlwaysHasRelatedInstance(r, spec); });
            }

    protected:
        void _Visit(RelatedInstanceNodesSpecification const& spec) override { DetermineChildren(spec); }

    public:
        SpecificationChildrenChecker(NavNodesProviderContextCR context) : m_context(context) {}
        bool AlwaysHasChildren() const { return m_result; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryBasedSpecificationNodesProvider::_HasNodes() const
    {
    SpecificationChildrenChecker visitor(GetContext());
    m_specification.Accept(visitor);
    if (visitor.AlwaysHasChildren())
        return true;

    return MultiNavNodesProvider::_HasNodes();
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2019
+===============+===============+===============+===============+===============+======*/
struct NodeArtifactsFilter : CustomizationRuleVisitor
{
private:
    bvector<NodeArtifactsRuleCP> m_filtered;
    NodeArtifactsFilter() {};
protected:
    void _Visit(NodeArtifactsRuleCR rule) override { m_filtered.push_back(&rule); }
public:
    static bvector<NodeArtifactsRuleCP> Apply(bvector<CustomizationRuleCP> const& rules)
        {
        NodeArtifactsFilter filter;
        for (CustomizationRuleCP rule : rules)
            rule->Accept(filter);
        return filter.m_filtered;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool HasNodeArtifactRules(PresentationRuleSetCR ruleset, ChildNodeSpecificationCR spec)
    {
    auto customizationRules = RulesPreprocessor::GetCustomizationRulesForSpecification(ruleset, spec, &PresentationRuleSet::GetNodeArtifactRules);
    return !NodeArtifactsFilter::Apply(customizationRules).empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NodeArtifacts> QueryBasedSpecificationNodesProvider::_GetArtifacts() const
    {
    if (!HasNodeArtifactRules(GetContext().GetRuleset(), m_specification))
        {
        // avoid calling base artifacts request if we can see there're no artifact presentation rules
        // that apply to nodes created by this specification
        return bvector<NodeArtifacts>();
        }

    return MultiNavNodesProvider::_GetArtifacts();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MultiSpecificationNodesProvider::MultiSpecificationNodesProvider(NavNodesProviderContextR baseContext, RootNodeRuleSpecificationsList const& specs)
    : MultiNavNodesProvider(baseContext)
    {
    SpecificationsVisitor visitor;
    for (RootNodeRuleSpecification const& specification : specs)
        {
        NavNodesProviderContextPtr nestedContext = CreateContextForSameHierarchyLevel(GetContext(), false);
        nestedContext->SetRootNodeContext(specification.GetRule());
        visitor.SetContext(*nestedContext);
        specification.GetSpecification().Accept(visitor);
        }
    SetProviders(visitor.GetNodeProviders());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MultiSpecificationNodesProvider::MultiSpecificationNodesProvider(NavNodesProviderContextR baseContext, ChildNodeRuleSpecificationsList const& specs, NavNodeCR virtualParent)
    : MultiNavNodesProvider(baseContext)
    {
    SpecificationsVisitor visitor;
    for (ChildNodeRuleSpecification const& specification : specs)
        {
        NavNodesProviderContextPtr nestedContext = CreateContextForSameHierarchyLevel(GetContext(), false);
        nestedContext->SetChildNodeContext(&specification.GetRule(), virtualParent);
        visitor.SetContext(*nestedContext);
        specification.GetSpecification().Accept(visitor);
        }
    SetProviders(visitor.GetNodeProviders());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultiSpecificationNodesProvider::_GetNode(JsonNavNodePtr& node, size_t index) const
    {
    if (m_replaceProvider.IsValid())
        return m_replaceProvider->GetNode(node, index);

    if (MultiNavNodesProvider::_GetNode(node, index))
        {
        if (0 == index && node->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode) && 1 == MultiNavNodesProvider::_GetNodesCount())
            {
            GetContext().GetNodesCache().MakeVirtual(*node);
            m_replaceProvider = GetContext().CreateHierarchyLevelProvider(*CreateContextForChildHierarchyLevel(GetContext(), *node), node.get());
            return m_replaceProvider->GetNode(node, 0);
            }
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MultiSpecificationNodesProvider::_GetNodesCount() const
    {
    // note: if parent node is a grouping node, we can make some assumptions based on
    // grouped instances count and avoid having to query for nodes count
    JsonNavNodeCPtr virtualParent = GetContext().GetVirtualParentNode();
    if (virtualParent.IsValid())
        {
        NavNodeExtendedData parentExtendedData(*virtualParent);
        if (parentExtendedData.HasGroupingType())
            {
            if (0 == parentExtendedData.GetGroupedInstanceKeysCount())
                return 0;
            if (1 == parentExtendedData.GetGroupedInstanceKeysCount())
                return 1;
            }
        }

    if (m_replaceProvider.IsValid())
        return m_replaceProvider->GetNodesCount();

    size_t count = MultiNavNodesProvider::_GetNodesCount();
    JsonNavNodePtr node;
    DisabledFullNodesLoadContext doNotCustomize(*this);
    if (1 == count && MultiNavNodesProvider::_GetNode(node, 0) && node->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode))
        {
        GetContext().GetNodesCache().MakeVirtual(*node);
        m_replaceProvider = GetContext().CreateHierarchyLevelProvider(*CreateContextForChildHierarchyLevel(GetContext(), *node), node.get());
        return m_replaceProvider->GetNodesCount();
        }
    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultiNavNodesProvider::_GetNode(JsonNavNodePtr& node, size_t index) const
    {
    for (NavNodesProviderPtr const& provider : m_providers)
        {
        if (provider.IsNull())
            continue;

        OptimizationFlagsCarrier useOptimizationFlags(*this, *provider);
        size_t count = provider->GetNodesCount();
        if (index < count)
            return provider->GetNode(node, index);
        index -= count;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultiNavNodesProvider::_HasNodes() const
    {
    for (NavNodesProviderPtr const& provider : m_providers)
        {
        if (provider.IsNull())
            continue;

        OptimizationFlagsCarrier useOptimizationFlags(*this, *provider);
        if (provider->HasNodes())
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MultiNavNodesProvider::_GetNodesCount() const
    {
    size_t count = 0;
    for (NavNodesProviderPtr const& provider : m_providers)
        {
        if (provider.IsNull())
            continue;

        OptimizationFlagsCarrier useOptimizationFlags(*this, *provider);
        count += provider->GetNodesCount();

        if (GetContext().IsCheckingChildren())
            {
            if (count > 0)
                break;
            }
        }
    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NodeArtifacts> MultiNavNodesProvider::_GetArtifacts() const
    {
    bvector<NodeArtifacts> artifacts;
    for (NavNodesProviderPtr const& provider : m_providers)
        {
        if (provider.IsNull())
            continue;

        OptimizationFlagsCarrier useOptimizationFlags(*this, *provider);
        bvector<NodeArtifacts> providerArtifacts = provider->GetArtifacts();
        std::move(providerArtifacts.begin(), providerArtifacts.end(), std::back_inserter(artifacts));
        }
    return artifacts;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SQLiteCacheNodesProvider::SQLiteCacheNodesProvider(NavNodesProviderContextCR context, Db& cache, StatementCache& statements)
    : NavNodesProvider(context), m_nodes(nullptr), m_nodesCount(nullptr), m_cache(cache), m_statements(statements)
    {
    InitializeUsedSettings();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SQLiteCacheNodesProvider::~SQLiteCacheNodesProvider()
    {
    DELETE_AND_CLEAR(m_nodes);
    DELETE_AND_CLEAR(m_nodesCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void SQLiteCacheNodesProvider::InitializeUsedSettings()
    {
    Utf8String query = "SELECT [us].[SettingId] "
                       "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                       "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id] "
                       "  JOIN [" NODESCACHE_TABLENAME_DataSourceSettings "] us ON [us].[DataSourceId] = [ds].[Id]"
                       " WHERE [hl].[VirtualParentNodeId] ";
    if (0 == GetContext().GetVirtualParentNodeId())
        query.append("IS NULL");
    else
        query.append(" = ?");
    query.append(" AND [hl].[ConnectionId] = ?");
    query.append(" AND [hl].[RulesetId] = ?");
    query.append(" AND [hl].[Locale] = ?");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }

    int bindingIndex = 1;
    if (nullptr != GetContext().GetVirtualParentNodeId())
        stmt->BindUInt64(bindingIndex++, *GetContext().GetVirtualParentNodeId());
    stmt->BindText(bindingIndex++, GetContext().GetConnection().GetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetLocale(), Statement::MakeCopy::No);

    while (BE_SQLITE_ROW == stmt->Step())
        GetContext().GetUsedSettingsListener()._OnUserSettingUsed(stmt->GetValueText(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void SQLiteCacheNodesProvider::InitializeNodes()
    {
    if (nullptr != m_nodes)
        return; // already initialized

    m_nodes = new bvector<JsonNavNodePtr>();
    CachedStatementPtr statement = _GetNodesStatement();
    if (!statement.IsValid())
        return;

    while (BE_SQLITE_ROW == statement->Step())
        {
        Utf8CP serializedNode = statement->GetValueText(0);
        if (nullptr == serializedNode || 0 == *serializedNode)
            {
            BeAssert(false);
            continue;
            }

        rapidjson::Document json;
        json.Parse(serializedNode);
        JsonNavNodePtr node = GetContext().GetNodesFactory().CreateFromJson(GetContext().GetConnection(), std::move(json));
        if (node.IsNull())
            continue;

        if (!statement->IsColumnNull(1))
            node->SetParentNodeId(statement->GetValueUInt64(1));

        if (!statement->IsColumnNull(2))
            NavNodeExtendedData(*node).SetVirtualParentId(statement->GetValueUInt64(2));

        node->SetNodeId(statement->GetValueUInt64(3));
        node->SetIsExpanded(!statement->IsColumnNull(4));

        node->SetNodeKey(*NavNodesHelper::CreateNodeKey(GetContext().GetConnection(), *node, statement->GetValueText(5)));

        m_nodes->push_back(node);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool SQLiteCacheNodesProvider::_GetNode(JsonNavNodePtr& node, size_t index) const
    {
    const_cast<SQLiteCacheNodesProvider*>(this)->InitializeNodes();
    if (m_nodes->empty() || index > m_nodes->size() - 1)
        return false;

    node = m_nodes->at(index);
    FinalizeNode(*node, false);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool SQLiteCacheNodesProvider::_HasNodes() const
    {
    if (nullptr != m_nodes)
        return !m_nodes->empty();
    return GetNodesCount() > 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
size_t SQLiteCacheNodesProvider::_GetNodesCount() const
    {
    if (nullptr != m_nodes)
        return m_nodes->size();
    if (nullptr == m_nodesCount)
        {
        CachedStatementPtr stmt = _GetCountStatement();
        if (BE_SQLITE_ROW != stmt->Step())
            {
            BeAssert(false);
            m_nodesCount = new size_t(0);
            }
        m_nodesCount = new size_t(stmt->GetValueInt(0));
        }
    return *m_nodesCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NodeArtifacts> SQLiteCacheNodesProvider::_GetArtifacts() const
    {
    const_cast<SQLiteCacheNodesProvider*>(this)->InitializeNodes();
    bvector<NodeArtifacts> result;
    for (JsonNavNodeCPtr node : *m_nodes)
        result.push_back(CustomizationHelper::EvaluateArtifacts(GetContext(), *node));
    return result;
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct DatasourceIdSet : VirtualSet, bset<uint64_t>
    {
    bool _IsInSet(int nVals, DbValue const* vals) const override
        {
        uint64_t value = vals[0].GetValueUInt64();
        return end() != find(value);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedCombinedHierarchyLevelProvider::_GetNodesStatement() const
    {
    Utf8String query = "SELECT [n].[Data], [hl].[PhysicalParentNodeId], [hl].[VirtualParentNodeId], [n].[Id], [ex].[NodeId], [nk].[PathFromRoot] "
                       "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                       "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id]"
                       "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id]"
                       "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id]"
                       "  JOIN [" NODESCACHE_TABLENAME_NodesOrder "] no ON [no].[HierarchyLevelId] = [hl].[Id] AND [no].[DataSourceId] = [ds].[Id] AND [no].[NodeId] = [n].[Id]"
                       "  LEFT JOIN [" NODESCACHE_TABLENAME_ExpandedNodes "] ex ON [n].[Id] = [ex].[NodeId]"
                       " WHERE     NOT [n].[IsVirtual] AND [hl].[RemovalId] IS NULL "
                       "       AND [hl].[ConnectionId] = ? AND [hl].[RulesetId] = ? AND [hl].[Locale] = ? AND [hl].[PhysicalParentNodeId] ";
    if (nullptr == m_info.GetPhysicalParentNodeId())
        query.append("IS NULL");
    else
        query.append(" = ?");
    query.append(" ORDER BY [no].[OrderValue]");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindText(1, m_info.GetConnectionId(), Statement::MakeCopy::No);
    stmt->BindText(2, m_info.GetRulesetId(), Statement::MakeCopy::No);
    stmt->BindText(3, m_info.GetLocale(), Statement::MakeCopy::No);
    if (nullptr != m_info.GetPhysicalParentNodeId())
        stmt->BindUInt64(4, *m_info.GetPhysicalParentNodeId());

    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedCombinedHierarchyLevelProvider::_GetCountStatement() const
    {
    Utf8String query = "SELECT COUNT(1) "
                       "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                       "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id]"
                       "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id]"
                       " WHERE     NOT [n].[IsVirtual] AND [hl].[RemovalId] IS NULL "
                       "       AND [hl].[ConnectionId] = ? AND [hl].[RulesetId] = ? AND [hl].[Locale] = ? AND [hl].[PhysicalParentNodeId] ";
    if (nullptr == m_info.GetPhysicalParentNodeId())
        query.append("IS NULL");
    else
        query.append(" = ?");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindText(1, m_info.GetConnectionId(), Statement::MakeCopy::No);
    stmt->BindText(2, m_info.GetRulesetId(), Statement::MakeCopy::No);
    stmt->BindText(3, m_info.GetLocale(), Statement::MakeCopy::No);
    if (nullptr != m_info.GetPhysicalParentNodeId())
        stmt->BindUInt64(4, *m_info.GetPhysicalParentNodeId());

    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedHierarchyLevelProvider::_GetNodesStatement() const
    {
    Utf8CP query = "SELECT [n].[Data], [hl].[PhysicalParentNodeId], [hl].[VirtualParentNodeId], [n].[Id], [ex].[NodeId], [nk].[PathFromRoot] "
                   "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                   "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id]"
                   "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id]"
                   "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id]"
                   "  JOIN [" NODESCACHE_TABLENAME_NodesOrder "] no ON [no].[HierarchyLevelId] = [hl].[Id] AND [no].[DataSourceId] = [ds].[Id] AND [no].[NodeId] = [n].[Id]"
                   "  LEFT JOIN [" NODESCACHE_TABLENAME_ExpandedNodes "] ex ON [n].[Id] = [ex].[NodeId]"
                   " WHERE NOT [n].[IsVirtual] AND [hl].[Id] = ? "
                   " ORDER BY [no].[OrderValue]";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindUInt64(1, m_hierarchyLevelId);

    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedHierarchyLevelProvider::_GetCountStatement() const
    {
    Utf8CP query = "SELECT COUNT(1) "
                   "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                   "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id]"
                   "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id]"
                   " WHERE NOT [n].[IsVirtual] AND [hl].[Id] = ? ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindUInt64(1, m_hierarchyLevelId);

    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedPartialDataSourceProvider::_GetNodesStatement() const
    {
    Utf8CP query = "SELECT [n].[Data], [hl].[PhysicalParentNodeId], [hl].[VirtualParentNodeId], [n].[Id], [ex].[NodeId], [nk].[PathFromRoot] "
                   "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                   "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id]"
                   "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id]"
                   "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id]"
                   "  JOIN [" NODESCACHE_TABLENAME_NodesOrder "] no ON [no].[HierarchyLevelId] = [hl].[Id] AND [no].[DataSourceId] = [ds].[Id] AND [no].[NodeId] = [n].[Id]"
                   "  LEFT JOIN [" NODESCACHE_TABLENAME_ExpandedNodes "] ex ON [n].[Id] = [ex].[NodeId]"
                   " WHERE [ds].[Id] = ? "
                   " ORDER BY [no].[OrderValue]";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindUInt64(1, m_dataSourceId);

    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedPartialDataSourceProvider::_GetCountStatement() const
    {
    Utf8CP query = "SELECT COUNT(1) "
                   "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
                   "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id]"
                   " WHERE [ds].[Id] = ? ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindUInt64(1, m_dataSourceId);

    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NodesWithUndeterminedChildrenProvider::NodesWithUndeterminedChildrenProvider(NavNodesProviderContextCR context, Db& cache, StatementCache& statements)
    : SQLiteCacheNodesProvider(context, cache, statements)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr NodesWithUndeterminedChildrenProvider::_GetCountStatement() const
    {
    Utf8String query = "   SELECT COUNT(1) "
                       "     FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hln "
                       "     JOIN [" NODESCACHE_TABLENAME_DataSources "] dsn ON [dsn].[HierarchyLevelId] = [hln].[Id] "
                       "     JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [dsn].[Id] "
                       "LEFT JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[VirtualParentNodeId] = [n].[Id] "
                       "LEFT JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id] "
                       "    WHERE     [hln].[ConnectionId] = ? AND [hln].[RulesetId] = ? AND [hln].[Locale] = ? "
                       "          AND ([hl].[Id] IS NULL OR [ds].[Id] IS NULL OR NOT [ds].[IsInitialized]) ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindText(1, GetContext().GetConnection().GetId(), Statement::MakeCopy::No);
    stmt->BindText(2, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(3, GetContext().GetLocale(), Statement::MakeCopy::No);
    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr NodesWithUndeterminedChildrenProvider::_GetNodesStatement() const
    {
    Utf8String query = "   SELECT [n].[Data], [hln].[PhysicalParentNodeId], [hln].[VirtualParentNodeId], [n].[Id], [ex].[NodeId], [nk].[PathFromRoot] "
                       "     FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hln "
                       "     JOIN [" NODESCACHE_TABLENAME_DataSources "] dsn ON [dsn].[HierarchyLevelId] = [hln].[Id] "
                       "     JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [dsn].[Id] "
                       "     JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id] "
                       "LEFT JOIN [" NODESCACHE_TABLENAME_ExpandedNodes "] ex ON [n].[Id] = [ex].[NodeId] "
                       "LEFT JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[VirtualParentNodeId] = [n].[Id] "
                       "LEFT JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id] "
                       "    WHERE     [hln].[ConnectionId] = ? AND [hln].[RulesetId] = ? AND [hln].[Locale] = ? "
                       "          AND ([hl].[Id] IS NULL OR [ds].[Id] IS NULL OR NOT [ds].[IsInitialized]) ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindText(1, GetContext().GetConnection().GetId(), Statement::MakeCopy::No);
    stmt->BindText(2, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(3, GetContext().GetLocale(), Statement::MakeCopy::No);
    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
FilteredNodesProvider::FilteredNodesProvider(NavNodesProviderContextCR context, Db& cache, StatementCache& statements, Utf8String filter)
    : SQLiteCacheNodesProvider(context, cache, statements), m_filter(filter)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr FilteredNodesProvider::_GetCountStatement() const
    {
    Utf8String query = "SELECT COUNT(1) "
        "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
        "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] k ON [k].[NodeId] = [n].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [n].[DataSourceId] "
        "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [ds].[HierarchyLevelId] "
        " WHERE [hl].[ConnectionId] = ? AND [hl].[RulesetId] = ? AND [hl].[Locale] = ? AND [n].[Label] LIKE ? ESCAPE \'\\\'";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    Utf8String filter;
    filter.append("%").append(m_filter).append("%");
    stmt->BindText(1, GetContext().GetConnection().GetId(), Statement::MakeCopy::No);
    stmt->BindText(2, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(3, GetContext().GetLocale().c_str(), Statement::MakeCopy::No);
    stmt->BindText(4, filter.c_str(), Statement::MakeCopy::Yes);
    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr FilteredNodesProvider::_GetNodesStatement() const
    {
    Utf8String query = "SELECT [n].[Data], [hl].[PhysicalParentNodeId], [hl].[VirtualParentNodeId], [n].[Id], [ex].[NodeId], [nk].[PathFromRoot] "
        "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
        "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [n].[DataSourceId] "
        "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [ds].[HierarchyLevelId] "
        "  LEFT JOIN [" NODESCACHE_TABLENAME_ExpandedNodes "] ex ON [n].[Id] = [ex].[NodeId] "
        " WHERE [hl].[ConnectionId] = ? AND [hl].[RulesetId] = ? AND [hl].[Locale] = ? AND [n].[Label] LIKE ? ESCAPE \'\\\'";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    Utf8String filter;
    filter.append("%").append(m_filter).append("%");
    stmt->BindText(1, GetContext().GetConnection().GetId(), Statement::MakeCopy::No);
    stmt->BindText(2, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(3, GetContext().GetLocale().c_str(), Statement::MakeCopy::No);
    stmt->BindText(4, filter.c_str(), Statement::MakeCopy::Yes);
    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool CachedNodeProvider::_InitializeNodes()
    {
    JsonNavNodePtr node = GetContext().GetNodesCache().GetNode(m_nodeId);
    FinalizeNode(*node, true);
    m_singleNodeProvider = SingleNavNodeProvider::Create(*node, GetContext());
    return true;
    }
