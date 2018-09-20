/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/NavNodeProviders.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
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
DataSourceRelatedSettingsUpdater::DataSourceRelatedSettingsUpdater(DataSourceInfo const& info, NavNodesProviderContextCR context)
    : m_datasourceInfo(info), m_context(context)
    {
    m_relatedSettingsCountBefore = m_context.GetRelatedSettings().size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceRelatedSettingsUpdater::~DataSourceRelatedSettingsUpdater()
    {
    bvector<UserSettingEntry> settings = m_context.GetRelatedSettings();
    if (settings.size() != m_relatedSettingsCountBefore)
        m_context.GetNodesCache().Update(m_datasourceInfo, nullptr, nullptr, &settings);
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
    m_isNodesCheck = false;
    m_isNodesCount = false;
    m_isFullLoadDisabled = false;
    m_baseProvider = nullptr;
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IHierarchyCacheR NavNodesProviderContext::GetNodesCache() const {return *m_nodesCache;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IUsedUserSettingsListener& NavNodesProviderContext::GetUsedSettingsListener() const
    {
    NavNodesProviderContextCP rootBaseProviderContext = this;
    while (nullptr != rootBaseProviderContext->GetBaseProvider())
        rootBaseProviderContext = &rootBaseProviderContext->GetBaseProvider()->GetContext();

    if (rootBaseProviderContext != this)
        return rootBaseProviderContext->GetUsedSettingsListener();
    return RulesDrivenProviderContext::GetUsedSettingsListener();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<UserSettingEntry> NavNodesProviderContext::GetRelatedSettings() const
    {
    NavNodesProviderContextCP rootBaseProviderContext = this;
    while (nullptr != rootBaseProviderContext->GetBaseProvider())
        rootBaseProviderContext = &rootBaseProviderContext->GetBaseProvider()->GetContext();

    if (rootBaseProviderContext != this)
        return rootBaseProviderContext->GetRelatedSettings();

    bvector<Utf8String> ids = RulesDrivenProviderContext::GetRelatedSettingIds();
    bvector<UserSettingEntry> idsWithValues;
    for (Utf8StringCR id : ids)
        idsWithValues.push_back(UserSettingEntry(id, GetUserSettings().GetSettingValueAsJson(id.c_str())));

    NavNodeCPtr physicalParentNode = GetPhysicalParentNode();
    if (physicalParentNode.IsValid() && NavNodesHelper::IsGroupingNode(*physicalParentNode))
        {
        // note: if parent node is a grouping node, we want to append all its related settings
        // because it may have derived them from it's virtual parent
        NavNodesProviderPtr provider = GetNodesCache().GetDataSource(physicalParentNode->GetNodeId(), false);
        if (provider.IsValid())
            {
            bvector<UserSettingEntry> parentSettings = provider->GetContext().GetRelatedSettings();
            std::move(parentSettings.begin(), parentSettings.end(), std::inserter(idsWithValues, idsWithValues.end()));
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
NavNodesProviderPtr NavNodesProviderContext::CreateProvider(NavNodesProviderContextR context, JsonNavNodeCP parentNode) const
    {
    return m_providerFactory.CreateForVirtualParent(context, parentNode);
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
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodesProviderContext::IsNodesCount(bool checkBase) const
    {
    return m_isNodesCount || (checkBase && nullptr != GetBaseProvider() && GetBaseProvider()->GetContext().IsNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodesProviderContext::IsNodesCheck(bool checkBase) const
    {
    return m_isNodesCheck || (checkBase && nullptr != GetBaseProvider() && GetBaseProvider()->GetContext().IsNodesCheck());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodesProviderContext::IsFullNodesLoadDisabled(bool checkBase) const
    {
    return m_isFullLoadDisabled || (checkBase && nullptr != GetBaseProvider() && GetBaseProvider()->GetContext().IsFullNodesLoadDisabled());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodesProviderContext::IsUpdatesDisabled(bool checkBase) const
    {
    return m_isUpdatesDisabled || (checkBase && nullptr != GetBaseProvider() && GetBaseProvider()->GetContext().IsUpdatesDisabled());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetLocale(RulesDrivenProviderContextCR context)
    {
    return context.IsLocalizationContext() ? context.GetLocale() : "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static DataSourceInfo CreateDataSourceInfo(NavNodesProviderContextCR ctx)
    {
    return DataSourceInfo(ctx.GetConnection().GetId(), ctx.GetRuleset().GetRuleSetId(),
        ctx.IsLocalizationContext() ? ctx.GetLocale() : "",
        ctx.GetPhysicalParentNodeId(), ctx.GetVirtualParentNodeId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProvider::NavNodesProvider(NavNodesProviderContextCR context) 
    : m_context(const_cast<NavNodesProviderContext*>(&context))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static NavNodesProviderContextPtr CreateContextForNestedProvider(NavNodesProviderCR baseProvider, NavNodeCR virtualParent)
    {
    NavNodesProviderContextPtr ctx = NavNodesProviderContext::Create(baseProvider.GetContext());
    ctx->SetBaseProvider(baseProvider);

    if (baseProvider.GetContext().IsRootNodeContext())
        ctx->SetRootNodeContext(baseProvider.GetContext().GetRootNodeRule());
    else if (baseProvider.GetContext().IsChildNodeContext())
        ctx->SetChildNodeContext(baseProvider.GetContext().GetChildNodeRule(), virtualParent);

    return ctx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static NavNodesProviderContextPtr CreateContextForNestedProvider(NavNodesProviderCR baseProvider, bool copyNodesContext = true)
    {
    NavNodesProviderContextPtr ctx = NavNodesProviderContext::Create(baseProvider.GetContext());
    ctx->SetBaseProvider(baseProvider);

    if (!copyNodesContext)
        return ctx;

    if (baseProvider.GetContext().IsRootNodeContext())
        ctx->SetRootNodeContext(baseProvider.GetContext());
    else if (baseProvider.GetContext().IsChildNodeContext())
        ctx->SetChildNodeContext(baseProvider.GetContext());
    return ctx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaikšsnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
EmptyNavNodesProvider::EmptyNavNodesProvider(NavNodesProviderContextCR context)
    : NavNodesProvider(context)
    {
    SetDataSourceInfo(CreateDataSourceInfo(context));
    context.GetNodesCache().Cache(GetDataSourceInfo(), DataSourceFilter(), bmap<ECClassId, bool>(), 
        context.GetRelatedSettings(), context.IsUpdatesDisabled());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodesProvider::GetNode(JsonNavNodePtr& node, size_t index) const {return _GetNode(node, index);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodesProvider::HasNodes() const
    {
    Holder<NodesCheckContext> ctx;
    if (!GetContext().IsNodesCheck())
        ctx = *new NodesCheckContext(*this);

    return _HasNodes();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t NavNodesProvider::GetNodesCount() const
    {
    Holder<NodesCountContext> ctx;
    if (!GetContext().IsNodesCount())
        ctx = *new NodesCountContext(*this);

    return _GetNodesCount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProvider::NotifyNodeChanged(JsonNavNodeCR node) const
    {
    GetContext().GetNodesCache().Update(node.GetNodeId(), node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static NavNodesProviderContextPtr CreateChildrenContext(NavNodesProviderContextCR parentContext, NavNodeCR parentNode, bool isParentPhysical)
    {
    NavNodesProviderContextPtr context = NavNodesProviderContext::Create(parentContext);
    context->SetVirtualParentNode(parentNode);
    if (isParentPhysical)
        context->SetPhysicalParentNode(parentNode);
    return context;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
HasChildrenFlag NavNodesProvider::AnyChildSpecificationReturnsNodes(JsonNavNode const& parentNode, bool isParentPhysical) const
    {
    NavNodesProviderContextPtr context = CreateChildrenContext(GetContext(), parentNode, isParentPhysical);
    NavNodesProviderPtr childrenProvider = GetContext().CreateProvider(*context, &parentNode);
    NodesCheckContext checkContext(*childrenProvider);
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
    if (extendedData.GetAlwaysReturnsChildren())
        return;
    
    node.SetHasChildren(HASCHILDREN_True == AnyChildSpecificationReturnsNodes(node, true));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NavNodesProvider::FinalizeNode(JsonNavNodeR node, bool customizeLabel) const
    {
    if (!GetContext().NeedsFullLoad())
        return;

    DataSourceRelatedSettingsUpdater updater(GetDataSourceInfo(), GetContext());
    bool changed = false;        

    // make sure the node is customized
    if (!NavNodeExtendedData(node).IsCustomized())
        {
        CustomizationHelper::Customize(GetRootBaseProvider().GetContext(), node, customizeLabel);
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
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderCR NavNodesProvider::GetRootBaseProvider() const
    {
    NavNodesProviderCP rootBaseProvider = this;
    while (nullptr != rootBaseProvider->GetContext().GetBaseProvider())
        rootBaseProvider = rootBaseProvider->GetContext().GetBaseProvider();
    return *rootBaseProvider;
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
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CustomNodesProvider::CustomNodesProvider(NavNodesProviderContextCR context, CustomNodeSpecificationCR specification) 
    : NavNodesProvider(context), m_specification(specification)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodesProvider::Initialize()
    {
    if (!GetDataSourceInfo().IsValid())
        {
        SetDataSourceInfo(CreateDataSourceInfo(GetContext()));
        GetContext().GetNodesCache().Cache(GetDataSourceInfo(), DataSourceFilter(), bmap<ECClassId, bool>(), GetContext().GetRelatedSettings(), GetContext().IsUpdatesDisabled());

        if (m_specification.GetNodeType().empty() || m_specification.GetLabel().empty())
            {
            LoggingHelper::LogMessage(Log::Navigation, "Type and Label are required properties for CustomNode specification");
            BeAssert(false);
            return;
            }

        Utf8String connectionId = GetContext().IsQueryContext() ? GetContext().GetConnection().GetId() : "";
        Utf8String type(m_specification.GetNodeType().c_str());
        Utf8String imageId(m_specification.GetImageId().c_str());
        Utf8String label(m_specification.GetLabel().c_str());
        Utf8String description(m_specification.GetDescription().c_str());
        m_node = GetContext().GetNodesFactory().CreateCustomNode(connectionId, GetLocale(GetContext()), label.c_str(), description.c_str(), imageId.c_str(), type.c_str());

        NavNodeExtendedData extendedData(*m_node);
        extendedData.SetRulesetId(GetContext().GetRuleset().GetRuleSetId().c_str());
        extendedData.SetSpecificationHash(m_specification.GetHash());
        if (m_specification.GetAlwaysReturnsChildren())
            extendedData.SetAlwaysReturnsChildren(true);

        if (nullptr != GetContext().GetPhysicalParentNodeId())
            m_node->SetParentNodeId(*GetContext().GetPhysicalParentNodeId());
        if (nullptr != GetContext().GetVirtualParentNodeId())
            extendedData.SetVirtualParentId(*GetContext().GetVirtualParentNodeId());
        if (GetContext().IsRootNodeContext() && GetContext().GetRootNodeRule().GetAutoExpand())
            m_node->SetIsExpanded(true);

        GetContext().GetNodesCache().Cache(*m_node, true);

        if (m_specification.GetHideNodesInHierarchy())
            {
            NavNodesProviderContextPtr childrenContext = CreateContextForNestedProvider(*this, false);
            m_childNodesProvider = GetContext().CreateProvider(*childrenContext, m_node.get());
            return;
            }
    
        HasChildrenFlag hasChildren = HASCHILDREN_Unknown;
        if (m_specification.GetAlwaysReturnsChildren())
            {
            hasChildren = HASCHILDREN_True;
            }
        else if (m_specification.GetHideIfNoChildren() && (HASCHILDREN_False == (hasChildren = AnyChildSpecificationReturnsNodes(*m_node, false))))
            {
            // if the node has "hide if no children" flag and none of the child specs return nodes, return false
            m_node = nullptr;
            return;
            }

        // we may already know whether the node has any children
        if (HASCHILDREN_Unknown != hasChildren)
            {
            m_node->SetHasChildren(HASCHILDREN_True == hasChildren);
            NotifyNodeChanged(*m_node);
            }

        GetContext().GetNodesCache().MakePhysical(*m_node);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomNodesProvider::_GetNode(JsonNavNodePtr& node, size_t index) const
    {
    const_cast<CustomNodesProvider*>(this)->Initialize();
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
    const_cast<CustomNodesProvider*>(this)->Initialize();
    return m_childNodesProvider.IsValid() ? m_childNodesProvider->HasNodes() : m_node.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CustomNodesProvider::_GetNodesCount() const
    {
    const_cast<CustomNodesProvider*>(this)->Initialize();
    if (m_childNodesProvider.IsValid())
        return m_childNodesProvider->GetNodesCount();
    return m_node.IsValid() ? 1 : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
QueryBasedNodesProvider::QueryBasedNodesProvider(NavNodesProviderContextCR context, NavigationQuery const& query, bmap<ECClassId, bool> const& usedClassIds) 
    : MultiNavNodesProvider(context), m_query(&query), m_executor(context.GetNodesFactory(), context.GetConnection(), GetLocale(context), context.GetStatementCache(), query), 
    m_executorIndex(0), m_inProvidersRequest(false), m_usedClassIds(usedClassIds)
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

    // when the node is created using a specification with "hide nodes in hierarchy flag", return its children instead
    if (extendedData.HideNodesInHierarchy())
        return true;

    // if the node has "hide if grouping value not specified" flag and is a value property grouping node, grouping null values, show its chilren
    if (extendedData.HideIfGroupingValueNotSpecified() && node.GetType().Equals(NAVNODE_TYPE_ECPropertyGroupingNode) && extendedData.HasPropertyValue() && extendedData.GetPropertyValue()->IsNull())
        return true;

    // if the node has only one child and also has "hide if only one child" flag, we want to display that child
    if (extendedData.HideIfOnlyOneChild())
        {
        NavNodesProviderPtr childrenProvider = GetContext().CreateProvider(*CreateChildrenContext(GetContext(), node, false), &node);
        NodesCountContext countCtx(*childrenProvider);

        size_t childrenCount = childrenProvider->GetNodesCount();
        hasChildren = (childrenCount > 0) ? HASCHILDREN_True : HASCHILDREN_False;
        
        if (childrenCount == 1)
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Pranciskus.Ambrazas             07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryBasedNodesProvider::HasSimilarNodeInHierarchy(JsonNavNodeCR node, uint64_t parentNodeId) const
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
        || HasSimilarNodeInHierarchy(node, parentNode->GetParentNodeId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr QueryBasedNodesProvider::CreateProvider(JsonNavNodeR node) const
    {
    // the specification may want to return nodes children instead of the node itself
    HasChildrenFlag hasChildren = HASCHILDREN_Unknown;
    if (ShouldReturnChildNodes(node, hasChildren))
        return GetContext().CreateProvider(*CreateContextForNestedProvider(*this, false), &node);
    
    NavNodesProviderContextPtr nestedContext = CreateContextForNestedProvider(*this, node);
    if (GetContext().IsChildNodeContext() && HasSimilarNodeInHierarchy(node, node.GetParentNodeId()))
        return EmptyNavNodesProvider::Create(*nestedContext);

    // there's some additional work if we don't know if the node has children
    NavNodeExtendedData extendedData(node);
    if (extendedData.GetAlwaysReturnsChildren())
        {
        hasChildren = HASCHILDREN_True;
        }
    else if (extendedData.HideIfNoChildren() && (HASCHILDREN_False == hasChildren || HASCHILDREN_False == (hasChildren = AnyChildSpecificationReturnsNodes(node, false))))
        {
        // if the node has "hide if no children" flag and none of the child specs return nodes, skip this node
        return EmptyNavNodesProvider::Create(*nestedContext);
        }

    // we may already know whether the node has any children
    if (HASCHILDREN_Unknown != hasChildren)
        {
        node.SetHasChildren(HASCHILDREN_True == hasChildren);
        NotifyNodeChanged(node);
        }

    GetContext().GetNodesCache().MakePhysical(node);
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
        m_provider.GetDataSourceInfo().SetDataSourceId(0);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBasedNodesProvider::Initialize()
    {
    if (GetDataSourceInfo().IsValid())
        return;

    RefCountedPtr<PerformanceLogger> _l = LoggingHelper::CreatePerformanceLogger(Log::Navigation, "[QueryBasedNodesProvider] Initialize", NativeLogging::LOG_TRACE);
    JsonNavNodeCPtr virtualParent = GetContext().GetVirtualParentNode();

    // create a savepoint so we can roll back if canceled
    Savepoint savepoint(*this);

    // cache data source before getting the nodes
    SetDataSourceInfo(CreateDataSourceInfo(GetContext()));
    DataSourceFilter dsFilter(GetSpecificationFilter(GetContext().GetNodesCache(), virtualParent.get(), *m_query));
    GetContext().GetNodesCache().Cache(GetDataSourceInfo(), dsFilter, m_usedClassIds, GetContext().GetRelatedSettings(), GetContext().IsUpdatesDisabled());
    DataSourceRelatedSettingsUpdater updater(GetDataSourceInfo(), GetContext());

    // set up the custom functions context
    CustomFunctionsContext fnContext(GetContext().GetSchemaHelper(), GetContext().GetConnections(), GetContext().GetConnection(), GetContext().GetRuleset(), 
        GetContext().GetLocale(), GetContext().GetUserSettings(), &GetContext().GetUsedSettingsListener(), GetContext().GetECExpressionsCache(), 
        GetContext().GetNodesFactory(), GetContext().GetUsedClassesListener(), virtualParent.get(), &m_query->GetExtendedData());
    if (GetContext().IsLocalizationContext())
        fnContext.SetLocalizationProvider(GetContext().GetLocalizationProvider());

    // read the nodes
    m_executor.ReadRecords(&GetContext().GetCancelationToken());

    // create providers for each node
    JsonNavNodePtr node;
    GetNodeProvidersR().reserve(m_executor.GetNodesCount());
    while ((node = m_executor.GetNode(m_executorIndex++)).IsValid())
        {
        NavNodeExtendedData extendedData(*node);
        extendedData.SetRulesetId(GetContext().GetRuleset().GetRuleSetId().c_str());
        if (nullptr != GetContext().GetPhysicalParentNodeId())
            node->SetParentNodeId(*GetContext().GetPhysicalParentNodeId());
        if (nullptr != GetContext().GetVirtualParentNodeId())
            extendedData.SetVirtualParentId(*GetContext().GetVirtualParentNodeId());        
        if (GetContext().IsRootNodeContext() && GetContext().GetRootNodeRule().GetAutoExpand())
            node->SetIsExpanded(true);

        GetContext().GetNodesCache().Cache(*node, true);
        
        if (GetContext().GetCancelationToken().IsCanceled())
            {
            savepoint.Cancel();
            LoggingHelper::LogMessage(Log::Navigation, "[QueryBasedNodesProvider] Initialization canceled", NativeLogging::LOG_DEBUG);
            return;
            }

        NavNodesProviderPtr provider = CreateProvider(*node);
        AddProvider(*provider);
        }

    LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[QueryBasedNodesProvider] Created node providers for %" PRIu64 " nodes", (uint64_t)m_executorIndex).c_str(), NativeLogging::LOG_DEBUG);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryBasedNodesProvider::_GetNode(JsonNavNodePtr& node, size_t index) const
    {
    const_cast<QueryBasedNodesProvider*>(this)->Initialize();
    if (!MultiNavNodesProvider::_GetNode(node, index))
        return false;

    FinalizeNode(*node, false);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryBasedNodesProvider::_HasNodes() const
    {
    if (m_query->GetResultParameters().GetNavNodeExtendedData().GetAlwaysReturnsChildren())
        return true;
    
    return GetNodesCount() > 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t QueryBasedNodesProvider::_GetNodesCount() const
    {
    const_cast<QueryBasedNodesProvider*>(this)->Initialize();
    return MultiNavNodesProvider::_GetNodesCount();
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

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct ProvidersRequestContext
    {
    bool m_prevFlag;
    bool& m_flag;
    ProvidersRequestContext(bool& flag) : m_prevFlag(flag), m_flag(flag) {flag = true;}
    ~ProvidersRequestContext() {m_flag = m_prevFlag;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBasedNodesProvider::_OnNodeProvidersRequest() const
    {
    if (m_inProvidersRequest)
        return;

    ProvidersRequestContext providersRequestContext(m_inProvidersRequest);
    const_cast<QueryBasedNodesProvider*>(this)->Initialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
QueryBasedSpecificationNodesProvider::QueryBasedSpecificationNodesProvider(NavNodesProviderContextCR context, ChildNodeSpecificationCR specification) 
    : MultiNavNodesProvider(context), m_specification(specification)
    {
    SetupNestedProviders(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBasedSpecificationNodesProvider::SetupNestedProviders(bool fresh)
    {
    SpecificationUsedClassesListener usedClasses(GetContextR());
    bvector<NavigationQueryPtr> queries = CreateQueries(m_specification);
    if (fresh)
        {
        NavNodesProviderContextPtr nestedContext = CreateContextForNestedProvider(*this);
        ClearProviders();
        for (NavigationQueryPtr const& query : queries)
            AddProvider(*QueryBasedNodesProvider::Create(*nestedContext, *query, usedClasses.GetUsedClassIds()));
        }
    else
        {
        BeAssert(queries.size() == GetNodeProviders().size());
        for (size_t i = 0; i < queries.size(); i++)
            {
            QueryBasedNodesProvider* provider = static_cast<QueryBasedNodesProvider*>(GetNodeProvidersR()[i].get());
            provider->SetQuery(*queries[i], usedClasses.GetUsedClassIds());
            }
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryBasedSpecificationNodesProvider::_HasNodes() const
    {
    if (m_specification.GetAlwaysReturnsChildren())
        return true;

    return MultiNavNodesProvider::_HasNodes();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MultiSpecificationNodesProvider::MultiSpecificationNodesProvider(NavNodesProviderContextR context, RootNodeRuleSpecificationsList const& specs)
    : MultiNavNodesProvider(context)
    {
    SetDataSourceInfo(CreateDataSourceInfo(context));
    GetContext().GetNodesCache().Cache(GetDataSourceInfo(), DataSourceFilter(), bmap<ECClassId, bool>(), context.GetRelatedSettings(), context.IsUpdatesDisabled());

    SpecificationsVisitor visitor;
    for (RootNodeRuleSpecification const& specification : specs)
        {
        NavNodesProviderContextPtr nestedContext = CreateContextForNestedProvider(*this, false);
        nestedContext->SetRootNodeContext(specification.GetRule());
        visitor.SetContext(*nestedContext);
        specification.GetSpecification().Accept(visitor);
        }
    SetProviders(visitor.GetNodeProviders());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MultiSpecificationNodesProvider::MultiSpecificationNodesProvider(NavNodesProviderContextR context, ChildNodeRuleSpecificationsList const& specs, NavNodeCR virtualParent)
    : MultiNavNodesProvider(context)
    {
    SpecificationsVisitor visitor;
    for (ChildNodeRuleSpecification const& specification : specs)
        {
        NavNodesProviderContextPtr nestedContext = CreateContextForNestedProvider(*this, false);
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
            m_replaceProvider = GetContext().CreateProvider(*CreateContextForNestedProvider(*this, false), node.get());
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
    if (m_replaceProvider.IsValid())
        return m_replaceProvider->GetNodesCount();

    size_t count = MultiNavNodesProvider::_GetNodesCount();
    JsonNavNodePtr node;
    if (1 == count && MultiNavNodesProvider::_GetNode(node, 0) && node->GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode))
        {
        GetContext().GetNodesCache().MakeVirtual(*node);
        m_replaceProvider = GetContext().CreateProvider(*CreateContextForNestedProvider(*this, false), node.get());
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
    bool hasNodes = false;
    for (NavNodesProviderPtr const& provider : m_providers)
        {
        if (provider.IsNull())
            continue;

        if (provider->HasNodes())
            {
            // note: we could immediately return "true" here, but the current implementation expects either 
            // no or all providers to be initialized. if only some of providers get initialized and we query
            // this provider from cache, the cached one only contains the initialized provider nodes.
            hasNodes = true;
            }
        }
    return hasNodes;
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

        NodesCountContext ctx(*provider);
        count += provider->GetNodesCount();
        }
    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodesProvider::GetPositionOffset(size_t& offset, NavNodesProviderCR provider) const
    {
    offset = 0;
    return GetPositionOffsetInternal(offset, GetRootBaseProvider(), provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavNodesProvider::GetPositionOffsetInternal(size_t& offset, NavNodesProviderCR base, NavNodesProviderCR provider) const
    {
    MultiNavNodesProviderCP multi = base.AsMultiProvider();
    if (nullptr == multi)
        return false;

    for (NavNodesProviderPtr const& subProvider : multi->GetNodeProviders())
        {
        if (subProvider.IsNull())
            continue;

        if (subProvider.get() == &provider)
            return true;
        
        if (nullptr != subProvider->AsMultiProvider())
            {
            size_t subOffset = 0;
            if (subProvider->AsMultiProvider()->GetPositionOffsetInternal(subOffset, *subProvider, provider))
                {
                offset += subOffset;
                return true;
                }
            offset += subOffset;
            }
        else
            {
            offset += subProvider->GetNodesCount();
            }
        }

    return false;
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
                       "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
                       "  JOIN [" NODESCACHE_TABLENAME_DataSourceSettings "] us ON [us].[DataSourceId] = [ds].[Id]"
                       " WHERE [ds].[VirtualParentNodeId] ";
    if (0 == GetContext().GetVirtualParentNodeId())
        query.append("IS NULL");
    else
        query.append(" = ?");
    query.append(" AND [ds].[ConnectionId] = ?");
    query.append(" AND [ds].[RulesetId] = ?");

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

        node->SetNodeId(statement->GetValueUInt64(2));
        node->SetIsExpanded(!statement->IsColumnNull(3));

        if (!statement->IsColumnNull(1))
            NavNodeExtendedData(*node).SetVirtualParentId(statement->GetValueUInt64(1));

        node->SetNodeKey(*NavNodesHelper::CreateNodeKey(GetContext().GetConnection(), *node, statement->GetValueText(4)));

        m_nodes->push_back(node);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool SQLiteCacheNodesProvider::_GetNode(JsonNavNodePtr& node, size_t index) const
    {
    const_cast<SQLiteCacheNodesProvider*>(this)->InitializeNodes();
    if (index > m_nodes->size() - 1)
        {
        BeAssert(false);
        return false;
        }

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
CachedHierarchyLevelProvider::CachedHierarchyLevelProvider(NavNodesProviderContextCR context, Db& cache, StatementCache& statements, uint64_t const* physicalParentNodeId)
    : SQLiteCacheNodesProvider(context, cache, statements)
    {
    InitDatasourceIds(physicalParentNodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedHierarchyLevelProvider::~CachedHierarchyLevelProvider() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CachedHierarchyLevelProvider::InitDatasourceIds(uint64_t const* physicalParentNodeId)
    {
    Utf8String query = "SELECT [Id] "
                        "  FROM [" NODESCACHE_TABLENAME_DataSources "] "
                        " WHERE [PhysicalParentNodeId] ";
    if (nullptr == physicalParentNodeId || 0 == *physicalParentNodeId)
        query.append("IS NULL");
    else
        query.append(" = ?");
    query.append(" AND [ConnectionId] = ?");
    query.append(" AND [RulesetId] = ?");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return;
        }

    int bindingIndex = 1;
    if (nullptr != physicalParentNodeId && 0 != *physicalParentNodeId)
        stmt->BindUInt64(bindingIndex++, *physicalParentNodeId);
    stmt->BindText(bindingIndex++, GetContext().GetConnection().GetId(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);

    m_datasourceIds.clear();
    while (BE_SQLITE_ROW == stmt->Step())
        {
        if (!m_datasourceIds.empty())
            m_datasourceIds.append(",");
        m_datasourceIds.append(std::to_string(stmt->GetValueUInt64(0)).c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedHierarchyLevelProvider::_GetNodesStatement() const
    {
    Utf8String query = "SELECT [n].[Data], [ds].[VirtualParentNodeId], [n].[Id], [ex].[NodeId], [nk].[PathFromRoot] "
                        "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
                        "  LEFT JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id]"
                        "  LEFT JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id]"
                        "  LEFT JOIN [" NODESCACHE_TABLENAME_ExpandedNodes "] ex ON [n].[Id] = [ex].[NodeId]"
                        " WHERE NOT [n].[IsVirtual] ";
    query.append("AND [ds].[Id] IN (").append(m_datasourceIds).append(") ");
    query.append("ORDER BY [n].[ROWID]");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    return stmt;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedHierarchyLevelProvider::_GetCountStatement() const
    {
    Utf8String query = "SELECT COUNT(1) "
                        "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
                        "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id]"
                        " WHERE NOT [n].[IsVirtual] ";
    query.append("AND [ds].[Id] IN (").append(m_datasourceIds).append(") ");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }
    
    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedVirtualNodeChildrenProvider::CachedVirtualNodeChildrenProvider(NavNodesProviderContextCR context, DataSourceInfo dsInfo, Db& cache, StatementCache& statements)
    : SQLiteCacheNodesProvider(context, cache, statements)
    {
    SetDataSourceInfo(dsInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedVirtualNodeChildrenProvider::_GetNodesStatement() const
    {
    Utf8String query = "SELECT [n].[Data], [ds].[VirtualParentNodeId], [n].[Id], [ex].[NodeId], [nk].[PathFromRoot] "
                        "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
                        "  LEFT JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id]"
                        "  LEFT JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id]"
                        "  LEFT JOIN [" NODESCACHE_TABLENAME_ExpandedNodes "] ex ON [n].[Id] = [ex].[NodeId]"
                        " WHERE NOT [n].[IsVirtual] "
                        "       AND [ds].[VirtualParentNodeId] ";
    if (0 == GetContext().GetPhysicalParentNodeId())
        query.append("IS NULL");
    else
        query.append(" = ?");
    query.append(" AND [ds].[ConnectionId] = ?");
    query.append(" AND [ds].[RulesetId] = ?");
    query.append(" ORDER BY [n].[ROWID]");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    int bindingIndex = 1;
    if (nullptr != GetContext().GetPhysicalParentNodeId())
        stmt->BindUInt64(bindingIndex++, *GetContext().GetPhysicalParentNodeId());
    stmt->BindText(bindingIndex++, GetContext().GetConnection().GetId(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);

    return stmt;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr CachedVirtualNodeChildrenProvider::_GetCountStatement() const
    {
    Utf8String query = "SELECT COUNT(1) "
                        "  FROM [" NODESCACHE_TABLENAME_DataSources "] ds "
                        "  JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [ds].[Id]"
                        " WHERE NOT [n].[IsVirtual]"
                        "       AND [ds].[VirtualParentNodeId] ";
    if (0 == GetContext().GetPhysicalParentNodeId())
        query.append("IS NULL");
    else
        query.append(" = ?");
    query.append(" AND [ds].[ConnectionId] = ?");
    query.append(" AND [ds].[RulesetId] = ?");

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }
    
    int bindingIndex = 1;
    if (nullptr != GetContext().GetPhysicalParentNodeId())
        stmt->BindUInt64(bindingIndex++, *GetContext().GetPhysicalParentNodeId());
    stmt->BindText(bindingIndex++, GetContext().GetConnection().GetId(), Statement::MakeCopy::No);
    stmt->BindText(bindingIndex++, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);

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
                       "     FROM [" NODESCACHE_TABLENAME_DataSources "] dsn "
                       "     JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [dsn].[Id]"
                       "LEFT JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[VirtualParentNodeId] = [n].[Id]"       
                       "    WHERE [ds].[VirtualParentNodeId] IS NULL AND [dsn].[ConnectionId] = ? AND [dsn].[RulesetId] = ?"; 

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindText(1, GetContext().GetConnection().GetId(), Statement::MakeCopy::No);
    stmt->BindText(2, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr NodesWithUndeterminedChildrenProvider::_GetNodesStatement() const
    {
    Utf8String query = "   SELECT [n].[Data], [dsn].[VirtualParentNodeId], [n].[Id], [ex].[NodeId], [nk].[PathFromRoot] "
                       "     FROM [" NODESCACHE_TABLENAME_DataSources "] dsn "
                       "     JOIN [" NODESCACHE_TABLENAME_Nodes "] n ON [n].[DataSourceId] = [dsn].[Id]"
                       "LEFT JOIN [" NODESCACHE_TABLENAME_NodeKeys "] nk ON [nk].[NodeId] = [n].[Id]"
                       "LEFT JOIN [" NODESCACHE_TABLENAME_ExpandedNodes "] ex ON [n].[Id] = [ex].[NodeId]"
                       "LEFT JOIN [" NODESCACHE_TABLENAME_DataSources "] ds ON [ds].[VirtualParentNodeId] = [n].[Id]"       
                       "    WHERE [ds].[VirtualParentNodeId] IS NULL AND [dsn].[ConnectionId] = ? AND [dsn].[RulesetId] = ?"; 

    CachedStatementPtr stmt;
    if (BE_SQLITE_OK != GetStatements().GetPreparedStatement(stmt, *GetCache().GetDbFile(), query.c_str()))
        {
        BeAssert(false);
        return nullptr;
        }
    
    stmt->BindText(1, GetContext().GetConnection().GetId(), Statement::MakeCopy::No);
    stmt->BindText(2, GetContext().GetRuleset().GetRuleSetId().c_str(), Statement::MakeCopy::No);
    return stmt;
    }
