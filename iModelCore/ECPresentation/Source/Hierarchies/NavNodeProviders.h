/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../Shared/Queries/QueryExecutor.h"
#include "../Shared/UsedClassesListener.h"
#include "../Shared/RulesDrivenProviderContext.h"
#include "DataSourceInfo.h"
#include "NavigationQueryResultsReader.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

struct ECDbUsedClassesListenerWrapper;

/*=================================================================================**//**
* @bsiclass
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
* @bsiclass
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
* @bsiclass
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
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct OptimizationFlagsContainer
{
private:
    OptimizationFlagsContainer const* m_parentContainer;
    bool m_isFullLoadDisabled;
    bool m_isPostProcessingDisabled;
    bool m_isUpdatesDisabled;
    Nullable<size_t> m_maxNodesToLoad;

public:
    OptimizationFlagsContainer()
        : m_isFullLoadDisabled(false), m_isPostProcessingDisabled(false), m_isUpdatesDisabled(false), m_maxNodesToLoad(nullptr), m_parentContainer(nullptr)
        {}
    OptimizationFlagsContainer(OptimizationFlagsContainer const&) = delete;
    OptimizationFlagsContainer(OptimizationFlagsContainer&&) = delete;

    void From(OptimizationFlagsContainer const& other)
        {
        m_parentContainer = other.m_parentContainer;
        m_isFullLoadDisabled = other.m_isFullLoadDisabled;
        m_isPostProcessingDisabled = other.m_isPostProcessingDisabled;
        m_isUpdatesDisabled = other.m_isUpdatesDisabled;
        m_maxNodesToLoad = other.m_maxNodesToLoad;
        }

    OptimizationFlagsContainer const* GetParentContainer() const {return m_parentContainer;}
    void SetParentContainer(OptimizationFlagsContainer const* parentContainer)
        {
        if (parentContainer == this)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Attempting to set parent optimizations flag container to 'this'");
        m_parentContainer = parentContainer;
        }

    bool IsFullNodesLoadDisabled(bool checkParent = true) const {return m_isFullLoadDisabled || checkParent && m_parentContainer && m_parentContainer->IsFullNodesLoadDisabled();}
    void SetDisableFullLoad(bool value) {m_isFullLoadDisabled = value;}

    bool IsPostProcessingDisabled(bool checkParent = true) const {return m_isPostProcessingDisabled || checkParent && m_parentContainer && m_parentContainer->IsPostProcessingDisabled();}
    void SetDisablePostProcessing(bool value) {m_isPostProcessingDisabled = value;}

    bool IsUpdatesDisabled(bool checkParent = true) const {return m_isUpdatesDisabled || checkParent && m_parentContainer && m_parentContainer->IsUpdatesDisabled();}
    void SetIsUpdatesDisabled(bool value) {m_isUpdatesDisabled = value;}

    size_t GetMaxNodesToLoad(bool checkParent = true) const
        {
        if (m_maxNodesToLoad.IsValid())
            return m_maxNodesToLoad.Value();
        return checkParent && m_parentContainer ? m_parentContainer->GetMaxNodesToLoad() : 0;
        }
    void SetMaxNodesToLoad(Nullable<size_t> value) {m_maxNodesToLoad = value;}
};

/*=================================================================================**//**
* Context for NavNodesProvider implementations.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavNodesProviderContext : RulesDrivenProviderContext
{
    struct PageOptions
        {
        private:
            size_t m_start;
            Nullable<size_t> m_size;
        public:
            PageOptions(size_t start) : m_start(start) {}
            PageOptions(size_t start, size_t size) : m_start(start), m_size(size) {}
            bool HasSize() const {return m_size.IsValid();}
            size_t GetStart() const {return m_start;}
            size_t GetSize() const {return m_size.Value();}
            size_t GetAdjustedPageSize(size_t totalCount) const;
        };

private:
    // common
    RuleTargetTree m_targetTree;
    std::shared_ptr<INavNodesCache> m_nodesCache;
    INodesProviderFactoryCR m_providerFactory;
    NavNodeCPtr m_physicalParentNode;
    NavNodeCPtr m_virtualParentNode;
    BeGuid m_removalId;
    mutable IProvidersIndexAllocatorPtr m_providersIndexAllocator;
    bset<ArtifactsCapturer*> m_artifactsCapturers;
    std::function<void(NavNodesProviderContextR)> m_onHierarchyLevelLoaded;

    // optimization flags
    OptimizationFlagsContainer m_optFlags;
    bool m_mayHaveArtifacts;

    // paging
    bool m_requestingAllNodes;
    std::shared_ptr<PageOptions> m_pageOptions;

    // root nodes context
    bool m_isRootNodeContext;
    RootNodeRuleCP m_rootNodeRule;

    // child nodes context
    bool m_isChildNodeContext;
    ChildNodeRuleCP m_childNodeRule;

    // ECDb context
    mutable NavigationQueryBuilder* m_queryBuilder;
    ECDbUsedClassesListenerWrapper* m_usedClassesListener;

    // hierarchy locking
    std::shared_ptr<IHierarchyLevelLocker> m_hierarchyLevelLocker;

private:
    void Init();
    ECPRESENTATION_EXPORT NavNodesProviderContext(PresentationRuleSetCR, RuleTargetTree, NavNodeCP, std::unique_ptr<RulesetVariables>, ECExpressionsCache&,
        RelatedPathsCache&, NavNodesFactory const&, std::shared_ptr<INavNodesCache>, INodesProviderFactoryCR, IJsonLocalState const*);
    ECPRESENTATION_EXPORT NavNodesProviderContext(NavNodesProviderContextCR other);

public:
    static NavNodesProviderContextPtr Create(PresentationRuleSetCR ruleset, RuleTargetTree targetTree, NavNodeCP physicalParent,
        std::unique_ptr<RulesetVariables> rulesetVariables, ECExpressionsCache& ecexpressionsCache, RelatedPathsCache& relatedPathsCache,
        NavNodesFactory const& nodesFactory, std::shared_ptr<INavNodesCache> nodesCache, INodesProviderFactoryCR providerFactory, IJsonLocalState const* localState)
        {
        return new NavNodesProviderContext(ruleset, targetTree, physicalParent, std::move(rulesetVariables), ecexpressionsCache,
            relatedPathsCache, nodesFactory, nodesCache, providerFactory, localState);
        }
    static NavNodesProviderContextPtr Create(NavNodesProviderContextCR other) {return new NavNodesProviderContext(other);}
    ~NavNodesProviderContext();

    // common
    RuleTargetTree GetTargetTree() const {return m_targetTree;}
    ECPRESENTATION_EXPORT INavNodesCacheR GetNodesCache() const;
    NavNodeCPtr GetPhysicalParentNode() const {return m_physicalParentNode;}
    void SetPhysicalParentNode(NavNodeCP node) {m_physicalParentNode = node;}
    NavNodeCPtr GetVirtualParentNode() const {return m_virtualParentNode;}
    void SetVirtualParentNode(NavNodeCP node) {m_virtualParentNode = node;}
    NavNodesProviderPtr CreateHierarchyLevelProvider(NavNodesProviderContextR, NavNodeCP parentNode, bool allowFromCache) const;
    CombinedHierarchyLevelIdentifier GetHierarchyLevelIdentifier() const;
    ECPRESENTATION_EXPORT bvector<RulesetVariableEntry> GetRelatedRulesetVariables() const;
    IProvidersIndexAllocator& GetProvidersIndexAllocator() const;
    void SetProvidersIndexAllocator(IProvidersIndexAllocator& allocator) {m_providersIndexAllocator = &allocator;}
    bset<ArtifactsCapturer*> const& GetArtifactsCapturers() const {return m_artifactsCapturers;}
    void SetArtifactsCapturers(bset<ArtifactsCapturer*> capturers) {m_artifactsCapturers = capturers;}
    void AddArtifactsCapturer(ArtifactsCapturer* capturer) {m_artifactsCapturers.insert(capturer);}
    void RemoveArtifactsCapturer(ArtifactsCapturer* capturer) {m_artifactsCapturers.erase(capturer);}
    void SetRemovalId(BeGuid id) {m_removalId = id;}
    BeGuidCR GetRemovalId() const {return m_removalId;}
    std::function<void(NavNodesProviderContextR)> const& GetHierarchyLevelLoadedCallback() const {return m_onHierarchyLevelLoaded;}
    void SetHierarchyLevelLoadedCallback(std::function<void(NavNodesProviderContextR)> cb) {m_onHierarchyLevelLoaded = cb;}

    // page options
    void SetPageOptions(std::shared_ptr<PageOptions> value) {m_pageOptions = value;}
    bool HasPageOptions() const {return m_pageOptions != nullptr;}
    bool HasPageOffset() const {return m_pageOptions && m_pageOptions->GetStart() > 0;}
    std::shared_ptr<PageOptions> GetPageOptions() const {return m_pageOptions;}

    // optimization flags
    OptimizationFlagsContainer const& GetOptimizationFlags() const {return m_optFlags;}
    OptimizationFlagsContainer& GetOptimizationFlags() {return m_optFlags;}
    bool MayHaveArtifacts() const {return m_mayHaveArtifacts;}
    void SetMayHaveArtifacts(bool value) {m_mayHaveArtifacts = value;}

    // root nodes context
    ECPRESENTATION_EXPORT void SetRootNodeContext(RootNodeRuleCP);
    ECPRESENTATION_EXPORT void SetRootNodeContext(NavNodesProviderContextCR other);
    bool IsRootNodeContext() const {return m_isRootNodeContext;}
    RootNodeRuleCP GetRootNodeRule() const {return m_rootNodeRule;}

    // child nodes context
    ECPRESENTATION_EXPORT void SetChildNodeContext(ChildNodeRuleCP, NavNodeCR virtualParentNode);
    ECPRESENTATION_EXPORT void SetChildNodeContext(NavNodesProviderContextCR other);
    bool IsChildNodeContext() const {return m_isChildNodeContext;}
    ChildNodeRuleCP GetChildNodeRule() const {return m_childNodeRule;}

    // ECDb context
    ECPRESENTATION_EXPORT void SetQueryContext(IConnectionManagerCR, IConnectionCR, IECDbUsedClassesListener*);
    ECPRESENTATION_EXPORT void SetQueryContext(NavNodesProviderContextCR other);
    NavigationQueryBuilder& GetQueryBuilder() const;
    IUsedClassesListener* GetUsedClassesListener() const;

    // hierarchy locking
    ECPRESENTATION_EXPORT IHierarchyLevelLocker& GetHierarchyLevelLocker() const;
    void SetHierarchyLevelLocker(std::shared_ptr<IHierarchyLevelLocker> locker) {m_hierarchyLevelLocker = locker;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IProvidedNodesPostProcessor
{
protected:
    virtual NavNodesProviderPtr _PostProcessProvider(NavNodesProviderR) const = 0;
public:
    virtual ~IProvidedNodesPostProcessor() {}
    NavNodesProviderPtr PostProcessProvider(NavNodesProviderR processedProvider) const {return _PostProcessProvider(processedProvider);}
};

/*=================================================================================**//**
* The post-processor makes sure display label grouping nodes are not displayed if they
* have no siblings. Instead, it returns children of the grouping node.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DisplayLabelGroupingNodesPostProcessor : IProvidedNodesPostProcessor
{
protected:
    ECPRESENTATION_EXPORT NavNodesProviderPtr _PostProcessProvider(NavNodesProviderR) const override;
};

/*
Display label post-processor is currently only useful when using hide expressions, which is a very rare and special
case. However, it introduces a huge performance bottleneck, especially with large lists of nodes, since it requires
all hierarchy level to be loaded into memory to re-sort it. Disable this post-processor to improve performance.

#define wip_enable_display_label_postprocessor 1
*/
#ifdef wip_enable_display_label_postprocessor
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DisplayLabelSortingPostProcessor : IProvidedNodesPostProcessor
{
protected:
    ECPRESENTATION_EXPORT NavNodesProviderPtr _PostProcessProvider(NavNodesProviderR) const override;
};
#endif

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesFinalizingPostProcessor : IProvidedNodesPostProcessor
{
protected:
    ECPRESENTATION_EXPORT NavNodesProviderPtr _PostProcessProvider(NavNodesProviderR) const override;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct INodesProviderContextFactory
{
protected:
    virtual NavNodesProviderContextPtr _Create(IConnectionCR, Utf8CP rulesetId, NavNodeCP parentNode, std::shared_ptr<INavNodesCache> cache,
        ICancelationTokenCP, RulesetVariables const& variables) const = 0;
public:
    virtual ~INodesProviderContextFactory() {}
    NavNodesProviderContextPtr Create(IConnectionCR connection, Utf8CP rulesetId, NavNodeCP parentNode, std::shared_ptr<INavNodesCache> cache,
        ICancelationTokenCP cancelationToken = nullptr, RulesetVariables const& variables = RulesetVariables()) const
        {
        return _Create(connection, rulesetId, parentNode, cache, cancelationToken, variables);
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum HasChildrenFlag
    {
    HASCHILDREN_False,
    HASCHILDREN_True,
    HASCHILDREN_Unknown
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DataSourceRelatedVariablesUpdater
    {
    NavNodesProviderContextCR m_context;
    DataSourceIdentifier m_dsIdentifier;
    RulesetVariables m_relatedVariablesBefore;

    DataSourceRelatedVariablesUpdater(NavNodesProviderContextCR context, DataSourceIdentifier);
    ~DataSourceRelatedVariablesUpdater();
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesInitializationState
{
private:
    BentleyStatus m_status;
    bvector<bpair<size_t, size_t>> m_initialized; // node index => count of initialized nodes
    size_t m_totalCount;
public:
    NodesInitializationState(BentleyStatus status): m_status(status), m_totalCount(0) {}
    NodesInitializationState(bvector<bpair<size_t, size_t>> initialized, size_t totalCount)
        : m_status(SUCCESS), m_initialized(initialized), m_totalCount(totalCount)
        {}
    BentleyStatus GetStatus() const {return m_status;}
    bvector<bpair<size_t, size_t>> const& GetInitializedNodes() const {return m_initialized;}
    size_t GetTotalCount() const {return m_totalCount;}
    bool IsFullyInitialized() const
        {
        if (SUCCESS != m_status)
            return false;
        if (0 == m_totalCount)
            return true;
        size_t initializedNodesCount = 0;
        for (auto const& entry : m_initialized)
            initializedNodesCount += entry.second;
        return m_totalCount == initializedNodesCount;
        }
    bool IsPageInitialized(NavNodesProviderContext::PageOptions const& page) const
        {
        if (!page.HasSize())
            return IsFullyInitialized();
        return ContainerHelpers::Contains(m_initialized, [&page, this](auto const& initializedNodesEntry)
            {
            size_t pageEnd = MIN(page.GetStart() + page.GetSize(), m_totalCount);
            size_t initializedNodesEntryPageEnd = initializedNodesEntry.first + initializedNodesEntry.second;
            return initializedNodesEntry.first <= page.GetStart()
                && initializedNodesEntryPageEnd >= pageEnd;
            });
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CountInfo
{
private:
    size_t m_count;
    bool m_isAccurate;
public:
    CountInfo(size_t count, bool isAccurate) : m_count(count), m_isAccurate(isAccurate) {}
    size_t GetCount() const {return m_count;}
    bool IsAccurate() const {return m_isAccurate;}
};

/*=================================================================================**//**
* Abstract class for navigation node providers.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavNodesProvider : RefCountedBase
{
    struct SpecificationsVisitor;
    typedef IteratorWrapper<NavNodePtr> Iterator;
    typedef NavNodesProviderContext::PageOptions PageOptions;

private:
    NavNodesProviderContextPtr m_context;
    mutable Nullable<bool> m_cachedHasNodes;
    mutable Nullable<size_t> m_cachedNodesCount;
    bool m_nodesInitialized;

protected:
    ECPRESENTATION_EXPORT NavNodesProvider(NavNodesProviderContextR context);
    void InitializeNodes();
    bool ShouldReturnChildNodes(NavNodeR node) const;
    NavNodesProviderPtr CreateProvider(NavNodeR node) const;
    void EvaluateChildrenArtifacts(NavNodeR) const;
    void EvaluateThisNodeArtifacts(NavNodeCR) const;
    bool IsInitialized() const {return m_nodesInitialized;}

protected:
    virtual Utf8CP _GetName() const = 0;
    virtual void _ResetInitializedNodes() {}
    virtual NodesInitializationState _InitializeNodes() {return NodesInitializationState(SUCCESS);}
    virtual NavNodesProviderPtr _FindNestedProvider(DataSourceIdentifier const&) {return nullptr;}
    virtual std::unordered_set<ECClassCP> _GetResultInstanceNodesClasses() const {return std::unordered_set<ECClassCP>();}
    ECPRESENTATION_EXPORT virtual DataSourceIdentifier const& _GetIdentifier() const;
    virtual RulesetVariables _GetRelatedRulesetVariables() const {return GetContext().GetRelatedRulesetVariables();}
    virtual bool _RequiresFullLoad() const {return !GetContext().GetArtifactsCapturers().empty() && GetContext().MayHaveArtifacts();}

    virtual Iterator _CreateFrontIterator() const = 0;
    virtual Iterator _CreateBackIterator() const = 0;

    virtual bool _HasNodes() const = 0;
    virtual CountInfo _GetTotalNodesCount() const = 0;

    virtual void _OnHasNodesFlagSet(bool) {}
    virtual void _OnNodesCountSet(size_t) {}
    virtual void _OnNodesInitialized(NodesInitializationState const&) {}

    ECPRESENTATION_EXPORT virtual void _OnPageOptionsSet();
    virtual void _InitializeDataSources() {}

public:
    virtual ~NavNodesProvider() {}
    Utf8CP GetName() const {return _GetName();}
    NavNodesProviderContextR GetContextR() const {return *m_context;}
    NavNodesProviderContextCR GetContext() const {return GetContextR();}

    RulesetVariables GetRelatedRulesetVariables() const {return _GetRelatedRulesetVariables();}
    DataSourceIdentifier const& GetIdentifier() const {return _GetIdentifier();}
    NavNodesProviderPtr FindNestedProvider(DataSourceIdentifier const& identifier) {return _FindNestedProvider(identifier);}
    std::unordered_set<ECClassCP> GetResultInstanceNodesClasses() const {return _GetResultInstanceNodesClasses();}
    bool RequiresFullLoad() const {return _RequiresFullLoad();}

    ECPRESENTATION_EXPORT size_t GetNodesCount() const;
    ECPRESENTATION_EXPORT bool HasNodes() const;
    ECPRESENTATION_EXPORT Iterator begin() const;
    ECPRESENTATION_EXPORT Iterator end() const;

    ECPRESENTATION_EXPORT void SetTotalNodesCount(size_t count);
    // returns total nodes count without page options applied
    ECPRESENTATION_EXPORT CountInfo GetTotalNodesCount() const;

    ECPRESENTATION_EXPORT NavNodesProviderPtr PostProcess(bvector<IProvidedNodesPostProcessor const*> const&);
    ECPRESENTATION_EXPORT NavNodesProviderCPtr PostProcess(bvector<IProvidedNodesPostProcessor const*> const&) const;

    ECPRESENTATION_EXPORT void SetPageOptions(std::shared_ptr<PageOptions>);
    void InitializeDataSources() {_InitializeDataSources();}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct INodesProviderFactory
{
protected:
    virtual bvector<IProvidedNodesPostProcessor const*> _GetPostProcessors() const = 0;
    virtual NavNodesProviderPtr _Create(NavNodesProviderContextR) const = 0;
public:
    virtual ~INodesProviderFactory() {}
    bvector<IProvidedNodesPostProcessor const*> GetPostProcessors() const {return _GetPostProcessors();}
    NavNodesProviderPtr Create(NavNodesProviderContextR context) const {return _Create(context);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
#define DEFINE_OPTIMIZATION_FLAG_CONTEXT(name, getter, setter, valueType, defaultValue) \
    struct name \
        { \
        NavNodesProviderCR m_provider; \
        valueType m_initialValue; \
        name(NavNodesProviderCR provider, valueType value = defaultValue) \
            : m_provider(provider) \
            { \
            m_initialValue = m_provider.GetContext().GetOptimizationFlags().getter(false); \
            m_provider.GetContextR().GetOptimizationFlags().setter(value); \
            } \
        ~name() {m_provider.GetContextR().GetOptimizationFlags().setter(m_initialValue);} \
        };
DEFINE_OPTIMIZATION_FLAG_CONTEXT(DisabledFullNodesLoadContext, IsFullNodesLoadDisabled, SetDisableFullLoad, bool, true);
DEFINE_OPTIMIZATION_FLAG_CONTEXT(DisabledPostProcessingContext, IsPostProcessingDisabled, SetDisablePostProcessing, bool, true);
DEFINE_OPTIMIZATION_FLAG_CONTEXT(MaxNodesToLoadContext, GetMaxNodesToLoad, SetMaxNodesToLoad, size_t, 0);

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TBaseProvider = NavNodesProvider>
struct CachingNavNodesProviderBase : TBaseProvider
{
private:
    mutable HierarchyLevelIdentifier m_hierarchyLevelIdentifier;
    mutable DataSourceIdentifier m_datasourceIdentifier;

private:
    HierarchyLevelIdentifier const& GetOrCreateHierarchyLevelIdentifier() const;
    ECPRESENTATION_EXPORT DataSourceIdentifier const& GetOrCreateDataSourceIdentifier(bool* createdNew = nullptr) const;

protected:
    CachingNavNodesProviderBase(NavNodesProviderContextR context) : TBaseProvider(context) {}
    ECPRESENTATION_EXPORT void UpdateDataSourceDirectNodesCount(DataSourceIdentifier const&, size_t value);
    ECPRESENTATION_EXPORT void UpdateDataSourceHasNodesFlag(DataSourceIdentifier const&, bool value);
    ECPRESENTATION_EXPORT void UpdateDataSourceTotalNodesCount(DataSourceIdentifier const&, size_t totalNodesCount);
    ECPRESENTATION_EXPORT void UpdateInitializedNodesState(DataSourceIdentifier const&, NodesInitializationState const&);
    ECPRESENTATION_EXPORT void OnCreated();

    template<typename TConcreteProvider> static RefCountedPtr<TConcreteProvider> CallOnCreated(TConcreteProvider& p) {p.OnCreated(); return &p;}

protected:
    virtual DataSourceIdentifier const& _GetIdentifier() const override {return GetOrCreateDataSourceIdentifier();}
    virtual void _OnHasNodesFlagSet(bool value) override {UpdateDataSourceHasNodesFlag(TBaseProvider::GetIdentifier(), value);}
    virtual void _OnNodesCountSet(size_t count) override {UpdateDataSourceTotalNodesCount(TBaseProvider::GetIdentifier(), count);}
    virtual void _OnNodesInitialized(NodesInitializationState const& state) override {UpdateInitializedNodesState(TBaseProvider::GetIdentifier(), state);}
    virtual void _OnFirstCreate() {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EmptyNavNodesProvider : NavNodesProvider
{
    DEFINE_T_SUPER(NavNodesProvider)

private:
    EmptyNavNodesProvider(NavNodesProviderContextR context) : T_Super(context) {}

protected:
    Utf8CP _GetName() const override {return "Empty node provider";}
    bool _HasNodes() const override {return false;}
    CountInfo _GetTotalNodesCount() const override {return CountInfo(0, true);}
    Iterator _CreateFrontIterator() const override {return Iterator(std::make_unique<EmptyIteratorImpl<NavNodePtr>>());}
    Iterator _CreateBackIterator() const override {return Iterator(std::make_unique<EmptyIteratorImpl<NavNodePtr>>());}

public:
    static RefCountedPtr<EmptyNavNodesProvider> Create(NavNodesProviderContextR context)
        {
        return new EmptyNavNodesProvider(context);
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MultiNavNodesProvider : NavNodesProvider
{
    DEFINE_T_SUPER(NavNodesProvider)

private:
    bvector<NavNodesProviderPtr> m_providers;

protected:
    MultiNavNodesProvider(NavNodesProviderContextR context) : NavNodesProvider(context) {}
    virtual Utf8CP _GetName() const override {return "Multi provider";}
    ECPRESENTATION_EXPORT virtual bool _HasNodes() const override;
    ECPRESENTATION_EXPORT virtual CountInfo _GetTotalNodesCount() const override;
    ECPRESENTATION_EXPORT virtual Iterator _CreateFrontIterator() const override;
    ECPRESENTATION_EXPORT virtual Iterator _CreateBackIterator() const override;
    ECPRESENTATION_EXPORT virtual NavNodesProviderPtr _FindNestedProvider(DataSourceIdentifier const&) override;
    ECPRESENTATION_EXPORT virtual std::unordered_set<ECClassCP> _GetResultInstanceNodesClasses() const override;
    ECPRESENTATION_EXPORT virtual void _OnPageOptionsSet() override;
    ECPRESENTATION_EXPORT virtual void _InitializeDataSources() override;
    ECPRESENTATION_EXPORT RulesetVariables _GetRelatedRulesetVariables() const override;
    ECPRESENTATION_EXPORT virtual bool _RequiresFullLoad() const override;

public:
    static RefCountedPtr<MultiNavNodesProvider> Create(NavNodesProviderContextR context) {return new MultiNavNodesProvider(context);}
    void SetProviders(bvector<NavNodesProviderPtr> providers)
        {
        m_providers = providers;
        for (NavNodesProviderPtr const& provider : m_providers)
            {
            provider->GetContextR().GetOptimizationFlags().SetParentContainer(
                (&provider->GetContext() != &GetContext()) ? &GetContext().GetOptimizationFlags() : nullptr);
            }
        }
    void AddProvider(NavNodesProviderR provider)
        {
        provider.GetContextR().GetOptimizationFlags().SetParentContainer(
            (&provider.GetContext() != &GetContext()) ? &GetContext().GetOptimizationFlags() : nullptr);
        m_providers.push_back(&provider);
        }
    void RemoveProvider(NavNodesProviderR provider) {m_providers.erase(std::find(m_providers.begin(), m_providers.end(), &provider));}
    void ClearProviders() {m_providers.clear();}
    bvector<NavNodesProviderPtr>& GetNodeProvidersR() {return m_providers;}
    bvector<NavNodesProviderPtr> const& GetNodeProviders() const {return m_providers;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DirectNodesIterator
{
protected:
    virtual NavNodePtr _NextNode() = 0;
    virtual bool _SkippedNodesToPageStart() const = 0;
    virtual size_t _NodesCount() const = 0;

public:
    virtual ~DirectNodesIterator() {}
    NavNodePtr NextNode() {return _NextNode();}
    bool SkippedNodesToPageStart() const {return _SkippedNodesToPageStart();}
    size_t NodesCount() const {return _NodesCount();}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesCreatingMultiNavNodesProvider : MultiNavNodesProvider
{
    DEFINE_T_SUPER(MultiNavNodesProvider)

private:
    ECPRESENTATION_EXPORT std::unique_ptr<DirectNodesIterator> GetDirectNodesIterator() const;

protected:
    NodesCreatingMultiNavNodesProvider(NavNodesProviderContextR context)
        : T_Super(context)
        {}
    virtual std::unique_ptr<DirectNodesIterator> _CreateDirectNodesIterator() const {return nullptr;}
    ECPRESENTATION_EXPORT virtual void _OnPageOptionsSet() override;
    ECPRESENTATION_EXPORT virtual void _ResetInitializedNodes() override;
    ECPRESENTATION_EXPORT virtual NodesInitializationState _InitializeNodes() override;
    ECPRESENTATION_EXPORT virtual CountInfo _GetTotalNodesCount() const override;
    ECPRESENTATION_EXPORT virtual bool _HasNodes() const override;
    ECPRESENTATION_EXPORT void _InitializeDataSources() override;
    virtual void _OnDirectNodesRead(size_t) {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct BVectorNodesProvider : NavNodesProvider
{
private:
    bvector<NavNodePtr> m_nodes;

private:
    BVectorNodesProvider(NavNodesProviderContextR context, bvector<NavNodePtr> nodes)
        : NavNodesProvider(context), m_nodes(nodes)
        {}

protected:
    Utf8CP _GetName() const override {return "Vector nodes provider";}
    bool _HasNodes() const override {return !m_nodes.empty();}
    CountInfo _GetTotalNodesCount() const override {return CountInfo(m_nodes.size(), true);}
    ECPRESENTATION_EXPORT Iterator _CreateFrontIterator() const override;
    ECPRESENTATION_EXPORT Iterator _CreateBackIterator() const override;

public:
    static RefCountedPtr<BVectorNodesProvider> Create(NavNodesProviderContextR context, bvector<NavNodePtr> nodes)
        {
        return new BVectorNodesProvider(context, nodes);
        }
};

typedef RefCountedPtr<struct MultiSpecificationNodesProvider> MultiSpecificationNodesProviderPtr;
/*=================================================================================**//**
* Creates nodes based on supplied specifications.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MultiSpecificationNodesProvider : MultiNavNodesProvider
{
    DEFINE_T_SUPER(MultiNavNodesProvider)

private:
    ECPRESENTATION_EXPORT MultiSpecificationNodesProvider(NavNodesProviderContextR context, RootNodeRuleSpecificationsList const& specs);
    ECPRESENTATION_EXPORT MultiSpecificationNodesProvider(NavNodesProviderContextR context, ChildNodeRuleSpecificationsList const& specs, NavNodeCR virtualParent);

protected:
    Utf8CP _GetName() const override {return "Multi-specification nodes provider";}

public:
    static MultiSpecificationNodesProviderPtr Create(NavNodesProviderContextR context, RootNodeRuleSpecificationsList const& specs)
        {
        return new MultiSpecificationNodesProvider(context, specs);
        }
    static MultiSpecificationNodesProviderPtr Create(NavNodesProviderContextR context, ChildNodeRuleSpecificationsList const& specs, NavNodeCR virtualParent)
        {
        return new MultiSpecificationNodesProvider(context, specs, virtualParent);
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IProvidedNodesPostProcessorDeprecated
{
protected:
    virtual NavNodesProviderPtr _PostProcess(NavNodesProviderCR processedProvider) const = 0;
public:
    virtual ~IProvidedNodesPostProcessorDeprecated() {}
    NavNodesProviderPtr PostProcess(NavNodesProviderCR processedProvider) const {return _PostProcess(processedProvider);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SameLabelGroupingNodesPostProcessorDeprecated : IProvidedNodesPostProcessorDeprecated
{
private:
    bset<ECClassCP> m_groupedClasses;
private:
    bool IsSuitableForMerge(NavNodeCR node) const;
    NavNodePtr MergeNodes(NavNodesProviderContextCR context, NavNodeR lhs, NavNodeR rhs) const;
    ECPRESENTATION_EXPORT void InitGroupedClasses(IRulesPreprocessorR, NavNodeCP, bvector<ChildNodeSpecificationCP> const&, ECSchemaHelper const&, IJsonLocalState const*);
    static HierarchyLevelIdentifier GetHierarchyLevelIdentifier(NavNodesProviderContextCR);
protected:
    ECPRESENTATION_EXPORT NavNodesProviderPtr _PostProcess(NavNodesProviderCR processedProvider) const override;
public:
    SameLabelGroupingNodesPostProcessorDeprecated(IRulesPreprocessorR rulesPreprocessor, NavNodeCP parentNode, bvector<ChildNodeSpecificationCP> const& specs, ECSchemaHelper const& schemaHelper, IJsonLocalState const* localState)
        {
        InitGroupedClasses(rulesPreprocessor, parentNode, specs, schemaHelper, localState);
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesFinalizer
{
private:
    NavNodesProviderContextPtr m_context;

private:
    HasChildrenFlag AnyChildSpecificationReturnsNodes(NavNodeR parentNode) const;
    bool HasSimilarNodeInHierarchy(NavNodeCR, NavNodeCR parentNode, int suppressCount) const;
    bool HasSimilarNodeInHierarchy(NavNodeCR, BeGuidCR parentNodeId, int suppressCount) const;
    bool HasSimilarNodeInHierarchy(NavNodeCR, int suppressCount = -1) const;

public:
    NodesFinalizer(NavNodesProviderContextR context)
        : m_context(&context)
        {}

    ECPRESENTATION_EXPORT void DetermineChildren(NavNodeR) const;
    ECPRESENTATION_EXPORT void Customize(NavNodeR) const;

    // DetermineChildren + Customize
    ECPRESENTATION_EXPORT NavNodePtr Finalize(NavNodeR node) const;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesFinalizingProvider : NavNodesProvider
{
    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct NodesFinalizingIteratorImpl : IteratorImpl<NavNodePtr>
        {
        private:
            RefCountedPtr<NodesFinalizingProvider const> m_provider;
            Iterator m_iter;
        protected:
            std::unique_ptr<IteratorImpl<NavNodePtr>> _Copy() const override {return std::make_unique<NodesFinalizingIteratorImpl>(*m_provider, m_iter);}
            bool _Equals(IteratorImpl<NavNodePtr> const& other) const override {return m_iter == static_cast<NodesFinalizingIteratorImpl const&>(other).m_iter;}
            void _Next(size_t count) override {m_iter += count;}
            NavNodePtr _GetCurrent() const override;
        public:
            NodesFinalizingIteratorImpl(NodesFinalizingProvider const& provider, Iterator iter)
                : m_provider(&provider), m_iter(iter)
                {}
        };

DEFINE_T_SUPER(NavNodesProvider)

private:
    NavNodesProviderPtr m_source;

private:
    NodesFinalizingProvider(NavNodesProviderContextR context, NavNodesProviderR source)
        : T_Super(context), m_source(&source)
        {}
    NavNodePtr FinalizeNode(NavNodeR node) const;

protected:
    Utf8CP _GetName() const override { return "Finalizing nodes provider"; }
    NavNodesProviderPtr _FindNestedProvider(DataSourceIdentifier const& id) override {return m_source->FindNestedProvider(id);}
    std::unordered_set<ECClassCP> _GetResultInstanceNodesClasses() const override {return m_source->GetResultInstanceNodesClasses();}
    Iterator _CreateFrontIterator() const override {return Iterator(std::make_unique<NodesFinalizingIteratorImpl>(*this, m_source->begin()));}
    Iterator _CreateBackIterator() const override {return Iterator(std::make_unique<NodesFinalizingIteratorImpl>(*this, m_source->end()));}
    bool _HasNodes() const override {return m_source->HasNodes();}
    CountInfo _GetTotalNodesCount() const override {return m_source->GetTotalNodesCount();}
    void _OnPageOptionsSet() override {m_source->SetPageOptions(GetContext().GetPageOptions());}
    void _InitializeDataSources() override {m_source->InitializeDataSources();}

public:
    static RefCountedPtr<NodesFinalizingProvider> Create(NavNodesProviderContextR context, NavNodesProviderR source)
        {
        return new NodesFinalizingProvider(context, source);
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PostProcessingNodesProviderDeprecated : MultiNavNodesProvider
{
    DEFINE_T_SUPER(MultiNavNodesProvider)

private:
    NavNodesProviderPtr m_wrappedProvider;
    bvector<std::unique_ptr<IProvidedNodesPostProcessorDeprecated const>> m_postProcessors;
    mutable bool m_isPostProcessed;

private:
    ECPRESENTATION_EXPORT PostProcessingNodesProviderDeprecated(NavNodesProviderR wrappedProvider);
    NavNodesProviderPtr GetProcessedProvider() const;

protected:
    Utf8CP _GetName() const override {return "Post-processing nodes provider";}
    ECPRESENTATION_EXPORT NavNodesProviderPtr _FindNestedProvider(DataSourceIdentifier const&) override;
    ECPRESENTATION_EXPORT CountInfo _GetTotalNodesCount() const override;
    ECPRESENTATION_EXPORT bool _HasNodes() const override;
    ECPRESENTATION_EXPORT Iterator _CreateFrontIterator() const override;
    ECPRESENTATION_EXPORT Iterator _CreateBackIterator() const override;
    ECPRESENTATION_EXPORT void _OnPageOptionsSet() override;
    ECPRESENTATION_EXPORT void _InitializeDataSources() override;

public:
    static RefCountedPtr<PostProcessingNodesProviderDeprecated> Create(NavNodesProviderR provider)
        {
        return new PostProcessingNodesProviderDeprecated(provider);
        }
    void RegisterPostProcessor(std::unique_ptr<IProvidedNodesPostProcessorDeprecated const> processor) {m_postProcessors.push_back(std::move(processor));}
};

/*=================================================================================**//**
* Creates nodes based on CustomNodeSpecifications.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CustomNodesProvider : CachingNavNodesProviderBase<NodesCreatingMultiNavNodesProvider>
{
    DEFINE_T_SUPER(CachingNavNodesProviderBase<NodesCreatingMultiNavNodesProvider>)

private:
    CustomNodeSpecificationCR m_specification;

private:
    ECPRESENTATION_EXPORT CustomNodesProvider(NavNodesProviderContextR context, CustomNodeSpecificationCR specification);

protected:
    Utf8CP _GetName() const override {return "Custom node specification nodes provider";}
    ECPRESENTATION_EXPORT void _OnFirstCreate() override;
    ECPRESENTATION_EXPORT std::unique_ptr<DirectNodesIterator> _CreateDirectNodesIterator() const override;
    void _OnDirectNodesRead(size_t nodeCount) override {UpdateDataSourceDirectNodesCount(GetIdentifier(), nodeCount);}

public:
    static RefCountedPtr<CustomNodesProvider> Create(NavNodesProviderContextR context, CustomNodeSpecificationCR specification)
        {
        return CallOnCreated(*new CustomNodesProvider(context, specification));
        }
};

/*=================================================================================**//**
* Creates nodes based on the specified query.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryBasedNodesProvider : CachingNavNodesProviderBase<NodesCreatingMultiNavNodesProvider>
{
    DEFINE_T_SUPER(CachingNavNodesProviderBase<NodesCreatingMultiNavNodesProvider>)

    // internally the provider splits query in pages of this size using LIMIT & OFFSET
    ECPRESENTATION_EXPORT static size_t const PartialProviderSize;

    struct Savepoint;
    struct PageNodeCounts;
    struct NodeCounts;

private:
    NavigationQueryCPtr m_query;
    mutable bmap<ECClassId, bool> m_usedClassIds;
    size_t m_offset;
    DataSourceIdentifier m_parentDatasourceIdentifier;

private:
    QueryBasedNodesProvider(NavNodesProviderContextR context, NavigationQuery const& query, bmap<ECClassId, bool> const& usedClassIds, DataSourceIdentifier parentDatasourceIdentifier)
        : T_Super(context), m_query(&query), m_usedClassIds(usedClassIds), m_offset(0), m_parentDatasourceIdentifier(parentDatasourceIdentifier)
        {}
    NodeCounts QueryNodeCounts() const;
    BentleyStatus InitializePartialProviders(bvector<PageNodeCounts> const&);

protected:
    Utf8CP _GetName() const override {return "Query-based nodes provider";}
    ECPRESENTATION_EXPORT std::unordered_set<ECClassCP> _GetResultInstanceNodesClasses() const override;
    ECPRESENTATION_EXPORT void _OnFirstCreate() override;
    ECPRESENTATION_EXPORT std::unique_ptr<DirectNodesIterator> _CreateDirectNodesIterator() const override;
    void _OnDirectNodesRead(size_t nodesCount) override {UpdateDataSourceDirectNodesCount(GetIdentifier(), nodesCount);}
    ECPRESENTATION_EXPORT NodesInitializationState _InitializeNodes() override;
    ECPRESENTATION_EXPORT bool _HasNodes() const override;
    ECPRESENTATION_EXPORT CountInfo _GetTotalNodesCount() const override;

public:
    static RefCountedPtr<QueryBasedNodesProvider> Create(NavNodesProviderContextR context,
        NavigationQuery const& query, bmap<ECClassId, bool> const& usedClassIds = bmap<ECClassId, bool>(), DataSourceIdentifier parentDatasourceIdentifier = {})
        {
        return CallOnCreated(*new QueryBasedNodesProvider(context, query, usedClassIds, parentDatasourceIdentifier));
        }
    bmap<ECClassId, bool> const& GetUsedClassIds() const {return m_usedClassIds;}
    void SetQuery(NavigationQuery const& query, bmap<ECClassId, bool> const&);
    void SetOffset(size_t value) {m_offset = value;}
};

/*=================================================================================**//**
* Creates nodes based on query based specifications.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryBasedSpecificationNodesProvider : MultiNavNodesProvider
{
    DEFINE_T_SUPER(MultiNavNodesProvider)

private:
    ChildNodeSpecificationCR m_specification;

private:
    ECPRESENTATION_EXPORT QueryBasedSpecificationNodesProvider(NavNodesProviderContextR context, ChildNodeSpecificationCR specification);
    bvector<NavigationQueryPtr> CreateQueries(ChildNodeSpecificationCR specification) const;

protected:
    Utf8CP _GetName() const override {return "Query-based specification nodes provider";}
    bool _HasNodes() const override;

public:
    static RefCountedPtr<QueryBasedSpecificationNodesProvider> Create(NavNodesProviderContextR context, ChildNodeSpecificationCR specification)
        {
        return new QueryBasedSpecificationNodesProvider(context, specification);
        }
};

/*=================================================================================**//**
* Base class for providers based on sqlite db.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TBaseProvider = NavNodesProvider>
struct SQLiteCacheNavNodesProviderBase : TBaseProvider
    {
    private:
        BeSQLite::Db& m_cache;
        BeSQLite::StatementCache& m_statements;
        BeMutex& m_cacheMutex;

    private:
        void InitializeUsedVariables();

    protected:
        SQLiteCacheNavNodesProviderBase(NavNodesProviderContextR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements, BeMutex& cacheMutex)
            : TBaseProvider(context), m_cache(cache), m_statements(statements), m_cacheMutex(cacheMutex)
            {}
        BeSQLite::Db& GetCache() const {return m_cache;}
        BeSQLite::StatementCache& GetStatements() const {return m_statements;}
        BeMutex& GetCacheMutex() const {return m_cacheMutex;}
        ECPRESENTATION_EXPORT void OnCreated();
        template<typename TConcreteProvider> static RefCountedPtr<TConcreteProvider> CallOnCreated(TConcreteProvider& p) {p.OnCreated(); return &p;}

    protected:
        ECPRESENTATION_EXPORT std::unordered_set<ECClassCP> _GetResultInstanceNodesClasses() const override;

    protected:
        virtual BeSQLite::CachedStatementPtr _GetUsedVariablesStatement() const = 0;
        virtual BeSQLite::CachedStatementPtr _GetResultInstanceNodesClassIdsStatement() const = 0;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CachedHierarchyLevelProvider : SQLiteCacheNavNodesProviderBase<NodesCreatingMultiNavNodesProvider>
    {
    DEFINE_T_SUPER(SQLiteCacheNavNodesProviderBase<NodesCreatingMultiNavNodesProvider>)

    private:
        BeGuid m_hierarchyLevelId;

    private:
        CachedHierarchyLevelProvider(NavNodesProviderContextR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements, BeMutex& cacheMutex, BeGuid hierarchyLevelId)
            : T_Super(context, cache, statements, cacheMutex), m_hierarchyLevelId(hierarchyLevelId)
            {}

    protected:
        Utf8CP _GetName() const override {return "Cached hierarchy level provider";}
        std::unique_ptr<DirectNodesIterator> _CreateDirectNodesIterator() const override;
        BeSQLite::CachedStatementPtr _GetUsedVariablesStatement() const override;
        BeSQLite::CachedStatementPtr _GetResultInstanceNodesClassIdsStatement() const override;

    public:
        ECPRESENTATION_EXPORT static RefCountedPtr<CachedHierarchyLevelProvider> Create(NavNodesProviderContextR, BeSQLite::Db&, BeSQLite::StatementCache&, BeMutex&, BeGuid hierarchyLevelId);
    };

/*=================================================================================**//**
* Uses NavNodeCache's backing sqlite db to retrieve cached nodes.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SQLiteCacheNodesProvider : SQLiteCacheNavNodesProviderBase<NavNodesProvider>
    {
    DEFINE_T_SUPER(SQLiteCacheNavNodesProviderBase<NavNodesProvider>)

    private:
        std::unique_ptr<bvector<NavNodePtr>> m_nodes;

    protected:
        SQLiteCacheNodesProvider(NavNodesProviderContextR, BeSQLite::Db&, BeSQLite::StatementCache&, BeMutex&);
        void _ResetInitializedNodes() override;
        NodesInitializationState _InitializeNodes() override;
        bool _HasNodes() const override;
        CountInfo _GetTotalNodesCount() const override;
        Iterator _CreateFrontIterator() const override {return m_nodes ? Iterator(std::make_unique<IterableIteratorImpl<bvector<NavNodePtr>::const_iterator, NavNodePtr>>(m_nodes->begin())) : Iterator(std::make_unique<EmptyIteratorImpl<NavNodePtr>>());}
        Iterator _CreateBackIterator() const override {return m_nodes ? Iterator(std::make_unique<IterableIteratorImpl<bvector<NavNodePtr>::const_iterator, NavNodePtr>>(m_nodes->end())) : Iterator(std::make_unique<EmptyIteratorImpl<NavNodePtr>>());}
        virtual void _OnPageOptionsSet() override;

    protected:
        virtual BeSQLite::CachedStatementPtr _GetNodesStatement() const = 0;
        virtual BeSQLite::CachedStatementPtr _GetCountStatement() const = 0;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CachedCombinedHierarchyLevelProvider : SQLiteCacheNodesProvider
    {
    private:
        BeGuid m_physicalHierarchyLevelId;

    private:
        CachedCombinedHierarchyLevelProvider(NavNodesProviderContextR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements, BeMutex& cacheMutex, BeGuidCR physicalHierarchyLevelId)
            : SQLiteCacheNodesProvider(context, cache, statements, cacheMutex), m_physicalHierarchyLevelId(physicalHierarchyLevelId)
            {}

    protected:
        Utf8CP _GetName() const override {return "Cached combined hierarchy level provider";}
        BeSQLite::CachedStatementPtr _GetUsedVariablesStatement() const override;
        BeSQLite::CachedStatementPtr _GetResultInstanceNodesClassIdsStatement() const override;
        BeSQLite::CachedStatementPtr _GetNodesStatement() const override;
        BeSQLite::CachedStatementPtr _GetCountStatement() const override;

    public:
        ECPRESENTATION_EXPORT static RefCountedPtr<CachedCombinedHierarchyLevelProvider> Create(NavNodesProviderContextR, BeSQLite::Db&, BeSQLite::StatementCache&, BeMutex&, BeGuidCR physicalHierarchyLevelId);
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CachedPartialDataSourceProvider : SQLiteCacheNodesProvider
    {
    private:
        BeGuid m_dataSourceId;
        bool m_wantOnlyVisibleNodes;
    private:
        CachedPartialDataSourceProvider(NavNodesProviderContextR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements, BeMutex& cacheMutex, BeGuidCR dataSourceId, bool loadOnlyVisibleNodes)
            : SQLiteCacheNodesProvider(context, cache, statements, cacheMutex), m_dataSourceId(dataSourceId), m_wantOnlyVisibleNodes(loadOnlyVisibleNodes)
            {}
    protected:
        Utf8CP _GetName() const override { return "Cached partial hierarchy level provider"; }
        BeSQLite::CachedStatementPtr _GetUsedVariablesStatement() const override;
        BeSQLite::CachedStatementPtr _GetResultInstanceNodesClassIdsStatement() const override;
        BeSQLite::CachedStatementPtr _GetNodesStatement() const override;
        BeSQLite::CachedStatementPtr _GetCountStatement() const override;
    public:
        ECPRESENTATION_EXPORT static RefCountedPtr<CachedPartialDataSourceProvider> Create(NavNodesProviderContextR, BeSQLite::Db&, BeSQLite::StatementCache&, BeMutex&, BeGuidCR dataSourceId, bool loadOnlyVisibleNodes = false);
        std::unique_ptr<DirectNodesIterator> CreateDirectNodesIterator() const;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesWithUndeterminedChildrenProvider : SQLiteCacheNodesProvider
    {
    private:
        NodesWithUndeterminedChildrenProvider(NavNodesProviderContextR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements, BeMutex& cacheMutex)
            : SQLiteCacheNodesProvider(context, cache, statements, cacheMutex)
            {}

    protected:
        Utf8CP _GetName() const override { return "Cached nodes with undetermined children provider"; }
        BeSQLite::CachedStatementPtr _GetUsedVariablesStatement() const override;
        BeSQLite::CachedStatementPtr _GetResultInstanceNodesClassIdsStatement() const override;
        BeSQLite::CachedStatementPtr _GetNodesStatement() const override;
        BeSQLite::CachedStatementPtr _GetCountStatement() const override;

    public:
        ECPRESENTATION_EXPORT static RefCountedPtr<NodesWithUndeterminedChildrenProvider> Create(NavNodesProviderContextR, BeSQLite::Db&, BeSQLite::StatementCache&, BeMutex&);
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FilteredNodesProvider : SQLiteCacheNodesProvider
    {
    private:
        Utf8String m_filter;
        FilteredNodesProvider(NavNodesProviderContextR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements, BeMutex& cacheMutex, Utf8String filter)
            : SQLiteCacheNodesProvider(context, cache, statements, cacheMutex), m_filter(filter)
            {}

    protected:
        Utf8CP _GetName() const override { return "Cached filtered nodes provider"; }
        BeSQLite::CachedStatementPtr _GetUsedVariablesStatement() const override;
        BeSQLite::CachedStatementPtr _GetResultInstanceNodesClassIdsStatement() const override;
        BeSQLite::CachedStatementPtr _GetNodesStatement() const override;
        BeSQLite::CachedStatementPtr _GetCountStatement() const override;

    public:
        ECPRESENTATION_EXPORT static RefCountedPtr<FilteredNodesProvider> Create(NavNodesProviderContextR, BeSQLite::Db&, BeSQLite::StatementCache&, BeMutex&, Utf8String filter);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
