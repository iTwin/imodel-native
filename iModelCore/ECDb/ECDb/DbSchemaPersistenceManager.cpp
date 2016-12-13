/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbSchemaPersistenceManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SqlNames.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        08/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::RepopulateClassHierarchyCacheTable(ECDbCR ecdb)
    {
    StopWatch timer(true);
    if (BE_SQLITE_OK != ecdb.ExecuteSql("DELETE FROM " TABLE_ClassHierarchyCache))
        return ERROR;

    if (BE_SQLITE_OK != ecdb.ExecuteSql(
                    "WITH RECURSIVE "
                     "  BaseClassList(ClassId, BaseClassId, Level, Ordinal) AS "
                     "  ("
                     "  SELECT Id, Id, 1, 0 FROM ec_Class "
                     "  UNION "
                     "  SELECT DCL.ClassId, BC.BaseClassId, DCL.Level + 1, COALESCE(NULLIF(BC.Ordinal, 0), DCL.Ordinal) "
                     "  FROM BaseClassList DCL INNER JOIN ec_ClassHasBaseClasses BC ON BC.ClassId = DCL.BaseClassId "
                     "  )"
                     "INSERT INTO " TABLE_ClassHierarchyCache " "
                     "SELECT DISTINCT NULL Id, ClassId, BaseClassId FROM BaseClassList ORDER BY Ordinal DESC, Level DESC;"))
        {
        return ERROR;
        }

    //Old SQL in case needed until LWC issue is fixed:
    /*
    "WITH RECURSIVE "
    "BaseClassList(ClassId, BaseClassId) AS "
    "("
    "   SELECT Id, Id FROM ec_Class"
    "   UNION"
    "   SELECT DCL.ClassId, BC.BaseClassId FROM BaseClassList DCL"
    "       INNER JOIN ec_ClassHasBaseClasses BC ON BC.ClassId = DCL.BaseClassId"
    ")"
    "INSERT INTO " ECDB_CACHETABLE_ClassHierarchy " SELECT NULL Id, ClassId, BaseClassId FROM BaseClassList"))*/

    timer.Stop();
    LOG.debugv("Re-populated table '" TABLE_ClassHierarchyCache "' in %.4f msecs.", timer.GetElapsedSeconds() * 1000.0);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        08/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::RepopulateClassHasTableCacheTable(ECDbCR ecdb)
    {
    StopWatch timer(true);
    if (BE_SQLITE_OK != ecdb.ExecuteSql("DELETE FROM " TABLE_ClassHasTablesCache))
        return ERROR;

    if (BE_SQLITE_OK != ecdb.ExecuteSql("INSERT INTO " TABLE_ClassHasTablesCache " "
                                        "SELECT NULL, ec_ClassMap.ClassId, ec_Table.Id FROM ec_PropertyMap "
                                        "          INNER JOIN ec_Column ON ec_Column.Id = ec_PropertyMap.ColumnId "
                                        "          INNER JOIN ec_ClassMap ON ec_ClassMap.ClassId = ec_PropertyMap.ClassId "
                                        "          INNER JOIN ec_Table ON ec_Table.Id = ec_Column.TableId "
                                        "    WHERE ec_ClassMap.MapStrategy <>  " SQLVAL_MapStrategy_ForeignKeyRelationshipInSourceTable
                                        "          AND ec_ClassMap.MapStrategy <>  " SQLVAL_MapStrategy_ForeignKeyRelationshipInTargetTable
                                        "          AND ec_Column.ColumnKind & " SQLVAL_DbColumn_Kind_ECClassId " = 0 "
                                        "    GROUP BY ec_ClassMap.ClassId, ec_Table.Id"))
        return ERROR;

    timer.Stop();
    LOG.debugv("Re-populated " TABLE_ClassHasTablesCache " in %.4f msecs.", timer.GetElapsedSeconds() * 1000.0);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
//static
DbSchemaPersistenceManager::CreateOrUpdateTableResult DbSchemaPersistenceManager::CreateOrUpdateTable(ECDbCR ecdb, DbTable const& table, DbSchemaModificationToken const* mayModifyDbSchemaToken)
    {
    if (table.GetPersistenceType() == PersistenceType::Virtual || table.GetType() == DbTable::Type::Existing)
        return CreateOrUpdateTableResult::Skipped;

    Utf8CP tableName = table.GetName().c_str();

    CreateOrUpdateTableResult mode;
    if (ecdb.TableExists(tableName))
        mode = IsTableChanged(ecdb, table) ? CreateOrUpdateTableResult::Updated : CreateOrUpdateTableResult::WasUpToDate;
    else
        mode = CreateOrUpdateTableResult::Created;

    if (mode == CreateOrUpdateTableResult::WasUpToDate)
        return mode;

    ECDbPolicy policy = ECDbPolicyManager::GetPolicy(MayModifyDbSchemaPolicyAssertion(ecdb, mayModifyDbSchemaToken));
    if (!policy.IsSupported())
        {
        //until we can enforce this, we just issue a warning, so that people can fix their ECSchemas
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Warning,
                                                      "DB-schema modifying ECSchema import: %s table '%s'.",
                                                      mode == CreateOrUpdateTableResult::Created ? "created" : "modified", table.GetName().c_str());
        /*ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, 
               "Failed to import ECSchemas: Imported ECSchemas would change the database schema. ECDb would have to %s table '%s'.",
                      mode == CreateOrUpdateTableResult::Created ? "create" : "modify", table.GetName().c_str());
        return CreateOrUpdateTableResult::Error;
        */
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
    auto buildECClassIdFilter = [] (Utf8StringR filterSqlExpression, StorageDescription const& desc, DbTable const& table, DbColumn const& classIdColumn, bool polymorphic)
        {
        if (table.GetPersistenceType() != PersistenceType::Persisted)
            return SUCCESS; //table is virtual -> noop

        Partition const* partition = desc.GetPartition(table);
        if (partition == nullptr)
            {
            BeAssert(false && "Should always find a partition for the given table");
            return ERROR;
            }

        Utf8String classIdColSql;
        classIdColSql.append(classIdColumn.GetName());
        Utf8Char classIdStr[ECClassId::ID_STRINGBUFFER_LENGTH];
        desc.GetClassId().ToString(classIdStr);

        if (!polymorphic)
            {
            //if partition's table is only used by a single class, no filter needed     
            if (partition->IsSharedTable())
                {
                filterSqlExpression.append(classIdColSql).append("=").append(classIdStr);
                }

            return SUCCESS;
            }

        partition->AppendECClassIdFilterSql(filterSqlExpression, classIdColSql.c_str());
        return SUCCESS;
        };

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

    DbColumn const& classIdCol = index.GetTable().GetECClassIdColumn();
    if (!index.HasClassId() || classIdCol.GetPersistenceType() == PersistenceType::Virtual)
        return SUCCESS;

    BeAssert(index.HasClassId());

    ECClassCP ecclass = ecdb.Schemas().GetECClass(index.GetClassId());
    if (ecclass == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    ClassMapCP classMap = ecdb.Schemas().GetDbMap().GetClassMap(*ecclass);
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
    if (SUCCESS != buildECClassIdFilter(classIdFilter, storageDescription, index.GetTable(), classIdCol, index.AppliesToSubclassesIfPartial()))
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

    if (colConstraint.GetCollation() != DbColumn::Constraints::Collation::Unset)
        ddl.append(" ").append(DbColumn::Constraints::CollationToSql(colConstraint.GetCollation()));

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


END_BENTLEY_SQLITE_EC_NAMESPACE