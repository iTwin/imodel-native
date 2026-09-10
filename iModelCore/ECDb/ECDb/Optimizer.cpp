/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/Optimizer.h>
#include "ECDbImpl.h"
#include "IntegrityChecker.h"
#include "SchemaManagerDispatcher.h"
#include "DbSchemaPersistenceManager.h"
#include "RemapManager.h"
#include "ClassMapColumnFactory.h"
#include "ECSql/NativeSqlBuilder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

namespace
{
struct OptionEntry final
    {
    Optimizer::Options m_option;
    Utf8CP m_name;
    };

constexpr OptionEntry s_optionEntries[] = {
    {Optimizer::Options::PurgeInvalidClassIds, "purge_invalid_class_ids"},
    {Optimizer::Options::DeleteOrphanRelationships, "delete_orphan_relationships"},
    {Optimizer::Options::NullifyOrphanNavProps, "nullify_orphan_navigation_properties"},
    {Optimizer::Options::CleanOrphanCustomAttributes, "clean_orphan_custom_attributes"},
    {Optimizer::Options::DropUnusedSchemas, "drop_unused_schemas"},
    {Optimizer::Options::DropEmptyDynamicClasses, "drop_empty_dynamic_classes"},
    {Optimizer::Options::DropEmptyDynamicProperties, "drop_empty_dynamic_properties"},
    {Optimizer::Options::CompactSharedColumns, "compact_shared_columns"},
    {Optimizer::Options::DropUnmappedTables, "drop_unmapped_tables"},
    {Optimizer::Options::DropUnmappedSharedColumns, "drop_unmapped_shared_columns"},
    {Optimizer::Options::RunAnalyze, "analyze"},
};

constexpr uint32_t ToMask(Optimizer::Options options)
    {
    return static_cast<uint32_t>(options);
    }

bool Contains(Optimizer::Options options, Optimizer::Options option)
    {
    return (ToMask(options) & ToMask(option)) != 0;
    }

bool ChangesMetadata(Optimizer::Options option)
    {
    return option == Optimizer::Options::CleanOrphanCustomAttributes ||
        option == Optimizer::Options::DropUnusedSchemas ||
        option == Optimizer::Options::DropEmptyDynamicClasses ||
        option == Optimizer::Options::DropEmptyDynamicProperties ||
        option == Optimizer::Options::CompactSharedColumns ||
        option == Optimizer::Options::DropUnmappedTables ||
        option == Optimizer::Options::DropUnmappedSharedColumns;
    }

bool RequiresCrudWritePermission(Optimizer::Options options)
    {
    constexpr uint32_t dataWriteMask =
        static_cast<uint32_t>(Optimizer::Options::PurgeInvalidClassIds) |
        static_cast<uint32_t>(Optimizer::Options::DeleteOrphanRelationships) |
        static_cast<uint32_t>(Optimizer::Options::NullifyOrphanNavProps) |
        static_cast<uint32_t>(Optimizer::Options::CompactSharedColumns);
    return (ToMask(options) & dataWriteMask) != 0;
    }

bool RequiresSchemaImportPermission(Optimizer::Options options)
    {
    for (OptionEntry const& entry : s_optionEntries)
        if (Contains(options, entry.m_option) && ChangesMetadata(entry.m_option))
            return true;
    return false;
    }

DbResult CountRows(ECDbCR ecdb, Utf8StringCR sql, uint64_t& count)
    {
    Statement stmt;
    if (const auto rc = stmt.Prepare(ecdb, sql.c_str()); rc != BE_SQLITE_OK)
        return rc;
    if (const auto rc = stmt.Step(); rc != BE_SQLITE_ROW)
        return rc;
    count += stmt.GetValueUInt64(0);
    return BE_SQLITE_OK;
    }

DbResult PurgeInvalidClassIds(ECDbR ecdb, bool isDryRun, uint64_t& candidates, uint64_t& changed)
    {
    ECDB_PERF_LOG_SCOPE("Optimizer> Purge invalid class IDs");
    candidates = 0;
    changed = 0;
    Statement columns;
    auto rc = columns.Prepare(ecdb, R"sql(
        SELECT t.Name, c.Name
        FROM ec_Table t
        JOIN ec_Column c ON c.TableId=t.Id
        WHERE t.Type IN (0,1,3)
          AND c.ColumnKind=2
          AND c.IsVirtual=0
        ORDER BY t.Id)sql");
    if (rc != BE_SQLITE_OK)
        return rc;

    while ((rc = columns.Step()) == BE_SQLITE_ROW)
        {
        Utf8String tableName = columns.GetValueText(0);
        Utf8String columnName = columns.GetValueText(1);
        NativeSqlBuilder predicate;
        predicate.AppendEscaped(columnName)
            .Append(" IS NOT NULL AND NOT EXISTS (SELECT 1 FROM [main].[ec_Class] c WHERE c.[Id]=")
            .AppendEscaped(columnName).Append(")");

        // Candidates are the invalid-class-id rows discovered before any deletion.
        NativeSqlBuilder countSql("SELECT COUNT(*) FROM [main].");
        countSql.AppendEscaped(tableName).Append(" WHERE ").Append(predicate);
        if ((rc = CountRows(ecdb, countSql.GetSql(), candidates)) != BE_SQLITE_OK)
            return rc;

        if (isDryRun)
            continue;

        NativeSqlBuilder sql("DELETE FROM [main].");
        sql.AppendEscaped(tableName).Append(" WHERE ").Append(predicate);
        if ((rc = ecdb.ExecuteSql(sql.GetSql().c_str())) != BE_SQLITE_OK && rc != BE_SQLITE_DONE)
            return rc;
        // Changed is the actual number of rows deleted.
        changed += static_cast<uint64_t>(ecdb.GetModifiedRowCount());
        }

    return rc == BE_SQLITE_DONE ? BE_SQLITE_OK : rc;
    }

constexpr Utf8CP s_orphanCustomAttributeCte = R"sql(
    WITH orphan(Id) AS (
        SELECT ca.Id
        FROM [main].[ec_CustomAttribute] ca
        WHERE ca.ContainerType=1
          AND NOT EXISTS (SELECT 1 FROM [main].[ec_Schema] container WHERE container.Id=ca.ContainerId)
        UNION ALL
        SELECT ca.Id
        FROM [main].[ec_CustomAttribute] ca
        WHERE ca.ContainerType=30
          AND NOT EXISTS (SELECT 1 FROM [main].[ec_Class] container WHERE container.Id=ca.ContainerId)
        UNION ALL
        SELECT ca.Id
        FROM [main].[ec_CustomAttribute] ca
        WHERE ca.ContainerType=992
          AND NOT EXISTS (SELECT 1 FROM [main].[ec_Property] container WHERE container.Id=ca.ContainerId)
        UNION ALL
        SELECT ca.Id
        FROM [main].[ec_CustomAttribute] ca
        WHERE ca.ContainerType=1024
          AND NOT EXISTS (
              SELECT 1
              FROM [main].[ec_RelationshipConstraint] container
              WHERE container.Id=ca.ContainerId AND container.RelationshipEnd=0)
        UNION ALL
        SELECT ca.Id
        FROM [main].[ec_CustomAttribute] ca
        WHERE ca.ContainerType=2048
          AND NOT EXISTS (
              SELECT 1
              FROM [main].[ec_RelationshipConstraint] container
              WHERE container.Id=ca.ContainerId AND container.RelationshipEnd=1)
    )
)sql";

DbResult CleanOrphanCustomAttributes(ECDbR ecdb, bool isDryRun, uint64_t& candidates, uint64_t& changed)
    {
    ECDB_PERF_LOG_SCOPE("Optimizer> Clean orphan custom attributes");
    candidates = 0;
    changed = 0;

    Statement countStmt;
    Utf8String countSql(s_orphanCustomAttributeCte);
    countSql.append("SELECT COUNT(*) FROM orphan");
    auto rc = countStmt.Prepare(ecdb, countSql.c_str());
    if (rc != BE_SQLITE_OK)
        return rc;
    if ((rc = countStmt.Step()) != BE_SQLITE_ROW)
        return rc;
    candidates = countStmt.GetValueUInt64(0);
    countStmt.Finalize();

    if (isDryRun || candidates == 0)
        return BE_SQLITE_OK;

    Statement deleteStmt;
    Utf8String deleteSql(s_orphanCustomAttributeCte);
    deleteSql.append("DELETE FROM [main].[ec_CustomAttribute] WHERE Id IN (SELECT Id FROM orphan)");
    if ((rc = deleteStmt.Prepare(ecdb, deleteSql.c_str())) != BE_SQLITE_OK)
        return rc;
    if ((rc = deleteStmt.Step()) != BE_SQLITE_DONE)
        return rc;

    changed = static_cast<uint64_t>(ecdb.GetModifiedRowCount());
    return BE_SQLITE_OK;
    }

bool IsProtectedSchema(ECSchemaCR schema)
    {
    return MainSchemaManager::IsProtectedFromOptimizer(schema);
    }

struct TempTableGuard final
    {
    ECDbR m_ecdb;
    Utf8CP m_name;
    TempTableGuard(ECDbR ecdb, Utf8CP name) : m_ecdb(ecdb), m_name(name) {}
    ~TempTableGuard()
        {
        m_ecdb.TryExecuteSql(SqlPrintfString("DROP TABLE IF EXISTS [temp].[%s]", m_name).GetUtf8CP());
        }
    };

//! RAII guard that coherently invalidates the whole ECDb schema/map cache when a metadata-changing
//! phase returns without dismissing it. This prevents cached ClassMap objects from retaining raw
//! pointers into DbTable/DbColumn state that a failed (or partially applied) phase mutated.
struct CacheInvalidationGuard final
    {
    ECDbR m_ecdb;
    bool m_armed;
    explicit CacheInvalidationGuard(ECDbR ecdb) : m_ecdb(ecdb), m_armed(true) {}
    void Dismiss() { m_armed = false; }
    ~CacheInvalidationGuard() { if (m_armed) m_ecdb.ClearECDbCache(); }
    };

