/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include "RulesDrivenProviderContext.h"
#include "QueryExecutor.h"
#include "RulesPreprocessor.h"
#include "DataSourceInfo.h"
#include "UsedClassesListener.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

struct ECDbUsedClassesListenerWrapper;

/*=================================================================================**//**
* @bsiclass                                     Mantas.Kontrimas                04/2018
+===============+===============+===============+===============+===============+======*/
struct UserSettingEntry
{
private:
    Utf8String m_id;
    Json::Value m_value;
public:
    UserSettingEntry(Utf8String id, Json::Value value) : m_id(id), m_value(value) {}
    Utf8StringCR GetId() const {return m_id;}
    JsonValueCR GetValue() const {return m_value;}
};

/*=================================================================================**//**
* An interface for provider index allocator
* @bsiclass                                     Grigas.Petraitis                10/2018
+===============+===============+===============+===============+===============+======*/
struct IProviderIndexAllocator
    {
    virtual uint64_t _AllocateIndex() = 0;
    virtual ~IProviderIndexAllocator() {}
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
    IProviderIndexAllocator* m_providerIndexAllocator;
    mutable HierarchyLevelInfo m_hierarchyLevelInfo;
    mutable DataSourceInfo m_dataSourceInfo;

    // optimization flags
    bool m_isFullLoadDisabled;
    bool m_isUpdatesDisabled;

    // root nodes context
    bool m_isRootNodeContext;
    RootNodeRuleCP m_rootNodeRule;

    // child nodes context
    bool m_isChildNodeContext;
    ChildNodeRuleCP m_childNodeRule;

    // ECDb context
    mutable NavigationQueryBuilder* m_queryBuilder;
    ECDbUsedClassesListenerWrapper* m_usedClassesListener;

    // Update context
    bool m_isUpdateContext;

private:
    void Init();
    ECPRESENTATION_EXPORT NavNodesProviderContext(PresentationRuleSetCR, bool, RuleTargetTree, Utf8String, uint64_t const*, IUserSettings const&, ECExpressionsCache&, 
        RelatedPathsCache&, PolymorphicallyRelatedClassesCache&, JsonNavNodesFactory const&, IHierarchyCache&, INodesProviderFactoryCR, IJsonLocalState const*);
    ECPRESENTATION_EXPORT NavNodesProviderContext(NavNodesProviderContextCR other);
    
public:
    static NavNodesProviderContextPtr Create(PresentationRuleSetCR ruleset, bool holdRuleset, RuleTargetTree targetTree, Utf8String locale, uint64_t const* physicalParentId, 
        IUserSettings const& userSettings, ECExpressionsCache& ecexpressionsCache, RelatedPathsCache& relatedPathsCache, 
        PolymorphicallyRelatedClassesCache& polymorphicallyRelatedClassesCache, JsonNavNodesFactory const& nodesFactory, IHierarchyCache& nodesCache, 
        INodesProviderFactoryCR providerFactory, IJsonLocalState const* localState) 
        {
        return new NavNodesProviderContext(ruleset, holdRuleset, targetTree, locale, physicalParentId, userSettings, ecexpressionsCache, 
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
    void SetVirtualParentNodeId(uint64_t id) {DELETE_AND_CLEAR(m_virtualParentNodeId); m_virtualParentNodeId = (0 != id) ? new uint64_t(id) : nullptr;}
    void SetVirtualParentNode(NavNodeCR node) {SetVirtualParentNodeId(node.GetNodeId());}
    NavNodesProviderPtr CreateHierarchyLevelProvider(NavNodesProviderContextR, JsonNavNodeCP parentNode) const;
    HierarchyLevelInfo const& GetHierarchyLevelInfo() const;
    DataSourceInfo const& GetDataSourceInfo() const;
    bvector<UserSettingEntry> GetRelatedSettings() const;
    IProviderIndexAllocator* GetProviderIndexAllocator() const {return m_providerIndexAllocator;}
    void SetProviderIndexAllocator(IProviderIndexAllocator* allocator) {m_providerIndexAllocator = allocator;}

    // optimization flags
    bool IsFullNodesLoadDisabled() const;
    void SetDisableFullLoad(bool value) {m_isFullLoadDisabled = value;}
    bool IsUpdatesDisabled() const;
    void SetIsUpdatesDisabled(bool value) { m_isUpdatesDisabled = value; }
    bool NeedsFullLoad() const {return !IsFullNodesLoadDisabled();}
    
    // root nodes context
    ECPRESENTATION_EXPORT void SetRootNodeContext(RootNodeRuleCR);
    ECPRESENTATION_EXPORT void SetRootNodeContext(NavNodesProviderContextCR other);
    bool IsRootNodeContext() const {return m_isRootNodeContext;}
    RootNodeRuleCR GetRootNodeRule() const {BeAssert(IsRootNodeContext()); return *m_rootNodeRule;}

    // child nodes context
    ECPRESENTATION_EXPORT void SetChildNodeContext(ChildNodeRuleCP, NavNodeCR virtualParentNode);
    ECPRESENTATION_EXPORT void SetChildNodeContext(NavNodesProviderContextCR other);
    bool IsChildNodeContext() const {return m_isChildNodeContext;}
    ChildNodeRuleCP GetChildNodeRule() const {BeAssert(IsChildNodeContext()); return m_childNodeRule;}

    // ECDb context
    ECPRESENTATION_EXPORT void SetQueryContext(IConnectionManagerCR, IConnectionCR, ECSqlStatementCache const&, CustomFunctionsInjector&, IECDbUsedClassesListener*);
    ECPRESENTATION_EXPORT void SetQueryContext(NavNodesProviderContextCR other);
    NavigationQueryBuilder& GetQueryBuilder() const;
    IUsedClassesListener* GetUsedClassesListener() const;

    // Update context
    ECPRESENTATION_EXPORT void SetUpdateContext(bool isUpdateContext);
    ECPRESENTATION_EXPORT void SetUpdateContext(NavNodesProviderContextCR other);
    bool IsUpdateContext() const {return m_isUpdateContext;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct INodesProviderContextFactory
{
protected:
    virtual NavNodesProviderContextPtr _Create(IConnectionCR, Utf8CP rulesetId, Utf8CP locale, uint64_t const* parentNodeId, 
        ICancelationTokenCP, bool disableUpdates) const = 0;
public:
    virtual ~INodesProviderContextFactory() {}
    NavNodesProviderContextPtr Create(IConnectionCR connection, Utf8CP rulesetId, Utf8CP locale, uint64_t const* parentNodeId, 
        ICancelationTokenCP cancelationToken = nullptr, bool disableUpdates = false) const 
        {
        return _Create(connection, rulesetId, locale, parentNodeId, cancelationToken, disableUpdates);
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
struct DataSourceRelatedSettingsUpdater
    {
    NavNodesProviderContextCR m_context;
    JsonNavNodeCP m_node;
    size_t m_relatedSettingsCountBefore;

    DataSourceRelatedSettingsUpdater(NavNodesProviderContextCR context);
    DataSourceRelatedSettingsUpdater(NavNodesProviderContextCR context, JsonNavNodeCR node);
    ~DataSourceRelatedSettingsUpdater();
    };

/*=================================================================================**//**
* Abstract class for navigation node providers.
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct NavNodesProvider : RefCountedBase, IProviderIndexAllocator
{
    struct SpecificationsVisitor;
    friend struct NavNodesProviderContext;
    friend struct NodesCountContext;
    friend struct NodesCheckContext;
    friend struct DisabledFullNodesLoadContext;

private:
    NavNodesProviderContextPtr m_context;
    uint64_t m_subProviderIndex;
    mutable bool m_nodesInitialized;
    mutable bool m_cachedHasNodesFlag;
    mutable bool m_hasCachedHasNodesFlag;
    mutable uint64_t m_cachedNodesCount;
    mutable bool m_hasCachedNodesCount;
        
protected:
    ECPRESENTATION_EXPORT NavNodesProvider(NavNodesProviderContextCR context);
    HasChildrenFlag AnyChildSpecificationReturnsNodes(JsonNavNode const& parentNode) const;
    NavNodesProviderPtr GetCachedProvider() const;
    void FinalizeNode(JsonNavNodeR, bool customizeLabel) const;
    ECPRESENTATION_EXPORT void InitializeNodes() const;
    
    virtual bool _IsCacheable() const = 0;
    virtual uint64_t _AllocateIndex() override {return m_subProviderIndex++;}
    virtual ProviderNodesInitializationStrategy _GetInitializationStrategy() const {return ProviderNodesInitializationStrategy::Automatic;}
    virtual bool _InitializeNodes() {return true;}

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
    void NotifyNodeChanged(JsonNavNodeCR node) const;
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
    uint64_t _AllocateIndex() override {return 0;}
    bool _IsCacheable() const override {return true;}
    ECPRESENTATION_EXPORT bool _InitializeNodes() override;
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override {return false;}
    bool _HasNodes() const override {return false;}
    size_t _GetNodesCount() const override {return 0;}
    EmptyNavNodesProviderCP _AsEmptyProvider() const override {return this;}
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
private:
    SingleNavNodeProvider(JsonNavNode& node, NavNodesProviderContextCR context) 
        : NavNodesProvider(context), m_node(&node)
        {}
protected:
    bool _IsCacheable() const override {return false;}
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override
        {
        if (0 != index)
            return false;
        node = m_node; 
        return true;
        }
    bool _HasNodes() const override {return true;}
    size_t _GetNodesCount() const override {return 1;}
    SingleNavNodeProviderCP _AsSingleProvider() const override {return this;}
public:
    static RefCountedPtr<SingleNavNodeProvider> Create(JsonNavNode& node, NavNodesProviderContextCR context)
        {
        return WithInitialize(new SingleNavNodeProvider(node, context));
        }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct MultiNavNodesProvider : NavNodesProvider
{
private:
    bvector<NavNodesProviderPtr> m_providers;
    // mutable size_t m_cachedNodesCount;
    // mutable bool m_hasCachedNodesCount;
protected:
    MultiNavNodesProvider(NavNodesProviderContextCR context) : NavNodesProvider(context) /*, m_cachedNodesCount(0), m_hasCachedNodesCount(false) */ {}
    void AddProvider(NavNodesProvider& provider) {m_providers.push_back(&provider);}
    void RemoveProvider(NavNodesProvider& provider) {m_providers.erase(std::find(m_providers.begin(), m_providers.end(), &provider));}
    void SetProviders(bvector<NavNodesProviderPtr> const& providers) {m_providers = providers;}
    void ClearProviders() {m_providers.clear(); }
    bvector<NavNodesProviderPtr>& GetNodeProvidersR() {return m_providers;}
    MultiNavNodesProviderCP _AsMultiProvider() const override {return this;}
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override;
    bool _HasNodes() const override;
    size_t _GetNodesCount() const override;
public:
    bvector<NavNodesProviderPtr> const& GetNodeProviders() const {return m_providers;}
};

typedef RefCountedPtr<struct MultiSpecificationNodesProvider> MultiSpecificationNodesProviderPtr;
/*=================================================================================**//**
* Creates nodes based on supplied specifications.
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct MultiSpecificationNodesProvider : MultiNavNodesProvider
{
private:
    mutable NavNodesProviderPtr m_replaceProvider;

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
* Creates nodes based on CustomNodeSpecifications.
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct CustomNodesProvider : NavNodesProvider
{
    using NavNodesProvider::GetNode;

private:
    mutable NavNodesProviderPtr m_childNodesProvider;
    CustomNodeSpecificationCR m_specification;
    JsonNavNodePtr m_node;

private:
    ECPRESENTATION_EXPORT CustomNodesProvider(NavNodesProviderContextCR context, CustomNodeSpecificationCR specification);
    
protected:
    bool _IsCacheable() const override {return true;}
    bool _InitializeNodes() override;
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override;
    bool _HasNodes() const override;
    size_t _GetNodesCount() const override;

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
    struct PostProcessTask;
    struct CleanupNullProvidersTask;
    struct DeletedExecutorNodeDiffTask;
    struct InsertedExecutorNodeDiffTask;
    struct UpdatedExecutorNodeDiffTask;
    struct Savepoint;

    using NavNodesProvider::GetNode;

private:
    NavigationQueryCPtr m_query;
    bmap<ECClassId, bool> m_usedClassIds;
    NavigationQueryExecutor m_executor;
    size_t m_executorIndex;
    mutable bool m_inProvidersRequest;

private:
    ECPRESENTATION_EXPORT QueryBasedNodesProvider(NavNodesProviderContextCR context, NavigationQuery const& query, bmap<ECClassId, bool> const&);
    bool ShouldReturnChildNodes(JsonNavNode const& node, HasChildrenFlag& hasChildren) const;
    NavNodesProviderPtr CreateProvider(JsonNavNodeR node) const;
    bool HasSimilarNodeInHierarchy(JsonNavNodeCR node, uint64_t parentNodeId) const;
    
protected:
    bool _IsCacheable() const override {return true;}
    ProviderNodesInitializationStrategy _GetInitializationStrategy() const override {return ProviderNodesInitializationStrategy::Manual;}
    bool _InitializeNodes() override;
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override;
    bool _HasNodes() const override;
    size_t _GetNodesCount() const override;

public:
    static RefCountedPtr<QueryBasedNodesProvider> Create(NavNodesProviderContextCR context, 
        NavigationQuery const& query, bmap<ECClassId, bool> const& usedClassIds = bmap<ECClassId, bool>())
        {
        return WithInitialize(new QueryBasedNodesProvider(context, query, usedClassIds));
        }
    NavigationQueryExecutor const& GetExecutor() const {return m_executor;}
    NavigationQueryExecutor& GetExecutorR() {return m_executor;}
    void SetQuery(NavigationQuery const& query, bmap<ECClassId, bool> const&);
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
    void SetupNestedProviders();
    
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
    void InitializeUsedSettings();

protected:
    SQLiteCacheNodesProvider(NavNodesProviderContextCR, BeSQLite::Db&, BeSQLite::StatementCache&);
    BeSQLite::Db& GetCache() const {return m_cache;}
    BeSQLite::StatementCache& GetStatements() const {return m_statements;}
    bool _IsCacheable() const override {return false;}
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override;
    bool _HasNodes() const override;
    size_t _GetNodesCount() const override;

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

END_BENTLEY_ECPRESENTATION_NAMESPACE
