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

//! Identifies a single edge of the graph. Used to suppress duplicates.
//! Duplicates arise naturally because a relationship class and its base classes can all be
//! applicable to the same seed, and each of them matches the very same persisted row.
struct GraphEdgeKey final
    {
    ECInstanceKey       m_related;
    ECN::ECClassId      m_relClassId;
    TraversalDirection  m_direction;

    GraphEdgeKey(RelatedInstance const& rel) : m_related(rel.GetKey()), m_relClassId(rel.GetRelClassId()), m_direction(rel.GetDirection()) {}

    bool operator<(GraphEdgeKey const& rhs) const
        {
        if (m_related != rhs.m_related)
            return m_related < rhs.m_related;
        if (m_relClassId != rhs.m_relClassId)
            return m_relClassId < rhs.m_relClassId;
        return (uint8_t) m_direction < (uint8_t) rhs.m_direction;
        }
    };

//! Appends an edge unless an identical one was already appended.
static void AppendUniqueEdge(bvector<RelatedInstance>& edges, bset<GraphEdgeKey>& seen, RelatedInstance const& rel)
    {
    if (seen.insert(GraphEdgeKey(rel)).second)
        edges.push_back(rel);
    }

//! Returns the column of a SystemPropertyMap in the given table, or nullptr if the property map
//! is not mapped to that table.
//! @remarks SystemPropertyMap is inherently multi-table. Never blindly take the first entry -
//! for constraint class id property maps the entries can span several entity tables.
static DbColumn const* FindColumnInTable(SystemPropertyMap const& map, DbTable const& table)
    {
    auto const* perTableMap = map.FindDataPropertyMap(table);
    return perTableMap == nullptr ? nullptr : &perTableMap->GetColumn();
    }

//! Returns the class id a virtual (i.e. not persisted) class id column stands for, or an invalid id.
static ECClassId GetDefaultClassId(SystemPropertyMap::PerTableIdPropertyMap const& perTableMap)
    {
    auto const* classIdMap = dynamic_cast<SystemPropertyMap::PerTableClassIdPropertyMap const*>(&perTableMap);
    return classIdMap == nullptr ? ECClassId() : classIdMap->GetDefaultECClassId();
    }

