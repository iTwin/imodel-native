/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/NavNodeProviders.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    NavNodesProviderCP m_baseProvider;

    // optimization flags
    bool m_isNodesCount;
    bool m_isNodesCheck;
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
    void SetPhysicalParentNode(NavNodeCR node) {DELETE_AND_CLEAR(m_physicalParentNodeId); m_physicalParentNodeId = new uint64_t(node.GetNodeId());}
    ECPRESENTATION_EXPORT JsonNavNodeCPtr GetVirtualParentNode() const;
    uint64_t const* GetVirtualParentNodeId() const {return m_virtualParentNodeId;}
    void SetVirtualParentNode(NavNodeCR node) {DELETE_AND_CLEAR(m_virtualParentNodeId); m_virtualParentNodeId = new uint64_t(node.GetNodeId());}
    NavNodesProviderCP GetBaseProvider() const {return m_baseProvider;}
    void SetBaseProvider(NavNodesProviderCR provider) {m_baseProvider = &provider;}
    NavNodesProviderPtr CreateProvider(NavNodesProviderContextR, JsonNavNodeCP parentNode) const;
    IUsedUserSettingsListener& GetUsedSettingsListener() const;
    bvector<UserSettingEntry> GetRelatedSettings() const;

    // optimization flags
    bool IsNodesCount(bool checkBase = true) const;
    void SetIsNodesCount(bool value) {m_isNodesCount = value;}
    bool IsNodesCheck(bool checkBase = true) const;
    void SetIsNodesCheck(bool value) {m_isNodesCheck = value;}
    bool IsFullNodesLoadDisabled(bool checkBase = true) const;
    void SetDisableFullLoad(bool value) {m_isFullLoadDisabled = value;}
    bool IsUpdatesDisabled(bool checkBase = true) const;
    void SetIsUpdatesDisabled(bool value) { m_isUpdatesDisabled = value; }
    bool NeedsFullLoad() const {return !IsNodesCheck() && !IsNodesCount() && !IsFullNodesLoadDisabled();}
    
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
    DataSourceInfo const& m_datasourceInfo;
    NavNodesProviderContextCR m_context;
    size_t m_relatedSettingsCountBefore;

    DataSourceRelatedSettingsUpdater(DataSourceInfo const& info, NavNodesProviderContextCR context);
    ~DataSourceRelatedSettingsUpdater();
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

private:
    NavNodesProviderContextPtr m_context;
    DataSourceInfo m_datasourceInfo;

private:
    bool GetPositionOffsetInternal(size_t& offset, NavNodesProviderCR base, NavNodesProviderCR provider) const;
    
