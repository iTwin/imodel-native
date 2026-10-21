/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECDbPch.h"
#include "SchemaCommand.h"
#include <tuple>

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
namespace {
// Mirrors the Created and added-column cases of DbSchemaPersistenceManager::CreateOrUpdateTable.
bool RequiresForeignKeyDerivation(ECDbCR db, DbSchema const& schema) {
    for (DbTable const* table : schema.Tables().GetTablesInDependencyOrder()) {
        if (table->GetType() == DbTable::Type::Virtual || table->GetType() == DbTable::Type::Existing)
            continue;
        if (!DbUtilities::TableExists(db, table->GetName().c_str()))
            return true;
        bvector<Utf8String> existing;
        if (!db.GetColumns(existing, table->GetName().c_str()))
            return true;
        bset<Utf8String, CompareIUtf8Ascii> names(existing.begin(), existing.end());
        for (DbColumn const* column : table->GetColumns())
            if (column->GetPersistenceType() == PersistenceType::Physical && names.find(column->GetName()) == names.end())
                return true;
    }
    return false;
}

//! The affected classes plus their ancestors, whose storage descriptions include them.
BentleyStatus CollectIndexScope(ECDbCR db, std::set<ECClassId> const& affected, MainSchemaManager::IndexScope& scope) {
    auto bases = db.GetCachedStatement("SELECT BaseClassId FROM main." TABLE_ClassHierarchyCache " WHERE ClassId=?");
    if (bases.IsNull())
        return ERROR;
    for (auto id : affected) {
        DbResult status;
        bases->Reset();
        bases->BindId(1, id);
        while ((status = bases->Step()) == BE_SQLITE_ROW)
            scope.m_classes.insert(bases->GetValueId<ECClassId>(0));
        if (status != BE_SQLITE_DONE)
            return ERROR;
        scope.m_classes.insert(id);
    }
    return SUCCESS;
}

int MappingOrder(ECClassCR ecClass) {
    if (ecClass.IsEntityClass() && ecClass.GetEntityClassCP()->IsMixin())
        return 0;
    return ecClass.IsRelationshipClass() ? 2 : 1;
}

// Match native depth-first hierarchy mapping without including classes outside the affected closure.
Utf8String MappingPath(ECClassCR ecClass) {
    Utf8String path;
    for (auto base : ecClass.GetBaseClasses())
        if (MappingOrder(*base) == MappingOrder(ecClass)) {
            path = MappingPath(*base);
            break;
        }
    if (!path.empty())
        path.append("/");
    return path.append(ecClass.GetFullName());
}

Utf8String QuoteIdentifier(Utf8StringCR value) {
    Utf8String quoted("\"");
    for (char c : value) {
        if (c == '"')
            quoted.push_back('"');
        quoted.push_back(c);
    }
    return quoted.append("\"");
}
}

