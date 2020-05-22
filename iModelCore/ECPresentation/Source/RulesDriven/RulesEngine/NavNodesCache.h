/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "RulesEngineTypes.h"
#include "NavNodesDataSource.h"
#include "NavNodeProviders.h"
#include "ECExpressionContextsProvider.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
enum class NodeVisibility
    {
    Visible, //!< The node is visible
    Virtual, //!< The node is not visible, but it's children are
    Hidden,  //!< The node and all the hierarchy below it is hidden
    };

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                09/2017
+===============+===============+===============+===============+===============+======*/
enum class NodesCacheType
    {
    Memory,
    Disk,
    };

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                10/2017
+===============+===============+===============+===============+===============+======*/
struct IECSqlStatementCacheProvider
{
protected:
    virtual ~IECSqlStatementCacheProvider() {}
    virtual ECSqlStatementCache& _GetECSqlStatementCache(IConnectionCR) = 0;

public:
    ECSqlStatementCache& GetECSqlStatementCache(IConnectionCR connection) {return _GetECSqlStatementCache(connection);}
};

//=======================================================================================
//! An interface for @ref NavNode objects locater which can find nodes by their keys.
//! @ingroup GROUP_Presentation_Navigation
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct  INavNodeLocater
{
protected:
    virtual JsonNavNodeCPtr _LocateNode(IConnectionCR, Utf8StringCR, Utf8StringCR, NavNodeKeyCR, RulesetVariables const&) const = 0;
public:
    virtual ~INavNodeLocater() {}
    JsonNavNodeCPtr LocateNode(IConnectionCR connection, Utf8StringCR rulesetId, Utf8StringCR locale, NavNodeKeyCR key, RulesetVariables const& variables) const {return _LocateNode(connection, rulesetId, locale, key, variables);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct IHierarchyCache
{
    enum NodeUpdateParts
        {
        UPDATE_NodeItself       = 1 << 0,
        UPDATE_NodeKey          = 1 << 1,
        UPDATE_NodeInstanceKeys = 1 << 2,
        UPDATE_NodeAll = UPDATE_NodeItself | UPDATE_NodeKey | UPDATE_NodeInstanceKeys,
        };

    struct Savepoint : RefCountedBase
    {
    protected:
        virtual void _Cancel() = 0;
    public:
        void Cancel() {_Cancel();}
    };
    typedef RefCountedPtr<Savepoint> SavepointPtr;

protected:
    virtual JsonNavNodePtr _GetNode(uint64_t nodeId) const = 0;
    virtual NodeVisibility _GetNodeVisibility(uint64_t nodeId) const = 0;
    virtual bvector<uint64_t> _GetNodeIndex(uint64_t nodeId) const = 0;

    virtual HierarchyLevelIdentifier _FindHierarchyLevel(Utf8CP connectionId, Utf8CP rulesetId, Utf8CP locale, uint64_t const* virtualParentNodeId) const = 0;
    virtual bvector<DataSourceInfo> _FindDataSources(CombinedHierarchyLevelIdentifier const&, RulesetVariables const&, int partsToGet) const = 0;
    virtual DataSourceInfo _FindDataSource(DataSourceIdentifier const&, RulesetVariables const&, int partsToGet) const = 0;
    virtual DataSourceInfo _FindDataSource(uint64_t nodeId, int partsToGet) const = 0;

    virtual NavNodesProviderPtr _GetCombinedHierarchyLevel(NavNodesProviderContextR context, CombinedHierarchyLevelIdentifier const&, bool) const = 0;
    virtual NavNodesProviderPtr _GetHierarchyLevel(NavNodesProviderContextR context, HierarchyLevelIdentifier const&, bool) const = 0;
    virtual NavNodesProviderPtr _GetDataSource(NavNodesProviderContextR context, DataSourceIdentifier const&, bool, bool) const = 0;
    virtual NavNodesProviderPtr _GetDataSource(NavNodesProviderContextR context, uint64_t nodeId, bool, bool) const = 0;

    virtual void _Cache(HierarchyLevelIdentifier&) = 0;
    virtual void _Cache(DataSourceInfo&) = 0;
    virtual void _Cache(JsonNavNodeR, DataSourceIdentifier const&, bvector<uint64_t> const&, NodeVisibility) = 0;

    virtual void _Update(uint64_t, JsonNavNodeCR, int partsToUpdate) = 0;
    virtual void _Update(DataSourceInfo const&, int partsToUpdate) = 0;

    virtual void _MakePhysical(JsonNavNodeCR) = 0;
    virtual void _MakeVirtual(JsonNavNodeCR) = 0;
    virtual void _MakeHidden(JsonNavNodeCR) = 0;

    virtual bool _IsInitialized(CombinedHierarchyLevelIdentifier const&, RulesetVariables const&) const = 0;
    virtual bool _IsInitialized(HierarchyLevelIdentifier const&, RulesetVariables const&) const = 0;
    virtual bool _IsInitialized(DataSourceIdentifier const&, RulesetVariables const&) const = 0;
    virtual void _FinalizeInitialization(DataSourceIdentifier const&) = 0;

    virtual SavepointPtr _CreateSavepoint() = 0;

    virtual BeMutex& _GetMutex() = 0;

public:
    virtual ~IHierarchyCache() {}

    JsonNavNodePtr GetNode(uint64_t nodeId) const {return _GetNode(nodeId);}
    NodeVisibility GetNodeVisibility(uint64_t nodeId) const {return _GetNodeVisibility(nodeId);}
    bvector<uint64_t> GetNodeIndex(uint64_t nodeId) const {return _GetNodeIndex(nodeId);}

    HierarchyLevelIdentifier FindHierarchyLevel(Utf8CP connectionId, Utf8CP rulesetId, Utf8CP locale, uint64_t const* virtualParentNodeId) const {return _FindHierarchyLevel(connectionId, rulesetId, locale, virtualParentNodeId);}
    bvector<DataSourceInfo> FindDataSources(CombinedHierarchyLevelIdentifier const& hlIdentifier, RulesetVariables const& variables, int partsToGet = 0) const {return _FindDataSources(hlIdentifier, variables, partsToGet);}
    DataSourceInfo FindDataSource(DataSourceIdentifier const& identifier, RulesetVariables const& variables, int partsToGet = 0) const {return _FindDataSource(identifier, variables, partsToGet);}
    DataSourceInfo FindDataSource(uint64_t nodeId, int partsToGet = 0) const {return _FindDataSource(nodeId, partsToGet);}

    //! Get data source for the combined hierarchy level for specified physical parent node
    //  Cached datasource can be deleted if user settings used by cached datasource have changed.
    NavNodesProviderPtr GetCombinedHierarchyLevel(NavNodesProviderContextR context, CombinedHierarchyLevelIdentifier const& info, bool onlyInitialized = true) const {return _GetCombinedHierarchyLevel(context, info, onlyInitialized);}
    //! Get data source for the whole hierarchy level.
    //  Cached datasource can be deleted if user settings used by cached datasource have changed.
    NavNodesProviderPtr GetHierarchyLevel(NavNodesProviderContextR context, HierarchyLevelIdentifier const& info, bool onlyInitialized = true) const {return _GetHierarchyLevel(context, info, onlyInitialized);}
    //! Get partial data source (which is a subset of a hierarchy level).
    //  Cached datasource can be deleted if user settings used by cached datasource have changed.
    NavNodesProviderPtr GetDataSource(NavNodesProviderContextR context, DataSourceIdentifier const& info, bool onlyInitialized = true, bool onlyVisible = false) const {return _GetDataSource(context, info, onlyInitialized, onlyVisible);}
    //! Get partial data source the node with the supplied ID belongs to.
    //  Cached datasource can be deleted if user settings used by cached datasource have changed.
    NavNodesProviderPtr GetDataSource(NavNodesProviderContextR context, uint64_t nodeId, bool onlyInitialized = true, bool onlyVisible = false) const {return _GetDataSource(context, nodeId, onlyInitialized, onlyVisible);}

    void Cache(HierarchyLevelIdentifier& info) {_Cache(info);}
    void Cache(DataSourceInfo& info) {_Cache(info);}
    void Cache(JsonNavNodeR node, DataSourceIdentifier const& dsInfo, bvector<uint64_t> const& index, NodeVisibility visibility) {_Cache(node, dsInfo, index, visibility);}
    void Cache(JsonNavNodeR node, DataSourceIdentifier const& dsInfo, uint64_t index, NodeVisibility visibility) {_Cache(node, dsInfo, {index}, visibility);}

    void Update(uint64_t nodeId, JsonNavNodeCR node, int partsToUpdate) {_Update(nodeId, node, partsToUpdate);}
    void Update(DataSourceInfo const& info, int partsToUpdate) {_Update(info, partsToUpdate);}

    void MakePhysical(JsonNavNodeCR node) {_MakePhysical(node);}
    void MakeVirtual(JsonNavNodeCR node) {_MakeVirtual(node);}
    void MakeHidden(JsonNavNodeCR node) {_MakeHidden(node);}

    bool IsInitialized(CombinedHierarchyLevelIdentifier const& info, RulesetVariables const& variables) const {return _IsInitialized(info, variables);}
    bool IsInitialized(HierarchyLevelIdentifier const& info, RulesetVariables const& variables) const {return _IsInitialized(info, variables);}
    bool IsInitialized(DataSourceIdentifier const& info, RulesetVariables const& variables) const {return _IsInitialized(info, variables);}
    void FinalizeInitialization(DataSourceIdentifier const& info) {_FinalizeInitialization(info);}

    SavepointPtr CreateSavepoint() {return _CreateSavepoint();}

    BeMutex& GetMutex() { return _GetMutex(); }
};

#define NODESCACHE_TABLENAME_HierarchyLevels    "HierarchyLevels"
#define NODESCACHE_TABLENAME_DataSources        "DataSources"
#define NODESCACHE_TABLENAME_DataSourceClasses  "DataSourceClasses"
#define NODESCACHE_TABLENAME_Variables          "Variables"
#define NODESCACHE_TABLENAME_Nodes              "Nodes"
#define NODESCACHE_TABLENAME_NodeKeys           "NodeKeys"
#define NODESCACHE_TABLENAME_NodeInstances      "NodeInstances"
#define NODESCACHE_TABLENAME_NodesOrder         "NodesOrder"
#define NODESCACHE_TABLENAME_Connections        "Connections"
#define NODESCACHE_TABLENAME_Rulesets           "Rulesets"

#define NODESCACHE_FUNCNAME_ConcatBinaryIndex   "ConcatBinaryIndex"
#define NODESCACHE_FUNCNAME_VariablesMatch      "VariablesMatch"

#define NODESCACHE_VARIABLES_Threshold           "5"

// Note: nodes cache uses a small in-memory structure of size NODESCACHE_QUICK_Size to store
// the most recently used data sources whose size is at least NODESCACHE_QUICK_Boundary nodes.
// The NODESCACHE_QUICK_Boundary should match the page size used in rules-driven controls to make
// sure we reuse the same data source for different pages.
#define NODESCACHE_QUICK_Boundary               20
#define NODESCACHE_QUICK_Size                   10

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct NodesCache : IHierarchyCache, INavNodeLocater
{
    struct UniquePtrEnabler;
    struct Savepoint;
    struct QuickDataSourceCacheKey;

    using IHierarchyCache::FindHierarchyLevel;

private:
    JsonNavNodesFactory const& m_nodesFactory;
    INodesProviderContextFactoryCR m_contextFactory;
    IConnectionCR m_connection;
    bool m_cacheUpdateData;
    bool m_tempCache;
    mutable BeSQLite::Db m_db;
    mutable BeSQLite::StatementCache m_statements;
    mutable bvector<bpair<QuickDataSourceCacheKey, NavNodesProviderPtr>> m_quickDataSourceCache;
    mutable bvector<bpair<uint64_t, JsonNavNodePtr>> m_quickNodesCache;
    uint64_t m_sizeLimit;
    bvector<BeSQLite::ScalarFunction*> m_customFunctions;
    mutable BeMutex m_mutex;
    mutable BeMutex m_quickCacheMutex;

private:
    ECPRESENTATION_EXPORT NodesCache(IConnectionCR, JsonNavNodesFactoryCR, INodesProviderContextFactoryCR, bool);
    BentleyStatus Initialize(BeFileNameCR directory, IConnectionCR, NodesCacheType);

    void CacheNode(DataSourceIdentifier const&, NavNodeR, bvector<uint64_t> const&, NodeVisibility visibility);
    void CacheEmptyHierarchyLevel(HierarchyLevelIdentifier& info);
    void CacheEmptyDataSource(DataSourceIdentifier&, DataSourceFilter const&, uint64_t variablesId, Utf8StringCR specificationHash, Utf8StringCR nodeTypes, bool isPartial);
    void CacheRelatedClassIds(uint64_t datasourceId, bmap<ECClassId, bool> const&);
    uint64_t CacheRelatedVariables(Utf8StringCR rulesetId, RulesetVariables const& variables);
    bool HasRelatedSettingsChanged(uint64_t datasourceId, Utf8StringCR rulesetId) const;
    void CacheNodeKey(NavNodeCR);
    void CacheNodeInstanceKeys(NavNodeCR);
    void ChangeVisibility(uint64_t nodeId, NodeVisibility visibility);
    void LimitCacheSize();
    void ResetDataSource(DataSourceIdentifier const&);
    bvector<DataSourceIdentifier> GetDataSourcesWithChangedUserSettings(CombinedHierarchyLevelIdentifier const&) const;
    bvector<DataSourceIdentifier> GetDataSourcesWithChangedUserSettings(HierarchyLevelIdentifier const&) const;
    HierarchyLevelIdentifier GetParentHierarchyLevelIdentifier(uint64_t nodeId) const;
    DataSourceInfo CreateDataSourceInfo(DataSourceIdentifier, int partsToGet) const;

    void AddQuick(CombinedHierarchyLevelIdentifier, NavNodesProviderR, RulesetVariables const&);
    void RemoveQuick(CombinedHierarchyLevelIdentifier const&, RulesetVariables const* = nullptr);
    NavNodesProviderPtr GetQuick(CombinedHierarchyLevelIdentifier const&, RulesetVariables const&) const;

    void AddQuick(JsonNavNodeR) const;
    void RemoveQuick(uint64_t) const;
    void RemoveQuick(std::function<bool(JsonNavNodeCR)> const&) const;
    JsonNavNodePtr GetQuick(uint64_t) const;

protected:
    // IHierarchyCache
    ECPRESENTATION_EXPORT JsonNavNodePtr _GetNode(uint64_t) const override;
    ECPRESENTATION_EXPORT NodeVisibility _GetNodeVisibility(uint64_t nodeId) const override;
    ECPRESENTATION_EXPORT bvector<uint64_t> _GetNodeIndex(uint64_t nodeId) const override;
    ECPRESENTATION_EXPORT HierarchyLevelIdentifier _FindHierarchyLevel(Utf8CP connectionId, Utf8CP rulesetId, Utf8CP locale, uint64_t const* virtualParentNodeId) const override;
    ECPRESENTATION_EXPORT bvector<DataSourceInfo> _FindDataSources(CombinedHierarchyLevelIdentifier const&, RulesetVariables const&, int) const override;
    ECPRESENTATION_EXPORT DataSourceInfo _FindDataSource(DataSourceIdentifier const&, RulesetVariables const&, int) const override;
    ECPRESENTATION_EXPORT DataSourceInfo _FindDataSource(uint64_t nodeId, int) const override;
    ECPRESENTATION_EXPORT NavNodesProviderPtr _GetCombinedHierarchyLevel(NavNodesProviderContextR, CombinedHierarchyLevelIdentifier const&, bool) const override;
    ECPRESENTATION_EXPORT NavNodesProviderPtr _GetHierarchyLevel(NavNodesProviderContextR, HierarchyLevelIdentifier const&, bool) const override;
    ECPRESENTATION_EXPORT NavNodesProviderPtr _GetDataSource(NavNodesProviderContextR, DataSourceIdentifier const&, bool, bool) const override;
    ECPRESENTATION_EXPORT NavNodesProviderPtr _GetDataSource(NavNodesProviderContextR, uint64_t nodeId, bool, bool) const override;
    ECPRESENTATION_EXPORT void _Cache(HierarchyLevelIdentifier&) override;
    ECPRESENTATION_EXPORT void _Cache(DataSourceInfo&) override;
    ECPRESENTATION_EXPORT void _Cache(JsonNavNodeR, DataSourceIdentifier const&, bvector<uint64_t> const&, NodeVisibility) override;
    ECPRESENTATION_EXPORT void _Update(uint64_t nodeId, JsonNavNodeCR, int partsToUpdate) override;
    ECPRESENTATION_EXPORT void _Update(DataSourceInfo const&, int partsToUpdate) override;
    ECPRESENTATION_EXPORT void _MakePhysical(JsonNavNodeCR) override;
    ECPRESENTATION_EXPORT void _MakeVirtual(JsonNavNodeCR) override;
    ECPRESENTATION_EXPORT void _MakeHidden(JsonNavNodeCR) override;
    ECPRESENTATION_EXPORT bool _IsInitialized(CombinedHierarchyLevelIdentifier const&, RulesetVariables const&) const override;
    ECPRESENTATION_EXPORT bool _IsInitialized(HierarchyLevelIdentifier const&, RulesetVariables const&) const override;
    ECPRESENTATION_EXPORT bool _IsInitialized(DataSourceIdentifier const&, RulesetVariables const&) const override;
    ECPRESENTATION_EXPORT void _FinalizeInitialization(DataSourceIdentifier const&) override;
    ECPRESENTATION_EXPORT SavepointPtr _CreateSavepoint() override;
    BeMutex& _GetMutex() override { return m_mutex; }

    // INavNodeLocater
    ECPRESENTATION_EXPORT JsonNavNodeCPtr _LocateNode(IConnectionCR, Utf8StringCR, Utf8StringCR, NavNodeKeyCR, RulesetVariables const&) const override;

public:
    ECPRESENTATION_EXPORT static std::unique_ptr<NodesCache> Create(IConnectionCR, BeFileNameCR, JsonNavNodesFactoryCR, INodesProviderContextFactoryCR,
        NodesCacheType type, bool cacheUpdateData);
    ECPRESENTATION_EXPORT ~NodesCache();

    ECPRESENTATION_EXPORT void CacheHierarchyLevel(CombinedHierarchyLevelIdentifier const&, NavNodesProviderR, RulesetVariables const&);

    ECPRESENTATION_EXPORT bool IsNodeCached(uint64_t nodeId) const;
    ECPRESENTATION_EXPORT bool IsHierarchyLevelCached(Utf8CP rulesetId, Utf8CP locale) const;
    ECPRESENTATION_EXPORT bool IsHierarchyLevelCached(uint64_t parentNodeId) const;

    ECPRESENTATION_EXPORT HierarchyLevelIdentifier FindHierarchyLevel(uint64_t id) const;

    ECPRESENTATION_EXPORT BeSQLite::BeGuid CreateRemovalId(CombinedHierarchyLevelIdentifier const&);
    ECPRESENTATION_EXPORT void RemoveHierarchyLevel(BeSQLite::BeGuidCR removalId);

    ECPRESENTATION_EXPORT void RemapNodeIds(bmap<uint64_t, uint64_t> const&);
    ECPRESENTATION_EXPORT bool HasParentNode(uint64_t nodeId, bset<uint64_t> const& parentNodeIds) const;

    ECPRESENTATION_EXPORT bvector<HierarchyLevelIdentifier> GetRelatedHierarchyLevels(IConnectionCR connection, bset<ECInstanceKey> const&) const;
    ECPRESENTATION_EXPORT bvector<HierarchyLevelIdentifier> GetRelatedHierarchyLevels(Utf8CP rulesetId, Utf8CP settingId) const;

    ECPRESENTATION_EXPORT void Clear(Utf8CP rulesetId = nullptr);
    ECPRESENTATION_EXPORT void Persist();

    ECPRESENTATION_EXPORT void OnRulesetUsed(PresentationRuleSetCR);
    ECPRESENTATION_EXPORT void OnRulesetVariablesUsed(RulesetVariables const&, Utf8StringCR);
    BeSQLite::Db const& GetDb() const {return m_db;}
    void SetCacheFileSizeLimit(uint64_t size) {m_sizeLimit = size;}

    ECPRESENTATION_EXPORT NavNodesProviderPtr GetUndeterminedNodesProvider(IConnectionCR connection, Utf8CP ruleSetId, Utf8CP locale, RulesetVariables const& variables) const;
    ECPRESENTATION_EXPORT NavNodesProviderPtr GetFilteredNodesProvider(Utf8CP filter, IConnectionCR connection, Utf8CP ruleSetId, Utf8CP locale, RulesetVariables const& variables) const;
};

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                03/2020
+===============+===============+===============+===============+===============+======*/
struct INodesCacheManager
{
protected:
    virtual NodesCache* _GetCache(Utf8StringCR connectionId) const = 0;
    virtual void _ClearCaches(Utf8CP rulesetId) const = 0;
    virtual bvector<NodesCache*> _GetAllNodeCaches() const = 0;

public:
    virtual ~INodesCacheManager() {}
    NodesCache* GetCache(Utf8StringCR connectionId) const { return _GetCache(connectionId); }
    void ClearCaches(Utf8CP rulesetId) const { _ClearCaches(rulesetId); }
    bvector<NodesCache*> GetAllNodeCaches() const { return _GetAllNodeCaches(); }
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
