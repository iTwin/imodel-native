/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/Rules/SpecificationVisitor.h>
#include <algorithm>
#include "../Shared/ECExpressions/ECExpressionContextsProvider.h"
#include "../Shared/Queries/QueryBuilderHelpers.h"
#include "../Shared/ECSchemaHelper.h"
#include "../Shared/CustomizationHelper.h"
#include "NavigationQueryBuilder.h"
#include "NavigationQueryContracts.h"
#include "NavNodeProviders.h"
#include "NavNodesDataSource.h"
#include "NavNodesCache.h"
#include "NavNodesHelper.h"
#include "HierarchiesFiltering.h"

//#define CHECK_ORDERED_QUERY_PLANS
#ifdef CHECK_ORDERED_QUERY_PLANS
static void CheckQueryPlan(Db& db, Utf8CP query, StatementCache const& stmtCache)
    {
    Utf8String explainQuery = "EXPLAIN QUERY PLAN ";
    explainQuery.append(query);

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != stmtCache.GetPreparedStatement(stmt, *db.GetDbFile(), explainQuery.c_str()))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare explain query statement");

    Utf8String plan;
    Utf8String lastLine;
    while (BE_SQLITE_ROW == stmt->Step())
        {
        plan.append(stmt->GetValueText(3)).append("\n");
        lastLine = stmt->GetValueText(3);
        }

    if (lastLine.ContainsI("ORDER BY"))
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_ERROR, "Possibly a slow query plan");
        }
    }