//! Determines the single class a table holds instances of. Only meaningful when the table's
//! ECClassId column is virtual, which by definition means the table is exclusive to one class.
static ECClassId GetExclusiveClassIdForTable(ECDbCR ecdb, DbTable const& table)
    {
    auto const& classIds = ecdb.Schemas().Main().GetLightweightCache().GetClassesForTable(table);
    if (classIds.size() == 1)
        return classIds[0];

    return table.HasExclusiveRootECClass() ? table.GetExclusiveRootECClassId() : ECClassId();
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

    DbResult stepStatus;
    while ((stepStatus = stmt->Step()) == BE_SQLITE_ROW)
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

    if (stepStatus != BE_SQLITE_DONE)
        {
        LOG.errorv("InstanceGraph: relationship discovery for class %s failed: %s",
                   entityClassId.ToString().c_str(), BeSQLiteLib::GetErrorName(stepStatus));
        out.clear();
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GraphStatementCache::GetApplicableRelationships(bvector<ApplicableRelationship>& out, ECN::ECClassId entityClassId)
    {
    BeMutexHolder holder(m_mutex);

    auto it = m_relDiscoveryCache.find(entityClassId);
    if (it != m_relDiscoveryCache.end())
        {
        out = it->second;
        return SUCCESS;
        }

    bvector<ApplicableRelationship> discovered;
    if (SUCCESS != DiscoverRelationshipsForClass(discovered, entityClassId))
        return ERROR; // never cache a failure - it would turn a transient error into permanent data loss

    m_relDiscoveryCache[entityClassId] = discovered;
    out = std::move(discovered);
    return SUCCESS;
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
    entry.m_direction = dir;
    entry.m_relatedInstanceIdColIdx = -1;
    entry.m_relatedClassIdColIdx = -1;
    entry.m_relClassIdColIdx = -1;
    entry.m_staticRelatedClassId = ECClassId();
    entry.m_staticRelClassId = ECClassId();

    DbTable const& linkTable = relMap.GetPrimaryTable();
    Utf8CP relClassName = relMap.GetClass().GetFullName();

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
        {
        LOG.errorv("InstanceGraph: relationship '%s' has no constraint ECInstanceId property map.", relClassName);
        return ERROR;
        }

    // The constraint ECInstanceId columns of a link table relationship always live in the link table itself.
    DbColumn const* seedIdCol = FindColumnInTable(*seedIdPropMap, linkTable);
    DbColumn const* relatedIdCol = FindColumnInTable(*relatedIdPropMap, linkTable);
    if (seedIdCol == nullptr || relatedIdCol == nullptr)
        {
        LOG.errorv("InstanceGraph: relationship '%s' constraint ECInstanceId columns are not mapped to link table '%s'.",
                   relClassName, linkTable.GetName().c_str());
        return ERROR;
        }

    // ------------------------------------------------------------------------------
    // Resolve the related instance's ECClassId.
    // The constraint ECClassId column can either live in the link table itself, or - when ECDb
    // determined the constraint end resolves to entity tables - in one or more entity tables.
    // In the latter case join each of those tables and coalesce, so that instances stored in
    // any of them are found (a single INNER JOIN against an arbitrary table would drop rows).
    // ------------------------------------------------------------------------------
    if (relatedClassIdPropMap == nullptr)
        {
        LOG.errorv("InstanceGraph: relationship '%s' has no constraint ECClassId property map.", relClassName);
        return ERROR;
        }

    Utf8String relatedClassIdExpr;
    Utf8String relatedEntityJoinSql;

    if (auto const* relatedClassIdInLinkTable = relatedClassIdPropMap->FindDataPropertyMap(linkTable))
        {
        DbColumn const& col = relatedClassIdInLinkTable->GetColumn();
        if (col.IsVirtual())
            {
            entry.m_staticRelatedClassId = GetDefaultClassId(*relatedClassIdInLinkTable);
            if (!entry.m_staticRelatedClassId.IsValid())
                {
                LOG.errorv("InstanceGraph: relationship '%s' has a virtual constraint ECClassId column without a default class id.", relClassName);
                return ERROR;
                }
            }
        else
            relatedClassIdExpr = Utf8PrintfString("lt.[%s]", col.GetName().c_str());
        }
    else
        {
        bvector<Utf8String> classIdExprs;
        int joinCount = 0;
        for (auto const* perTableMap : relatedClassIdPropMap->GetDataPropertyMaps())
            {
            DbTable const& entityTable = perTableMap->GetColumn().GetTable();
            DbColumn const* entityIdCol = entityTable.FindFirst(DbColumn::Kind::ECInstanceId);
            if (entityIdCol == nullptr)
                continue;

            Utf8String alias = Utf8PrintfString("_re%d", joinCount++);
            relatedEntityJoinSql += Utf8PrintfString(" LEFT JOIN [%s] %s ON %s.[%s]=lt.[%s]",
                entityTable.GetName().c_str(), alias.c_str(), alias.c_str(),
                entityIdCol->GetName().c_str(), relatedIdCol->GetName().c_str());

            if (perTableMap->GetColumn().IsVirtual())
                {
                ECClassId defaultClassId = GetDefaultClassId(*perTableMap);
                if (!defaultClassId.IsValid())
                    defaultClassId = GetExclusiveClassIdForTable(m_ecdb, entityTable);
                if (!defaultClassId.IsValid())
                    continue;

                classIdExprs.push_back(Utf8PrintfString("CASE WHEN %s.[%s] IS NULL THEN NULL ELSE %s END",
                    alias.c_str(), entityIdCol->GetName().c_str(), defaultClassId.ToString().c_str()));
                }
            else
                classIdExprs.push_back(Utf8PrintfString("%s.[%s]", alias.c_str(), perTableMap->GetColumn().GetName().c_str()));
            }

        if (classIdExprs.empty())
            {
            LOG.errorv("InstanceGraph: cannot resolve the related ECClassId for relationship '%s'.", relClassName);
            return ERROR;
            }

        if (classIdExprs.size() == 1)
            relatedClassIdExpr = classIdExprs[0];
        else
            {
            relatedClassIdExpr = "COALESCE(";
            for (size_t i = 0; i < classIdExprs.size(); ++i)
                {
                if (i > 0)
                    relatedClassIdExpr.append(",");
                relatedClassIdExpr.append(classIdExprs[i]);
                }
            relatedClassIdExpr.append(")");
            }
        }

    // ------------------------------------------------------------------------------
    // SELECT list
    // ------------------------------------------------------------------------------
    int colIdx = 0;
    Utf8String sql("SELECT ");
    entry.m_relatedInstanceIdColIdx = colIdx++;
    sql += Utf8PrintfString("lt.[%s]", relatedIdCol->GetName().c_str());

    if (!relatedClassIdExpr.empty())
        {
        entry.m_relatedClassIdColIdx = colIdx++;
        sql += "," + relatedClassIdExpr;
        }

    // Relationship ECClassId (may be virtual)
    auto const* ecClassIdPropMap = relMap.GetECClassIdPropertyMap();
    DbColumn const* relECClassIdCol = ecClassIdPropMap != nullptr ? FindColumnInTable(*ecClassIdPropMap, linkTable) : nullptr;
    if (relECClassIdCol != nullptr && !relECClassIdCol->IsVirtual())
        {
        entry.m_relClassIdColIdx = colIdx++;
        sql += Utf8PrintfString(",lt.[%s]", relECClassIdCol->GetName().c_str());
        }
    else
        {
        entry.m_staticRelClassId = relMap.GetClass().GetId();
        if (relECClassIdCol != nullptr)
            {
            ECClassId defaultClassId = GetDefaultClassId(*ecClassIdPropMap->FindDataPropertyMap(linkTable));
            if (defaultClassId.IsValid())
                entry.m_staticRelClassId = defaultClassId;
            }
        }

    // ------------------------------------------------------------------------------
    // FROM / JOINs / WHERE
    // ------------------------------------------------------------------------------
    sql += Utf8PrintfString(" FROM [%s] lt", linkTable.GetName().c_str());
    sql.append(relatedEntityJoinSql);

    // Seed constraint class filter - only when the column is physically in the link table.
    // When the column resides in an entity table the filter is redundant: the discovery query
    // already established the relationship is applicable to the seed's class.
    if (seedClassIdPropMap != nullptr)
        {
        DbColumn const* seedClassIdCol = FindColumnInTable(*seedClassIdPropMap, linkTable);
        if (seedClassIdCol != nullptr && !seedClassIdCol->IsVirtual())
            {
            ECN::ECRelationshipConstraintCR constraint = (dir == TraversalDirection::Forward)
                ? relMap.GetRelationshipClass().GetSource()
                : relMap.GetRelationshipClass().GetTarget();
            ECClassCP abstractConstraint = constraint.GetAbstractConstraint();
            if (abstractConstraint != nullptr)
                {
                int joinIdx = 0;
                Utf8String colExpr = Utf8PrintfString("lt.[%s]", seedClassIdCol->GetName().c_str());
                AppendClassHierarchyJoin(sql, "ch_seed", colExpr.c_str(), abstractConstraint->GetId(), joinIdx);
                }
            }
        }

    sql += Utf8PrintfString(" WHERE lt.[%s]=?", seedIdCol->GetName().c_str());

    // Relationship ECClassId filter for TPH (if physical)
    if (relECClassIdCol != nullptr && !relECClassIdCol->IsVirtual())
        AppendClassHierarchyFilter(sql, Utf8PrintfString("lt.[%s]", relECClassIdCol->GetName().c_str()).c_str(), relMap.GetClass().GetId());

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
    entry.m_relatedInstanceIdColIdx = -1;
    entry.m_relatedClassIdColIdx = -1;
    entry.m_relClassIdColIdx = -1;
    entry.m_staticRelatedClassId = ECClassId();
    entry.m_staticRelClassId = ECClassId();
    Utf8CP relClassName = relClass.GetFullName();
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
        if (otherEndTable == nullptr)
            {
            LOG.errorv("InstanceGraph: cannot resolve the referenced end table of relationship '%s'.", relClassName);
            return ERROR;
            }

        // Need JOIN to get the referenced entity's ECClassId
        // Check if the other table's ECClassId is physical
        DbColumn const* otherEndECClassIdCol = (dir == TraversalDirection::Forward)
            ? partition.GetTargetECClassIdColumn()
            : partition.GetSourceECClassIdColumn();

        if (otherEndECClassIdCol == nullptr)
            {
            LOG.errorv("InstanceGraph: cannot resolve the referenced end ECClassId column of relationship '%s'.", relClassName);
            return ERROR;
            }

        if (!otherEndECClassIdCol->IsVirtual())
            {
            entry.m_relatedClassIdColIdx = colIdx++;
            bool isSelfRef = (&fkTable == otherEndTable);
            Utf8CP refAlias = isSelfRef ? "_ReferencedEnd" : "ref_tbl";
            sql += Utf8PrintfString(",%s.[%s]", refAlias, otherEndECClassIdCol->GetName().c_str());
            }
        else
            {
            // A virtual ECClassId column means the table is exclusive to a single class.
            entry.m_staticRelatedClassId = GetExclusiveClassIdForTable(m_ecdb, *otherEndTable);
            if (!entry.m_staticRelatedClassId.IsValid())
                {
                LOG.errorv("InstanceGraph: cannot resolve the related ECClassId of relationship '%s' (table '%s' is not exclusive to a single class).",
                           relClassName, otherEndTable->GetName().c_str());
                return ERROR;
                }
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
            DbTable const& classIdTable = fkEntityClassIdCol != nullptr ? fkEntityClassIdCol->GetTable() : fkTable;
            entry.m_staticRelatedClassId = GetExclusiveClassIdForTable(m_ecdb, classIdTable);
            if (!entry.m_staticRelatedClassId.IsValid())
                {
                LOG.errorv("InstanceGraph: cannot resolve the related ECClassId of relationship '%s' (table '%s' is not exclusive to a single class).",
                           relClassName, classIdTable.GetName().c_str());
                return ERROR;
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
    if (fkHolderIsSeed && entry.m_relatedClassIdColIdx >= 0)
        {
        DbTable const* otherEndTable = partition.GetOtherEndTable();
        DbColumn const* otherEndECClassIdCol = (dir == TraversalDirection::Forward)
            ? partition.GetTargetECClassIdColumn()
            : partition.GetSourceECClassIdColumn();

        BeAssert(otherEndTable != nullptr && otherEndECClassIdCol != nullptr && !otherEndECClassIdCol->IsVirtual());
        bool isSelfRef = (&fkTable == otherEndTable);
        Utf8CP refAlias = isSelfRef ? "_ReferencedEnd" : "ref_tbl";
        DbColumn const* otherEndIdCol = otherEndTable->FindFirst(DbColumn::Kind::ECInstanceId);
        if (otherEndIdCol == nullptr)
            {
            // The SELECT list already references refAlias, so we cannot silently drop the join.
            LOG.errorv("InstanceGraph: referenced end table '%s' of relationship '%s' has no ECInstanceId column.",
                       otherEndTable->GetName().c_str(), relClassName);
            return ERROR;
            }

        sql += Utf8PrintfString(" INNER JOIN [%s] %s ON %s.[%s]=et.[%s]",
            otherEndTable->GetName().c_str(), refAlias, refAlias, otherEndIdCol->GetName().c_str(), navIdCol.GetName().c_str());
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

    // NOTE: No extra filter is needed when the navigation id column is shared. Disambiguation
    // between the relationships sharing that column is already provided by the relationship
    // class hierarchy filter above (and, when the relationship class id column is virtual, the
    // column is not shared with any other relationship in the first place). An earlier version
    // filtered the FK holder's own ECClassId column against the *other* end's constraint
    // hierarchy, which are disjoint hierarchies and therefore always matched zero rows.

    entry.m_sql = sql;
    return SUCCESS;
    }

// =====================================================================================
// GraphStatementCache — Entry Lookup
// =====================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GraphStatementCache::GetOrBuildEntryUnsafe(GraphStatementEntry const*& out, ApplicableRelationship const& rel, TraversalDirection dir, size_t partitionIdx)
    {
    out = nullptr;
    GraphStatementKey key{rel.m_relClass->GetId(), rel.m_thisEnd, dir, partitionIdx};
    auto it = m_entries.find(key);
    if (it != m_entries.end())
        {
        out = &it->second;
        return SUCCESS;
        }

    GraphStatementEntry entry;
    BentleyStatus status = ERROR;

    if (rel.m_mapType == ClassMap::Type::RelationshipLinkTable)
        {
        ClassMap const* classMap = m_ecdb.Schemas().Main().GetClassMap(*rel.m_relClass);
        auto const* relMap = classMap != nullptr ? dynamic_cast<RelationshipClassMap const*>(classMap) : nullptr;
        if (relMap == nullptr)
            {
            LOG.errorv("InstanceGraph: no relationship class map for '%s'.", rel.m_relClass->GetFullName());
            return ERROR;
            }

        status = BuildLinkTableSql(entry, *relMap, dir);
        }
    else if (rel.m_mapType == ClassMap::Type::RelationshipEndTable)
        {
        auto fkView = ForeignKeyPartitionView::CreateReadonly(m_ecdb.Schemas().Main(), *rel.m_relClass);
        if (fkView == nullptr)
            {
            LOG.errorv("InstanceGraph: no foreign key partition view for '%s'.", rel.m_relClass->GetFullName());
            return ERROR;
            }

        auto partitions = fkView->GetPartitions(true /*onlyPhysical*/);
        if (partitionIdx >= partitions.size())
            {
            LOG.errorv("InstanceGraph: partition %" PRIu64 " out of range for '%s'.", (uint64_t) partitionIdx, rel.m_relClass->GetFullName());
            return ERROR;
            }

        status = BuildEndTableSql(entry, *partitions[partitionIdx], *fkView, *rel.m_relClass, dir);
        }
    else
        {
        BeAssert(false && "Unexpected relationship map type");
        return ERROR;
        }

    if (status != SUCCESS)
        return ERROR;

    auto insertResult = m_entries.emplace(key, std::move(entry));
    out = &insertResult.first->second;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GraphStatementCache::GetOrBuildEntry(GraphStatementEntry& out, ApplicableRelationship const& rel, TraversalDirection dir, size_t partitionIdx)
    {
    BeMutexHolder holder(m_mutex);

    GraphStatementEntry const* entry = nullptr;
    if (SUCCESS != GetOrBuildEntryUnsafe(entry, rel, dir, partitionIdx))
        return ERROR;

    out = *entry; // copy out: the cache may be cleared while the caller steps the statement
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GraphStatementCache::GetEndTableEntries(bvector<GraphStatementEntry>& out, ApplicableRelationship const& rel, TraversalDirection dir)
    {
    out.clear();

    if (rel.m_mapType != ClassMap::Type::RelationshipEndTable)
        {
        BeAssert(false && "GetEndTableEntries called for a non end-table relationship");
        return ERROR;
        }

    size_t partitionCount = 0;
        {
        auto fkView = ForeignKeyPartitionView::CreateReadonly(m_ecdb.Schemas().Main(), *rel.m_relClass);
        if (fkView == nullptr)
            {
            LOG.errorv("InstanceGraph: no foreign key partition view for '%s'.", rel.m_relClass->GetFullName());
            return ERROR;
            }

        partitionCount = fkView->GetPartitions(true /*onlyPhysical*/).size();
        }

    BeMutexHolder holder(m_mutex);
    for (size_t i = 0; i < partitionCount; ++i)
        {
        GraphStatementEntry const* entry = nullptr;
        if (SUCCESS != GetOrBuildEntryUnsafe(entry, rel, dir, i))
            return ERROR;

        out.push_back(*entry);
        }

    return SUCCESS;
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
    // A seed added twice must not be traversed twice.
    for (auto const& existing : m_seeds)
        {
        if (existing == seed)
            return;
        }

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
    if (!key.IsValid())
        return ERROR;

    // Get the ECDb-level shared cache (cleared on ClearECDbCache)
    GraphStatementCache& cache = m_ecdb.GetImpl().GetGraphStatementCache();

    bvector<ApplicableRelationship> rels;
    if (SUCCESS != cache.GetApplicableRelationships(rels, key.GetClassId()))
        return ERROR;

    // Snapshot the SQL entries up front. They are copies, so the ECDb level cache may be
    // cleared (ECDb::ClearECDbCache) while the statements below are being stepped.
    bvector<bpair<GraphStatementEntry, TraversalDirection>> plan;
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
            if (rel.m_mapType == ClassMap::Type::RelationshipLinkTable)
                {
                GraphStatementEntry entry;
                if (SUCCESS != cache.GetOrBuildEntry(entry, rel, traversalDir))
                    return ERROR;

                plan.push_back(make_bpair(std::move(entry), traversalDir));
                }
            else if (rel.m_mapType == ClassMap::Type::RelationshipEndTable)
                {
                bvector<GraphStatementEntry> entries;
                if (SUCCESS != cache.GetEndTableEntries(entries, rel, traversalDir))
                    return ERROR;

                for (auto& entry : entries)
                    plan.push_back(make_bpair(std::move(entry), traversalDir));
                }
            }
        }

    // The node is part of the graph and is now considered expanded, even if it has no edges.
    // Re-expanding a node replaces its edges rather than appending a second copy of each.
    m_visited.insert(key);
    bvector<RelatedInstance>& edges = m_adjacency[key];
    edges.clear();

    // A relationship class and its base classes can all be applicable to the same seed and all
    // match the very same persisted row, so identical edges must be suppressed.
    bset<GraphEdgeKey> seenEdges;

    for (auto const& planEntry : plan)
        {
        GraphStatementEntry const& entry = planEntry.first;
        TraversalDirection traversalDir = planEntry.second;

        CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement(entry.m_sql.c_str());
        if (stmt == nullptr)
            {
            LOG.errorv("InstanceGraph: Failed to prepare SQL for class %s: %s", key.GetClassId().ToString().c_str(), entry.m_sql.c_str());
            return ERROR;
            }

        stmt->BindId(1, key.GetInstanceId());

        DbResult stepStatus;
        while ((stepStatus = stmt->Step()) == BE_SQLITE_ROW)
            {
            // Read related instance ID
            ECInstanceId relatedId = stmt->GetValueId<ECInstanceId>(entry.m_relatedInstanceIdColIdx);
            if (!relatedId.IsValid())
                continue;

            // Read related class ID (physical or virtual/static)
            ECClassId relatedClassId;
            if (entry.m_relatedClassIdColIdx >= 0)
                relatedClassId = stmt->GetValueId<ECClassId>(entry.m_relatedClassIdColIdx);
            else
                relatedClassId = entry.m_staticRelatedClassId;

            // Never insert a node with an unresolved class id: it would create a bogus node that
            // does not de-duplicate against the same instance found via another path, and that
            // cannot be expanded further.
            if (!relatedClassId.IsValid())
                {
                LOG.warningv("InstanceGraph: skipping related instance %s because its ECClassId could not be resolved. SQL: %s",
                             relatedId.ToString().c_str(), entry.m_sql.c_str());
                continue;
                }

            // Read relationship class ID (physical or virtual/static)
            ECClassId relClassId;
            if (entry.m_relClassIdColIdx >= 0)
                relClassId = stmt->GetValueId<ECClassId>(entry.m_relClassIdColIdx);
            else
                relClassId = entry.m_staticRelClassId;

            ECInstanceKey relatedKey(relatedClassId, relatedId);
            AppendUniqueEdge(edges, seenEdges, RelatedInstance(relatedKey, relClassId, traversalDir));
            m_visited.insert(relatedKey);
            }

        if (stepStatus != BE_SQLITE_DONE)
            {
            LOG.errorv("InstanceGraph: traversal failed (%s). SQL: %s", BeSQLiteLib::GetErrorName(stepStatus), entry.m_sql.c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceGraph::ExpandAll(uint8_t maxDepth)
    {
    if (maxDepth == 0)
        {
        // Seed only: the seeds are part of the graph and have no (known) edges.
        for (auto const& seed : m_seeds)
            m_adjacency.emplace(seed, bvector<RelatedInstance>());

        return SUCCESS;
        }

    // BFS expansion
    bvector<ECInstanceKey> currentLevel = m_seeds;

    for (uint8_t depth = 0; depth < maxDepth && !currentLevel.empty(); ++depth)
        {
        bvector<ECInstanceKey> nextLevel;
        bset<ECInstanceKey> queuedForNextLevel;

        for (auto const& key : currentLevel)
            {
            // Only expand a node once (cycle avoidance), but always look at its edges: a node
            // may already have been expanded by an earlier ExpandNode/ExpandAll call, and its
            // neighbours would otherwise never be visited.
            if (m_adjacency.find(key) == m_adjacency.end())
                {
                if (SUCCESS != ExpandNodeInternal(key, TraversalDirection::Both))
                    return ERROR;
                }

            auto const* related = GetRelated(key);
            if (related == nullptr)
                continue;

            for (auto const& rel : *related)
                {
                if (m_adjacency.find(rel.GetKey()) != m_adjacency.end())
                    continue; // already expanded

                if (queuedForNextLevel.insert(rel.GetKey()).second)
                    nextLevel.push_back(rel.GetKey());
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
    // Instance keys of different files are not comparable.
    if (&a.m_ecdb != &b.m_ecdb)
        {
        BeAssert(false && "InstanceGraph::Overlaps requires both graphs to belong to the same ECDb");
        return false;
        }

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
    // Instance keys of different files are not comparable.
    if (&a.m_ecdb != &b.m_ecdb)
        {
        BeAssert(false && "InstanceGraph::Intersection requires both graphs to belong to the same ECDb");
        return nullptr;
        }

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

    // Preserve edges where both endpoints are in the intersection. The same edge is typically
    // present in both graphs, so it must only be kept once.
    bmap<ECInstanceKey, bset<GraphEdgeKey>> seenEdges;
    for (InstanceGraph const* graph : {&a, &b})
        {
        for (auto const& pair : graph->m_adjacency)
            {
            if (visited.find(pair.first) == visited.end())
                continue;

            for (auto const& rel : pair.second)
                {
                if (visited.find(rel.GetKey()) == visited.end())
                    continue;

                AppendUniqueEdge(adjacency[pair.first], seenEdges[pair.first], rel);
                }
            }
        }

    return std::unique_ptr<InstanceGraph>(new InstanceGraph(a.m_ecdb, std::move(visited), std::move(adjacency)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<InstanceGraph> InstanceGraph::Union(InstanceGraph const& a, InstanceGraph const& b)
    {
    // Instance keys of different files are not comparable.
    if (&a.m_ecdb != &b.m_ecdb)
        {
        BeAssert(false && "InstanceGraph::Union requires both graphs to belong to the same ECDb");
        return nullptr;
        }

    bset<ECInstanceKey> visited = a.m_visited;
    visited.insert(b.m_visited.begin(), b.m_visited.end());

    bmap<ECInstanceKey, bvector<RelatedInstance>> adjacency;
    bmap<ECInstanceKey, bset<GraphEdgeKey>> seenEdges;
    for (InstanceGraph const* graph : {&a, &b})
        {
        for (auto const& pair : graph->m_adjacency)
            {
            // Make sure an expanded node without edges keeps an (empty) adjacency entry.
            bvector<RelatedInstance>& edges = adjacency[pair.first];
            bset<GraphEdgeKey>& seen = seenEdges[pair.first];
            for (auto const& rel : pair.second)
                AppendUniqueEdge(edges, seen, rel);
            }
        }

    return std::unique_ptr<InstanceGraph>(new InstanceGraph(a.m_ecdb, std::move(visited), std::move(adjacency)));
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