protected:
    ECPRESENTATION_EXPORT NavNodesProvider(NavNodesProviderContextCR context);
    DataSourceInfo& GetDataSourceInfo() {return m_datasourceInfo;}
    void SetDataSourceInfo(DataSourceInfo info) {m_datasourceInfo = info;}
    HasChildrenFlag AnyChildSpecificationReturnsNodes(JsonNavNode const& parentNode, bool isParentPhysical) const;
    NavNodesProviderCR GetRootBaseProvider() const;
    bool GetPositionOffset(size_t& offset, NavNodesProviderCR provider) const;
    void FinalizeNode(JsonNavNodeR, bool customizeLabel) const;

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
    DataSourceInfo const& GetDataSourceInfo() const {return m_datasourceInfo;}
    MultiNavNodesProviderCP AsMultiProvider() const {return _AsMultiProvider();}
    EmptyNavNodesProviderCP AsEmptyProvider() const {return _AsEmptyProvider();}
    SingleNavNodeProviderCP AsSingleProvider() const {return _AsSingleProvider();}
    ECPRESENTATION_EXPORT void FinalizeNodes();
    ECPRESENTATION_EXPORT bool GetNode(JsonNavNodePtr& node, size_t index) const;
    ECPRESENTATION_EXPORT size_t GetNodesCount() const;
    ECPRESENTATION_EXPORT bool HasNodes() const;
    void DetermineChildren(JsonNavNodeR) const;
    void NotifyNodeChanged(JsonNavNodeCR node) const;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct INodesProviderFactory
{
protected:
    virtual NavNodesProviderPtr _CreateForHierarchyLevel(NavNodesProviderContextR, JsonNavNodeCP) const = 0;
    virtual NavNodesProviderPtr _CreateForVirtualParent(NavNodesProviderContextR, JsonNavNodeCP) const = 0;
public:
    virtual ~INodesProviderFactory() {}
    NavNodesProviderPtr CreateForHierarchyLevel(NavNodesProviderContextR context, JsonNavNodeCP parent) const {return _CreateForHierarchyLevel(context, parent);}
    NavNodesProviderPtr CreateForVirtualParent(NavNodesProviderContextR context, JsonNavNodeCP parent) const {return _CreateForVirtualParent(context, parent);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
+===============+===============+===============+===============+===============+======*/
struct NodesCountContext
    {
    NavNodesProviderCR m_provider;
    bool m_wasNodesCount;
    NodesCountContext(NavNodesProviderCR provider) 
        : m_provider(provider)
        {
        m_wasNodesCount = m_provider.GetContext().IsNodesCount(false);
        m_provider.GetContextR().SetIsNodesCount(true);
        }
    ~NodesCountContext() {m_provider.GetContextR().SetIsNodesCount(m_wasNodesCount);}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
+===============+===============+===============+===============+===============+======*/
struct NodesCheckContext
    {
    NavNodesProviderCR m_provider;
    bool m_wasNodesCheck;
    NodesCheckContext(NavNodesProviderCR provider) 
        : m_provider(provider)
        {
        m_wasNodesCheck = m_provider.GetContext().IsNodesCheck(false);
        m_provider.GetContextR().SetIsNodesCheck(true);
        }
    ~NodesCheckContext() {m_provider.GetContextR().SetIsNodesCheck(m_wasNodesCheck);}
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
        m_wasFullLoadDisabled = m_provider.GetContext().IsFullNodesLoadDisabled(false);
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
    EmptyNavNodesProvider(NavNodesProviderContextCR context);
protected:
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override {return false;}
    bool _HasNodes() const override {return false;}
    size_t _GetNodesCount() const override {return 0;}
    EmptyNavNodesProviderCP _AsEmptyProvider() const override {return this;}
public:
    static RefCountedPtr<EmptyNavNodesProvider> Create(NavNodesProviderContextCR context)
        {
        return new EmptyNavNodesProvider(context);
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
        return new SingleNavNodeProvider(node, context);
        }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct MultiNavNodesProvider : NavNodesProvider
{
private:
    bvector<NavNodesProviderPtr> m_providers;
protected:
    MultiNavNodesProvider(NavNodesProviderContextCR context) : NavNodesProvider(context) {}
    void AddProvider(NavNodesProvider& provider) {m_providers.push_back(&provider);}
    void RemoveProvider(NavNodesProvider& provider) {m_providers.erase(std::find(m_providers.begin(), m_providers.end(), &provider));}
    void SetProviders(bvector<NavNodesProviderPtr> const& providers) {m_providers = providers;}
    void ClearProviders() {m_providers.clear(); }
    bvector<NavNodesProviderPtr>& GetNodeProvidersR() {return m_providers;}
    MultiNavNodesProviderCP _AsMultiProvider() const override {return this;}
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override;
    bool _HasNodes() const override;
    size_t _GetNodesCount() const override;
    virtual void _OnNodeProvidersRequest() const {}
public:
    bvector<NavNodesProviderPtr> const& GetNodeProviders() const {_OnNodeProvidersRequest(); return m_providers;}
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
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override;
    size_t _GetNodesCount() const override;

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
    void Initialize();

protected:
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override;
    bool _HasNodes() const override;
    size_t _GetNodesCount() const override;

public:
    static RefCountedPtr<CustomNodesProvider> Create(NavNodesProviderContextCR context, CustomNodeSpecificationCR specification)
        {
        return new CustomNodesProvider(context, specification);
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
    void Initialize();
    NavNodesProviderPtr CreateProvider(JsonNavNodeR node) const;
    bool HasSimilarNodeInHierarchy(JsonNavNodeCR node, uint64_t parentNodeId) const;
    
protected:
    bool _GetNode(JsonNavNodePtr& node, size_t index) const override;
    bool _HasNodes() const override;
    size_t _GetNodesCount() const override;
    void _OnNodeProvidersRequest() const override;

public:
    static RefCountedPtr<QueryBasedNodesProvider> Create(NavNodesProviderContextCR context, 
        NavigationQuery const& query, bmap<ECClassId, bool> const& usedClassIds = bmap<ECClassId, bool>())
        {
        return new QueryBasedNodesProvider(context, query, usedClassIds);
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
    void SetupNestedProviders(bool fresh);
    
protected:
    bool _HasNodes() const override;

public:
    static RefCountedPtr<QueryBasedSpecificationNodesProvider> Create(NavNodesProviderContextCR context, ChildNodeSpecificationCR specification)
        {
        return new QueryBasedSpecificationNodesProvider(context, specification);
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
struct CachedHierarchyLevelProvider : SQLiteCacheNodesProvider
{
private:
    Utf8String m_datasourceIds;

private:
    CachedHierarchyLevelProvider(NavNodesProviderContextCR, BeSQLite::Db&, BeSQLite::StatementCache&, uint64_t const* physicalParentNodeId);
    void InitDatasourceIds(uint64_t const* physicalParentNodeId);

protected:
    BeSQLite::CachedStatementPtr _GetNodesStatement() const override;
    BeSQLite::CachedStatementPtr _GetCountStatement() const override;

public:
    ~CachedHierarchyLevelProvider();
    static RefCountedPtr<CachedHierarchyLevelProvider> Create(NavNodesProviderContextCR context, BeSQLite::Db& cache, BeSQLite::StatementCache& statements, uint64_t const* physicalParentNodeId)
        {
        return new CachedHierarchyLevelProvider(context, cache, statements, physicalParentNodeId);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct CachedVirtualNodeChildrenProvider : SQLiteCacheNodesProvider
{
private:
    CachedVirtualNodeChildrenProvider(NavNodesProviderContextCR, DataSourceInfo dsInfo, BeSQLite::Db&, BeSQLite::StatementCache&);

protected:
    BeSQLite::CachedStatementPtr _GetNodesStatement() const override;
    BeSQLite::CachedStatementPtr _GetCountStatement() const override;

public:
    static RefCountedPtr<CachedVirtualNodeChildrenProvider> Create(NavNodesProviderContextCR context, DataSourceInfo dsInfo, BeSQLite::Db& cache, BeSQLite::StatementCache& statements)
        {
        return new CachedVirtualNodeChildrenProvider(context, dsInfo, cache, statements);
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
        return new NodesWithUndeterminedChildrenProvider(context, cache, statements);
        }
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
