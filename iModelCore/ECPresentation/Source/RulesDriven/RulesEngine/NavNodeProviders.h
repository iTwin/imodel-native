/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "RulesDrivenProviderContext.h"
#include "QueryExecutor.h"
#include "RulesPreprocessor.h"
#include "DataSourceInfo.h"
#include "UsedClassesListener.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

struct ECDbUsedClassesListenerWrapper;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2019
+===============+===============+===============+===============+===============+======*/
struct IProvidersIndexAllocator : RefCountedBase
{
protected:
    virtual bvector<uint64_t> _GetCurrentIndex() const = 0;
    virtual bvector<uint64_t> _AllocateIndex() = 0;
public:
    bvector<uint64_t> GetCurrentIndex() const {return _GetCurrentIndex();}
    bvector<uint64_t> AllocateIndex() {return _AllocateIndex();}
};
typedef RefCountedPtr<IProvidersIndexAllocator> IProvidersIndexAllocatorPtr;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2019
+===============+===============+===============+===============+===============+======*/
struct ProvidersIndexAllocator : IProvidersIndexAllocator
{
private:
    bvector<uint64_t> m_parentIndex;
    uint64_t m_currIndex;
protected:
    ECPRESENTATION_EXPORT bvector<uint64_t> _GetCurrentIndex() const override;
    ECPRESENTATION_EXPORT bvector<uint64_t> _AllocateIndex() override;
public:
    ProvidersIndexAllocator() : m_currIndex(0) {}
    ProvidersIndexAllocator(bvector<uint64_t> parentIndex) : m_parentIndex(parentIndex), m_currIndex(0) {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2019
+===============+===============+===============+===============+===============+======*/
struct ArtifactsCapturer : RefCountedBase
{
private:
    bvector<NodeArtifacts> m_artifacts;
    ArtifactsCapturer() {}
public:
    void AddArtifact(NodeArtifacts artifact) {m_artifacts.push_back(artifact);}
    bvector<NodeArtifacts> const& GetArtifacts() const {return m_artifacts;}
    static RefCountedPtr<ArtifactsCapturer> Create() {return new ArtifactsCapturer();}
};

/*=================================================================================**//**
* Context for NavNodesProvider implementations.
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct NavNodesProviderContext : RulesDrivenProviderContext
{
private:
    // common
    RuleTargetTree m_targetTree;
    IHierarchyCache* m_nodesCache;
    INodesProviderFactoryCR m_providerFactory;
    uint64_t const* m_physicalParentNodeId;
    uint64_t const* m_virtualParentNodeId;
    IProvidersIndexAllocatorPtr m_providersIndexAllocator;
    mutable HierarchyLevelInfo m_hierarchyLevelInfo;
    mutable DataSourceInfo m_dataSourceInfo;
    bset<ArtifactsCapturer*> m_artifactsCapturers;

    // optimization flags
    bool m_isFullLoadDisabled;
    bool m_isUpdatesDisabled;
    bool m_isCheckingChildren;
    bool m_hasPageSize;
    bool m_mayHaveArtifacts;
    size_t m_pageSize;

    // root nodes context
    bool m_isRootNodeContext;
    RootNodeRuleCP m_rootNodeRule;

    // child nodes context
    bool m_isChildNodeContext;
    ChildNodeRuleCP m_childNodeRule;

    // ECDb context
    mutable NavigationQueryBuilder* m_queryBuilder;
    ECDbUsedClassesListenerWrapper* m_usedClassesListener;

private:
    void Init();
    void InitProvidersIndexAllocator(uint64_t const* virtualParentNodeId);
    ECPRESENTATION_EXPORT NavNodesProviderContext(PresentationRuleSetCR, RuleTargetTree, Utf8String, uint64_t const*, std::unique_ptr<RulesetVariables>, ECExpressionsCache&,
        RelatedPathsCache&, PolymorphicallyRelatedClassesCache&, JsonNavNodesFactory const&, IHierarchyCache&, INodesProviderFactoryCR, IJsonLocalState const*);
    ECPRESENTATION_EXPORT NavNodesProviderContext(NavNodesProviderContextCR other);

public:
    static NavNodesProviderContextPtr Create(PresentationRuleSetCR ruleset, RuleTargetTree targetTree, Utf8String locale, uint64_t const* physicalParentId,
        std::unique_ptr<RulesetVariables> rulesetVariables, ECExpressionsCache& ecexpressionsCache, RelatedPathsCache& relatedPathsCache,
        PolymorphicallyRelatedClassesCache& polymorphicallyRelatedClassesCache, JsonNavNodesFactory const& nodesFactory, IHierarchyCache& nodesCache,
        INodesProviderFactoryCR providerFactory, IJsonLocalState const* localState)
        {
        return new NavNodesProviderContext(ruleset, targetTree, locale, physicalParentId, std::move(rulesetVariables), ecexpressionsCache,
            relatedPathsCache, polymorphicallyRelatedClassesCache, nodesFactory, nodesCache, providerFactory, localState);
        }
    static NavNodesProviderContextPtr Create(NavNodesProviderContextCR other) {return new NavNodesProviderContext(other);}
    ~NavNodesProviderContext();

    // common
    RuleTargetTree GetTargetTree() const {return m_targetTree;}
    ECPRESENTATION_EXPORT IHierarchyCacheR GetNodesCache() const;
    ECPRESENTATION_EXPORT JsonNavNodeCPtr GetPhysicalParentNode() const;
    uint64_t const* GetPhysicalParentNodeId() const {return m_physicalParentNodeId;}
    void SetPhysicalParentNodeId(uint64_t id) {DELETE_AND_CLEAR(m_physicalParentNodeId); m_physicalParentNodeId = (0 != id) ? new uint64_t(id) : nullptr;}
    void SetPhysicalParentNode(NavNodeCR node) {SetPhysicalParentNodeId(node.GetNodeId());}
    ECPRESENTATION_EXPORT JsonNavNodeCPtr GetVirtualParentNode() const;
    uint64_t const* GetVirtualParentNodeId() const {return m_virtualParentNodeId;}
    ECPRESENTATION_EXPORT void SetVirtualParentNodeId(uint64_t id);
    void SetVirtualParentNode(NavNodeCR node) {SetVirtualParentNodeId(node.GetNodeId());}
    NavNodesProviderPtr CreateHierarchyLevelProvider(NavNodesProviderContextR, JsonNavNodeCP parentNode) const;
    HierarchyLevelInfo const& GetHierarchyLevelInfo() const;
    DataSourceInfo const& GetDataSourceInfo() const;
    bvector<RulesetVariableEntry> GetRelatedRulesetVariables() const;
    IProvidersIndexAllocator& GetProvidersIndexAllocator() const {return *m_providersIndexAllocator;}
    void SetProvidersIndexAllocator(IProvidersIndexAllocator& allocator) {m_providersIndexAllocator = &allocator;}
    bset<ArtifactsCapturer*> const& GetArtifactsCapturers() const {return m_artifactsCapturers;}
    void SetArtifactsCapturers(bset<ArtifactsCapturer*> capturers) {m_artifactsCapturers = capturers;}
    void AddArtifactsCapturer(ArtifactsCapturer* capturer) {m_artifactsCapturers.insert(capturer);}
    void RemoveArtifactsCapturer(ArtifactsCapturer* capturer) {m_artifactsCapturers.erase(capturer);}

    // optimization flags
    bool IsFullNodesLoadDisabled() const {return m_isFullLoadDisabled;}
    void SetDisableFullLoad(bool value) {m_isFullLoadDisabled = value;}
    bool IsUpdatesDisabled() const {return m_isUpdatesDisabled;}
    void SetIsUpdatesDisabled(bool value) { m_isUpdatesDisabled = value; }
    bool NeedsFullLoad() const {return !IsFullNodesLoadDisabled();}
    bool IsCheckingChildren() const {return m_isCheckingChildren;}
    void SetIsCheckingChildren(bool value) {m_isCheckingChildren = value;}
    void SetPageSize(size_t value) {m_pageSize = value; m_hasPageSize = true;}
    bool HasPageSize() const {return m_hasPageSize;}
    size_t GetPageSize() const {return m_pageSize;}
    bool MayHaveArtifacts() const {return m_mayHaveArtifacts;}
    void SetMayHaveArtifacts(bool value) {m_mayHaveArtifacts = value;}
    bool RequiresFullProviderLoad() const {return !m_artifactsCapturers.empty() && m_mayHaveArtifacts;}

    // root nodes context
    ECPRESENTATION_EXPORT void SetRootNodeContext(RootNodeRuleCP);
    ECPRESENTATION_EXPORT void SetRootNodeContext(NavNodesProviderContextCR other);
    bool IsRootNodeContext() const {return m_isRootNodeContext;}
    RootNodeRuleCP GetRootNodeRule() const {BeAssert(IsRootNodeContext()); return m_rootNodeRule;}

    // child nodes context
    ECPRESENTATION_EXPORT void SetChildNodeContext(ChildNodeRuleCP, NavNodeCR virtualParentNode);
    ECPRESENTATION_EXPORT void SetChildNodeContext(NavNodesProviderContextCR other);
    bool IsChildNodeContext() const {return m_isChildNodeContext;}
    ChildNodeRuleCP GetChildNodeRule() const {BeAssert(IsChildNodeContext()); return m_childNodeRule;}

    // ECDb context
    ECPRESENTATION_EXPORT void SetQueryContext(IConnectionManagerCR, IConnectionCR, IECDbUsedClassesListener*);
    ECPRESENTATION_EXPORT void SetQueryContext(NavNodesProviderContextCR other);
    NavigationQueryBuilder& GetQueryBuilder() const;
    IUsedClassesListener* GetUsedClassesListener() const;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct INodesProviderContextFactory
{
protected:
    virtual NavNodesProviderContextPtr _Create(IConnectionCR, Utf8CP rulesetId, Utf8CP locale, uint64_t const* parentNodeId,
        ICancelationTokenCP, size_t pageSize, RulesetVariables const& variables) const = 0;
public:
    virtual ~INodesProviderContextFactory() {}
    NavNodesProviderContextPtr Create(IConnectionCR connection, Utf8CP rulesetId, Utf8CP locale, uint64_t const* parentNodeId,
        ICancelationTokenCP cancelationToken = nullptr, size_t pageSize = -1, RulesetVariables const& variables = RulesetVariables()) const
        {
        return _Create(connection, rulesetId, locale, parentNodeId, cancelationToken, pageSize, variables);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2018
+===============+===============+===============+===============+===============+======*/
enum class ProviderNodesInitializationStrategy
    {
    Automatic,  //!< InitializeNodes is called automatically when GetNode, GetNodesCount or HasNodes are called
    Manual,     //!< Provider implementation is responsible for calling InitializeNodes before starting to cache its nodes
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2015
+===============+===============+===============+===============+===============+======*/
enum HasChildrenFlag
    {
    HASCHILDREN_False,
    HASCHILDREN_True,
    HASCHILDREN_Unknown
    };

struct NodesCountContext;
struct NodesCheckContext;
struct DisabledFullNodesLoadContext;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2017
+===============+===============+===============+===============+===============+======*/
struct DataSourceRelatedVariablesUpdater
    {
    NavNodesProviderContextCR m_context;
    JsonNavNodeCP m_node;
    size_t m_relatedVariablesCountBefore;

    DataSourceRelatedVariablesUpdater(NavNodesProviderContextCR context, JsonNavNodeCP node);
    ~DataSourceRelatedVariablesUpdater();
    };

/*=================================================================================**//**
* Abstract class for navigation node providers.
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct NavNodesProvider : RefCountedBase
{
    struct SpecificationsVisitor;
    friend struct NavNodesProviderContext;
    friend struct NodesCountContext;
    friend struct NodesCheckContext;
    friend struct DisabledFullNodesLoadContext;
    typedef IteratorWrapper<JsonNavNodePtr> Iterator;

private:
    NavNodesProviderContextPtr m_context;
    mutable bool m_nodesInitialized;
    mutable bool m_cachedHasNodesFlag;
    mutable bool m_hasCachedHasNodesFlag;
    mutable uint64_t m_cachedNodesCount;
    mutable bool m_hasCachedNodesCount;

protected:
    ECPRESENTATION_EXPORT NavNodesProvider(NavNodesProviderContextCR context);
    HasChildrenFlag AnyChildSpecificationReturnsNodes(JsonNavNodeR parentNode, bool captureChildrenArtifacts) const;
    NavNodesProviderPtr GetCachedProvider() const;
    void FinalizeNode(JsonNavNodeR, bool customizeLabel) const;
    ECPRESENTATION_EXPORT void InitializeNodes() const;
    bool HasSimilarNodeInHierarchy(JsonNavNodeCR node, uint64_t parentNodeId) const;
    bool ShouldReturnChildNodes(JsonNavNodeR node) const;
    NavNodesProviderPtr CreateProvider(JsonNavNodeR node, bool customizeLabel) const;
    NavNodesProviderPtr CreateProviderForCachedNode(JsonNavNodeR node, bool customizeLabel) const;
    void EvaluateArtifacts(JsonNavNodeCR node) const;

    virtual bool _IsCacheable() const = 0;
    virtual ProviderNodesInitializationStrategy _GetInitializationStrategy() const {return ProviderNodesInitializationStrategy::Automatic;}
    virtual bool _InitializeNodes() {return true;}

    virtual Iterator _CreateFrontIterator() const = 0;
    virtual Iterator _CreateBackIterator() const = 0;

    virtual EmptyNavNodesProviderCP _AsEmptyProvider() const {return nullptr;}
    virtual SingleNavNodeProviderCP _AsSingleProvider() const {return nullptr;}
    virtual MultiNavNodesProviderCP _AsMultiProvider() const {return nullptr;}

    virtual bool _GetNode(JsonNavNodePtr& node, size_t index) const = 0;
    virtual bool _HasNodes() const = 0;
    virtual size_t _GetNodesCount() const = 0;

public:
    virtual ~NavNodesProvider() {}
    NavNodesProviderContextR GetContextR() const {return *m_context;}
    NavNodesProviderContextCR GetContext() const {return GetContextR();}
    MultiNavNodesProviderCP AsMultiProvider() const {return _AsMultiProvider();}
    EmptyNavNodesProviderCP AsEmptyProvider() const {return _AsEmptyProvider();}
    SingleNavNodeProviderCP AsSingleProvider() const {return _AsSingleProvider();}
    ECPRESENTATION_EXPORT void Initialize() const;
    ECPRESENTATION_EXPORT void FinalizeNodes();
    ECPRESENTATION_EXPORT bool GetNode(JsonNavNodePtr& node, size_t index) const;
    ECPRESENTATION_EXPORT size_t GetNodesCount() const;
    ECPRESENTATION_EXPORT bool HasNodes() const;
    void DetermineChildren(JsonNavNodeR) const;
    void NotifyNodeChanged(JsonNavNodeCR node, int partsThatChanged) const;
    void SetNodesCount(size_t count) {m_cachedNodesCount = count; m_hasCachedNodesCount = true;}
    ECPRESENTATION_EXPORT Iterator begin() const;
    Iterator end() const {return _CreateBackIterator();}
    JsonNavNodePtr operator[](size_t index) const {JsonNavNodePtr node; return GetNode(node, index) ? node : nullptr;}
};

/*---------------------------------------------------------------------------------**//**
* A helper method to initialize nodes provider
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static T WithInitialize(T provider)
    {
    provider->Initialize();
    return provider;
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2018
+===============+===============+===============+===============+===============+======*/
enum class ProviderCacheType
    {
    None,       // always create fresh data provider
    Combined,   // create cached data provider for physical parent node
    Full,       // create cached data provider for virtual parent node
    Partial,    // create cached data provider for a single part of hierarchy level
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct INodesProviderFactory
{
protected:
    virtual NavNodesProviderPtr _Create(NavNodesProviderContextR, JsonNavNodeCP, ProviderCacheType) const = 0;
public:
    virtual ~INodesProviderFactory() {}
    NavNodesProviderPtr Create(NavNodesProviderContextR context, JsonNavNodeCP parent, ProviderCacheType cacheType) const {return _Create(context, parent, cacheType);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct DisabledFullNodesLoadContext
    {
    NavNodesProviderCR m_provider;
    bool m_wasFullLoadDisabled;
    DisabledFullNodesLoadContext(NavNodesProviderCR provider)
        : m_provider(provider)
        {
        m_wasFullLoadDisabled = m_provider.GetContext().IsFullNodesLoadDisabled();
        m_provider.GetContextR().SetDisableFullLoad(true);
        }
    ~DisabledFullNodesLoadContext() {m_provider.GetContextR().SetDisableFullLoad(m_wasFullLoadDisabled);}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
+===============+===============+===============+===============+===============+======*/
struct EmptyNavNodesProvider : NavNodesProvider
{
private:
    EmptyNavNodesProvider(NavNodesProviderContextR context);
protected:
    bool _IsCacheable() const override {return true;}
    ECPRESENTATION_EXPORT bool _InitializeNodes() override;
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override {return false;}
    bool _HasNodes() const override {return false;}
    size_t _GetNodesCount() const override {return 0;}
    EmptyNavNodesProviderCP _AsEmptyProvider() const override {return this;}
    Iterator _CreateFrontIterator() const override {return Iterator(std::make_unique<EmptyIteratorImpl<JsonNavNodePtr>>());}
    Iterator _CreateBackIterator() const override {return Iterator(std::make_unique<EmptyIteratorImpl<JsonNavNodePtr>>());}
public:
    static RefCountedPtr<EmptyNavNodesProvider> Create(NavNodesProviderContextR context)
        {
        return WithInitialize(new EmptyNavNodesProvider(context));
        }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2015
+===============+===============+===============+===============+===============+======*/
struct SingleNavNodeProvider : NavNodesProvider
{
private:
    JsonNavNodePtr m_node;
    bool m_customizeLabel;
private:
    SingleNavNodeProvider(JsonNavNode& node, NavNodesProviderContextCR context, bool customizeLabel)
        : NavNodesProvider(context), m_node(&node), m_customizeLabel(customizeLabel)
        {}
protected:
    bool _IsCacheable() const override {return false;}
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override
        {
        if (0 != index)
            return false;
        node = m_node;
        FinalizeNode(*node, m_customizeLabel);
        return true;
        }
    bool _HasNodes() const override {return true;}
    size_t _GetNodesCount() const override {return 1;}
    SingleNavNodeProviderCP _AsSingleProvider() const override {return this;}
    Iterator _CreateFrontIterator() const override {return Iterator(std::make_unique<RandomAccessIteratorImpl<NavNodesProvider, JsonNavNodePtr>>(*this));}
    Iterator _CreateBackIterator() const override {return Iterator(std::make_unique<RandomAccessIteratorImpl<NavNodesProvider, JsonNavNodePtr>>(*this, GetNodesCount()));}
public:
    static RefCountedPtr<SingleNavNodeProvider> Create(JsonNavNode& node, NavNodesProviderContextCR context, bool customizeLabel)
        {
        return WithInitialize(new SingleNavNodeProvider(node, context, customizeLabel));
        }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct MultiNavNodesProvider : NavNodesProvider
{
private:
    bvector<NavNodesProviderPtr> m_providers;
private:
    bool RequiresFullLoad() const;
protected:
    MultiNavNodesProvider(NavNodesProviderContextCR context) : NavNodesProvider(context) {}
    MultiNavNodesProviderCP _AsMultiProvider() const override {return this;}
    bool _IsCacheable() const override {return false;}
    ECPRESENTATION_EXPORT virtual bool _GetNode(JsonNavNodePtr& node, size_t index) const override;
    ECPRESENTATION_EXPORT virtual bool _HasNodes() const override;
    ECPRESENTATION_EXPORT virtual size_t _GetNodesCount() const override;
    ECPRESENTATION_EXPORT virtual Iterator _CreateFrontIterator() const override;
    ECPRESENTATION_EXPORT virtual Iterator _CreateBackIterator() const override;
public:
    static RefCountedPtr<MultiNavNodesProvider> Create(NavNodesProviderContextCR context) {return new MultiNavNodesProvider(context);}
    void AddProvider(NavNodesProvider& provider) {m_providers.push_back(&provider);}
    void RemoveProvider(NavNodesProvider& provider) {m_providers.erase(std::find(m_providers.begin(), m_providers.end(), &provider));}
    void SetProviders(bvector<NavNodesProviderPtr> const& providers) {m_providers = providers;}
    void ClearProviders() {m_providers.clear();}
    bvector<NavNodesProviderPtr>& GetNodeProvidersR() {return m_providers;}
    bvector<NavNodesProviderPtr> const& GetNodeProviders() const {return m_providers;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct BVectorNodesProvider : NavNodesProvider
{
private:
    bvector<JsonNavNodePtr> m_nodes;

private:
    BVectorNodesProvider(NavNodesProviderContext const& context, bvector<JsonNavNodePtr> nodes)
        : NavNodesProvider(context), m_nodes(nodes)
        {}

protected:
    bool _IsCacheable() const override {return false;}
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override
        {
        if (index < GetNodesCount())
            {
            node = m_nodes[index];
            return true;
            }
        return false;
        }
    bool _HasNodes() const override {return !m_nodes.empty();}
    size_t _GetNodesCount() const override {return m_nodes.size();}
    Iterator _CreateFrontIterator() const override {return Iterator(std::make_unique<RandomAccessIteratorImpl<bvector<JsonNavNodePtr>, JsonNavNodePtr>>(m_nodes));}
    Iterator _CreateBackIterator() const override {return Iterator(std::make_unique<RandomAccessIteratorImpl<bvector<JsonNavNodePtr>, JsonNavNodePtr>>(m_nodes, m_nodes.size()));}

public:
    static RefCountedPtr<BVectorNodesProvider> Create(NavNodesProviderContext const& context, bvector<JsonNavNodePtr> nodes)
        {
        return new BVectorNodesProvider(context, nodes);
        }
};

typedef RefCountedPtr<struct MultiSpecificationNodesProvider> MultiSpecificationNodesProviderPtr;
/*=================================================================================**//**
* Creates nodes based on supplied specifications.
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct MultiSpecificationNodesProvider : MultiNavNodesProvider
{
private:
    ECPRESENTATION_EXPORT MultiSpecificationNodesProvider(NavNodesProviderContextR context, RootNodeRuleSpecificationsList const& specs);
    ECPRESENTATION_EXPORT MultiSpecificationNodesProvider(NavNodesProviderContextR context, ChildNodeRuleSpecificationsList const& specs, NavNodeCR virtualParent);

protected:
    bool _IsCacheable() const override {return false;}
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override;
    size_t _GetNodesCount() const override;

public:
    static MultiSpecificationNodesProviderPtr Create(NavNodesProviderContextR context, RootNodeRuleSpecificationsList const& specs)
        {
        return WithInitialize(new MultiSpecificationNodesProvider(context, specs));
        }
    static MultiSpecificationNodesProviderPtr Create(NavNodesProviderContextR context, ChildNodeRuleSpecificationsList const& specs, NavNodeCR virtualParent)
        {
        return WithInitialize(new MultiSpecificationNodesProvider(context, specs, virtualParent));
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2019
+===============+===============+===============+===============+===============+======*/
struct IProvidedNodesPostProcessor
{
protected:
    virtual NavNodesProviderPtr _PostProcessNodeRequest(NavNodesProviderCR, JsonNavNodeCR, size_t index) const = 0;
    virtual NavNodesProviderPtr _PostProcessCountRequest(NavNodesProviderCR, size_t count) const = 0;
public:
    virtual ~IProvidedNodesPostProcessor() {}
    NavNodesProviderPtr PostProcessNodeRequest(NavNodesProviderCR processedProvider, JsonNavNodeCR node, size_t index) const {return _PostProcessNodeRequest(processedProvider, node, index);}
    NavNodesProviderPtr PostProcessCountRequest(NavNodesProviderCR processedProvider, size_t count) const {return _PostProcessCountRequest(processedProvider, count);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2019
+===============+===============+===============+===============+===============+======*/
struct DisplayLabelGroupingNodesPostProcessor : IProvidedNodesPostProcessor
{
protected:
    ECPRESENTATION_EXPORT NavNodesProviderPtr _PostProcessNodeRequest(NavNodesProviderCR processedProvider, JsonNavNodeCR node, size_t index) const override;
    ECPRESENTATION_EXPORT NavNodesProviderPtr _PostProcessCountRequest(NavNodesProviderCR processedProvider, size_t count) const override;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2019
+===============+===============+===============+===============+===============+======*/
struct SameLabelGroupingNodesPostProcessor : IProvidedNodesPostProcessor
{
private:
    bset<ECClassCP> m_groupedClasses;
private:
    bool IsSuitableForMerge(JsonNavNodeCR node) const;
    JsonNavNodePtr MergeNodes(NavNodesProviderContextCR context, JsonNavNodeR lhs, JsonNavNodeR rhs) const;
    ECPRESENTATION_EXPORT NavNodesProviderPtr CreatePostProcessedProvider(NavNodesProviderCR processedProvider) const;
    ECPRESENTATION_EXPORT void InitGroupedClasses(PresentationRuleSetCR, bvector<ChildNodeRuleCP> const&, ECSchemaHelper const&, IJsonLocalState const*);
protected:
    NavNodesProviderPtr _PostProcessNodeRequest(NavNodesProviderCR processedProvider, JsonNavNodeCR, size_t) const override {return CreatePostProcessedProvider(processedProvider);}
    NavNodesProviderPtr _PostProcessCountRequest(NavNodesProviderCR processedProvider, size_t count) const override {return CreatePostProcessedProvider(processedProvider);}
public:
    SameLabelGroupingNodesPostProcessor(PresentationRuleSetCR ruleset, bvector<ChildNodeRuleCP> const& rules, ECSchemaHelper const& schemaHelper, IJsonLocalState const* localState)
        {
        InitGroupedClasses(ruleset, rules, schemaHelper, localState);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2019
+===============+===============+===============+===============+===============+======*/
struct PostProcessingNodesProvider : NavNodesProvider
{
private:
    NavNodesProviderCPtr m_wrappedProvider;
    bvector<std::unique_ptr<IProvidedNodesPostProcessor const>> m_postProcessors;
    mutable NavNodesProviderCPtr m_processedProvider;

private:
    ECPRESENTATION_EXPORT PostProcessingNodesProvider(NavNodesProviderCR wrappedProvider);
    NavNodesProviderCPtr GetProcessedProvider() const;

protected:
    bool _IsCacheable() const override {return false;}
    ECPRESENTATION_EXPORT bool _GetNode(JsonNavNodePtr& node, size_t index) const override;
    ECPRESENTATION_EXPORT size_t _GetNodesCount() const override;
    ECPRESENTATION_EXPORT bool _HasNodes() const override;
    Iterator _CreateFrontIterator() const override {return GetProcessedProvider()->begin();}
    Iterator _CreateBackIterator() const override {return GetProcessedProvider()->end();}

public:
    static RefCountedPtr<PostProcessingNodesProvider> Create(NavNodesProviderCR provider)
        {
        return WithInitialize(new PostProcessingNodesProvider(provider));
        }
    void RegisterPostProcessor(std::unique_ptr<IProvidedNodesPostProcessor const> processor) {m_postProcessors.push_back(std::move(processor));}
};

/*=================================================================================**//**
* Creates nodes based on CustomNodeSpecifications.
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct CustomNodesProvider : NavNodesProvider
{
    using NavNodesProvider::GetNode;

private:
    CustomNodeSpecificationCR m_specification;
    NavNodesProviderPtr m_provider;

private:
    ECPRESENTATION_EXPORT CustomNodesProvider(NavNodesProviderContextCR context, CustomNodeSpecificationCR specification);

protected:
    bool _IsCacheable() const override {return true;}
    bool _InitializeNodes() override;
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override;
    bool _HasNodes() const override;
    size_t _GetNodesCount() const override;
    Iterator _CreateFrontIterator() const override {return m_provider->begin();}
    Iterator _CreateBackIterator() const override {return m_provider->end();}

public:
    static RefCountedPtr<CustomNodesProvider> Create(NavNodesProviderContextCR context, CustomNodeSpecificationCR specification)
        {
        return WithInitialize(new CustomNodesProvider(context, specification));
        }
};

/*=================================================================================**//**
* Creates nodes based on the specified query.
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct QueryBasedNodesProvider : MultiNavNodesProvider
{
    struct Savepoint;

    using NavNodesProvider::GetNode;

private:
    NavigationQueryCPtr m_query;
    mutable bmap<ECClassId, bool> m_usedClassIds;
    mutable NavigationQueryExecutor m_executor;
    size_t m_executorIndex;
    size_t m_offset;

private:
    ECPRESENTATION_EXPORT QueryBasedNodesProvider(NavNodesProviderContextCR context, NavigationQuery const& query, bmap<ECClassId, bool> const&);
    void InitializeDataSource();
    bool InitializeProvidersFromCache();
    bool InitializeProvidersForAllNodes();
    bool InitializeProvidersForPagedQueries(size_t nodesCount, size_t pageSize);

protected:
    bool _IsCacheable() const override {return true;}
    ProviderNodesInitializationStrategy _GetInitializationStrategy() const override {return ProviderNodesInitializationStrategy::Manual;}
    bool _InitializeNodes() override;
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override;
    bool _HasNodes() const override;
    size_t _GetNodesCount() const override;
    Iterator _CreateFrontIterator() const override;

public:
    static RefCountedPtr<QueryBasedNodesProvider> Create(NavNodesProviderContextCR context,
        NavigationQuery const& query, bmap<ECClassId, bool> const& usedClassIds = bmap<ECClassId, bool>())
        {
        return WithInitialize(new QueryBasedNodesProvider(context, query, usedClassIds));
        }
    NavigationQueryExecutor const& GetExecutor() const {return m_executor;}
    NavigationQueryExecutor& GetExecutorR() {return m_executor;}
    bmap<ECClassId, bool> const& GetUsedClassIds() const {return m_usedClassIds;}
    void SetQuery(NavigationQuery const& query, bmap<ECClassId, bool> const&);
    void SetOffset(size_t value) {m_offset = value;}
};

/*=================================================================================**//**
* Creates nodes based on query based specifications.
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct QueryBasedSpecificationNodesProvider : MultiNavNodesProvider
{
    using NavNodesProvider::GetNode;

private:
    ChildNodeSpecificationCR m_specification;

private:
    ECPRESENTATION_EXPORT QueryBasedSpecificationNodesProvider(NavNodesProviderContextCR context, ChildNodeSpecificationCR specification);
    bvector<NavigationQueryPtr> CreateQueries(ChildNodeSpecificationCR specification) const;

protected:
    bool _IsCacheable() const override {return false;}
    bool _HasNodes() const override;

public:
    static RefCountedPtr<QueryBasedSpecificationNodesProvider> Create(NavNodesProviderContextCR context, ChildNodeSpecificationCR specification)
        {
        return WithInitialize(new QueryBasedSpecificationNodesProvider(context, specification));
        }
};

/*=================================================================================**//**
* Uses NavNodeCache's backing sqlite db to retrieve cached nodes.
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct SQLiteCacheNodesProvider : NavNodesProvider
{
private:
    BeSQLite::Db& m_cache;
    BeSQLite::StatementCache& m_statements;
    mutable bvector<JsonNavNodePtr>* m_nodes;
    mutable size_t* m_nodesCount;

private:
    void InitializeNodes();
    void InitializeUsedVariables();

protected:
    SQLiteCacheNodesProvider(NavNodesProviderContextCR, BeSQLite::Db&, BeSQLite::StatementCache&);
    BeSQLite::Db& GetCache() const {return m_cache;}
    BeSQLite::StatementCache& GetStatements() const {return m_statements;}
    bool _IsCacheable() const override {return false;}
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override;
    bool _HasNodes() const override;
    size_t _GetNodesCount() const override;
    Iterator _CreateFrontIterator() const override;
    Iterator _CreateBackIterator() const override;
    virtual BeSQLite::CachedStatementPtr _GetNodesStatement() const = 0;
    virtual BeSQLite::CachedStatementPtr _GetCountStatement() const = 0;

public:
    virtual ~SQLiteCacheNodesProvider();
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct CachedCombinedHierarchyLevelProvider : SQLiteCacheNodesProvider
{
private:
    CombinedHierarchyLevelInfo m_info;
private:
    CachedCombinedHierarchyLevelProvider(NavNodesProviderContextCR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements, CombinedHierarchyLevelInfo info)
        : SQLiteCacheNodesProvider(context, cache, statements), m_info(info)
        {}
protected:
    BeSQLite::CachedStatementPtr _GetNodesStatement() const override;
    BeSQLite::CachedStatementPtr _GetCountStatement() const override;
public:
    static RefCountedPtr<CachedCombinedHierarchyLevelProvider> Create(NavNodesProviderContextCR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements, CombinedHierarchyLevelInfo info)
        {
        return WithInitialize(new CachedCombinedHierarchyLevelProvider(context, cache, statements, info));
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct CachedHierarchyLevelProvider : SQLiteCacheNodesProvider
{
private:
    uint64_t m_hierarchyLevelId;
private:
    CachedHierarchyLevelProvider(NavNodesProviderContextCR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements, uint64_t hierarchyLevelId)
        : SQLiteCacheNodesProvider(context, cache, statements), m_hierarchyLevelId(hierarchyLevelId)
        {}
protected:
    BeSQLite::CachedStatementPtr _GetNodesStatement() const override;
    BeSQLite::CachedStatementPtr _GetCountStatement() const override;
public:
    static RefCountedPtr<CachedHierarchyLevelProvider> Create(NavNodesProviderContextCR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements, uint64_t hierarchyLevelId)
        {
        return WithInitialize(new CachedHierarchyLevelProvider(context, cache, statements, hierarchyLevelId));
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct CachedPartialDataSourceProvider : SQLiteCacheNodesProvider
{
private:
    uint64_t m_dataSourceId;
private:
    CachedPartialDataSourceProvider(NavNodesProviderContextCR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements, uint64_t dataSourceId)
        : SQLiteCacheNodesProvider(context, cache, statements), m_dataSourceId(dataSourceId)
        {}
protected:
    BeSQLite::CachedStatementPtr _GetNodesStatement() const override;
    BeSQLite::CachedStatementPtr _GetCountStatement() const override;
public:
    static RefCountedPtr<CachedPartialDataSourceProvider> Create(NavNodesProviderContextCR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements, uint64_t dataSourceId)
        {
        return WithInitialize(new CachedPartialDataSourceProvider(context, cache, statements, dataSourceId));
        }
};

/*=================================================================================**//**
* @bsiclass                                     Aidas.Vaiksnoras                10/2017
+===============+===============+===============+===============+===============+======*/
struct NodesWithUndeterminedChildrenProvider : SQLiteCacheNodesProvider
{
private:
    NodesWithUndeterminedChildrenProvider(NavNodesProviderContextCR, BeSQLite::Db&, BeSQLite::StatementCache&);

protected:
    BeSQLite::CachedStatementPtr _GetNodesStatement() const override;
    BeSQLite::CachedStatementPtr _GetCountStatement() const override;

public:
    static RefCountedPtr<NodesWithUndeterminedChildrenProvider> Create(NavNodesProviderContextCR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements)
        {
        return WithInitialize(new NodesWithUndeterminedChildrenProvider(context, cache, statements));
        }
};

/*=================================================================================**//**
* @bsiclass                                     Aidas.Vaiksnoras                10/2017
+===============+===============+===============+===============+===============+======*/
struct FilteredNodesProvider : SQLiteCacheNodesProvider
{
private:
    Utf8String m_filter;
    FilteredNodesProvider(NavNodesProviderContextCR, BeSQLite::Db&, BeSQLite::StatementCache&, Utf8String filter);

protected:
    BeSQLite::CachedStatementPtr _GetNodesStatement() const override;
    BeSQLite::CachedStatementPtr _GetCountStatement() const override;

public:
    static RefCountedPtr<FilteredNodesProvider> Create(NavNodesProviderContextCR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements, Utf8String filter)
        {
        return WithInitialize(new FilteredNodesProvider(context, cache, statements, filter));
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2019
+===============+===============+===============+===============+===============+======*/
struct CachedNodeProvider : NavNodesProvider
{
private:
    uint64_t m_nodeId;
    RefCountedPtr<SingleNavNodeProvider> m_singleNodeProvider;
private:
    CachedNodeProvider(NavNodesProviderContextCR context, uint64_t id)
        : NavNodesProvider(context), m_nodeId(id)
        {}
protected:
    bool _InitializeNodes() override;
    bool _IsCacheable() const override {return false;}
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override {return m_singleNodeProvider->GetNode(node, index);}
    bool _HasNodes() const override {return m_singleNodeProvider->HasNodes();}
    size_t _GetNodesCount() const override {return m_singleNodeProvider->GetNodesCount();}
    SingleNavNodeProviderCP _AsSingleProvider() const override {return m_singleNodeProvider.get();}
    Iterator _CreateFrontIterator() const override {return m_singleNodeProvider->begin();}
    Iterator _CreateBackIterator() const override {return m_singleNodeProvider->end();}
public:
    static RefCountedPtr<CachedNodeProvider> Create(NavNodesProviderContextCR context, uint64_t id)
        {
        return WithInitialize(new CachedNodeProvider(context, id));
        }
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
