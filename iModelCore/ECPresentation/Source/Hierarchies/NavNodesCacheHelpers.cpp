/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "NavNodesHelper.h"
#include "NavNodesCacheHelpers.h"
#include "NavNodesCache.h"

static const unsigned DS_INDEX_PADDING = 20;
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String NodesCacheHelpers::GetPaddedNumber(uint64_t number)
    {
    Utf8String padded;
    padded.reserve(DS_INDEX_PADDING);
    auto numberStr = std::to_string(number);
    for (int i = numberStr.length(); i < DS_INDEX_PADDING; i++)
        padded.append("0");
    padded.append(numberStr.c_str());
    return padded;
    }

static const Utf8String DS_INDEX_SEPARATOR = "-";
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String NodesCacheHelpers::IndexToString(bvector<uint64_t> const& index, bool pad)
    {
    Utf8String str;
    if (pad)
        str.reserve(index.size() * (DS_INDEX_PADDING + 1));
    for (uint64_t i : index)
        {
        if (!str.empty())
            str.append(DS_INDEX_SEPARATOR);
        if (pad)
            str.append(GetPaddedNumber(i).c_str());
        else
            str.append(std::to_string(i).c_str());
        }
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<uint64_t> NodesCacheHelpers::IndexFromString(Utf8StringCR str)
    {
    bvector<uint64_t> index;
    size_t offset = 0;
    Utf8String token;
    while (Utf8String::npos != (offset = str.GetNextToken(token, DS_INDEX_SEPARATOR.c_str(), offset)))
        index.push_back(BeStringUtilities::ParseUInt64(token.c_str()));
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int NodesCacheHelpers::CompareIndexes(bvector<uint64_t> const& lhs, bvector<uint64_t> const& rhs)
    {
    for (size_t i = 0; i < lhs.size() && i < rhs.size(); ++i)
        {
        if (lhs[i] < rhs[i])
            return -1;
        if (rhs[i] < lhs[i])
            return 1;
        }
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<BeGuid> ParseGuidsFromConcatenatedString(Utf8CP str)
    {
    bvector<BeGuid> result;
    if (str && *str)
        {
        T_Utf8StringVector idStrs;
        BeStringUtilities::Split(str, ",", idStrs);
        for (auto const& idStr : idStrs)
            {
            BeGuid guid;
            guid.FromString(idStr.c_str());
            result.push_back(guid);
            }
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<BeGuid> GetVirtualParentIds(Statement& stmt, int index)
    {
    if (stmt.IsColumnNull(index))
        return bvector<BeGuid>();
    if (DbValueType::TextVal == stmt.GetColumnType(index))
        return ParseGuidsFromConcatenatedString(stmt.GetValueText(index));
    return { NodesCacheHelpers::GetGuid(stmt, index) };
    }

/*---------------------------------------------------------------------------------**//**
* Note: Column indexes based on the order in NODE_SELECT_STMT macro
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static NavNodeKeyPtr CreateNodeKeyFromStatement(Statement& stmt, IConnectionCR connection)
    {
    Utf8CP serializedHashPath = stmt.GetValueText(14);
    auto hashPath = NavNodesHelper::NodeKeyHashPathFromString(serializedHashPath);

    Utf8CP specificationIdentifier = stmt.GetValueText(7);

    Utf8CP nodeType = stmt.GetValueText(6);
    if (0 == strcmp(nodeType, NAVNODE_TYPE_ECInstancesNode))
        {
        auto instanceKeys = ValueHelpers::GetECInstanceKeysFromJsonString(stmt.GetValueText(16));
        auto classInstanceKeys = ContainerHelpers::TransformContainer<bvector<ECClassInstanceKey>>(instanceKeys, [&connection](auto const& k)
            {
            return ECClassInstanceKey(connection.GetECDb().Schemas().GetClass(k.GetClassId()), k.GetInstanceId());
            });
        return ECInstancesNodeKey::Create(classInstanceKeys, specificationIdentifier, hashPath);
        }
    if (0 == strcmp(nodeType, NAVNODE_TYPE_ECClassGroupingNode))
        {
        ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(stmt.GetValueId<ECClassId>(8));
        auto instanceKeys = stmt.IsColumnNull(13) ? nullptr : std::make_unique<bvector<ECInstanceKey>>(ValueHelpers::GetECInstanceKeysFromJsonString(stmt.GetValueText(13)));
        return ECClassGroupingNodeKey::Create(*ecClass, stmt.GetValueBoolean(9), specificationIdentifier, hashPath, stmt.GetValueUInt64(12), std::move(instanceKeys));
        }
    if (0 == strcmp(nodeType, NAVNODE_TYPE_ECPropertyGroupingNode))
        {
        ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(stmt.GetValueId<ECClassId>(8));
        rapidjson::Document::AllocatorType groupedValuesAllocator(256);
        rapidjson::Document groupedValues(&groupedValuesAllocator);
        groupedValues.Parse(stmt.GetValueText(11));
        auto instanceKeys = stmt.IsColumnNull(13) ? nullptr : std::make_unique<bvector<ECInstanceKey>>(ValueHelpers::GetECInstanceKeysFromJsonString(stmt.GetValueText(13)));
        return ECPropertyGroupingNodeKey::Create(*ecClass, stmt.GetValueText(10), groupedValues, specificationIdentifier, hashPath, stmt.GetValueUInt64(12), std::move(instanceKeys));
        }
    if (0 == strcmp(nodeType, NAVNODE_TYPE_DisplayLabelGroupingNode))
        {
        auto instanceKeys = stmt.IsColumnNull(13) ? nullptr : std::make_unique<bvector<ECInstanceKey>>(ValueHelpers::GetECInstanceKeysFromJsonString(stmt.GetValueText(13)));
        return LabelGroupingNodeKey::Create(stmt.GetValueText(3), specificationIdentifier, hashPath, stmt.GetValueUInt64(12), std::move(instanceKeys));
        }
    return NavNodeKey::Create(nodeType, specificationIdentifier, hashPath);
    }

/*---------------------------------------------------------------------------------**//**
* Note: Column indexes based on the order in NODE_SELECT_STMT macro
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NodesCacheHelpers::CreateNodeFromStatement(Statement& stmt, NavNodesFactoryCR nodesFactory, IConnectionCR connection)
    {
    NavNodeKeyPtr key = CreateNodeKeyFromStatement(stmt, connection);
    if (key.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to deserialize node key");

    Utf8CP serializedNode = stmt.GetValueText(2);
    if (nullptr == serializedNode || 0 == *serializedNode)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Invalid serialized node content");

    rapidjson::Document json;
    json.Parse(serializedNode);
    NavNodePtr node = nodesFactory.CreateFromJson(connection, std::move(json), *key);
    if (node.IsNull())
        return nullptr;

    // FIXME: we shouldn't cache these at all, but first we need to get rid of node extended data altogether
    NavNodeExtendedData e(*node);
    e.SetIsCustomized(false);
    e.SetNodeInitialized(false);

    node->SetNodeId(NodesCacheHelpers::GetGuid(stmt, 5));
    node->SetParentNodeId(stmt.IsColumnNull(0) ? BeGuid() : NodesCacheHelpers::GetGuid(stmt, 0));
    if (!stmt.IsColumnNull(4))
        {
        json.Parse(stmt.GetValueText(4));
        node->SetInstanceKeysSelectQuery(StringGenericQuery::FromJson(json));
        }

    NavNodeExtendedData extendedData(*node);
    extendedData.SetVirtualParentIds(GetVirtualParentIds(stmt, 1));
    extendedData.SetMergedNodeIds(ParseGuidsFromConcatenatedString(stmt.GetValueText(15)));

    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheHelpers::BindGuid(Statement& stmt, int index, BeGuidCR id)
    {
    if (!id.IsValid())
        {
        stmt.BindNull(index);
        return;
        }

#ifdef NAVNODES_CACHE_BINARY_GUID
    stmt.BindGuid(index, id);
#else
    stmt.BindText(index, id.ToString(), Statement::MakeCopy::Yes);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheHelpers::BindGuid(Statement& target, int targetIndex, Statement& source, int sourceIndex)
    {
    if (source.IsColumnNull(sourceIndex))
        {
        target.BindNull(targetIndex);
        return;
        }

#ifdef NAVNODES_CACHE_BINARY_GUID
    target.BindGuid(targetIndex, source.GetValueGuid(sourceIndex));
#else
    target.BindText(targetIndex, source.GetValueText(sourceIndex), Statement::MakeCopy::No);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid NodesCacheHelpers::GetGuid(Statement& stmt, int index)
    {
#ifdef NAVNODES_CACHE_BINARY_GUID
    return stmt.GetValueGuid(index);
#else
    BeGuid guid;
    guid.FromString(stmt.GetValueText(index));
    return guid;
#endif
    }

#define EXISTS_QUERY(tableName, key) "SELECT 1 FROM [" tableName "] WHERE [" key "] = ? "
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCacheHelpers::VariablesExists(Db& db, BeGuidCR variablesId)
    {
    SAFE_CACHED_STATEMENT(stmt, db, EXISTS_QUERY(NODESCACHE_TABLENAME_Variables, "id"), false);
    NodesCacheHelpers::BindGuid(*stmt, 1, variablesId);
    return BE_SQLITE_ROW == stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCacheHelpers::RulesetExists(Db& db, Utf8StringCR rulesetId)
    {
    SAFE_CACHED_STATEMENT(stmt, db, EXISTS_QUERY(NODESCACHE_TABLENAME_Rulesets, "Identifier"), false);
    stmt->BindText(1, rulesetId, Statement::MakeCopy::No);
    return BE_SQLITE_ROW == stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCacheHelpers::NodeExists(Db& db, BeGuidCR nodeId)
    {
    SAFE_CACHED_STATEMENT(stmt, db, EXISTS_QUERY(NODESCACHE_TABLENAME_Nodes, "Id"), false);
    NodesCacheHelpers::BindGuid(*stmt, 1, nodeId);
    return BE_SQLITE_ROW == stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCacheHelpers::DataSourceExists(Db& db, BeGuidCR datasourceId)
    {
    SAFE_CACHED_STATEMENT(stmt, db, EXISTS_QUERY(NODESCACHE_TABLENAME_DataSources, "Id"), false);
    NodesCacheHelpers::BindGuid(*stmt, 1, datasourceId);
    return BE_SQLITE_ROW == stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCacheHelpers::HierarchyLevelExists(Db& db, BeGuidCR hierarchyLevelId)
    {
    SAFE_CACHED_STATEMENT(stmt, db, EXISTS_QUERY(NODESCACHE_TABLENAME_HierarchyLevels, "Id"), false);
    NodesCacheHelpers::BindGuid(*stmt, 1, hierarchyLevelId);
    return BE_SQLITE_ROW == stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid NodesCacheHelpers::GetPhysicalHierarchyLevelId(Db& db, CombinedHierarchyLevelIdentifier const& identifier)
    {
    static Utf8CP query =
        " SELECT [phl].[Id] "
        " FROM [" NODESCACHE_TABLENAME_PhysicalHierarchyLevels "] phl "
        " JOIN [" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [phl].[RulesetId] "
        " WHERE [phl].[PhysicalParentNodeId] IS ? AND [r].[Identifier] = ? AND [phl].[RemovalId] IS ? ";
    SAFE_CACHED_STATEMENT(stmt, db, query, BeGuid());
    NodesCacheHelpers::BindGuid(*stmt, 1, identifier.GetPhysicalParentNodeId());
    stmt->BindText(2, identifier.GetRulesetId(), Statement::MakeCopy::No);
    NodesCacheHelpers::BindGuid(*stmt, 3, identifier.GetRemovalId());
    if (BE_SQLITE_ROW != stmt->Step())
        return BeGuid();

    return NodesCacheHelpers::GetGuid(*stmt, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCacheHelpers::HierarchyLevelHasDataSources(Db& db, BeGuidCR hierarchyLevelId)
    {
    SAFE_CACHED_STATEMENT(stmt, db, EXISTS_QUERY(NODESCACHE_TABLENAME_DataSources, "HierarchyLevelId"), false);
    NodesCacheHelpers::BindGuid(*stmt, 1, hierarchyLevelId);
    return BE_SQLITE_ROW == stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<BeGuid> NodesCacheHelpers::GetChildHierarchyLevelIds(Db& db, bool isPhysical, BeGuidCR nodeId, Utf8CP rulesetId)
    {
    Utf8String query =
        " SELECT [hl].[Id] "
        " FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
        " JOIN [" NODESCACHE_TABLENAME_PhysicalHierarchyLevels "] phl ON [phl].[Id] = [hl].[PhysicalHierarchyLevelId]";
    if (nullptr != rulesetId)
        query.append(" JOIN [" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [phl].[RulesetId] ");

    query.append(isPhysical ? " WHERE [phl].[PhysicalParentNodeId] IS ? " : " WHERE [hl].[VirtualParentNodeId] IS ? ");
    if (nullptr != rulesetId)
        query.append(" AND [r].[Identifier] = ? ");

    SAFE_CACHED_STATEMENT(stmt, db, query.c_str(), bvector<BeGuid>());
    int bindIndex = 1;
    BindGuid(*stmt, bindIndex++, nodeId);
    if (nullptr != rulesetId)
        stmt->BindText(bindIndex++, rulesetId, Statement::MakeCopy::No);

    bvector<BeGuid> hierarchyLevelIds;
    while (BE_SQLITE_ROW == stmt->Step())
        hierarchyLevelIds.push_back(NodesCacheHelpers::GetGuid(*stmt, 0));

    return hierarchyLevelIds;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String NodesCacheHelpers::GetNodePathHash(Db& db, BeGuidCR nodeId)
    {
    static Utf8String defaultValue;
    if (!nodeId.IsValid())
        return defaultValue;

    static Utf8CP query =
        " SELECT [nk].[PathFromRoot] "
        " FROM [" NODESCACHE_TABLENAME_NodeKeys "] [nk] "
        " WHERE [NodeId] = ? ";
    SAFE_CACHED_STATEMENT(stmt, db, query, defaultValue);
    NodesCacheHelpers::BindGuid(*stmt, 1, nodeId);
    if (BE_SQLITE_ROW != stmt->Step())
        return defaultValue;

    return stmt->GetValueText(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool NodesCacheHelpers::LimitHierarchyVariations(Db& db, Utf8CP query, BeGuidCR hierarchyLevelId, int threshold)
    {
    SAFE_CACHED_STATEMENT(stmt, db, query, false);
    BindGuid(*stmt, 1, hierarchyLevelId);
    BindGuid(*stmt, 2, hierarchyLevelId);
    stmt->BindInt(3, threshold);
    stmt->Step();

    if (0 == db.GetModifiedRowCount())
        return false;

    // cleanup unused variables
    static Utf8CP deleteVariablesQuery = "DELETE FROM [" NODESCACHE_TABLENAME_Variables "] AS v WHERE NOT EXISTS("
        "SELECT [ds].[Id] FROM [" NODESCACHE_TABLENAME_DataSources "] ds WHERE [ds].[VariablesId] = [v].[Id])";

    SAFE_CACHED_STATEMENT(deleteVariablesStmt, db, deleteVariablesQuery, true);
    deleteVariablesStmt->Step();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int NodesCacheHelpers::DataSourceNodesCount(Db& db, BeGuidCR dataSourceId)
    {
    static Utf8CP query =
        "SELECT COUNT(1) "
        "FROM [" NODESCACHE_TABLENAME_Nodes "] "
        "WHERE [DataSourceId] = ? ";

    SAFE_CACHED_STATEMENT(stmt, db, query, false);
    BindGuid(*stmt, 1, dataSourceId);
    if (BE_SQLITE_ROW != stmt->Step())
        return 0;
    return stmt->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid NodesCacheHelpers::CachePhysicalHierarchyLevel(Db& db, CombinedHierarchyLevelIdentifier const& identifier, BeGuidCR id)
    {
    static Utf8CP query =
        "INSERT INTO [" NODESCACHE_TABLENAME_PhysicalHierarchyLevels "] "
        "([Id], [PhysicalParentNodeId], [RemovalId], [RulesetId]) "
        "VALUES (?, ?, ?, "
        "  (SELECT [Id] FROM [" NODESCACHE_TABLENAME_Rulesets "] WHERE [Identifier] = ?)"
        ") ";

    SAFE_CACHED_STATEMENT(stmt, db, query, BeGuid());

    BeGuid levelId = id.IsValid() ? id : BeGuid(true);
    int bindingIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, levelId);
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, identifier.GetPhysicalParentNodeId());
    NodesCacheHelpers::BindGuid(*stmt, bindingIndex++, identifier.GetRemovalId());
    stmt->BindText(bindingIndex++, identifier.GetRulesetId(), Statement::MakeCopy::No);

    DbResult insertResult = stmt->Step();
    if (BE_SQLITE_DONE != insertResult)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, Utf8PrintfString("Unexpected step result: %d", (int)insertResult));

    return levelId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static uint64_t GetCacheFileSize(Utf8CP fileName)
    {
    BeFileName dbFile(fileName);
    uint64_t size = 0;
    BeFileNameStatus status = dbFile.GetFileSize(size);
    if (BeFileNameStatus::Success != status)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, Utf8PrintfString("Failed to get file size. Error: %d", (int)status));
    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult QueryOldestRuleset(BeSQLite::Db& db, BeGuidR rulesetId, uint64_t& lastUsedTime)
    {
    static Utf8CP query = "SELECT [Id], [LastUsedTime] FROM [" NODESCACHE_TABLENAME_Rulesets "] ORDER BY [LastUsedTime] ASC";
    Statement stmt(db, query);
    if (!stmt.IsPrepared())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare oldest ruleset query");

    DbResult result = stmt.Step();
    if (BE_SQLITE_ROW == result)
        {
        rulesetId = NodesCacheHelpers::GetGuid(stmt, 0);
        lastUsedTime = stmt.GetValueUInt64(1);
        }

    return result;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------*/
static void RemoveRuleset(BeSQLite::Db& db, BeGuidCR rulesetId)
    {
    static Utf8CP rulesetsQuery = "DELETE FROM [" NODESCACHE_TABLENAME_Rulesets "] WHERE [Id] = ?";
    Statement rulesetsStmt(db, rulesetsQuery);
    if (!rulesetsStmt.IsPrepared())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare ruleset removal query");
    NodesCacheHelpers::BindGuid(rulesetsStmt, 1, rulesetId);
    rulesetsStmt.Step();

    static Utf8CP physicalHierarchyLevelsQuery = "DELETE FROM [" NODESCACHE_TABLENAME_PhysicalHierarchyLevels "] WHERE [RulesetId] = ?";
    Statement physicalHierarchyLevelsStmt(db, physicalHierarchyLevelsQuery);
    if (!physicalHierarchyLevelsStmt.IsPrepared())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare physical hierarchy levels removal query");
    NodesCacheHelpers::BindGuid(physicalHierarchyLevelsStmt, 1, rulesetId);
    physicalHierarchyLevelsStmt.Step();

    static Utf8CP variablesQuery = "DELETE FROM [" NODESCACHE_TABLENAME_Variables "] WHERE [RulesetId] = ?";
    Statement variablesStmt(db, variablesQuery);
    if (!variablesStmt.IsPrepared())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Failed to prepare ruleset variables removal query");
    NodesCacheHelpers::BindGuid(variablesStmt, 1, rulesetId);
    variablesStmt.Step();

    static Utf8CP hierarchyLevelsQuery = "DELETE FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] WHERE NOT EXISTS("
        "SELECT [phl].[Id] FROM [" NODESCACHE_TABLENAME_PhysicalHierarchyLevels "] phl WHERE [phl].[Id] = [PhysicalHierarchyLevelId])";
    db.ExecuteSql(hierarchyLevelsQuery);

    static Utf8CP dataSourcesQuery = "DELETE FROM [" NODESCACHE_TABLENAME_DataSources "] WHERE NOT EXISTS("
        "SELECT [hl].[Id] FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl WHERE [hl].[Id] = [HierarchyLevelId])";
    db.ExecuteSql(dataSourcesQuery);

    static Utf8CP dataSourceClassesQuery = "DELETE FROM [" NODESCACHE_TABLENAME_DataSourceClasses "] WHERE NOT EXISTS("
        "SELECT [ds].[Id] FROM [" NODESCACHE_TABLENAME_DataSources "] ds WHERE [ds].[Id] = [DataSourceId])";
    db.ExecuteSql(dataSourceClassesQuery);

    static Utf8CP nodesQuery = "DELETE FROM [" NODESCACHE_TABLENAME_Nodes "] WHERE NOT EXISTS("
        "SELECT [ds].[Id] FROM [" NODESCACHE_TABLENAME_DataSources "] ds WHERE [ds].[Id] = [DataSourceId])";
    db.ExecuteSql(nodesQuery);

    static Utf8CP nodesKeysQuery = "DELETE FROM [" NODESCACHE_TABLENAME_NodeKeys "] WHERE NOT EXISTS("
        "SELECT [n].[Id] FROM [" NODESCACHE_TABLENAME_Nodes "] n WHERE [n].[Id] = [NodeId])";
    db.ExecuteSql(nodesKeysQuery);

    static Utf8CP nodeInstancesQuery = "DELETE FROM [" NODESCACHE_TABLENAME_NodeInstances "] WHERE NOT EXISTS("
        "SELECT [n].[Id] FROM [" NODESCACHE_TABLENAME_Nodes "] n WHERE [n].[Id] = [NodeId])";
    db.ExecuteSql(nodeInstancesQuery);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheHelpers::LimitCacheSize(Db& db, uint64_t sizeLimit, bool removeOnlyStaleData)
    {
    if (0 == sizeLimit || Utf8String::IsNullOrEmpty(db.GetDbFileName()) || GetCacheFileSize(db.GetDbFileName()) <= sizeLimit) \
        return;

    db.TryExecuteSql("PRAGMA foreign_keys=off");
    static const uint64_t s_staleRulesetTime = 5 * 60 * 1000;
    while (GetCacheFileSize(db.GetDbFileName()) > sizeLimit)
        {
        BeGuid rulesetId;
        uint64_t lastUsedTime = 0;
        WalSavepoint savepoint(db, "RemoveOldestRuleset", BeSQLiteTxnMode::Immediate);
        if (BE_SQLITE_ROW != QueryOldestRuleset(db, rulesetId, lastUsedTime))
            break;

        if (removeOnlyStaleData && (BeTimeUtilities::GetCurrentTimeAsUnixMillis() - lastUsedTime) < s_staleRulesetTime)
            break;

        RemoveRuleset(db, rulesetId);
        savepoint.Commit();

        DbResult vacuumResult = db.TryExecuteSql("VACUUM");
        db.TryExecuteSql("PRAGMA wal_checkpoint(TRUNCATE)");
        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, (BE_SQLITE_OK == vacuumResult), Utf8PrintfString("Unexpected step result: %d", (int)vacuumResult));
        }
    db.TryExecuteSql("PRAGMA foreign_keys=on");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WalSavepoint::WalSavepoint(Db& db, Utf8CP txnName, BeSQLiteTxnMode txnMode, bool begin)
    : m_savepoint(db, txnName, false, txnMode)
    {
    if (begin)
        Begin();
    }

#define MAX_BEGIN_SAVEPOINT_RETRIES 50
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult WalSavepoint::Begin()
    {
    DbResult beginResult = m_savepoint.Begin();
    // in WAL mode `BEGIN IMMEDIATE` might return BE_SQLITE_BUSY or BE_SQLITE_BUSY_SNAPSHOT if other connections is doing something with db
    // to solve this it should be enough to try start transaction again
    int retries = 0;
    while (BE_SQLITE_BUSY == beginResult || BE_SQLITE_BUSY_SNAPSHOT == beginResult)
        {
        beginResult = m_savepoint.Begin();
        if (retries > MAX_BEGIN_SAVEPOINT_RETRIES)
            break;
        retries++;
        }

    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::HierarchiesCache, beginResult == BE_SQLITE_OK, Utf8PrintfString("WalSavepoint failed to start with status - %d", (int)beginResult).c_str());
    return beginResult;
    }

#define COPY_RULESET_FROM_ATTACHED_QUERY COPY_FROM_ATTACHED_QUERY(NODESCACHE_TABLENAME_Rulesets, ) " WHERE [Identifier] = ? "
#define COPY_RULESET_VARIABLES_FROM_ATTACHED_QUERY COPY_FROM_ATTACHED_QUERY(NODESCACHE_TABLENAME_Variables, ) " WHERE [RulesetId] IN (SELECT [Id] FROM [" NODESCACHE_TABLENAME_Rulesets "] WHERE [Identifier] = ?) "
#define COPY_PHYSICAL_HIERARCHYLEVEL_FROM_ATTACHED_QUERY COPY_FROM_ATTACHED_QUERY(NODESCACHE_TABLENAME_PhysicalHierarchyLevels, ) " WHERE [Id] = ? "
#define COPY_HIERARCHYLEVEL_FROM_ATTACHED_QUERY COPY_FROM_ATTACHED_QUERY(NODESCACHE_TABLENAME_HierarchyLevels, ) " WHERE [Id] = ? "
#define COPY_DATASOURCE_FROM_ATTACHED_QUERY COPY_FROM_ATTACHED_QUERY(NODESCACHE_TABLENAME_DataSources, ) " WHERE [Id] = ? "
#define COPY_DATASOURCE_CLASSES_FROM_ATTACHED_QUERY COPY_FROM_ATTACHED_QUERY(NODESCACHE_TABLENAME_DataSourceClasses, ) " WHERE [DataSourceId] = ? "
#define COPY_NODE_FROM_ATTACHED_QUERY COPY_FROM_ATTACHED_QUERY(NODESCACHE_TABLENAME_Nodes, ) " WHERE [Id] = ? "
#define COPY_NODE_KEY_FROM_ATTACHED_QUERY COPY_FROM_ATTACHED_QUERY(NODESCACHE_TABLENAME_NodeKeys, ) " WHERE [NodeId] = ? "
#define COPY_NODE_INSTANCES_FROM_ATTACHED_QUERY COPY_FROM_ATTACHED_QUERY(NODESCACHE_TABLENAME_NodeInstances, ) " WHERE [NodeId] = ? "
#define COPY_MERGED_NODES_FROM_ATTACHED_QUERY COPY_FROM_ATTACHED_QUERY(NODESCACHE_TABLENAME_MergedNodes, ) " WHERE [MergingNodeId] = ? "

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AttachedNodesCacheHelper::ExecuteQuery(Db& db, Utf8StringCR text, Utf8CP query)
    {
    SAFE_CACHED_STATEMENT(stmt, db, query, );
    stmt->BindText(1, text, Statement::MakeCopy::No);
    VALIDATE_STEP(stmt, BE_SQLITE_DONE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AttachedNodesCacheHelper::ExecuteQuery(Db& db, BeGuidCR guid, Utf8CP query)
    {
    SAFE_CACHED_STATEMENT(stmt, db, query, );
    NodesCacheHelpers::BindGuid(*stmt, 1, guid);
    VALIDATE_STEP(stmt, BE_SQLITE_DONE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AttachedNodesCacheHelper::Exists(Db& db, Utf8CP query, BeGuid id)
    {
    SAFE_CACHED_STATEMENT(existsStmt, db, query, false);
    NodesCacheHelpers::BindGuid(*existsStmt, 1, id);
    return BE_SQLITE_ROW == existsStmt->Step() ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AttachedNodesCacheHelper::CopyRuleset(Db& db, Utf8StringCR rulesetId)
    {
    ExecuteQuery(db, rulesetId, COPY_RULESET_FROM_ATTACHED_QUERY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AttachedNodesCacheHelper::CopyRulesetVariables(Db& db, Utf8StringCR rulesetId)
    {
    ExecuteQuery(db, rulesetId, COPY_RULESET_VARIABLES_FROM_ATTACHED_QUERY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AttachedNodesCacheHelper::CopyHierarchyLevel(Db& db, BeGuidCR hierarchyLevelId, BeGuidCR physicalHierarchyLevelId)
    {
    ExecuteQuery(db, physicalHierarchyLevelId, COPY_PHYSICAL_HIERARCHYLEVEL_FROM_ATTACHED_QUERY);
    ExecuteQuery(db, hierarchyLevelId, COPY_HIERARCHYLEVEL_FROM_ATTACHED_QUERY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AttachedNodesCacheHelper::CopyDataSource(Db& db, BeGuidCR dataSourceId)
    {
    BeGuid parentDataSourceId = GetParentDataSourceId(db, dataSourceId);
    if (parentDataSourceId.IsValid())
        ExecuteQuery(db, parentDataSourceId, COPY_DATASOURCE_FROM_ATTACHED_QUERY);
    ExecuteQuery(db, dataSourceId, COPY_DATASOURCE_FROM_ATTACHED_QUERY);
    ExecuteQuery(db, dataSourceId, COPY_DATASOURCE_CLASSES_FROM_ATTACHED_QUERY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AttachedNodesCacheHelper::CopyMergedNodes(Db& db, BeGuidCR nodeId)
    {
    ExecuteQuery(db, nodeId, COPY_MERGED_NODES_FROM_ATTACHED_QUERY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AttachedNodesCacheHelper::CopyNode(Db& db, BeGuidCR nodeId)
    {
    ExecuteQuery(db, nodeId, COPY_NODE_FROM_ATTACHED_QUERY);
    ExecuteQuery(db, nodeId, COPY_NODE_KEY_FROM_ATTACHED_QUERY);
    ExecuteQuery(db, nodeId, COPY_NODE_INSTANCES_FROM_ATTACHED_QUERY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AttachedNodesCacheHelper::DataSourceHasNodes(Db& db, BeGuidCR dataSourceId)
    {
    static Utf8CP query = "SELECT 1 FROM [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_Nodes "] WHERE [DataSourceId] = ? ";
    SAFE_CACHED_STATEMENT(stmt, db, query, false);
    NodesCacheHelpers::BindGuid(*stmt, 1, dataSourceId);
    return BE_SQLITE_ROW == stmt->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<BeGuid> AttachedNodesCacheHelper::GetChildHierarchyLevelIds(Db& db, bool isPhysical, BeGuidCR nodeId, Utf8CP rulesetId)
    {
    Utf8String query =
        "SELECT [hl].[Id] "
        "FROM [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
        "JOIN [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_PhysicalHierarchyLevels "] phl ON [phl].[Id] = [hl].[PhysicalHierarchyLevelId] ";
    if (nullptr != rulesetId)
        query.append("JOIN [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [phl].[RulesetId] ");

    query.append("WHERE ");
    query.append(isPhysical ? " [phl].[PhysicalParentNodeId] IS ? " : " [hl].[VirtualParentNodeId] IS ? ");
    if (nullptr != rulesetId)
        query.append(" AND [r].[Identifier] = ? ");

    SAFE_CACHED_STATEMENT(stmt, db, query.c_str(), bvector<BeGuid>());
    int bindIndex = 1;
    NodesCacheHelpers::BindGuid(*stmt, bindIndex++, nodeId);
    if (nullptr != rulesetId)
        stmt->BindText(bindIndex++, rulesetId, Statement::MakeCopy::No);
    bvector<BeGuid> hierarchyLevelIds;
    while (BE_SQLITE_ROW == stmt->Step())
        hierarchyLevelIds.push_back(NodesCacheHelpers::GetGuid(*stmt, 0));

    return hierarchyLevelIds;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AttachedNodesCacheHelper::DataSourceInitialized(Db& db, BeGuidCR dataSourceId)
    {
    static Utf8CP query =
        " SELECT [ds].[IsInitialized] "
        " FROM [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_DataSources "] ds  "
        " WHERE [ds].[Id] = ? ";
    SAFE_CACHED_STATEMENT(stmt, db, query, false);
    NodesCacheHelpers::BindGuid(*stmt, 1, dataSourceId);
    return BE_SQLITE_ROW == stmt->Step() ? stmt->GetValueBoolean(0) : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid AttachedNodesCacheHelper::GetParentDataSourceId(Db& db, BeGuidCR dataSourceId)
    {
    static Utf8CP query =
        " SELECT [ds].[ParentId] "
        " FROM [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_DataSources "] ds  "
        " WHERE [ds].[Id] = ? ";
    SAFE_CACHED_STATEMENT(stmt, db, query, BeGuid());
    NodesCacheHelpers::BindGuid(*stmt, 1, dataSourceId);
    VALIDATE_STEP(stmt, BE_SQLITE_ROW);
    return NodesCacheHelpers::GetGuid(*stmt, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid AttachedNodesCacheHelper::GetDataSourceHierarchyLevelId(Db& db, BeGuidCR dataSourceId)
    {
    static Utf8CP query =
        " SELECT [ds].[HierarchyLevelId] "
        " FROM [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_DataSources "] ds  "
        " WHERE [ds].[Id] = ? ";
    SAFE_CACHED_STATEMENT(stmt, db, query, BeGuid());
    NodesCacheHelpers::BindGuid(*stmt, 1, dataSourceId);
    VALIDATE_STEP(stmt, BE_SQLITE_ROW);
    return NodesCacheHelpers::GetGuid(*stmt, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid AttachedNodesCacheHelper::GetNodeDataSourceId(Db& db, BeGuidCR nodeId)
    {
    static Utf8CP query =
        " SELECT [n].[DataSourceId] "
        " FROM [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_Nodes "] n "
        " WHERE [n].[Id] = ? ";
    SAFE_CACHED_STATEMENT(stmt, db, query, BeGuid());
    NodesCacheHelpers::BindGuid(*stmt, 1, nodeId);
    VALIDATE_STEP(stmt, BE_SQLITE_ROW);
    return NodesCacheHelpers::GetGuid(*stmt, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid AttachedNodesCacheHelper::GetPhysicalHierarchyLevelId(Db& db, CombinedHierarchyLevelIdentifier const& identifier)
    {
    static Utf8CP query =
        " SELECT [phl].[Id] "
        " FROM [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_PhysicalHierarchyLevels "] phl "
        " JOIN [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [phl].[RulesetId] "
        " WHERE [phl].[PhysicalParentNodeId] IS ? AND [r].[Identifier] = ? AND [phl].[RemovalId] IS ? ";
    SAFE_CACHED_STATEMENT(stmt, db, query, BeGuid());
    NodesCacheHelpers::BindGuid(*stmt, 1, identifier.GetPhysicalParentNodeId());
    stmt->BindText(2, identifier.GetRulesetId(), Statement::MakeCopy::No);
    NodesCacheHelpers::BindGuid(*stmt, 3, identifier.GetRemovalId());
    if (BE_SQLITE_ROW != stmt->Step())
        return BeGuid();

    return NodesCacheHelpers::GetGuid(*stmt, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<BeGuid> AttachedNodesCacheHelper::GetMergedNodes(Db& db, BeGuidCR nodeId)
    {
    static Utf8CP query = "SELECT [MergedNodeId] FROM [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_MergedNodes "] WHERE [MergingNodeId] = ? ";
    SAFE_CACHED_STATEMENT(stmt, db, query, bvector<BeGuid>());
    NodesCacheHelpers::BindGuid(*stmt, 1, nodeId);

    bvector<BeGuid> mergedNodes;
    while (BE_SQLITE_ROW == stmt->Step())
        mergedNodes.push_back(NodesCacheHelpers::GetGuid(*stmt, 0));

    return mergedNodes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AttachedNodesCacheHelper::HierarchyLevelCopyInfo AttachedNodesCacheHelper::GetHierarchyLevelCopyInfo(Db& db, BeGuidCR id)
    {
    static HierarchyLevelCopyInfo s_defaultValue;
    static Utf8CP query =
        "SELECT [r].[Identifier], [phl].[PhysicalParentNodeId], [hl].[VirtualParentNodeId], [phl].[Id] "
        "FROM [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
        "JOIN [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_PhysicalHierarchyLevels "] phl ON [phl].[Id] = [hl].[PhysicalHierarchyLevelId] "
        "JOIN [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [phl].[RulesetId] "
        "WHERE [hl].[Id] = ?";
    SAFE_CACHED_STATEMENT(stmt, db, query, s_defaultValue);
    NodesCacheHelpers::BindGuid(*stmt, 1, id);

    if (BE_SQLITE_ROW != stmt->Step())
        return s_defaultValue;

    HierarchyLevelCopyInfo info;
    info.RulesetId = stmt->GetValueText(0);
    info.PhysicalParentNodeId = NodesCacheHelpers::GetGuid(*stmt, 1);
    info.VirtualParentNodeId = NodesCacheHelpers::GetGuid(*stmt, 2);
    info.PhysicalHierarchyLevelId = NodesCacheHelpers::GetGuid(*stmt, 3);
    return info;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int AttachedNodesCacheHelper::DataSourceNodesCount(Db& db, BeGuidCR dataSourceId)
    {
    static Utf8CP query =
        "SELECT COUNT(1) "
        "FROM [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_Nodes "] "
        "WHERE [DataSourceId] = ? ";

    SAFE_CACHED_STATEMENT(stmt, db, query, false);
    NodesCacheHelpers::BindGuid(*stmt, 1, dataSourceId);
    if (BE_SQLITE_ROW != stmt->Step())
        return 0;
    return stmt->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AttachedNodesCacheHelper::CopyPhysicalHierarchy(Db& db, CombinedHierarchyLevelIdentifier const& identifier)
    {
    static Utf8CP query =
        " INSERT OR IGNORE INTO [main].[" NODESCACHE_TABLENAME_PhysicalHierarchyLevels "] "
        " SELECT * FROM [" ATTACHED_DB_ALIAS "].[" NODESCACHE_TABLENAME_PhysicalHierarchyLevels "] "
        " WHERE [PhysicalParentNodeId] IS ? "
        "   AND [RulesetId] IN (SELECT [Id] FROM [" NODESCACHE_TABLENAME_Rulesets "] WHERE [Identifier] = ?) "
        "   AND [RemovalId] IS ? ";
    SAFE_CACHED_STATEMENT(stmt, db, query, );
    NodesCacheHelpers::BindGuid(*stmt, 1, identifier.GetPhysicalParentNodeId());
    stmt->BindText(2, identifier.GetRulesetId(), Statement::MakeCopy::No);
    NodesCacheHelpers::BindGuid(*stmt, 3, identifier.GetRemovalId());
    stmt->Step();
    }