DbResult DropUnusedSchemas(ECDbR ecdb, bool isDryRun, SchemaImportToken const* token, uint64_t& candidates, uint64_t& changed, Utf8StringR error)
    {
    ECDB_PERF_LOG_SCOPE("Optimizer> Drop unused schemas");
    constexpr Utf8CP liveSchemaTable = "ecdbopt_live_schema";
    TempTableGuard tempGuard(ecdb, liveSchemaTable);
    auto rc = ecdb.TryExecuteSql(
        "DROP TABLE IF EXISTS [temp].[ecdbopt_live_schema];"
        "CREATE TEMP TABLE [ecdbopt_live_schema](Id INTEGER PRIMARY KEY) WITHOUT ROWID");
    if (rc != BE_SQLITE_OK)
        {
        error = ecdb.GetLastError();
        return rc;
        }

    Statement mappedTables;
    rc = mappedTables.Prepare(ecdb, R"sql(
        SELECT t.Name, c.Name, c.IsVirtual, t.ExclusiveRootClassId
        FROM ec_Table t
        JOIN ec_Column c ON c.TableId=t.Id AND c.ColumnKind=2
        WHERE t.Type IN (0,1,3)
        ORDER BY t.Id)sql");
    if (rc != BE_SQLITE_OK)
        {
        error = ecdb.GetLastError();
        return rc;
        }

    while ((rc = mappedTables.Step()) == BE_SQLITE_ROW)
        {
        Utf8String tableName = mappedTables.GetValueText(0);
        Utf8String classIdColumn = mappedTables.GetValueText(1);
        bool isVirtual = mappedTables.GetValueBoolean(2);
        NativeSqlBuilder sql("INSERT OR IGNORE INTO [temp].[ecdbopt_live_schema] ");
        if (isVirtual)
            {
            if (mappedTables.IsColumnNull(3))
                continue;
            sql.Append("SELECT c.SchemaId FROM ec_Class c WHERE c.Id=")
                .Append(mappedTables.GetValueId<ECClassId>(3))
                .Append(" AND EXISTS(SELECT 1 FROM [main].")
                .AppendEscaped(tableName).Append(" LIMIT 1)");
            }
        else
            {
            sql.Append("SELECT DISTINCT c.SchemaId FROM [main].")
                .AppendEscaped(tableName).Append(" d JOIN ec_Class c ON c.Id=d.")
                .AppendEscaped(classIdColumn);
            }
        if ((rc = ecdb.ExecuteSql(sql.GetSql().c_str())) != BE_SQLITE_OK)
            {
            error = ecdb.GetLastError();
            return rc;
            }
        }
    if (rc != BE_SQLITE_DONE)
        {
        error = ecdb.GetLastError();
        return rc;
        }

    bvector<ECSchemaCP> schemas = ecdb.Schemas().GetSchemas();
    bset<ECSchemaId> retained;
    bmap<ECSchemaId, Utf8String> droppable;

    Statement isLive;
    if ((rc = isLive.Prepare(ecdb, "SELECT 1 FROM [temp].[ecdbopt_live_schema] WHERE Id=?")) != BE_SQLITE_OK)
        {
        error = ecdb.GetLastError();
        return rc;
        }

    for (ECSchemaCP schema : schemas)
        {
        if (schema == nullptr)
            continue;

        isLive.Reset();
        isLive.ClearBindings();
        isLive.BindId(1, schema->GetId());
        rc = isLive.Step();
        if (rc != BE_SQLITE_ROW && rc != BE_SQLITE_DONE)
            {
            error = ecdb.GetLastError();
            return rc;
            }
        if (IsProtectedSchema(*schema) || rc == BE_SQLITE_ROW)
            retained.insert(schema->GetId());
        else
            droppable[schema->GetId()] = schema->GetName();
        }

    bool retainedChanged;
    do
        {
        retainedChanged = false;
        Statement refs;
        if (const auto rc = refs.Prepare(ecdb,
            "SELECT ReferencedSchemaId FROM ec_SchemaReference WHERE SchemaId=?"); rc != BE_SQLITE_OK)
            return rc;

        bvector<ECSchemaId> retainedSnapshot(retained.begin(), retained.end());
        for (ECSchemaId schemaId : retainedSnapshot)
            {
            refs.Reset();
            refs.ClearBindings();
            refs.BindId(1, schemaId);
            DbResult rc;
            while ((rc = refs.Step()) == BE_SQLITE_ROW)
                {
                ECSchemaId referencedId = refs.GetValueId<ECSchemaId>(0);
                if (retained.insert(referencedId).second)
                    retainedChanged = true;
                }
            if (rc != BE_SQLITE_DONE)
                return rc;
            }
        }
    while (retainedChanged);

    bvector<Utf8String> names;
    for (auto const& [schemaId, schemaName] : droppable)
        {
        if (retained.find(schemaId) == retained.end())
            names.push_back(schemaName);
        }

    mappedTables.Finalize();
    isLive.Finalize();
    candidates = names.size();
    if (isDryRun || names.empty())
        return BE_SQLITE_OK;

    DropSchemaResult result = ecdb.Schemas().DropSchemas(std::move(names), token);
    if (result.IsError())
        {
        error = result.GetStatusAsString();
        return BE_SQLITE_ERROR;
        }

    changed = candidates;
    return BE_SQLITE_OK;
    }