#endif

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavNodesProvider::SpecificationsVisitor : PresentationRuleSpecificationVisitor
{
private:
    bvector<NavNodesProviderPtr> m_nodeProviders;
    NavNodesProviderContext* m_context;

private:
    void EnsureFilteringSupported(ChildNodeSpecificationCR spec) const
        {
        // the hierarchy level was requested with an instance filter - ensure the specification supports filtering
        if (m_context->GetInstanceFilter())
            ENSURE_SUPPORTS_FILTERING(spec);
        }
    void AddQueryBasedNodeProvider(ChildNodeSpecification const& specification)
        {
        auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Create nodes provider for %s", DiagnosticsHelpers::CreateRuleIdentifier(specification).c_str()));
        EnsureFilteringSupported(specification);
        NavNodesProviderPtr provider = QueryBasedSpecificationNodesProvider::Create(*m_context, specification);
        m_nodeProviders.push_back(provider);
        }
    void AddCustomNodeProvider(CustomNodeSpecification const& specification)
        {
        auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Create nodes provider for %s", DiagnosticsHelpers::CreateRuleIdentifier(specification).c_str()));
        EnsureFilteringSupported(specification);
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
    void SetContext(NavNodesProviderContextR context) {m_context = &context;}
    bvector<NavNodesProviderPtr> const& GetNodeProviders() const {return m_nodeProviders;}
};

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DataSourceRelatedVariablesTracker
{
private:
    NavNodesProviderContextCR m_context;
    DataSourceIdentifier const& m_dsIdentifier;
    RulesetVariables m_relatedVariablesBefore;
public:
    DataSourceRelatedVariablesTracker(NavNodesProviderContextCR context, DataSourceIdentifier const& id)
        : m_context(context), m_dsIdentifier(id)
        {
        m_relatedVariablesBefore = m_context.GetRelatedRulesetVariables();
        }
    std::unique_ptr<RulesetVariables> GetChangedVariables()
        {
        if (!m_dsIdentifier.IsValid())
            return nullptr;

        bvector<RulesetVariableEntry> relatedVariablesAfter = m_context.GetRelatedRulesetVariables();
        if (m_relatedVariablesBefore.Contains(relatedVariablesAfter, false))
            {
            // didn't notice any new variables
            return nullptr;
            }

        m_relatedVariablesBefore = relatedVariablesAfter;
        return std::make_unique<RulesetVariables>(relatedVariablesAfter);
        }
};
END_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<uint64_t> ProvidersIndexAllocator::_GetCurrentIndex() const
    {
    bvector<uint64_t> index = m_parentIndex;
    index.push_back(m_currIndex - 1);
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<uint64_t> ProvidersIndexAllocator::_AllocateIndex()
    {
    bvector<uint64_t> index = m_parentIndex;
    index.push_back(m_currIndex++);
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool HasNodeArtifactRules(IRulesPreprocessorR rules, ChildNodeSpecificationCR spec)
    {
    return !rules.GetNodeArtifactRules(IRulesPreprocessor::CustomizationRuleBySpecParameters(spec)).empty();
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ChildrenArtifactsCaptureContext
{
private:
    RefCountedPtr<ArtifactsCapturer> m_capturer;
    NavNodesProviderContextR m_childrenProviderContext;
    NavNodeR m_node;
public:
    ChildrenArtifactsCaptureContext(NavNodesProviderContextR childrenProviderContext, NavNodeR node)
        : m_childrenProviderContext(childrenProviderContext), m_node(node)
        {
        m_capturer = ArtifactsCapturer::Create();
        m_childrenProviderContext.AddArtifactsCapturer(m_capturer.get());
        }
    ~ChildrenArtifactsCaptureContext()
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Detected %" PRIu64 " artifacts", (uint64_t)m_capturer->GetArtifacts().size()));
        NavNodeExtendedData(m_node).SetChildrenArtifacts(m_capturer->GetArtifacts());
        m_childrenProviderContext.RemoveArtifactsCapturer(m_capturer.get());
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SetupCustomFunctionsContext
{
private:
    std::unique_ptr<CustomFunctionsContext> m_ctx;
public:
    SetupCustomFunctionsContext(NavNodesProviderContextCR providerContext, RapidJsonValueCR queryExtendedData)
        {
        IECPropertyFormatter const* formatter = providerContext.IsPropertyFormattingContext() ? &providerContext.GetECPropertyFormatter() : nullptr;
        ECPresentation::UnitSystem unitSystem = providerContext.IsPropertyFormattingContext() ? providerContext.GetUnitSystem() : ECPresentation::UnitSystem::Undefined;
        m_ctx = std::make_unique<CustomFunctionsContext>(providerContext.GetSchemaHelper(), providerContext.GetConnections(), providerContext.GetConnection(),
            providerContext.GetRuleset().GetRuleSetId(), providerContext.GetRulesPreprocessor(), providerContext.GetRulesetVariables(),
            &providerContext.GetUsedVariablesListener(), providerContext.GetECExpressionsCache(), providerContext.GetNodesFactory(), providerContext.GetUsedClassesListener(),
            providerContext.GetVirtualParentNode().get(), &queryExtendedData, formatter, unitSystem);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderContext::NavNodesProviderContext(PresentationRuleSetCR ruleset, NavNodeCP physicalParent,
    std::unique_ptr<RulesetVariables> rulesetVariables, ECExpressionsCache& ecexpressionsCache, RelatedPathsCache& relatedPathsCache,
    NavNodesFactory const& nodesFactory, std::shared_ptr<INavNodesCache> nodesCache, INodesProviderFactoryCR providerFactory, IJsonLocalState const* localState)
    : RulesDrivenProviderContext(ruleset, std::move(rulesetVariables), ecexpressionsCache, relatedPathsCache, nodesFactory, localState),
    m_nodesCache(nodesCache), m_providerFactory(providerFactory), m_physicalParentNode(physicalParent), m_requestingAllNodes(false)
    {
    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderContext::NavNodesProviderContext(NavNodesProviderContextCR other)
    : RulesDrivenProviderContext(other), m_nodesCache(other.m_nodesCache),
    m_physicalParentNode(other.m_physicalParentNode), m_providerFactory(other.m_providerFactory), m_onHierarchyLevelLoaded(other.m_onHierarchyLevelLoaded),
    m_requestingAllNodes(other.m_requestingAllNodes), m_hierarchyLevelLocker(other.m_hierarchyLevelLocker), m_instanceFilter(other.m_instanceFilter),
    m_resultSetSizeLimit(other.m_resultSetSizeLimit)
    {
    Init();

    if (other.IsQueryContext())
        SetQueryContext(other);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderContext::~NavNodesProviderContext()
    {
    DELETE_AND_CLEAR(m_usedClassesListener);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::Init()
    {
    m_ancestorContext = nullptr;
    m_isRootNodeContext = false;
    m_rootNodeRule = nullptr;
    m_isChildNodeContext = false;
    m_childNodeRule = nullptr;
    m_usedClassesListener = nullptr;
    m_pageOptions = nullptr;
    m_mayHaveArtifacts = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IProvidersIndexAllocator& NavNodesProviderContext::GetProvidersIndexAllocator() const
    {
    if (m_providersIndexAllocator.IsNull())
        {
        m_providersIndexAllocator = new ProvidersIndexAllocator();
        }
    return *m_providersIndexAllocator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
INavNodesCacheR NavNodesProviderContext::GetNodesCache() const {return *m_nodesCache;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RulesetVariableEntry> NavNodesProviderContext::GetRelatedRulesetVariables() const
    {
    auto const& ids = RulesDrivenProviderContext::GetRelatedVariablesIds();
    bvector<RulesetVariableEntry> idsWithValues;
    for (Utf8StringCR id : ids)
        idsWithValues.push_back(RulesetVariableEntry(id, GetRulesetVariables().GetJsonValue(id.c_str())));
    return idsWithValues;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bset<ArtifactsCapturer*> NavNodesProviderContext::GetArtifactsCapturers(bool onlyDirect) const
    {
    bset<ArtifactsCapturer*> result;
    ContainerHelpers::Push(result, m_artifactsCapturers);
    if (m_ancestorContext && !onlyDirect)
        ContainerHelpers::MovePush(result, m_ancestorContext->GetArtifactsCapturers(false));
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
OptimizationFlagsContainer NavNodesProviderContext::GetMergedOptimizationFlags() const
    {
    OptimizationFlagsContainer merged;
    merged.Merge(m_optFlags);
    if (m_ancestorContext)
        merged.Merge(m_ancestorContext->GetMergedOptimizationFlags());
    return merged;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NavNodesProviderContext::CreateHierarchyLevelProvider(NavNodesProviderContextR context, NavNodeCP parentNode) const
    {
    GetHierarchyLevelLocker().WaitForUnlock();
    if (!context.HasPageOffset() && context.GetResultSetSizeLimit().IsNull())
        {
        BeGuid hlId = m_nodesCache->FindHierarchyLevelId(
            context.GetConnection().GetId().c_str(),
            context.GetRuleset().GetRuleSetId().c_str(),
            parentNode->GetNodeId(),
            context.GetRemovalId());
        NavNodesProviderPtr provider = hlId.IsValid() ? m_nodesCache->GetHierarchyLevel(context, hlId) : nullptr;
        if (provider.IsValid())
            return provider;
        }
    return m_providerFactory.Create(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetRootNodeContext(RootNodeRuleCP rootNodeRule)
    {
    m_isRootNodeContext = true;
    m_rootNodeRule = rootNodeRule;
    SetVirtualParentNode(m_physicalParentNode.IsValid() ? m_physicalParentNode.get() : nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetRootNodeContext(NavNodesProviderContextCR other)
    {
    m_isRootNodeContext = other.m_isRootNodeContext;
    m_rootNodeRule = other.m_rootNodeRule;
    SetVirtualParentNode(other.m_virtualParentNode.IsValid() ? other.m_virtualParentNode.get() : nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetChildNodeContext(ChildNodeRuleCP childNodeRule, NavNodeCR virtualParentNode)
    {
    m_isChildNodeContext = true;
    m_childNodeRule = childNodeRule;
    SetVirtualParentNode(&virtualParentNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetChildNodeContext(NavNodesProviderContextCR other)
    {
    m_isChildNodeContext = other.m_isChildNodeContext;
    m_childNodeRule = other.m_childNodeRule;
    SetVirtualParentNode(other.m_virtualParentNode.IsValid() ? other.m_virtualParentNode.get() : nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetQueryContext(IConnectionManagerCR connections, IConnectionCR connection, IECDbUsedClassesListener* usedClassesListener)
    {
    RulesDrivenProviderContext::SetQueryContext(connections, connection);
    if (nullptr != usedClassesListener)
        m_usedClassesListener = new ECDbUsedClassesListenerWrapper(GetConnection(), *usedClassesListener);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProviderContext::SetQueryContext(NavNodesProviderContextCR other)
    {
    RulesDrivenProviderContext::SetQueryContext(other);
    if (nullptr != other.m_usedClassesListener)
        m_usedClassesListener = new ECDbUsedClassesListenerWrapper(*other.m_usedClassesListener);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IUsedClassesListener* NavNodesProviderContext::GetUsedClassesListener() const {return m_usedClassesListener;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CombinedHierarchyLevelIdentifier NavNodesProviderContext::GetHierarchyLevelIdentifier() const
    {
    CombinedHierarchyLevelIdentifier id(
        GetConnection().GetId(),
        GetRuleset().GetRuleSetId(),
        GetPhysicalParentNode().IsValid() ? GetPhysicalParentNode()->GetNodeId() : BeGuid());
    id.SetRemovalId(m_removalId);
    return id;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NoopHierarchyLevelLocker : IHierarchyLevelLocker
{
protected:
    bool _Lock(int) override {return true;}
    void _Unlock() override {}
    void _WaitForUnlock() override {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IHierarchyLevelLocker& NavNodesProviderContext::GetHierarchyLevelLocker() const
    {
    static NoopHierarchyLevelLocker noopLocker;
    return m_hierarchyLevelLocker ? *m_hierarchyLevelLocker : noopLocker;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProvider::NavNodesProvider(NavNodesProviderContextR context)
    : m_context(const_cast<NavNodesProviderContext*>(&context)), m_nodesInitialized(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProvider::InitializeNodes()
    {
    auto scope = Diagnostics::Scope::Create("Initialize nodes");

    ThrowIfCancelled(GetContext().GetCancelationToken());

    if (m_nodesInitialized)
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Nodes already initialized");
        return;
        }

    if (GetContext().HasPageOptions() && GetContext().GetPageOptions()->HasSize() && 0 == GetContext().GetPageOptions()->GetSize())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Skipping initialization due to 0 page size");
        _ResetInitializedNodes();
        return;
        }

    GetContext().GetHierarchyLevelLocker().WaitForUnlock();
    NodesInitializationState initState = _InitializeNodes();
    if (SUCCESS != initState.GetStatus())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Nodes not initialized - _InitializeNodes returned 'false'");
        return;
        }

    _OnNodesInitialized(initState);

    if (initState.IsFullyInitialized())
        {
        m_nodesInitialized = true;
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Nodes initialized fully");
        }
    else if (GetContext().HasPageOptions() && initState.IsPageInitialized(*GetContext().GetPageOptions()))
        {
        m_nodesInitialized = true;
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Page of nodes initialized");
        }
    else
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Nodes initialized partially");
        }
    }

/*---------------------------------------------------------------------------------**//**
* Note: ancestorContext doesn't necessarily mean immediate parent context
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static NavNodesProviderContextPtr CreateContextForChildHierarchyLevel(NavNodesProviderContextCR ancestorContext, NavNodeCR parentNode)
    {
    NavNodesProviderContextPtr ctx = NavNodesProviderContext::Create(ancestorContext);
    ctx->SetAncestorContext(&ancestorContext);
    ctx->SetChildNodeContext(nullptr, parentNode);
    if (NodeVisibility::Virtual == ancestorContext.GetNodesCache().GetNodeVisibility(parentNode.GetNodeId(), ctx->GetRulesetVariables(), ctx->GetInstanceFilter()))
        ctx->SetPhysicalParentNode(ancestorContext.GetNodesCache().GetPhysicalParentNode(parentNode.GetNodeId(), ctx->GetRulesetVariables(), ctx->GetInstanceFilter()).get());
    else
        ctx->SetPhysicalParentNode(&parentNode);
    if (!NavNodeExtendedData(parentNode).HideNodesInHierarchy() && nullptr == parentNode.GetKey()->AsGroupingNodeKey())
        ctx->SetInstanceFilter(nullptr);
    ctx->SetRemovalId(ancestorContext.GetRemovalId());
    return ctx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static NavNodesProviderContextPtr CreateContextForSameHierarchyLevel(NavNodesProviderContextCR baseContext, bset<Utf8String> const& usedVariableIds, bool copyNodesContext)
    {
    NavNodesProviderContextPtr ctx = NavNodesProviderContext::Create(baseContext);
    ctx->SetAncestorContext(&baseContext);
    ctx->SetProvidersIndexAllocator(baseContext.GetProvidersIndexAllocator());
    ctx->SetMayHaveArtifacts(baseContext.MayHaveArtifacts());
    ctx->InitUsedVariablesListener(usedVariableIds, &baseContext.GetUsedVariablesListener());
    ctx->SetRemovalId(baseContext.GetRemovalId());

    if (copyNodesContext)
        {
        if (baseContext.IsRootNodeContext())
            ctx->SetRootNodeContext(baseContext);
        else if (baseContext.IsChildNodeContext())
            ctx->SetChildNodeContext(baseContext);
        }
    else
        {
        ctx->SetPhysicalParentNode(baseContext.GetPhysicalParentNode().get());
        ctx->SetVirtualParentNode(baseContext.GetVirtualParentNode().get());
        }
    return ctx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProvider::SetPageOptions(std::shared_ptr<PageOptions> pageOptions)
    {
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Hierarchies, !GetContext().HasPageOptions(), "Setting page options on a provider that already has them.");
    GetContextR().SetPageOptions(pageOptions);
    _OnPageOptionsSet();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProvider::_OnPageOptionsSet()
    {
    m_nodesInitialized = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t NavNodesProviderContext::PageOptions::GetAdjustedPageSize(size_t totalCount) const
    {
    if (GetStart() >= totalCount || (HasSize() && GetSize() == 0))
        return 0;

    size_t countFromPageStart = totalCount - GetStart();
    if (!HasSize() || GetSize() > countFromPageStart)
        return countFromPageStart;

    return GetSize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TResult>
static TResult GetResultOrLockHierarchy(IHierarchyLevelLockerR locker, std::function<bpair<bool, TResult>()> getter)
    {
    // if expected result is returned immediately no need to additionally check if hierarchy level is not locked.
    bpair<bool, TResult> result = getter();
    if (result.first)
        return std::move(result.second);

    bool lockedLevel = false;
    while (true)
        {
        // if hierarchy level is locked wait for it to be unlocked and try to get result
        locker.WaitForUnlock();
        result = getter();
        // if expected result is returned or hierarchy level lock is acquired return current result
        if (result.first || lockedLevel)
            break;

        // attempt to acquire hierarchy level lock
        lockedLevel = locker.Lock(IHierarchyLevelLocker::LockOptions::DisableLockWait);
        }
    return std::move(result.second);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodesProvider::HasNodes() const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("%s: Has nodes?", GetName()));
    if (m_cachedHasNodes.IsNull())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "'Has nodes' flag not cached in memory");

        Nullable<bool> cachedHasNodesInfo = GetResultOrLockHierarchy<Nullable<bool>>(GetContext().GetHierarchyLevelLocker(),
            [&]()
            {
            if (!GetIdentifier().IsValid())
                return make_bpair(true, Nullable<bool>());

            Nullable<bool> hasNodes = GetContext().GetNodesCache().FindDataSource(GetIdentifier(), GetContext().GetRulesetVariables(), DataSourceInfo::PART_HasNodes).HasNodes();
            return make_bpair(hasNodes.IsValid(), hasNodes);
            });

        if (cachedHasNodesInfo.IsValid())
            {
            m_cachedHasNodes = cachedHasNodesInfo.Value();
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("'Has nodes' flag found in persistent cache: `%s`", m_cachedHasNodes.Value() ? "TRUE" : "FALSE"));
            }
        else
            {
            auto parent = GetContext().GetVirtualParentNode();
            if (parent.IsValid())
                {
                if (parent->DeterminedChildren())
                    {
                    m_cachedHasNodes = parent->HasChildren();
                    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("'Has nodes' flag determined from parent: `%s`", m_cachedHasNodes.Value() ? "TRUE" : "FALSE"));
                    }
                else if (!RequiresFullLoad())
                    {
                    switch (NavNodeExtendedData(*parent).GetChildrenHint())
                        {
                        case ChildrenHint::Always: m_cachedHasNodes = true; break;
                        case ChildrenHint::Never: m_cachedHasNodes = false; break;
                        }
                    if (m_cachedHasNodes.IsValid())
                        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("'Has nodes' flag determined from children hint: `%s`", m_cachedHasNodes.Value() ? "TRUE" : "FALSE"));
                    }
                }
            if (m_cachedHasNodes.IsNull())
                {
                MaxNodesToLoadContext checkingNodes(*this, 1);
                m_cachedHasNodes = _HasNodes();
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("'Has nodes' flag calculated: `%s`", m_cachedHasNodes.Value() ? "TRUE" : "FALSE"));
                }
            const_cast<NavNodesProviderP>(this)->_OnHasNodesFlagSet(m_cachedHasNodes.Value());
            }
        }
    return m_cachedHasNodes.Value();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CountInfo NavNodesProvider::GetTotalNodesCount() const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("%s: Get nodes count", GetName()));
    if (m_cachedNodesCount.IsNull())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Nodes count not cached in memory");
        Nullable<uint64_t> cachedTotalNodesCount = GetResultOrLockHierarchy<Nullable<uint64_t>>(GetContext().GetHierarchyLevelLocker(),
            [&]()
            {
            if (!GetIdentifier().IsValid())
                return make_bpair(true, Nullable<uint64_t>());

            Nullable<uint64_t> totalCount = GetContext().GetNodesCache().FindDataSource(GetIdentifier(), GetContext().GetRulesetVariables(), DataSourceInfo::PART_TotalNodesCount).GetTotalNodesCount();
            return make_bpair(totalCount.IsValid(), totalCount);
            });

        if (cachedTotalNodesCount.IsValid())
            {
            m_cachedNodesCount = (size_t)cachedTotalNodesCount.Value();
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Nodes count found in persistent cache: %" PRIu64, m_cachedNodesCount.Value()));
            }
        else if (GetContext().GetMergedOptimizationFlags().GetMaxNodesToLoad() == 1)
            {
            // max nodes to load set to 1 means we're just checking whether provider returns any nodes - no need to
            // count anything...
            auto hasNodes = HasNodes();
            if (hasNodes)
                return CountInfo(1, false);

            m_cachedNodesCount = (size_t)0;
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Nodes count deduced from 'has nodes' flag: %" PRIu64, m_cachedNodesCount.Value()));
            }
        else
            {
            CountInfo count = _GetTotalNodesCount();
            if (!count.IsAccurate())
                return count;

            m_cachedNodesCount = count.GetCount();
            const_cast<NavNodesProviderP>(this)->_OnNodesCountSet(m_cachedNodesCount.Value());
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Calculated nodes count: %" PRIu64, m_cachedNodesCount.Value()));
            }
        }
    return CountInfo(m_cachedNodesCount.Value(), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t NavNodesProvider::GetNodesCount() const
    {
    auto count = GetTotalNodesCount();
    if (count.IsAccurate() && GetContext().HasPageOptions())
        return GetContext().GetPageOptions()->GetAdjustedPageSize(count.GetCount());
    return count.GetCount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProvider::Iterator NavNodesProvider::begin() const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("%s: begin iterator", GetName()));
    const_cast<NavNodesProviderP>(this)->InitializeNodes();
    return _CreateFrontIterator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProvider::Iterator NavNodesProvider::end() const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("%s: end iterator", GetName()));
    const_cast<NavNodesProviderP>(this)->InitializeNodes();
    return _CreateBackIterator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NodesFinalizer::Finalize(NavNodeR node) const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Finalize node %s", DiagnosticsHelpers::CreateNodeIdentifier(node).c_str()));
    Customize(node);
    DetermineChildren(node);
    DetermineFilteringSupport(node);
    return &node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesFinalizer::Customize(NavNodeR node) const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Customize %s", DiagnosticsHelpers::CreateNodeIdentifier(node).c_str()));
    if (NavNodeExtendedData(node).IsCustomized())
        return;

    // note: we can't use `context.GetVirtualParentNode()` because the node might come from
    // deeper hierarchy levels than the context
    NavNodeCPtr parentNode;
    auto parentIds = NavNodeExtendedData(node).GetVirtualParentIds();
    if (!parentIds.empty())
        parentNode = m_context->GetNodesCache().GetNode(parentIds.front());

    CustomizationHelper::Customize(*m_context, parentNode.get(), node);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodeHasChildrenFlagUpdater
    {
    IHierarchyCacheR m_cache;
    NavNodePtr m_node;
    HasChildrenFlag const& m_flag;
    HasChildrenFlag m_initialFlag;

    NodeHasChildrenFlagUpdater(IHierarchyCacheR cache, NavNodePtr node, HasChildrenFlag const& flag)
        : m_cache(cache), m_node(node), m_flag(flag), m_initialFlag(flag)
        {}
    ~NodeHasChildrenFlagUpdater() { Update(); }
    void Update()
        {
        if (m_flag == m_initialFlag)
            return;
        m_node->SetHasChildren(HASCHILDREN_True == m_flag);
        m_initialFlag = m_flag;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool NeedsChildrenArtifactsDetermined(Utf8CP expression) { return strstr(expression, ".ChildrenArtifacts"); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool NeedsChildrenArtifactsDetermined(NavNodeExtendedData const& extendedData)
    {
    return extendedData.HasHideExpression() && NeedsChildrenArtifactsDetermined(extendedData.GetHideExpression());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
HasChildrenFlag NodesFinalizer::AnyChildSpecificationReturnsNodes(NavNodeR parentNode) const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Any child of %s specification returns nodes?", DiagnosticsHelpers::CreateNodeIdentifier(parentNode).c_str()));

    NavNodesProviderContextPtr childrenContext = CreateContextForChildHierarchyLevel(*m_context, parentNode);
    // we consider that this provider depends on a variable if we need the variable to determine
    // if node of this provider has children
    childrenContext->InitUsedVariablesListener({}, &m_context->GetUsedVariablesListener());

    NavNodesProviderPtr childrenProvider = m_context->CreateHierarchyLevelProvider(*childrenContext, &parentNode);
    return childrenProvider->HasNodes() ? HASCHILDREN_True : HASCHILDREN_False;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesFinalizer::DetermineChildren(NavNodeR node) const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Determine children for %s", DiagnosticsHelpers::CreateNodeIdentifier(node).c_str()));
    if (node.DeterminedChildren())
        return;

    NavNodeExtendedData extendedData(node);
    switch (extendedData.GetChildrenHint())
        {
        case ChildrenHint::Always: node.SetHasChildren(true); return;
        case ChildrenHint::Never: node.SetHasChildren(false); return;
        default: break;
        }

    // returning a node without children if we already have a similar ancestor (prevent infinite hierarchies)
    if (m_context->IsChildNodeContext() && HasSimilarNodeInHierarchy(node))
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Node has similar ancestor - marking has children false.");
        node.SetHasChildren(false);
        return;
        }

    node.SetHasChildren(HASCHILDREN_True == AnyChildSpecificationReturnsNodes(node));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesFinalizer::DetermineFilteringSupport(NavNodeR node) const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Determine filtering support for %s", DiagnosticsHelpers::CreateNodeIdentifier(node).c_str()));
    if (node.IsFilteringSupportDetermined())
        return;

    NavNodesProviderContextPtr childrenContext = CreateContextForChildHierarchyLevel(*m_context, node);
    // we consider that this provider depends on a variable if we need the variable to determine
    // if node supports filtering
    childrenContext->InitUsedVariablesListener({}, &m_context->GetUsedVariablesListener());

    node.SetSupportsFiltering(HierarchiesFilteringHelper::SupportsFiltering(
        &node,
        TraverseHierarchyRulesProps(childrenContext->GetNodesCache(), childrenContext->GetNodesFactory(),
            childrenContext->GetRulesPreprocessor(), childrenContext->GetRuleset(), childrenContext->GetSchemaHelper()),
        nullptr
        ));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesFinalizer::HasSimilarNodeInHierarchy(NavNodeCR node, NavNodeCR parentNode, int suppressCount) const
    {
    Utf8StringCR nodeHash = node.GetKey()->GetHash();
    Utf8StringCR parentHash = parentNode.GetKey()->GetHash();
    bool areNodesSimilar = nodeHash.Equals(parentHash);
    if (areNodesSimilar && node.GetKey()->AsGroupingNodeKey() && parentNode.GetKey()->AsGroupingNodeKey())
        {
        areNodesSimilar = (node.GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount() == parentNode.GetKey()->AsGroupingNodeKey()->GetGroupedInstancesCount());
        if (areNodesSimilar)
            {
            if (node.GetKey()->GetInstanceKeysSelectQuery() == nullptr || parentNode.GetKey()->GetInstanceKeysSelectQuery() == nullptr)
                {
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_ERROR, "Grouping node has no instance keys select query.");
                areNodesSimilar = false;
                }
            else
                {
                // the query returns all rows that are only returned by one of the queries
                auto query = UnionQueryBuilder::Create({
                    ExceptQueryBuilder::Create(*StringQueryBuilder::Create(*node.GetKey()->GetInstanceKeysSelectQuery()), *StringQueryBuilder::Create(*parentNode.GetKey()->GetInstanceKeysSelectQuery())),
                    ExceptQueryBuilder::Create(*StringQueryBuilder::Create(*parentNode.GetKey()->GetInstanceKeysSelectQuery()), *StringQueryBuilder::Create(*node.GetKey()->GetInstanceKeysSelectQuery())),
                    });
                static GenericQueryResultReader<bool> s_rowsExistReader([](ECSqlStatementCR){return true;});
                SetupCustomFunctionsContext fnContext(*m_context, query->GetExtendedData());
                QueryExecutor executor(m_context->GetConnection(), *query->GetQuery());
                areNodesSimilar = !executor.ReadNext<bool>(s_rowsExistReader);
                }
            }
        }

    if (areNodesSimilar && --suppressCount <= 0)
        return true;

    NavNodeExtendedData parentNodeExtendedData(parentNode);
    return ContainerHelpers::Contains(parentNodeExtendedData.GetVirtualParentIds(), [&](BeGuidCR grandParentNodeId)
        {
        return HasSimilarNodeInHierarchy(node, grandParentNodeId, suppressCount);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesFinalizer::HasSimilarNodeInHierarchy(NavNodeCR node, BeGuidCR parentNodeId, int suppressCount) const
    {
    NavNodeCPtr parentNode = m_context->GetNodesCache().GetNode(parentNodeId);
    if (parentNode.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to find parent node by ID");
    return HasSimilarNodeInHierarchy(node, *parentNode, suppressCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesFinalizer::HasSimilarNodeInHierarchy(NavNodeCR node, int suppressCount) const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Check if %s similar node in hierarchy", DiagnosticsHelpers::CreateNodeIdentifier(node).c_str()));

    if (-1 == suppressCount)
        suppressCount = NavNodeExtendedData(node).GetAllowedSimilarAncestors();

    NavNodeExtendedData thisNodeExtendedData(node);
    return ContainerHelpers::Contains(thisNodeExtendedData.GetVirtualParentIds(), [&](BeGuidCR parentNodeId)
        {
        return HasSimilarNodeInHierarchy(node, parentNodeId, suppressCount);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesFinalizer::HasSimilarNodeInHierarchy(NavNodeCR node) const
    {
    return HasSimilarNodeInHierarchy(node, -1);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesCreatingMultiNavNodesProvider::CreateNodeProviderContext
{
private:
    NavNodeR m_node;
    NavNodesProviderContextCR m_nodeContext;
    NavNodesProviderPtr m_childrenProvider;
public:
    CreateNodeProviderContext(NavNodeR node, NavNodesProviderContextCR ctx) : m_node(node), m_nodeContext(ctx) {}
    NavNodeR GetNode() {return m_node;}
    NavNodesProviderContextCR GetNodeContext() const {return m_nodeContext;}
    NavNodesProviderPtr GetChildrenProvider() const {return m_childrenProvider;}
    void SetChildrenProvider(NavNodesProviderPtr provider) {m_childrenProvider = provider;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static NavNodesProviderPtr GetOrCreateChildrenProvider(NodesCreatingMultiNavNodesProvider::CreateNodeProviderContext& ctx)
    {
    if (ctx.GetChildrenProvider().IsNull())
        {
        NavNodesProviderContextPtr childrenContext = CreateContextForChildHierarchyLevel(ctx.GetNodeContext(), ctx.GetNode());
        NavNodesProviderPtr childrenProvider = ctx.GetNodeContext().CreateHierarchyLevelProvider(*childrenContext, &ctx.GetNode());
        ctx.SetChildrenProvider(childrenProvider);
        }
    return ctx.GetChildrenProvider();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCreatingMultiNavNodesProvider::EvaluateChildrenArtifacts(CreateNodeProviderContext& nodeProviderContext) const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("%s: Evaluate children artifacts for %s", GetName(), DiagnosticsHelpers::CreateNodeIdentifier(nodeProviderContext.GetNode()).c_str()));

    NavNodeExtendedData ext(nodeProviderContext.GetNode());

    if (!NeedsChildrenArtifactsDetermined(ext))
        return;

    if (ext.GetJson().HasMember(NAVNODE_EXTENDEDDATA_ChildrenArtifacts))
        return;

    auto childrenProvider = GetOrCreateChildrenProvider(nodeProviderContext);
    ChildrenArtifactsCaptureContext captureChildrenArtifacts(childrenProvider->GetContextR(), nodeProviderContext.GetNode());
    DisabledFullNodesLoadContext disableFullLoad(*childrenProvider);
    DisabledPostProcessingContext disablePostProcessing(*childrenProvider);
    for (auto const node : *childrenProvider)
        ;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCreatingMultiNavNodesProvider::EvaluateThisNodeArtifacts(NavNodeCR node) const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("%s: Evaluate artifacts for %s", GetName(), DiagnosticsHelpers::CreateNodeIdentifier(node).c_str()));

    if (!GetContext().MayHaveArtifacts())
        return;

    auto capturers = GetContext().GetArtifactsCapturers();
    if (capturers.empty())
        return;

    NodeArtifacts artifacts = CustomizationHelper::EvaluateArtifacts(GetContext(), node);
    if (!artifacts.empty())
        {
        for (ArtifactsCapturer* capturer : capturers)
            capturer->AddArtifact(artifacts);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool NeedsChildrenDetermined(Utf8CP expression) {return strstr(expression, ".HasChildren") || NeedsChildrenArtifactsDetermined(expression);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool NeedsChildrenDetermined(NavNodeExtendedData const& extendedData)
    {
    return extendedData.HideIfNoChildren()
        || extendedData.HasHideExpression() && NeedsChildrenDetermined(extendedData.GetHideExpression());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ShouldHideNodeBasedOnHideExpression(NavNodesProviderContextCR context, NavNodeCR node, Utf8StringCR expression)
    {
    if (expression.empty())
        return false;

    NavNodeCPtr parentNode = context.GetVirtualParentNode();
    ECExpressionContextsProvider::CustomizationRulesContextParameters params(node, parentNode.get(),
        context.GetConnection(), context.GetRulesetVariables(), &context.GetUsedVariablesListener());
    ExpressionContextPtr expressionContext = ECExpressionContextsProvider::GetCustomizationRulesContext(params);
    ECValue value;
    if (ECExpressionEvaluationStatus::Success == ECExpressionsHelper(context.GetECExpressionsCache()).EvaluateECExpression(value, expression, *expressionContext) && value.IsPrimitive() && value.ConvertToPrimitiveType(PRIMITIVETYPE_Boolean))
        return value.GetBoolean();
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ContainsOnlyNullOrEmptyStringValues(RapidJsonValueCR jsonArr)
    {
    bool containsNonEmptyValues = false;
    for (rapidjson::SizeType i = 0; i < jsonArr.Size(); ++i)
        {
        if (jsonArr[i].IsNull())
            continue;
        if (jsonArr[i].IsString() && 0 == *jsonArr[i].GetString())
            continue;
        containsNonEmptyValues = true;
        break;
        }
    return !containsNonEmptyValues;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCreatingMultiNavNodesProvider::ShouldReturnChildNodes(CreateNodeProviderContext& nodeProviderContext) const
    {
    NavNodeR node = nodeProviderContext.GetNode();
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("%s: Should return %s child nodes?", GetName(), DiagnosticsHelpers::CreateNodeIdentifier(node).c_str()));

    NavNodeExtendedData extendedData(node);

    // when the node is created using a specification with "hide nodes in hierarchy" flag
    if (extendedData.HideNodesInHierarchy())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "`TRUE`, node has 'hide nodes in hierarchy' flag");
        return true;
        }

    // if the node has "hide if grouping value not specified" flag and is a value property grouping node, grouping null or "" values, show its children
    if (extendedData.HideIfGroupingValueNotSpecified() && node.GetType().Equals(NAVNODE_TYPE_ECPropertyGroupingNode) && extendedData.HasPropertyValues() && ContainsOnlyNullOrEmptyStringValues(extendedData.GetPropertyValues()))
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "`TRUE`, node has 'hide if grouping value not specified' flag and no grouping value");
        return true;
        }

    if (node.DeterminedChildren() && !node.HasChildren())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "`FALSE`, node has been determined to have no children");
        return false;
        }

    HasChildrenFlag hasChildren = HASCHILDREN_Unknown;
    NodeHasChildrenFlagUpdater hasChildrenUpdater(GetContext().GetNodesCache(), &node, hasChildren);

    if (ChildrenHint::Always == extendedData.GetChildrenHint())
        {
        hasChildren = HASCHILDREN_True;
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Node has children based on children hint");
        }
    else if (ChildrenHint::Never == extendedData.GetChildrenHint() && !node.GetKey()->AsGroupingNodeKey())
        {
        hasChildren = HASCHILDREN_False;
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Node has no children based on children hint");
        }

    // if the node has only one child and also has "hide if only one child" flag, we want to display that child
    if (extendedData.HideIfOnlyOneChild())
        {
        NavNodesProviderPtr childrenProvider = GetOrCreateChildrenProvider(nodeProviderContext);
        MaxNodesToLoadContext maxNodesToLoad(*childrenProvider, 2);
        size_t childrenCount = childrenProvider->GetNodesCount();
        hasChildren = (childrenCount > 0) ? HASCHILDREN_True : HASCHILDREN_False;
        if (childrenCount == 1)
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "`TRUE`, node has 'hide if only on child' flag and one child");
            return true;
            }
        }

    // determine children if absolutely necessary for furher processing
    // - still don't have the result
    // - have the result, but need children artifacts - they're evaluated while checking children
    bool needsChildrenDetermined = (HASCHILDREN_Unknown == hasChildren && NeedsChildrenDetermined(extendedData))
        || NeedsChildrenArtifactsDetermined(extendedData);
    if (needsChildrenDetermined)
        hasChildren = NodesFinalizer(GetContextR()).Finalize(node)->HasChildren() ? HASCHILDREN_True : HASCHILDREN_False;

    hasChildrenUpdater.Update();

    // if node has 'hide if no children' and it has no children, hide it
    if (extendedData.HideIfNoChildren() && HASCHILDREN_False == hasChildren)
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "`TRUE`, node has 'hide if no children' flag and no children");
        return true;
        }

    // when the node is created using a specification with "hide expression" flag
    if (extendedData.HasHideExpression() && ShouldHideNodeBasedOnHideExpression(GetContext(), node, extendedData.GetHideExpression()))
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "`TRUE`, node has hide expression that evaluates to `TRUE`");
        return true;
        }

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "`FALSE`");
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TProvider>
HierarchyLevelIdentifier const& CachingNavNodesProviderBase<TProvider>::GetOrCreateHierarchyLevelIdentifier() const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("%s: Get or create hierarchy level identifier", TProvider::GetName()));

    if (m_hierarchyLevelIdentifier.IsValid())
        return m_hierarchyLevelIdentifier;

    NavNodesProviderContextCR context = TProvider::GetContext();
    auto combinedIdentifier = context.GetHierarchyLevelIdentifier();
    BeGuid virtualParentId = context.GetVirtualParentNode().IsValid() ? context.GetVirtualParentNode()->GetNodeId() : BeGuid();

    // try to get hierarchy identifier from cache and lock hierarchy level if it's not cached yet
    m_hierarchyLevelIdentifier = GetResultOrLockHierarchy<HierarchyLevelIdentifier>(context.GetHierarchyLevelLocker(), [&]()
        {
        HierarchyLevelIdentifier identifier(
            combinedIdentifier.GetConnectionId().c_str(),
            combinedIdentifier.GetRulesetId().c_str(),
            virtualParentId,
            combinedIdentifier.GetRemovalId());
        identifier.SetId(context.GetNodesCache().FindHierarchyLevelId(
            combinedIdentifier.GetConnectionId().c_str(),
            combinedIdentifier.GetRulesetId().c_str(),
            virtualParentId,
            combinedIdentifier.GetRemovalId()));
        return make_bpair<bool, HierarchyLevelIdentifier>(identifier.IsValid(), identifier);
        });

    if (context.GetHierarchyLevelLoadedCallback() && m_hierarchyLevelIdentifier.IsValid())
        context.GetHierarchyLevelLoadedCallback()(const_cast<NavNodesProviderContextR>(context));

    if (!m_hierarchyLevelIdentifier.IsValid())
        {
        IHierarchyCache::SavepointPtr savepoint = context.GetNodesCache().CreateSavepoint();
        m_hierarchyLevelIdentifier = HierarchyLevelIdentifier(combinedIdentifier, virtualParentId);
        context.GetNodesCache().Cache(m_hierarchyLevelIdentifier);
        }

    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Hierarchies, m_hierarchyLevelIdentifier.IsValid(), "Failed to create a hierarchy level identifier");
    return m_hierarchyLevelIdentifier;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TProvider>
DataSourceIdentifier const& CachingNavNodesProviderBase<TProvider>::GetOrCreateDataSourceIdentifier(bool* createdNew) const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("%s: Get or create data source identifier", TProvider::GetName()));

    NavNodesProviderContextCR context = TProvider::GetContext();
    if (!m_datasourceIdentifier.IsValid())
        {
        DataSourceIdentifier lookup(
            GetOrCreateHierarchyLevelIdentifier().GetId(),
            context.GetProvidersIndexAllocator().AllocateIndex(),
            context.GetInstanceFilterPtr());
        lookup.SetResultSetSizeLimit(context.GetResultSetSizeLimit());

        // try to get data source identifier from cache and lock hierarchy level if it's not cached yet
        m_datasourceIdentifier = GetResultOrLockHierarchy<DataSourceIdentifier>(context.GetHierarchyLevelLocker(), [&]()
            {
            DataSourceIdentifier identifier = context.GetNodesCache().FindDataSource(lookup, context.GetRulesetVariables()).GetIdentifier();
            return make_bpair(identifier.IsValid(), identifier);
            });

        if (!m_datasourceIdentifier.IsValid())
            {
            IHierarchyCache::SavepointPtr savepoint = context.GetNodesCache().CreateSavepoint();
            DataSourceInfo datasourceInfo(lookup, context.GetRelatedRulesetVariables(), DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
            context.GetNodesCache().Cache(datasourceInfo);
            m_datasourceIdentifier = datasourceInfo.GetIdentifier();
            if (nullptr != createdNew)
                *createdNew = true;
            }
        }

    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Hierarchies, m_datasourceIdentifier.IsValid(), "Failed to create a data source identifier");
    return m_datasourceIdentifier;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TProvider>
CachingNavNodesProviderBase<TProvider>::CachingNavNodesProviderBase(NavNodesProviderContextR context)
    : TProvider(context)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TProvider>
CachingNavNodesProviderBase<TProvider>::~CachingNavNodesProviderBase()
    {
    // only here to hide use of `DataSourceRelatedVariablesTracker`
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TProvider>
void CachingNavNodesProviderBase<TProvider>::OnCreated()
    {
    // call to ensure it's cached if necessary
    bool createdNew = false;
    auto const& identifier = GetOrCreateDataSourceIdentifier(&createdNew);
    if (createdNew)
        _OnFirstCreate();

    m_variablesTracker = std::make_unique<DataSourceRelatedVariablesTracker>(TProvider::GetContext(), identifier);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TProvider>
void CachingNavNodesProviderBase<TProvider>::UpdateDataSourceDirectNodesCount(DataSourceIdentifier const& id, size_t value)
    {
    DataSourceInfo info(id);
    int partsToUpdate = 0;

    info.SetDirectNodesCount(value);
    partsToUpdate |= DataSourceInfo::PART_DirectNodesCount;

    if (auto changedVars = m_variablesTracker->GetChangedVariables())
        {
        info.SetRelatedVariables(*changedVars);
        partsToUpdate |= DataSourceInfo::PART_Vars;
        }

    TProvider::GetContext().GetNodesCache().Update(info, partsToUpdate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TProvider>
void CachingNavNodesProviderBase<TProvider>::UpdateDataSourceHasNodesFlag(DataSourceIdentifier const& id, bool value)
    {
    NavNodesProviderContextCR context = TProvider::GetContext();
    if (context.GetNodesCache().FindDataSource(id, context.GetRulesetVariables(), DataSourceInfo::PART_HasNodes).HasNodes().IsValid())
        return;

    DataSourceInfo info(id);
    int partsToUpdate = 0;

    info.SetHasNodes(value);
    partsToUpdate |= DataSourceInfo::PART_HasNodes;

    if (!value)
        {
        info.SetIsInitialized(true);
        info.SetTotalNodesCount((size_t)0);
        partsToUpdate |= DataSourceInfo::PART_IsFinalized | DataSourceInfo::PART_TotalNodesCount;
        }

    if (auto changedVars = m_variablesTracker->GetChangedVariables())
        {
        info.SetRelatedVariables(*changedVars);
        partsToUpdate |= DataSourceInfo::PART_Vars;
        }

    context.GetNodesCache().Update(info, partsToUpdate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TProvider>
void CachingNavNodesProviderBase<TProvider>::UpdateDataSourceTotalNodesCount(DataSourceIdentifier const& id, size_t totalNodesCount)
    {
    NavNodesProviderContextCR context = TProvider::GetContext();
    if (context.GetNodesCache().FindDataSource(id, context.GetRulesetVariables(), DataSourceInfo::PART_TotalNodesCount).GetTotalNodesCount().IsValid())
        return;

    DataSourceInfo info(id);
    int partsToUpdate = 0;

    info.SetTotalNodesCount(totalNodesCount);
    info.SetHasNodes(totalNodesCount > 0);
    partsToUpdate |= DataSourceInfo::PART_TotalNodesCount | DataSourceInfo::PART_HasNodes;

    if (0 == totalNodesCount)
        {
        info.SetIsInitialized(true);
        partsToUpdate |= DataSourceInfo::PART_IsFinalized;
        }

    if (auto changedVars = m_variablesTracker->GetChangedVariables())
        {
        info.SetRelatedVariables(*changedVars);
        partsToUpdate |= DataSourceInfo::PART_Vars;
        }

    context.GetNodesCache().Update(info, partsToUpdate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TProvider>
void CachingNavNodesProviderBase<TProvider>::UpdateInitializedNodesState(DataSourceIdentifier const& id, NodesInitializationState const& state)
    {
    if (SUCCESS != state.GetStatus())
        return;

    NavNodesProviderContextCR context = TProvider::GetContext();

    DataSourceInfo info(id);
    int partsToUpdate = 0;

    if (state.IsFullyInitialized() && !context.GetNodesCache().FindDataSource(id, context.GetRulesetVariables(), DataSourceInfo::PART_IsFinalized).IsInitialized())
        {
        info.SetIsInitialized(true);
        partsToUpdate |= DataSourceInfo::PART_IsFinalized;
        }

    if (auto changedVars = m_variablesTracker->GetChangedVariables())
        {
        info.SetRelatedVariables(*changedVars);
        partsToUpdate |= DataSourceInfo::PART_Vars;
        }

    context.GetNodesCache().Update(info, partsToUpdate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCreatingMultiNavNodesProvider::CreateProvider(CreateNodeProviderContext& nodeProviderContext) const
    {
    NavNodeR node = nodeProviderContext.GetNode();
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("%s: Create provider for %s", GetName(), DiagnosticsHelpers::CreateNodeIdentifier(node).c_str()));

    auto nodeContext = CreateContextForSameHierarchyLevel(GetContext(), GetContext().GetRelatedVariablesIds(), true);

    NavNodeExtendedData extendedData(node);
    if (extendedData.IsNodeInitialized())
        {
        if (NodeVisibility::Virtual == GetContext().GetNodesCache().GetNodeVisibility(node.GetNodeId(), GetContext().GetRulesetVariables(), GetContext().GetInstanceFilter()))
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Node is already initialized and is virtual - returning children data provider for it.");
            return GetOrCreateChildrenProvider(nodeProviderContext);
            }

        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Node is already initialized and is not virtual - returning single node data provider for it.");
        return BVectorNodesProvider::Create(*nodeContext, {&node});
        }

    GetContext().GetHierarchyLevelLocker().Lock();
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Node is not initialized.");
    extendedData.SetNodeInitialized(true);

    // the specification may want to return node's children instead of the node itself
    if (ShouldReturnChildNodes(nodeProviderContext))
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Should return node's children - marking virtual and returning children data provider.");
        GetContext().GetNodesCache().MakeVirtual(node.GetNodeId(), GetContext().GetRulesetVariables(), GetContext().GetInstanceFilter(), GetContext().GetResultSetSizeLimit());
        auto childrenProviderContext = CreateContextForChildHierarchyLevel(GetContext(), node);
        childrenProviderContext->SetPhysicalParentNode(GetContext().GetNodesCache().GetPhysicalParentNode(node.GetNodeId(), GetContext().GetRulesetVariables(), GetContext().GetInstanceFilter()).get());
        return GetContext().CreateHierarchyLevelProvider(*childrenProviderContext, &node);
        }

    // otherwise, just use BVectorNodesProvider with single node
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Returning single node data provider.");
    return BVectorNodesProvider::Create(*nodeContext, {&node});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NavNodesProvider::PostProcess(bvector<IProvidedNodesPostProcessor const*> const& postProcessors)
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("%s: Post-process", GetName()));
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderCPtr NavNodesProvider::PostProcess(bvector<IProvidedNodesPostProcessor const*> const& postProcessors) const
    {
    return const_cast<NavNodesProvider&>(*this).PostProcess(postProcessors);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceIdentifier const& NavNodesProvider::_GetIdentifier() const
    {
    static DataSourceIdentifier const s_invalid;
    return s_invalid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProvider::SetTotalNodesCount(size_t count)
    {
    m_cachedNodesCount = count;
    _OnNodesCountSet(count);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void SetupProviderPageOptions(NavNodesProviderR provider, size_t& pageStart, Nullable<size_t>& pageSize)
    {
    // save initial 'pageStart' for this provider because 'pageStart' will be updated to page start of the next provider.
    size_t initialPageStart = pageStart;

    // if 'pageStart' is not zero current provider should skip nodes up to page start. Check if provider has enough nodes in order to skip to page start
    if (pageStart != 0)
        {
        MaxNodesToLoadContext maxNodesToLoad(provider, pageStart);
        // subtract provider's nodes count from current 'pageStart' in order to know 'pageStart' for the next provider
        // 'pageStart' will become zero if provider has enough nodes to skip to page start
        pageStart -= MIN(provider.GetTotalNodesCount().GetCount(), pageStart);
        }

    // set provider's pageOptions to not return any nodes:
    //  - if 'pageStart' is not zero. It means that provider does not have enough nodes to reach 'pageStart'
    //  - if 'pageSize' is zero. It means that no nodes are required from this provider as previous providers have enough nodes.
    if (pageStart != 0 || (pageSize.IsValid() && pageSize.Value() == 0))
        {
        // set page options with zero size in order to mark that this provider does not need to return any nodes.
        provider.SetPageOptions(std::make_shared<NavNodesProviderContext::PageOptions>(0, 0));
        return;
        }

    // if 'pageSize' is null all nodes from this provider should be returned
    if (pageSize.IsNull())
        {
        // if initial 'pageStart' for this provider is zero set provider's pageOptions to nullptr as it needs to return all nodes
        // otherwise set page options to return all nodes from initial 'pageStart'
        provider.SetPageOptions(initialPageStart == 0 ? nullptr : std::make_shared<NavNodesProviderContext::PageOptions>(initialPageStart));
        return;
        }

    // provider has more nodes than initial 'pageStart' or this provider does not need to skip any nodes
    // set pageOptions to return 'pageSize' of nodes from initial 'pageStart'
    provider.SetPageOptions(std::make_shared<NavNodesProviderContext::PageOptions>(initialPageStart, pageSize.Value()));

    // subtract provider's nodes count from 'pageSize' in order to know 'pageSize' for next provider.
    // 'pageSize' will become zero if current provider has requested amount of nodes
    pageSize.ValueR() -= MIN(provider.GetNodesCount(), pageSize.Value());
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PageOptionsSetter
{
private:
    NavNodesProviderContext::PageOptions const* m_pageOptions;
    size_t m_currPageStart;
    Nullable<size_t> m_currPageSize;

public:
    PageOptionsSetter(NavNodesProviderContext::PageOptions const* pageOptions, bool forceZeroPageStart)
        : m_pageOptions(pageOptions)
        {
        m_currPageStart = (pageOptions && !forceZeroPageStart) ? pageOptions->GetStart() : 0;
        m_currPageSize = pageOptions && pageOptions->HasSize() ? pageOptions->GetSize() : Nullable<size_t>();
        }
    void Accept(NavNodesProviderR provider)
        {
        if (m_pageOptions)
            SetupProviderPageOptions(provider, m_currPageStart, m_currPageSize);
        }
    bool HasEnoughNodes() const
        {
        return m_pageOptions && m_currPageSize.IsValid() && m_currPageSize.Value() == 0;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomNodesProvider::CustomNodesProvider(NavNodesProviderContextR context, CustomNodeSpecificationCR specification)
    : T_Super(context), m_specification(specification)
    {
    GetContextR().SetMayHaveArtifacts(HasNodeArtifactRules(GetContext().GetRulesPreprocessor(), specification));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodesProvider::_OnFirstCreate()
    {
    DataSourceInfo info(GetIdentifier());
    info.SetSpecificationHash(m_specification.GetHash());
    info.SetNodeTypes(m_specification.GetNodeType());
    GetContext().GetNodesCache().Update(info, DataSourceInfo::PART_NodeTypes | DataSourceInfo::PART_SpecificationHash);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<DirectNodesIterator> CustomNodesProvider::_CreateDirectNodesIterator() const
    {
    bvector<NavNodePtr> nodes;

    if (m_specification.GetNodeType().empty())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_ERROR, "Type is a required attribute for custom node specifications and is not set. Returning empty list.");
        return nullptr;
        }

    if (m_specification.GetLabel().empty())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_ERROR, "Label is a required attribute for custom node specifications and is not set. Returning empty list.");
        return nullptr;
        }

    Utf8String connectionId = GetContext().IsQueryContext() ? GetContext().GetConnection().GetId() : "";
    Utf8String type(m_specification.GetNodeType().c_str());
    Utf8String imageId(m_specification.GetImageId().c_str());
    Utf8String label(m_specification.GetLabel().c_str());
    Utf8String description(m_specification.GetDescription().c_str());
    NavNodeCPtr parent = GetContext().GetVirtualParentNode();
    NavNodePtr node = GetContext().GetNodesFactory().CreateCustomNode(GetContext().GetConnection(), m_specification.GetHash(), parent.IsValid() ? parent->GetKey().get() : nullptr,
        *LabelDefinition::Create(label.c_str()), description.c_str(), imageId.c_str(), type.c_str(), nullptr);
    NavNodeExtendedData extendedData(*node);
    extendedData.SetHideIfNoChildren(m_specification.GetHideIfNoChildren());
    extendedData.SetHideNodesInHierarchy(m_specification.GetHideNodesInHierarchy());
    extendedData.SetHideExpression(m_specification.GetHideExpression());
    if (m_specification.ShouldSuppressSimilarAncestorsCheck())
        extendedData.SetAllowedSimilarAncestors(MAX_ALLOWED_SIMILAR_ANCESTORS_WHEN_SUPPRESSED);
    if (ChildrenHint::Unknown != m_specification.GetHasChildren())
        extendedData.SetChildrenHint(m_specification.GetHasChildren());
    if (GetContext().GetVirtualParentNode().IsValid())
        extendedData.SetVirtualParentIds({GetContext().GetVirtualParentNode()->GetNodeId()});
    if (GetContext().IsRootNodeContext() && GetContext().GetRootNodeRule() && GetContext().GetRootNodeRule()->GetAutoExpand())
        node->SetShouldAutoExpand(true);
    IHierarchyCache::SavepointPtr savepoint = GetContext().GetNodesCache().CreateSavepoint();
    GetContext().GetNodesCache().Cache(*node, GetIdentifier(), 0, NodeVisibility::Visible);
    nodes.push_back(node);
    const_cast<CustomNodesProvider*>(this)->_OnDirectNodesRead(1);
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Created 1 direct node based on CustomNode specification");
    return std::make_unique<BVectorDirectNodesIterator>(nodes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBasedNodesProvider::SetQuery(PresentationQueryBuilderCP query, bmap<ECClassId, bool> const& usedClassIds)
    {
    m_query = query;
    m_usedClassIds = usedClassIds;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SpecificationFiltersBuilder : PresentationRuleSpecificationVisitor
{
private:
    NavNodesProviderContextCR m_context;
    PresentationQueryBuilderCR m_query;
    DataSourceFilter m_filter;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodeCPtr GetParentInstanceNode() const
        {
        NavNodeCPtr parentInstanceNode = m_context.GetVirtualParentNode();
        while (parentInstanceNode.IsValid() && nullptr == parentInstanceNode->GetKey()->AsECInstanceNodeKey())
            parentInstanceNode = m_context.GetNodesCache().GetPhysicalParentNode(parentInstanceNode->GetNodeId(), m_context.GetRulesetVariables(), m_context.GetInstanceFilter());
        return parentInstanceNode;
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _Visit(AllInstanceNodesSpecification const& specification) override {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _Visit(AllRelatedInstanceNodesSpecification const& specification) override
        {
        NavNodeCPtr parent = GetParentInstanceNode();
        if (parent.IsNull())
            return;

        bvector<ECClassId> usedRelationshipIds = ContainerHelpers::TransformContainer<bvector<ECClassId>>(
            m_query.GetNavigationResultParameters().GetUsedRelationshipClasses(), [](ECRelationshipClassCP rel){return rel->GetId();});
        m_filter = DataSourceFilter(std::make_unique<DataSourceFilter::RelatedInstanceInfo>(usedRelationshipIds,
            specification.GetRequiredRelationDirection(), parent->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys()), nullptr);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _Visit(CustomNodeSpecification const& specification) override {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _Visit(InstanceNodesOfSpecificClassesSpecification const& specification) override {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _Visit(RelatedInstanceNodesSpecification const& specification) override
        {
        NavNodeCPtr parent = GetParentInstanceNode();
        if (parent.IsNull())
            return;

        bvector<ECClassId> usedRelationshipIds = ContainerHelpers::TransformContainer<bvector<ECClassId>>(
            m_query.GetNavigationResultParameters().GetUsedRelationshipClasses(), [](ECRelationshipClassCP rel) {return rel->GetId(); });
        m_filter = DataSourceFilter(std::make_unique<DataSourceFilter::RelatedInstanceInfo>(usedRelationshipIds,
            specification.GetRequiredRelationDirection(), parent->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys()), nullptr);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _Visit(SearchResultInstanceNodesSpecification const& specification) override {}

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    SpecificationFiltersBuilder(NavNodesProviderContextCR context, PresentationQueryBuilderCR query)
        : m_context(context), m_query(query)
        {}
    DataSourceFilter GetFilter() const {return m_filter;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DataSourceFilter GetSpecificationFilter(NavNodesProviderContextCR context, PresentationQueryBuilderCR query)
    {
    auto const& params = query.GetNavigationResultParameters();
    if (nullptr == params.GetSpecification())
        return DataSourceFilter();

    SpecificationFiltersBuilder filtersBuilder(context, query);
    params.GetSpecification()->Accept(filtersBuilder);
    return filtersBuilder.GetFilter();
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryBasedNodesProvider::Savepoint
    {
    QueryBasedNodesProvider const& m_provider;
    IHierarchyCache::SavepointPtr m_cacheSavepoint;
    bool m_shouldCancel;
    Savepoint(QueryBasedNodesProvider const& provider)
        : m_provider(provider), m_cacheSavepoint(provider.GetContext().GetNodesCache().CreateSavepoint(true)), m_shouldCancel(true)
        {}
    ~Savepoint()
        {
        if (m_shouldCancel)
            {
            m_cacheSavepoint->Cancel();
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Cancelled hierarchy cache savepoint");
            }
        }
    void Commit() {m_shouldCancel = false;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP GetNodesTypeFromNavigationQuery(PresentationQueryBuilderCR query)
    {
    switch (query.GetNavigationResultParameters().GetResultType())
        {
        case NavigationQueryResultType::ECInstanceNodes:
        case NavigationQueryResultType::MultiECInstanceNodes:
            return NAVNODE_TYPE_ECInstancesNode;
        case NavigationQueryResultType::ClassGroupingNodes:
            return NAVNODE_TYPE_ECClassGroupingNode;
        case NavigationQueryResultType::DisplayLabelGroupingNodes:
            return NAVNODE_TYPE_DisplayLabelGroupingNode;
        case NavigationQueryResultType::PropertyGroupingNodes:
            return NAVNODE_TYPE_ECPropertyGroupingNode;
        }
    return "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8StringCR GetSpecificationHashFromNavigationQuery(PresentationQueryBuilderCR query)
    {
    static const Utf8String s_empty;
    ChildNodeSpecificationCP spec = query.GetNavigationResultParameters().GetSpecification();
    return spec ? spec->GetHash() : s_empty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool NodesCountMightChangeToZeroCheap(PresentationQueryBuilderCR query)
    {
    auto const& params = query.GetNavigationResultParameters();
    bool hasHideFlags = params.GetNavNodeExtendedData().HideIfNoChildren()
        || params.GetNavNodeExtendedData().HideNodesInHierarchy()
        || params.GetNavNodeExtendedData().HasHideExpression();
    return hasHideFlags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool NodesCountMightChangeToZeroExpensive(NavNodesProviderContextCR ctx, PresentationQueryBuilderCR query)
    {
    // we may also need to hide nodes if they're being created by the same specification as
    // one of the parent nodes
    Utf8StringCR specHash = GetSpecificationHashFromNavigationQuery(query);
    NavNodeCPtr currNode = ctx.GetVirtualParentNode();
    while (currNode.IsValid())
        {
        if (specHash.Equals(currNode->GetKey()->GetSpecificationIdentifier()) && currNode->GetType().Equals(GetNodesTypeFromNavigationQuery(query)))
            return true;

        currNode = ctx.GetNodesCache().GetPhysicalParentNode(currNode->GetNodeId(), ctx.GetRulesetVariables(), ctx.GetInstanceFilter());
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool NodesCountMightChangeToZero(NavNodesProviderContextCR ctx, PresentationQueryBuilderCR query)
    {
    return NodesCountMightChangeToZeroCheap(query) || NodesCountMightChangeToZeroExpensive(ctx, query);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool NodesCountMightChange(NavNodesProviderContextCR ctx, PresentationQueryBuilderCR query)
    {
    auto const& params = query.GetNavigationResultParameters();
    bool hasHideFlags = NodesCountMightChangeToZeroCheap(query)
        || params.GetNavNodeExtendedData().HideIfGroupingValueNotSpecified() && params.GetResultType() == NavigationQueryResultType::PropertyGroupingNodes
        || params.GetNavNodeExtendedData().HideIfOnlyOneChild();
    return hasHideFlags || NodesCountMightChangeToZeroExpensive(ctx, query);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<DirectNodesIterator> QueryBasedNodesProvider::_CreateDirectNodesIterator() const
    {
    bvector<NavNodePtr> nodes;
    NavNodeCPtr parent = GetContext().GetVirtualParentNode();
    Savepoint savepoint(*this);

    // set up the custom functions context
    SetupCustomFunctionsContext fnContext(GetContext(), m_query->GetExtendedData());

    // read all nodes in this provider
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Query for direct nodes: `%s`", m_query->GetQuery()->GetQueryString().c_str()));
    NavigationQueryContractsFilter contractsProvider(*m_query);
    auto nodesReader = NavNodesReader::Create(GetContext().GetNodesFactory(), GetContext().GetConnection(), contractsProvider,
        m_query->GetNavigationResultParameters().GetResultType(), m_query->GetNavigationResultParameters().GetNavNodeExtendedData(), parent.IsValid() ? parent->GetKey().get() : nullptr);
    QueryExecutor executor(GetContext().GetConnection(), *m_query->GetQuery());
    NavNodePtr node;
    while (QueryExecutorStatus::Row == executor.ReadNext(node, *nodesReader))
        {
        ThrowIfCancelled(GetContext().GetCancelationToken());

        NavNodeExtendedData extendedData(*node);
        if (GetContext().GetVirtualParentNode().IsValid())
            extendedData.SetVirtualParentIds({ GetContext().GetVirtualParentNode()->GetNodeId() });
        if (GetContext().IsRootNodeContext() && GetContext().GetRootNodeRule() && GetContext().GetRootNodeRule()->GetAutoExpand())
            node->SetShouldAutoExpand(true);

        GetContext().GetNodesCache().Cache(*node, GetIdentifier(), nodes.size() + m_offset, NodeVisibility::Visible);
        nodes.push_back(node);
        }

    // note: this callback must be called _before_ the cancellation check to make sure nodes count is in sync with nodes
    const_cast<QueryBasedNodesProvider*>(this)->_OnDirectNodesRead(nodes.size());
    savepoint.Commit();

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Created %" PRIu64 " direct nodes based on query based specification", (uint64_t)nodes.size()));
    return std::make_unique<BVectorDirectNodesIterator>(nodes);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryBasedNodesProvider::PageNodeCounts
    {
    size_t total;
    size_t unique;
    };
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryBasedNodesProvider::NodeCounts
    {
    size_t totalUnique;
    bvector<PageNodeCounts> pages;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus QueryBasedNodesProvider::InitializePartialProviders(bvector<PageNodeCounts> const& pageSizes)
    {
    auto scope = Diagnostics::Scope::Create("Initialize providers for paged queries");

    std::shared_ptr<PageOptions> pageOptions = GetContext().GetPageOptions();
    PageOptionsSetter pageOptionsSetter(pageOptions.get(), false);

    GetNodeProvidersR().clear();

    size_t offset = 0;
    for (auto const& pageCounts : pageSizes)
        {
        auto pageQuery = ComplexQueryBuilder::Create();
        pageQuery->SelectAll();
        pageQuery->From(*m_query->Clone());
        pageQuery->Limit(pageCounts.total, offset);

        NavNodesProviderContextPtr nestedContext = CreateContextForSameHierarchyLevel(GetContext(), GetContext().GetRelatedVariablesIds(), true);
        RefCountedPtr<QueryBasedNodesProvider> provider = QueryBasedNodesProvider::Create(*nestedContext, *pageQuery, m_usedClassIds, GetIdentifier());
        provider->SetTotalNodesCount(pageCounts.unique);
        AddProvider(*provider);
        pageOptionsSetter.Accept(*provider);

        offset += pageCounts.total;
        }
    return SUCCESS;
    }

size_t const QueryBasedNodesProvider::PartialProviderSize = 1000;
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QueryBasedNodesProvider::QueryBasedNodesProvider(NavNodesProviderContextR context, PresentationQueryBuilderCR query, bmap<ECClassId, bool> const& usedClassIds, DataSourceIdentifier parentDatasourceIdentifier)
    : T_Super(context), m_query(&query), m_usedClassIds(usedClassIds), m_offset(0), m_parentDatasourceIdentifier(parentDatasourceIdentifier)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NodesInitializationState QueryBasedNodesProvider::_InitializeNodes()
    {
    bool shouldInitializeAllNodes = false;

    DataSourceInfo datasourceInfo = GetContext().GetNodesCache().FindDataSource(
        GetIdentifier(),
        GetContext().GetRulesetVariables(),
        DataSourceInfo::PART_IsFinalized | DataSourceInfo::PART_HasPartialProviders
        );
    if (datasourceInfo.IsInitialized() && datasourceInfo.HasPartialProviders().IsValid())
        {
        // the data source is already initialized and the decision on whether we're splitting it into partials or not is already made
        shouldInitializeAllNodes = !datasourceInfo.HasPartialProviders().Value();
        }
    else
        {
        // check whether we want to initialize all nodes at once or split the provider into partial providers
        // note: the checks should be made in order of the cost from cheapest to most expensive
        shouldInitializeAllNodes = false
            // must initialize all if provider is asking for full load (e.g. to evaluate child node artifacts)
            || RequiresFullLoad()
            // initialize all if we don't have page options - requestor is asking for all nodes at once and getting them all at once is the most performance way to do it
            || !GetContext().GetPageOptions() || !GetContext().GetPageOptions()->HasSize() || 0 == GetContext().GetPageOptions()->GetSize()
            // must initialize all if we're going to hide any nodes
            || NodesCountMightChange(GetContext(), *m_query)
            // initialize all if we have less than `PageSize` nodes
            || GetTotalNodesCount().GetCount() <= PartialProviderSize;
        }

    if (shouldInitializeAllNodes)
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Initializing in general way");
        BeMutexHolder lock(GetContext().GetNodesCache().GetMutex());
        return T_Super::_InitializeNodes();
        }

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Initializing paged partial providers");
    InitializePartialProviders(QueryNodeCounts().pages);

    return NodesInitializationState({}, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBasedNodesProvider::_OnFirstCreate()
    {
    DataSourceInfo updateInfo(GetIdentifier(), RulesetVariables(), GetSpecificationFilter(GetContext(), *m_query),
        m_usedClassIds, GetSpecificationHashFromNavigationQuery(*m_query), GetNodesTypeFromNavigationQuery(*m_query));
    updateInfo.SetParentId(m_parentDatasourceIdentifier.GetId());

    // note: do not update related variables because query is not executed yet and related variables will be incorect
    GetContext().GetNodesCache().Update(updateInfo, DataSourceInfo::PARTS_All & ~DataSourceInfo::PART_Vars);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PageCountsAccumulator : IQueryResultAccumulator
{
private:
    bvector<QueryBasedNodesProvider::PageNodeCounts> m_pageCounts;
    Utf8String m_prevIdentifier;
    QueryBasedNodesProvider::PageNodeCounts m_curr;
    size_t m_totalUnique;
protected:
    QueryResultAccumulatorStatus _ReadRow(ECSqlStatementCR stmt) override
        {
        if (stmt.IsValueNull(0) || !m_prevIdentifier.Equals(stmt.GetValueText(0)))
            {
            if (m_curr.total >= QueryBasedNodesProvider::PartialProviderSize)
                {
                m_pageCounts.push_back(m_curr);
                m_curr = { 0, 0 };
                }
            ++m_totalUnique;
            ++m_curr.unique;
            m_prevIdentifier = stmt.GetValueText(0);
            }
        ++m_curr.total;
        return QueryResultAccumulatorStatus::Continue;
        }
    void _Complete(ECSqlStatementCR stmt, QueryResultAccumulatorStatus) override
        {
        if (m_curr.total != 0)
            m_pageCounts.push_back(m_curr);
        }
public:
    PageCountsAccumulator() : m_curr{ 0, 0 }, m_totalUnique(0) {}
    bvector<QueryBasedNodesProvider::PageNodeCounts> const& GetPageCounts() const {return m_pageCounts;}
    size_t GetTotalUniqueRecordsCount() const {return m_totalUnique;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<QueryBasedNodesProvider::PageNodeCounts> ParsePageNodeCounts(BeJsConst json)
    {
    bvector<QueryBasedNodesProvider::PageNodeCounts> counts;
    for (BeJsConst::ArrayIndex i = 0; i < json.size(); ++i)
        counts.push_back({ (size_t)json[i]["Total"].asUInt64(), (size_t)json[i]["Unique"].asUInt64() });
    return counts;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void SerializePageNodeCounts(BeJsValue json, bvector<QueryBasedNodesProvider::PageNodeCounts> const& counts)
    {
    for (auto const& entry : counts)
        {
        BeJsValue entryJson = json[json.size()];
        entryJson["Total"] = (int64_t)entry.total;
        entryJson["Unique"] = (int64_t)entry.unique;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QueryBasedNodesProvider::NodeCounts QueryBasedNodesProvider::QueryNodeCounts() const
    {
    auto scope = Diagnostics::Scope::Create("Query node counts");

    auto dsInfo = GetResultOrLockHierarchy<DataSourceInfo>(GetContext().GetHierarchyLevelLocker(),
        [&]()
        {
        DataSourceInfo info = GetContext().GetNodesCache().FindDataSource(
            GetIdentifier(),
            GetContext().GetRulesetVariables(),
            (int)DataSourceInfo::PART_CustomJson | (int)DataSourceInfo::PART_TotalNodesCount
            );
        bool isExpected = !info.GetCustomJson().isNull() && info.GetCustomJson().isObject() && info.GetCustomJson().hasMember("PageCounts");
        return make_bpair(isExpected, info);
        });

    if (!dsInfo.GetCustomJson().isNull() && dsInfo.GetCustomJson().isObject() && dsInfo.GetCustomJson().hasMember("PageCounts"))
        {
        // todo: log
        return NodeCounts{ dsInfo.GetTotalNodesCount().Value(), ParsePageNodeCounts(dsInfo.GetCustomJson()["PageCounts"]) };
        }

    // run a separate query to get the total nodes count (we already know this count
    // won't change during post-processing)
    SetupCustomFunctionsContext fnContext(GetContext(), m_query->GetExtendedData());

    RefCountedPtr<SimpleQueryContract> contract = SimpleQueryContract::Create();
    switch (m_query->GetNavigationResultParameters().GetResultType())
        {
        case NavigationQueryResultType::ECInstanceNodes:
            contract->AddField(*PresentationQueryContractSimpleField::Create("NodeIdentifier", ECInstanceNodesQueryContract::ECInstanceIdFieldName));
            break;
        case NavigationQueryResultType::MultiECInstanceNodes:
            contract->AddField(*PresentationQueryContractSimpleField::Create("NodeIdentifier", MultiECInstanceNodesQueryContract::InstanceKeysFieldName));
            break;
        default:
            contract->AddField(*PresentationQueryContractSimpleField::Create("NodeIdentifier", "NULL"));
        }
    auto countQuery = ComplexQueryBuilder::Create();
    countQuery->SelectContract(*contract);
    countQuery->From(*m_query->Clone());

    PageCountsAccumulator pageCountsAccumulator;
    QueryExecutorHelper::ExecuteQuery(GetContext().GetConnection(), *countQuery->GetQuery(), pageCountsAccumulator, GetContext().GetCancelationToken());
    NodeCounts counts{ pageCountsAccumulator.GetTotalUniqueRecordsCount(), pageCountsAccumulator.GetPageCounts() };

    dsInfo.SetTotalNodesCount(counts.totalUnique);
    dsInfo.SetHasNodes(counts.totalUnique > 0);
    BeJsValue json = dsInfo.GetCustomJson();
    if (!json.isNull() && !json.isObject())
        json.SetEmptyObject();
    SerializePageNodeCounts(json["PageCounts"], counts.pages);
    int partsToUpdate = DataSourceInfo::PART_TotalNodesCount | DataSourceInfo::PART_HasNodes | DataSourceInfo::PART_CustomJson;
    GetContext().GetNodesCache().Update(dsInfo, partsToUpdate);

    return counts;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CountInfo QueryBasedNodesProvider::_GetTotalNodesCount() const
    {
    if (RequiresFullLoad() || NodesCountMightChange(GetContext(), *m_query))
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Getting nodes count by loading nodes");
        return T_Super::_GetTotalNodesCount();
        }

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Getting nodes count using query");
    auto queryNodeCounts = QueryNodeCounts();
    return CountInfo(queryNodeCounts.totalUnique, !GetContext().GetCancelationToken().IsCanceled());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static std::unique_ptr<PresentationQuery> CreateSelect1Query(PresentationQueryBuilderCR navigationQuery)
    {
    RefCountedPtr<SimpleQueryContract> select1 = SimpleQueryContract::Create({ PresentationQueryContractSimpleField::Create("", "1", false) });

    auto queryWithoutOrdering = navigationQuery.Clone();
    QueryBuilderHelpers::RemoveOrdering(*queryWithoutOrdering);

    auto genericQuery = ComplexQueryBuilder::Create();
    genericQuery->SelectContract(*select1);
    genericQuery->From(*queryWithoutOrdering);
    return genericQuery->CreateQuery();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static std::unique_ptr<PresentationQuery> CreateRowsCheckQuery(PresentationQueryBuilderCR source)
    {
    if (source.AsUnionQueryBuilder())
        {
        return UnionQueryBuilder::Create(ContainerHelpers::TransformContainer<bvector<PresentationQueryBuilderPtr>>(source.AsUnionQueryBuilder()->GetQueries(), [](auto const& nestedQuery)
            {
            return StringQueryBuilder::Create(*CreateRowsCheckQuery(*nestedQuery));
            }))->CreateQuery();
        }
    if (source.AsExceptQueryBuilder())
        {
        return ExceptQueryBuilder::Create(
            *StringQueryBuilder::Create(*CreateRowsCheckQuery(*source.AsExceptQueryBuilder()->GetBase())),
            *StringQueryBuilder::Create(*CreateRowsCheckQuery(*source.AsExceptQueryBuilder()->GetExcept()))
            )->CreateQuery();
        }

    if (source.AsStringQueryBuilder())
        {
        return CreateSelect1Query(source);
        }

    if (source.AsComplexQueryBuilder())
        {
        auto const& complexSource = *source.AsComplexQueryBuilder();
        auto contract = complexSource.GetContract();
        auto resultType = source.GetNavigationResultParameters().GetResultType();

        if (resultType == NavigationQueryResultType::ClassGroupingNodes && dynamic_cast<ECClassGroupingNodesQueryContract const*>(contract))
            return CreateSelect1Query(static_cast<ECClassGroupingNodesQueryContract const*>(contract)->GetInstanceKeysSelectQuery());

        if (resultType == NavigationQueryResultType::DisplayLabelGroupingNodes && dynamic_cast<DisplayLabelGroupingNodesQueryContract const*>(contract))
            return CreateSelect1Query(static_cast<DisplayLabelGroupingNodesQueryContract const*>(contract)->GetInstanceKeysSelectQuery());

        if (resultType == NavigationQueryResultType::PropertyGroupingNodes && dynamic_cast<ECPropertyGroupingNodesQueryContract const*>(contract))
            return CreateSelect1Query(static_cast<ECPropertyGroupingNodesQueryContract const*>(contract)->GetInstanceKeysSelectQuery());

        return CreateSelect1Query(source);
        }

    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Invalid query type");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t QueryHasNodes(NavNodesProviderContextCR context, NavNodeCP virtualParent, PresentationQueryBuilderCR query, DataSourceIdentifier const& dsIdentifier)
    {
    auto scope = Diagnostics::Scope::Create("Query has nodes");

    // run a separate query to check if there are any nodes
    SetupCustomFunctionsContext fnContext(context, query.GetExtendedData());

    auto rowsCheckQuery = CreateRowsCheckQuery(query);
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Rows check query: `%s`", rowsCheckQuery->GetQueryString().c_str()));

    int value;
    static GenericQueryResultReader<int> s_emptyReader([](ECSqlStatementCR){return 0;});
    QueryExecutor executor(context.GetConnection(), *rowsCheckQuery);
    return QueryExecutorStatus::Row == executor.ReadNext<int>(value, s_emptyReader);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryBasedNodesProvider::_HasNodes() const
    {
    if (RequiresFullLoad() || NodesCountMightChangeToZero(GetContext(), *m_query))
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Checking if there are nodes by loading them");
        return T_Super::_HasNodes();
        }

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Checking if there are nodes using count query");
    return QueryHasNodes(GetContext(), GetContext().GetVirtualParentNode().get(), *m_query, GetIdentifier()) > 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unordered_set<ECClassCP> QueryBasedNodesProvider::_GetResultInstanceNodesClasses() const
    {
    return m_query->GetNavigationResultParameters().GetSelectInstanceClasses();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t QueryBasedNodesProvider::_GetLimitedInstancesCount(size_t limit) const
    {
    auto scope = Diagnostics::Scope::Create("Get limited instances count");

    // attempt to find the count in cache
    auto cachedCount = GetResultOrLockHierarchy<Nullable<uint64_t>>(GetContext().GetHierarchyLevelLocker(),
        [&]()
        {
        DataSourceInfo info = GetContext().GetNodesCache().FindDataSource(
            GetIdentifier(),
            GetContext().GetRulesetVariables(),
            DataSourceInfo::PART_LimitedInstancesCount
        );
        return make_bpair(info.GetLimitedInstancesCount().IsValid(), info.GetLimitedInstancesCount());
        });
    if (cachedCount.IsValid())
        return (size_t)*cachedCount;

    // when hiding nodes produced by this query, we want to count the ones we're going to actually show
    if (m_query->GetNavigationResultParameters().GetNavNodeExtendedData().HideNodesInHierarchy())
        return T_Super::_GetLimitedInstancesCount(limit);

    // run the limited query to count resulting instances
    auto instanceKeysQuery = QueryBuilderHelpers::GetInstanceKeysQuery(*m_query);
    QueryBuilderHelpers::RemoveOrdering(*instanceKeysQuery);
    QueryBuilderHelpers::Limit(*instanceKeysQuery, limit);

    auto countQuery = ComplexQueryBuilder::Create();
    countQuery->SelectContract(*SimpleQueryContract::Create({
        PresentationQueryContractSimpleField::Create("Count", "COUNT(1)", false),
        }));
    countQuery->From(*instanceKeysQuery);

    SetupCustomFunctionsContext fnContext(GetContext(), countQuery->GetExtendedData());
    auto count = QueryExecutorHelper::ReadUInt64(GetContext().GetConnection(), *countQuery->GetQuery());

    DataSourceInfo dsInfo(GetIdentifier());
    dsInfo.SetLimitedInstancesCount(count);
    GetContext().GetNodesCache().Update(dsInfo, DataSourceInfo::PART_LimitedInstancesCount);

    return (size_t)count;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SpecificationUsedClassesListener : IUsedClassesListener
    {
    IUsedClassesListener* m_baseListener;
    bmap<ECClassId, bool> m_usedClassIds;
    SpecificationUsedClassesListener(IUsedClassesListener* baseListener)
        : m_baseListener(baseListener)
        {}
    bmap<ECClassId, bool> const& GetUsedClassIds() const {return m_usedClassIds;}
    void _OnClassUsed(ECClassCR ecClass, bool polymorphically) override
        {
        m_usedClassIds[ecClass.GetId()] = polymorphically;
        if (nullptr != m_baseListener)
            m_baseListener->_OnClassUsed(ecClass, polymorphically);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QueryBasedSpecificationNodesProvider::QueryBasedSpecificationNodesProvider(NavNodesProviderContextR context, ChildNodeSpecificationCR specification)
    : T_Super(context), m_specification(specification)
    {
    GetContextR().SetMayHaveArtifacts(HasNodeArtifactRules(GetContext().GetRulesPreprocessor(), specification));

    SpecificationUsedClassesListener usedClasses(GetContext().GetUsedClassesListener());
    auto queryBuilder = CreateQueryBuilder(usedClasses);
    auto queries = CreateQueries(*queryBuilder);

    bset<Utf8String> usedVariables = GetContext().GetRelatedVariablesIds();

    for (auto const& query : queries.GetQueries())
        {
        NavNodesProviderContextPtr nestedContext = CreateContextForSameHierarchyLevel(GetContext(), usedVariables, true);
        AddProvider(*QueryBasedNodesProvider::Create(*nestedContext, *query, usedClasses.GetUsedClassIds()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<NavigationQueryBuilder> QueryBasedSpecificationNodesProvider::CreateQueryBuilder(IUsedClassesListener& usedClassesListener) const
    {
    auto scope = Diagnostics::Scope::Create("Create query builder");
    NavigationQueryBuilderParameters params(
        GetContext().GetSchemaHelper(), GetContext().GetConnections(), GetContext().GetConnection(), &GetContext().GetCancelationToken(),
        GetContext().GetRulesPreprocessor(), GetContext().GetRuleset(), GetContext().GetRulesetVariables(), &GetContext().GetUsedVariablesListener(),
        GetContext().GetECExpressionsCache(), GetContext().GetNodesCache(), GetContext().GetLocalState());
    params.SetUsedClassesListener(&usedClassesListener);
    if (!m_specification.GetHideNodesInHierarchy())
        params.SetInstanceFilter(GetContext().GetInstanceFilter());
    return std::make_unique<NavigationQueryBuilder>(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QuerySet QueryBasedSpecificationNodesProvider::CreateQueries(NavigationQueryBuilderCR queryBuilder) const
    {
    auto scope = Diagnostics::Scope::Create("Create queries");

    if (!GetContext().IsQueryContext())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Attempting to create queries in non-query context");

    if (GetContext().IsRootNodeContext() && GetContext().GetRootNodeRule())
        {
        if (nullptr == GetContext().GetRootNodeRule())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Got root node context without a root node rule");

        return queryBuilder.GetQueries(*GetContext().GetRootNodeRule(), m_specification);
        }

    if (GetContext().IsChildNodeContext())
        {
        if (nullptr == GetContext().GetChildNodeRule())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Got child node context without a child node rule");

        NavNodeCPtr parent = GetContext().GetVirtualParentNode();
        if (parent.IsNull())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Got child node context without a parent node");

        return queryBuilder.GetQueries(*GetContext().GetChildNodeRule(), m_specification, *parent);
        }

    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Attempting to create queries for provider which has neither root nor child nodes context");
    }

/*=================================================================================**//**
* @bsiclass
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
            // FIXME: use relationship paths
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryBasedSpecificationNodesProvider::_HasNodes() const
    {
    if (!RequiresFullLoad())
        {
        SpecificationChildrenChecker visitor(GetContext());
        m_specification.Accept(visitor);
        if (visitor.AlwaysHasChildren())
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "The provider always returns nodes due to requirements of relationship used in specification");
            return true;
            }
        }

    return T_Super::_HasNodes();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MultiSpecificationNodesProvider::MultiSpecificationNodesProvider(NavNodesProviderContextR baseContext, RootNodeRuleSpecificationsList const& specs)
    : T_Super(baseContext)
    {
    baseContext.SetVirtualParentNode(nullptr);
    bset<Utf8String> usedVariables = baseContext.GetRelatedVariablesIds();

    SpecificationsVisitor visitor;
    for (RootNodeRuleSpecification const& specification : specs)
        {
        NavNodesProviderContextPtr nestedContext = CreateContextForSameHierarchyLevel(GetContext(), usedVariables, false);
        nestedContext->SetRootNodeContext(&specification.GetRule());
        visitor.SetContext(*nestedContext);
        specification.GetSpecification().Accept(visitor);
        }
    SetProviders(visitor.GetNodeProviders());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MultiSpecificationNodesProvider::MultiSpecificationNodesProvider(NavNodesProviderContextR baseContext, ChildNodeRuleSpecificationsList const& specs, NavNodeCR virtualParent)
    : T_Super(baseContext)
    {
    baseContext.SetVirtualParentNode(&virtualParent);
    bset<Utf8String> usedVariables = baseContext.GetRelatedVariablesIds();

    SpecificationsVisitor visitor;
    for (ChildNodeRuleSpecification const& specification : specs)
        {
        NavNodesProviderContextPtr nestedContext = CreateContextForSameHierarchyLevel(GetContext(), usedVariables, false);
        nestedContext->SetChildNodeContext(&specification.GetRule(), virtualParent);
        visitor.SetContext(*nestedContext);
        specification.GetSpecification().Accept(visitor);
        }
    SetProviders(visitor.GetNodeProviders());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<DirectNodesIterator> NodesCreatingMultiNavNodesProvider::GetDirectNodesIterator() const
    {
    auto loggingScope = Diagnostics::Scope::Create("[NodesCreatingMultiNavNodesProvider] Get direct nodes");

    auto cachedDirectNodesIterator = GetResultOrLockHierarchy<std::unique_ptr<DirectNodesIterator>>(GetContext().GetHierarchyLevelLocker(), [&]() -> bpair<bool, std::unique_ptr<DirectNodesIterator>>
        {
        // cached direct nodes count suggests that direct nodes for this data source have been cached
        Nullable<uint64_t> count = GetContext().GetNodesCache().FindDataSource(
            GetIdentifier(),
            GetContext().GetRulesetVariables(),
            DataSourceInfo::PART_DirectNodesCount
            ).GetDirectNodesCount();
        if (count.IsNull())
            return make_bpair(false, nullptr);

        auto iterator = GetContext().GetNodesCache().GetCachedDirectNodesIterator(GetContext(), GetIdentifier());
        bool hasIterator = (iterator != nullptr);
        return std::make_pair(hasIterator, std::move(iterator));
        });
    if (cachedDirectNodesIterator)
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Direct nodes found in cache");
        return cachedDirectNodesIterator;
        }

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Direct nodes not found in cache, creating");
    return _CreateDirectNodesIterator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCreatingMultiNavNodesProvider::_OnPageOptionsSet()
    {
    // note: we intentionally don't call direct base class, because we only want to reset the 'is initialized'
    // flag without setting page options on nested providers
    NavNodesProvider::_OnPageOptionsSet();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCreatingMultiNavNodesProvider::_ResetInitializedNodes()
    {
    GetNodeProvidersR().clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NodesInitializationState NodesCreatingMultiNavNodesProvider::_InitializeNodes()
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("%s: Initialize nodes", GetName()));

    std::unique_ptr<DirectNodesIterator> nodesIterator = GetDirectNodesIterator();
    if (nullptr == nodesIterator)
        return SUCCESS;

    bool shouldReinitializeFromStart = nodesIterator->SkippedNodesToPageStart();
    std::shared_ptr<PageOptions> pageOptions = GetContext().GetPageOptions();
    PageOptionsSetter pageOptionsSetter(pageOptions.get(), shouldReinitializeFromStart);

    // if direct nodes iterator has skipped nodes up to page start clear already existing providers and create new ones
    if (shouldReinitializeFromStart)
        GetNodeProvidersR().clear();

    // create providers for each node
    size_t handledNodesCount = 0;
    size_t loadedNodesCount = 0;
    size_t providerIndex = 0;
    NavNodePtr node;
    GetContext().GetHierarchyLevelLocker().WaitForUnlock();
    while ((node = nodesIterator->NextNode()) != nullptr)
        {
        ThrowIfCancelled(GetContext().GetCancelationToken());

        if (pageOptionsSetter.HasEnoughNodes())
            {
            // ensure we don't have too many providers created from previous runs
            GetNodeProvidersR().erase(GetNodeProvidersR().begin() + providerIndex, GetNodeProvidersR().end());
            break;
            }

        ++handledNodesCount;

        CreateNodeProviderContext nodeProviderContext(*node, GetContext());
        if (!NavNodeExtendedData(*node).IsNodeInitialized())
            {
            GetContext().GetHierarchyLevelLocker().Lock();
            EvaluateChildrenArtifacts(nodeProviderContext);
            EvaluateThisNodeArtifacts(*node);
            }

        // look for existing provider or create new one
        NavNodesProviderPtr provider;
        if (GetNodeProviders().size() > providerIndex)
            provider = GetNodeProviders().at(providerIndex);
        if (provider.IsNull())
            {
            provider = CreateProvider(nodeProviderContext);
            GetNodeProvidersR().push_back(provider);
            }
        pageOptionsSetter.Accept(*provider);

        providerIndex++;

        auto optimizations = GetContext().GetMergedOptimizationFlags();
        if (!RequiresFullLoad() && optimizations.GetMaxNodesToLoad() != 0)
            {
            // count nodes if optimization flags has max nodes count set
            MaxNodesToLoadContext maxNodesToLoad(*provider, optimizations.GetMaxNodesToLoad());
            loadedNodesCount += provider->GetNodesCount();
            if (optimizations.GetMaxNodesToLoad() <= loadedNodesCount)
                {
                // found what we're looking for - no need to handle other nodes
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Found provider with nodes while looking for children. Break at %" PRIu64 " after handling %" PRIu64 " / %" PRIu64 " nodes.",
                    (uint64_t)providerIndex, (uint64_t)handledNodesCount, (uint64_t)nodesIterator->NodesCount()));
                break;
                }
            }
        }

    return NodesInitializationState({ make_bpair(pageOptions ? pageOptions->GetStart() : 0, handledNodesCount) }, nodesIterator->NodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CountInfo NodesCreatingMultiNavNodesProvider::_GetTotalNodesCount() const
    {
    const_cast<NodesCreatingMultiNavNodesProvider*>(this)->InitializeNodes();
    auto result = T_Super::_GetTotalNodesCount();
    return CountInfo(result.GetCount(), result.IsAccurate() && IsInitialized() && !GetContext().HasPageOptions());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCreatingMultiNavNodesProvider::_HasNodes() const
    {
    const_cast<NodesCreatingMultiNavNodesProvider*>(this)->InitializeNodes();
    return T_Super::_HasNodes();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCreatingMultiNavNodesProvider::_InitializeDataSources()
    {
    // requesting count of all nodes makes sure that all data source identifiers are cached
    GetTotalNodesCount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MultiNavNodesProvider::_OnPageOptionsSet()
    {
    T_Super::_OnPageOptionsSet();

    auto pageOptions = GetContext().GetPageOptions();
    if (nullptr == pageOptions)
        {
        for (auto const& provider : m_providers)
            provider->SetPageOptions(nullptr);
        return;
        }

    size_t currPageStart = pageOptions->GetStart();
    Nullable<size_t> currPageSize = pageOptions->HasSize() ? pageOptions->GetSize() : Nullable<size_t>();
    for (auto const& provider : m_providers)
        SetupProviderPageOptions(*provider, currPageStart, currPageSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MultiNavNodesProvider::_InitializeDataSources()
    {
    for (auto const& provider : m_providers)
        provider->InitializeDataSources();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariables MultiNavNodesProvider::_GetRelatedRulesetVariables() const
    {
    RulesetVariables variables = T_Super::_GetRelatedRulesetVariables();
    for (auto const& provider : m_providers)
        variables.Merge(provider->GetRelatedRulesetVariables());
    return variables;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultiNavNodesProvider::_RequiresFullLoad() const
    {
    if (T_Super::_RequiresFullLoad())
        return true;

    for (auto const& provider : m_providers)
        {
        if (provider.IsValid() && provider->RequiresFullLoad())
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultiNavNodesProvider::_HasNodes() const
    {
    bool requiresFullLoad = RequiresFullLoad();
    bool hasNodes = false;
    for (auto const& provider : m_providers)
        {
        ThrowIfCancelled(GetContext().GetCancelationToken());

        if (provider.IsNull())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Detected NULL nested provider");

        hasNodes |= provider->HasNodes();
        if (hasNodes && !requiresFullLoad)
            return true;
        }
    return hasNodes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CountInfo MultiNavNodesProvider::_GetTotalNodesCount() const
    {
    bool requiresFullLoad = RequiresFullLoad();
    bool isAccurate = true;
    size_t count = 0;
    for (size_t i = 0; i < m_providers.size(); ++i)
        {
        ThrowIfCancelled(GetContext().GetCancelationToken());

        auto const& provider = m_providers[i];
        if (provider.IsNull())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Detected NULL nested provider");

        auto nestedCount = provider->GetTotalNodesCount();
        isAccurate &= nestedCount.IsAccurate();
        count += nestedCount.GetCount();

        auto optimizations = GetContext().GetMergedOptimizationFlags();
        if (!requiresFullLoad && optimizations.GetMaxNodesToLoad() != 0 && optimizations.GetMaxNodesToLoad() <= count)
            return CountInfo(count, isAccurate && i == (m_providers.size() - 1));
        }
    return CountInfo(count, isAccurate);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NestedProvidersBasedIteratorImpl : IteratorImpl<NavNodePtr>
{
private:
    bvector<NavNodesProviderPtr> const& m_providers;
    size_t m_providerIndex;
    NavNodesProvider::Iterator m_currNodesIterator;
    size_t m_currNodePosInProvider;
private:
    NavNodesProvider::Iterator CreateNodesIterator()
        {
        NavNodesProvider::Iterator iter;
        while (!iter.IsValid() && m_providerIndex < m_providers.size())
            {
            iter = m_providers[m_providerIndex]->begin();
            if (iter == m_providers[m_providerIndex]->end())
                {
                iter = NavNodesProvider::Iterator();
                ++m_providerIndex;
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
        while (m_currNodesIterator.IsValid() && m_currNodesIterator == m_providers[m_providerIndex]->end())
            {
            if (++m_providerIndex < m_providers.size())
                {
                // TODO: this causes an iterator to be created possibly unnecessarily (e.g. when creating end() iterator) and creating
                // an iterator might cause nodes to be loaded. Need to delay creating the iterator until it's actually being used.
                m_currNodesIterator = m_providers[m_providerIndex]->begin();
                }
            else
                m_currNodesIterator = NavNodesProvider::Iterator();
            m_currNodePosInProvider = 0;
            }
        }
protected:
    std::unique_ptr<IteratorImpl<NavNodePtr>> _Copy() const override {return std::make_unique<NestedProvidersBasedIteratorImpl>(m_providers, m_providerIndex, m_currNodesIterator, m_currNodePosInProvider);}
    bool _Equals(IteratorImpl<NavNodePtr> const& otherBase) const override
        {
        NestedProvidersBasedIteratorImpl const& other = static_cast<NestedProvidersBasedIteratorImpl const&>(otherBase);
        return m_providerIndex == other.m_providerIndex && m_currNodesIterator == other.m_currNodesIterator;
        }
    void _Next(size_t count) override
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Advance nested providers iterator by %" PRIu64, (uint64_t)count));

        if (0 == count)
            return;

        if (m_providers.empty() || m_providerIndex >= m_providers.size())
            return;

        if (1 == count)
            {
            // use quicker approach for majority of cases (no need to get counts)
            NextOne();
            return;
            }

        // calculate the number of nodes we need to skip past the end for current provider
        size_t nodesCountInCurrentProvider = m_providers[m_providerIndex]->GetNodesCount();
        size_t nodesLeftInCurrentProvider = nodesCountInCurrentProvider - m_currNodePosInProvider - 1;
        if (count > nodesLeftInCurrentProvider)
            {
            // skip at the level of providers
            count -= nodesLeftInCurrentProvider + 1;
            ++m_providerIndex;
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Advancing outside of current nested provider. Nodes left: %" PRIu64, (uint64_t)count));
            while (m_providerIndex < m_providers.size() && m_providers[m_providerIndex]->GetNodesCount() <= count)
                {
                count -= m_providers[m_providerIndex]->GetNodesCount();
                ++m_providerIndex;
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Advancing outside of current nested provider. Nodes left: %" PRIu64, (uint64_t)count));
                }
            if (m_providerIndex >= m_providers.size())
                {
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Passed through all providers and reached the end");
                m_currNodesIterator = NavNodesProvider::Iterator();
                m_currNodePosInProvider = 0;
                return;
                }
            }

        // m_providerIterator now points to the provider we need to step into
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Advancing %" PRIu64 " inside current nested provider", (uint64_t)count));
        m_currNodesIterator = (m_providers[m_providerIndex]->begin() += count);
        m_currNodePosInProvider = count;
        }
    NavNodePtr _GetCurrent() const override
        {
        NavNodePtr node = *m_currNodesIterator;
        return node;
        }
public:
    NestedProvidersBasedIteratorImpl(bvector<NavNodesProviderPtr> const& providers)
        : m_providers(providers), m_providerIndex(0), m_currNodePosInProvider(0)
        {
        m_currNodesIterator = CreateNodesIterator();
        }
    NestedProvidersBasedIteratorImpl(bvector<NavNodesProviderPtr> const& providers, size_t providerIndex)
        : m_providers(providers), m_providerIndex(providerIndex), m_currNodePosInProvider(0)
        {}
    NestedProvidersBasedIteratorImpl(bvector<NavNodesProviderPtr> const& providers, size_t providerIndex, NavNodesProvider::Iterator currNodesIterator, size_t currNodePosInProvider)
        : m_providers(providers), m_providerIndex(providerIndex), m_currNodesIterator(currNodesIterator), m_currNodePosInProvider(currNodePosInProvider)
        {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProvider::Iterator MultiNavNodesProvider::_CreateFrontIterator() const {return Iterator(std::make_unique<NestedProvidersBasedIteratorImpl>(m_providers));}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProvider::Iterator MultiNavNodesProvider::_CreateBackIterator() const {return Iterator(std::make_unique<NestedProvidersBasedIteratorImpl>(m_providers, m_providers.size()));}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr MultiNavNodesProvider::_FindNestedProvider(DataSourceIdentifier const& identifier)
    {
    InitializeNodes();
    for (auto const& provider : m_providers)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unordered_set<ECClassCP> MultiNavNodesProvider::_GetResultInstanceNodesClasses() const
    {
    std::unordered_set<ECClassCP> classes;
    for (auto const& provider : m_providers)
        ContainerHelpers::Push(classes, provider->GetResultInstanceNodesClasses());
    return classes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MultiNavNodesProvider::_GetLimitedInstancesCount(size_t limit) const
    {
    // need to make sure `m_providers` is initialized
        {
        MaxNodesToLoadContext limitLoadedNodesCount(*this, limit);
        const_cast<MultiNavNodesProvider*>(this)->InitializeNodes();
        }

    size_t count = 0;
    for (auto const& provider : m_providers)
        {
        if (count >= limit)
            return count;

        count += provider->GetLimitedInstancesCount(limit - count);
        }
    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static NavNodesProvider::Iterator CreateNodesVectorFrontIterator(bvector<NavNodePtr> const& nodes, NavNodesProviderContext::PageOptions* pageOptions)
    {
    size_t offset = 0;
    if (nullptr != pageOptions)
        offset = (pageOptions->GetStart() < nodes.size()) ? pageOptions->GetStart() : nodes.size();
    auto iter = nodes.begin() + offset;
    return NavNodesProvider::Iterator(std::make_unique<IterableIteratorImpl<bvector<NavNodePtr>::const_iterator, NavNodePtr>>(iter));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static NavNodesProvider::Iterator CreateNodesVectorBackIterator(bvector<NavNodePtr> const& nodes, NavNodesProviderContext::PageOptions* pageOptions)
    {
    size_t offset = nodes.size();
    if (nullptr != pageOptions && pageOptions->HasSize())
        offset = (pageOptions->GetStart() + pageOptions->GetSize()) < nodes.size() ? pageOptions->GetStart() + pageOptions->GetSize() : nodes.size();
    auto iter = nodes.begin() + offset;
    return NavNodesProvider::Iterator(std::make_unique<IterableIteratorImpl<bvector<NavNodePtr>::const_iterator, NavNodePtr>>(iter));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProvider::Iterator BVectorNodesProvider::_CreateFrontIterator() const
    {
    return CreateNodesVectorFrontIterator(m_nodes, GetContext().GetPageOptions().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProvider::Iterator BVectorNodesProvider::_CreateBackIterator() const
    {
    return CreateNodesVectorBackIterator(m_nodes, GetContext().GetPageOptions().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t BVectorNodesProvider::_GetLimitedInstancesCount(size_t limit) const
    {
    size_t count = 0;
    for (auto const& node : m_nodes)
        {
        if (count >= limit)
            return count;

        if (auto instanceKey = node->GetKey()->AsECInstanceNodeKey())
            count += instanceKey->GetInstanceKeys().size();
        else if (auto groupingKey = node->GetKey()->AsGroupingNodeKey())
            count += (size_t)groupingKey->GetGroupedInstancesCount();
        }
    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr DisplayLabelGroupingNodesPostProcessor::_PostProcessProvider(NavNodesProviderR processedProvider) const
    {
    auto scope = Diagnostics::Scope::Create("Display label grouping nodes post-processor: Post-process");
    DisabledFullNodesLoadContext disableFullLoad(processedProvider);

    auto const& context = processedProvider.GetContext();
    context.GetHierarchyLevelLocker().WaitForUnlock();
    {
    MaxNodesToLoadContext maxNodesToLoad(processedProvider, 2);
    if (processedProvider.GetNodesCount() > 1)
        {
        // do nothing if there are more than 1 node
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Provider has more than 1 node - nothing to do.");
        return nullptr;
        }
    }

    auto iter = processedProvider.begin();
    if (processedProvider.end() == iter || !(*iter)->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode))
        {
        // nothing to do if only node in provider is not label grouping
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "First node is not a label grouping node - nothing to do.");
        return nullptr;
        }

    context.GetHierarchyLevelLocker().Lock();
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Provider has 1 label grouping node without siblings - hide node and show its children.");

    // make display label grouping node virtual
    NavNodeCPtr node = *iter;
    processedProvider.GetContextR().GetNodesCache().MakeVirtual(node->GetNodeId(), context.GetRulesetVariables(), context.GetInstanceFilter(), context.GetResultSetSizeLimit());

    // return children of the grouping node
    auto childrenContext = CreateContextForChildHierarchyLevel(context, *node);
    childrenContext->SetAncestorContext(context.GetAncestorContext());
    return context.CreateHierarchyLevelProvider(*childrenContext, (*iter).get());
    }

#ifdef wip_enable_display_label_postprocessor
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool CompareNodePtrsByLabel(NavNodePtr const& lhs, NavNodePtr const& rhs)
    {
    Utf8String paddedLhsLabel = ValueHelpers::PadNumbersInString(lhs->GetLabelDefinition().GetDisplayValue());
    Utf8String paddedRhsLabel = ValueHelpers::PadNumbersInString(rhs->GetLabelDefinition().GetDisplayValue());
    return paddedLhsLabel.CompareTo(paddedRhsLabel) < 0;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DelayedLabelSortingNodesProvider : MultiNavNodesProvider
{
    DEFINE_T_SUPER(MultiNavNodesProvider)

private:
    bvector<NavNodesProviderPtr> m_dataSources;
    mutable bvector<NavNodePtr> m_sortedNodes;

private:
    DelayedLabelSortingNodesProvider(NavNodesProviderContextR context, bvector<NavNodesProviderPtr> providers)
        : MultiNavNodesProvider(context), m_dataSources(providers)
        {
        SetProviders(providers);
        }

    void InitSortedNodes() const
        {
        auto scope = Diagnostics::Scope::Create("Sorting nodes");
        if (!m_sortedNodes.empty())
            return;

        for (auto const& provider : m_dataSources)
            {
            DisabledFullNodesLoadContext disableFullLoad(*provider);
            size_t sizeBefore = m_sortedNodes.size();
            for (auto node : *provider)
                {
                NOT_NULL_PRECONDITION(node, "DelayedLabelSortingNodesProvider::InitSortedNodes");
                m_sortedNodes.push_back(node);
                }
            if (0 != sizeBefore)
                std::inplace_merge(m_sortedNodes.begin(), m_sortedNodes.begin() + sizeBefore, m_sortedNodes.end(), CompareNodePtrsByLabel);
            }
        }

protected:
    Utf8CP _GetName() const override { return "Delayed label sorting nodes provider"; }

    Iterator _CreateFrontIterator() const override
        {
        if (GetContext().GetMergedOptimizationFlags().IsPostProcessingDisabled())
            return T_Super::_CreateFrontIterator();

        InitSortedNodes();
        return CreateNodesVectorFrontIterator(m_sortedNodes, GetContext().GetPageOptions().get());
        }

    Iterator _CreateBackIterator() const override
        {
        if (GetContext().GetMergedOptimizationFlags().IsPostProcessingDisabled())
            return T_Super::_CreateBackIterator();

        InitSortedNodes();
        return CreateNodesVectorBackIterator(m_sortedNodes, GetContext().GetPageOptions().get());
        }

public:
    static RefCountedPtr<DelayedLabelSortingNodesProvider> Create(NavNodesProviderContextR context, bvector<NavNodesProviderPtr> providers)
        {
        return new DelayedLabelSortingNodesProvider(context, providers);
        }
};

/*=================================================================================**//**
* Only needed to hold the source provider.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MergedNavNodesProvider : MultiNavNodesProvider
{
private:
    NavNodesProviderCPtr m_source;

private:
    MergedNavNodesProvider(NavNodesProviderContextR context, NavNodesProviderCR source)
        : MultiNavNodesProvider(context), m_source(&source)
        {}
protected:
    Utf8CP _GetName() const override {return "Merged nodes provider";}
public:
    static RefCountedPtr<MergedNavNodesProvider> Create(NavNodesProviderContextR context, NavNodesProviderCR source)
        {
        return new MergedNavNodesProvider(context, source);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsDataSourceLabelSorted(IRulesPreprocessorR rules, Utf8StringCR specificationHash, NavNodeCP parent)
    {
    ChildNodeSpecificationCP spec = rules.FindChildNodeSpecification(specificationHash);
    if (!spec)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Failed to find child node specification by hash: '%s'", specificationHash.c_str()));

    if (spec->GetDoNotSort())
        return false;

    bvector<SortingRuleCP> sortingRules = rules.GetSortingRules(IRulesPreprocessor::AggregateCustomizationRuleParameters(parent, specificationHash));
    if (!sortingRules.empty())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void CreateNodeGroupsBySpecification(bmap<Utf8String, bvector<NavNodePtr>>& groups, NavNodesProviderCR provider)
    {
    DisabledFullNodesLoadContext disableFullLoad(provider);
    for (NavNodePtr node : provider)
        {
        NOT_NULL_PRECONDITION(node, "CreateNodeGroupsBySpecification");

        Utf8StringCR specIdentifier = node->GetKey()->GetSpecificationIdentifier();
        auto groupIter = groups.find(specIdentifier);
        if (groups.end() == groupIter)
            groups.Insert(specIdentifier, bvector<NavNodePtr>{node});
        else
            groupIter->second.push_back(node);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void InsertProviderByIndex(bvector<std::pair<NavNodesProviderPtr, bvector<uint64_t>>>& list, NavNodesProviderPtr provider, bvector<uint64_t> const& providerIndex)
    {
    for (size_t i = 0; i < list.size(); ++i)
        {
        if (NodesCacheHelpers::CompareIndexes(list[i].second, providerIndex) > 0)
            {
            list.insert(list.begin() + i, std::make_pair(provider, providerIndex));
            return;
            }
        }
    list.push_back(std::make_pair(provider, providerIndex));
    }

/*=================================================================================**//**
* @bsiclass
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr DisplayLabelSortingPostProcessor::_PostProcessProvider(NavNodesProviderR processedProvider) const
    {
    auto scope = Diagnostics::Scope::Create("Display label sorting post-processor: Post-process");

    NavNodesProviderContextCR context = processedProvider.GetContext();
    if (context.GetMergedOptimizationFlags().IsPostProcessingDisabled())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Post-processing disabled");
        return nullptr;
        }

    CombinedHierarchyLevelIdentifier hlIdentifier = context.GetHierarchyLevelIdentifier();
    IHierarchyCache& cache = context.GetNodesCache();
    bool isNodesLoadingEnabled = !hlIdentifier.GetRemovalId().IsValid();
    // make sure all data sources are initialized
    processedProvider.InitializeDataSources();

    bvector<DataSourceInfo> dsInfos = cache.FindDataSources(hlIdentifier, context.GetRulesetVariables(),
        DataSourceInfo::PART_SpecificationHash | DataSourceInfo::PART_NodeTypes | DataSourceInfo::PART_IsPartial);
    size_t nonPartialDatasourcesCount = ContainerHelpers::Count(dsInfos, [](auto const& dsInfo){return !dsInfo.IsPartial();});
    if (nonPartialDatasourcesCount < 2)
        {
        // if hierarchy level consists of only one datasource, then it's already sorted and we don't
        // need any post-processing here
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Found less than 2 data sources - nothing to sort.");
        return nullptr;
        }
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Found %" PRIu64 " data sources", (uint64_t)dsInfos.size()));

    NavNodeCPtr parent = context.GetVirtualParentNode();
    bmap<Utf8String, bvector<NavNodePtr>> deprecatedSameLabelPostProcessedNodes;

    // group adjacent label-sorted providers by specification hash and node types
    bvector<bpair<DataSourcesMergeKey, bpair<bvector<NavNodesProviderPtr>, bvector<uint64_t>>>> providers;
    for (DataSourceInfo const& info : dsInfos)
        {
        ThrowIfCancelled(context.GetCancelationToken());

        NavNodesProviderPtr provider = context.GetNodesCache().GetDataSource(*CreateContextForSameHierarchyLevel(context, true), info.GetIdentifier(), isNodesLoadingEnabled, true);
        if (provider.IsNull() && isNodesLoadingEnabled)
            provider = processedProvider.FindNestedProvider(info.GetIdentifier());
        if (provider.IsNull())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to come up with a nodes' provider");

        if (!provider->HasNodes())
            {
            // provider has no nodes - skip it
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Nested provider has no nodes - skip it");
            continue;
            }

        if (info.GetSpecificationHash().empty())
            {
            // this is a special case for data sources created by SameLabelGroupingNodesPostProcessorDeprecated - they don't get
            // an index, so their order is not correct. as a workaround, we take all their nodes and place in appropriate positions.
            // hopefully the solution is temporary until we have a single place where we merge branch into cache and can use it to
            // post-process before merging - that should allow us to completely get rid of the deprecated post-processor
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Data source is not based on specification. Query nodes to find individual specification info.");
            CreateNodeGroupsBySpecification(deprecatedSameLabelPostProcessedNodes, *provider);
            continue;
            }

        DataSourcesMergeKey mergeKey(info);
        bool mergeWithPrevious = !providers.empty()
            && providers.back().first.ShouldMergeWith(mergeKey)
            && IsDataSourceLabelSorted(context.GetRulesPreprocessor(), info.GetSpecificationHash(), parent.get());

        if (mergeWithPrevious)
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Will merge provider with previous one.");
            providers.back().second.first.push_back(provider);
            }
        else
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Will not merge provider with previous one.");
            providers.push_back(make_bpair(mergeKey, make_bpair(bvector<NavNodesProviderPtr>{provider}, info.GetIdentifier().GetIndex())));
            }
        }

    auto mergedProviderContext = CreateContextForSameHierarchyLevel(context, true);
    bvector<std::pair<NavNodesProviderPtr, bvector<uint64_t>>> mergedProviders;
    for (auto const& entry : providers)
        {
        ThrowIfCancelled(context.GetCancelationToken());

        if (entry.second.first.empty())
            continue;

        auto sameLabelGroupedNodesIter = deprecatedSameLabelPostProcessedNodes.find(entry.first.m_specHash);
        if (deprecatedSameLabelPostProcessedNodes.end() != sameLabelGroupedNodesIter)
            {
            // found grouped nodes with created using the same spec - they should be sorted together with the rest of the nodes
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Create delayed label sorting nodes provider from individual nodes and providers.");
            std::sort(sameLabelGroupedNodesIter->second.begin(), sameLabelGroupedNodesIter->second.end(), CompareNodePtrsByLabel);
            bvector<NavNodesProviderPtr> sortedProviders = entry.second.first;
            sortedProviders.push_back(BVectorNodesProvider::Create(*CreateContextForSameHierarchyLevel(*mergedProviderContext, true), sameLabelGroupedNodesIter->second));
            mergedProviders.push_back(std::make_pair(DelayedLabelSortingNodesProvider::Create(*CreateContextForSameHierarchyLevel(*mergedProviderContext, true), sortedProviders), entry.second.second));
            deprecatedSameLabelPostProcessedNodes.erase(sameLabelGroupedNodesIter);
            }
        else if (entry.second.first.size() > 1)
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Create delayed label sorting nodes provider from providers.");
            mergedProviders.push_back(std::make_pair(DelayedLabelSortingNodesProvider::Create(*CreateContextForSameHierarchyLevel(*mergedProviderContext, true), entry.second.first), entry.second.second));
            }
        else
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Provider is not grouped with others. Just reuse it.");
            mergedProviders.push_back(std::make_pair(entry.second.first.front(), entry.second.second));
            }
        }
    // might still have some nodes... just put them based on their index
    for (auto const& entry : deprecatedSameLabelPostProcessedNodes)
        {
        if (entry.second.empty())
            continue;

        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Found some left over individual nodes - insert by index.");
        auto firstNodeIndex = context.GetNodesCache().GetNodeIndex(entry.second.front()->GetNodeId());
        InsertProviderByIndex(mergedProviders, BVectorNodesProvider::Create(*CreateContextForSameHierarchyLevel(*mergedProviderContext, true), entry.second), firstNodeIndex);
        }

    // create a merged provider
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Create merged nodes provider");
    auto mergedProvider = MergedNavNodesProvider::Create(*mergedProviderContext, processedProvider);
    mergedProvider->SetProviders(ContainerHelpers::TransformContainer<bvector<NavNodesProviderPtr>>(mergedProviders, [](auto const& entry){return entry.first;}));
    return mergedProvider;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NodesFinalizingProvider::NodesFinalizingIteratorImpl::_GetCurrent() const
    {
    NavNodePtr node = *m_iter;
    NOT_NULL_PRECONDITION(node, "NodesFinalizingIteratorImpl::_GetCurrent");
    return m_provider->FinalizeNode(*node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NodesFinalizingProvider::FinalizeNode(NavNodeR node) const
    {
    if (GetContext().GetMergedOptimizationFlags().IsFullNodesLoadDisabled())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Full load disabled - return.");
        return &node;
        }

    return NodesFinalizer(GetContextR()).Finalize(node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesFinalizingPostProcessor::_PostProcessProvider(NavNodesProviderR processedProvider) const
    {
    processedProvider.GetContext().GetHierarchyLevelLocker().WaitForUnlock();
    auto context = CreateContextForSameHierarchyLevel(processedProvider.GetContext(), processedProvider.GetContext().GetRelatedVariablesIds(), true);
    return NodesFinalizingProvider::Create(*context, processedProvider);
    }

/*=================================================================================**//**
* @bsiclass
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
    ECPresentation::IJsonLocalState const* m_localState;
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
    PostProcessedClassesFinder(ECSchemaHelper const& schemaHelper, ECPresentation::IJsonLocalState const* localState)
        : m_schemaHelper(schemaHelper), m_localState(localState)
        {}
    bset<ECClassCP> const& GetClasses() const { return m_classes; }
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SameLabelGroupingNodesPostProcessorDeprecated::InitGroupedClasses(IRulesPreprocessorR rulesPreprocessor, NavNodeCP parentNode, bvector<ChildNodeSpecificationCP> const& specs,
    ECSchemaHelper const& schemaHelper, IJsonLocalState const* localState)
    {
    auto scope = Diagnostics::Scope::Create("Init grouped classes");

    PostProcessedClassesFinder sameLabelPostProcessedClasses(schemaHelper, localState);

    auto specHashes = ContainerHelpers::TransformContainer<bvector<Utf8String>>(specs, [](auto const& spec){return spec->GetHash();});
    IRulesPreprocessor::AggregateCustomizationRuleParameters params(parentNode, specHashes);
    for (auto rule : rulesPreprocessor.GetGroupingRules(params))
        {
        DiagnosticsHelpers::ReportRule(*rule);
        rule->Accept(sameLabelPostProcessedClasses);
        }

    m_groupedClasses = sameLabelPostProcessedClasses.GetClasses();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SameLabelGroupingNodesPostProcessorDeprecated::IsSuitableForMerge(NavNodeCR node) const
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void MergeInstanceKeys(NavNodesProviderContextCR context, NavNodeR target, NavNodeCR source)
    {
    NavNodeExtendedData targetExtendedData(target);
    NavNodeExtendedData sourceExtendedData(source);

    bvector<ECClassInstanceKey> instanceKeys = target.GetKey()->AsECInstanceNodeKey()->GetInstanceKeys();
    for (auto const& sourceInstanceKey : source.GetKey()->AsECInstanceNodeKey()->GetInstanceKeys())
        {
        if (!ContainerHelpers::Contains(instanceKeys, sourceInstanceKey))
            instanceKeys.push_back(sourceInstanceKey);
        }

    auto targetVirtualParentIds = targetExtendedData.GetVirtualParentIds();
    for (BeGuidCR sourceVirtualParentId : sourceExtendedData.GetVirtualParentIds())
        {
        if (!ContainerHelpers::Contains(targetVirtualParentIds, sourceVirtualParentId))
            targetExtendedData.AddVirtualParentId(sourceVirtualParentId);
        }

    targetExtendedData.AddMergedNodeId(source.GetNodeId());

    NavNodeCPtr parent = context.GetVirtualParentNode();
    ECInstancesNodeKeyPtr nodeKey = ECInstancesNodeKey::Create(context.GetConnection(), target.GetKey()->GetSpecificationIdentifier(),
        parent.IsValid() ? parent->GetKey().get() : nullptr, instanceKeys);

    nodeKey->SetInstanceKeysSelectQuery(UnionQueryBuilder::Create({
        StringQueryBuilder::Create(*target.GetKey()->GetInstanceKeysSelectQuery()),
        StringQueryBuilder::Create(*source.GetKey()->GetInstanceKeysSelectQuery()),
        })->CreateQuery());

    target.SetNodeKey(*nodeKey);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr SameLabelGroupingNodesPostProcessorDeprecated::MergeNodes(NavNodesProviderContextCR context, NavNodeR lhs, NavNodeR rhs) const
    {
    if (!lhs.GetKey()->AsECInstanceNodeKey() || !rhs.GetKey()->AsECInstanceNodeKey())
        {
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Both nodes must be ECInstance nodes. "
            "Got lhs : '%s', rhs : '%s'", lhs.GetType().c_str(), rhs.GetType().c_str()));
        }

    if (!lhs.GetLabelDefinition().GetDisplayValue().Equals(rhs.GetLabelDefinition().GetDisplayValue()))
        {
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Labels of both nodes must match. "
            "Got lhs : '%s', rhs : '%s'", lhs.GetLabelDefinition().GetDisplayValue().c_str(), rhs.GetLabelDefinition().GetDisplayValue().c_str()));
        }

    NavNodePtr node = lhs.Clone();
    node->SetNodeId(BeGuid());
    node->ResetHasChildren();
    NavNodeExtendedData(*node).AddMergedNodeId(lhs.GetNodeId());
    MergeInstanceKeys(context, *node, rhs);
    return node;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
HierarchyLevelIdentifier SameLabelGroupingNodesPostProcessorDeprecated::GetHierarchyLevelIdentifier(NavNodesProviderContextCR context, bool createNew)
    {
    auto combinedHlIdentifier = context.GetHierarchyLevelIdentifier();
    BeGuid virtualParentId = context.GetVirtualParentNode().IsValid() ? context.GetVirtualParentNode()->GetNodeId() : BeGuid();
    BeGuid hlId = context.GetNodesCache().FindHierarchyLevelId(combinedHlIdentifier.GetConnectionId().c_str(),
        combinedHlIdentifier.GetRulesetId().c_str(), virtualParentId, combinedHlIdentifier.GetRemovalId());

    HierarchyLevelIdentifier identifier(combinedHlIdentifier, virtualParentId);
    if (hlId.IsValid())
        identifier.SetId(hlId);
    else if (createNew)
        context.GetNodesCache().Cache(identifier);

    return identifier;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceInfo SameLabelGroupingNodesPostProcessorDeprecated::GetMergedDataSourceInfo(NavNodesProviderContextCR context, BeGuidCR hierarchyLevelId, bool createNew)
    {
    if (!hierarchyLevelId.IsValid())
        return DataSourceInfo();

    DataSourceIdentifier mergedDatasourceIdentifier(hierarchyLevelId, {}, context.GetInstanceFilterPtr());
    mergedDatasourceIdentifier.SetResultSetSizeLimit(context.GetResultSetSizeLimit());
    DataSourceInfo mergedDatasourceInfo = context.GetNodesCache().FindDataSource(mergedDatasourceIdentifier, context.GetRulesetVariables(), DataSourceInfo::PART_IsFinalized);
    if (mergedDatasourceInfo.GetIdentifier().IsValid() || !createNew)
        return mergedDatasourceInfo;

    mergedDatasourceInfo = DataSourceInfo(mergedDatasourceIdentifier, context.GetRelatedRulesetVariables(), DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
    context.GetNodesCache().Cache(mergedDatasourceInfo);
    return mergedDatasourceInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr SameLabelGroupingNodesPostProcessorDeprecated::_PostProcess(NavNodesProviderCR processedProvider) const
    {
    auto scope = Diagnostics::Scope::Create("Same label grouping nodes post-processor: Post-process");

    if (processedProvider.GetContext().GetMergedOptimizationFlags().IsPostProcessingDisabled())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Post-processing disabled");
        return nullptr;
        }

    // don't need to do anything if there are no classes to which the same label grouping applies
    if (m_groupedClasses.empty())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "No grouped classes - return.");
        return nullptr;
        }

    // don't need to do anything if we're not using any of the classes that should be grouped
    auto usedClasses = processedProvider.GetResultInstanceNodesClasses();
    bool isAnyOfUsedClassesGrouped = ContainerHelpers::Contains(m_groupedClasses, [&usedClasses](ECClassCP grouped)
        {
        for (ECClassCP usedClass : usedClasses)
            {
            if (usedClass->Is(grouped))
                return true;
            }
        return false;
        });
    if (!isAnyOfUsedClassesGrouped)
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "None of used classes should be grouped - return.");
        return nullptr;
        }

    processedProvider.GetContext().GetHierarchyLevelLocker().WaitForUnlock();

    // create a context for merged nodes provider
    NavNodesProviderContextPtr context = CreateContextForSameHierarchyLevel(processedProvider.GetContext(), processedProvider.GetContext().GetRelatedVariablesIds(), false);

    // attempt to find cached merged nodes provider - success means the whole hierarchy level is already post-processed and in cache.
    // it's more efficient to use the cached version compared to loading and merging everything again, so just return the cached provider.
    HierarchyLevelIdentifier hlIdentifier = GetHierarchyLevelIdentifier(*context);
    DataSourceInfo mergedDatasourceInfo = GetMergedDataSourceInfo(*context, hlIdentifier.GetId());
    if (mergedDatasourceInfo.IsInitialized())
        {
        auto cachedProcessedProvider = context->GetNodesCache().GetCombinedHierarchyLevel(*context, context->GetHierarchyLevelIdentifier());
        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Hierarchies, cachedProcessedProvider.IsValid(), "Expected the processed provider to be cached but it's not.");
        return cachedProcessedProvider;
        }

    // nothing to group if there are less than 2 nodes
    {
    MaxNodesToLoadContext maxNodesToLoad(processedProvider, 2);
    if (processedProvider.GetNodesCount() < 2)
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Processed provider has less than 2 nodes - return.");
        return nullptr;
        }
    }

    processedProvider.GetContext().GetHierarchyLevelLocker().Lock();

    // if we didn't find the cached provider, it means we're here for the first time - create and cache the merged nodes provider (even if it's going to be empty)
    IHierarchyCache::SavepointPtr savepoint = context->GetNodesCache().CreateSavepoint();

    // attempt to get merged data source again in case it was cached while we were checking nodes count. Otherwise cache new merged data source.
    hlIdentifier = GetHierarchyLevelIdentifier(*context, true);
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Hierarchies, hlIdentifier.IsValid(), "Expected hierarchy level to be cached but it's not.");
    mergedDatasourceInfo = GetMergedDataSourceInfo(*context, hlIdentifier.GetId(), true);
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Hierarchies, mergedDatasourceInfo.GetIdentifier().IsValid(), "Expected data source to be cached but it's not.");

    // return cached provider if it was created while we were checking nodes count
    if (mergedDatasourceInfo.IsInitialized())
        {
        auto cachedProcessedProvider = context->GetNodesCache().GetCombinedHierarchyLevel(*context, context->GetHierarchyLevelIdentifier());
        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Hierarchies, cachedProcessedProvider.IsValid(), "Expected the processed provider to be cached but it's not.");
        return cachedProcessedProvider;
        }

    DataSourceIdentifier mergedDatasourceIdentifier = mergedDatasourceInfo.GetIdentifier();

    MaxNodesToLoadContext disableMaxNodesToLoad(processedProvider, (size_t)0);
    DisabledFullNodesLoadContext disableFullNodesLoad(processedProvider);

    size_t mergedNodesCount = 0;
    bvector<NavNodePtr> nodes;
    bmap<Utf8String, size_t> labelsMap;
    bmap<size_t, bvector<uint64_t>> mergedNodeIndexes;
    for (NavNodePtr node : processedProvider)
        {
        NOT_NULL_PRECONDITION(node, "SameLabelGroupingNodesPostProcessorDeprecated::_PostProcess");
        auto iter = labelsMap.find(node->GetLabelDefinition().GetDisplayValue());
        if (labelsMap.end() != iter)
            {
            size_t pos = iter->second;
            if (IsSuitableForMerge(*nodes[pos]) && IsSuitableForMerge(*node))
                {
                NavNodePtr merged;
                auto mergedNodeIndexIter = mergedNodeIndexes.find(pos);
                if (mergedNodeIndexes.end() != mergedNodeIndexIter)
                    {
                    // nodes[pos] is an already merged node
                    merged = nodes[pos];
                    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Merge %s into %s at %" PRIu64,
                        DiagnosticsHelpers::CreateNodeIdentifier(*node).c_str(), DiagnosticsHelpers::CreateNodeIdentifier(*merged).c_str(), (uint64_t)pos));
                    MergeInstanceKeys(*context, *merged, *node);
                    }
                else
                    {
                    // first time merge
                    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("First time merge: %s with %s at %" PRIu64,
                        DiagnosticsHelpers::CreateNodeIdentifier(*nodes[pos]).c_str(), DiagnosticsHelpers::CreateNodeIdentifier(*node).c_str(), (uint64_t)pos));
                    merged = MergeNodes(*context, *nodes[pos], *node);
                    mergedNodeIndexIter = mergedNodeIndexes.Insert(
                        pos,
                        context->GetNodesCache().GetNodeIndex(mergedDatasourceIdentifier.GetHierarchyLevelId(), nodes[pos]->GetNodeId(), context->GetRulesetVariables(), context->GetInstanceFilter(), context->GetResultSetSizeLimit())
                        ).first;
                    context->GetNodesCache().MakeHidden(nodes[pos]->GetNodeId(), context->GetRulesetVariables(), context->GetInstanceFilter(), context->GetResultSetSizeLimit());
                    nodes[pos] = merged;
                    ++mergedNodesCount;
                    }
                context->GetNodesCache().MakeHidden(node->GetNodeId(), context->GetRulesetVariables(), context->GetInstanceFilter(), context->GetResultSetSizeLimit());
                continue;
                }
            }
        nodes.push_back(node);
        labelsMap.Insert(node->GetLabelDefinition().GetDisplayValue(), nodes.size() - 1);
        }

    // cache the merged nodes
    for (auto const& mergedNodeIndexEntry : mergedNodeIndexes)
        {
        uint64_t mergedNodeIndex = mergedNodeIndexEntry.first;
        NavNodePtr mergedNode = nodes[mergedNodeIndex];
        context->GetNodesCache().Cache(*mergedNode, mergedDatasourceIdentifier, mergedNodeIndexEntry.second, NodeVisibility::Visible);
        }

    // update the merged nodes provider with the actual information
    mergedDatasourceInfo.SetIsInitialized(true);
    mergedDatasourceInfo.SetDirectNodesCount(mergedNodesCount);
    mergedDatasourceInfo.SetTotalNodesCount(mergedNodesCount);
    mergedDatasourceInfo.SetHasNodes(mergedNodesCount > 0);
    context->GetNodesCache().Update(mergedDatasourceInfo, DataSourceInfo::PART_TotalNodesCount | DataSourceInfo::PART_HasNodes | DataSourceInfo::PART_IsFinalized | DataSourceInfo::PART_DirectNodesCount);
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Updated merged provider");

    // return the merged nodes list
    return BVectorNodesProvider::Create(*context, nodes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static NavNodesProviderContextPtr CreatePostProcessingProviderContext(NavNodesProviderContextR baseContext)
    {
    NavNodesProviderContextPtr context = CreateContextForSameHierarchyLevel(baseContext, baseContext.GetRelatedVariablesIds(), true);
    context->SetVirtualParentNode(baseContext.GetVirtualParentNode().get());

    context->SetAncestorContext(baseContext.GetAncestorContext());
    baseContext.SetAncestorContext(context.get());

    context->GetOptimizationFlags().From(baseContext.GetOptimizationFlags());
    baseContext.GetOptimizationFlags().Reset();

    return context;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PostProcessingNodesProviderDeprecated::PostProcessingNodesProviderDeprecated(NavNodesProviderR wrappedProvider)
    : MultiNavNodesProvider(*CreatePostProcessingProviderContext(wrappedProvider.GetContextR())), m_wrappedProvider(&wrappedProvider), m_isPostProcessed(false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr PostProcessingNodesProviderDeprecated::_FindNestedProvider(DataSourceIdentifier const& identifier)
    {
    NavNodesProviderPtr nestedProvider = T_Super::_FindNestedProvider(identifier);
    if (nestedProvider.IsNull())
        nestedProvider = m_wrappedProvider->FindNestedProvider(identifier);
    return nestedProvider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr PostProcessingNodesProviderDeprecated::GetProcessedProvider() const
    {
    if (!m_isPostProcessed)
        {
        NavNodesProviderPtr provider = m_wrappedProvider;
        for (auto const& postProcessor : m_postProcessors)
            {
            NavNodesProviderPtr processed = postProcessor->PostProcess(*provider);
            if (processed.IsNull())
                continue;

            provider = processed;
            }
        const_cast<PostProcessingNodesProviderDeprecated*>(this)->AddProvider(*provider);
        m_isPostProcessed = true;
        }
    return GetNodeProviders().front();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CountInfo PostProcessingNodesProviderDeprecated::_GetTotalNodesCount() const
    {
    return GetProcessedProvider()->GetTotalNodesCount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PostProcessingNodesProviderDeprecated::_OnPageOptionsSet()
    {
    return GetProcessedProvider()->SetPageOptions(GetContext().GetPageOptions());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PostProcessingNodesProviderDeprecated::_InitializeDataSources()
    {
    return m_wrappedProvider->InitializeDataSources();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PostProcessingNodesProviderDeprecated::_HasNodes() const
    {
    return m_wrappedProvider->HasNodes();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProvider::Iterator PostProcessingNodesProviderDeprecated::_CreateFrontIterator() const
    {
    GetProcessedProvider();
    return T_Super::_CreateFrontIterator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProvider::Iterator PostProcessingNodesProviderDeprecated::_CreateBackIterator() const
    {
    GetProcessedProvider();
    return T_Super::_CreateBackIterator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TProvider>
void SQLiteCacheNavNodesProviderBase<TProvider>::OnCreated()
    {
    InitializeUsedVariables();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TProvider>
void SQLiteCacheNavNodesProviderBase<TProvider>::InitializeUsedVariables()
    {
    auto scope = Diagnostics::Scope::Create("Initialize used ruleset variables");

    BeMutexHolder lock(m_cacheMutex);
    CachedStatementPtr stmt = _GetUsedVariablesStatement();
    if (stmt.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare used variables statement");

    auto& listener = TProvider::GetContext().GetUsedVariablesListener();
    while (BE_SQLITE_ROW == stmt->Step())
        {
        bvector<Utf8String> usedVariables = RulesetVariables::FromSerializedInternalJsonObjectString(stmt->GetValueText(0)).GetVariableNames();
        for (Utf8StringCR name : usedVariables)
            listener.OnVariableUsed(name.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TProvider>
std::unordered_set<ECClassCP> SQLiteCacheNavNodesProviderBase<TProvider>::_GetResultInstanceNodesClasses() const
    {
    BeMutexHolder lock(m_cacheMutex);
    std::unordered_set<ECClassCP> classes;
    CachedStatementPtr stmt = _GetResultInstanceNodesClassIdsStatement();
    if (stmt.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Got invalid node classes statement");

    while (BE_SQLITE_ROW == stmt->Step())
        {
        ECClassId classId = stmt->GetValueId<ECClassId>(0);
        ECClassCP ecClass = TProvider::GetContext().GetSchemaHelper().GetECClass(classId);
        if (!ecClass)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Invalid ECClass ID: %" PRIu64, classId.GetValue()));

        classes.insert(ecClass);
        }
    return classes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TProvider>
size_t SQLiteCacheNavNodesProviderBase<TProvider>::_GetLimitedInstancesCount(size_t limit) const
    {
    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Not expecting instance count requests on cached node providers");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<CachedHierarchyLevelProvider> CachedHierarchyLevelProvider::Create(NavNodesProviderContextR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements, BeMutex& cacheMutex, BeGuid hierarchyLevelId)
    {
    return CallOnCreated(*new CachedHierarchyLevelProvider(context, cache, statements, cacheMutex, hierarchyLevelId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedHierarchyLevelProvider::_GetUsedVariablesStatement() const
    {
    static Utf8CP query =
        "SELECT [dsv].[Variables] "
        "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
        "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId]"
        " WHERE [hl].[Id] = ?"
        "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) ";

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Used variables query: `%s`", query));

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare used variables statement");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, m_hierarchyLevelId);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);

    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedHierarchyLevelProvider::_GetResultInstanceNodesClassIdsStatement() const
    {
    static Utf8CP query =
        "SELECT DISTINCT [ni].[ECClassId] "
        "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
        "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId]"
        "  JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn ON [dsn].[DataSourceId] = [ds].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[Id] = [dsn].[NodeId]"
        "  JOIN [" NODESCACHE_TABLENAME_NodeInstances "] ni on [ni].[NodeId] = [n].[Id]"
        " WHERE [hl].[Id] = ?"
        "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) ";

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Result instance node class IDs query: `%s`", query));

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare instance node class IDs statement");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, m_hierarchyLevelId);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);

    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<DirectNodesIterator> CachedHierarchyLevelProvider::_CreateDirectNodesIterator() const
    {
    return SqliteCacheDirectNodeIteratorBase::CreateForHierarchyLevel(GetContext(), GetCache(), GetStatements(), GetCacheMutex(), m_hierarchyLevelId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SQLiteCacheNodesProvider::SQLiteCacheNodesProvider(NavNodesProviderContextR context, Db& cache, StatementCache& statements, BeMutex& cacheMutex)
    : T_Super(context, cache, statements, cacheMutex)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SQLiteCacheNodesProvider::_ResetInitializedNodes()
    {
    m_nodes = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NodesInitializationState SQLiteCacheNodesProvider::_InitializeNodes()
    {
    if (nullptr != m_nodes)
        return NodesInitializationState({ make_bpair(0, m_nodes->size()) }, m_nodes->size()); // already initialized

    auto scope = Diagnostics::Scope::Create("Initialize nodes from cache");

    BeMutexHolder lock(GetCacheMutex());
    m_nodes = std::make_unique<bvector<NavNodePtr>>();
    CachedStatementPtr statement = _GetNodesStatement();
    if (!statement.IsValid())
        return ERROR;

    rapidjson::Document json;
    DbResult stepResult;
    while (BE_SQLITE_ROW == (stepResult = statement->Step()))
        {
        NavNodePtr node = NodesCacheHelpers::CreateNodeFromStatement(*statement, GetContext().GetNodesFactory(), GetContext().GetConnection());
        if (node.IsNull())
            continue;

        m_nodes->push_back(node);
        }

    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Hierarchies, BE_SQLITE_DONE == stepResult, Utf8PrintfString("Nodes initialization ended with unexpected db result code: %d", (int)stepResult));
    return NodesInitializationState({ make_bpair(0, m_nodes->size()) }, m_nodes->size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SQLiteCacheNodesProvider::_OnPageOptionsSet()
    {
    T_Super::_OnPageOptionsSet();
    m_nodes = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SQLiteCacheNodesProvider::_HasNodes() const
    {
    if (nullptr != m_nodes)
        return !m_nodes->empty();

    // attempting to get nodes count from here may get us into infinite recursion if
    // this optimization flag is not reset
    MaxNodesToLoadContext getValidCountContext(*this, (size_t)0);
    return GetNodesCount() > 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CountInfo SQLiteCacheNodesProvider::_GetTotalNodesCount() const
    {
    auto scope = Diagnostics::Scope::Create("Execute query to get count");
    BeMutexHolder lock(GetCacheMutex());
    CachedStatementPtr stmt = _GetCountStatement();
    DbResult result = stmt->Step();
    if (BE_SQLITE_ROW != result)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Unexpected step result: %d", (int)result));
    return CountInfo((size_t)stmt->GetValueInt64(0), true);
    }

/*=================================================================================**//**
* @bsiclass
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<CachedCombinedHierarchyLevelProvider> CachedCombinedHierarchyLevelProvider::Create(NavNodesProviderContextR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements,
    BeMutex& cacheMutex, BeGuidCR physicalHierarchyLevelId)
    {
    return CallOnCreated(*new CachedCombinedHierarchyLevelProvider(context, cache, statements, cacheMutex, physicalHierarchyLevelId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedCombinedHierarchyLevelProvider::_GetUsedVariablesStatement() const
    {
    static Utf8CP query =
        WITH_PHYSICAL_HIERARCHY_LEVELS(
            PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".[Id] = ? "
            "AND " PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".[RemovalId] IS ?"
        )
        "SELECT [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_Variables "] "
        "  FROM [" PHYSICAL_HIERARCHY_LEVELS_TABLE_NAME "] phl "
        " WHERE " NODESCACHE_FUNCNAME_VariablesMatch "([phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_Variables "], ?) ";

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Used variables query: `%s`", query));

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare used variables statement");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, m_physicalHierarchyLevelId);
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, GetContext().GetRemovalId());
    stmt->BindInt(bindingIndex++, (int)NodeVisibility::Visible);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);

    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedCombinedHierarchyLevelProvider::_GetResultInstanceNodesClassIdsStatement() const
    {
    static Utf8CP query =
        WITH_PHYSICAL_HIERARCHY_LEVELS(
            PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".[Id] = ? "
            "AND " PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".[RemovalId] IS ? "
        )
        "SELECT DISTINCT [ni].[ECClassId] "
        "  FROM [" PHYSICAL_HIERARCHY_LEVELS_TABLE_NAME "] phl "
        "  CROSS JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn ON [dsn].[DataSourceId] = [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_DataSourceId "] "
        "  CROSS JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_DataSourceId "] "
        "  CROSS JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[Id] = [dsn].[NodeId]"
        "  CROSS JOIN [" NODESCACHE_TABLENAME_NodeInstances "] ni on [ni].[NodeId] = [n].[Id]"
        " WHERE [ds].[InstanceFilter] IS ? "
        "       AND [ds].[ResultSetSizeLimit] IS ? "
        "       AND [dsn].[Visibility] = ? "
        "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_Variables "], ?) ";

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Result instance node class IDs query: `%s`", query));

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare instance node class IDs statement");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, m_physicalHierarchyLevelId);
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, GetContext().GetRemovalId());
    NodesCacheHelpers::BindInstanceFilter(*stmt, bindingIndex++, GetContext().GetInstanceFilter());
    NodesCacheHelpers::BindNullable(*stmt, bindingIndex++, GetContext().GetResultSetSizeLimit(), &Statement::BindUInt64);
    stmt->BindInt(bindingIndex++, (int)NodeVisibility::Visible);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);

    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedCombinedHierarchyLevelProvider::_GetNodesStatement() const
    {
    Utf8String query =
        WITH_PHYSICAL_HIERARCHY_LEVELS(
            PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".[Id] = ? "
            "AND " PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".[RemovalId] IS ? "
        )
        "SELECT " NODE_SELECT_STMT("hl", "n", "nk")
        "  FROM [" PHYSICAL_HIERARCHY_LEVELS_TABLE_NAME "] phl "
        "  CROSS JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_HierarchyLevelId "] "
        "  CROSS JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_DataSourceId "] "
        "  CROSS JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn ON [dsn].[DataSourceId] = [ds].[Id] "
        "  CROSS JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[Id] = [dsn].[NodeId] "
        "  CROSS JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id] "
        " WHERE [ds].[InstanceFilter] IS ? "
        "       AND [ds].[ResultSetSizeLimit] IS ? "
        "       AND [dsn].[Visibility] = ? "
        "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_Variables "], ?) "
        " ORDER BY " NODESCACHE_FUNCNAME_ConcatBinaryIndex "([phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_DataSourceIndex "], [dsn].[NodeIndex]) ";
    if (GetContext().HasPageOptions())
        query.append(" LIMIT ? OFFSET ? ");

#ifdef CHECK_ORDERED_QUERY_PLANS
    CheckQueryPlan(GetCache(), query.c_str(), GetStatements());
#endif

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Nodes query: `%s`", query.c_str()));

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare nodes statement");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, m_physicalHierarchyLevelId);
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, GetContext().GetRemovalId());
    NodesCacheHelpers::BindInstanceFilter(*stmt, bindingIndex++, GetContext().GetInstanceFilter());
    NodesCacheHelpers::BindNullable(*stmt, bindingIndex++, GetContext().GetResultSetSizeLimit(), &Statement::BindUInt64);
    stmt->BindInt(bindingIndex++, (int)NodeVisibility::Visible);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
    if (GetContext().GetPageOptions())
        {
        stmt->BindInt64(bindingIndex++, GetContext().GetPageOptions()->HasSize() ? GetContext().GetPageOptions()->GetSize() : -1);
        stmt->BindInt64(bindingIndex++, GetContext().GetPageOptions()->GetStart());
        }

    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedCombinedHierarchyLevelProvider::_GetCountStatement() const
    {
    static Utf8CP query =
        WITH_PHYSICAL_HIERARCHY_LEVELS(
            PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".[Id] = ? "
            "AND " PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".[RemovalId] IS ? "
        )
        "SELECT COUNT(1) "
        "  FROM [" PHYSICAL_HIERARCHY_LEVELS_TABLE_NAME "] phl "
        "  CROSS JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_DataSourceId "] "
        "  CROSS JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn ON [dsn].[DataSourceId] = [ds].[Id] "
        " WHERE [dsn].[Visibility] = ? "
        "       AND [ds].[InstanceFilter] IS ? "
        "       AND [ds].[ResultSetSizeLimit] IS ? "
        "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_Variables "], ?) ";

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Nodes count query: `%s`", query));

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare nodes count statement");

    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, m_physicalHierarchyLevelId);
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, GetContext().GetRemovalId());
    stmt->BindInt(bindingIndex++, (int)NodeVisibility::Visible);
    NodesCacheHelpers::BindInstanceFilter(*stmt, bindingIndex++, GetContext().GetInstanceFilter());
    NodesCacheHelpers::BindNullable(*stmt, bindingIndex++, GetContext().GetResultSetSizeLimit(), &Statement::BindUInt64);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);

    return stmt;
    }

#define HIDDEN_NODES_TABLE_NAME "hidden_nodes"
#define HIDDEN_NODES_COLUMN_NAME_DataSourceId "DataSourceId"
#define HIDDEN_NODES_COLUMN_NAME_Variables "Variables"
#define HIDDEN_NODES_COLUMN_NAME_NodeId "NodeId"
#define WITH_HIDDEN_NODES \
    "WITH RECURSIVE" \
    "    " HIDDEN_NODES_TABLE_NAME "(" HIDDEN_NODES_COLUMN_NAME_DataSourceId ", " HIDDEN_NODES_COLUMN_NAME_Variables ", " HIDDEN_NODES_COLUMN_NAME_NodeId ") AS ( " \
    "        SELECT ds.Id, dsv.Variables, dsn.NodeId " \
    "        FROM [" NODESCACHE_TABLENAME_DataSourceNodes "] AS dsn " \
    "        JOIN [" NODESCACHE_TABLENAME_DataSources "] AS ds ON [ds].[Id] = [dsn].[DataSourceId] " \
    "        JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId] " \
    "        WHERE [dsn].[Visibility] = 2 " \
    "        UNION ALL " \
    "        SELECT ds.Id, dsv.Variables, dsn.NodeId " \
    "        FROM [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn " \
    "        JOIN [" NODESCACHE_TABLENAME_DataSources "] AS ds ON [ds].[Id] = [dsn].[DataSourceId] " \
    "        JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId] " \
    "        JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] AS hl ON [hl].[Id] = [ds].[HierarchyLevelId] " \
    "        JOIN " HIDDEN_NODES_TABLE_NAME " AS hdsn ON [hl].[ParentNodeId] = [hdsn].[" HIDDEN_NODES_COLUMN_NAME_NodeId "] OR [hdsn].[" HIDDEN_NODES_COLUMN_NAME_NodeId "] IS NULL AND [hl].[ParentNodeId] IS NULL " \
    "    )"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<NodesWithUndeterminedChildrenProvider> NodesWithUndeterminedChildrenProvider::Create(NavNodesProviderContextR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements,
    BeMutex& cacheMutex)
    {
    return CallOnCreated(*new NodesWithUndeterminedChildrenProvider(context, cache, statements, cacheMutex));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr NodesWithUndeterminedChildrenProvider::_GetUsedVariablesStatement() const
    {
    static Utf8CP query =
        "SELECT [dsv].[Variables] "
        "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hln "
        "  JOIN [" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [hln].[RulesetId] "
        "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hln].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId] "
        "  JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn ON [dsn].[DataSourceId] = [ds].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[Id] = [dsn].[NodeId] "
        " WHERE [r].[Identifier] = ? "
        "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) ";

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Used variables query: `%s`", query));

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare used variables statement");

    int bindingIndex = 1;
    stmt->BindText(bindingIndex++, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr NodesWithUndeterminedChildrenProvider::_GetResultInstanceNodesClassIdsStatement() const
    {
    static Utf8CP query =
        "SELECT DISTINCT [ni].[ECClassId] "
        "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hln "
        "  JOIN [" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [hln].[RulesetId] "
        "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hln].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId]"
        "  JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn ON [dsn].[DataSourceId] = [ds].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[Id] = [dsn].[NodeId] "
        "  JOIN [" NODESCACHE_TABLENAME_NodeInstances "] ni on [ni].[NodeId] = [n].[Id]"
        "    WHERE [r].[Identifier] = ? "
        "          AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) ";

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Result instance node class IDs query: `%s`", query));

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare instance node class IDs statement");

    int bindingIndex = 1;
    stmt->BindText(bindingIndex++, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr NodesWithUndeterminedChildrenProvider::_GetCountStatement() const
    {
    static Utf8CP query =
        WITH_HIDDEN_NODES
        "SELECT COUNT(1) "
        "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hln "
        "  JOIN [" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [hln].[RulesetId] "
        "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hln].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId] "
        "  JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn ON [dsn].[DataSourceId] = [ds].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[Id] = [dsn].[NodeId] "
        " WHERE [r].[Identifier] = ? "
        "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?)"
        "       AND ("
        "           NOT EXISTS ("
        "               SELECT 1"
        "                 FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl"
        "                 JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id]"
        "                WHERE [hl].[ParentNodeId] = [n].[Id]"
        "           )"
        "           OR EXISTS ("
        "              SELECT 1"
        "                FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl"
        "                JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id]"
        "               WHERE [hl].[ParentNodeId] = [n].[Id] AND NOT [ds].[IsInitialized]"
        "           )"
        "       )"
        "       AND NOT EXISTS ("
        "           SELECT 1 "
        "             FROM [" HIDDEN_NODES_TABLE_NAME "]"
        "            WHERE [" HIDDEN_NODES_COLUMN_NAME_NodeId "] = [n].[Id]"
        "                  AND " NODESCACHE_FUNCNAME_VariablesMatch "([" HIDDEN_NODES_COLUMN_NAME_Variables "], ?)"
        "       )";

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Nodes count query: `%s`", query));

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare nodes count statement");

    int bindingIndex = 1;
    stmt->BindText(bindingIndex++, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr NodesWithUndeterminedChildrenProvider::_GetNodesStatement() const
    {
    static Utf8CP query =
        WITH_HIDDEN_NODES
        "SELECT " NODE_SELECT_STMT("hln", "n", "nk")
        "  FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hln "
        "  JOIN [" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [hln].[RulesetId] "
        "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hln].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId] "
        "  JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn ON [dsn].[DataSourceId] = [ds].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[Id] = [dsn].[NodeId] "
        "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id] "
        " WHERE [r].[Identifier] = ? "
        "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?)"
        "       AND ("
        "           NOT EXISTS ("
        "               SELECT 1"
        "                 FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl"
        "                 JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id]"
        "                WHERE [hl].[ParentNodeId] = [n].[Id]"
        "           )"
        "           OR EXISTS ("
        "              SELECT 1"
        "                FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl"
        "                JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[HierarchyLevelId] = [hl].[Id]"
        "               WHERE [hl].[ParentNodeId] = [n].[Id] AND NOT [ds].[IsInitialized]"
        "           )"
        "       )"
        "       AND NOT EXISTS ("
        "           SELECT 1 "
        "             FROM [" HIDDEN_NODES_TABLE_NAME "]"
        "            WHERE [" HIDDEN_NODES_COLUMN_NAME_NodeId "] = [n].[Id]"
        "                  AND " NODESCACHE_FUNCNAME_VariablesMatch "([" HIDDEN_NODES_COLUMN_NAME_Variables "], ?)"
        "       )";

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Nodes query: `%s`", query));

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare nodes statement");

    int bindingIndex = 1;
    stmt->BindText(bindingIndex++, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<FilteredNodesProvider> FilteredNodesProvider::Create(NavNodesProviderContextR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements, BeMutex& cacheMutex, Utf8String filter)
    {
    filter.ReplaceAll("\\", "\\\\");
    filter.ReplaceAll("%", "\\%");
    filter.ReplaceAll("_", "\\_");
    return CallOnCreated(*new FilteredNodesProvider(context, cache, statements, cacheMutex, Utf8String("%").append(filter).append("%")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr FilteredNodesProvider::_GetUsedVariablesStatement() const
    {
    static Utf8CP query =
        "SELECT [dsv].[Variables] "
        "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds"
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId]"
        "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [ds].[HierarchyLevelId] "
        "  JOIN [" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [hl].[RulesetId] "
        " WHERE [r].[Identifier] = ? "
        "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?)";

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Used variables query: `%s`", query));

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare used variables statement");

    int bindingIndex = 1;
    stmt->BindText(bindingIndex++, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);

    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr FilteredNodesProvider::_GetResultInstanceNodesClassIdsStatement() const
    {
    static Utf8CP query =
        "SELECT DISTINCT [ni].[ECClassId] "
        "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds"
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId]"
        "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [ds].[HierarchyLevelId] "
        "  JOIN [" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [hl].[RulesetId] "
        "  JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn ON [dsn].[DataSourceId] = [ds].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[Id] = [dsn].[NodeId] "
        "  JOIN [" NODESCACHE_TABLENAME_NodeInstances "] ni on [ni].[NodeId] = [n].[Id]"
        " WHERE [r].[Identifier] = ? "
        "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?)";

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Result instance node class IDs query: `%s`", query));

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare instance node class IDs statement");

    int bindingIndex = 1;
    stmt->BindText(bindingIndex++, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);

    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr FilteredNodesProvider::_GetCountStatement() const
    {
    static Utf8CP query =
        WITH_HIDDEN_NODES
        "SELECT COUNT(1) "
        "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
        "  JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn ON [dsn].[NodeId] = [n].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [dsn].[DataSourceId] "
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId]"
        "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [ds].[HierarchyLevelId] "
        "  JOIN [" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [hl].[RulesetId] "
        " WHERE [r].[Identifier] = ? "
        "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) "
        "       AND [dsn].[Visibility] = ? "
        "       AND [n].[Label] LIKE ? ESCAPE \'\\\' "
        "       AND NOT EXISTS ("
        "           SELECT 1 "
        "             FROM [" HIDDEN_NODES_TABLE_NAME "]"
        "            WHERE [" HIDDEN_NODES_COLUMN_NAME_NodeId "] = [n].[Id]"
        "                  AND " NODESCACHE_FUNCNAME_VariablesMatch "([" HIDDEN_NODES_COLUMN_NAME_Variables "], ?)"
        "       )";

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Nodes count query: `%s`", query));

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare nodes count statement");

    int bindingIndex = 1;
    stmt->BindText(bindingIndex++, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
    stmt->BindInt(bindingIndex++, (int)NodeVisibility::Visible);
    stmt->BindText(bindingIndex++, m_filter.c_str(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);

    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr FilteredNodesProvider::_GetNodesStatement() const
    {
    static Utf8CP query =
        WITH_HIDDEN_NODES
        "SELECT " NODE_SELECT_STMT("hl", "n", "nk")
        "  FROM [" NODESCACHE_TABLENAME_Nodes "] n "
        "  JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn ON [dsn].[NodeId] = [n].[Id] "
        "  JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[Id] = [dsn].[DataSourceId] "
        "  JOIN [" NODESCACHE_TABLENAME_Variables "] dsv ON [dsv].[Id] = [ds].[VariablesId]"
        "  JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] hl ON [hl].[Id] = [ds].[HierarchyLevelId] "
        "  JOIN [" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [hl].[RulesetId] "
        " WHERE [r].[Identifier] = ? "
        "       AND " NODESCACHE_FUNCNAME_VariablesMatch "([dsv].[Variables], ?) "
        "       AND [dsn].[Visibility] = ? "
        "       AND [n].[Label] LIKE ? ESCAPE \'\\\'"
        "       AND NOT EXISTS ("
        "           SELECT 1 "
        "             FROM [" HIDDEN_NODES_TABLE_NAME "]"
        "            WHERE [" HIDDEN_NODES_COLUMN_NAME_NodeId "] = [n].[Id]"
        "                  AND " NODESCACHE_FUNCNAME_VariablesMatch "([" HIDDEN_NODES_COLUMN_NAME_Variables "], ?)"
        "       )";

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Nodes query: `%s`", query));

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Failed to prepare nodes statement");

    int bindingIndex = 1;
    stmt->BindText(bindingIndex++, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);
    stmt->BindInt(bindingIndex++, (int)NodeVisibility::Visible);
    stmt->BindText(bindingIndex++, m_filter.c_str(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetRulesetVariables().GetSerializedInternalJsonObjectString(), Statement::MakeCopy::No);

    return stmt;
    }