SchemaCommandResult IncrementalSchemaMapper::PlanOverflowRows(SchemaEditContext& edit,
                                                              std::set<ECClassId> const& affected) {
    if (affected.empty())
        return {};
    auto const& manager = edit.m_db.Schemas().Main();
    Statement tables;
    if (BE_SQLITE_OK != tables.Prepare(edit.m_db,
                                       "SELECT DISTINCT p.Id,o.Id FROM main.ec_Table p JOIN main.ec_Table o ON o.ParentTableId=p.Id "
                                       "JOIN main.ec_Column c ON c.TableId=o.Id JOIN main.ec_PropertyMap m ON m.ColumnId=c.Id "
                                       "WHERE o.Type=" SQLVAL_DbTable_Type_Overflow))
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR,
                                            "SchemaEditor.Overflow", "", edit.m_db.GetLastError());
    DbResult status;
    while ((status = tables.Step()) == BE_SQLITE_ROW) {
        auto primary = manager.GetDbSchema().FindTable(tables.GetValueId<DbTableId>(0));
        auto overflow = manager.GetDbSchema().FindTable(tables.GetValueId<DbTableId>(1));
        if (primary == nullptr || overflow == nullptr)
            return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR,
                                                "SchemaEditor.Overflow", "", "An overflow mapping references a missing table.");
        auto primaryId = primary->FindFirst(DbColumn::Kind::ECInstanceId);
        auto primaryClass = primary->FindFirst(DbColumn::Kind::ECClassId);
        auto overflowId = overflow->FindFirst(DbColumn::Kind::ECInstanceId);
        auto overflowClass = overflow->FindFirst(DbColumn::Kind::ECClassId);
        if (!primaryId || !primaryClass || !overflowId || !overflowClass)
            return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR,
                                                "SchemaEditor.Overflow", "", "Overflow mappings require instance and class ID columns.");
        Utf8String ids;
        for (auto id : affected) {
            if (!ids.empty())
                ids.append(",");
            ids.append(Utf8PrintfString("%" PRIu64, id.GetValueUnchecked()));
        }
        Utf8String select = Utf8PrintfString(
            "SELECT P.%s,P.%s FROM %s P WHERE P.%s IN (%s) "
            "AND P.%s IN (SELECT m.ClassId FROM main.ec_PropertyMap m "
            "JOIN main.ec_Column c ON c.Id=m.ColumnId WHERE c.TableId=%" PRIu64 ") "
            "AND NOT EXISTS(SELECT 1 FROM %s O WHERE O.%s=P.%s)",
            QuoteIdentifier(primaryId->GetName()).c_str(), QuoteIdentifier(primaryClass->GetName()).c_str(),
            QuoteIdentifier(primary->GetName()).c_str(), QuoteIdentifier(primaryClass->GetName()).c_str(),
            ids.c_str(), QuoteIdentifier(primaryClass->GetName()).c_str(), overflow->GetId().GetValueUnchecked(),
            QuoteIdentifier(overflow->GetName()).c_str(), QuoteIdentifier(overflowId->GetName()).c_str(),
            QuoteIdentifier(primaryId->GetName()).c_str());
        Statement missing;
        if (BE_SQLITE_OK != missing.Prepare(edit.m_db, (select + " LIMIT 1").c_str()))
            return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR,
                                                "SchemaEditor.Overflow", "", edit.m_db.GetLastError());
        auto missingStatus = missing.Step();
        if (missingStatus == BE_SQLITE_DONE)
            continue;
        if (missingStatus != BE_SQLITE_ROW)
            return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR,
                                                "SchemaEditor.Overflow", "", edit.m_db.GetLastError());
        auto insert = Utf8PrintfString("INSERT INTO %s(%s,%s) %s",
                                       QuoteIdentifier(overflow->GetName()).c_str(), QuoteIdentifier(overflowId->GetName()).c_str(),
                                       QuoteIdentifier(overflowClass->GetName()).c_str(), select.c_str());
        edit.m_mapping.GetDataTransform().Append("Create missing overflow rows for the edited classes.", insert);
    }
    if (status != BE_SQLITE_DONE)
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR,
                                            "SchemaEditor.Overflow", "", edit.m_db.GetLastError());
    return {};
}

namespace {
//! Same effect as ViewGenerator::DropECClassViews, but issues DROP only for class views that exist
//! instead of one DROP VIEW IF EXISTS per class, which made every mapping pass linear in all classes.
BentleyStatus DropExistingClassViews(ECDbCR db) {
    bvector<Utf8String> drops;
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(db, "SELECT 'DROP VIEW IF EXISTS main.[' || s.Alias || '.' || c.Name || '];' FROM main.ec_Class c "
                                         "JOIN main.ec_Schema s ON s.Id=c.SchemaId "
                                         "JOIN main.sqlite_master m ON m.type='view' AND m.name=s.Alias || '.' || c.Name COLLATE NOCASE"))
        return ERROR;
    while (stmt.Step() == BE_SQLITE_ROW)
        drops.push_back(stmt.GetValueText(0));
    stmt.Finalize();
    for (auto const& sql : drops)
        if (BE_SQLITE_OK != db.ExecuteSql(sql.c_str()))
            return ERROR;
    return SUCCESS;
}
}

