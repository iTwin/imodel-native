/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
#include <algorithm>
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
DataSourceRelatedVariablesUpdater::DataSourceRelatedVariablesUpdater(NavNodesProviderContextCR context, JsonNavNodeCP node)
    : m_context(context), m_node(node)
    {
    m_relatedVariablesCountBefore = m_context.GetRelatedRulesetVariables().size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceRelatedVariablesUpdater::~DataSourceRelatedVariablesUpdater()
    {
    bvector<RulesetVariableEntry> variables = m_context.GetRelatedRulesetVariables();
    if (variables.size() == m_relatedVariablesCountBefore)
        return;

    DataSourceIdentifier identifier = m_node ? m_context.GetNodesCache().FindDataSource(m_node->GetNodeId()).GetIdentifier() : m_context.GetDataSourceIdentifier();
    if (!identifier.IsValid())
        {
        BeAssert(false);
        return;
        }

    DataSourceInfo info(identifier, RulesetVariables(variables), DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
    m_context.GetNodesCache().Update(info, DataSourceInfo::PART_Vars);
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                05/2020
+===============+===============+===============+===============+===============+======*/
struct NoDerivedOptimizationFlagsContext
{
private:
    NavNodesProviderContextR m_context;
    OptimizationFlagsContainer const* m_prevParentContainer;
public:
    NoDerivedOptimizationFlagsContext(NavNodesProviderCR provider)
        : m_context(provider.GetContextR())
        {
        m_prevParentContainer = m_context.GetOptimizationFlags().GetParentContainer();
        m_context.GetOptimizationFlags().SetParentContainer(nullptr);
        }
    ~NoDerivedOptimizationFlagsContext()
        {
        m_context.GetOptimizationFlags().SetParentContainer(m_prevParentContainer);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<uint64_t> ProvidersIndexAllocator::_GetCurrentIndex() const
    {
    bvector<uint64_t> index = m_parentIndex;
    index.push_back(m_currIndex - 1);
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<uint64_t> ProvidersIndexAllocator::_AllocateIndex()
    {
    bvector<uint64_t> index = m_parentIndex;
    index.push_back(m_currIndex++);
    return index;
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

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2019
+===============+===============+===============+===============+===============+======*/
struct ChildrenArtifactsCaptureContext
{
private:
    RefCountedPtr<ArtifactsCapturer> m_capturer;
    NavNodesProviderContextR m_childrenProviderContext;
    JsonNavNodeR m_node;
public:
    ChildrenArtifactsCaptureContext(NavNodesProviderContextR childrenProviderContext, JsonNavNodeR node, bool enabled)
        : m_childrenProviderContext(childrenProviderContext), m_node(node)
        {
        if (enabled)
            {
            m_capturer = ArtifactsCapturer::Create();
            m_childrenProviderContext.AddArtifactsCapturer(m_capturer.get());
            }
        }
    ~ChildrenArtifactsCaptureContext()
        {
        if (m_capturer.IsValid())
            {
            NavNodeExtendedData(m_node).SetChildrenArtifacts(m_capturer->GetArtifacts());
            m_childrenProviderContext.RemoveArtifactsCapturer(m_capturer.get());
            }
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderContext::NavNodesProviderContext(PresentationRuleSetCR ruleset, RuleTargetTree targetTree, Utf8String locale, uint64_t const* physicalParentId,
    std::unique_ptr<RulesetVariables> rulesetVariables, ECExpressionsCache& ecexpressionsCache, RelatedPathsCache& relatedPathsCache, PolymorphicallyRelatedClassesCache& polymorphicallyRelatedClassesCache,
    JsonNavNodesFactory const& nodesFactory, IHierarchyCache& nodesCache, INodesProviderFactoryCR providerFactory, IJsonLocalState const* localState)
    : RulesDrivenProviderContext(ruleset, locale, std::move(rulesetVariables), ecexpressionsCache, relatedPathsCache, polymorphicallyRelatedClassesCache, nodesFactory, localState),
    m_targetTree(targetTree), m_nodesCache(&nodesCache), m_providerFactory(providerFactory)
    {
    Init();

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

    if (nullptr != other.m_physicalParentNodeId)
        m_physicalParentNodeId = new uint64_t(*other.m_physicalParentNodeId);

    if (other.IsQueryContext())
        SetQueryContext(other);

    if (other.HasPageSize())
        SetPageSize(other.GetPageSize());
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
    m_isRootNodeContext = false;
    m_rootNodeRule = nullptr;
    m_isChildNodeContext = false;
    m_physicalParentNodeId = nullptr;
    m_virtualParentNodeId = nullptr;
    m_childNodeRule = nullptr;
    m_queryBuilder = nullptr;
    m_usedClassesListener = nullptr;
    m_hasPageSize = false;
    m_pageSize = 0;
    m_mayHaveArtifacts = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::InitProvidersIndexAllocator(uint64_t const* virtualParentNodeId)
    {
    if (nullptr != virtualParentNodeId)
        {
        bvector<uint64_t> parentIndex = m_nodesCache->GetNodeIndex(*virtualParentNodeId);
        if (!parentIndex.empty())
            m_providersIndexAllocator = new ProvidersIndexAllocator(parentIndex);
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
bvector<RulesetVariableEntry> NavNodesProviderContext::GetRelatedRulesetVariables() const
    {
    bset<Utf8String> ids = RulesDrivenProviderContext::GetRelatedVariablesIds();
    bvector<RulesetVariableEntry> idsWithValues;
    for (Utf8StringCR id : ids)
        idsWithValues.push_back(RulesetVariableEntry(id, GetRulesetVariables().GetJsonValue(id.c_str())));

    if (m_virtualParentNodeId && (!m_physicalParentNodeId || *m_physicalParentNodeId != *m_virtualParentNodeId))
        {
        // note: if virtual parent doesn't match physical one, we need to include virtual parent's variables
        NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*this);
        context->SetPhysicalParentNodeId(0);
        NavNodesProviderPtr provider = GetNodesCache().GetDataSource(*context, *m_virtualParentNodeId);
        if (provider.IsValid())
            {
            bvector<RulesetVariableEntry> virtualParentVariables = provider->GetContext().GetRelatedRulesetVariables();
            std::move(virtualParentVariables.begin(), virtualParentVariables.end(), std::inserter(idsWithValues, idsWithValues.end()));
            }
        }

    NavNodeCPtr physicalParentNode = GetPhysicalParentNode();
    if (physicalParentNode.IsValid() && NavNodesHelper::IsGroupingNode(*physicalParentNode))
        {
        // note: if parent node is a grouping node, we want to append all its related variables
        // because it may have derived them from it's virtual parent
        NavNodesProviderContextPtr context = NavNodesProviderContext::Create(*this);
        context->SetPhysicalParentNodeId(0);
        NavNodesProviderPtr provider = GetNodesCache().GetDataSource(*context, physicalParentNode->GetNodeId());
        if (provider.IsValid())
            {
            bvector<RulesetVariableEntry> physicalParentVariables = provider->GetContext().GetRelatedRulesetVariables();
            std::move(physicalParentVariables.begin(), physicalParentVariables.end(), std::inserter(idsWithValues, idsWithValues.end()));
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
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetVirtualParentNodeId(uint64_t id)
    {
    DELETE_AND_CLEAR(m_virtualParentNodeId);
    m_virtualParentNodeId = (0 != id) ? new uint64_t(id) : nullptr;

    if (m_providersIndexAllocator.IsNull())
        InitProvidersIndexAllocator(m_virtualParentNodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NavNodesProviderContext::CreateHierarchyLevelProvider(NavNodesProviderContextR context, JsonNavNodeCP parentNode) const
    {
    if (m_nodesCache->IsInitialized(context.GetHierarchyLevelIdentifier(), context.GetRulesetVariables()))
        {
        NavNodesProviderPtr provider = m_nodesCache->GetCombinedHierarchyLevel(context, context.GetHierarchyLevelIdentifier());
        if (provider.IsValid())
            {
            provider->GetContextR().Adopt(context);
            return provider;
            }
        }
    return m_providerFactory.Create(context, parentNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NavNodesProviderContext::CreatePartialNodesProvider(NavNodesProviderContextR context, DataSourceIdentifier const& id) const
    {
    NavNodesProviderPtr provider = m_nodesCache->GetDataSource(context, id);
    if (provider.IsValid())
        {
        provider->GetContextR().Adopt(context);
        return provider;
        }
    return m_providerFactory.Create(context, context.GetVirtualParentNode().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetRootNodeContext(RootNodeRuleCP rootNodeRule)
    {
    BeAssert(!IsRootNodeContext());
    BeAssert(!IsChildNodeContext());
    m_isRootNodeContext = true;
    m_rootNodeRule = rootNodeRule;
    SetVirtualParentNodeId((nullptr != m_physicalParentNodeId) ? *m_physicalParentNodeId : 0);
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
    SetVirtualParentNodeId((nullptr != other.m_virtualParentNodeId) ? *other.m_virtualParentNodeId : 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetChildNodeContext(ChildNodeRuleCP childNodeRule, NavNodeCR virtualParentNode)
    {
    BeAssert(!IsRootNodeContext());
    BeAssert(!IsChildNodeContext());
    m_isChildNodeContext = true;
    m_childNodeRule = childNodeRule;
    SetVirtualParentNode(virtualParentNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetChildNodeContext(NavNodesProviderContextCR other)
    {
    BeAssert(!IsRootNodeContext());
    BeAssert(!IsChildNodeContext());
    m_isChildNodeContext = other.m_isChildNodeContext;
    m_childNodeRule = other.m_childNodeRule;
    SetVirtualParentNodeId((nullptr != other.m_virtualParentNodeId) ? *other.m_virtualParentNodeId : 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetQueryContext(IConnectionManagerCR connections, IConnectionCR connection, IECDbUsedClassesListener* usedClassesListener)
    {
    RulesDrivenProviderContext::SetQueryContext(connections, connection);
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
            GetRulesetVariables(), &GetUsedVariablesListener(), GetECExpressionsCache(), *m_nodesCache, GetLocalState());

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
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
HierarchyLevelIdentifier const& NavNodesProviderContext::GetHierarchyLevelIdentifier() const
    {
    // mutex is required to avoid other threads looking for hierarchy level info while it is not cached
    BeMutexHolder lock(GetNodesCache().GetMutex());
    if (!m_hierarchyLevelIdentifier.IsValid())
        {
        m_hierarchyLevelIdentifier = GetNodesCache().FindHierarchyLevel(GetConnection().GetId().c_str(),
            GetRuleset().GetRuleSetId().c_str(), IsLocalizationContext() ? GetLocale().c_str() : "",
            GetVirtualParentNodeId() ? GetVirtualParentNodeId() : GetPhysicalParentNodeId());
        }
    if (!m_hierarchyLevelIdentifier.IsValid())
        {
        m_hierarchyLevelIdentifier = HierarchyLevelIdentifier(GetConnection().GetId(), GetRuleset().GetRuleSetId(),
            IsLocalizationContext() ? GetLocale() : "",
            GetPhysicalParentNodeId() ? *GetPhysicalParentNodeId() : 0,
            GetVirtualParentNodeId() ? *GetVirtualParentNodeId() : 0);
        GetNodesCache().Cache(m_hierarchyLevelIdentifier);
        }
    BeAssert(m_hierarchyLevelIdentifier.IsValid());
    return m_hierarchyLevelIdentifier;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceIdentifier const& NavNodesProviderContext::GetDataSourceIdentifier() const
    {
    // mutex is required to avoid other threads looking for data source while it is not cached
    BeMutexHolder lock(GetNodesCache().GetMutex());
    if (!m_dataSourceIdentifier.IsValid())
        {
        bvector<uint64_t> index;
        if (m_providersIndexAllocator.IsValid())
            index = GetProvidersIndexAllocator().AllocateIndex();
        else
            BeAssert(false);

        m_dataSourceIdentifier = GetNodesCache().FindDataSource(DataSourceIdentifier(GetHierarchyLevelIdentifier().GetId(), index), GetRulesetVariables()).GetIdentifier();
        if (!m_dataSourceIdentifier.IsValid())
            {
            DataSourceInfo datasourceInfo(DataSourceIdentifier(GetHierarchyLevelIdentifier().GetId(), index), GetRelatedRulesetVariables(),
                DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
            GetNodesCache().Cache(datasourceInfo);
            m_dataSourceIdentifier = datasourceInfo.GetIdentifier();
            }
        }
    BeAssert(m_dataSourceIdentifier.IsValid());
    return m_dataSourceIdentifier;
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
void NavNodesProvider::Initialize()
    {
    // call to ensure it's cached if necessary
    GetIdentifier();
    _OnPostInitialize();
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
* @bsimethod                                    Grigas.Petraitis                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceIdentifier const& NavNodesProvider::GetIdentifier() const
    {
    if (!_IsCacheable())
        {
        static DataSourceIdentifier s_invalid;
        return s_invalid;
        }

    // this creates and caches the identifier if not created yet
    return GetContext().GetDataSourceIdentifier();
    }

/*---------------------------------------------------------------------------------**//**
* Note: ancestorContext doesn't necessarily mean immediate parent context
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static NavNodesProviderContextPtr CreateContextForChildHierarchyLevel(NavNodesProviderContextCR ancestorContext, JsonNavNodeCR parentNode)
    {
    NavNodesProviderContextPtr ctx = NavNodesProviderContext::Create(ancestorContext);
    ctx->SetArtifactsCapturers(ancestorContext.GetArtifactsCapturers());
    ctx->GetOptimizationFlags().SetParentContainer(&ancestorContext.GetOptimizationFlags());
    ctx->SetVirtualParentNode(parentNode);
    if (NodeVisibility::Virtual == ancestorContext.GetNodesCache().GetNodeVisibility(parentNode.GetNodeId()))
        ctx->SetPhysicalParentNodeId(parentNode.GetParentNodeId());
    else
        ctx->SetPhysicalParentNode(parentNode);
    return ctx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static NavNodesProviderContextPtr CreateContextForSameHierarchyLevel(NavNodesProviderContextCR baseContext, bool copyNodesContext)
    {
    NavNodesProviderContextPtr ctx = NavNodesProviderContext::Create(baseContext);
    ctx->SetProvidersIndexAllocator(baseContext.GetProvidersIndexAllocator());
    ctx->SetUsedVariablesListener(baseContext);
    ctx->SetMayHaveArtifacts(baseContext.MayHaveArtifacts());
    ctx->SetArtifactsCapturers(baseContext.GetArtifactsCapturers());
    ctx->GetOptimizationFlags().SetParentContainer(&baseContext.GetOptimizationFlags());

    if (copyNodesContext)
        {
        if (baseContext.IsRootNodeContext())
            ctx->SetRootNodeContext(baseContext);
        else if (baseContext.IsChildNodeContext())
            ctx->SetChildNodeContext(baseContext);
        }
    else
        {
        if (baseContext.GetPhysicalParentNodeId())
            ctx->SetPhysicalParentNodeId(*baseContext.GetPhysicalParentNodeId());
        if (baseContext.GetVirtualParentNodeId())
            ctx->SetVirtualParentNodeId(*baseContext.GetVirtualParentNodeId());
        }
    return ctx;
    }

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

        if (!GetContext().GetOptimizationFlags().IsCheckingChildren() && !GetContext().RequiresFullProviderLoad())
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
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProvider::Iterator NavNodesProvider::begin() const
    {
    if (_GetInitializationStrategy() == ProviderNodesInitializationStrategy::Automatic)
        InitializeNodes();
    return _CreateFrontIterator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProvider::NotifyNodeChanged(JsonNavNodeCR node, int partsThatChanged) const
    {
    GetContext().GetNodesCache().Update(node.GetNodeId(), node, partsThatChanged);
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
        m_cache.Update(m_node->GetNodeId(), *m_node, IHierarchyCache::UPDATE_NodeItself);
        m_initialFlag = m_flag;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
HasChildrenFlag NavNodesProvider::AnyChildSpecificationReturnsNodes(JsonNavNodeR parentNode, bool enableChildrenArtifactsCapture) const
    {
    NavNodesProviderContextPtr childrenContext = CreateContextForChildHierarchyLevel(GetContext(), parentNode);
    ChildrenArtifactsCaptureContext captureChildrenArtifacts(*childrenContext, parentNode, enableChildrenArtifactsCapture);
    NavNodesProviderPtr childrenProvider = GetContext().CreateHierarchyLevelProvider(*childrenContext, &parentNode);
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

    node.SetHasChildren(HASCHILDREN_True == AnyChildSpecificationReturnsNodes(node, false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProvider::FinalizeNode(JsonNavNodeR node) const
    {
    if (!GetContext().NeedsFullLoad())
        return;

    BeMutexHolder lock(GetContext().GetNodesCache().GetMutex());
    DataSourceRelatedVariablesUpdater updater(GetContext(), &node);
    int updatedParts = 0;

    // make sure the node is customized
    if (!NavNodeExtendedData(node).IsCustomized())
        {
        updatedParts |= IHierarchyCache::UPDATE_NodeItself;
        CustomizationHelper::Customize(GetContext(), node);
        }

    // make sure the node has determined if it has any children
    if (!node.DeterminedChildren())
        {
        updatedParts |= IHierarchyCache::UPDATE_NodeItself;
        DetermineChildren(node);
        }

    // if any changes mande, update the node in cache
    if (0 != updatedParts)
        NotifyNodeChanged(node, updatedParts);
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
        FinalizeNode(*node);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NavNodesProvider::GetCachedProvider() const
    {
    if (!GetContext().GetNodesCache().IsInitialized(GetContext().GetDataSourceIdentifier(), GetContext().GetRulesetVariables()))
        return nullptr;

    NavNodesProviderPtr provider = GetContext().GetNodesCache().GetDataSource(GetContextR(), GetContext().GetDataSourceIdentifier());
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

    Utf8StringCR nodeHash = node.GetKey()->GetHash();
    Utf8StringCR parentHash = parentNode->GetKey()->GetHash();

    return nodeHash.Equals(parentHash) && 0 == strcmp(thisNodeExtendedData.GetSpecificationHash(), parentNodeExtendedData.GetSpecificationHash())
        || HasSimilarNodeInHierarchy(node, parentNodeExtendedData.HasVirtualParentId() ? parentNodeExtendedData.GetVirtualParentId() : 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProvider::EvaluateArtifacts(JsonNavNodeCR node) const
    {
    if (!GetContext().MayHaveArtifacts())
        return;

    if (GetContext().GetArtifactsCapturers().empty())
        return;

    NodeArtifacts artifacts = CustomizationHelper::EvaluateArtifacts(GetContext(), node);
    for (ArtifactsCapturer* capturer : GetContext().GetArtifactsCapturers())
        capturer->AddArtifact(artifacts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool NeedsChildrenArtifactsDetermined(Utf8CP expression) {return strstr(expression, ".ChildrenArtifacts");}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool NeedsChildrenDetermined(Utf8CP expression) {return strstr(expression, ".HasChildren") || NeedsChildrenArtifactsDetermined(expression);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool NeedsChildrenDetermined(NavNodeExtendedData const& extendedData)
    {
    return extendedData.HideIfNoChildren()
        || extendedData.HasHideExpression() && NeedsChildrenDetermined(extendedData.GetHideExpression());
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
        context.GetConnection(), context.GetLocale(), context.GetRulesetVariables(), &context.GetUsedVariablesListener());
    ExpressionContextPtr expressionContext = ECExpressionContextsProvider::GetCustomizationRulesContext(params);
    ECValue value;
    if (ECExpressionsHelper(context.GetECExpressionsCache()).EvaluateECExpression(value, expression, *expressionContext) && value.IsPrimitive() && value.ConvertToPrimitiveType(PRIMITIVETYPE_Boolean))
        return value.GetBoolean();
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodesProvider::ShouldReturnChildNodes(JsonNavNodeR node) const
    {
    NavNodeExtendedData extendedData(node);

    // when the node is created using a specification with "hide nodes in hierarchy" flag
    if (extendedData.HideNodesInHierarchy())
        return true;

    // if the node has "hide if grouping value not specified" flag and is a value property grouping node, grouping null values, show its chilren
    if (extendedData.HideIfGroupingValueNotSpecified() && node.GetType().Equals(NAVNODE_TYPE_ECPropertyGroupingNode) && extendedData.HasPropertyValue() && extendedData.GetPropertyValue()->IsNull())
        return true;

    HasChildrenFlag hasChildren = HASCHILDREN_Unknown;
    NodeHasChildrenFlagUpdater hasChildrenUpdater(GetContext().GetNodesCache(), &node, hasChildren);

    if (ChildrenHint::Always == extendedData.GetChildrenHint())
        hasChildren = HASCHILDREN_True;
    else if (ChildrenHint::Never == extendedData.GetChildrenHint() && !extendedData.HasGroupingType())
        hasChildren = HASCHILDREN_False;

    // if the node has only one child and also has "hide if only one child" flag, we want to display that child
    if (extendedData.HideIfOnlyOneChild())
        {
        NavNodesProviderPtr childrenProvider = GetContext().CreateHierarchyLevelProvider(*CreateContextForChildHierarchyLevel(GetContext(), node), &node);
        size_t childrenCount = childrenProvider->GetNodesCount();
        hasChildren = (childrenCount > 0) ? HASCHILDREN_True : HASCHILDREN_False;
        if (childrenCount == 1)
            return true;
        }

    // determine children if absolutely necessary for furher processing
    // - still don't have the result
    // - have the result, but need children artifacts - they're evaluated while checking children
    bool needsChildrenDetermined = (HASCHILDREN_Unknown == hasChildren && NeedsChildrenDetermined(extendedData))
        || (extendedData.HasHideExpression() && NeedsChildrenArtifactsDetermined(extendedData.GetHideExpression()));
    if (needsChildrenDetermined)
        hasChildren = AnyChildSpecificationReturnsNodes(node, true);

    hasChildrenUpdater.Update();

    // if node has 'hide if no children' and it has no children, hide it
    if (extendedData.HideIfNoChildren() && HASCHILDREN_False == hasChildren)
        return true;

    // when the node is created using a specification with "hide expression" flag
    if (extendedData.HasHideExpression() && ShouldHideNodeBasedOnHideExpression(GetContext(), node, extendedData.GetHideExpression()))
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NavNodesProvider::CreateProvider(JsonNavNodeR node) const
    {
    NavNodeExtendedData extendedData(node);

    // avoid returning a node if we already have a similar ancestor (prevent infinite hierarchies)
    NavNodesProviderContextPtr nestedContext = CreateContextForSameHierarchyLevel(GetContext(), true);
    if (GetContext().IsChildNodeContext() && HasSimilarNodeInHierarchy(node, extendedData.HasVirtualParentId() ? extendedData.GetVirtualParentId() : 0))
        {
        GetContext().GetNodesCache().MakeVirtual(node);
        return EmptyNavNodesProvider::Create(*nestedContext);
        }

    // the specification may want to return node's children instead of the node itself
    if (ShouldReturnChildNodes(node))
        {
        GetContext().GetNodesCache().MakeVirtual(node);
        return GetContext().CreateHierarchyLevelProvider(*CreateContextForChildHierarchyLevel(GetContext(), node), &node);
        }

    // otherwise, make node physical
    return SingleNavNodeProvider::Create(node, *nestedContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NavNodesProvider::CreateProviderForCachedNode(JsonNavNodeR node) const
    {
    NavNodesProviderContextPtr nestedContext = CreateContextForSameHierarchyLevel(GetContext(), true);
    NavNodeExtendedData extendedData(node);
    if (GetContext().IsChildNodeContext() && HasSimilarNodeInHierarchy(node, extendedData.HasVirtualParentId() ? extendedData.GetVirtualParentId() : 0))
        return EmptyNavNodesProvider::Create(*nestedContext);
    if (NodeVisibility::Virtual != GetContext().GetNodesCache().GetNodeVisibility(node.GetNodeId()))
        return SingleNavNodeProvider::Create(node, *nestedContext);
    return GetContext().CreateHierarchyLevelProvider(*CreateContextForChildHierarchyLevel(GetContext(), node), &node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NavNodesProvider::PostProcess(bvector<IProvidedNodesPostProcessor const*> const& postProcessors)
    {
    NavNodesProviderPtr curr = this;
    for (IProvidedNodesPostProcessor const* postProcessor : postProcessors)
        {
        NavNodesProviderPtr processed = postProcessor->PostProcessProvider(*curr);
        if (processed.IsValid())
            curr = processed;
        }
    return curr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderCPtr NavNodesProvider::PostProcess(bvector<IProvidedNodesPostProcessor const*> const& postProcessors) const
    {
    return const_cast<NavNodesProvider&>(*this).PostProcess(postProcessors);
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
    GetContext().GetNodesCache().FinalizeInitialization(GetContext().GetDataSourceIdentifier());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CustomNodesProvider::CustomNodesProvider(NavNodesProviderContextCR context, CustomNodeSpecificationCR specification)
    : NavNodesProvider(context), m_specification(specification)
    {
    GetContextR().SetMayHaveArtifacts(HasNodeArtifactRules(GetContext().GetRuleset(), specification));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodesProvider::_OnPostInitialize()
    {
    GetContext().GetNodesCache().Update(DataSourceInfo(GetContext().GetDataSourceIdentifier(), RulesetVariables(), DataSourceFilter(),
        bmap<ECClassId, bool>(), m_specification.GetHash(), m_specification.GetNodeType()), DataSourceInfo::PART_NodeTypes | DataSourceInfo::PART_SpecificationHash);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomNodesProvider::_InitializeNodes()
    {
    if (!NavNodesProvider::_InitializeNodes())
        return false;

    // mutex is required for savepoint what no read/write requests would happen from another thread during savepoint and also does synchronization of datasource initialization
    BeMutexHolder lock(GetContext().GetNodesCache().GetMutex());

    NavNodesProviderPtr cachedProvider = GetCachedProvider();
    if (cachedProvider.IsValid())
        {
        DisabledFullNodesLoadContext doNotCustomizeCachedNodes(*cachedProvider);
        JsonNavNodePtr cachedNode;
        if (!cachedProvider->GetNode(cachedNode, 0))
            {
            BeAssert(false);
            return false;
            }
        EvaluateArtifacts(*cachedNode);
        m_provider = CreateProviderForCachedNode(*cachedNode);
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
    JsonNavNodePtr node = GetContext().GetNodesFactory().CreateCustomNode(connectionId, GetLocale(GetContext()), label.c_str(), description.c_str(), imageId.c_str(), type.c_str());
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId(GetContext().GetRuleset().GetRuleSetId().c_str());
    extendedData.SetSpecificationHash(m_specification.GetHash());
    extendedData.SetHideIfNoChildren(m_specification.GetHideIfNoChildren());
    extendedData.SetHideNodesInHierarchy(m_specification.GetHideNodesInHierarchy());
    extendedData.SetHideExpression(m_specification.GetHideExpression());
    if (ChildrenHint::Unknown != m_specification.GetHasChildren())
        extendedData.SetChildrenHint(m_specification.GetHasChildren());
    if (nullptr != GetContext().GetPhysicalParentNodeId())
        node->SetParentNodeId(*GetContext().GetPhysicalParentNodeId());
    if (nullptr != GetContext().GetVirtualParentNodeId())
        extendedData.SetVirtualParentId(*GetContext().GetVirtualParentNodeId());
    if (GetContext().IsRootNodeContext() && GetContext().GetRootNodeRule() && GetContext().GetRootNodeRule()->GetAutoExpand())
        node->SetIsExpanded(true);
    node->SetNodeKey(*NavNodesHelper::CreateNodeKey(GetContext().GetConnection(), *node, parent.IsValid() ? parent->GetKey().get() : nullptr));
    EvaluateArtifacts(*node);
    GetContext().GetNodesCache().Cache(*node, GetContext().GetDataSourceIdentifier(), 0, NodeVisibility::Visible);
    m_provider = CreateProvider(*node);
    GetContext().GetNodesCache().FinalizeInitialization(GetContext().GetDataSourceIdentifier());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomNodesProvider::_GetNode(JsonNavNodePtr& node, size_t index) const
    {
    if (m_provider.IsNull())
        return false;

    return m_provider->GetNode(node, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomNodesProvider::_HasNodes() const
    {
    if (m_provider.IsNull())
        return false;

    return m_provider->HasNodes();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CustomNodesProvider::_GetNodesCount() const
    {
    if (m_provider.IsNull())
        return 0;

    return m_provider->GetNodesCount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
QueryBasedNodesProvider::QueryBasedNodesProvider(NavNodesProviderContextCR context, NavigationQuery const& query, bmap<ECClassId, bool> const& usedClassIds, bool isPartial)
    : MultiNavNodesProvider(context), m_query(&query), m_usedClassIds(usedClassIds), m_offset(0), m_isPartial(isPartial)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBasedNodesProvider::SetQuery(NavigationQuery const& query, bmap<ECClassId, bool> const& usedClassIds)
    {
    BeAssert(m_query->GetResultParameters().GetResultType() == query.GetResultParameters().GetResultType());
    m_query = &query;
    m_usedClassIds = usedClassIds;
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
            specification.GetRequiredRelationDirection(), parent->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys());
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
            specification.GetRequiredRelationDirection(), parent->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys());
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
            EvaluateArtifacts(*cachedNode);
            AddProvider(*CreateProviderForCachedNode(*cachedNode));
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

    JsonNavNodeCPtr virtualParent = GetContext().GetVirtualParentNode();
    DataSourceRelatedVariablesUpdater updater(GetContext(), virtualParent.get());

    // set up the custom functions context
    IECPropertyFormatter const* formatter = GetContext().IsPropertyFormattingContext() ? &GetContext().GetECPropertyFormatter() : nullptr;
    ECPresentation::UnitSystem unitSystem = GetContext().IsPropertyFormattingContext() ? GetContext().GetUnitSystem() : ECPresentation::UnitSystem::Undefined;
    CustomFunctionsContext fnContext(GetContext().GetSchemaHelper(), GetContext().GetConnections(), GetContext().GetConnection(), GetContext().GetRuleset(),
        GetContext().GetLocale(), GetContext().GetRulesetVariables(), &GetContext().GetUsedVariablesListener(), GetContext().GetECExpressionsCache(),
        GetContext().GetNodesFactory(), GetContext().GetUsedClassesListener(), virtualParent.get(), &m_query->GetExtendedData(), formatter, unitSystem);
    if (GetContext().IsLocalizationContext())
        fnContext.SetLocalizationProvider(GetContext().GetLocalizationProvider());

    // read the nodes
    auto nodesReader = NavNodesReader::Create(GetContext().GetNodesFactory(), GetContext().GetConnection(), GetContext().GetLocale(), *m_query->GetContract(),
        m_query->GetResultParameters().GetResultType(), &m_query->GetResultParameters().GetNavNodeExtendedData());
    QueryExecutor executor(GetContext().GetConnection(), *m_query);

    // create providers for each node
    uint64_t readerIndex = 0;
    JsonNavNodePtr node;
    while (QueryExecutorStatus::Row == executor.ReadNext(node, *nodesReader))
        {
        if (GetContext().GetCancelationToken().IsCanceled())
            break;

        NavNodeExtendedData extendedData(*node);
        extendedData.SetRulesetId(GetContext().GetRuleset().GetRuleSetId().c_str());
        if (nullptr != GetContext().GetPhysicalParentNodeId())
            node->SetParentNodeId(*GetContext().GetPhysicalParentNodeId());
        if (nullptr != GetContext().GetVirtualParentNodeId())
            extendedData.SetVirtualParentId(*GetContext().GetVirtualParentNodeId());
        if (GetContext().IsRootNodeContext() && GetContext().GetRootNodeRule() && GetContext().GetRootNodeRule()->GetAutoExpand())
            node->SetIsExpanded(true);
        node->SetNodeKey(*NavNodesHelper::CreateNodeKey(GetContext().GetConnection(), *node, virtualParent.IsValid() ? virtualParent->GetKey().get() : nullptr));

        EvaluateArtifacts(*node);

        GetContext().GetNodesCache().Cache(*node, GetIdentifier(), readerIndex + m_offset, NodeVisibility::Visible);

        NavNodesProviderPtr provider = CreateProvider(*node);
        AddProvider(*provider);
        ++readerIndex;
        }

    if (GetContext().GetCancelationToken().IsCanceled())
        {
        savepoint.Cancel();
        LoggingHelper::LogMessage(Log::Navigation, "[QueryBasedNodesProvider] Initialization canceled", NativeLogging::LOG_DEBUG);
        return false;
        }

    GetContext().GetNodesCache().FinalizeInitialization(GetIdentifier());
    LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[QueryBasedNodesProvider] Created node providers for %" PRIu64 " nodes", readerIndex).c_str(), NativeLogging::LOG_DEBUG);
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
        RefCountedPtr<QueryBasedNodesProvider> provider = QueryBasedNodesProvider::Create(*nestedContext, *pageQuery, m_usedClassIds, true);
        provider->SetNodesCount((i < pagesCount) ? pageSize : (nodesCount % pageSize));
        AddProvider(*provider);
        }
    GetContext().GetNodesCache().FinalizeInitialization(GetIdentifier());
    return true;
    }

size_t const QueryBasedNodesProvider::PageSize = 1000;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryBasedNodesProvider::_InitializeNodes()
    {
    if (!MultiNavNodesProvider::_InitializeNodes())
        return false;

    // mutex is required for savepoint what no read/write requests would happen from another thread during savepoint and also does synchronization of datasource initialization
    BeMutexHolder lock(GetContext().GetNodesCache().GetMutex());

    if (GetContext().GetNodesCache().IsInitialized(GetIdentifier(), GetContext().GetRulesetVariables()))
        return InitializeProvidersFromCache();

    bool isRequestingAllNodes = (GetContext().HasPageSize() && 0 == GetContext().GetPageSize()) || GetContext().RequiresFullProviderLoad();
    if (isRequestingAllNodes || NodesCountMightChange(GetContext().GetNodesCache(), GetContext().GetVirtualParentNode().get(),  *m_query) || GetNodesCount() <= PageSize)
        return InitializeProvidersForAllNodes();

    return InitializeProvidersForPagedQueries(GetNodesCount(), PageSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetNodesTypeFromNavigationQuery(NavigationQueryCR query)
    {
    switch (query.GetResultParameters().GetResultType())
        {
        case NavigationQueryResultType::ECInstanceNodes:
        case NavigationQueryResultType::MultiECInstanceNodes:
            return NAVNODE_TYPE_ECInstancesNode;
        case NavigationQueryResultType::ClassGroupingNodes:
        case NavigationQueryResultType::BaseClassGroupingNodes:
            return NAVNODE_TYPE_ECClassGroupingNode;
        case NavigationQueryResultType::DisplayLabelGroupingNodes:
            return NAVNODE_TYPE_DisplayLabelGroupingNode;
        case NavigationQueryResultType::PropertyGroupingNodes:
            return NAVNODE_TYPE_ECPropertyGroupingNode;
        }
    return "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetSpecificationHashFromNavigationQuery(NavigationQueryCR query)
    {
    ChildNodeSpecificationCP spec = query.GetResultParameters().GetSpecification();
    if (!spec)
        return "";
    return spec->GetHash();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBasedNodesProvider::_OnPostInitialize()
    {
    GetContext().GetNodesCache().Update(DataSourceInfo(GetIdentifier(),
        GetContext().GetRelatedRulesetVariables(),
        GetSpecificationFilter(GetContext().GetNodesCache(), GetContext().GetVirtualParentNode().get(), *m_query),
        m_usedClassIds, GetSpecificationHashFromNavigationQuery(*m_query), GetNodesTypeFromNavigationQuery(*m_query),
        m_isPartial),
        DataSourceInfo::PARTS_All);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryBasedNodesProvider::_GetNode(JsonNavNodePtr& node, size_t index) const
    {
    InitializeNodes();

    if (!MultiNavNodesProvider::_GetNode(node, index))
        return false;

    // TODO: should this be moved to post-processing step?
    NavNodeExtendedData extendedData(*node);
    if (extendedData.HideIfOnlyOneChild() && extendedData.HasGroupingType() && 1 == extendedData.GetInstanceKeysCount())
        {
        // note: for grouping nodes that have the HideIfOnlyOneChild flag and group only one node,
        // we may need to create a children provider and returns its node
        uint64_t nodeId = node->GetNodeId();
        HierarchyLevelIdentifier hl = GetContext().GetNodesCache().FindHierarchyLevel(GetContext().GetConnection().GetId().c_str(),
            GetContext().GetRuleset().GetRuleSetId().c_str(),
            GetContext().IsLocalizationContext() ? GetContext().GetLocale().c_str() : "", &nodeId);
        if (!hl.IsValid())
            {
            BeAssert(false);
            return false;
            }
        if (!GetContext().GetNodesCache().IsInitialized(hl, GetContext().GetRulesetVariables()))
            {
            NavNodesProviderPtr childrenProvider = GetContext().CreateHierarchyLevelProvider(*CreateContextForChildHierarchyLevel(GetContext(), *node), node.get());
            return childrenProvider->GetNode(node, 0);
            }
        }
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
    BeMutexHolder lock(GetContext().GetNodesCache().GetMutex());
    if (GetContext().GetNodesCache().IsInitialized(GetIdentifier(), GetContext().GetRulesetVariables()))
        {
        const_cast<QueryBasedNodesProvider*>(this)->InitializeProvidersFromCache();
        return MultiNavNodesProvider::_GetNodesCount();
        }

    JsonNavNodeCPtr virtualParent = GetContext().GetVirtualParentNode();

    bool isRequestingAllNodes = (GetContext().HasPageSize() && 0 == GetContext().GetPageSize()) || GetContext().RequiresFullProviderLoad();
    if (isRequestingAllNodes || NodesCountMightChange(GetContext().GetNodesCache(), virtualParent.get(), *m_query))
        {
        const_cast<QueryBasedNodesProvider*>(this)->InitializeProvidersForAllNodes();
        return MultiNavNodesProvider::_GetNodesCount();
        }

    DataSourceRelatedVariablesUpdater updater(GetContext(), virtualParent.get());

    // run a separate query to get the total nodes count (we already know this count
    // won't change during post-processing)
    IECPropertyFormatter const* formatter = GetContext().IsPropertyFormattingContext() ? &GetContext().GetECPropertyFormatter() : nullptr;
    ECPresentation::UnitSystem unitSystem = GetContext().IsPropertyFormattingContext() ? GetContext().GetUnitSystem() : ECPresentation::UnitSystem::Undefined;
    CustomFunctionsContext fnContext(GetContext().GetSchemaHelper(), GetContext().GetConnections(), GetContext().GetConnection(),
        GetContext().GetRuleset(), GetContext().GetLocale(), GetContext().GetRulesetVariables(), &GetContext().GetUsedVariablesListener(),
        GetContext().GetECExpressionsCache(), GetContext().GetNodesFactory(), GetContext().GetUsedClassesListener(), virtualParent.get(),
        &m_query->GetExtendedData(), formatter, unitSystem);
    if (GetContext().IsLocalizationContext())
        fnContext.SetLocalizationProvider(GetContext().GetLocalizationProvider());

    NavigationQueryPtr queryNotSorted = m_query->Clone();
    QueryBuilderHelpers::Order(*queryNotSorted, "");

    RefCountedPtr<CountQueryContract> contract = CountQueryContract::Create();
    ComplexGenericQueryPtr countQuery = ComplexGenericQuery::Create();
    countQuery->SelectContract(*contract);
    countQuery->GroupByContract(*contract);
    countQuery->From(*StringGenericQuery::Create(queryNotSorted->ToString(), queryNotSorted->GetBoundValues()));

    size_t count = (size_t)QueryExecutorHelper::ReadUInt64(GetContext().GetConnection(), *countQuery);
    if (0 == count)
        GetContext().GetNodesCache().FinalizeInitialization(GetIdentifier());

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProvider::Iterator QueryBasedNodesProvider::_CreateFrontIterator() const
    {
    InitializeNodes();
    return MultiNavNodesProvider::_CreateFrontIterator();
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
    GetContextR().SetMayHaveArtifacts(HasNodeArtifactRules(GetContext().GetRuleset(), specification));
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

    if (GetContext().IsRootNodeContext() && GetContext().GetRootNodeRule())
        return GetContext().GetQueryBuilder().GetQueries(*GetContext().GetRootNodeRule(), specification);

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
    if (!GetContext().RequiresFullProviderLoad())
        {
        SpecificationChildrenChecker visitor(GetContext());
        m_specification.Accept(visitor);
        if (visitor.AlwaysHasChildren())
            return true;
        }

    return MultiNavNodesProvider::_HasNodes();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MultiSpecificationNodesProvider::MultiSpecificationNodesProvider(NavNodesProviderContextR baseContext, RootNodeRuleSpecificationsList const& specs)
    : MultiNavNodesProvider(baseContext)
    {
    baseContext.SetVirtualParentNodeId(0);

    SpecificationsVisitor visitor;
    for (RootNodeRuleSpecification const& specification : specs)
        {
        NavNodesProviderContextPtr nestedContext = CreateContextForSameHierarchyLevel(GetContext(), false);
        nestedContext->SetRootNodeContext(&specification.GetRule());
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
    baseContext.SetVirtualParentNode(virtualParent);

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
    return MultiNavNodesProvider::_GetNode(node, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MultiSpecificationNodesProvider::_GetNodesCount() const
    {
    if (!GetContext().RequiresFullProviderLoad())
        {
        // note: if parent node is a grouping node, we can make some assumptions based on
        // grouped instances count and avoid having to query for nodes count
        JsonNavNodeCPtr virtualParent = GetContext().GetVirtualParentNode();
        if (virtualParent.IsValid())
            {
            NavNodeExtendedData parentExtendedData(*virtualParent);
            if (parentExtendedData.HasGroupingType() && 0 == parentExtendedData.GetInstanceKeysCount())
                return 0;
            }
        }

    return MultiNavNodesProvider::_GetNodesCount();
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

        size_t count = provider->GetNodesCount();
        if (index < count)
            return provider->GetNode(node, index);
        index -= count;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultiNavNodesProvider::RequiresFullLoad() const
    {
    if (GetContext().RequiresFullProviderLoad())
        return true;

    for (NavNodesProviderPtr const& provider : m_providers)
        {
        if (provider.IsValid() && provider->GetContext().RequiresFullProviderLoad())
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultiNavNodesProvider::_HasNodes() const
    {
    bool requiresFullLoad = RequiresFullLoad();
    bool hasNodes = false;
    for (NavNodesProviderPtr const& provider : m_providers)
        {
        if (provider.IsNull())
            continue;

        hasNodes |= provider->HasNodes();
        if (hasNodes && !requiresFullLoad)
            return true;
        }
    return hasNodes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MultiNavNodesProvider::_GetNodesCount() const
    {
    bool requiresFullLoad = RequiresFullLoad();
    size_t count = 0;
    for (NavNodesProviderPtr const& provider : m_providers)
        {
        if (GetContext().GetCancelationToken().IsCanceled())
            {
            LoggingHelper::LogMessage(Log::Navigation, "[MultiNavNodesProvider] GetNodesCount cancelled", NativeLogging::LOG_DEBUG);
            return 0;
            }

        if (provider.IsNull())
            continue;

        count += provider->GetNodesCount();

        if (GetContext().GetOptimizationFlags().IsCheckingChildren() && !requiresFullLoad)
            {
            if (count > 0)
                break;
            }
        }
    return count;
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2020
+===============+===============+===============+===============+===============+======*/
struct NestedProvidersBasedIteratorImpl : IteratorImpl<JsonNavNodePtr>
{
private:
    bvector<NavNodesProviderPtr> const& m_providers;
    bvector<NavNodesProviderPtr>::const_iterator m_providerIterator;
    NavNodesProvider::Iterator m_currNodesIterator;
    size_t m_currNodePosInProvider;
private:
    NavNodesProvider::Iterator CreateNodesIterator()
        {
        NavNodesProvider::Iterator iter;
        while (!iter.IsValid() && m_providerIterator != m_providers.end())
            {
            iter = (*m_providerIterator)->begin();
            if (iter == (*m_providerIterator)->end())
                {
                iter = NavNodesProvider::Iterator();
                ++m_providerIterator;
                }
            }
        m_currNodePosInProvider = 0;
        return iter;
        }
    void NextOne()
        {
        // 1. step nodes' iterator
        // 2. if reached the end - step providers' iterator
        // 3. if reached the end - reset nodes' iterator
        ++m_currNodesIterator;
        ++m_currNodePosInProvider;
        while (m_currNodesIterator.IsValid() && m_currNodesIterator == (*m_providerIterator)->end())
            {
            if (++m_providerIterator != m_providers.end())
                m_currNodesIterator = (*m_providerIterator)->begin();
            else
                m_currNodesIterator = NavNodesProvider::Iterator();
            m_currNodePosInProvider = 0;
            }
        }
protected:
    std::unique_ptr<IteratorImpl<JsonNavNodePtr>> _Copy() const override {return std::make_unique<NestedProvidersBasedIteratorImpl>(m_providers, m_providerIterator, m_currNodesIterator, m_currNodePosInProvider);}
    bool _Equals(IteratorImpl<JsonNavNodePtr> const& otherBase) const override
        {
        NestedProvidersBasedIteratorImpl const& other = static_cast<NestedProvidersBasedIteratorImpl const&>(otherBase);
        return m_providerIterator == other.m_providerIterator && m_currNodesIterator == other.m_currNodesIterator;
        }
    void _Next(size_t count) override
        {
        if (0 == count)
            return;

        if (1 == count)
            {
            // use quicker approach for majority of cases (no need to get counts)
            NextOne();
            return;
            }

        // calculate the number of nodes we need to skip past the end for current provider
        size_t nodesCountInCurrentProvider = (*m_providerIterator)->GetNodesCount();
        size_t nodesLeftInCurrentProvider = nodesCountInCurrentProvider - m_currNodePosInProvider - 1;
        if (count > nodesLeftInCurrentProvider)
            {
            // skip at the level of providers
            count -= nodesLeftInCurrentProvider + 1;
            ++m_providerIterator;
            while (m_providerIterator != m_providers.end() && (*m_providerIterator)->GetNodesCount() <= count)
                {
                count -= (*m_providerIterator)->GetNodesCount();
                ++m_providerIterator;
                }
            if (m_providerIterator == m_providers.end())
                {
                m_currNodesIterator = NavNodesProvider::Iterator();
                m_currNodePosInProvider = 0;
                return;
                }
            }

        // m_providerIterator now points to the provider we need to step into
        m_currNodesIterator = ((*m_providerIterator)->begin() += count);
        m_currNodePosInProvider = count;
        }
    JsonNavNodePtr _GetCurrent() const override {return *m_currNodesIterator;}
public:
    NestedProvidersBasedIteratorImpl(bvector<NavNodesProviderPtr> const& providers)
        : m_providers(providers), m_providerIterator(providers.begin()), m_currNodePosInProvider(0)
        {
        m_currNodesIterator = CreateNodesIterator();
        }
    NestedProvidersBasedIteratorImpl(bvector<NavNodesProviderPtr> const& providers, bvector<NavNodesProviderPtr>::const_iterator providerIter)
        : m_providers(providers), m_providerIterator(providerIter), m_currNodePosInProvider(0)
        {}
    NestedProvidersBasedIteratorImpl(bvector<NavNodesProviderPtr> const& providers, bvector<NavNodesProviderPtr>::const_iterator providerIter, NavNodesProvider::Iterator currNodesIterator, size_t currNodePosInProvider)
        : m_providers(providers), m_providerIterator(providerIter), m_currNodesIterator(currNodesIterator), m_currNodePosInProvider(currNodePosInProvider)
        {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProvider::Iterator MultiNavNodesProvider::_CreateFrontIterator() const {return Iterator(std::make_unique<NestedProvidersBasedIteratorImpl>(m_providers));}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProvider::Iterator MultiNavNodesProvider::_CreateBackIterator() const {return Iterator(std::make_unique<NestedProvidersBasedIteratorImpl>(m_providers, m_providers.end()));}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr MultiNavNodesProvider::_FindNestedProvider(DataSourceIdentifier const& identifier) const
    {
    for (NavNodesProviderPtr const& provider : m_providers)
        {
        if (provider->GetIdentifier() == identifier)
            return provider;

        NavNodesProviderPtr nested = provider->FindNestedProvider(identifier);
        if (nested.IsValid())
            return nested;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr DisplayLabelGroupingNodesPostProcessor::_PostProcessProvider(NavNodesProviderCR processedProvider) const
    {
    DisabledFullNodesLoadContext disableFullLoad(processedProvider);

    JsonNavNodePtr node;
    if (!processedProvider.GetNode(node, 0) || !node->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode))
        {
        // nothing to do if there're no nodes or the first node is not a display label grouping node
        return nullptr;
        }

    if (processedProvider.GetNode(node, 1))
        {
        // nothing to do if the grouping node has siblings
        return nullptr;
        }

    // return children of the grouping node
    auto context = CreateContextForChildHierarchyLevel(processedProvider.GetContext(), *node);
    context->GetOptimizationFlags().SetParentContainer(nullptr);
    return processedProvider.GetContext().CreateHierarchyLevelProvider(*context, node.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static bool CompareNodePtrsByLabel(JsonNavNodePtr const& lhs, JsonNavNodePtr const& rhs)
    {
    Utf8String paddedLhsLabel = ValueHelpers::PadNumbersInString(lhs->GetLabelDefinition().GetDisplayValue());
    Utf8String paddedRhsLabel = ValueHelpers::PadNumbersInString(rhs->GetLabelDefinition().GetDisplayValue());
    return paddedLhsLabel.CompareTo(paddedRhsLabel) < 0;
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct DelayedLabelSortingNodesProvider : MultiNavNodesProvider
{
private:
    bvector<NavNodesProviderPtr> m_dataSources;
    mutable bvector<JsonNavNodePtr> m_sortedNodes;

private:
    DelayedLabelSortingNodesProvider(NavNodesProviderContext const& context, bvector<NavNodesProviderPtr> providers)
        : MultiNavNodesProvider(context), m_dataSources(providers)
        {
        SetProviders(providers);
        }

    void InitSortedNodes() const
        {
        if (!m_sortedNodes.empty())
            return;

        auto _l = LoggingHelper::CreatePerformanceLogger(Log::Navigation, "Sorting nodes at post-processing step");
        for (NavNodesProviderPtr const& provider : m_dataSources)
            {
            DisabledFullNodesLoadContext disableFullLoad(*provider);
            size_t sizeBefore = m_sortedNodes.size();
            for (auto node : *provider)
                m_sortedNodes.push_back(node);
            if (0 != sizeBefore)
                std::inplace_merge(m_sortedNodes.begin(), m_sortedNodes.begin() + sizeBefore, m_sortedNodes.end(), CompareNodePtrsByLabel);
            }
        }

protected:
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override
        {
        if (GetContext().GetOptimizationFlags().IsPostProcessingDisabled())
            return MultiNavNodesProvider::_GetNode(node, index);

        if (index < GetNodesCount())
            {
            InitSortedNodes();
            node = m_sortedNodes[index];
            FinalizeNode(*node);
            return true;
            }
        return false;
        }

    Iterator _CreateFrontIterator() const override
        {
        if (GetContext().GetOptimizationFlags().IsPostProcessingDisabled())
            return MultiNavNodesProvider::_CreateFrontIterator();

        InitSortedNodes();
        return Iterator(std::make_unique<RandomAccessIteratorImpl<NavNodesProvider, JsonNavNodePtr>>(*this));
        }

    Iterator _CreateBackIterator() const override
        {
        if (GetContext().GetOptimizationFlags().IsPostProcessingDisabled())
            return MultiNavNodesProvider::_CreateBackIterator();

        InitSortedNodes();
        return Iterator(std::make_unique<RandomAccessIteratorImpl<NavNodesProvider, JsonNavNodePtr>>(*this, GetNodesCount()));
        }

public:
    static RefCountedPtr<DelayedLabelSortingNodesProvider> Create(NavNodesProviderContext const& context, bvector<NavNodesProviderPtr> providers)
        {
        return new DelayedLabelSortingNodesProvider(context, providers);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct InsertByIndexNodesProvider : NavNodesProvider
{
private:
    NavNodesProviderCPtr m_source;
    bvector<JsonNavNodePtr> m_nodesToInsert;
    mutable bvector<JsonNavNodePtr> m_result;

private:
    InsertByIndexNodesProvider(NavNodesProviderContext const& context, NavNodesProviderCR source, bvector<JsonNavNodePtr> nodesToInsert)
        : NavNodesProvider(context), m_source(&source), m_nodesToInsert(nodesToInsert)
        {}

    void InitResult() const
        {
        if (!m_result.empty())
            return;

        auto _l = LoggingHelper::CreatePerformanceLogger(Log::Navigation, "Inserting nodes by index at post-processing step");

        for (JsonNavNodePtr node : *m_source)
            m_result.push_back(node);

        for (JsonNavNodePtr node : m_nodesToInsert)
            {
            auto fullIndex = GetContext().GetNodesCache().GetNodeIndex(node->GetNodeId());
            if (!fullIndex.empty() && fullIndex.back() < m_result.size())
                m_result.insert(m_result.begin() + fullIndex.back(), node);
            else
                m_result.push_back(node);
            }
        }

protected:
    bool _IsCacheable() const override {return false;}
    NavNodesProviderPtr _FindNestedProvider(DataSourceIdentifier const& id) const override {return m_source->FindNestedProvider(id);}
    size_t _GetNodesCount() const override {return m_source->GetNodesCount() + m_nodesToInsert.size();}
    bool _HasNodes() const override {return !m_nodesToInsert.empty() || m_source->HasNodes();}

    bool _GetNode(JsonNavNodePtr& node, size_t index) const override
        {
        if (index < GetNodesCount())
            {
            InitResult();
            node = m_result[index];
            FinalizeNode(*node);
            return true;
            }
        return false;
        }

    Iterator _CreateFrontIterator() const override
        {
        InitResult();
        return Iterator(std::make_unique<RandomAccessIteratorImpl<NavNodesProvider, JsonNavNodePtr>>(*this));
        }

    Iterator _CreateBackIterator() const override
        {
        InitResult();
        return Iterator(std::make_unique<RandomAccessIteratorImpl<NavNodesProvider, JsonNavNodePtr>>(*this, GetNodesCount()));
        }

public:
    static RefCountedPtr<InsertByIndexNodesProvider> Create(NavNodesProviderContext const& context, NavNodesProviderCR source, bvector<JsonNavNodePtr> nodesToInsert)
        {
        return new InsertByIndexNodesProvider(context, source, nodesToInsert);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsDataSourceLabelSorted(RulesPreprocessor& rules, Utf8StringCR specificationHash, NavNodeCP parent)
    {
    ChildNodeSpecificationCP spec = rules.FindChildNodeSpecification(specificationHash);
    if (!spec)
        {
        BeAssert(false);
        return true;
        }

    if (spec->GetDoNotSort())
        return false;

    bvector<SortingRuleCP> sortingRules = rules.GetSortingRules(IRulesPreprocessor::AggregateCustomizationRuleParameters(parent, specificationHash));
    if (!sortingRules.empty())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static bmap<Utf8String, bvector<JsonNavNodePtr>> CreateNodeGroupsBySpecification(NavNodesProviderCR provider)
    {
    DisabledFullNodesLoadContext disableFullLoad(provider);
    bmap<Utf8String, bvector<JsonNavNodePtr>> groups;
    for (JsonNavNodePtr node : provider)
        {
        Utf8CP hash = NavNodeExtendedData(*node).GetSpecificationHash();
        if (!hash || !*hash)
            {
            BeAssert(false);
            continue;
            }
        auto groupIter = groups.find(hash);
        if (groups.end() == groupIter)
            groups.Insert(hash, bvector<JsonNavNodePtr>{node});
        else
            groupIter->second.push_back(node);
        }
    return groups;
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                05/2020
+===============+===============+===============+===============+===============+======*/
struct DataSourcesMergeKey
    {
    Utf8String m_specHash;
    Utf8String m_nodesType;
    bool m_isPartial;
    DataSourcesMergeKey(DataSourceInfo const& info)
        : m_specHash(info.GetSpecificationHash()), m_nodesType(info.GetNodeTypes()), m_isPartial(info.IsPartial())
        {}
    static bool ShouldMergeTypes(Utf8StringCR lhs, Utf8StringCR rhs)
        {
        return lhs.Equals(rhs)
            || (lhs.Equals(NAVNODE_TYPE_DisplayLabelGroupingNode) && rhs.Equals(NAVNODE_TYPE_ECInstancesNode)
                || lhs.Equals(NAVNODE_TYPE_ECInstancesNode) && rhs.Equals(NAVNODE_TYPE_DisplayLabelGroupingNode));
        }
    bool ShouldMergeWith(DataSourcesMergeKey const& other) const
        {
        if (m_isPartial || other.m_isPartial)
            return false;
        return m_specHash == other.m_specHash
            && ShouldMergeTypes(m_nodesType, other.m_nodesType);
        }
    };

//#define DEBUG_LABEL_SORTING_POSTPROCESSOR 1
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr DisplayLabelSortingPostProcessor::_PostProcessProvider(NavNodesProviderCR processedProvider) const
    {
    NavNodesProviderContextCR context = processedProvider.GetContext();
    if (context.GetOptimizationFlags().IsPostProcessingDisabled())
        return nullptr;

    IHierarchyCache& cache = context.GetNodesCache();
    bvector<DataSourceInfo> dsInfos = cache.FindDataSources(context.GetHierarchyLevelIdentifier(), context.GetRulesetVariables(),
        DataSourceInfo::PART_SpecificationHash | DataSourceInfo::PART_NodeTypes | DataSourceInfo::PART_IsPartial);
    if (dsInfos.size() < 2)
        {
        // if hierarchy level consists of only one datasource, then it's already sorted and we don't
        // need any post-processing here
        return nullptr;
        }

    NavNodeCPtr parent = context.GetVirtualParentNode();
    RulesPreprocessor rules(context.GetConnections(), context.GetConnection(), context.GetRuleset(), context.GetLocale(),
        context.GetRulesetVariables(), nullptr, context.GetECExpressionsCache());
    bmap<Utf8String, bvector<JsonNavNodePtr>> deprecatedSameLabelPostProcessedNodes;

    // group adjacent label-sorted providers by specification hash and node types
    bvector<bpair<DataSourcesMergeKey, bvector<NavNodesProviderPtr>>> providers;
    for (DataSourceInfo const& info : dsInfos)
        {
        NavNodesProviderPtr provider;
        if (context.GetNodesCache().IsInitialized(info.GetIdentifier(), context.GetRulesetVariables()))
            provider = context.GetNodesCache().GetDataSource(*CreateContextForSameHierarchyLevel(context, true), info.GetIdentifier(), true, true);
        else
            provider = processedProvider.FindNestedProvider(info.GetIdentifier());
        if (provider.IsNull())
            {
            BeAssert(false);
            continue;
            }

        provider->GetContextR().Adopt(context);
        if (!provider->HasNodes())
            continue;

        if (info.GetSpecificationHash().empty())
            {
            // this is a special case for data sources created by SameLabelGroupingNodesPostProcessorDeprecated - they don't get
            // an index, so their order is not correct. as a workaround, we take all their nodes and place in appropriate positions.
            // hopefully the solution is temporary until we have a single place where we merge branch into cache and can use it to
            // post-process before merging - that should allow us to completely get rid of the deprecated post-processor
            deprecatedSameLabelPostProcessedNodes = CreateNodeGroupsBySpecification(*provider);
            continue;
            }

        DataSourcesMergeKey mergeKey(info);
        bool mergeWithPrevious = !providers.empty()
            && providers.back().first.ShouldMergeWith(mergeKey)
            && IsDataSourceLabelSorted(rules, info.GetSpecificationHash(), parent.get());

        if (mergeWithPrevious)
            providers.back().second.push_back(provider);
        else
            providers.push_back(make_bpair(mergeKey, bvector<NavNodesProviderPtr>{provider}));
        }

    // create a merged provider
    auto mergedProviderContext = CreateContextForSameHierarchyLevel(context, true);
    mergedProviderContext->GetOptimizationFlags().SetParentContainer(nullptr);

#ifdef DEBUG_LABEL_SORTING_POSTPROCESSOR
    bvector<JsonNavNodePtr> nodes;
    for (auto const& entry : providers)
        {
        for (NavNodesProviderPtr const& provider : entry.second)
            {
            for (JsonNavNodePtr node : *provider)
                nodes.push_back(node);
            }
        }
    auto mergedProvider = BVectorNodesProvider::Create(*mergedProviderContext, nodes, true);
#else
    auto mergedProvider = MultiNavNodesProvider::Create(*mergedProviderContext);
    for (auto const& entry : providers)
        {
        auto sameLabelGroupedNodesIter = deprecatedSameLabelPostProcessedNodes.find(entry.first.m_specHash);
        if (entry.second.size() == 1)
            {
            NavNodesProviderPtr provider = entry.second.back();
            if (sameLabelGroupedNodesIter != deprecatedSameLabelPostProcessedNodes.end())
                provider = InsertByIndexNodesProvider::Create(*CreateContextForSameHierarchyLevel(*mergedProviderContext, true), *provider, sameLabelGroupedNodesIter->second);
            mergedProvider->AddProvider(*provider);
            }
        else
            {
            bvector<NavNodesProviderPtr> sortedProviders = entry.second;
            if (sameLabelGroupedNodesIter != deprecatedSameLabelPostProcessedNodes.end())
                {
                std::sort(sameLabelGroupedNodesIter->second.begin(), sameLabelGroupedNodesIter->second.end(), CompareNodePtrsByLabel);
                sortedProviders.push_back(BVectorNodesProvider::Create(*CreateContextForSameHierarchyLevel(*mergedProviderContext, true), sameLabelGroupedNodesIter->second, true));
                }
            mergedProvider->AddProvider(*DelayedLabelSortingNodesProvider::Create(*CreateContextForSameHierarchyLevel(*mergedProviderContext, true), sortedProviders));
            }
        if (sameLabelGroupedNodesIter != deprecatedSameLabelPostProcessedNodes.end())
            deprecatedSameLabelPostProcessedNodes.erase(sameLabelGroupedNodesIter);
        }
    // might still have some grouping nodes... just put them as-is
    for (auto const& entry : deprecatedSameLabelPostProcessedNodes)
        mergedProvider->AddProvider(*BVectorNodesProvider::Create(*CreateContextForSameHierarchyLevel(*mergedProviderContext, true), entry.second, true));
#endif
    return mergedProvider;
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2019
+===============+===============+===============+===============+===============+======*/
struct PostProcessedClassesFinder : CustomizationRuleVisitor
{
    struct PostProcessedSameLabelInstanceGroupFilter : GroupingRuleSpecificationVisitor
        {
        bool m_isPostProcessedSameLabelInstanceGroup;
        void _Visit(SameLabelInstanceGroup const& spec) override
            {
            if (SameLabelInstanceGroupApplicationStage::PostProcess == spec.GetApplicationStage())
                m_isPostProcessedSameLabelInstanceGroup = true;
            }
        bool Matches(GroupSpecificationCR spec)
            {
            m_isPostProcessedSameLabelInstanceGroup = false;
            spec.Accept(*this);
            return m_isPostProcessedSameLabelInstanceGroup;
            }
        };

private:
    ECSchemaHelper const& m_schemaHelper;
    IJsonLocalState const* m_localState;
    bset<ECClassCP> m_classes;
protected:
    void _Visit(GroupingRuleCR rule) override
        {
        GroupSpecificationCP spec = QueryBuilderHelpers::GetActiveGroupingSpecification(rule, m_localState);
        if (spec && PostProcessedSameLabelInstanceGroupFilter().Matches(*spec))
            {
            ECClassCP groupingClass = m_schemaHelper.GetECClass(rule.GetSchemaName().c_str(), rule.GetClassName().c_str());
            if (groupingClass)
                m_classes.insert(groupingClass);
            }
        }
public:
    PostProcessedClassesFinder(ECSchemaHelper const& schemaHelper, IJsonLocalState const* localState)
        : m_schemaHelper(schemaHelper), m_localState(localState)
        {}
    bset<ECClassCP> const& GetClasses() const { return m_classes; }
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SameLabelGroupingNodesPostProcessorDeprecated::InitGroupedClasses(PresentationRuleSetCR ruleset, bvector<ChildNodeRuleCP> const& rules, ECSchemaHelper const& schemaHelper, IJsonLocalState const* localState)
    {
    PostProcessedClassesFinder sameLabelPostProcessedClasses(schemaHelper, localState);
    for (ChildNodeRuleCP rule : rules)
        {
        for (CustomizationRuleCP customizationRule : rule->GetCustomizationRules())
            customizationRule->Accept(sameLabelPostProcessedClasses);
        }
    for (CustomizationRuleCP rule : ruleset.GetGroupingRules())
        rule->Accept(sameLabelPostProcessedClasses);
    m_groupedClasses = sameLabelPostProcessedClasses.GetClasses();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool SameLabelGroupingNodesPostProcessorDeprecated::IsSuitableForMerge(JsonNavNodeCR node) const
    {
    if (!node.GetKey()->AsECInstanceNodeKey())
        return false;

    bvector<ECClassInstanceKey> instanceKeys = node.GetKey()->AsECInstanceNodeKey()->GetInstanceKeys();
    for (ECClassInstanceKeyCR k : instanceKeys)
        {
        for (ECClassCP groupedClass : m_groupedClasses)
            {
            if (k.GetClass()->Is(groupedClass))
                return true;
            }
        }
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static void MergeInstanceKeys(NavNodesProviderContextCR context, JsonNavNodeR target, JsonNavNodeCR source)
    {
    bvector<ECInstanceKey> targetKeys = NavNodeExtendedData(target).GetInstanceKeys();
    bvector<ECInstanceKey> sourceKeys = NavNodeExtendedData(source).GetInstanceKeys();
    std::move(sourceKeys.begin(), sourceKeys.end(), std::back_inserter(targetKeys));
    NavNodeExtendedData(target).SetInstanceKeys(targetKeys);

    JsonNavNodeCPtr parent = context.GetVirtualParentNode();
    target.SetNodeKey(*NavNodesHelper::CreateNodeKey(context.GetConnection(), target, parent.IsValid() ? parent->GetKey().get() : nullptr));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr SameLabelGroupingNodesPostProcessorDeprecated::MergeNodes(NavNodesProviderContextCR context, JsonNavNodeR lhs, JsonNavNodeR rhs) const
    {
    if (!lhs.GetKey()->AsECInstanceNodeKey() || !rhs.GetKey()->AsECInstanceNodeKey())
        {
        BeAssert(false && "Both nodes must be ECInstance nodes");
        return &lhs;
        }
    if (!lhs.GetLabelDefinition().GetDisplayValue().Equals(rhs.GetLabelDefinition().GetDisplayValue()))
        {
        BeAssert(false && "Labels of both nodes must match");
        return &lhs;
        }
    JsonNavNodePtr node = lhs.Clone();
    node->SetNodeId(0);
    node->ResetHasChildren();
    MergeInstanceKeys(context, *node, rhs);
    return node;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<uint64_t> CalculateMergedNodeIndex(DataSourceIdentifier const& dsInfo, bvector<uint64_t> const& nodeIndex)
    {
    bvector<uint64_t> const& dsIndex = dsInfo.GetIndex();
    BeAssert(dsIndex.size() < nodeIndex.size());
    return bvector<uint64_t>(nodeIndex.begin() + dsIndex.size(), nodeIndex.end());
    }
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2019
+===============+===============+===============+===============+===============+======*/
struct ParentProviderIndexAllocator : IProvidersIndexAllocator
{
private:
    bvector<uint64_t> m_parentIndex;
protected:
    bvector<uint64_t> _GetCurrentIndex() const override {return m_parentIndex;}
    bvector<uint64_t> _AllocateIndex() override {return m_parentIndex;}
public:
    ParentProviderIndexAllocator(bvector<uint64_t> parentIndex) : m_parentIndex(parentIndex) {}
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr SameLabelGroupingNodesPostProcessorDeprecated::CreatePostProcessedProvider(NavNodesProviderCR processedProvider) const
    {
    if (m_groupedClasses.empty())
        return nullptr;

    if (processedProvider.GetNodesCount() < 2)
        return nullptr;

    NavNodesProviderContextPtr context = CreateContextForSameHierarchyLevel(processedProvider.GetContext(), false);
    context->GetOptimizationFlags().SetParentContainer(nullptr);
    context->SetProvidersIndexAllocator(*new ParentProviderIndexAllocator(processedProvider.GetContext().GetVirtualParentNodeId() ? processedProvider.GetContext().GetNodesCache().GetNodeIndex(*processedProvider.GetContext().GetVirtualParentNodeId()) : bvector<uint64_t>()));

    DataSourceIdentifier const& dsInfo = context->GetDataSourceIdentifier();
    DisabledFullNodesLoadContext disableFullNodesLoad(processedProvider);

    bvector<JsonNavNodePtr> nodes;
    bmap<Utf8String, size_t> labelsMap;
    bmap<size_t, bvector<uint64_t>> mergedNodeIndexes;
    for (JsonNavNodePtr node : processedProvider)
        {
        auto iter = labelsMap.find(node->GetLabelDefinition().GetDisplayValue());
        if (labelsMap.end() != iter)
            {
            size_t pos = iter->second;
            BeAssert(pos < nodes.size());
            BeAssert(nodes[pos]->GetLabelDefinition().GetDisplayValue().Equals(node->GetLabelDefinition().GetDisplayValue()));
            if (IsSuitableForMerge(*nodes[pos]) && IsSuitableForMerge(*node))
                {
                JsonNavNodePtr merged;
                auto mergedNodeIndexIter = mergedNodeIndexes.find(pos);
                if (mergedNodeIndexes.end() != mergedNodeIndexIter)
                    {
                    // nodes[pos] is an already merged node
                    merged = nodes[pos];
                    MergeInstanceKeys(*context, *merged, *node);
                    context->GetNodesCache().Update(merged->GetNodeId(), *merged, IHierarchyCache::UPDATE_NodeAll);
                    }
                else
                    {
                    // first time merge
                    merged = MergeNodes(*context, *nodes[pos], *node);
                    mergedNodeIndexIter = mergedNodeIndexes.Insert(pos, CalculateMergedNodeIndex(dsInfo, context->GetNodesCache().GetNodeIndex(nodes[pos]->GetNodeId()))).first;
                    context->GetNodesCache().MakeHidden(*nodes[pos]);
                    context->GetNodesCache().Cache(*merged, dsInfo, mergedNodeIndexIter->second, NodeVisibility::Visible);
                    nodes[pos] = merged;
                    }
                context->GetNodesCache().MakeHidden(*node);
                continue;
                }
            }
        nodes.push_back(node);
        labelsMap.Insert(node->GetLabelDefinition().GetDisplayValue(), nodes.size() - 1);
        }
    context->GetNodesCache().FinalizeInitialization(dsInfo);
    return BVectorNodesProvider::Create(*context, nodes, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static NavNodesProviderContextPtr CreatePostProcessingProviderContext(NavNodesProviderContextCR baseContext)
    {
    NavNodesProviderContextPtr context = CreateContextForSameHierarchyLevel(baseContext, true);
    if (baseContext.GetVirtualParentNodeId())
        context->SetVirtualParentNodeId(*baseContext.GetVirtualParentNodeId());
    return context;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
PostProcessingNodesProviderDeprecated::PostProcessingNodesProviderDeprecated(NavNodesProviderCR wrappedProvider)
    : NavNodesProvider(*CreatePostProcessingProviderContext(wrappedProvider.GetContext())), m_wrappedProvider(&wrappedProvider)
    {
    OptimizationFlagsContainer const* parentContainer = wrappedProvider.GetContext().GetOptimizationFlags().GetParentContainer();
    GetContextR().GetOptimizationFlags().SetParentContainer(parentContainer);
    wrappedProvider.GetContextR().GetOptimizationFlags().SetParentContainer(&GetContext().GetOptimizationFlags());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderCPtr PostProcessingNodesProviderDeprecated::GetProcessedProvider() const
    {
    if (m_processedProvider.IsNull())
        {
        JsonNavNodePtr node;
        if (m_wrappedProvider->GetNode(node, 0))
            {
            for (auto const& postProcessor : m_postProcessors)
                {
                NavNodesProviderPtr processed = postProcessor->PostProcessNodeRequest(m_processedProvider.IsValid() ? *m_processedProvider : *m_wrappedProvider, *node, 0);
                if (processed.IsNull())
                    continue;

                m_processedProvider = processed;

                if (!m_processedProvider->GetNode(node, 0))
                    break;
                }
            }
        if (m_processedProvider.IsNull())
            m_processedProvider = m_wrappedProvider;
        }
    return m_processedProvider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool PostProcessingNodesProviderDeprecated::_GetNode(JsonNavNodePtr& node, size_t index) const
    {
    return GetProcessedProvider()->GetNode(node, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
size_t PostProcessingNodesProviderDeprecated::_GetNodesCount() const
    {
    if (m_processedProvider.IsValid())
        return m_processedProvider->GetNodesCount();

    size_t count = 0;
    {
    // note: we intentionally don't carry forward this provider's optimization
    // flags to wrapped provider, because we need to know exact nodes count when
    // processing
    NoDerivedOptimizationFlagsContext dontOptimizeRequest(*m_wrappedProvider);
    count = m_wrappedProvider->GetNodesCount();
    }

    NavNodesProviderPtr processed = nullptr;
    for (auto const& postProcessor : m_postProcessors)
        {
        NavNodesProviderPtr result = postProcessor->PostProcessCountRequest(processed.IsValid() ? *processed : *m_wrappedProvider, count);
        if (result.IsNull())
            continue;

        processed = result;
        count = processed->GetNodesCount();
        }

    if (!GetContext().GetOptimizationFlags().IsCheckingChildren())
        m_processedProvider = processed;

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool PostProcessingNodesProviderDeprecated::_HasNodes() const {return GetNodesCount() > 0;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SQLiteCacheNodesProvider::SQLiteCacheNodesProvider(NavNodesProviderContextCR context, Db& cache, StatementCache& statements)
    : NavNodesProvider(context), m_nodes(nullptr), m_nodesCount(nullptr), m_cache(cache), m_statements(statements)
    {
    InitializeUsedVariables();
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
void SQLiteCacheNodesProvider::InitializeUsedVariables()
    {
    Utf8String query = "SELECT [v].[Variables] "
                       "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                       "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id] "
                       "  JOIN [" NODESCACHE_TABLENAME_Variables "] v ON [v].[Id] = [ds].[VariablesId]"
                       " WHERE [hl].[VirtualParentNodeId] ";
    if (0 == GetContext().GetVirtualParentNodeId())
        query.append("IS NULL");
    else
        query.append(" = ?");
    query.append(" AND [hl].[RulesetId] = ?");
    query.append(" AND [hl].[Locale] = ?");
    query.append(" AND " NODESCACHE_FUNCNAME_VariablesMatch "([v].[Variables], ?) ");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }

    int bindingIndex = 1;
    if (nullptr != GetContext().GetVirtualParentNodeId())
        stmt->BindUInt64(bindingIndex++, *GetContext().GetVirtualParentNodeId());
    stmt->BindText(bindingIndex++, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetLocale(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerialized(), Statement::MakeCopy::No);

    BeMutexHolder lock(GetContext().GetNodesCache().GetMutex());
    while (BE_SQLITE_ROW == stmt->Step())
        {
        bvector<Utf8String> usedVariables = RulesetVariables(stmt->GetValueText(0)).GetVariableNames();
        for (Utf8StringCR name : usedVariables)
            GetContext().GetUsedVariablesListener().OnVariableUsed(name.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void SQLiteCacheNodesProvider::InitializeNodes()
    {
    if (nullptr != m_nodes)
        return; // already initialized

    BeMutexHolder lock(GetContext().GetNodesCache().GetMutex());

    m_nodes = new bvector<JsonNavNodePtr>();
    CachedStatementPtr statement = _GetNodesStatement();
    if (!statement.IsValid())
        return;

    rapidjson::Document json;
    while (BE_SQLITE_ROW == statement->Step())
        {
        Utf8CP serializedNode = statement->GetValueText(0);
        if (nullptr == serializedNode || 0 == *serializedNode)
            {
            BeAssert(false);
            continue;
            }

        json.GetAllocator().Clear();
        json.SetNull();
        json.Parse(serializedNode);
        JsonNavNodePtr node = GetContext().GetNodesFactory().CreateFromJson(GetContext().GetConnection(), json);
        if (node.IsNull())
            continue;

        if (!statement->IsColumnNull(1))
            node->SetParentNodeId(statement->GetValueUInt64(1));

        if (!statement->IsColumnNull(2))
            NavNodeExtendedData(*node).SetVirtualParentId(statement->GetValueUInt64(2));

        node->SetNodeId(statement->GetValueUInt64(3));
        node->SetNodeKey(*NavNodesHelper::CreateNodeKey(GetContext().GetConnection(), *node, NavNodesHelper::NodeKeyHashPathFromString(statement->GetValueText(4)), false));
        NavNodeExtendedData(*node).SetIsLabelCustomized(true);
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
    FinalizeNode(*node);
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
        BeMutexHolder lock(GetContext().GetNodesCache().GetMutex());
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
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProvider::Iterator SQLiteCacheNodesProvider::_CreateFrontIterator() const
    {
    const_cast<SQLiteCacheNodesProvider*>(this)->InitializeNodes();
    return Iterator(std::make_unique<RandomAccessIteratorImpl<NavNodesProvider, JsonNavNodePtr>>(*this));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProvider::Iterator SQLiteCacheNodesProvider::_CreateBackIterator() const
    {
    const_cast<SQLiteCacheNodesProvider*>(this)->InitializeNodes();
    return Iterator(std::make_unique<RandomAccessIteratorImpl<NavNodesProvider, JsonNavNodePtr>>(*this, GetNodesCount()));
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
    Utf8String query = "SELECT [n].[Data], [hl].[PhysicalParentNodeId], [hl].[VirtualParentNodeId], [n].[Id], [nk].[PathFromRoot] "
                       "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                       "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id]"
                       "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId]"
                       "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id]"
                       "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id]"
                       "  JOIN [" NODESCACHE_TABLENAME_NodesOrder "] no ON [no].[HierarchyLevelId] = [hl].[Id] AND [no].[DataSourceId] = [ds].[Id] AND [no].[NodeId] = [n].[Id]"
                       " WHERE     [n].[Visibility] = ? AND [hl].[RemovalId] IS NULL "
                       "       AND [hl].[RulesetId] = ? AND [hl].[Locale] = ? AND [hl].[PhysicalParentNodeId] ";
    if (nullptr == m_info.GetPhysicalParentNodeId())
        query.append("IS NULL");
    else
        query.append(" = ?");
    query.append(" AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) ");
    query.append(" ORDER BY [no].[OrderValue]");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    int bindingIndex = 1;
    stmt->BindInt(bindingIndex++, (int)NodeVisibility::Visible);
    stmt->BindText(bindingIndex++, m_info.GetRulesetId(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, m_info.GetLocale(), Statement::MakeCopy::No);
    if (nullptr != m_info.GetPhysicalParentNodeId())
        stmt->BindUInt64(bindingIndex++, *m_info.GetPhysicalParentNodeId());
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerialized(), Statement::MakeCopy::No);

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
                       "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId]"
                       "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id]"
                       " WHERE     [n].[Visibility] = ? AND [hl].[RemovalId] IS NULL "
                       "       AND [hl].[RulesetId] = ? AND [hl].[Locale] = ? AND [hl].[PhysicalParentNodeId] ";
    if (nullptr == m_info.GetPhysicalParentNodeId())
        query.append("IS NULL");
    else
        query.append(" = ?");
    query.append(" AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) ");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    int bindingIndex = 1;
    stmt->BindInt(bindingIndex++, (int)NodeVisibility::Visible);
    stmt->BindText(bindingIndex++, m_info.GetRulesetId(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, m_info.GetLocale(), Statement::MakeCopy::No);
    if (nullptr != m_info.GetPhysicalParentNodeId())
        stmt->BindUInt64(bindingIndex++, *m_info.GetPhysicalParentNodeId());
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerialized(), Statement::MakeCopy::No);

    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedHierarchyLevelProvider::_GetNodesStatement() const
    {
    Utf8CP query = "SELECT [n].[Data], [hl].[PhysicalParentNodeId], [hl].[VirtualParentNodeId], [n].[Id], [nk].[PathFromRoot] "
                   "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
                   "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id]"
                   "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId]"
                   "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id]"
                   "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id]"
                   "  JOIN [" NODESCACHE_TABLENAME_NodesOrder "] no ON [no].[HierarchyLevelId] = [hl].[Id] AND [no].[DataSourceId] = [ds].[Id] AND [no].[NodeId] = [n].[Id]"
                   " WHERE [n].[Visibility] = ? AND [hl].[Id] = ?  AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) "
                   " ORDER BY [no].[OrderValue]";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        {
        BeAssert(false);
        return nullptr;
        }

    int bindingIndex = 1;
    stmt->BindInt(bindingIndex++, (int)NodeVisibility::Visible);
    stmt->BindUInt64(bindingIndex++, m_hierarchyLevelId);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerialized(), Statement::MakeCopy::No);

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
                   "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId]"
                   "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id]"
                   " WHERE [n].[Visibility] = ? AND [hl].[Id] = ?  AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        {
        BeAssert(false);
        return nullptr;
        }

    int bindingIndex = 1;
    stmt->BindInt(bindingIndex++, (int)NodeVisibility::Visible);
    stmt->BindUInt64(bindingIndex++, m_hierarchyLevelId);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerialized(), Statement::MakeCopy::No);

    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedPartialDataSourceProvider::_GetNodesStatement() const
    {
    Utf8String query =
        "SELECT [n].[Data], [hl].[PhysicalParentNodeId], [hl].[VirtualParentNodeId], [n].[Id], [nk].[PathFromRoot] "
        "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
        "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id]"
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId]"
        "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id]"
        "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id]"
        "  JOIN [" NODESCACHE_TABLENAME_NodesOrder "] no ON [no].[HierarchyLevelId] = [hl].[Id] AND [no].[DataSourceId] = [ds].[Id] AND [no].[NodeId] = [n].[Id]"
        " WHERE [ds].[Id] = ? "
        "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) ";
    if (m_wantOnlyVisibleNodes)
        query.append("AND [n].[Visibility] = ? ");
    query.append("ORDER BY [no].[OrderValue]");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    int bindingIndex = 1;
    stmt->BindUInt64(bindingIndex++, m_dataSourceId);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerialized(), Statement::MakeCopy::No);
    if (m_wantOnlyVisibleNodes)
        stmt->BindInt(bindingIndex++, (int)NodeVisibility::Visible);

    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedPartialDataSourceProvider::_GetCountStatement() const
    {
    Utf8String query =
        "SELECT COUNT(1) "
        "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId]"
        "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id]"
        " WHERE [ds].[Id] = ? "
        "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) ";
    if (m_wantOnlyVisibleNodes)
        query.append("AND [n].[Visibility] = ?");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    int bindingIndex = 1;
    stmt->BindUInt64(bindingIndex++, m_dataSourceId);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerialized(), Statement::MakeCopy::No);
    if (m_wantOnlyVisibleNodes)
        stmt->BindInt(bindingIndex++, (int)NodeVisibility::Visible);

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
                       "     JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [dsn].[VariablesId]"
                       "     JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [dsn].[Id] "
                       "LEFT JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[VirtualParentNodeId] = [n].[Id] "
                       "LEFT JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id] "
                       "    WHERE     [hln].[RulesetId] = ? AND [hln].[Locale] = ? "
                       "          AND ([hl].[Id] IS NULL OR [ds].[Id] IS NULL OR NOT [ds].[IsInitialized]) "
                       "          AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindText(1, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(2, GetContext().GetLocale(), Statement::MakeCopy::No);
    stmt->BindText(3, GetContext().GetRulesetVariables().GetSerialized(), Statement::MakeCopy::No);
    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr NodesWithUndeterminedChildrenProvider::_GetNodesStatement() const
    {
    Utf8String query = "   SELECT [n].[Data], [hln].[PhysicalParentNodeId], [hln].[VirtualParentNodeId], [n].[Id], [nk].[PathFromRoot] "
                       "     FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hln "
                       "     JOIN [" NODESCACHE_TABLENAME_DataSources "] dsn ON [dsn].[HierarchyLevelId] = [hln].[Id] "
                       "     JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [dsn].[VariablesId]"
                       "     JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [dsn].[Id] "
                       "     JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id] "
                       "LEFT JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[VirtualParentNodeId] = [n].[Id] "
                       "LEFT JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id] "
                       "    WHERE     [hln].[RulesetId] = ? AND [hln].[Locale] = ? "
                       "          AND ([hl].[Id] IS NULL OR [ds].[Id] IS NULL OR NOT [ds].[IsInitialized]) "
                       "          AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) ";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindText(1, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(2, GetContext().GetLocale(), Statement::MakeCopy::No);
    stmt->BindText(3, GetContext().GetRulesetVariables().GetSerialized(), Statement::MakeCopy::No);
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
        "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [n].[DataSourceId] "
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId]"
        "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [ds].[HierarchyLevelId] "
        " WHERE [hl].[RulesetId] = ? AND [hl].[Locale] = ?  AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) AND [n].[Label] LIKE ? ESCAPE \'\\\'";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    Utf8String filter;
    filter.append("%").append(m_filter).append("%");
    stmt->BindText(1, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(2, GetContext().GetLocale().c_str(), Statement::MakeCopy::No);
    stmt->BindText(3, GetContext().GetRulesetVariables().GetSerialized(), Statement::MakeCopy::No);
    stmt->BindText(4, filter.c_str(), Statement::MakeCopy::Yes);
    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr FilteredNodesProvider::_GetNodesStatement() const
    {
    Utf8String query = "SELECT [n].[Data], [hl].[PhysicalParentNodeId], [hl].[VirtualParentNodeId], [n].[Id], [nk].[PathFromRoot] "
        "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
        "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [n].[DataSourceId] "
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId]"
        "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [ds].[HierarchyLevelId] "
        " WHERE [hl].[RulesetId] = ? AND [hl].[Locale] = ? AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) AND [n].[Label] LIKE ? ESCAPE \'\\\'";

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    Utf8String filter;
    filter.append("%").append(m_filter).append("%");
    stmt->BindText(1, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(2, GetContext().GetLocale().c_str(), Statement::MakeCopy::No);
    stmt->BindText(3, GetContext().GetRulesetVariables().GetSerialized(), Statement::MakeCopy::No);
    stmt->BindText(4, filter.c_str(), Statement::MakeCopy::Yes);
    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool CachedNodeProvider::_InitializeNodes()
    {
    JsonNavNodePtr node = GetContext().GetNodesCache().GetNode(m_nodeId);
    m_singleNodeProvider = SingleNavNodeProvider::Create(*node, GetContext());
    return true;
    }
