/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SqlNames.h"
#include "InstanceGraphImpl.h"
#include "SystemPropertyMap.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//! Helper: get the single DbColumn from a SystemPropertyMap (safe for link-table use where there is exactly one table)
static DbColumn const& FirstCol(SystemPropertyMap const& map)
    {
    BeAssert(!map.GetDataPropertyMaps().empty());
    return map.GetDataPropertyMaps().front()->GetColumn();
    }

// =====================================================================================
// GraphStatementCache — Relationship Discovery
// =====================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GraphStatementCache::DiscoverRelationshipsForClass(bvector<ApplicableRelationship>& out, ECN::ECClassId entityClassId)
    {
    out.clear();
    // Reuse ECInstanceFinder's recursive CTE pattern but resolve to ClassMap immediately
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement(
        " WITH RECURSIVE"
        "    BaseClassesOfEndClass(ClassId) AS ("
        "    VALUES (:endClassId)"
        "    UNION "
        "    SELECT BaseClassId FROM main.ec_ClassHasBaseClasses, BaseClassesOfEndClass"
        "      WHERE ec_ClassHasBaseClasses.ClassId=BaseClassesOfEndClass.ClassId"
        "    )"
        " SELECT DISTINCT ECRelationshipClass.Id AS RelationshipId,"
        "   ForeignEndConstraint.RelationshipEnd AS ForeignEndIsTarget"
        " FROM main.ec_Class ECRelationshipClass"
        " JOIN main.ec_RelationshipConstraint ForeignEndConstraint"
        "   ON ForeignEndConstraint.RelationshipClassId = ECRelationshipClass.Id"
        " JOIN main.ec_RelationshipConstraintClass ForeignEndConstraintClass"
        "   ON ForeignEndConstraintClass.ConstraintId=ForeignEndConstraint.Id"
        " JOIN BaseClassesOfEndClass"
        " WHERE ForeignEndConstraintClass.ClassId = :endClassId"
        "    OR (ForeignEndConstraint.IsPolymorphic = " SQLVAL_True
        "        AND ForeignEndConstraintClass.ClassId = BaseClassesOfEndClass.ClassId)");

    if (stmt == nullptr)
        return ERROR;

    stmt->BindId(stmt->GetParameterIndex(":endClassId"), entityClassId);
    MainSchemaManager const& schemaManager = m_ecdb.Schemas().Main();

    while (BE_SQLITE_ROW == stmt->Step())
        {
        ECClassId relClassId = stmt->GetValueId<ECClassId>(0);
        ECRelationshipEnd thisEnd = (ECRelationshipEnd) stmt->GetValueInt(1);

        ECClassCP ecClass = schemaManager.GetClass(relClassId);
        if (ecClass == nullptr)
            continue;

        ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP();
        if (relClass == nullptr)
            continue;

        ClassMap const* classMap = schemaManager.GetClassMap(*relClass);
        if (classMap == nullptr)
            continue;

        MapStrategy strategy = classMap->GetMapStrategy().GetStrategy();
        if (strategy == MapStrategy::NotMapped)
            continue;

        ClassMap::Type mapType = classMap->GetType();
        if (mapType != ClassMap::Type::RelationshipLinkTable && mapType != ClassMap::Type::RelationshipEndTable)
            continue;

        out.push_back(ApplicableRelationship(*relClass, thisEnd, mapType, strategy));
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ApplicableRelationship> const& GraphStatementCache::GetApplicableRelationships(ECN::ECClassId entityClassId)
    {
    auto it = m_relDiscoveryCache.find(entityClassId);
    if (it != m_relDiscoveryCache.end())
        return it->second;

    bvector<ApplicableRelationship>& vec = m_relDiscoveryCache[entityClassId];
    DiscoverRelationshipsForClass(vec, entityClassId);
    return vec;
    }

// =====================================================================================
// GraphStatementCache — SQL Generation Helpers
// =====================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GraphStatementCache::AppendClassHierarchyJoin(Utf8StringR sql, Utf8CP alias, Utf8CP columnExpr, ECN::ECClassId baseClassId, int& joinIdx)
    {
    sql.append(" INNER JOIN [" TABLE_ClassHierarchyCache "] ");
    sql.append(alias);
    sql.append(" ON ").append(alias).append(".ClassId=").append(columnExpr);
    sql.append(" AND ").append(alias).append(".BaseClassId=").append(baseClassId.ToString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GraphStatementCache::AppendClassHierarchyFilter(Utf8StringR sql, Utf8CP columnName, ECN::ECClassId baseClassId)
    {
    sql += Utf8PrintfString(" AND %s IN (SELECT ClassId FROM [" TABLE_ClassHierarchyCache "] WHERE BaseClassId=%s)",
                     columnName, baseClassId.ToString().c_str());
    }

// =====================================================================================
// GraphStatementCache — Link Table SQL Generation
// =====================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GraphStatementCache::BuildLinkTableSql(GraphStatementEntry& entry, RelationshipClassMap const& relMap, TraversalDirection dir)
    {
    Utf8String sql("SELECT ");
    entry.m_direction = dir;

    // Determine which columns to select/filter based on direction
    // Forward: seed is source, find target
    // Backward: seed is target, find source
    ConstraintECInstanceIdPropertyMap const* seedIdPropMap;
    ConstraintECInstanceIdPropertyMap const* relatedIdPropMap;
    ConstraintECClassIdPropertyMap const* seedClassIdPropMap;
    ConstraintECClassIdPropertyMap const* relatedClassIdPropMap;

    if (dir == TraversalDirection::Forward)
        {
        seedIdPropMap = relMap.GetSourceECInstanceIdPropMap();
        seedClassIdPropMap = relMap.GetSourceECClassIdPropMap();
        relatedIdPropMap = relMap.GetTargetECInstanceIdPropMap();
        relatedClassIdPropMap = relMap.GetTargetECClassIdPropMap();
        }
    else
        {
        seedIdPropMap = relMap.GetTargetECInstanceIdPropMap();
        seedClassIdPropMap = relMap.GetTargetECClassIdPropMap();
        relatedIdPropMap = relMap.GetSourceECInstanceIdPropMap();
        relatedClassIdPropMap = relMap.GetSourceECClassIdPropMap();
        }

    if (seedIdPropMap == nullptr || relatedIdPropMap == nullptr)
        return ERROR;

    DbColumn const& seedIdCol = FirstCol(*seedIdPropMap);
    DbColumn const& relatedIdCol = FirstCol(*relatedIdPropMap);
    DbTable const& linkTable = seedIdCol.GetTable();

    // SELECT related instance ID (always physical)
    int colIdx = 0;
    entry.m_relatedInstanceIdColIdx = colIdx++;
    sql += Utf8PrintfString("lt.[%s]", relatedIdCol.GetName().c_str());

    // SELECT related class ID (may be virtual, or may reside in entity table rather than link table)
    // When foreignEndTableCount==1, ECDb stores the constraint ECClassId column on the entity table
    // instead of the link table, so we must check which table the column belongs to.
    bool needRelatedEntityJoin = false;
    Utf8String relatedEntityJoinSql;
    if (relatedClassIdPropMap != nullptr)
        {
        DbColumn const& relatedClassIdCol = FirstCol(*relatedClassIdPropMap);
        if (relatedClassIdCol.IsVirtual())
            {
            entry.m_relatedClassIdColIdx = -1;
            auto const* perTableMap = dynamic_cast<SystemPropertyMap::PerTableClassIdPropertyMap const*>(relatedClassIdPropMap->GetDataPropertyMaps().front());
            entry.m_staticRelatedClassId = perTableMap != nullptr ? perTableMap->GetDefaultECClassId() : ECClassId();
            }
        else if (&relatedClassIdCol.GetTable() == &linkTable)
            {
            entry.m_relatedClassIdColIdx = colIdx++;
            sql += Utf8PrintfString(",lt.[%s]", relatedClassIdCol.GetName().c_str());
            }
        else
            {
            // Column is in entity table — need JOIN to retrieve it
            needRelatedEntityJoin = true;
            DbTable const& entityTable = relatedClassIdCol.GetTable();
            DbColumn const* entityIdCol = entityTable.FindFirst(DbColumn::Kind::ECInstanceId);
            if (entityIdCol != nullptr)
                {
                entry.m_relatedClassIdColIdx = colIdx++;
                sql += Utf8PrintfString(",_re.[%s]", relatedClassIdCol.GetName().c_str());
                relatedEntityJoinSql = Utf8PrintfString(" INNER JOIN [%s] _re ON _re.[%s]=lt.[%s]",
                    entityTable.GetName().c_str(), entityIdCol->GetName().c_str(), relatedIdCol.GetName().c_str());
                }
            else
                {
                entry.m_relatedClassIdColIdx = -1;
                entry.m_staticRelatedClassId = ECClassId();
                }
            }
        }
    else
        {
        entry.m_relatedClassIdColIdx = -1;
        entry.m_staticRelatedClassId = ECClassId();
        }

    // SELECT relationship ECClassId (may be virtual)
    auto const* ecClassIdPropMap = relMap.GetECClassIdPropertyMap();
    if (ecClassIdPropMap != nullptr)
        {
        DbColumn const& ecClassIdCol = FirstCol(*ecClassIdPropMap);
        if (ecClassIdCol.IsVirtual())
            {
            entry.m_relClassIdColIdx = -1;
            auto const* perTableClassIdMap = dynamic_cast<SystemPropertyMap::PerTableClassIdPropertyMap const*>(ecClassIdPropMap->GetDataPropertyMaps().front());
            entry.m_staticRelClassId = perTableClassIdMap != nullptr ? perTableClassIdMap->GetDefaultECClassId() : relMap.GetClass().GetId();
            }
        else
            {
            entry.m_relClassIdColIdx = colIdx++;
            sql += Utf8PrintfString(",lt.[%s]", ecClassIdCol.GetName().c_str());
            }
        }
    else
        {
        entry.m_relClassIdColIdx = -1;
        entry.m_staticRelClassId = relMap.GetClass().GetId();
        }

    // FROM
    sql += Utf8PrintfString(" FROM [%s] lt", linkTable.GetName().c_str());

    // Related entity JOIN (when constraint ECClassId lives on entity table, not link table)
    if (needRelatedEntityJoin)
        sql.append(relatedEntityJoinSql);

    // JOIN for seed constraint class filter — only when the column is physically in the link table.
    // When the column resides in the entity table (foreignEndTableCount==1), the filter is redundant:
    // the discovery query already ensures the relationship is applicable, and the relationship class
    // filter (for TPH) handles disambiguation.
    int joinIdx = 0;
    if (seedClassIdPropMap != nullptr)
        {
        DbColumn const& seedClassIdCol = FirstCol(*seedClassIdPropMap);
        if (!seedClassIdCol.IsVirtual() && &seedClassIdCol.GetTable() == &linkTable)
            {
            Utf8String alias("ch_seed");
            Utf8String colExpr = Utf8PrintfString("lt.[%s]", seedClassIdCol.GetName().c_str());
            ECN::ECRelationshipConstraintCR constraint = (dir == TraversalDirection::Forward)
                ? relMap.GetRelationshipClass().GetSource()
                : relMap.GetRelationshipClass().GetTarget();
            ECClassId constraintBaseClassId = constraint.GetAbstractConstraint() != nullptr
                ? constraint.GetAbstractConstraint()->GetId()
                : ECClassId();
            if (constraintBaseClassId.IsValid())
                AppendClassHierarchyJoin(sql, alias.c_str(), colExpr.c_str(), constraintBaseClassId, joinIdx);
            }
        }

    // WHERE seed instance ID = ?
    sql += Utf8PrintfString(" WHERE lt.[%s]=?", seedIdCol.GetName().c_str());

    // Relationship ECClassId filter for TPH (if physical)
    if (ecClassIdPropMap != nullptr)
        {
        DbColumn const& ecClassIdCol = FirstCol(*ecClassIdPropMap);
        if (!ecClassIdCol.IsVirtual())
            AppendClassHierarchyFilter(sql, Utf8PrintfString("lt.[%s]", ecClassIdCol.GetName().c_str()).c_str(), relMap.GetClass().GetId());
        }

    entry.m_sql = sql;
    return SUCCESS;
    }

// =====================================================================================
// GraphStatementCache — End Table SQL Generation
// =====================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GraphStatementCache::BuildEndTableSql(GraphStatementEntry& entry,
    ForeignKeyPartitionView::Partition const& partition,
    ForeignKeyPartitionView const& fkView,
    ECN::ECRelationshipClassCR relClass,
    TraversalDirection dir)
    {
    entry.m_direction = dir;
    ForeignKeyPartitionView::PersistedEnd persistedEnd = fkView.GetPersistedEnd();

    // Determine traversal semantics:
    // Forward = Source→Target, Backward = Target→Source
    // PersistedEnd::SourceTable means FK is in source table (nav prop on source pointing to target)
    // PersistedEnd::TargetTable means FK is in target table (nav prop on target pointing to source)

    bool fkHolderIsSeed;
    if (persistedEnd == ForeignKeyPartitionView::PersistedEnd::SourceTable)
        fkHolderIsSeed = (dir == TraversalDirection::Forward);  // source table has FK, seed IS the source
    else
        fkHolderIsSeed = (dir == TraversalDirection::Backward); // target table has FK, seed IS the target

    ForeignKeyPartitionView::NavigationInfo navInfo = partition.GetNavigationColumns();
    DbColumn const& navIdCol = navInfo.GetIdColumn();          // FK column pointing to other end
    DbColumn const& navRelClassIdCol = navInfo.GetRelECClassIdColumn();  // relationship class ID column

    DbColumn const& fkTableECInstanceIdCol = partition.GetECInstanceIdColumn();
    DbTable const& fkTable = partition.GetTable();

    // FK-holder entity's ECClassId column (NOT the relationship class ID!)
    // PersistedEnd::SourceTable → FK-holder = Source → GetSourceECClassIdColumn()
    // PersistedEnd::TargetTable → FK-holder = Target → GetTargetECClassIdColumn()
    DbColumn const* fkEntityClassIdCol = (persistedEnd == ForeignKeyPartitionView::PersistedEnd::SourceTable)
        ? partition.GetSourceECClassIdColumn()
        : partition.GetTargetECClassIdColumn();

    Utf8String sql("SELECT ");
    int colIdx = 0;

    if (fkHolderIsSeed)
        {
        // Seed is on FK-holder table, related entity is on the referenced end
        // SELECT the nav prop target ID (= related instance ID)
        entry.m_relatedInstanceIdColIdx = colIdx++;
        sql += Utf8PrintfString("et.[%s]", navIdCol.GetName().c_str());

        // Related class ID: lives on the OTHER table (referenced end) — may need JOIN
        DbTable const* otherEndTable = partition.GetOtherEndTable();
        if (otherEndTable != nullptr)
            {
            // Need JOIN to get the referenced entity's ECClassId
            // Check if the other table's ECClassId is physical
            DbColumn const* otherEndECClassIdCol = nullptr;
            if (dir == TraversalDirection::Forward)
                otherEndECClassIdCol = partition.GetTargetECClassIdColumn();
            else
                otherEndECClassIdCol = partition.GetSourceECClassIdColumn();

            if (otherEndECClassIdCol != nullptr && !otherEndECClassIdCol->IsVirtual())
                {
                entry.m_relatedClassIdColIdx = colIdx++;
                bool isSelfRef = (&fkTable == otherEndTable);
                Utf8CP refAlias = isSelfRef ? "_ReferencedEnd" : "ref_tbl";
                sql += Utf8PrintfString(",%s.[%s]", refAlias, otherEndECClassIdCol->GetName().c_str());
                }
            else if (otherEndECClassIdCol != nullptr && otherEndECClassIdCol->IsVirtual())
                {
                entry.m_relatedClassIdColIdx = -1;
                auto const& classIds = m_ecdb.Schemas().Main().GetLightweightCache().GetClassesForTable(*otherEndTable);
                entry.m_staticRelatedClassId = classIds.size() == 1 ? classIds[0] : ECClassId();
                }
            else
                {
                entry.m_relatedClassIdColIdx = -1;
                entry.m_staticRelatedClassId = ECClassId();
                }
            }
        else
            {
            entry.m_relatedClassIdColIdx = -1;
            entry.m_staticRelatedClassId = ECClassId();
            }
        }
    else
        {
        // Seed is on referenced end, related entity is on FK-holder table
        // SELECT FK-holder ECInstanceId (= related instance ID)
        entry.m_relatedInstanceIdColIdx = colIdx++;
        sql += Utf8PrintfString("et.[%s]", fkTableECInstanceIdCol.GetName().c_str());

        // Related class ID is the FK-holder entity's ECClassId (not the relationship class ID!)
        if (fkEntityClassIdCol != nullptr && !fkEntityClassIdCol->IsVirtual())
            {
            entry.m_relatedClassIdColIdx = colIdx++;
            sql += Utf8PrintfString(",et.[%s]", fkEntityClassIdCol->GetName().c_str());
            }
        else
            {
            entry.m_relatedClassIdColIdx = -1;
            if (fkEntityClassIdCol != nullptr)
                {
                auto const& classIds = m_ecdb.Schemas().Main().GetLightweightCache().GetClassesForTable(fkEntityClassIdCol->GetTable());
                entry.m_staticRelatedClassId = classIds.size() == 1 ? classIds[0] : ECClassId();
                }
            else
                {
                auto const& classIds = m_ecdb.Schemas().Main().GetLightweightCache().GetClassesForTable(fkTable);
                entry.m_staticRelatedClassId = classIds.size() == 1 ? classIds[0] : ECClassId();
                }
            }
        }

    // Relationship class ID (from navProp.RelECClassId)
    if (!navRelClassIdCol.IsVirtual())
        {
        entry.m_relClassIdColIdx = colIdx++;
        sql += Utf8PrintfString(",et.[%s]", navRelClassIdCol.GetName().c_str());
        }
    else
        {
        entry.m_relClassIdColIdx = -1;
        entry.m_staticRelClassId = relClass.GetId();
        }

    // FROM
    sql += Utf8PrintfString(" FROM [%s] et", fkTable.GetName().c_str());

    // Referenced-end JOIN (only when fkHolderIsSeed and referenced end ECClassId is physical)
    if (fkHolderIsSeed)
        {
        DbTable const* otherEndTable = partition.GetOtherEndTable();
        if (otherEndTable != nullptr)
            {
            DbColumn const* otherEndECClassIdCol = (dir == TraversalDirection::Forward)
                ? partition.GetTargetECClassIdColumn()
                : partition.GetSourceECClassIdColumn();

            if (otherEndECClassIdCol != nullptr && !otherEndECClassIdCol->IsVirtual())
                {
                bool isSelfRef = (&fkTable == otherEndTable);
                Utf8CP refAlias = isSelfRef ? "_ReferencedEnd" : "ref_tbl";
                DbColumn const* otherEndIdCol = otherEndTable->FindFirst(DbColumn::Kind::ECInstanceId);
                if (otherEndIdCol != nullptr)
                    sql += Utf8PrintfString(" INNER JOIN [%s] %s ON %s.[%s]=et.[%s]",
                        otherEndTable->GetName().c_str(), refAlias, refAlias, otherEndIdCol->GetName().c_str(), navIdCol.GetName().c_str());
                }
            }
        }

    // FK-holder entity class filter (ec_cache_ClassHierarchy JOIN) — uses entity ECClassId, not rel ECClassId
    if (fkEntityClassIdCol != nullptr && !fkEntityClassIdCol->IsVirtual())
        {
        // Determine which constraint this entity belongs to
        ECN::ECRelationshipConstraintCR constraint = (persistedEnd == ForeignKeyPartitionView::PersistedEnd::SourceTable)
            ? relClass.GetSource()
            : relClass.GetTarget();
        ECClassCP abstractConstraint = constraint.GetAbstractConstraint();
        if (abstractConstraint != nullptr)
            {
            int joinIdx = 0;
            Utf8String colExpr = Utf8PrintfString("et.[%s]", fkEntityClassIdCol->GetName().c_str());
            AppendClassHierarchyJoin(sql, "ch_ent", colExpr.c_str(), abstractConstraint->GetId(), joinIdx);
            }
        }

    // WHERE clause
    if (fkHolderIsSeed)
        {
        // Seed is FK holder: filter by ECInstanceId
        sql += Utf8PrintfString(" WHERE et.[%s]=?", fkTableECInstanceIdCol.GetName().c_str());
        // NavProp must not be null
        sql += Utf8PrintfString(" AND et.[%s] IS NOT NULL", navIdCol.GetName().c_str());
        }
    else
        {
        // Seed is referenced end: filter by nav prop FK = seed ID
        sql += Utf8PrintfString(" WHERE et.[%s]=?", navIdCol.GetName().c_str());
        }

    // Relationship class filter (for TPH: multiple rel hierarchies in same table)
    if (!navRelClassIdCol.IsVirtual())
        AppendClassHierarchyFilter(sql, Utf8PrintfString("et.[%s]", navRelClassIdCol.GetName().c_str()).c_str(), relClass.GetId());

    // Shared column disambiguation
    if (navIdCol.IsShared() && !fkHolderIsSeed)
        {
        // When querying backward through a shared column, add foreign-end class filter
        ECN::ECRelationshipConstraintCR foreignConstraint = (persistedEnd == ForeignKeyPartitionView::PersistedEnd::SourceTable)
            ? relClass.GetTarget()
            : relClass.GetSource();
        ECClassCP foreignAbstractConstraint = foreignConstraint.GetAbstractConstraint();
        DbColumn const* foreignClassIdCol = (dir == TraversalDirection::Forward)
            ? partition.GetTargetECClassIdColumn()
            : partition.GetSourceECClassIdColumn();

        if (foreignAbstractConstraint != nullptr && foreignClassIdCol != nullptr && !foreignClassIdCol->IsVirtual())
            AppendClassHierarchyFilter(sql, Utf8PrintfString("et.[%s]", foreignClassIdCol->GetName().c_str()).c_str(), foreignAbstractConstraint->GetId());
        }

    entry.m_sql = sql;
    return SUCCESS;
    }

// =====================================================================================
// GraphStatementCache — Entry Lookup
// =====================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GraphStatementEntry const* GraphStatementCache::GetOrBuildEntry(ApplicableRelationship const& rel, TraversalDirection dir, size_t partitionIdx)
    {
    GraphStatementKey key{rel.m_relClass->GetId(), rel.m_thisEnd, dir, partitionIdx};
    auto it = m_entries.find(key);
    if (it != m_entries.end())
        return &it->second;

    GraphStatementEntry entry;
    BentleyStatus status = ERROR;

    if (rel.m_mapType == ClassMap::Type::RelationshipLinkTable)
        {
        ClassMap const* classMap = m_ecdb.Schemas().Main().GetClassMap(*rel.m_relClass);
        if (classMap == nullptr)
            return nullptr;

        auto const* relMap = dynamic_cast<RelationshipClassMap const*>(classMap);
        if (relMap == nullptr)
            return nullptr;

        status = BuildLinkTableSql(entry, *relMap, dir);
        }
    else if (rel.m_mapType == ClassMap::Type::RelationshipEndTable)
        {
        auto fkView = ForeignKeyPartitionView::CreateReadonly(m_ecdb.Schemas().Main(), *rel.m_relClass);
        if (fkView == nullptr)
            return nullptr;

        auto partitions = fkView->GetPartitions(true /*onlyPhysical*/);
        if (partitionIdx >= partitions.size())
            return nullptr;

        status = BuildEndTableSql(entry, *partitions[partitionIdx], *fkView, *rel.m_relClass, dir);
        }

    if (status != SUCCESS)
        return nullptr;

    auto insertResult = m_entries.emplace(key, std::move(entry));
    return &insertResult.first->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<GraphStatementEntry const*> GraphStatementCache::GetEndTableEntries(ApplicableRelationship const& rel, TraversalDirection dir)
    {
    bvector<GraphStatementEntry const*> result;

    if (rel.m_mapType != ClassMap::Type::RelationshipEndTable)
        return result;

    auto fkView = ForeignKeyPartitionView::CreateReadonly(m_ecdb.Schemas().Main(), *rel.m_relClass);
    if (fkView == nullptr)
        return result;

    auto partitions = fkView->GetPartitions(true /*onlyPhysical*/);
    for (size_t i = 0; i < partitions.size(); ++i)
        {
        auto const* entry = GetOrBuildEntry(rel, dir, i);
        if (entry != nullptr)
            result.push_back(entry);
        }

    return result;
    }

// =====================================================================================
// InstanceGraph — Construction / Destruction
// =====================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceGraph::InstanceGraph(ECDbCR ecdb) : m_ecdb(ecdb) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceGraph::InstanceGraph(ECDbCR ecdb, bset<ECInstanceKey>&& visited, bmap<ECInstanceKey, bvector<RelatedInstance>>&& adjacency)
    : m_ecdb(ecdb), m_visited(std::move(visited)), m_adjacency(std::move(adjacency)) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceGraph::~InstanceGraph() {}

// =====================================================================================
// InstanceGraph — Seeds and Expansion
// =====================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceGraph::AddSeed(ECInstanceKeyCR seed)
    {
    m_seeds.push_back(seed);
    m_visited.insert(seed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceGraph::ExpandNode(ECInstanceKeyCR key, TraversalDirection dir)
    {
    return ExpandNodeInternal(key, dir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceGraph::ExpandNodeInternal(ECInstanceKeyCR key, TraversalDirection dir)
    {
    // Get the ECDb-level shared cache (cleared on ClearECDbCache)
    GraphStatementCache& cache = m_ecdb.GetImpl().GetGraphStatementCache();

    // Ensure expanded node has an adjacency entry even if no results are found
    m_adjacency.emplace(key, bvector<RelatedInstance>());

    bvector<ApplicableRelationship> const& rels = cache.GetApplicableRelationships(key.GetClassId());

    for (auto const& rel : rels)
        {
        // Determine which directions to traverse based on the seed's position
        bvector<TraversalDirection> dirs;
        if (dir == TraversalDirection::Both || dir == TraversalDirection::Forward)
            {
            // Our class is on Source end → forward traversal finds targets
            if (rel.m_thisEnd == ECRelationshipEnd_Source)
                dirs.push_back(TraversalDirection::Forward);
            }
        if (dir == TraversalDirection::Both || dir == TraversalDirection::Backward)
            {
            // Our class is on Target end → backward traversal finds sources
            if (rel.m_thisEnd == ECRelationshipEnd_Target)
                dirs.push_back(TraversalDirection::Backward);
            }

        for (TraversalDirection traversalDir : dirs)
            {
            bvector<GraphStatementEntry const*> entries;

            if (rel.m_mapType == ClassMap::Type::RelationshipLinkTable)
                {
                auto const* entry = cache.GetOrBuildEntry(rel, traversalDir);
                if (entry != nullptr)
                    entries.push_back(entry);
                }
            else if (rel.m_mapType == ClassMap::Type::RelationshipEndTable)
                {
                entries = cache.GetEndTableEntries(rel, traversalDir);
                }

            for (auto const* entry : entries)
                {
                CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement(entry->m_sql.c_str());
                if (stmt == nullptr)
                    {
                    LOG.errorv("InstanceGraph: Failed to prepare SQL for class %" PRIu64 ": %s", key.GetClassId().GetValue(), entry->m_sql.c_str());
                    return ERROR;
                    }

                stmt->BindId(1, key.GetInstanceId());

                while (BE_SQLITE_ROW == stmt->Step())
                    {
                    // Read related instance ID
                    ECInstanceId relatedId = stmt->GetValueId<ECInstanceId>(entry->m_relatedInstanceIdColIdx);
                    if (!relatedId.IsValid())
                        continue;

                    // Read related class ID (physical or virtual/static)
                    ECClassId relatedClassId;
                    if (entry->m_relatedClassIdColIdx >= 0)
                        relatedClassId = stmt->GetValueId<ECClassId>(entry->m_relatedClassIdColIdx);
                    else
                        relatedClassId = entry->m_staticRelatedClassId;

                    // Read relationship class ID (physical or virtual/static)
                    ECClassId relClassId;
                    if (entry->m_relClassIdColIdx >= 0)
                        relClassId = stmt->GetValueId<ECClassId>(entry->m_relClassIdColIdx);
                    else
                        relClassId = entry->m_staticRelClassId;

                    ECInstanceKey relatedKey(relatedClassId, relatedId);
                    m_adjacency[key].push_back(RelatedInstance(relatedKey, relClassId, traversalDir));
                    m_visited.insert(relatedKey);
                    }
                }
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceGraph::ExpandAll(uint8_t maxDepth)
    {
    // BFS expansion
    bvector<ECInstanceKey> currentLevel;
    for (auto const& seed : m_seeds)
        currentLevel.push_back(seed);

    for (uint8_t depth = 0; depth < maxDepth && !currentLevel.empty(); ++depth)
        {
        bvector<ECInstanceKey> nextLevel;
        for (auto const& key : currentLevel)
            {
            // Skip if already expanded (cycle avoidance)
            if (m_adjacency.find(key) != m_adjacency.end())
                continue;

            BentleyStatus status = ExpandNodeInternal(key, TraversalDirection::Both);
            if (status != SUCCESS)
                return status;

            // Collect newly discovered nodes for next level
            auto const* related = GetRelated(key);
            if (related != nullptr)
                {
                for (auto const& rel : *related)
                    {
                    if (m_adjacency.find(rel.GetKey()) == m_adjacency.end())
                        nextLevel.push_back(rel.GetKey());
                    }
                }
            }
        currentLevel = std::move(nextLevel);
        }

    return SUCCESS;
    }

// =====================================================================================
// InstanceGraph — Set Operations
// =====================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceGraph::Overlaps(InstanceGraph const& a, InstanceGraph const& b)
    {
    auto const& smaller = (a.NodeCount() < b.NodeCount()) ? a : b;
    auto const& larger  = (a.NodeCount() < b.NodeCount()) ? b : a;
    for (auto const& key : smaller.m_visited)
        {
        if (larger.Contains(key))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<InstanceGraph> InstanceGraph::Intersection(InstanceGraph const& a, InstanceGraph const& b)
    {
    bset<ECInstanceKey> visited;
    bmap<ECInstanceKey, bvector<RelatedInstance>> adjacency;

    // Find common nodes
    auto const& smaller = (a.NodeCount() < b.NodeCount()) ? a : b;
    auto const& larger  = (a.NodeCount() < b.NodeCount()) ? b : a;
    for (auto const& key : smaller.m_visited)
        {
        if (larger.Contains(key))
            visited.insert(key);
        }

    // Preserve edges where both endpoints are in the intersection
    for (auto const& pair : a.m_adjacency)
        {
        if (visited.find(pair.first) == visited.end())
            continue;
        for (auto const& rel : pair.second)
            {
            if (visited.find(rel.GetKey()) != visited.end())
                adjacency[pair.first].push_back(rel);
            }
        }
    for (auto const& pair : b.m_adjacency)
        {
        if (visited.find(pair.first) == visited.end())
            continue;
        for (auto const& rel : pair.second)
            {
            if (visited.find(rel.GetKey()) != visited.end())
                adjacency[pair.first].push_back(rel);
            }
        }

    return std::unique_ptr<InstanceGraph>(new InstanceGraph(a.m_ecdb, std::move(visited), std::move(adjacency)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<InstanceGraph> InstanceGraph::Union(InstanceGraph const& a, InstanceGraph const& b)
    {
    bset<ECInstanceKey> visited = a.m_visited;
    visited.insert(b.m_visited.begin(), b.m_visited.end());

    bmap<ECInstanceKey, bvector<RelatedInstance>> adjacency = a.m_adjacency;
    for (auto const& pair : b.m_adjacency)
        {
        auto& vec = adjacency[pair.first];
        for (auto const& rel : pair.second)
            vec.push_back(rel);
        }

    return std::unique_ptr<InstanceGraph>(new InstanceGraph(a.m_ecdb, std::move(visited), std::move(adjacency)));
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