SchemaCommandResult IncrementalSchemaMapper::Map(SchemaEditContext& edit) {
    auto& db = edit.m_db;
    auto const& manager = db.Schemas().Main();
    auto& ctx = edit.m_mapping;
    auto failure = [&](Utf8CP stage) {
        ctx.ReportMappingFailureDiagnostics();
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR,
                                            "SchemaEditor.Mapping", "", Utf8PrintfString("Schema command mapping failed during %s: %s", stage, db.GetLastError().c_str()));
    };
    if (SUCCESS != ctx.GetSchemaPoliciesR().ReadPolicies(db))
        return failure("policy loading");
    if (SUCCESS != DropExistingClassViews(db))
        return failure("class view invalidation");
    if (edit.m_affectedClasses.empty()) {
        if (!edit.m_affectedSchemas.empty() && SUCCESS != DbSchemaPersistenceManager::RepopulateClassHasTableCacheTable(db))
            return failure("class-has-table cache population");
        return {};
    }

    // A struct edit affects every transitive user, including users in other schemas.
    std::set<ECClassId> affected;
    bool hasDeletedClass = false;
    for (auto id : edit.m_affectedClasses) {
        Statement existsStmt;
        if (BE_SQLITE_OK != existsStmt.Prepare(db, "SELECT 1 FROM main.ec_Class WHERE Id=?") ||
            BE_SQLITE_OK != existsStmt.BindId(1, id))
            return failure("dependency discovery");
        if (existsStmt.Step() == BE_SQLITE_DONE) {
            hasDeletedClass = true;
            continue;
        }
        Statement stmt;
        if (BE_SQLITE_OK != stmt.Prepare(db,
                                         "WITH RECURSIVE affected(Id) AS (SELECT ? UNION "
                                         "SELECT b.ClassId FROM main.ec_ClassHasBaseClasses b JOIN affected a ON a.Id=b.BaseClassId UNION "
                                         "SELECT p.ClassId FROM main.ec_Property p JOIN affected a ON a.Id=p.StructClassId UNION "
                                         "SELECT p.ClassId FROM main.ec_Property p JOIN affected a ON a.Id=p.NavigationRelationshipClassId) "
                                         "SELECT a.Id FROM affected a JOIN main.ec_Class c ON c.Id=a.Id") ||
            BE_SQLITE_OK != stmt.BindId(1, id))
            return failure("dependency discovery");
        DbResult status;
        while ((status = stmt.Step()) == BE_SQLITE_ROW)
            affected.insert(stmt.GetValueId<ECClassId>(0));
        if (status != BE_SQLITE_DONE)
            return failure("dependency discovery");
    }
    // Only the closure's property maps can have changed since the cache was last populated;
    // rows of deleted classes and tables were removed by their foreign key cascades.
    if (SUCCESS != DbSchemaPersistenceManager::RepopulateClassHasTableCacheTable(db, affected))
        return failure("class-has-table cache population");
    if (SUCCESS != ctx.RemapManager().CleanModifiedMappings())
        return failure("property remap preparation");
    bvector<ECSchemaCP> involvedSchemas;
    std::set<ECSchemaId> involvedSchemaIds;
    bvector<ECClassCP> classes;
    for (auto id : affected) {
        auto ecClass = manager.GetClass(id);
        if (ecClass == nullptr)
            return failure("edited class loading");
        if (involvedSchemaIds.insert(ecClass->GetSchema().GetId()).second)
            involvedSchemas.push_back(&ecClass->GetSchema());
        if (ecClass->IsEntityClass() || ecClass->IsRelationshipClass() || ecClass->IsStructClass() || ecClass->IsCustomAttributeClass())
            classes.push_back(ecClass);
    }
    if (SUCCESS != ctx.RemapManager().EnsureInvolvedSchemasAreLoaded(involvedSchemas))
        return failure("schema dependency loading");
    if (SUCCESS != manager.GetDbSchemaR().SynchronizeExistingTables())
        return failure("physical table synchronization");
    std::sort(classes.begin(), classes.end(), [](ECClassCP lhs, ECClassCP rhs) {
        return std::make_tuple(MappingOrder(*lhs), MappingPath(*lhs)) <
               std::make_tuple(MappingOrder(*rhs), MappingPath(*rhs));
    });
    // Multiple mixin bases must all precede their consumer, even when the first base's path sorts first.
    bvector<ECClassCP> ordered;
    std::set<ECClassId> scheduled;
    std::function<void(ECClassCP)> schedule = [&](ECClassCP ecClass) {
        if (!scheduled.insert(ecClass->GetId()).second)
            return;
        for (auto base : ecClass->GetBaseClasses())
            if (affected.count(base->GetId()) != 0)
                schedule(base);
        ordered.push_back(ecClass);
    };
    for (auto ecClass : classes)
        schedule(ecClass);
    classes.swap(ordered);
    // Update only the closure selected above. Unlike MainSchemaManager::MapClass, this
    // does not recursively remap unrelated descendants a second time.
    std::set<ECClassId> createdMaps;
    for (auto ecClass : classes) {
        ClassMap* classMap = nullptr;
        if (SUCCESS != manager.TryGetClassMap(classMap, ctx.GetClassMapLoadContext(), *ecClass))
            return failure("class map loading");
        if (classMap != nullptr) {
            if (SUCCESS != classMap->Update(ctx))
                return failure("class map update");
            continue;
        }
        ClassMappingInfo info(ctx, *ecClass);
        auto status = info.Initialize();
        if (status == ClassMappingStatus::Error || status == ClassMappingStatus::BaseClassesNotMapped)
            return failure("class mapping validation");

        auto const& strategy = info.GetMapStrategy();
        std::unique_ptr<ClassMap> created;
        if (strategy.GetStrategy() == MapStrategy::NotMapped)
            created = ClassMap::Create<NotMappedClassMap>(db, manager, *ecClass, strategy);
        else if (auto rel = ecClass->GetRelationshipClassCP()) {
            if (MapStrategyExtendedInfo::IsForeignKeyMapping(strategy))
                created = ClassMap::Create<RelationshipClassEndTableMap>(db, manager, *rel, strategy);
            else
                created = ClassMap::Create<RelationshipClassLinkTableMap>(db, manager, *rel, strategy);
        } else
            created = ClassMap::Create<ClassMap>(db, manager, *ecClass, strategy);
        classMap = manager.AddClassMap(std::move(created));
        if (classMap == nullptr)
            return failure("class map registration");
        ctx.AddClassMapForSaving(ecClass->GetId());
        createdMaps.insert(ecClass->GetId());
        status = classMap->Map(ctx, info);
        if (status == ClassMappingStatus::Error || status == ClassMappingStatus::BaseClassesNotMapped)
            return failure("class mapping");
        if (SUCCESS != DbMappingManager::Classes::MapIndexes(ctx, *classMap, false))
            return failure("index mapping");
    }
    if (SUCCESS != DbMappingManager::FkRelationships::FinishMapping(ctx))
        return failure("foreign key mapping");
    auto persistMappings = [&]() -> BentleyStatus {
        if (SUCCESS != manager.GetDbSchema().SaveOrUpdateTables())
            return ERROR;
        DbMapSaveContext saveContext(db);
        for (auto const& entry : manager.GetClassMapCache()) {
            auto& classMap = *entry.second;
            if (classMap.GetState() != ObjectState::Persisted &&
                SUCCESS != classMap.Save(ctx, saveContext))
                return ERROR;
        }
        // ClassMap::Save wrote every property map of a class created above, including inherited ones;
        // they can only differ from the persisted rows after the remapper restores cleaned maps.
        bool createdMapsAreCurrent = !ctx.RemapManager().HasCleanedPropertyMappings();
        for (auto ecClass : classes) {
            ClassMap* cm = nullptr;
            if (SUCCESS != manager.TryGetClassMap(cm, ctx.GetClassMapLoadContext(), *ecClass) || cm == nullptr)
                return ERROR;
            bool currentDataMaps = createdMapsAreCurrent && createdMaps.count(ecClass->GetId()) != 0;
            for (PropertyMap const* pm : cm->GetPropertyMaps()) {
                if (pm->GetType() == PropertyMap::Type::Navigation) {
                    auto const& navPm = pm->GetAs<NavigationPropertyMap>();
                    if (!navPm.IsComplete())
                        return ERROR;
                    if (SUCCESS != manager.GetDbSchema().UpdateTable(navPm.GetTable()))
                        return ERROR;
                } else if (currentDataMaps || !pm->IsData() || pm->GetProperty().GetClass().GetId() == ecClass->GetId())
                    continue;
                // Loading a class map can clone an inherited map without marking it modified.
                // Native rows may retain an ancestor's property id; reuse their logical mapping
                // instead of inserting another row under the cloned property's id.
                DbMapSaveContext navSaveContext(db);
                navSaveContext.BeginSaving(*cm);
                DbClassMapSaveContext classMapCtx(navSaveContext);
                SavePropertyMapVisitor saveVisitor(classMapCtx, SavePropertyMapVisitor::Action::SkipIfMapped);
                auto status = pm->AcceptVisitor(saveVisitor);
                navSaveContext.EndSaving(*cm);
                if (SUCCESS != status)
                    return ERROR;
            }
        }
        manager.m_lightweightCache.Clear();
        return SUCCESS;
    };
    if (SUCCESS != persistMappings())
        return failure("mapping persistence");
    // Without cleaned property maps nothing is restored, so persisting again would repeat the pass above.
    if (ctx.RemapManager().HasCleanedPropertyMappings() &&
        (SUCCESS != ctx.RemapManager().RestoreAndProcessCleanedPropertyMaps(ctx) || SUCCESS != persistMappings()))
        return failure("property remap persistence");
    if (SUCCESS != DbSchemaPersistenceManager::RepopulateClassHasTableCacheTable(db, affected))
        return failure("derived database structures");
    // Foreign key derivation loads every relationship constraint's class maps, so skip it
    // when CreateOrUpdateRequiredTables will not emit any DDL that carries foreign keys.
    auto status = RequiresForeignKeyDerivation(db, manager.GetDbSchema()) ? DerivedDbStructures::Derive(manager)
                                                                         : DerivedDbStructures::DeriveTriggers(manager);
    if (SUCCESS != status)
        return failure("derived database structures");
    // Index definitions depend on their table's mapped classes and their class's storage. Removed
    // classes or columns leave no cache rows to scope by, so they reconcile every index.
    MainSchemaManager::IndexScope indexScope;
    bool scopeIndexes = !hasDeletedClass && !ctx.RemapManager().HasFreedColumns();
    if (scopeIndexes && SUCCESS != CollectIndexScope(db, affected, indexScope))
        return failure("index scope discovery");
    if (SUCCESS != manager.CreateOrUpdateRequiredTables(&affected) ||
        SUCCESS != manager.CreateOrUpdateIndexesInDb(ctx, scopeIndexes ? &indexScope : nullptr))
        return failure("physical schema reconciliation");
    // Drop any tables that became orphaned after property or class removal.
    if ((hasDeletedClass || ctx.RemapManager().HasFreedColumns()) && SUCCESS != manager.PurgeOrphanTables(ctx))
        return failure("orphan table cleanup");
    // The target rows must exist before remap UPDATE statements move values into them.
    auto overflowResult = PlanOverflowRows(edit, affected);
    if (!overflowResult.IsOk())
        return overflowResult;
    if (SUCCESS != ctx.RemapManager().UpgradeExistingECInstancesWithRemappedProperties(ctx))
        return failure("data transformation planning");
    if (!ctx.GetDataTransform().IsEmpty()) {
        if (!ctx.AllowDataTransform())
            return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_DATA_TRANSFORM_REQUIRED,
                                                "SchemaEditor.DataTransformRequired", "", "The mapping requires data movement; enable AllowDataTransformDuringSchemaUpgrade.");
        if (!ctx.GetDataTransform().Validate(db) || BE_SQLITE_OK != ctx.GetDataTransform().Execute(db))
            return failure("data transformation");
    }
    return {};
}
END_BENTLEY_SQLITE_EC_NAMESPACE