DbResult DropEmptyDynamicClasses(ECDbR ecdb, bool isDryRun, SchemaImportToken const* token, uint64_t& candidates, uint64_t& changed, uint64_t& skipped, Utf8StringR error)
    {
    ECDB_PERF_LOG_SCOPE("Optimizer> Drop empty dynamic classes");
    constexpr Utf8CP candidateTable = "ecdbopt_empty_class";
    TempTableGuard tempGuard(ecdb, candidateTable);
    auto rc = ecdb.TryExecuteSql(
        "DROP TABLE IF EXISTS [temp].[ecdbopt_empty_class];"
        "CREATE TEMP TABLE [ecdbopt_empty_class](Id INTEGER PRIMARY KEY) WITHOUT ROWID");
    if (rc != BE_SQLITE_OK)
        {
        error = ecdb.GetLastError();
        return rc;
        }

    Statement insertCandidate;
    if ((rc = insertCandidate.Prepare(ecdb, "INSERT INTO [temp].[ecdbopt_empty_class](Id) VALUES(?)")) != BE_SQLITE_OK)
        {
        error = ecdb.GetLastError();
        return rc;
        }

    for (ECSchemaCP schema : ecdb.Schemas().GetSchemas())
        {
        if (schema == nullptr || !schema->IsDynamicSchema() || IsProtectedSchema(*schema))
            continue;

        for (ECClassCP ecClass : schema->GetClasses())
            {
            if (ecClass->IsStructClass())
                {
                ++skipped;
                continue;
                }

            ClassMap const* classMap = ecdb.Schemas().Main().GetClassMap(*ecClass);
            if (classMap != nullptr && MapStrategyExtendedInfo::IsForeignKeyMapping(classMap->GetMapStrategy()))
                {
                ++skipped;
                continue;
                }

            insertCandidate.Reset();
            insertCandidate.ClearBindings();
            if ((rc = insertCandidate.BindId(1, ecClass->GetId())) != BE_SQLITE_OK ||
                (rc = insertCandidate.Step()) != BE_SQLITE_DONE)
                {
                error = ecdb.GetLastError();
                return rc;
                }
            }
        }
    insertCandidate.Finalize();

    Statement mappedTables;
    rc = mappedTables.Prepare(ecdb, R"sql(
        SELECT t.Name, c.Name, c.IsVirtual, t.ExclusiveRootClassId
        FROM ec_Table t
        JOIN ec_Column c ON c.TableId=t.Id AND c.ColumnKind=2
        WHERE t.Type IN (0,1,3)
        ORDER BY t.Id)sql");
    if (rc != BE_SQLITE_OK)
        {
        error = ecdb.GetLastError();
        return rc;
        }

    while ((rc = mappedTables.Step()) == BE_SQLITE_ROW)
        {
        Utf8String tableName = mappedTables.GetValueText(0);
        Utf8String classIdColumn = mappedTables.GetValueText(1);
        NativeSqlBuilder sql("DELETE FROM [temp].[ecdbopt_empty_class] WHERE Id IN (SELECT ");
        if (mappedTables.GetValueBoolean(2))
            {
            if (mappedTables.IsColumnNull(3))
                continue;
            sql.Append(mappedTables.GetValueId<ECClassId>(3))
                .Append(" FROM [main].").AppendEscaped(tableName).Append(" LIMIT 1)");
            }
        else
            {
            sql.AppendEscaped(classIdColumn).Append(" FROM [main].").AppendEscaped(tableName).Append(")");
            }

        if ((rc = ecdb.ExecuteSql(sql.GetSql().c_str())) != BE_SQLITE_OK)
            {
            error = ecdb.GetLastError();
            return rc;
            }
        }
    if (rc != BE_SQLITE_DONE)
        {
        error = ecdb.GetLastError();
        return rc;
        }
    mappedTables.Finalize();

    if ((rc = ecdb.ExecuteSql(R"sql(
        DELETE FROM [temp].[ecdbopt_empty_class]
        WHERE Id IN (SELECT ClassId FROM ec_CustomAttribute)
           OR Id IN (SELECT StructClassId FROM ec_Property WHERE StructClassId IS NOT NULL)
           OR Id IN (SELECT NavigationRelationshipClassId FROM ec_Property WHERE NavigationRelationshipClassId IS NOT NULL)
           OR Id IN (SELECT AbstractConstraintClassId FROM ec_RelationshipConstraint WHERE AbstractConstraintClassId IS NOT NULL)
           OR Id IN (
                SELECT rcc.ClassId
                FROM ec_RelationshipConstraintClass rcc))sql")) != BE_SQLITE_OK)
        {
        error = ecdb.GetLastError();
        return rc;
        }

    Statement ordered;
    rc = ordered.Prepare(ecdb, R"sql(
        WITH RECURSIVE candidate_depth(Id, Depth) AS (
            SELECT Id, 0 FROM [temp].[ecdbopt_empty_class]
            UNION
            SELECT b.BaseClassId, d.Depth+1
            FROM candidate_depth d
            JOIN ec_ClassHasBaseClasses b ON b.ClassId=d.Id
            JOIN [temp].[ecdbopt_empty_class] base ON base.Id=b.BaseClassId
            WHERE d.Depth < (SELECT COUNT(*) FROM [temp].[ecdbopt_empty_class])
        )
        SELECT candidate.Id
        FROM [temp].[ecdbopt_empty_class] candidate
        JOIN candidate_depth depth ON depth.Id=candidate.Id
        WHERE NOT EXISTS (
            SELECT 1
            FROM ec_ClassHasBaseClasses b
            WHERE b.BaseClassId=candidate.Id
              AND NOT EXISTS (SELECT 1 FROM [temp].[ecdbopt_empty_class] derived WHERE derived.Id=b.ClassId))
        GROUP BY candidate.Id
        ORDER BY MAX(depth.Depth) ASC, candidate.Id DESC)sql");
    if (rc != BE_SQLITE_OK)
        {
        error = ecdb.GetLastError();
        return rc;
        }

    bvector<ECClassId> classIds;
    while ((rc = ordered.Step()) == BE_SQLITE_ROW)
        classIds.push_back(ordered.GetValueId<ECClassId>(0));
    if (rc != BE_SQLITE_DONE)
        {
        error = ecdb.GetLastError();
        return rc;
        }
    ordered.Finalize();

    candidates = classIds.size();
    if (isDryRun || classIds.empty())
        return BE_SQLITE_OK;

    if (SUCCESS != ecdb.Schemas().Main().DeleteClassesForOptimizer(classIds, changed, token))
        {
        error = "Failed to delete an empty dynamic ECClass.";
        return BE_SQLITE_ERROR;
        }
    return BE_SQLITE_OK;
    }

bool CanDeletePropertyWithoutRemapping(MainSchemaManager const& manager, ECPropertyCR property)
    {
    ClassMap const* classMap = manager.GetClassMap(property.GetClass());
    if (classMap == nullptr)
        return false;

    // NotMapped classes have no physical storage to reason about; treat as unsupported (skip).
    if (classMap->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        return false;

    auto const& partitions = classMap->GetStorageDescription().GetHorizontalPartitions();
    if (partitions.empty())
        return false;

    for (Partition const& partition : partitions)
        {
        ECClassCP rootClass = manager.GetClass(partition.GetRootClassId());
        if (rootClass == nullptr)
            return false;

        ClassMap const* rootClassMap = manager.GetClassMap(*rootClass);
        if (rootClassMap == nullptr)
            return false;

        PropertyMap const* propertyMap = rootClassMap->GetPropertyMaps().Find(property.GetName().c_str());
        if (propertyMap == nullptr)
            return false;

        GetColumnsPropertyMapVisitor columnVisitor(PropertyMap::Type::Data);
        if (SUCCESS != propertyMap->AcceptVisitor(columnVisitor))
            return false;

        if (columnVisitor.GetColumns().empty())
            return false;

        for (DbColumn const* column : columnVisitor.GetColumns())
            {
            if (!column->IsShared() &&
                column->GetPersistenceType() != PersistenceType::Virtual &&
                column->GetTable().GetType() != DbTable::Type::Virtual)
                return false;
            }
        }

    return true;
    }

//! Optimizer supports empty-value deletion only for scalar primitive and Point2d/Point3d properties.
//! Struct, array, navigation and overridden properties are unsupported and must be skipped (never
//! folded into an aggregate usage batch, which could be unpreparable and poison other candidates).
bool IsSupportedDynamicPropertyForDeletion(ECPropertyCR property)
    {
    if (property.GetIsNavigation() || property.GetBaseProperty() != nullptr)
        return false;
    return property.GetIsPrimitive(); // PrimitiveECProperty covers scalars and Point2d/Point3d
    }

//! Builds an ECSQL emptiness probe for a supported property. Point2d/Point3d are non-null if ANY
//! component holds a value. Returns false if the property type is unsupported for a scalar probe.
bool BuildPropertyEmptinessEcsql(ECClassCR ecClass, ECPropertyCR property, Utf8StringR ecsql)
    {
    PrimitiveECPropertyCP primitive = property.GetAsPrimitiveProperty();
    if (primitive == nullptr)
        return false;

    Utf8String name = property.GetName();
    Utf8String predicate;
    if (primitive->GetType() == PRIMITIVETYPE_Point2d)
        predicate.Sprintf("[%s].X IS NOT NULL OR [%s].Y IS NOT NULL", name.c_str(), name.c_str());
    else if (primitive->GetType() == PRIMITIVETYPE_Point3d)
        predicate.Sprintf("[%s].X IS NOT NULL OR [%s].Y IS NOT NULL OR [%s].Z IS NOT NULL", name.c_str(), name.c_str(), name.c_str());
    else
        predicate.Sprintf("[%s] IS NOT NULL", name.c_str());

    ecsql.Sprintf("SELECT 1 FROM %s WHERE %s LIMIT 1", ecClass.GetECSqlName().c_str(), predicate.c_str());
    return true;
    }

DbResult DropEmptyDynamicProperties(ECDbR ecdb, bool isDryRun, SchemaImportToken const* token, uint64_t& candidates, uint64_t& changed, uint64_t& skipped, Utf8StringR error)
    {
    ECDB_PERF_LOG_SCOPE("Optimizer> Drop empty dynamic properties");
    bvector<ECPropertyCP> emptyProperties;
    DbResult rc = BE_SQLITE_OK;

    for (ECSchemaCP schema : ecdb.Schemas().GetSchemas())
        {
        if (schema == nullptr || !schema->IsDynamicSchema() || IsProtectedSchema(*schema))
            continue;

        for (ECClassCP ecClass : schema->GetClasses())
            {
            if (ecClass->IsMixin())
                {
                for (ECPropertyCP property : ecClass->GetProperties(false))
                    {
                    (void) property;
                    ++skipped;
                    }
                continue;
                }

            bvector<ECPropertyCP> localProperties;
            for (ECPropertyCP property : ecClass->GetProperties(false))
                {
                if (!IsSupportedDynamicPropertyForDeletion(*property) ||
                    !CanDeletePropertyWithoutRemapping(ecdb.Schemas().Main(), *property))
                    {
                    // Unsupported (struct/array/navigation/override/NotMapped/empty-partition/non-shared)
                    // properties are reported as skipped and never affect other candidates.
                    ++skipped;
                    continue;
                    }
                localProperties.push_back(property);
                }

            for (ECPropertyCP property : localProperties)
                {
                Utf8String ecsql;
                if (!BuildPropertyEmptinessEcsql(*ecClass, *property, ecsql))
                    {
                    ++skipped;
                    continue;
                    }

                ECSqlStatement stmt;
                if (ECSqlStatus::Success != stmt.Prepare(ecdb, ecsql.c_str()))
                    {
                    // One unpreparable probe must not poison the batch: skip only this property.
                    ++skipped;
                    continue;
                    }

                rc = stmt.Step();
                if (rc == BE_SQLITE_DONE)
                    emptyProperties.push_back(property); // no row carries a value -> deletable
                else if (rc != BE_SQLITE_ROW)
                    {
                    error = ecdb.GetLastError();
                    return rc;
                    }
                }
            }
        }

    candidates = emptyProperties.size();
    if (isDryRun || emptyProperties.empty())
        return BE_SQLITE_OK;

    if (SUCCESS != ecdb.Schemas().Main().DeletePropertiesForOptimizer(emptyProperties, changed, token))
        {
        error = "Failed to delete an empty dynamic ECProperty.";
        return BE_SQLITE_ERROR;
        }
    return BE_SQLITE_OK;
    }

bool IsSystemTableName(Utf8StringCR name)
    {
    return name.StartsWithIAscii("ec_") || name.StartsWithIAscii("be_");
    }

bool IsProtectedOrSystemTableName(Utf8StringCR name)
    {
    return IsSystemTableName(name) || name.StartsWithIAscii("bis_");
    }

DbResult DropUnmappedTables(ECDbR ecdb, bool isDryRun, uint64_t& candidates, uint64_t& changed, uint64_t& skipped, Utf8StringR error)
    {
    ECDB_PERF_LOG_SCOPE("Optimizer> Drop unmapped tables");
    struct Table final
        {
        DbTableId m_id;
        Utf8String m_name;
        };

    bvector<Table> tables;
    CachedStatementPtr dependencyStmt = ecdb.GetCachedStatement(
        "SELECT 1 FROM sqlite_master "
        "WHERE sql IS NOT NULL AND name<>? AND tbl_name<>? AND instr(lower(sql),lower(?))>0 LIMIT 1");
    if (dependencyStmt == nullptr)
        {
        error = ecdb.GetLastError();
        return BE_SQLITE_ERROR;
        }
    Statement stmt;
    auto rc = stmt.Prepare(ecdb, R"sql(
        WITH RECURSIVE table_depth(Id, Depth) AS (
            SELECT Id, 0 FROM ec_Table WHERE ParentTableId IS NULL
            UNION ALL
            SELECT t.Id, d.Depth+1 FROM ec_Table t JOIN table_depth d ON d.Id=t.ParentTableId
        )
        SELECT t.Id, t.Name
        FROM ec_Table t
        JOIN table_depth d ON d.Id=t.Id
        WHERE t.Type IN (0,1,3)
          AND t.Name<>'ec_NullTable'
          AND NOT EXISTS (
              SELECT 1
              FROM ec_PropertyMap pm
              JOIN ec_Column c ON c.Id=pm.ColumnId
              WHERE c.TableId=t.Id)
          AND NOT EXISTS (
              SELECT 1
              FROM ec_ClassMap cm
              JOIN ec_cache_ClassHasTables cht ON cht.ClassId=cm.ClassId
              WHERE cht.TableId=t.Id)
        ORDER BY d.Depth DESC, t.Id DESC)sql");
    if (rc != BE_SQLITE_OK)
        {
        error = ecdb.GetLastError();
        return rc;
        }

    while ((rc = stmt.Step()) == BE_SQLITE_ROW)
        {
        Utf8String name = stmt.GetValueText(1);
        if (IsProtectedOrSystemTableName(name))
            {
            ++skipped;
            continue;
            }
        dependencyStmt->Reset();
        dependencyStmt->ClearBindings();
        if ((rc = dependencyStmt->BindText(1, name, Statement::MakeCopy::No)) != BE_SQLITE_OK ||
            (rc = dependencyStmt->BindText(2, name, Statement::MakeCopy::No)) != BE_SQLITE_OK ||
            (rc = dependencyStmt->BindText(3, name, Statement::MakeCopy::No)) != BE_SQLITE_OK)
            {
            error = ecdb.GetLastError();
            return rc;
            }
        if ((rc = dependencyStmt->Step()) == BE_SQLITE_ROW)
            {
            ++skipped;
            continue;
            }
        if (rc != BE_SQLITE_DONE)
            {
            error = ecdb.GetLastError();
            return rc;
            }
        tables.push_back({stmt.GetValueId<DbTableId>(0), std::move(name)});
        }
    if (rc != BE_SQLITE_DONE)
        {
        error = ecdb.GetLastError();
        return rc;
        }

    candidates = tables.size();
    if (isDryRun)
        return BE_SQLITE_OK;

    Statement deleteMetadata;
    if ((rc = deleteMetadata.Prepare(ecdb, "DELETE FROM ec_Table WHERE Id=?")) != BE_SQLITE_OK)
        {
        error = ecdb.GetLastError();
        return rc;
        }

    // Dropping tables and their metadata invalidates cached ClassMap/DbTable pointers; guarantee a
    // coherent cache rebuild on every exit path rather than surgically editing the DbTable cache.
    CacheInvalidationGuard cacheGuard(ecdb);
    for (Table const& table : tables)
        {
        if ((rc = ecdb.GetImpl().ExecuteDDL(SqlPrintfString("DROP TABLE [%s]", table.m_name.c_str()).GetUtf8CP())) != BE_SQLITE_OK)
            {
            error.Sprintf("Failed to drop unmapped table '%s': %s", table.m_name.c_str(), ecdb.GetLastError().c_str());
            return rc;
            }

        deleteMetadata.Reset();
        deleteMetadata.ClearBindings();
        if ((rc = deleteMetadata.BindId(1, table.m_id)) != BE_SQLITE_OK ||
            (rc = deleteMetadata.Step()) != BE_SQLITE_DONE)
            {
            error.Sprintf("Dropped table '%s' but failed to remove its ECDb metadata: %s", table.m_name.c_str(), ecdb.GetLastError().c_str());
            return rc;
            }
        ++changed;
        }

    return BE_SQLITE_OK;
    }

DbResult DropUnmappedSharedColumns(ECDbR ecdb, bool isDryRun, uint64_t& candidates, uint64_t& changed, uint64_t& skipped, Utf8StringR error)
    {
    ECDB_PERF_LOG_SCOPE("Optimizer> Drop unmapped shared columns");
    struct Column final
        {
        DbColumnId m_id;
        Utf8String m_name;
        };

    bmap<Utf8String, bvector<Column>, CompareIUtf8Ascii> columnsByTable;
    bset<DbTableId> protectedMappingTables;
    Statement protectedTableStmt;
    auto rc = protectedTableStmt.Prepare(ecdb, R"sql(
        SELECT DISTINCT c.TableId, mappedSchema.Name, rootSchema.Name
        FROM ec_PropertyMap pm
        JOIN ec_Column c ON c.Id=pm.ColumnId
        JOIN ec_Class mappedClass ON mappedClass.Id=pm.ClassId
        JOIN ec_Schema mappedSchema ON mappedSchema.Id=mappedClass.SchemaId
        JOIN ec_PropertyPath pp ON pp.Id=pm.PropertyPathId
        JOIN ec_Property rootProperty ON rootProperty.Id=pp.RootPropertyId
        JOIN ec_Class rootClass ON rootClass.Id=rootProperty.ClassId
        JOIN ec_Schema rootSchema ON rootSchema.Id=rootClass.SchemaId)sql");
    if (rc != BE_SQLITE_OK)
        {
        error = ecdb.GetLastError();
        return rc;
        }
    while ((rc = protectedTableStmt.Step()) == BE_SQLITE_ROW)
        {
        ECSchemaCP mappedSchema = ecdb.Schemas().GetSchema(protectedTableStmt.GetValueText(1));
        ECSchemaCP rootSchema = ecdb.Schemas().GetSchema(protectedTableStmt.GetValueText(2));
        if ((mappedSchema != nullptr && IsProtectedSchema(*mappedSchema)) ||
            (rootSchema != nullptr && IsProtectedSchema(*rootSchema)))
            protectedMappingTables.insert(protectedTableStmt.GetValueId<DbTableId>(0));
        }
    if (rc != BE_SQLITE_DONE)
        {
        error = ecdb.GetLastError();
        return rc;
        }
    protectedTableStmt.Finalize();

    Statement stmt;
    rc = stmt.Prepare(ecdb, R"sql(
        SELECT c.Id, t.Id, t.Name, c.Name
        FROM ec_Column c
        JOIN ec_Table t ON t.Id=c.TableId
        WHERE c.ColumnKind=4
          AND c.IsVirtual=0
          AND t.Type IN (0,1,3)
          AND NOT EXISTS (SELECT 1 FROM ec_PropertyMap pm WHERE pm.ColumnId=c.Id)
          AND NOT EXISTS (
              SELECT 1
              FROM ec_IndexColumn candidateIndexColumn
              JOIN ec_IndexColumn retainedIndexColumn ON retainedIndexColumn.IndexId=candidateIndexColumn.IndexId
              JOIN ec_PropertyMap retainedMap ON retainedMap.ColumnId=retainedIndexColumn.ColumnId
              WHERE candidateIndexColumn.ColumnId=c.Id)
        ORDER BY t.Id, c.Ordinal DESC)sql");
    if (rc != BE_SQLITE_OK)
        {
        error = ecdb.GetLastError();
        return rc;
        }

    while ((rc = stmt.Step()) == BE_SQLITE_ROW)
        {
        DbTableId tableId = stmt.GetValueId<DbTableId>(1);
        Utf8String tableName = stmt.GetValueText(2);
        if (IsSystemTableName(tableName) || protectedMappingTables.count(tableId) != 0)
            {
            ++skipped;
            continue;
            }
        columnsByTable[tableName].push_back({stmt.GetValueId<DbColumnId>(0), stmt.GetValueText(3)});
        }
    if (rc != BE_SQLITE_DONE)
        {
        error = ecdb.GetLastError();
        return rc;
        }

    for (auto const& entry : columnsByTable)
        candidates += entry.second.size();
    if (isDryRun)
        return BE_SQLITE_OK;
    if (columnsByTable.empty())
        return BE_SQLITE_OK;

    if (SUCCESS != ViewGenerator::DropECClassViews(ecdb))
        {
        error = "Failed to drop ECClass views before removing unmapped shared columns.";
        ecdb.ClearECDbCache();
        return BE_SQLITE_ERROR;
        }

    // From here on physical/metadata state is being mutated: any early return must coherently
    // invalidate the ECDb schema/map cache so cached ClassMaps do not retain stale pointers.
    CacheInvalidationGuard cacheGuard(ecdb);

    Statement indexQuery;
    if ((rc = indexQuery.Prepare(ecdb, R"sql(
        SELECT DISTINCT i.Id, i.Name
        FROM ec_Index i
        JOIN ec_IndexColumn ic ON ic.IndexId=i.Id
        JOIN ec_Column c ON c.Id=ic.ColumnId
        WHERE c.TableId=(SELECT Id FROM ec_Table WHERE Name=?)
          AND InVirtualSet(?,c.Id))sql")) != BE_SQLITE_OK)
        {
        error = ecdb.GetLastError();
        return rc;
        }

    Statement deleteIndex;
    Statement deleteColumn;
    if ((rc = deleteIndex.Prepare(ecdb, "DELETE FROM ec_Index WHERE Id=?")) != BE_SQLITE_OK ||
        (rc = deleteColumn.Prepare(ecdb, "DELETE FROM ec_Column WHERE Id=?")) != BE_SQLITE_OK)
        {
        error = ecdb.GetLastError();
        return rc;
        }

    for (auto const& [tableName, columns] : columnsByTable)
        {
        DbTable const* table = ecdb.Schemas().Main().GetDbSchema().FindTable(tableName);
        if (table == nullptr)
            {
            error.Sprintf("Failed to load mapping table '%s'.", tableName.c_str());
            return BE_SQLITE_ERROR;
            }

        BeIdSet columnIds;
        for (Column const& column : columns)
            columnIds.insert(column.m_id);
        IdSet<BeInt64Id> columnIdSet(std::move(columnIds));

        bvector<std::pair<BeInt64Id, Utf8String>> indexes;
        indexQuery.Reset();
        indexQuery.ClearBindings();
        if ((rc = indexQuery.BindText(1, tableName, Statement::MakeCopy::No)) != BE_SQLITE_OK ||
            (rc = indexQuery.BindVirtualSet(2, columnIdSet)) != BE_SQLITE_OK)
            {
            error = ecdb.GetLastError();
            return rc;
            }
        while ((rc = indexQuery.Step()) == BE_SQLITE_ROW)
            indexes.emplace_back(indexQuery.GetValueId<BeInt64Id>(0), indexQuery.GetValueText(1));
        if (rc != BE_SQLITE_DONE)
            {
            error = ecdb.GetLastError();
            return rc;
            }

        for (auto const& [indexId, indexName] : indexes)
            {
            NativeSqlBuilder ddl("DROP INDEX ");
            ddl.AppendEscaped(indexName);
            if ((rc = ecdb.GetImpl().ExecuteDDL(ddl.GetSql().c_str())) != BE_SQLITE_OK)
                {
                error.Sprintf("Failed to drop index '%s' before removing an unmapped shared column: %s", indexName.c_str(), ecdb.GetLastError().c_str());
                return rc;
                }
            deleteIndex.Reset();
            deleteIndex.ClearBindings();
            if ((rc = deleteIndex.BindId(1, indexId)) != BE_SQLITE_OK ||
                (rc = deleteIndex.Step()) != BE_SQLITE_DONE)
                {
                error = ecdb.GetLastError();
                return rc;
                }
            }

        std::vector<Utf8String> names;
        names.reserve(columns.size());
        for (Column const& column : columns)
            names.push_back(column.m_name);
        if (DbSchemaPersistenceManager::DropColumns(ecdb, *table, names) != SUCCESS)
            {
            error = ecdb.GetLastError();
            return BE_SQLITE_ERROR;
            }

        for (Column const& column : columns)
            {
            deleteColumn.Reset();
            deleteColumn.ClearBindings();
            if ((rc = deleteColumn.BindId(1, column.m_id)) != BE_SQLITE_OK ||
                (rc = deleteColumn.Step()) != BE_SQLITE_DONE)
                {
                error = ecdb.GetLastError();
                return rc;
                }
            ++changed;
            }
        }

    // Coherently rebuild caches after the mutation, then recreate the views from final definitions.
    ecdb.ClearECDbCache();
    cacheGuard.Dismiss();
    if (SUCCESS != ecdb.Schemas().Main().CreateClassViews())
        {
        error = "Failed to recreate ECClass views after removing unmapped shared columns.";
        return BE_SQLITE_ERROR;
        }
    return BE_SQLITE_OK;
    }

struct SharedColumnSlot final
    {
    DbColumnId m_id;
    DbTableId m_tableId;
    Utf8String m_tableName;
    Utf8String m_name;
    Utf8String m_idColumn;
    Utf8String m_classIdColumn;
    DbColumn const* m_dbColumn = nullptr;
    int m_ordinal = 0;
    bool m_pinned = false;
    };

struct SharedPropertyComponent final
    {
    bvector<uint64_t> m_propertyMapIds;
    DbColumnId m_sourceColumnId;
    Utf8String m_accessString;
    };

struct SharedPropertyGroup final
    {
    ECPropertyId m_rootPropertyId;
    bset<ECClassId> m_rowClassIds;
    bvector<SharedPropertyComponent> m_components;
    bvector<SharedColumnSlot const*> m_sources;
    bvector<SharedColumnSlot const*> m_destinations;
    bool m_hasValues = false;
    bool m_isUnsafe = false;
    };

struct SharedTableInfo final
    {
    DbTableId m_id;
    DbTableId m_parentId;
    Utf8String m_name;
    Utf8String m_idColumn;
    Utf8String m_classIdColumn;
    int m_type = 0;
    };

bool AreCompatible(SharedColumnSlot const& lhs, SharedColumnSlot const& rhs)
    {
    return lhs.m_dbColumn != nullptr && rhs.m_dbColumn != nullptr &&
        ClassMapColumnFactory::AreSharedColumnsCompatible(*lhs.m_dbColumn, *rhs.m_dbColumn);
    }

Utf8String ClassRowPredicate(SharedTableInfo const& table, bset<ECClassId> const& classIds, Utf8CP alias = nullptr)
    {
    if (table.m_classIdColumn.empty())
        return "1";

    NativeSqlBuilder sql;
    if (!Utf8String::IsNullOrEmpty(alias))
        sql.AppendEscaped(alias).Append(".");
    sql.AppendEscaped(table.m_classIdColumn).Append(" IN (");
    bool first = true;
    for (ECClassId classId : classIds)
        {
        if (!first)
            sql.Append(",");
        sql.Append(classId);
        first = false;
        }
    sql.Append(")");
    return sql.GetSql();
    }

Utf8String ClassRowPredicate(SharedTableInfo const& table, ECClassId classId)
    {
    bset<ECClassId> classIds = {classId};
    return ClassRowPredicate(table, classIds);
    }

DbResult GroupHasValues(ECDbR ecdb, SharedPropertyGroup const& group, SharedTableInfo const& table, bool& hasValues)
    {
    NativeSqlBuilder sql("SELECT 1 FROM [main].");
    sql.AppendEscaped(table.m_name).Append(" WHERE ").Append(ClassRowPredicate(table, group.m_rowClassIds)).Append(" AND (");
    bool first = true;
    for (SharedColumnSlot const* source : group.m_sources)
        {
        if (!first)
            sql.Append(" OR ");
        sql.AppendEscaped(source->m_name).Append(" IS NOT NULL");
        first = false;
        }
    sql.Append(") LIMIT 1");

    Statement stmt;
    auto rc = stmt.Prepare(ecdb, sql.GetSql().c_str());
    if (rc != BE_SQLITE_OK)
        return rc;
    rc = stmt.Step();
    hasValues = rc == BE_SQLITE_ROW;
    return rc == BE_SQLITE_ROW || rc == BE_SQLITE_DONE ? BE_SQLITE_OK : rc;
    }

DbResult DropClassViews(ECDbR ecdb, bset<ECClassId> const& classIds, bvector<ECClassId>& droppedClassIds, Utf8StringR error)
    {
    Statement nameStmt;
    auto rc = nameStmt.Prepare(ecdb,
        "SELECT s.Alias,c.Name,EXISTS(SELECT 1 FROM sqlite_master m "
        "WHERE m.type='view' AND m.name=s.Alias||'.'||c.Name) "
        "FROM ec_Class c JOIN ec_Schema s ON s.Id=c.SchemaId WHERE c.Id=?");
    if (rc != BE_SQLITE_OK)
        return rc;

    bvector<Utf8String> viewNames;
    for (ECClassId classId : classIds)
        {
        nameStmt.Reset();
        nameStmt.ClearBindings();
        if ((rc = nameStmt.BindId(1, classId)) != BE_SQLITE_OK || (rc = nameStmt.Step()) != BE_SQLITE_ROW)
            {
            error = ecdb.GetLastError();
            return rc == BE_SQLITE_DONE ? BE_SQLITE_ERROR : rc;
            }
        if (nameStmt.GetValueBoolean(2))
            {
            viewNames.push_back(Utf8PrintfString("%s.%s", nameStmt.GetValueText(0), nameStmt.GetValueText(1)));
            droppedClassIds.push_back(classId);
            }
        }
    nameStmt.Finalize();
    for (Utf8StringCR viewName : viewNames)
        {
        NativeSqlBuilder sql("DROP VIEW IF EXISTS [main].");
        sql.AppendEscaped(viewName);
        if ((rc = ecdb.TryExecuteSql(sql.GetSql().c_str())) != BE_SQLITE_OK)
            {
            error = ecdb.GetLastError();
            return rc;
            }
        }
    return BE_SQLITE_OK;
    }

//! Removes overflow-table data rows whose stored ECClassId is not valid for that overflow table. A
//! class is valid for an overflow table when it maps a data property into the table, or derives from a
//! class that does (inheritance-aware). The valid set is computed set-based with a bounded recursive
//! CTE (UNION over the single Id column dedupes, bounding recursion to the number of classes), and each
//! overflow table is cleaned with a single COUNT/DELETE, with no per-row C++ iteration. Mapping definitions
//! are never modified; only orphan data rows are removed. Honors dry-run (counts only, no writes).
DbResult PurgeOverflowOrphanRows(
    ECDbR ecdb,
    bvector<std::pair<uint64_t, DbColumnId>> const& projectedPropertyMapUpdates,
    bool isDryRun,
    uint64_t& candidates,
    uint64_t& changed,
    bvector<Utf8String>& messages,
    Utf8StringR error)
    {
    ECDB_PERF_LOG_SCOPE("Optimizer> Compact shared columns> Purge overflow orphan rows");
    candidates = 0;
    changed = 0;

    constexpr Utf8CP projectedMapTable = "ecdbopt_projected_property_map";
    TempTableGuard projectedMapGuard(ecdb, projectedMapTable);
    if (!projectedPropertyMapUpdates.empty())
        {
        auto rc = ecdb.TryExecuteSql(
            "DROP TABLE IF EXISTS [temp].[ecdbopt_projected_property_map];"
            "CREATE TEMP TABLE [ecdbopt_projected_property_map]("
            "PropertyMapId INTEGER PRIMARY KEY,ColumnId INTEGER NOT NULL) WITHOUT ROWID");
        if (rc != BE_SQLITE_OK)
            {
            error = ecdb.GetLastError();
            return rc;
            }

        Statement insertProjection;
        if ((rc = insertProjection.Prepare(ecdb,
            "INSERT INTO [temp].[ecdbopt_projected_property_map](PropertyMapId,ColumnId) VALUES(?,?)")) != BE_SQLITE_OK)
            {
            error = ecdb.GetLastError();
            return rc;
            }
        for (auto const& update : projectedPropertyMapUpdates)
            {
            insertProjection.Reset();
            insertProjection.ClearBindings();
            if ((rc = insertProjection.BindUInt64(1, update.first)) != BE_SQLITE_OK ||
                (rc = insertProjection.BindId(2, update.second)) != BE_SQLITE_OK ||
                (rc = insertProjection.Step()) != BE_SQLITE_DONE)
                {
                error = ecdb.GetLastError();
                return rc;
                }
            }
        }

    struct OverflowTable final
        {
        DbTableId m_id;
        Utf8String m_name;
        Utf8String m_classIdColumn;
        };

    bvector<OverflowTable> overflowTables;
    Statement overflowStmt;
    auto rc = overflowStmt.Prepare(ecdb, R"sql(
        SELECT t.Id, t.Name, cid.Name
        FROM ec_Table t
        JOIN ec_Column cid ON cid.TableId=t.Id AND cid.ColumnKind=2 AND cid.IsVirtual=0
        WHERE t.Type=3)sql");
    if (rc != BE_SQLITE_OK)
        {
        error = ecdb.GetLastError();
        return rc;
        }
    while ((rc = overflowStmt.Step()) == BE_SQLITE_ROW)
        {
        if (overflowStmt.IsColumnNull(2))
            continue; // an overflow table without a persisted ECClassId column cannot be reasoned about
        overflowTables.push_back({overflowStmt.GetValueId<DbTableId>(0), overflowStmt.GetValueText(1), overflowStmt.GetValueText(2)});
        }
    if (rc != BE_SQLITE_DONE)
        {
        error = ecdb.GetLastError();
        return rc;
        }
    overflowStmt.Finalize();

    for (OverflowTable const& table : overflowTables)
        {
        auto appendCte = [&table, &projectedPropertyMapUpdates](NativeSqlBuilder& sql)
            {
            sql.Append("WITH RECURSIVE valid(Id) AS ("
                       "SELECT DISTINCT pm.ClassId FROM [main].[ec_PropertyMap] pm "
                       );
            if (!projectedPropertyMapUpdates.empty())
                sql.Append(
                    "LEFT JOIN [temp].[ecdbopt_projected_property_map] projected ON projected.PropertyMapId=pm.Id "
                    "JOIN [main].[ec_Column] c ON c.Id=coalesce(projected.ColumnId,pm.ColumnId) ");
            else
                sql.Append("JOIN [main].[ec_Column] c ON c.Id=pm.ColumnId ");
            sql.Append("WHERE c.TableId=")
               .Append(Utf8PrintfString("%" PRIu64, table.m_id.GetValueUnchecked()).c_str())
               .Append(" AND c.ColumnKind IN (0,4) "
                       "UNION "
                       "SELECT b.ClassId FROM valid v "
                       "JOIN [main].[ec_ClassHasBaseClasses] b ON b.BaseClassId=v.Id) ");
            };
        auto appendPredicate = [&table](NativeSqlBuilder& sql)
            {
            sql.AppendEscaped(table.m_name).Append(" WHERE ")
               .AppendEscaped(table.m_classIdColumn).Append(" IS NULL OR ")
               .AppendEscaped(table.m_classIdColumn).Append(" NOT IN (SELECT Id FROM valid)");
            };

        // Candidates are the orphan rows discovered before any deletion.
        NativeSqlBuilder countSql;
        appendCte(countSql);
        countSql.Append("SELECT COUNT(*) FROM [main].");
        appendPredicate(countSql);
        uint64_t orphanCount = 0;
        if ((rc = CountRows(ecdb, countSql.GetSql(), orphanCount)) != BE_SQLITE_OK)
            {
            error = ecdb.GetLastError();
            return rc;
            }
        candidates += orphanCount;
        if (orphanCount != 0)
            messages.push_back(Utf8PrintfString("Overflow table '%s' has %" PRIu64 " row(s) whose ECClassId is not valid for it.", table.m_name.c_str(), orphanCount));

        if (isDryRun || orphanCount == 0)
            continue;

        NativeSqlBuilder deleteSql;
        appendCte(deleteSql);
        deleteSql.Append("DELETE FROM [main].");
        appendPredicate(deleteSql);
        if ((rc = ecdb.ExecuteSql(deleteSql.GetSql().c_str())) != BE_SQLITE_OK && rc != BE_SQLITE_DONE)
            {
            error = ecdb.GetLastError();
            return rc;
            }
        changed += static_cast<uint64_t>(ecdb.GetModifiedRowCount());
        }

    return BE_SQLITE_OK;
    }

DbResult CompactSharedColumns(ECDbR ecdb, bool isDryRun, uint64_t& candidates, uint64_t& changed, uint64_t& skipped, bvector<Utf8String>& messages, Utf8StringR error)
    {
    ECDB_PERF_LOG_SCOPE("Optimizer> Compact shared columns");
    candidates = changed = skipped = 0;

    ECDB_PERF_LOG_SCOPE_BEGIN(compactDiscovery, "Optimizer> Compact shared columns> Discovery");
    std::map<DbTableId, SharedTableInfo> tables;
    Statement tableStmt;
    auto rc = tableStmt.Prepare(ecdb, R"sql(
        SELECT t.Id,t.ParentTableId,t.Name,t.Type,
               idc.Name,cid.Name
        FROM ec_Table t
        LEFT JOIN ec_Column idc ON idc.TableId=t.Id AND idc.ColumnKind=1
        LEFT JOIN ec_Column cid ON cid.TableId=t.Id AND cid.ColumnKind=2
        WHERE t.Type IN (0,1,3))sql");
    if (rc != BE_SQLITE_OK)
        return rc;
    while ((rc = tableStmt.Step()) == BE_SQLITE_ROW)
        {
        SharedTableInfo info;
        info.m_id = tableStmt.GetValueId<DbTableId>(0);
        if (!tableStmt.IsColumnNull(1))
            info.m_parentId = tableStmt.GetValueId<DbTableId>(1);
        info.m_name = tableStmt.GetValueText(2);
        info.m_type = tableStmt.GetValueInt(3);
        if (!tableStmt.IsColumnNull(4))
            info.m_idColumn = tableStmt.GetValueText(4);
        if (!tableStmt.IsColumnNull(5))
            info.m_classIdColumn = tableStmt.GetValueText(5);
        tables[info.m_id] = info;
        }
    if (rc != BE_SQLITE_DONE)
        return rc;
    tableStmt.Finalize();

    bset<DbColumnId> indexedColumns;
    Statement indexStmt;
    if ((rc = indexStmt.Prepare(ecdb, "SELECT DISTINCT ColumnId FROM ec_IndexColumn")) != BE_SQLITE_OK)
        return rc;
    while ((rc = indexStmt.Step()) == BE_SQLITE_ROW)
        indexedColumns.insert(indexStmt.GetValueId<DbColumnId>(0));
    if (rc != BE_SQLITE_DONE)
        return rc;
    indexStmt.Finalize();

    bset<DbTableId> triggerTables;
    Statement triggerStmt;
    if ((rc = triggerStmt.Prepare(ecdb, "SELECT t.Id FROM sqlite_master m JOIN ec_Table t ON t.Name=m.tbl_name COLLATE NOCASE WHERE m.type='trigger'")) != BE_SQLITE_OK)
        return rc;
    while ((rc = triggerStmt.Step()) == BE_SQLITE_ROW)
        triggerTables.insert(triggerStmt.GetValueId<DbTableId>(0));
    if (rc != BE_SQLITE_DONE)
        return rc;
    triggerStmt.Finalize();

    bvector<Utf8String> externalViewDefinitions;
    Statement viewStmt;
    if ((rc = viewStmt.Prepare(ecdb, "SELECT sql FROM sqlite_master WHERE type='view' AND sql IS NOT NULL AND instr(sql,'ECCLASS VIEW')=0")) != BE_SQLITE_OK)
        return rc;
    while ((rc = viewStmt.Step()) == BE_SQLITE_ROW)
        externalViewDefinitions.push_back(viewStmt.GetValueText(0));
    if (rc != BE_SQLITE_DONE)
        return rc;
    viewStmt.Finalize();

    for (auto const& table : tables)
        {
        if (std::any_of(externalViewDefinitions.begin(), externalViewDefinitions.end(),
            [&table](Utf8StringCR sql) { return sql.ContainsI(table.second.m_name.c_str()); }))
            triggerTables.insert(table.first);
        }

    std::map<DbColumnId, SharedColumnSlot> slotsById;
    std::map<DbTableId, bvector<SharedColumnSlot const*>> slotsByTable;
    Statement slotStmt;
    if ((rc = slotStmt.Prepare(ecdb, R"sql(
        SELECT c.Id,c.TableId,t.Name,c.Name,c.Ordinal
        FROM ec_Column c JOIN ec_Table t ON t.Id=c.TableId
        WHERE c.ColumnKind=4 AND c.IsVirtual=0 AND t.Type IN (0,1,3)
        ORDER BY t.Id,c.Ordinal)sql")) != BE_SQLITE_OK)
        return rc;
    while ((rc = slotStmt.Step()) == BE_SQLITE_ROW)
        {
        SharedColumnSlot slot;
        slot.m_id = slotStmt.GetValueId<DbColumnId>(0);
        slot.m_tableId = slotStmt.GetValueId<DbTableId>(1);
        slot.m_tableName = slotStmt.GetValueText(2);
        slot.m_name = slotStmt.GetValueText(3);
        slot.m_ordinal = slotStmt.GetValueInt(4);
        slot.m_pinned = indexedColumns.count(slot.m_id) != 0 || triggerTables.count(slot.m_tableId) != 0;
        auto tableIt = tables.find(slot.m_tableId);
        if (tableIt == tables.end() || tableIt->second.m_idColumn.empty())
            continue;
        DbTable const* dbTable = ecdb.Schemas().Main().GetDbSchema().FindTable(slot.m_tableId);
        if (dbTable == nullptr)
            continue;
        slot.m_dbColumn = dbTable->FindColumnP(slot.m_name.c_str());
        if (slot.m_dbColumn == nullptr)
            continue;
        slot.m_idColumn = tableIt->second.m_idColumn;
        slot.m_classIdColumn = tableIt->second.m_classIdColumn;
        slotsById[slot.m_id] = slot;
        }
    if (rc != BE_SQLITE_DONE)
        return rc;
    slotStmt.Finalize();
    for (auto const& entry : slotsById)
        slotsByTable[entry.second.m_tableId].push_back(&entry.second);

    std::map<ECClassId, bvector<ECClassId>> derivedClasses;
    Statement inheritanceStmt;
    if ((rc = inheritanceStmt.Prepare(ecdb, "SELECT ClassId,BaseClassId FROM ec_ClassHasBaseClasses")) != BE_SQLITE_OK)
        return rc;
    while ((rc = inheritanceStmt.Step()) == BE_SQLITE_ROW)
        derivedClasses[inheritanceStmt.GetValueId<ECClassId>(1)].push_back(inheritanceStmt.GetValueId<ECClassId>(0));
    if (rc != BE_SQLITE_DONE)
        return rc;
    inheritanceStmt.Finalize();

    std::map<ECClassId, std::map<ECPropertyId, SharedPropertyGroup>> groupsByClass;
    bset<DbColumnId> protectedMappingColumns;
    Statement mapStmt;
    if ((rc = mapStmt.Prepare(ecdb, R"sql(
        SELECT pm.Id,pm.ClassId,pp.RootPropertyId,pp.AccessString,pm.ColumnId,
               cs.Name,rs.Name,rp.ClassId,cm.ShareColumnsMode
        FROM ec_PropertyMap pm
        JOIN ec_PropertyPath pp ON pp.Id=pm.PropertyPathId
        JOIN ec_Property rp ON rp.Id=pp.RootPropertyId
        JOIN ec_Class mc ON mc.Id=pm.ClassId
        JOIN ec_Schema cs ON cs.Id=mc.SchemaId
        JOIN ec_Class rc ON rc.Id=rp.ClassId
        JOIN ec_Schema rs ON rs.Id=rc.SchemaId
        JOIN ec_ClassMap cm ON cm.ClassId=pm.ClassId
        JOIN ec_Column col ON col.Id=pm.ColumnId
        WHERE col.ColumnKind=4 AND col.IsVirtual=0
        ORDER BY pm.ClassId,pp.RootPropertyId,col.TableId,col.Ordinal)sql")) != BE_SQLITE_OK)
        return rc;
    while ((rc = mapStmt.Step()) == BE_SQLITE_ROW)
        {
        ECClassId classId = mapStmt.GetValueId<ECClassId>(1);
        ECPropertyId rootPropertyId = mapStmt.GetValueId<ECPropertyId>(2);
        ECClassId rootClassId = mapStmt.GetValueId<ECClassId>(7);
        DbColumnId columnId = mapStmt.GetValueId<DbColumnId>(4);
        ECSchemaCP classSchema = ecdb.Schemas().GetSchema(mapStmt.GetValueText(5));
        ECSchemaCP rootSchema = ecdb.Schemas().GetSchema(mapStmt.GetValueText(6));
        if (classSchema == nullptr || rootSchema == nullptr || IsProtectedSchema(*classSchema) || IsProtectedSchema(*rootSchema))
            {
            protectedMappingColumns.insert(columnId);
            continue;
            }
        if (mapStmt.IsColumnNull(8) || mapStmt.GetValueInt(8) != 1)
            {
            // The shared column is mapped by a class whose ShareColumnsMode is not Yes. Its storage is
            // not proven eligible for compaction, so pin it: never move, overwrite, or NULL it.
            protectedMappingColumns.insert(columnId);
            continue;
            }

        auto slotIt = slotsById.find(columnId);
        if (slotIt == slotsById.end())
            continue;
        auto& group = groupsByClass[rootClassId][rootPropertyId];
        group.m_rootPropertyId = rootPropertyId;
        Utf8String accessString = mapStmt.GetValueText(3);
        auto componentIt = std::find_if(group.m_components.begin(), group.m_components.end(),
            [&accessString](SharedPropertyComponent const& component) { return component.m_accessString.EqualsI(accessString); });
        if (componentIt == group.m_components.end())
            {
            SharedPropertyComponent component;
            component.m_propertyMapIds.push_back(mapStmt.GetValueUInt64(0));
            component.m_sourceColumnId = slotIt->first;
            component.m_accessString = accessString;
            group.m_components.push_back(component);
            group.m_sources.push_back(&slotIt->second);
            }
        else
            {
            size_t componentIndex = static_cast<size_t>(componentIt - group.m_components.begin());
            if (componentIt->m_sourceColumnId != slotIt->first)
                {
                group.m_isUnsafe = true;
                protectedMappingColumns.insert(componentIt->m_sourceColumnId);
                protectedMappingColumns.insert(slotIt->first);
                }
            componentIt->m_propertyMapIds.push_back(mapStmt.GetValueUInt64(0));
            BeAssert(componentIndex < group.m_sources.size());
            }
        }
    if (rc != BE_SQLITE_DONE)
        return rc;
    mapStmt.Finalize();

    for (DbColumnId columnId : protectedMappingColumns)
        {
        auto slotIt = slotsById.find(columnId);
        if (slotIt != slotsById.end())
            slotIt->second.m_pinned = true;
        }

    for (auto& classEntry : groupsByClass)
        {
        bvector<ECClassId> pending = {classEntry.first};
        bset<ECClassId> visited;
        while (!pending.empty())
            {
            ECClassId classId = pending.back();
            pending.pop_back();
            if (!visited.insert(classId).second)
                continue;
            for (auto& propertyEntry : classEntry.second)
                propertyEntry.second.m_rowClassIds.insert(classId);
            auto derivedIt = derivedClasses.find(classId);
            if (derivedIt != derivedClasses.end())
                pending.insert(pending.end(), derivedIt->second.begin(), derivedIt->second.end());
            }
        }

    bvector<RemapManager::ColumnRemap> remaps;
    bvector<std::pair<uint64_t, DbColumnId>> propertyMapUpdates;
    std::map<DbTableId, bvector<SharedPropertyGroup*>> groupsByFamily;
    std::map<DbTableId, bset<DbTableId>> tablesByFamily;
    bset<std::pair<ECClassId, DbColumnId>> cellsToClear;
    uint64_t nextMoveId = 1;

    for (auto& classEntry : groupsByClass)
        {
        for (auto& groupEntry : classEntry.second)
            {
            SharedPropertyGroup& group = groupEntry.second;
            if (group.m_sources.empty())
                continue;

            DbTableId sourceTableId = group.m_sources.front()->m_tableId;
            if (group.m_isUnsafe || !std::all_of(group.m_sources.begin(), group.m_sources.end(),
                [sourceTableId](SharedColumnSlot const* slot) { return slot->m_tableId == sourceTableId; }))
                {
                group.m_isUnsafe = true;
                ++skipped;
                messages.push_back(Utf8PrintfString("Skipped shared property %" PRIu64 ": its canonical component mappings do not use one source table.", group.m_rootPropertyId.GetValueUnchecked()));
                continue;
                }

            auto tableIt = tables.find(sourceTableId);
            if (tableIt == tables.end())
                {
                group.m_isUnsafe = true;
                ++skipped;
                messages.push_back(Utf8PrintfString("Skipped shared property %" PRIu64 ": its source table metadata is unavailable.", group.m_rootPropertyId.GetValueUnchecked()));
                continue;
                }
            DbTableId baseTableId = tableIt->second.m_type == 3 ? tableIt->second.m_parentId : sourceTableId;
            if (!baseTableId.IsValid())
                {
                group.m_isUnsafe = true;
                ++skipped;
                messages.push_back(Utf8PrintfString("Skipped shared property %" PRIu64 ": its source table is not a supported primary/joined plus overflow family.", group.m_rootPropertyId.GetValueUnchecked()));
                continue;
                }
            if ((rc = GroupHasValues(ecdb, group, tableIt->second, group.m_hasValues)) != BE_SQLITE_OK)
                {
                error = ecdb.GetLastError();
                return rc;
                }

            groupsByFamily[baseTableId].push_back(&group);
            tablesByFamily[baseTableId].insert(baseTableId);
            for (auto const& possibleChild : tables)
                if (possibleChild.second.m_parentId == baseTableId && possibleChild.second.m_type == 3)
                    tablesByFamily[baseTableId].insert(possibleChild.first);
            }
        }

    auto overlaps = [](SharedPropertyGroup const& lhs, SharedPropertyGroup const& rhs)
        {
        return std::any_of(lhs.m_rowClassIds.begin(), lhs.m_rowClassIds.end(),
            [&rhs](ECClassId classId) { return rhs.m_rowClassIds.count(classId) != 0; });
        };

    for (auto& familyEntry : groupsByFamily)
        {
        bvector<SharedPropertyGroup*>& groups = familyEntry.second;
        std::sort(groups.begin(), groups.end(), [](SharedPropertyGroup const* lhs, SharedPropertyGroup const* rhs)
            {
            if (lhs->m_hasValues != rhs->m_hasValues)
                return lhs->m_hasValues;
            SharedColumnSlot const* lhsFirst = lhs->m_sources.front();
            SharedColumnSlot const* rhsFirst = rhs->m_sources.front();
            return std::tie(lhsFirst->m_tableId, lhsFirst->m_ordinal, lhs->m_rootPropertyId) <
                   std::tie(rhsFirst->m_tableId, rhsFirst->m_ordinal, rhs->m_rootPropertyId);
            });

        bvector<SharedColumnSlot const*> available;
        for (DbTableId tableId : tablesByFamily[familyEntry.first])
            {
            auto slotList = slotsByTable.find(tableId);
            if (slotList == slotsByTable.end())
                continue;
            available.insert(available.end(), slotList->second.begin(), slotList->second.end());
            }
        std::sort(available.begin(), available.end(), [&tables](SharedColumnSlot const* lhs, SharedColumnSlot const* rhs)
            {
            auto rank = [&tables](SharedColumnSlot const* slot)
                {
                auto const& table = tables.at(slot->m_tableId);
                return std::make_tuple(table.m_type == 3 ? 1 : 0, table.m_id, slot->m_ordinal);
                };
            return rank(lhs) < rank(rhs);
            });

        for (SharedPropertyGroup* group : groups)
            {
            if (std::any_of(group->m_sources.begin(), group->m_sources.end(),
                [](SharedColumnSlot const* slot) { return slot->m_pinned; }))
                {
                group->m_isUnsafe = true;
                ++skipped;
                messages.push_back(Utf8PrintfString("Skipped shared property %" PRIu64 ": a mapped slot is protected by schema ownership or a physical index, trigger, or external view.", group->m_rootPropertyId.GetValueUnchecked()));
                }
            }

        std::map<DbColumnId, bvector<SharedPropertyGroup const*>> assignments;
        bool retryPlan;
        do
            {
            retryPlan = false;
            assignments.clear();
            for (SharedPropertyGroup* group : groups)
                {
                group->m_destinations.clear();
                if (!group->m_isUnsafe)
                    continue;
                group->m_destinations = group->m_sources;
                for (SharedColumnSlot const* source : group->m_sources)
                    assignments[source->m_id].push_back(group);
                }

            for (SharedPropertyGroup* group : groups)
                {
                if (group->m_isUnsafe)
                    continue;
                bool found = false;
                for (size_t i = 0; i + group->m_sources.size() <= available.size(); ++i)
                    {
                    DbTableId tableId = available[i]->m_tableId;
                    bool compatible = true;
                    for (size_t componentIndex = 0; componentIndex < group->m_sources.size(); ++componentIndex)
                        {
                        SharedColumnSlot const* destination = available[i + componentIndex];
                        if (destination->m_tableId != tableId || destination->m_pinned ||
                            !AreCompatible(*group->m_sources[componentIndex], *destination))
                            {
                            compatible = false;
                            break;
                            }
                        auto assignedIt = assignments.find(destination->m_id);
                        if (assignedIt != assignments.end() && std::any_of(assignedIt->second.begin(), assignedIt->second.end(),
                            [group, &overlaps](SharedPropertyGroup const* assignedGroup) { return overlaps(*group, *assignedGroup); }))
                            {
                            compatible = false;
                            break;
                            }
                        }
                    if (!compatible)
                        continue;
                    group->m_destinations.assign(available.begin() + i, available.begin() + i + group->m_sources.size());
                    for (SharedColumnSlot const* destination : group->m_destinations)
                        assignments[destination->m_id].push_back(group);
                    found = true;
                    break;
                    }
                if (found)
                    continue;

                group->m_isUnsafe = true;
                ++skipped;
                messages.push_back(Utf8PrintfString("Skipped shared property %" PRIu64 ": no atomic compatible destination group is available.", group->m_rootPropertyId.GetValueUnchecked()));
                retryPlan = true;
                break;
                }
            }
        while (retryPlan);

        for (SharedPropertyGroup const* group : groups)
            {
            if (group->m_isUnsafe)
                continue;
            bool groupMoves = false;
            for (size_t i = 0; i < group->m_components.size(); ++i)
                {
                if (group->m_sources[i]->m_id == group->m_destinations[i]->m_id)
                    continue;
                groupMoves = true;
                RemapManager::ColumnRemap remap;
                remap.m_id = nextMoveId++;
                remap.m_sourceTable = group->m_sources[i]->m_tableName;
                remap.m_sourceIdColumn = group->m_sources[i]->m_idColumn;
                remap.m_sourceColumn = group->m_sources[i]->m_name;
                remap.m_destinationTable = group->m_destinations[i]->m_tableName;
                remap.m_destinationIdColumn = group->m_destinations[i]->m_idColumn;
                remap.m_destinationColumn = group->m_destinations[i]->m_name;
                remap.m_rowPredicate = ClassRowPredicate(tables.at(group->m_sources[i]->m_tableId), group->m_rowClassIds);
                remap.m_rowClassIds = group->m_rowClassIds;
                remap.m_sourceColumnId = group->m_sources[i]->m_id;
                bool firstClass = true;
                for (ECClassId classId : group->m_rowClassIds)
                    {
                    if (!firstClass)
                        remap.m_planClassIds.append(",");
                    remap.m_planClassIds.append(classId.ToHexStr());
                    firstClass = false;
                    }
                auto const& destinationTable = tables.at(group->m_destinations[i]->m_tableId);
                if (destinationTable.m_type == 3 && destinationTable.m_parentId.IsValid())
                    {
                    auto const& parentTable = tables.at(destinationTable.m_parentId);
                    remap.m_destinationParentTable = parentTable.m_name;
                    remap.m_destinationParentIdColumn = parentTable.m_idColumn;
                    remap.m_destinationClassIdColumn = destinationTable.m_classIdColumn;
                    remap.m_destinationParentClassIdColumn = parentTable.m_classIdColumn;
                    remap.m_destinationParentPredicate = ClassRowPredicate(parentTable, group->m_rowClassIds, "p");
                    }
                remaps.push_back(remap);
                for (uint64_t propertyMapId : group->m_components[i].m_propertyMapIds)
                    propertyMapUpdates.push_back(std::make_pair(propertyMapId, group->m_destinations[i]->m_id));
                }
            if (groupMoves)
                ++candidates;
            }

        bset<ECClassId> rowClasses;
        for (SharedPropertyGroup const* group : groups)
            rowClasses.insert(group->m_rowClassIds.begin(), group->m_rowClassIds.end());
        for (ECClassId classId : rowClasses)
            {
            for (SharedColumnSlot const* slot : available)
                {
                auto assignedIt = assignments.find(slot->m_id);
                bool used = assignedIt != assignments.end() && std::any_of(assignedIt->second.begin(), assignedIt->second.end(),
                    [classId](SharedPropertyGroup const* group) { return group->m_rowClassIds.count(classId) != 0; });
                if (!used && !slot->m_pinned)
                    cellsToClear.insert(std::make_pair(classId, slot->m_id));
                }
            }
        }

    for (RemapManager::ColumnRemap& remap : remaps)
        {
        remap.m_clearSource = true;
        for (auto const& familyEntry : groupsByFamily)
            {
            bool sourceStillUsed = std::any_of(familyEntry.second.begin(), familyEntry.second.end(),
                [&remap](SharedPropertyGroup const* group)
                    {
                    if (group->m_destinations.empty() ||
                        !std::any_of(group->m_destinations.begin(), group->m_destinations.end(),
                            [&remap](SharedColumnSlot const* slot) { return slot->m_id == remap.m_sourceColumnId; }))
                        return false;
                    return std::any_of(remap.m_rowClassIds.begin(), remap.m_rowClassIds.end(),
                        [group](ECClassId classId) { return group->m_rowClassIds.count(classId) != 0; });
                    });
            if (sourceStillUsed)
                {
                remap.m_clearSource = false;
                break;
                }
            }
        }

    if (remaps.empty() && cellsToClear.empty())
        {
        ECDB_PERF_LOG_SCOPE_END(compactDiscovery);
        // No column moves planned, but overflow orphan rows may still need cleanup.
        uint64_t overflowCandidatesOnly = 0, overflowChangedOnly = 0;
        if ((rc = PurgeOverflowOrphanRows(ecdb, propertyMapUpdates, isDryRun, overflowCandidatesOnly, overflowChangedOnly, messages, error)) != BE_SQLITE_OK)
            return rc;
        candidates += overflowCandidatesOnly;
        if (!isDryRun && overflowChangedOnly != 0)
            {
            changed += overflowChangedOnly;
            ecdb.ClearECDbCache();
            }
        return BE_SQLITE_OK;
        }

    ECDB_PERF_LOG_SCOPE_END(compactDiscovery);
    ECDB_PERF_LOG_SCOPE_BEGIN(compactMovement, "Optimizer> Compact shared columns> Data movement");

    uint64_t const movedGroupCandidates = candidates;

    bset<ECClassId> affectedClassIds;
    for (RemapManager::ColumnRemap const& remap : remaps)
        affectedClassIds.insert(remap.m_rowClassIds.begin(), remap.m_rowClassIds.end());

    bvector<ECClassId> droppedClassViewIds;
    // Covers every mutation below (view drops, data remaps, metadata updates, stale-cell clears, and
    // overflow orphan-row deletes); dismissed on the dry-run and on the fully successful execute path.
    CacheInvalidationGuard cacheGuard(ecdb);
    if (!isDryRun && !affectedClassIds.empty() && DropClassViews(ecdb, affectedClassIds, droppedClassViewIds, error) != BE_SQLITE_OK)
        {
        if (error.empty())
            error = "Failed to drop affected ECClass views before compacting shared columns.";
        return BE_SQLITE_ERROR;
        }

    if (!remaps.empty() && (rc = RemapManager::ApplyColumnRemaps(ecdb, remaps, isDryRun, error)) != BE_SQLITE_OK)
        return rc;

    if (!isDryRun)
        {
        Statement updateMap;
        if ((rc = updateMap.Prepare(ecdb, "UPDATE ec_PropertyMap SET ColumnId=? WHERE Id=?")) != BE_SQLITE_OK)
            return rc;
        for (auto const& update : propertyMapUpdates)
            {
            updateMap.Reset();
            updateMap.ClearBindings();
            if ((rc = updateMap.BindId(1, update.second)) != BE_SQLITE_OK ||
                (rc = updateMap.BindUInt64(2, update.first)) != BE_SQLITE_OK ||
                (rc = updateMap.Step()) != BE_SQLITE_DONE)
                {
                error = ecdb.GetLastError();
                return rc;
                }
            }
        updateMap.Finalize();

        for (auto const& cell : cellsToClear)
            {
            auto slotIt = slotsById.find(cell.second);
            if (slotIt == slotsById.end())
                continue;
            auto tableIt = tables.find(slotIt->second.m_tableId);
            if (tableIt == tables.end())
                continue;
            NativeSqlBuilder sql("UPDATE [main].");
            sql.AppendEscaped(tableIt->second.m_name).Append(" SET ")
                .AppendEscaped(slotIt->second.m_name).Append("=NULL WHERE ")
                .Append(ClassRowPredicate(tableIt->second, cell.first));
            if ((rc = ecdb.TryExecuteSql(sql.GetSql().c_str())) != BE_SQLITE_OK)
                {
                error = ecdb.GetLastError();
                return rc;
                }
            }
        }

    // Overflow orphan-row cleanup runs after moves so validity is computed from the final ec_PropertyMap
    // state: rows that legitimately received moved data remain valid and are retained; only rows whose
    // class is not valid for the overflow table are removed.
    uint64_t overflowCandidates = 0, overflowChanged = 0;
    if ((rc = PurgeOverflowOrphanRows(ecdb, propertyMapUpdates, isDryRun, overflowCandidates, overflowChanged, messages, error)) != BE_SQLITE_OK)
        return rc;

    // candidates counts the movable groups discovered during planning plus overflow orphan rows
    // discovered; changed counts the groups actually relocated plus overflow rows deleted. Group moves
    // are applied atomically by ApplyColumnRemaps, so every discovered movable group is a changed group.
    candidates = movedGroupCandidates + overflowCandidates;
    if (isDryRun)
        {
        cacheGuard.Dismiss();
        return BE_SQLITE_OK;
        }

    changed = movedGroupCandidates + overflowChanged;
    ecdb.ClearECDbCache();
    cacheGuard.Dismiss();
    if (!droppedClassViewIds.empty() && SUCCESS != ecdb.Schemas().Main().CreateClassViews(droppedClassViewIds))
        {
        error = "Failed to recreate ECClass views after compacting shared columns.";
        return BE_SQLITE_ERROR;
        }
    return BE_SQLITE_OK;
    }

DbResult ExecuteOption(ECDbR ecdb, Optimizer::Options option, bool isDryRun, SchemaImportToken const* schemaToken, ECCrudWriteToken const* crudToken, uint64_t& candidates, uint64_t& changed, uint64_t& skipped, bvector<Utf8String>& messages, Utf8StringR error)
    {
    switch (option)
        {
        case Optimizer::Options::DropUnusedSchemas:
            return DropUnusedSchemas(ecdb, isDryRun, schemaToken, candidates, changed, error);
        case Optimizer::Options::DropEmptyDynamicClasses:
            return DropEmptyDynamicClasses(ecdb, isDryRun, schemaToken, candidates, changed, skipped, error);
        case Optimizer::Options::DropEmptyDynamicProperties:
            return DropEmptyDynamicProperties(ecdb, isDryRun, schemaToken, candidates, changed, skipped, error);
        case Optimizer::Options::CompactSharedColumns:
            return CompactSharedColumns(ecdb, isDryRun, candidates, changed, skipped, messages, error);
        case Optimizer::Options::PurgeInvalidClassIds:
            {
            const auto rc = PurgeInvalidClassIds(ecdb, isDryRun, candidates, changed);
            if (rc != BE_SQLITE_OK)
                error = ecdb.GetLastError();
            return rc;
            }
        case Optimizer::Options::DeleteOrphanRelationships:
            {
            IntegrityChecker checker(ecdb);
            const auto rc = checker.PurgeOrphanRelationships(candidates, isDryRun, crudToken, true);
            changed = isDryRun ? 0 : candidates;
            if (rc != BE_SQLITE_OK)
                error = checker.GetLastError();
            return rc;
            }
        case Optimizer::Options::NullifyOrphanNavProps:
            {
            IntegrityChecker checker(ecdb);
            const auto rc = checker.RepairNavIds(candidates, isDryRun, crudToken);
            changed = isDryRun ? 0 : candidates;
            if (rc != BE_SQLITE_OK)
                error = checker.GetLastError();
            return rc;
            }
        case Optimizer::Options::CleanOrphanCustomAttributes:
            {
            const auto rc = CleanOrphanCustomAttributes(ecdb, isDryRun, candidates, changed);
            if (rc != BE_SQLITE_OK)
                error = ecdb.GetLastError();
            return rc;
            }
        case Optimizer::Options::DropUnmappedTables:
            return DropUnmappedTables(ecdb, isDryRun, candidates, changed, skipped, error);
        case Optimizer::Options::DropUnmappedSharedColumns:
            return DropUnmappedSharedColumns(ecdb, isDryRun, candidates, changed, skipped, error);
        case Optimizer::Options::RunAnalyze:
            if (isDryRun)
                return BE_SQLITE_OK;
            {
            // Run ANALYZE through the ordinary SQL execution path so it stays inside the caller's active
            // transaction. Deliberately NOT Db::Analyze(), which uses SuspendDefaultTxn and would commit
            // the caller's default transaction. sqlite_stat1 is intentionally change-tracked (see
            // BeSQLite Db creation), so no separate untracked routing is required.
            const auto rc = ecdb.TryExecuteSql("ANALYZE [main]");
            if (rc != BE_SQLITE_OK)
                error = ecdb.GetLastError();
            return rc;
            }
        default:
            error.Sprintf("Optimizer option '%s' is not implemented.", Optimizer::GetOptionName(option));
            return BE_SQLITE_ERROR;
        }
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8CP Optimizer::GetOptionName(Options option)
    {
    for (auto const& entry : s_optionEntries)
        {
        if (entry.m_option == option)
            return entry.m_name;
        }

    switch (option)
        {
        case Options::None:
            return "none";
        case Options::SchemaCleanup:
            return "schema_cleanup";
        case Options::DataCleanup:
            return "data_cleanup";
        case Options::StorageCleanup:
            return "storage_cleanup";
        case Options::All:
            return "all";
        default:
            return "unknown";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void Optimizer::Result::ToJson(BeJsValue value) const
    {
    value.SetEmptyObject();
    value["dryRun"] = m_isDryRun;
    value["status"] = static_cast<int>(m_status);
    value["failedOption"] = Optimizer::GetOptionName(m_failedOption);

    BeJsValue phases = value["phases"];
    phases.SetEmptyArray();
    for (Phase const& phase : m_phases)
        {
        BeJsValue jsonPhase = phases.appendValue();
        jsonPhase["option"] = Optimizer::GetOptionName(phase.m_option);
        jsonPhase["candidates"] = phase.m_candidates;
        jsonPhase["changed"] = phase.m_changed;
        jsonPhase["skipped"] = phase.m_skipped;
        jsonPhase["elapsedSeconds"] = phase.m_elapsed.ToSeconds();
        }

    BeJsValue messages = value["messages"];
    messages.SetEmptyArray();
    for (Utf8StringCR message : m_messages)
        messages.appendValue() = message;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult Optimizer::Optimize(Options options, Result& result, SchemaImportToken const* schemaToken, ECCrudWriteToken const* crudToken)
    {
    return Run(options, result, false, schemaToken, crudToken);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult Optimizer::DryRun(Options options, Result& result)
    {
    return Run(options, result, true, nullptr, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult Optimizer::Run(Options options, Result& result, bool isDryRun, SchemaImportToken const* schemaToken, ECCrudWriteToken const* crudToken)
    {
    result = Result();
    result.m_isDryRun = isDryRun;
    m_lastError.clear();

    if ((ToMask(options) & ~ToMask(Options::All)) != 0)
        {
        m_lastError = "Optimizer options contain an unknown flag.";
        result.m_status = BE_SQLITE_MISUSE;
        return result.m_status;
        }

    if (options == Options::None)
        return BE_SQLITE_OK;

    if (!isDryRun && m_ecdb.IsReadonly())
        {
        m_lastError = "Optimizer requires a writable ECDb.";
        result.m_status = BE_SQLITE_READONLY;
        return result.m_status;
        }

    if (!m_ecdb.CheckProfileVersion().IsUpToDate())
        {
        m_lastError = "Optimizer requires current BeSQLite and ECDb profiles.";
        result.m_status = BE_SQLITE_SCHEMA;
        return result.m_status;
        }

    if (!isDryRun && !m_ecdb.IsTransactionActive())
        {
        m_lastError = "Optimizer requires an active caller-owned transaction.";
        result.m_status = BE_SQLITE_MISUSE;
        return result.m_status;
        }

    if (!isDryRun && RequiresCrudWritePermission(options) &&
        !PolicyManager::GetPolicy(ECCrudPermissionPolicyAssertion(m_ecdb, true, crudToken)).IsSupported())
        {
        m_lastError = "Optimizer requires a valid ECCrudWriteToken for data cleanup.";
        result.m_status = BE_SQLITE_AUTH;
        return result.m_status;
        }

    if (!isDryRun && RequiresSchemaImportPermission(options) &&
        !PolicyManager::GetPolicy(SchemaImportPermissionPolicyAssertion(m_ecdb, schemaToken)).IsSupported())
        {
        m_lastError = "Optimizer requires a valid SchemaImportToken for schema or mapping cleanup.";
        result.m_status = BE_SQLITE_AUTH;
        return result.m_status;
        }

    ECDB_PERF_LOG_SCOPE_BEGIN(optimizerRun, isDryRun ? "Optimizer> DryRun" : "Optimizer> Optimize");
    for (OptionEntry const& entry : s_optionEntries)
        {
        if (!Contains(options, entry.m_option))
            continue;

        StopWatch timer(true);
        Result::Phase phase;
        phase.m_option = entry.m_option;
        Utf8String error;
        const auto rc = ExecuteOption(m_ecdb, entry.m_option, isDryRun, schemaToken, crudToken, phase.m_candidates, phase.m_changed, phase.m_skipped, result.m_messages, error);
        timer.Stop();
        phase.m_elapsed = timer.GetElapsed();
        result.m_phases.push_back(phase);
        if (!isDryRun && phase.m_changed != 0 && ChangesMetadata(entry.m_option))
            m_ecdb.ClearECDbCache();
        if (rc == BE_SQLITE_OK)
            continue;

        m_lastError = error.empty() ? m_ecdb.GetLastError() : error;
        result.m_messages.push_back(m_lastError);
        result.m_failedOption = entry.m_option;
        result.m_status = rc;
        return rc;
        }

    result.m_status = BE_SQLITE_OK;
    return BE_SQLITE_OK;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
