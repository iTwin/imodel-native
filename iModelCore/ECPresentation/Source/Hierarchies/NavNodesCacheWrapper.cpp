/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "NavNodesCacheWrapper.h"

#define MERGE_QUERY_IGNORE(tableName) "INSERT OR IGNORE INTO [" ATTACHED_DB_ALIAS "].[" tableName "] SELECT * FROM [main].[" tableName "] "
#define MERGE_QUERY_REPLACE(tableName) "INSERT OR REPLACE INTO [" ATTACHED_DB_ALIAS "].[" tableName "] SELECT * FROM [main].[" tableName "] "

#define MERGE_RULESETS_TO_ATTACHED_QUERY MERGE_QUERY_REPLACE(NODESCACHE_TABLENAME_Rulesets)
#define MERGE_RULESETS_VARIABLES_TO_ATTACHED_QUERY MERGE_QUERY_IGNORE(NODESCACHE_TABLENAME_Variables)
#define MERGE_HIERARCHYLEVELS_TO_ATTACHED_QUERY MERGE_QUERY_REPLACE(NODESCACHE_TABLENAME_HierarchyLevels)
#define MERGE_DATASOURCES_TO_ATTACHED_QUERY MERGE_QUERY_REPLACE(NODESCACHE_TABLENAME_DataSources)
#define MERGE_DATASOURCES_CLASSES_TO_ATTACHED_QUERY MERGE_QUERY_IGNORE(NODESCACHE_TABLENAME_DataSourceClasses)
#define MERGE_NODES_TO_ATTACHED_QUERY MERGE_QUERY_REPLACE(NODESCACHE_TABLENAME_Nodes)
#define MERGE_NODES_KEYS_TO_ATTACHED_QUERY MERGE_QUERY_IGNORE(NODESCACHE_TABLENAME_NodeKeys)
#define MERGE_NODES_INSTANCES_TO_ATTACHED_QUERY MERGE_QUERY_IGNORE(NODESCACHE_TABLENAME_NodeInstances)
#define MERGE_MERGED_NODES_TO_ATTACHED_QUERY MERGE_QUERY_IGNORE(NODESCACHE_TABLENAME_MergedNodes)
#define MERGE_DATASOURCE_NODES_TO_ATTACHED_QUERY MERGE_QUERY_IGNORE(NODESCACHE_TABLENAME_DataSourceNodes)

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesCacheMerger
    {
    private:
        Db& m_db;
        std::unique_ptr<BeSQLite::Savepoint> m_savepoint;

    private:
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        void ExecuteQuery(Utf8CP query)
            {
            SAFE_CACHED_STATEMENT(stmt, m_db, query, );
            VALIDATE_STEP(stmt, BE_SQLITE_DONE);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        void MergeAllDataSourceClasses()
            {
            static Utf8CP query =
                " DELETE FROM [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_DataSourceClasses "] "
                " WHERE [DataSourceId] IN (SELECT [DataSourceId] FROM [main].[" NODESCACHE_TABLENAME_DataSourceClasses "]) ";
            ExecuteQuery(query);
            ExecuteQuery(MERGE_DATASOURCES_CLASSES_TO_ATTACHED_QUERY);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        void MergeAllNodeKeys()
            {
            static Utf8CP query =
                " DELETE FROM [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_NodeKeys "] "
                " WHERE [NodeId] IN (SELECT [NodeId] FROM [main].[" NODESCACHE_TABLENAME_NodeKeys "]) ";
            ExecuteQuery(query);
            ExecuteQuery(MERGE_NODES_KEYS_TO_ATTACHED_QUERY);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        void MergeAllNodeInstances()
            {
            static Utf8CP query =
                " DELETE FROM [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_NodeInstances "] "
                " WHERE [NodeId] IN (SELECT [NodeId] FROM [main].[" NODESCACHE_TABLENAME_NodeInstances "]) ";
            ExecuteQuery(query);
            ExecuteQuery(MERGE_NODES_INSTANCES_TO_ATTACHED_QUERY);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        void MergeAllMergedNodes()
            {
            static Utf8CP query =
                " DELETE FROM [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_MergedNodes "] "
                " WHERE [MergingNodeId] IN (SELECT [MergingNodeId] FROM [main].[" NODESCACHE_TABLENAME_MergedNodes "]) ";
            ExecuteQuery(query);
            ExecuteQuery(MERGE_MERGED_NODES_TO_ATTACHED_QUERY);
            }

    public:
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        NodesCacheMerger(Db& db)
            : m_db(db)
            {
            m_db.GetDefaultTransaction()->Commit();
            m_db.TryExecuteSql("PRAGMA foreign_keys=off");
            m_savepoint = std::make_unique<BeSQLite::Savepoint>(m_db, "Merge", true, BeSQLiteTxnMode::Immediate);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        ~NodesCacheMerger()
            {
            m_savepoint->Commit();
            m_db.TryExecuteSql("PRAGMA foreign_keys=on");
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        void Merge()
            {
            ExecuteQuery(MERGE_RULESETS_VARIABLES_TO_ATTACHED_QUERY);
            ExecuteQuery(MERGE_HIERARCHYLEVELS_TO_ATTACHED_QUERY);
            ExecuteQuery(MERGE_DATASOURCES_TO_ATTACHED_QUERY);
            MergeAllDataSourceClasses();
            ExecuteQuery(MERGE_NODES_TO_ATTACHED_QUERY);
            MergeAllNodeKeys();
            MergeAllNodeInstances();
            MergeAllMergedNodes();
            ExecuteQuery(MERGE_DATASOURCE_NODES_TO_ATTACHED_QUERY);
            }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NodesCacheWrapper::~NodesCacheWrapper()
    {
    PersistMemoryData();
    if (m_memoryCache)
        {
        m_memoryCache->GetDb().DetachDb(ATTACHED_DB_ALIAS);
        m_memoryCache = nullptr;
        m_diskCache.Optimize();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IHierarchyCache::SavepointPtr NodesCacheWrapper::_CreateSavepoint(bool)
    {
    InitializeMemoryCache();
    return m_memoryCache->CreateSavepointInternal(BeSQLiteTxnMode::Deferred, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NodesCacheWrapper::_GetNode(BeGuidCR nodeId) const
    {
    NavNodePtr node;
    if (IsMemoryCacheInitialized())
        node = m_memoryCache->GetNode(nodeId);

    if (node.IsValid())
        return node;

    return m_diskCache.GetNode(nodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NodeVisibility NodesCacheWrapper::_GetNodeVisibility(BeGuidCR nodeId, RulesetVariables const& contextVariables, Utf8StringCR instanceFilter) const
    {
    if (IsMemoryCacheInitialized() && NodesCacheHelpers::NodeExists(m_memoryCache->GetDb(), nodeId))
        return m_memoryCache->GetNodeVisibility(nodeId, contextVariables, instanceFilter);

    return m_diskCache.GetNodeVisibility(nodeId, contextVariables, instanceFilter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NodesCacheWrapper::_GetPhysicalParentNode(BeGuidCR nodeId, RulesetVariables const& contextVariables, Utf8StringCR instanceFilter) const
    {
    if (IsMemoryCacheInitialized() && NodesCacheHelpers::NodeExists(m_memoryCache->GetDb(), nodeId))
        return m_memoryCache->GetPhysicalParentNode(nodeId, contextVariables, instanceFilter);

    return m_diskCache.GetPhysicalParentNode(nodeId, contextVariables, instanceFilter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid NodesCacheWrapper::_GetVirtualParentNodeId(BeGuidCR nodeId) const
    {
    if (IsMemoryCacheInitialized() && NodesCacheHelpers::NodeExists(m_memoryCache->GetDb(), nodeId))
        return m_memoryCache->GetVirtualParentNodeId(nodeId);

    return m_diskCache.GetVirtualParentNodeId(nodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<uint64_t> NodesCacheWrapper::_GetNodeIndex(BeGuidCR hierarchyLevelId, BeGuidCR nodeId, RulesetVariables const& contextVariables, Utf8StringCR instanceFilter) const
    {
    if (IsMemoryCacheInitialized() && NodesCacheHelpers::NodeExists(m_memoryCache->GetDb(), nodeId))
        return m_memoryCache->GetNodeIndex(hierarchyLevelId, nodeId, contextVariables, instanceFilter);

    return m_diskCache.GetNodeIndex(hierarchyLevelId, nodeId, contextVariables, instanceFilter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid NodesCacheWrapper::_FindHierarchyLevelId(Utf8CP connectionId, Utf8CP rulesetId, BeGuidCR virtualParentNodeId, BeGuidCR removalId) const
    {
    if (IsMemoryCacheInitialized() && ShouldTakeHierarchyLevelFromMemory(rulesetId, virtualParentNodeId))
        return m_memoryCache->FindHierarchyLevelId(connectionId, rulesetId, virtualParentNodeId, removalId);

    return m_diskCache.FindHierarchyLevelId(connectionId, rulesetId, virtualParentNodeId, removalId);
    }

#ifdef wip_enable_display_label_postprocessor
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void InsertDataSourceInfoByIndex(bvector<DataSourceInfo>& list, DataSourceInfo const& dataSourceInfo)
    {
    for (size_t i = 0; i < list.size(); ++i)
        {
        if (NodesCacheHelpers::CompareIndexes(list[i].GetIdentifier().GetIndex(), dataSourceInfo.GetIdentifier().GetIndex()) > 0)
            {
            list.insert(list.begin() + i, dataSourceInfo);
            return;
            }
        }
    list.push_back(dataSourceInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DataSourceInfo> NodesCacheWrapper::_FindDataSources(CombinedHierarchyLevelIdentifier const& identifier, RulesetVariables const& variables, int partsToGet) const
    {
    bvector<DataSourceInfo> memoryDataSources;
    if (IsMemoryCacheInitialized() && HierarchyLevelExistInMemory(identifier.GetRulesetId().c_str(), identifier.GetPhysicalParentNodeId(), true))
        memoryDataSources = m_memoryCache->FindDataSources(identifier, variables, partsToGet);

    bvector<DataSourceInfo> persistedDataSources = m_diskCache.FindDataSources(identifier, variables, partsToGet);
    for (DataSourceInfo const& memoryDataSource : memoryDataSources)
        {
        auto iter = std::find_if(persistedDataSources.begin(), persistedDataSources.end(), [&memoryDataSource](DataSourceInfo const& dsInfo) {return dsInfo.GetIdentifier() == memoryDataSource.GetIdentifier(); });
        if (persistedDataSources.end() == iter)
            InsertDataSourceInfoByIndex(persistedDataSources, memoryDataSource);
        }

    return persistedDataSources;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid NodesCacheWrapper::_FindPhysicalHierarchyLevelId(CombinedHierarchyLevelIdentifier const& identifier) const
    {
    if (IsMemoryCacheInitialized() && HierarchyLevelExistInMemory(identifier.GetRulesetId().c_str(), identifier.GetPhysicalParentNodeId()))
        return m_memoryCache->FindPhysicalHierarchyLevelId(identifier);

    return m_diskCache.FindPhysicalHierarchyLevelId(identifier);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCacheWrapper::_IsHierarchyLevelLocked(CombinedHierarchyLevelIdentifier const& identifier) const
    {
    return m_diskCache.IsHierarchyLevelLocked(identifier);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceInfo NodesCacheWrapper::_FindDataSource(DataSourceIdentifier const& identifier, RulesetVariables const& variables, int partsToGet) const
    {
    DataSourceInfo info;
    bool shouldTakeFromMemory = ShouldTakeDataSourceFromMemory(identifier.GetId(), identifier.GetHierarchyLevelId());
    if (shouldTakeFromMemory)
        {
        info = m_memoryCache->FindDataSource(identifier, variables, partsToGet);
        if (info.GetIdentifier().IsValid())
            return info;
        }

    // take data source from disk cache:
    // - if memory cache is not initialized or hierarchy level does not exists in it.
    // - if it was not found in memory cache and same hierarchy level exists in disk cache.
    if (!shouldTakeFromMemory || m_hierarchyLevelsFromDisk.end() != m_hierarchyLevelsFromDisk.find(identifier.GetHierarchyLevelId()))
        info = m_diskCache.FindDataSource(identifier, variables, partsToGet);

    return info;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCacheWrapper::_GetCombinedHierarchyLevel(NavNodesProviderContextR context, CombinedHierarchyLevelIdentifier const& identifier, bool onlyInitialized) const
    {
    if (IsMemoryCacheInitialized() && ShouldTakeHierarchyLevelFromMemory(identifier.GetRulesetId().c_str(), identifier.GetPhysicalParentNodeId()))
        return m_memoryCache->GetCombinedHierarchyLevel(context, identifier, onlyInitialized);

    return m_diskCache.GetCombinedHierarchyLevel(context, identifier, onlyInitialized);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr NodesCacheWrapper::_GetHierarchyLevel(NavNodesProviderContextR context, BeGuidCR id, bool onlyInitialized) const
    {
    if (IsMemoryCacheInitialized() && m_hierarchyLevelsFromDisk.end() == m_hierarchyLevelsFromDisk.find(id) && NodesCacheHelpers::HierarchyLevelExists(m_memoryCache->GetDb(), id))
        return m_memoryCache->GetHierarchyLevel(context, id, onlyInitialized);

    return m_diskCache.GetHierarchyLevel(context, id, onlyInitialized);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<DirectNodesIterator> NodesCacheWrapper::_GetCachedDirectNodesIterator(NavNodesProviderContextCR context, DataSourceIdentifier const& identifier) const
    {
    if (ShouldTakeDataSourceFromMemory(identifier.GetId(), identifier.GetHierarchyLevelId()))
        return m_memoryCache->GetCachedDirectNodesIterator(context, identifier);

    return m_diskCache.GetCachedDirectNodesIterator(context, identifier);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheWrapper::_Cache(HierarchyLevelIdentifier& identifier)
    {
    InitializeMemoryCache();
    EnsureRulesetExists(identifier.GetRulesetId());
    EnsureParentNodesExist(identifier);
    if (!NodesCacheHelpers::GetHierarchyLevelId(m_memoryCache->GetDb(), identifier.GetCombined()).IsValid())
        AttachedNodesCacheHelper::CopyPhysicalHierarchy(m_memoryCache->GetDb(), identifier.GetCombined());
    m_memoryCache->Cache(identifier);
#ifdef NAVNODES_CACHE_DEBUG
    m_memoryCache->Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheWrapper::_Cache(DataSourceInfo& dataSourceInfo)
    {
    InitializeMemoryCache();
    EnsureHierarchyLevelExists(dataSourceInfo.GetIdentifier().GetHierarchyLevelId());
    m_memoryCache->Cache(dataSourceInfo);
#ifdef NAVNODES_CACHE_DEBUG
    m_memoryCache->Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheWrapper::_Cache(NavNodeR node, DataSourceIdentifier const& dataSourceIdentifier, bvector<uint64_t> const& index, NodeVisibility visibility)
    {
    InitializeMemoryCache();
    EnsureDataSourceExists(dataSourceIdentifier.GetId());
    m_memoryCache->Cache(node, dataSourceIdentifier, index, visibility);
#ifdef NAVNODES_CACHE_DEBUG
    m_memoryCache->Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheWrapper::_Update(DataSourceInfo const& dataSourceInfo, int partsToUpdate)
    {
    InitializeMemoryCache();
    EnsureDataSourceExists(dataSourceInfo.GetIdentifier().GetId());
    if (dataSourceInfo.GetParentId().IsValid())
        EnsureDataSourceExists(dataSourceInfo.GetParentId());
    m_memoryCache->Update(dataSourceInfo, partsToUpdate);
#ifdef NAVNODES_CACHE_DEBUG
    m_memoryCache->Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheWrapper::_MakeVirtual(BeGuidCR nodeId, RulesetVariables const& contextVariables, Utf8StringCR instanceFilter)
    {
    InitializeMemoryCache();
    m_memoryCache->MakeVirtual(nodeId, contextVariables, instanceFilter);
#ifdef NAVNODES_CACHE_DEBUG
    m_memoryCache->Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheWrapper::_MakeHidden(BeGuidCR nodeId, RulesetVariables const& contextVariables, Utf8StringCR instanceFilter)
    {
    InitializeMemoryCache();
    m_memoryCache->MakeHidden(nodeId, contextVariables, instanceFilter);
#ifdef NAVNODES_CACHE_DEBUG
    m_memoryCache->Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCacheWrapper::_IsCombinedHierarchyLevelInitialized(CombinedHierarchyLevelIdentifier const& identifier, RulesetVariables const& contextVariables, Utf8StringCR instanceFilter) const
    {
    if (IsMemoryCacheInitialized() && ShouldTakeHierarchyLevelFromMemory(identifier.GetRulesetId().c_str(), identifier.GetPhysicalParentNodeId()))
        return m_memoryCache->IsCombinedHierarchyLevelInitialized(identifier, contextVariables, instanceFilter);

    return m_diskCache.IsCombinedHierarchyLevelInitialized(identifier, contextVariables, instanceFilter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCacheWrapper::_IsHierarchyLevelInitialized(BeGuidCR id, RulesetVariables const& contextVariables, Utf8StringCR instanceFilter) const
    {
    if (IsMemoryCacheInitialized() && m_hierarchyLevelsFromDisk.end() == m_hierarchyLevelsFromDisk.find(id) && NodesCacheHelpers::HierarchyLevelExists(m_memoryCache->GetDb(), id))
        return m_memoryCache->IsHierarchyLevelInitialized(id, contextVariables, instanceFilter);

    return m_diskCache.IsHierarchyLevelInitialized(id, contextVariables, instanceFilter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCacheWrapper::_IsDataSourceInitialized(BeGuidCR id) const
    {
    if (ShouldTakeDataSourceFromMemory(id, BeGuid()))
        return m_memoryCache->IsDataSourceInitialized(id);

    return m_diskCache.IsDataSourceInitialized(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeCPtr NodesCacheWrapper::_LocateNode(IConnectionCR connection, Utf8StringCR rulesetId, NavNodeKeyCR key) const
    {
    NavNodeCPtr node;
    if (IsMemoryCacheInitialized())
        node = m_memoryCache->LocateNode(connection, rulesetId, key);
    if (node.IsValid())
        return node;

    return m_diskCache.LocateNode(connection, rulesetId, key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheWrapper::_OnRulesetUsed(PresentationRuleSetCR ruleset)
    {
    bool inserted = m_rulesetsUsed.insert(ruleset.GetRuleSetId()).second;
    m_diskCache.OnRulesetUsed(ruleset);
    if (inserted && IsMemoryCacheInitialized() && !m_rootParentId.IsValid())
        LoadRootHierarchyLevels(ruleset.GetRuleSetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<IHierarchyLevelLocker> NodesCacheWrapper::_CreateHierarchyLevelLocker(CombinedHierarchyLevelIdentifier const& identifier)
    {
    if (!m_hierarchyLevelLocker)
        m_hierarchyLevelLocker = m_diskCache._CreateHierarchyLevelLocker(identifier);
    return m_hierarchyLevelLocker;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void SetupMemoryDb(Db& db)
    {
    bool wasTransactionActive = db.GetDefaultTransaction()->IsActive();
    if (wasTransactionActive)
        db.GetDefaultTransaction()->Commit();

    db.TryExecuteSql("PRAGMA " ATTACHED_DB_ALIAS ".synchronous = 1");
    db.TryExecuteSql("PRAGMA " ATTACHED_DB_ALIAS ".journal_size_limit = 0");

    if (wasTransactionActive)
        db.GetDefaultTransaction()->Begin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheWrapper::InitializeMemoryCache()
    {
    if (nullptr != m_memoryCache)
        return;

    BeFileName diskCacheFileName(m_diskCache.GetDb().GetDbFileName());
    m_memoryCache = NodesCache::Create(m_diskCache.GetConnection(), diskCacheFileName.GetDirectoryName(), m_diskCache.GetNodesFactory(), m_diskCache.GetNodesProviderContextFactory(),
        m_diskCache.GetNodesProviderFactory(), NodesCacheType::HybridMemory, false);
    m_memoryCache->GetDb().AttachDb(m_diskCache.GetDb().GetDbFileName(), ATTACHED_DB_ALIAS);
    SetupMemoryDb(m_memoryCache->GetDb());

    {
    BeSQLite::Savepoint savepoint(m_memoryCache->GetDb(), "InitializeMemoryCache");
    for (Utf8StringCR ruleset : m_rulesetsUsed)
        LoadRootHierarchyLevels(ruleset);
    }

    if (m_hierarchyLevelLocker)
        m_hierarchyLevelLocker->Lock();

#ifdef NAVNODES_CACHE_DEBUG
    m_memoryCache->Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheWrapper::LoadRootHierarchyLevels(Utf8StringCR rulesetId)
    {
    EnsureRulesetExists(rulesetId);

    bvector<BeGuid> hierarchyLevels = AttachedNodesCacheHelper::GetChildHierarchyLevelIds(m_memoryCache->GetDb(), m_rootParentId, rulesetId.c_str());
    for (BeGuidCR levelId : hierarchyLevels)
        EnsureHierarchyLevelExists(levelId);

    if (m_rootParentId.IsValid())
        EnsureNodeExists(m_rootParentId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCacheWrapper::ShouldTakeHierarchyLevelFromMemory(Utf8CP rulesetId, BeGuidCR parentId) const
    {
    if (parentId.IsValid() && NodesCacheHelpers::NodeExists(m_memoryCache->GetDb(), parentId))
        return true;

    bvector<BeGuid> hierarchyLevelIds = NodesCacheHelpers::GetChildHierarchyLevelIds(m_memoryCache->GetDb(), parentId, rulesetId);
    for (BeGuidCR levelId : hierarchyLevelIds)
        {
        if (m_hierarchyLevelsFromDisk.end() == m_hierarchyLevelsFromDisk.find(levelId))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCacheWrapper::ShouldTakeDataSourceFromMemory(BeGuidCR dataSourceId, BeGuidCR hierarchyLevelId) const
    {
    if (!IsMemoryCacheInitialized())
        return false;
    if (!dataSourceId.IsValid())
        return NodesCacheHelpers::HierarchyLevelExists(m_memoryCache->GetDb(), hierarchyLevelId);

    return NodesCacheHelpers::DataSourceExists(m_memoryCache->GetDb(), dataSourceId) &&
        NodesCacheHelpers::DataSourceNodesCount(m_memoryCache->GetDb(), dataSourceId) >= AttachedNodesCacheHelper::DataSourceNodesCount(m_memoryCache->GetDb(), dataSourceId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCacheWrapper::HierarchyLevelExistInMemory(Utf8CP rulesetId, BeGuidCR parentId) const
    {
    if (parentId.IsValid() && !NodesCacheHelpers::NodeExists(m_memoryCache->GetDb(), parentId))
        return false;

    return !NodesCacheHelpers::GetChildHierarchyLevelIds(m_memoryCache->GetDb(), parentId, rulesetId).empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheWrapper::EnsureParentNodesExist(HierarchyLevelIdentifier const& identifier)
    {
    EnsureParentNodesExist({identifier.GetPhysicalParentNodeId(), identifier.GetVirtualParentNodeId()});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheWrapper::EnsureParentNodesExist(bvector<BeGuid> const& parents)
    {
    for (BeGuidCR parentId : parents)
        {
        if (parentId.IsValid())
            EnsureNodeExists(parentId);
        }

#ifdef NAVNODES_CACHE_DEBUG
    m_memoryCache->Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheWrapper::EnsureRulesetExists(Utf8StringCR rulesetId)
    {
    if (NodesCacheHelpers::RulesetExists(m_memoryCache->GetDb(), rulesetId))
        return;

    AttachedNodesCacheHelper::CopyRuleset(m_memoryCache->GetDb(), rulesetId);
    AttachedNodesCacheHelper::CopyRulesetVariables(m_memoryCache->GetDb(), rulesetId);

#ifdef NAVNODES_CACHE_DEBUG
    m_memoryCache->Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheWrapper::EnsureHierarchyLevelExists(BeGuidCR hierarchyLevelId)
    {
    if (!hierarchyLevelId.IsValid())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Using invalid hierarchy level id");

    if (NodesCacheHelpers::HierarchyLevelExists(m_memoryCache->GetDb(), hierarchyLevelId))
        return;

    AttachedNodesCacheHelper::HierarchyLevelCopyInfo hierarchyLevelCopyInfo = AttachedNodesCacheHelper::GetHierarchyLevelCopyInfo(m_memoryCache->GetDb(), hierarchyLevelId);
    EnsureParentNodesExist({hierarchyLevelCopyInfo.ParentNodeId});
    EnsureRulesetExists(hierarchyLevelCopyInfo.RulesetId);
    AttachedNodesCacheHelper::CopyHierarchyLevel(m_memoryCache->GetDb(), hierarchyLevelId);

    m_hierarchyLevelsFromDisk.insert(hierarchyLevelId);

#ifdef NAVNODES_CACHE_DEBUG
    m_memoryCache->Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheWrapper::EnsureDataSourceExists(BeGuidCR dataSourceId)
    {
    if (!dataSourceId.IsValid())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Using invalid data source id");

    if (NodesCacheHelpers::DataSourceExists(m_memoryCache->GetDb(), dataSourceId))
        return;

    BeGuid hierarchyLevelId = AttachedNodesCacheHelper::GetDataSourceHierarchyLevelId(m_memoryCache->GetDb(), dataSourceId);
    EnsureHierarchyLevelExists(hierarchyLevelId);
    AttachedNodesCacheHelper::CopyDataSource(m_memoryCache->GetDb(), dataSourceId);

#ifdef NAVNODES_CACHE_DEBUG
    m_memoryCache->Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheWrapper::EnsureNodeExists(BeGuidCR nodeId)
    {
    if (!nodeId.IsValid())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Using invalid node id");

    if (NodesCacheHelpers::NodeExists(m_memoryCache->GetDb(), nodeId))
        return;

    BeGuid dataSourceId = AttachedNodesCacheHelper::GetNodeDataSourceId(m_memoryCache->GetDb(), nodeId);
    EnsureDataSourceExists(dataSourceId);

    bvector<BeGuid> mergedNodes = AttachedNodesCacheHelper::GetMergedNodes(m_memoryCache->GetDb(), nodeId);
    for (BeGuidCR mergedNode : mergedNodes)
        EnsureNodeExists(mergedNode);
    AttachedNodesCacheHelper::CopyNode(m_memoryCache->GetDb(), nodeId);
    AttachedNodesCacheHelper::CopyMergedNodes(m_memoryCache->GetDb(), nodeId);

#ifdef NAVNODES_CACHE_DEBUG
    m_memoryCache->Persist();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheWrapper::PersistMemoryData()
    {
    if (IsMemoryCacheInitialized())
        {
#ifdef NAVNODES_CACHE_DEBUG
        m_memoryCache->Persist();
#endif
        NodesCacheMerger cacheMerger(m_memoryCache->GetDb());
        cacheMerger.Merge();

        // remove nodes from quick cache that were updated
        m_diskCache.RemoveQuick([&](NavNodeCR node) {return m_updatedNodeIds.end() != m_updatedNodeIds.find(node.GetNodeId());});
        }
    // unlock hierarchy level if it was locked
    m_hierarchyLevelLocker = nullptr;
    }
