/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationTypes.h>
#include "DataSourceInfo.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define NODESCACHE_FUNCNAME_ConcatBinaryIndex   "ConcatBinaryIndex"
#define NODESCACHE_FUNCNAME_VariablesMatch      "VariablesMatch"
#define NODESCACHE_FUNCNAME_GuidConcat          "GuidConcat"

//! A macro that creates a correlated subquery for selecting virtual parent node ids for merged nodes
#define MERGED_NODES_VIRTUAL_PARENT_IDS_CORRELATED_SELECT_STMT(nodes_table_alias) \
    "SELECT " NODESCACHE_FUNCNAME_GuidConcat "([ihl].[ParentNodeId]) " \
    "FROM [" NODESCACHE_TABLENAME_MergedNodes "] [imn] " \
    "JOIN [" NODESCACHE_TABLENAME_Nodes "] [in] ON [in].[Id] = [imn].[MergedNodeId] " \
    "JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] [idsn] ON [idsn].[NodeId] = [in].[Id] " \
    "JOIN [" NODESCACHE_TABLENAME_DataSources "] [ids] ON [ids].[Id] = [idsn].[DataSourceId] " \
    "JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] [ihl] ON [ihl].[Id] = [ids].[HierarchyLevelId] " \
    "WHERE [imn].[MergingNodeId] = [" nodes_table_alias "].[Id] "

//! A macro that creates a correlated subquery for selecting merged node ids
#define MERGED_NODE_IDS_CORRELATED_SELECT_STMT(nodes_table_alias) \
    "SELECT " NODESCACHE_FUNCNAME_GuidConcat "([imn].[MergedNodeId]) " \
    "FROM [" NODESCACHE_TABLENAME_MergedNodes "] [imn] " \
    "WHERE [imn].[MergingNodeId] = [" nodes_table_alias "].[Id] "

//! A macro that creates a correlated subquery for selecting node instance keys
#define NODE_INSTANCE_KEYS_CORRELATED_SELECT_STMT(nodes_table_alias) \
    "SELECT '[' || group_concat('{\"c\":' || [ni].[ECClassId] || ',\"i\":' || [ni].[ECInstanceId] || '}') || ']' " \
    "FROM [" NODESCACHE_TABLENAME_NodeInstances "] [ni] " \
    "WHERE [ni].[NodeId] = [" nodes_table_alias "].[Id] "

//! A macro that creates a query for selecting columns that later can be read using NodesCacheHelpers::CreateNodeFromStatement
#define NODE_SELECT_STMT(hierarchy_levels_table_alias, nodes_table_alias, node_keys_table_alias) \
    /*  0 */ "COALESCE((" MERGED_NODES_VIRTUAL_PARENT_IDS_CORRELATED_SELECT_STMT(nodes_table_alias) "), [" hierarchy_levels_table_alias "].[ParentNodeId]) VirtualParentNodeIds, " \
    /*  1 */ "[" nodes_table_alias "].[Data], " \
    /*  2 */ "[" nodes_table_alias "].[Label], " \
    /*  3 */ "[" nodes_table_alias "].[InstanceKeysSelectQuery], " \
    /*  4 */ "[" nodes_table_alias "].[Id], " \
    /*  5 */ "[" node_keys_table_alias "].[Type]," \
    /*  6 */ "[" node_keys_table_alias "].[SpecificationIdentifier]," \
    /*  7 */ "[" node_keys_table_alias "].[ClassId]," \
    /*  8 */ "[" node_keys_table_alias "].[IsClassPolymorphic]," \
    /*  9 */ "[" node_keys_table_alias "].[PropertyName]," \
    /* 10 */ "[" node_keys_table_alias "].[SerializedPropertyValues]," \
    /* 11 */ "[" node_keys_table_alias "].[GroupedInstanceKeysCount]," \
    /* 12 */ "[" node_keys_table_alias "].[GroupedInstanceKeys]," \
    /* 13 */ "[" node_keys_table_alias "].[PathFromRoot]," \
    /* 14 */ "(" MERGED_NODE_IDS_CORRELATED_SELECT_STMT(nodes_table_alias) ")," \
    /* 15 */ "(" NODE_INSTANCE_KEYS_CORRELATED_SELECT_STMT(nodes_table_alias) ")"

