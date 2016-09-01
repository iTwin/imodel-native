/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbSchemaPersistenceManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

#define EC_INDEX_TableName "ec_Index"
#define EC_INDEXCOLUMN_TableName "ec_IndexColumn"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        08/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::RepopulateClassHierarchyCacheTable(ECDbCR ecdb)
    {
    StopWatch timer(true);
    if (BE_SQLITE_OK != ecdb.ExecuteSql("DELETE FROM " ECDB_CACHETABLE_ClassHierarchy))
        return ERROR;

    if (BE_SQLITE_OK != ecdb.ExecuteSql(
                    "WITH RECURSIVE "
                    "BaseClassList(ClassId, BaseClassId) AS "
                    "("
                    "   SELECT Id, Id FROM ec_Class"
                    "   UNION"
                    "   SELECT DCL.ClassId, BC.BaseClassId FROM BaseClassList DCL"
                    "       INNER JOIN ec_ClassHasBaseClasses BC ON BC.ClassId = DCL.BaseClassId"
                    ")"
                    "INSERT INTO " ECDB_CACHETABLE_ClassHierarchy " SELECT NULL Id, ClassId, BaseClassId FROM BaseClassList"))
        {
        return ERROR;
        }

    timer.Stop();
    LOG.debugv("Re-populated table '" ECDB_CACHETABLE_ClassHierarchy "' in %.4f msecs.", timer.GetElapsedSeconds() * 1000.0);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        08/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::RepopulateClassHasTableCacheTable(ECDbCR ecdb)
    {
    StopWatch timer(true);
    if (BE_SQLITE_OK != ecdb.ExecuteSql("DELETE FROM " ECDB_CACHETABLE_ClassHasTables))
        return ERROR;

    Utf8String sql;
    sql.Sprintf("INSERT INTO " ECDB_CACHETABLE_ClassHasTables " "
                "SELECT NULL, ec_ClassMap.ClassId, ec_Table.Id FROM ec_PropertyMap "
                "          INNER JOIN ec_Column ON ec_Column.Id = ec_PropertyMap.ColumnId "
                "          INNER JOIN ec_ClassMap ON ec_ClassMap.Id = ec_PropertyMap.ClassMapId "
                "          INNER JOIN ec_Table ON ec_Table.Id = ec_Column.TableId "
                "    WHERE ec_ClassMap.MapStrategy <> %d "
                "          AND ec_ClassMap.MapStrategy <> %d "
                "          AND ec_Column.ColumnKind & %d = 0 "
                "    GROUP BY ec_ClassMap.ClassId, ec_Table.Id;",
                Enum::ToInt(ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable),
                Enum::ToInt(ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable),
                Enum::ToInt(DbColumn::Kind::ECClassId));

    if (BE_SQLITE_OK != ecdb.ExecuteSql(sql.c_str()))
        return ERROR;

    timer.Stop();
    LOG.debugv("Re-populated " ECDB_CACHETABLE_ClassHasTables " in %.4f msecs.", timer.GetElapsedSeconds() * 1000.0);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
DbSchemaPersistenceManager::CreateOrUpdateTableResult DbSchemaPersistenceManager::CreateOrUpdateTable(ECDbCR ecdb, DbTable const& table)
    {
    if (table.GetPersistenceType() == PersistenceType::Virtual || table.GetType() == DbTable::Type::Existing)
        return CreateOrUpdateTableResult::Skipped;

    Utf8CP tableName = table.GetName().c_str();
    BeBriefcaseId briefcaseId = ecdb.GetBriefcaseId();
    const bool allowDbSchemaChange = briefcaseId.IsMasterId() || briefcaseId.IsStandaloneId();

    CreateOrUpdateTableResult mode;
    if (ecdb.TableExists(tableName))
        mode = IsTableChanged(ecdb, table) ? CreateOrUpdateTableResult::Updated : CreateOrUpdateTableResult::WasUpToDate;
    else
        mode = CreateOrUpdateTableResult::Created;

    if (mode == CreateOrUpdateTableResult::WasUpToDate)
        return mode;

    if (!allowDbSchemaChange)
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import ECSchemas: Imported ECSchemas would change the database schema. "
                                                      "This is only allowed for standalone briefcases or the master briefcase. Briefcase id: %" PRIu32, briefcaseId.GetValue());
        return CreateOrUpdateTableResult::Error;
        }

    BentleyStatus stat = SUCCESS;
    if (mode == CreateOrUpdateTableResult::Created)
        stat = CreateTable(ecdb, table);
    else
        {
        BeAssert(mode == CreateOrUpdateTableResult::Updated);
        stat = UpdateTable(ecdb, table);
        }

    return SUCCESS == stat ? mode : CreateOrUpdateTableResult::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::CreateTable(ECDbCR ecdb, DbTable const& table)
    {
    if (!table.IsValid())
        {
        BeAssert(false && "Table definition is not valid");
        return ERROR;
        }

    if (table.GetType() == DbTable::Type::Existing || table.GetPersistenceType() == PersistenceType::Virtual)
        {
        BeAssert(false && "CreateTable must not be called on virtual table or table not owned by ECDb");
        return ERROR;
        }

    std::vector<DbColumn const*> columns;
    table.GetFilteredColumnList(columns, PersistenceType::Persisted);
    if (columns.empty())
        {
        BeAssert(false && "Table have no persisted columns");
        return ERROR;
        }

    Utf8String ddl("CREATE TABLE [");
    ddl.append(table.GetName()).append("](");

    bool isFirstCol = true;
    for (DbColumn const* col : columns)
        {
        if (!isFirstCol)
            ddl.append(", ");

        if (SUCCESS != AppendColumnDdl(ddl, *col))
            return ERROR;

        isFirstCol = false;
        }

    // Append constraints;
    PrimaryKeyDbConstraint const* pkConstraint = table.GetPrimaryKeyConstraint();
    if (pkConstraint != nullptr && pkConstraint->GetColumns().size() > 1)
        {
        //only use a PRIMARY KEY constraint clause for multi-column keys. Single column keys use the column constraint clause (handled above)
        ddl.append(", PRIMARY KEY(");
        AppendColumnNamesToDdl(ddl, pkConstraint->GetColumns());
        ddl.append(")");
        }

    std::vector<DbConstraint const*> tableConstraints = table.GetConstraints();
    for (DbConstraint const* tableConstraint : tableConstraints)
        {
        ddl.append(", ");

        if (tableConstraint->GetType() == DbConstraint::Type::ForeignKey)
            {
            ForeignKeyDbConstraint const* fkConstraint = static_cast<ForeignKeyDbConstraint const*>(tableConstraint);
            if (SUCCESS != AppendForeignKeyDdl(ddl, *fkConstraint))
                return ERROR;
            }
        else
            {
            BeAssert(false && "Unsupported DbConstraint type");
            }
        }
    ddl.append(")");

    if (ecdb.ExecuteSql(ddl.c_str()) != BE_SQLITE_OK)
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to create table %s: %s",
                                                      table.GetName().c_str(), ecdb.GetLastError().c_str());
        return ERROR;
        }

    return CreateTriggers(ecdb, table, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::UpdateTable(ECDbCR ecdb, DbTable const& table)
    {
    if (table.GetType() == DbTable::Type::Existing || table.GetPersistenceType() == PersistenceType::Virtual)
        {
        BeAssert(false && "UpdateTable must not be called on virtual table or table not owned by ECDb");
        return ERROR;
        }

    Utf8CP tableName = table.GetName().c_str();
    DbSchema::EntityType type = DbSchema::GetEntityType(ecdb, tableName);
    if (type == DbSchema::EntityType::None)
        {
        BeAssert(false && "Table is expected to exist already");
        return ERROR;
        }

    //! Object type is view and exist in db. Action = DROP it and recreate it.
    if (type == DbSchema::EntityType::View)
        {
        Utf8String sql("DROP VIEW [");
        sql.append(tableName).append("]");
        auto r = ecdb.ExecuteSql(sql.c_str());
        if (r != BE_SQLITE_OK)
            {
            ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to drop view '%s'", tableName);
            return ERROR;
            }

        return CreateTable(ecdb, table);
        }

    BeAssert(type == DbSchema::EntityType::Table);

    bvector<Utf8String> existingColumnNamesInDb;
    if (!ecdb.GetColumns(existingColumnNamesInDb, tableName))
        {
        BeAssert(false && "Failed to get column list for table");
        return ERROR;
        }

    //Create a fast hash set of existing db column list
    bset<Utf8String, CompareIUtf8Ascii> existingColumnNamesInDbSet;
    for (Utf8StringCR existingDbColumn : existingColumnNamesInDb)
        {
        existingColumnNamesInDbSet.insert(existingDbColumn);
        }

    //Create a fast hash set of in-memory column list;
    std::vector<DbColumn const*> columns;
    table.GetFilteredColumnList(columns, PersistenceType::Persisted);

    std::vector<DbColumn const*> newColumns;
    //compute new columns;
    for (DbColumn const* col : columns)
        {
        if (existingColumnNamesInDbSet.find(col->GetName().c_str()) == existingColumnNamesInDbSet.end())
            newColumns.push_back(col);
        }


    if (SUCCESS != AlterTable(ecdb, table, newColumns))
        return ERROR;

    return CreateTriggers(ecdb, table, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::AlterTable(ECDbCR ecdb, DbTable const& table, std::vector<DbColumn const*> const& columnsToAdd)
    {
    if (columnsToAdd.empty())
        return SUCCESS;

    Utf8String alterDdlTemplate;
    alterDdlTemplate.Sprintf("ALTER TABLE [%s] ADD COLUMN ", table.GetName().c_str());
    for (DbColumn const* columnToAdd : columnsToAdd)
        {
        BeAssert(&table == &columnToAdd->GetTable());
        //Limitation of ADD COLUMN http://www.sqlite.org/lang_altertable.html
        
        Utf8String ddl(alterDdlTemplate);
        if (SUCCESS != AppendColumnDdl(ddl, *columnToAdd))
            return ERROR;

        //append FK constraints, if defined for this column
        for (DbConstraint const* constraint : table.GetConstraints())
            {
            if (constraint->GetType() != DbConstraint::Type::ForeignKey)
                continue;

            ForeignKeyDbConstraint const* fkConstraint = static_cast<ForeignKeyDbConstraint const*> (constraint);
            if (!fkConstraint->IsValid())
                return ERROR;

            if (fkConstraint->GetFkColumns().size() != 1 || fkConstraint->GetFkColumns()[0] != columnToAdd)
                continue;

            if (SUCCESS != AppendForeignKeyToColumnDdl(ddl, *fkConstraint, *columnToAdd))
                return ERROR;
            }

        if (BE_SQLITE_OK != ecdb.ExecuteSql(ddl.c_str()))
            {
            ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to add new column (%s). Error message: %s", ddl.c_str(), ecdb.GetLastError().c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static
bool DbSchemaPersistenceManager::IsTableChanged(ECDbCR ecdb, DbTable const& table)
    {
    bvector<Utf8String> namesOfExistingColumns;
    if (!ecdb.GetColumns(namesOfExistingColumns, table.GetName().c_str()))
        {
        BeAssert(false && "Failed to get column list for table");
        return true;
        }

    //Create a fast hash set of existing db column list
    bset<Utf8String, CompareIUtf8Ascii> namesOfExistingColumnsSet;
    for (Utf8StringCR name : namesOfExistingColumns)
        {
        namesOfExistingColumnsSet.insert(name);
        }

    std::vector<DbColumn const*> persistedColumns;
    for (DbColumn const* col : table.GetColumns())
        {
        if (col->GetPersistenceType() == PersistenceType::Persisted)
            persistedColumns.push_back(col);
        }

    for (DbColumn const* col : persistedColumns)
        {
        if (namesOfExistingColumnsSet.find(col->GetName()) == namesOfExistingColumnsSet.end())
            return true; //new column
        }

    //no columns were added. So difference in columns means that columns was deleted -> which also means that the table changed.
    return persistedColumns.size() != namesOfExistingColumns.size();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle 10/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::BuildCreateIndexDdl(Utf8StringR ddl, Utf8StringR comparableIndexDef, ECDbCR ecdb, DbIndex const& index)
    {
    //this is a string that contains the pieces of the index definition that altogether make the definition unique.
    //that string leaves out index name and keywords common to all indexes (e.g. CREATE, INDEX, WHERE).
    comparableIndexDef.clear();

    ddl.assign("CREATE ");
    if (index.GetIsUnique())
        {
        ddl.append("UNIQUE ");
        comparableIndexDef.assign("u ");
        }

    Utf8CP indexName = index.GetName().c_str();
    Utf8CP tableName = index.GetTable().GetName().c_str();
    Utf8String columnsDdl;
    AppendColumnNamesToDdl(columnsDdl, index.GetColumns());

    ddl.append("INDEX [").append(indexName).append("] ON [").append(tableName).append("](").append(columnsDdl).append(")");
    comparableIndexDef.append(tableName).append("(").append(columnsDdl).append(")");

    Utf8String whereClause;
    if (SUCCESS != GenerateIndexWhereClause(whereClause, ecdb, index))
        return ERROR;

    if (!whereClause.empty())
        {
        ddl.append(" WHERE ").append(whereClause);
        comparableIndexDef.append(whereClause);
        }

    comparableIndexDef.ToLower();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  10/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::GenerateIndexWhereClause(Utf8StringR whereClause, ECDbCR ecdb, DbIndex const& index)
    {
    if (index.IsAddColumnsAreNotNullWhereExp())
        {
        Utf8String notNullWhereExp;
        bool isFirstCol = true;
        for (DbColumn const* indexCol : index.GetColumns())
            {
            if (indexCol->DoNotAllowDbNull())
                continue;

            if (!isFirstCol)
                notNullWhereExp.append(" AND ");

            notNullWhereExp.append("[").append(indexCol->GetName()).append("] IS NOT NULL");
            isFirstCol = false;
            }

        if (!notNullWhereExp.empty())
            whereClause.append("(").append(notNullWhereExp).append(")");
        }

    DbColumn const* classIdCol = nullptr;
    if (!index.HasClassId() || !index.GetTable().TryGetECClassIdColumn(classIdCol))
        return SUCCESS;

    BeAssert(index.HasClassId());

    BeAssert(classIdCol != nullptr);

    ECClassCP ecclass = ecdb.Schemas().GetECClass(index.GetClassId());
    if (ecclass == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    ClassMapCP classMap = ecdb.GetECDbImplR().GetECDbMap().GetClassMap(*ecclass);
    if (classMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    StorageDescription const& storageDescription = classMap->GetStorageDescription();
    if (index.AppliesToSubclassesIfPartial() && storageDescription.HierarchyMapsToMultipleTables() && classMap->GetClass().GetRelationshipClassCP() == nullptr)
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                        "Index %s cannot be created for ECClass '%s' because the ECClass has subclasses in other tables and the index is defined to apply to subclasses.",
                                                        index.GetName().c_str(), ecclass->GetFullName());
        return ERROR;
        }

    Utf8String classIdFilter;
    if (SUCCESS != storageDescription.GenerateECClassIdFilter(classIdFilter, index.GetTable(), *classIdCol, index.AppliesToSubclassesIfPartial()))
        return ERROR;

    if (classIdFilter.empty())
        return SUCCESS;

    //now we know the index would have to be partial.
    //non-unique indexes will never be made partial as they don't enforce anything and usually
    //are just there for performance - which a partial index will spoil.
    if (!index.GetIsUnique())
        return SUCCESS;

    //unique indexes are always created to not lose enforcement of the uniqueness
    const bool needsParens = !whereClause.empty();
    if (needsParens)
        whereClause.append(" AND (");

    whereClause.append(classIdFilter);

    if (needsParens)
        whereClause.append(")");

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::CreateTriggers(ECDbCR ecdb, DbTable const& table, bool failIfExists)
    {
    for (DbTrigger const* trigger : table.GetTriggers())
        {
        if (TriggerExistsInDb(ecdb, *trigger))
            {
            if (failIfExists)
                {
                ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Trigger %s already exists on table %s.", trigger->GetName(), trigger->GetTable().GetName().c_str());
                return ERROR;
                }

            continue;
            }

        Utf8String ddl("CREATE TRIGGER [");
        ddl.append(trigger->GetName()).append("] ");

        switch (trigger->GetType())
            {
                case DbTrigger::Type::After:
                    ddl.append("AFTER");
                    break;
                case DbTrigger::Type::Before:
                    ddl.append("BEFORE");

                default:
                    break;
            }

        ddl.append(" UPDATE ON [").append(trigger->GetTable().GetName()).append("] WHEN ");
        ddl.append(trigger->GetCondition()).append(" ").append(trigger->GetBody());

        if (ecdb.ExecuteSql(ddl.c_str()) != BE_SQLITE_OK)
            {
            ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to create trigger %s on table %s. Error: %s", trigger->GetName(), trigger->GetTable().GetName().c_str(),
                                                          ecdb.GetLastError().c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static
bool DbSchemaPersistenceManager::TriggerExistsInDb(ECDbCR ecdb, DbTrigger const& trigger)
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != ecdb.GetCachedStatement(stmt, "select NULL from sqlite_master WHERE type='trigger' and name=?"))
        {
        BeAssert(false);
        return false;
        }

    stmt->BindText(1, trigger.GetName(), Statement::MakeCopy::No);
    return BE_SQLITE_ROW == stmt->Step();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::AppendColumnDdl(Utf8StringR ddl, DbColumn const& col)
    {
    ddl.append("[").append(col.GetName()).append("] ");
    Utf8CP typeStr = DbColumn::TypeToSql(col.GetType());
    if (typeStr == nullptr)
        return ERROR;

    ddl.append(typeStr);

    if (col.IsOnlyColumnOfPrimaryKeyConstraint())
        {
        ddl.append(" PRIMARY KEY");
        BeAssert(!col.GetConstraints().HasNotNullConstraint() && !col.GetConstraints().HasUniqueConstraint());
        return SUCCESS;
        }

    DbColumn::Constraints const& colConstraint = col.GetConstraints();
    if (colConstraint.HasNotNullConstraint())
        ddl.append(" NOT NULL");

    if (colConstraint.HasUniqueConstraint())
        ddl.append(" UNIQUE");

    if (colConstraint.GetCollation() != DbColumn::Constraints::Collation::Default)
        ddl.append(" COLLATE ").append(DbColumn::Constraints::CollationToSql(colConstraint.GetCollation()));

    if (!colConstraint.GetDefaultValueConstraint().empty())
        ddl.append(" DEFAULT(").append(colConstraint.GetDefaultValueConstraint()).append(")");

    if (!colConstraint.GetCheckConstraint().empty())
        ddl.append(" CHECK('").append(colConstraint.GetCheckConstraint()).append("')");

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
void DbSchemaPersistenceManager::AppendColumnNamesToDdl(Utf8StringR ddl, std::vector<DbColumn const*> const& columns)
    {
    bool isFirstCol = true;
    for (DbColumn const* col : columns)
        {
        if (!isFirstCol)
            ddl.append(", ");

        ddl.append("[").append(col->GetName()).append("]");

        isFirstCol = false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::AppendForeignKeyDdl(Utf8StringR ddl, ForeignKeyDbConstraint const& fkConstraint)
    {
    if (!fkConstraint.IsValid())
        return ERROR;

    ddl.append("FOREIGN KEY(");
    AppendColumnNamesToDdl(ddl, fkConstraint.GetFkColumns());
    ddl.append(")");

    DoAppendForeignKeyDdl(ddl, fkConstraint);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::AppendForeignKeyToColumnDdl(Utf8StringR ddl, ForeignKeyDbConstraint const& fkConstraint, DbColumn const& singleFkColumn)
    {
    if (!fkConstraint.IsValid())
        return ERROR;

    if (fkConstraint.GetFkColumns().size() != 1 || fkConstraint.GetFkColumns()[0] != &singleFkColumn)
        return ERROR;

    DoAppendForeignKeyDdl(ddl, fkConstraint);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
void DbSchemaPersistenceManager::DoAppendForeignKeyDdl(Utf8StringR ddl, ForeignKeyDbConstraint const& fkConstraint)
    {
    ddl.append(" REFERENCES [").append(fkConstraint.GetReferencedTable().GetName()).append("](");
    AppendColumnNamesToDdl(ddl, fkConstraint.GetReferencedTableColumns());
    ddl.append(")");

    if (fkConstraint.GetOnDeleteAction() != ForeignKeyDbConstraint::ActionType::NotSpecified)
        ddl.append(" ON DELETE ").append(ForeignKeyDbConstraint::ActionTypeToSql(fkConstraint.GetOnDeleteAction()));

    if (fkConstraint.GetOnUpdateAction() != ForeignKeyDbConstraint::ActionType::NotSpecified)
        ddl.append(" ON UPDATE ").append(ForeignKeyDbConstraint::ActionTypeToSql(fkConstraint.GetOnUpdateAction()));
    }

//---------------------------------------------------------------------------------------
//! This method is not called from anywhere, because it provides compile-time asserts
//! for all enumerations that ECDb persists. This is a safe-guard to notify us whenever
//! somebody changes the enums. We then need to react to that so that old files don't get hornswaggled
// @bsimethod                                                    Krischan.Eberle  04/2016
//---------------------------------------------------------------------------------------
void AssertPersistedEnumsAreUnchanged()
    {
    static_assert((int) CustomAttributeContainerType::Any == 4095 &&
                  (int) CustomAttributeContainerType::AnyClass == 30 &&
                  (int) CustomAttributeContainerType::AnyProperty == 992 &&
                  (int) CustomAttributeContainerType::AnyRelationshipConstraint == 3072 &&
                  (int) CustomAttributeContainerType::ArrayProperty == 128 &&
                  (int) CustomAttributeContainerType::CustomAttributeClass == 4 &&
                  (int) CustomAttributeContainerType::EntityClass == 2 &&
                  (int) CustomAttributeContainerType::NavigationProperty == 512 &&
                  (int) CustomAttributeContainerType::PrimitiveProperty == 32 &&
                  (int) CustomAttributeContainerType::RelationshipClass == 16 &&
                  (int) CustomAttributeContainerType::Schema == 1 &&
                  (int) CustomAttributeContainerType::SourceRelationshipConstraint == 1024 &&
                  (int) CustomAttributeContainerType::StructProperty == 64 &&
                  (int) CustomAttributeContainerType::StructArrayProperty == 256 &&
                  (int) CustomAttributeContainerType::StructClass == 8 &&
                  (int) CustomAttributeContainerType::TargetRelationshipConstraint == 2048, "Persisted Enum has changed: ECN::CustomAttributeContainerType.");

    static_assert((int) DbColumn::Kind::DataColumn == 512 &&
                  (int) DbColumn::Kind::ECClassId == 2 &&
                  (int) DbColumn::Kind::ECInstanceId == 1 &&
                  (int) DbColumn::Kind::RelECClassId == 2048 &&
                  (int) DbColumn::Kind::SharedDataColumn == 1024 &&
                  (int) DbColumn::Kind::SourceECClassId == 64 &&
                  (int) DbColumn::Kind::SourceECInstanceId == 32 &&
                  (int) DbColumn::Kind::TargetECClassId == 256 &&
                  (int) DbColumn::Kind::TargetECInstanceId == 128 &&
                  (int) DbColumn::Kind::Unknown == 0, "Persisted Enum has changed: DbColumn::Kind.");

    static_assert((int) DbColumn::Type::Any == 0 &&
                  (int) DbColumn::Type::Blob == 2 &&
                  (int) DbColumn::Type::Boolean == 1 &&
                  (int) DbColumn::Type::Integer == 5 &&
                  (int) DbColumn::Type::Real == 4 &&
                  (int) DbColumn::Type::Text == 6 &&
                  (int) DbColumn::Type::TimeStamp == 3, "Persisted Enum has changed: DbColumn::Type.");

    static_assert((int) DbTable::Type::Existing == 2 &&
                  (int) DbTable::Type::Joined == 1 &&
                  (int) DbTable::Type::Primary == 0, "Persisted Enum has changed: DbTable::Type.");

    static_assert((int) ECClassModifier::Abstract == 1 &&
                  (int) ECClassModifier::None == 0 &&
                  (int) ECClassModifier::Sealed == 2, "Persisted Enum has changed: ECN::ECClassModifier.");

    static_assert((int) ECClassType::CustomAttribute == 3 &&
                  (int) ECClassType::Entity == 0 &&
                  (int) ECClassType::Relationship == 1 &&
                  (int) ECClassType::Struct == 2, "Persisted Enum has changed: ECN::ECClassType.");

    static_assert((int) ECDbMapStrategy::Options::JoinedTable == 4 &&
                  (int) ECDbMapStrategy::Options::None == 0 &&
                  (int) ECDbMapStrategy::Options::ParentOfJoinedTable == 2 &&
                  (int) ECDbMapStrategy::Options::SharedColumns == 1, "Persisted Enum has changed: ECDbMapStrategy::Options.");

    static_assert((int) ECDbMapStrategy::Strategy::ExistingTable == 3 &&
                  (int) ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable == 101 &&
                  (int) ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable == 100 &&
                  (int) ECDbMapStrategy::Strategy::NotMapped == 0 &&
                  (int) ECDbMapStrategy::Strategy::OwnTable == 1 &&
                  (int) ECDbMapStrategy::Strategy::SharedTable == 2, "Persisted Enum has changed: ECDbMapStrategy::Strategy.");

    static_assert((int) ECPropertyKind::Navigation == 4 &&
                  (int) ECPropertyKind::Primitive == 0 &&
                  (int) ECPropertyKind::PrimitiveArray == 2 &&
                  (int) ECPropertyKind::Struct == 1 &&
                  (int) ECPropertyKind::StructArray == 3, "Persisted Enum has changed: ECPropertyKind.");

    static_assert((int) StrengthType::Embedding == 2 &&
                  (int) StrengthType::Holding == 1 &&
                  (int) StrengthType::Referencing == 0, "Persisted Enum has changed: ECN::StrengthType.");

    static_assert((int) ECRelatedInstanceDirection::Backward == 2 &&
                  (int) ECRelatedInstanceDirection::Forward == 1, "Persisted Enum has changed: ECN::ECRelatedInstanceDirection.");

    static_assert((int) ECRelationshipEnd::ECRelationshipEnd_Source == 0 &&
                  (int) ECRelationshipEnd::ECRelationshipEnd_Target == 1, "Persisted Enum has changed: ECN::ECRelationshipEnd.");

    static_assert((int) ForeignKeyDbConstraint::ActionType::Cascade == 1 &&
                  (int) ForeignKeyDbConstraint::ActionType::NoAction == 2 &&
                  (int) ForeignKeyDbConstraint::ActionType::NotSpecified == 0 &&
                  (int) ForeignKeyDbConstraint::ActionType::Restrict == 5 &&
                  (int) ForeignKeyDbConstraint::ActionType::SetDefault == 4 &&
                  (int) ForeignKeyDbConstraint::ActionType::SetNull == 3, "Persisted Enum has changed: ForeignKeyDbConstraint::ActionType.");

    static_assert((int) ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class == 30 &&
                  (int) ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Property == 992 &&
                  (int) ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Schema == 1 &&
                  (int) ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::SourceRelationshipConstraint == 1024 &&
                  (int) ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::TargetRelationshipConstraint == 2048, "Persisted Enum has changed: ECDbSchemaPersistenceHelper::GeneralizedCustomAttributeContainerType.");

    static_assert((int) PrimitiveType::PRIMITIVETYPE_Binary == 0x101 &&
                  (int) PrimitiveType::PRIMITIVETYPE_Boolean == 0x201 &&
                  (int) PrimitiveType::PRIMITIVETYPE_DateTime == 0x301 &&
                  (int) PrimitiveType::PRIMITIVETYPE_Double == 0x401 &&
                  (int) PrimitiveType::PRIMITIVETYPE_IGeometry == 0xa01 &&
                  (int) PrimitiveType::PRIMITIVETYPE_Integer == 0x501 &&
                  (int) PrimitiveType::PRIMITIVETYPE_Long == 0x601 &&
                  (int) PrimitiveType::PRIMITIVETYPE_Point2D == 0x701 &&
                  (int) PrimitiveType::PRIMITIVETYPE_Point3D == 0x801 &&
                  (int) PrimitiveType::PRIMITIVETYPE_String == 0x901, "Persisted Enum has changed: ECN::PrimitiveType.");
    }

END_BENTLEY_SQLITE_EC_NAMESPACE