#define SAFE_CACHED_STATEMENT(stmt, db, query, returnValue) \
    auto stmt = db.GetCachedStatement(query);\
    if (stmt.IsNull()) \
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, Utf8PrintfString("Failed to prepare query: '%s'", query));

#define VALIDATE_STEP(stmt, expectedResult) \
    if (expectedResult != stmt->Step()) \
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesCache, "Unexpected step result");

#define PHYSICAL_HIERARCHY_LEVELS_TABLE_NAME "CTE_PhysicalHierarchyLevels"
#define PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS "[hl]"
#define PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_DataSourceId "DataSourceId"
#define PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_DataSourceIndex "DataSourceIndex"
#define PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_VariablesId "VariablesId"
#define PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_InstanceFilter "InstanceFilter"
#define PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_HierarchyLevelId "HierarchyLevelId"
#define PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_VirtualParentNodeId "VirtualParentNodeId"
#define PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_PhysicalHierarchyLevelId "PhysicalHierarchyLevelId"
#define PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_PhysicalParentNodeId "PhysicalParentNodeId"
#define PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_RulesetId "RulesetId"
#define PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_RemovalId "RemovalId"
#define WITH_PHYSICAL_HIERARCHY_LEVELS(SETUP_WHERE_CLAUSE) \
    "WITH RECURSIVE " \
    PHYSICAL_HIERARCHY_LEVELS_TABLE_NAME " (" \
        PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_DataSourceId ", " \
        PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_DataSourceIndex ", " \
        PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_VariablesId ", " \
        PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_InstanceFilter ", " \
        PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_HierarchyLevelId ", " \
        PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_VirtualParentNodeId ", " \
        PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_PhysicalHierarchyLevelId ", " \
        PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_PhysicalParentNodeId ", " \
        PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_RulesetId ", " \
        PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_RemovalId \
    ") AS (" \
    "    SELECT ds.Id, " \
    "           ds.[Index], " \
    "           ds.VariablesId, " \
    "           ds.InstanceFilter, " \
    "           " PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".Id, " \
    "           " PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".ParentNodeId, " \
    "           " PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".Id, " \
    "           " PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".ParentNodeId, " \
    "           " PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".RulesetId, " \
    "           " PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".RemovalId " \
    "    FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] AS " PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS \
    "    CROSS JOIN [" NODESCACHE_TABLENAME_DataSources "] AS ds ON [ds].[HierarchyLevelId] = " PHYSICAL_HIERARCHY_LEVELS_SETUP_HL_TABLE_ALIAS ".[Id] " \
    "    WHERE " SETUP_WHERE_CLAUSE \
    "    UNION ALL " \
    "    SELECT ds.Id, " \
    "           " NODESCACHE_FUNCNAME_ConcatBinaryIndex "([phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_DataSourceIndex "], [dsn].[NodeIndex], [ds].[Index]), " \
    "           ds.VariablesId, " \
    "           ds.InstanceFilter, " \
    "           chl.Id, " \
    "           dsn.NodeId, " \
    "           phl." PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_PhysicalHierarchyLevelId ", " \
    "           phl." PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_PhysicalParentNodeId ", " \
    "           chl.RulesetId, " \
    "           chl.RemovalId " \
    "    FROM [" PHYSICAL_HIERARCHY_LEVELS_TABLE_NAME "] phl " \
    "    CROSS JOIN [" NODESCACHE_TABLENAME_DataSourceNodes "] dsn ON [dsn].[DataSourceId] = [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_DataSourceId "] AND [dsn].[Visibility] = 1 " \
    "    CROSS JOIN [" NODESCACHE_TABLENAME_HierarchyLevels "] AS chl ON [chl].[ParentNodeId] = [dsn].[NodeId] " \
    "    CROSS JOIN [" NODESCACHE_TABLENAME_DataSources "] AS ds " \
    "         ON     [ds].[HierarchyLevelId] = [chl].[Id] " \
    "            AND [ds].[VariablesId] = [phl].[VariablesId] " \
    "            AND [ds].[InstanceFilter] = [phl].[" PHYSICAL_HIERARCHY_LEVELS_COLUMN_NAME_InstanceFilter "] " \
    ")"

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesCacheHelpers
{
private:
    NodesCacheHelpers() {}

public:
    ECPRESENTATION_EXPORT static Utf8String GetPaddedNumber(uint64_t number);
    ECPRESENTATION_EXPORT static int CompareIndexes(bvector<uint64_t> const& lhs, bvector<uint64_t> const& rhs);
    ECPRESENTATION_EXPORT static void PushToIndex(bvector<uint64_t>& target, uint64_t const* source, size_t sourceSize);
    ECPRESENTATION_EXPORT static DbResult BindVectorIndex(Statement&, int columnIndex, bvector<uint64_t> const&, bool swapEndian);
    ECPRESENTATION_EXPORT static bvector<uint64_t> GetVectorIndex(Statement&, int columnIndex, bool swapEndian);

    ECPRESENTATION_EXPORT static NavNodePtr CreateNodeFromStatement(Statement&, NavNodesFactoryCR, IConnectionCR);

    ECPRESENTATION_EXPORT static void BindGuid(Statement& stmt, int index, BeGuidCR id);
    ECPRESENTATION_EXPORT static void BindGuid(Statement& target, int targetIndex, Statement& source, int sourceIndex);
    ECPRESENTATION_EXPORT static BeGuid GetGuid(Statement& stmt, int index);

    ECPRESENTATION_EXPORT static bool VariablesExists(Db& db, BeGuidCR variablesId);
    ECPRESENTATION_EXPORT static bool RulesetExists(Db& db, Utf8StringCR rulesetId);
    ECPRESENTATION_EXPORT static bool NodeExists(Db& db, BeGuidCR nodeId);
    ECPRESENTATION_EXPORT static bool DataSourceExists(Db& db, BeGuidCR datasourceId);
    ECPRESENTATION_EXPORT static bool HierarchyLevelExists(Db& db, BeGuidCR hierarchyLevelId);
    ECPRESENTATION_EXPORT static bool HierarchyLevelHasDataSources(Db& db, BeGuidCR hierarchyLevelId);
    ECPRESENTATION_EXPORT static bvector<BeGuid> GetChildHierarchyLevelIds(Db& db, BeGuidCR nodeId, Utf8CP rulesetId);
    ECPRESENTATION_EXPORT static Utf8String GetNodePathHash(Db& db, BeGuidCR nodeId);
    ECPRESENTATION_EXPORT static int DataSourceNodesCount(Db& db, BeGuidCR dataSourceId);
    ECPRESENTATION_EXPORT static BeGuid GetHierarchyLevelId(Db& db, CombinedHierarchyLevelIdentifier const& identifier);
    ECPRESENTATION_EXPORT static void LimitCacheSize(Db& db, uint64_t sizeLimit, bool removeOnlyStaleData = true);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct WalSavepoint
{
private:
    Savepoint m_savepoint;
public:
    WalSavepoint(Db& db, Utf8CP txnName, BeSQLiteTxnMode txnMode, bool begin = true);
    ~WalSavepoint() {Commit();}

    DbResult Begin();
    DbResult Commit() {return m_savepoint.Commit();}
    DbResult Cancel() {return m_savepoint.Cancel();}
    bool IsActive() const {return m_savepoint.IsActive();}
};

#define ATTACHED_DB_ALIAS "persistedCache"

#define COPY_FROM_ATTACHED_QUERY(tableName, tableAlias) "INSERT OR IGNORE INTO [main].[" tableName "] SELECT * FROM [" ATTACHED_DB_ALIAS "].[" tableName "] " tableAlias " "

#define EXISTS_IN_ATTACHED_QUERY(tableName) "SELECT 1 FROM [" ATTACHED_DB_ALIAS "].[" tableName "] "
#define HIERARCHYLEVEL_EXISTS_IN_ATTACHED_QUERY EXISTS_IN_ATTACHED_QUERY(NODESCACHE_TABLENAME_HierarchyLevels) " WHERE [Id] = ? "
#define DATASOURCE_EXISTS_IN_ATTACHED_QUERY EXISTS_IN_ATTACHED_QUERY(NODESCACHE_TABLENAME_DataSources) " WHERE [Id] = ? "
#define NODE_EXISTS_IN_ATTACHED_QUERY EXISTS_IN_ATTACHED_QUERY(NODESCACHE_TABLENAME_Nodes) " WHERE [Id] = ? "

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct AttachedNodesCacheHelper
{
    struct HierarchyLevelCopyInfo
        {
        Utf8String RulesetId;
        BeGuid ParentNodeId;
        };

private:
    AttachedNodesCacheHelper() {}

    ECPRESENTATION_EXPORT static void ExecuteQuery(Db& db, Utf8StringCR text, Utf8CP query);
    ECPRESENTATION_EXPORT static void ExecuteQuery(Db& db, BeGuidCR guid, Utf8CP query);

public:
    ECPRESENTATION_EXPORT static bool Exists(Db& db, Utf8CP query, BeGuid id);
    ECPRESENTATION_EXPORT static void CopyRuleset(Db& db, Utf8StringCR rulesetId);
    ECPRESENTATION_EXPORT static void CopyRulesetVariables(Db& db, Utf8StringCR rulesetId);
    ECPRESENTATION_EXPORT static void CopyHierarchyLevel(Db& db, BeGuidCR hierarchyLevelId);
    ECPRESENTATION_EXPORT static void CopyDataSource(Db& db, BeGuidCR dataSourceId);
    ECPRESENTATION_EXPORT static void CopyMergedNodes(Db& db, BeGuidCR nodeId);
    ECPRESENTATION_EXPORT static void CopyNode(Db& db, BeGuidCR nodeId);
    ECPRESENTATION_EXPORT static bool DataSourceHasNodes(Db& db, BeGuidCR dataSourceId);
    ECPRESENTATION_EXPORT static bvector<BeGuid> GetChildHierarchyLevelIds(Db& db, BeGuidCR nodeId, Utf8CP rulesetId);
    ECPRESENTATION_EXPORT static bool DataSourceInitialized(Db& db, BeGuidCR dataSourceId);
    ECPRESENTATION_EXPORT static BeGuid GetParentDataSourceId(Db& db, BeGuidCR dataSourceId);
    ECPRESENTATION_EXPORT static BeGuid GetDataSourceHierarchyLevelId(Db& db, BeGuidCR dataSourceId);
    ECPRESENTATION_EXPORT static BeGuid GetNodeDataSourceId(Db& db, BeGuidCR nodeId);
    ECPRESENTATION_EXPORT static BeGuid GetHierarchyLevelId(Db& db, CombinedHierarchyLevelIdentifier const& identifier);
    ECPRESENTATION_EXPORT static bvector<BeGuid> GetMergedNodes(Db& db, BeGuidCR nodeId);
    ECPRESENTATION_EXPORT static HierarchyLevelCopyInfo GetHierarchyLevelCopyInfo(Db& db, BeGuidCR id);
    ECPRESENTATION_EXPORT static int DataSourceNodesCount(Db& db, BeGuidCR dataSourceId);
    ECPRESENTATION_EXPORT static void CopyPhysicalHierarchy(Db& db, CombinedHierarchyLevelIdentifier const& identifier);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
