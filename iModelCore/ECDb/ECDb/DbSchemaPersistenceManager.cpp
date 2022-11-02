/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::RepopulateClassHierarchyCacheTable(ECDbCR ecdb)
    {
    PERFLOG_START("ECDb", "Repopulate table " TABLE_ClassHierarchyCache);
    if (BE_SQLITE_OK != ecdb.ExecuteSql("DELETE FROM main." TABLE_ClassHierarchyCache))
        return ERROR;

    if (BE_SQLITE_OK != ecdb.ExecuteSql(
                    "WITH RECURSIVE "
                     "  BaseClassList(ClassId, BaseClassId, Level, Ordinal) AS "
                     "  ("
                     "  SELECT Id, Id, 1, 0 FROM main." TABLE_Class
                     "  UNION "
                     "  SELECT DCL.ClassId, BC.BaseClassId, DCL.Level + 1, COALESCE(NULLIF(BC.Ordinal, 0), DCL.Ordinal) "
                     "  FROM BaseClassList DCL INNER JOIN main. " TABLE_ClassHasBaseClasses " BC ON BC.ClassId = DCL.BaseClassId "
                     "  )"
                     "INSERT INTO main." TABLE_ClassHierarchyCache " "
                     "SELECT DISTINCT NULL Id, ClassId, BaseClassId FROM BaseClassList ORDER BY Ordinal DESC, Level DESC;"))
        {
        return ERROR;
        }

    PERFLOG_FINISH("ECDb", "Repopulate table " TABLE_ClassHierarchyCache);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::RepopulateClassHasTableCacheTable(ECDbCR ecdb)
    {
    PERFLOG_START("ECDb", "Repopulate table " TABLE_ClassHasTablesCache);
    if (BE_SQLITE_OK != ecdb.ExecuteSql("DELETE FROM main." TABLE_ClassHasTablesCache))
        return ERROR;

    if (BE_SQLITE_OK != ecdb.ExecuteSql("INSERT INTO main." TABLE_ClassHasTablesCache " "
                                        "SELECT NULL,cm.ClassId,t.Id FROM main." TABLE_PropertyMap " pm"
                                        " INNER JOIN main." TABLE_Column " c ON c.Id=pm.ColumnId"
                                        " INNER JOIN main." TABLE_ClassMap " cm ON cm.ClassId=pm.ClassId"
                                        " INNER JOIN main." TABLE_Table " t ON t.Id=c.TableId"
                                        " WHERE c.ColumnKind & " SQLVAL_DbColumn_Kind_ECClassId "=0 AND "
                                        "   cm.MapStrategy NOT IN(" SQLVAL_MapStrategy_ForeignKeyRelationshipInSourceTable "," SQLVAL_MapStrategy_ForeignKeyRelationshipInTargetTable ")"
                                        " GROUP BY cm.ClassId, t.Id"))
        return ERROR;

    PERFLOG_FINISH("ECDb", "Repopulate table " TABLE_ClassHasTablesCache);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
DbSchemaPersistenceManager::CreateOrUpdateTableResult DbSchemaPersistenceManager::CreateOrUpdateTable(ECDbCR ecdb, DbTable const& table)
    {
    if (table.GetType() == DbTable::Type::Virtual || table.GetType() == DbTable::Type::Existing)
        return CreateOrUpdateTableResult::Skipped;

    CreateOrUpdateTableResult mode;
    if (DbUtilities::TableExists(ecdb, table.GetName().c_str()))
        mode = IsTableChanged(ecdb, table) ? CreateOrUpdateTableResult::Updated : CreateOrUpdateTableResult::WasUpToDate;
    else
        mode = CreateOrUpdateTableResult::Created;

    if (mode == CreateOrUpdateTableResult::WasUpToDate)
        return mode;

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
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::CreateTable(ECDbCR ecdb, DbTable const& table)
    {
    if (!table.IsValid())
        {
        BeAssert(false && "Table definition is not valid");
        return ERROR;
        }

    if (table.GetType() == DbTable::Type::Existing || table.GetType() == DbTable::Type::Virtual)
        {
        BeAssert(false && "CreateTable must not be called on virtual table or table not owned by ECDb");
        return ERROR;
        }

    Utf8String ddl("CREATE TABLE ");
    ddl.append("[").append(table.GetName()).append("](");

    bool isFirstCol = true;
    for (DbColumn const* col : table.GetColumns())
        {
        if (col->GetPersistenceType() == PersistenceType::Virtual)
            continue;

        if (!isFirstCol)
            ddl.append(", ");

        if (SUCCESS != AppendColumnDdl(ddl, *col))
            return ERROR;

        isFirstCol = false;
        }

    if (isFirstCol)
        {
        BeAssert(false && "Table doesn't have any columns");
        return ERROR;
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

    if (ecdb.ExecuteDdl(ddl.c_str()) != BE_SQLITE_OK)
        {
        ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to create table %s: %s [%s]", table.GetName().c_str(), ecdb.GetLastError().c_str(), ddl.c_str());
        return ERROR;
        }

    return CreateTriggers(ecdb, table, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::UpdateTable(ECDbCR ecdb, DbTable const& table)
    {
    if (table.GetType() == DbTable::Type::Existing || table.GetType() == DbTable::Type::Virtual)
        {
        BeAssert(false && "UpdateTable must not be called on virtual table or table not owned by ECDb");
        return ERROR;
        }

    bvector<Utf8String> existingColumnNamesInDb;
    if (!ecdb.GetColumns(existingColumnNamesInDb, table.GetName().c_str()))
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
    const std::vector<DbColumn const*> columns = table.FindAll(PersistenceType::Physical);

    std::vector<DbColumn const*> newColumns;
    //compute new columns;
    for (DbColumn const* col : columns)
        {
        if (existingColumnNamesInDbSet.find(col->GetName()) == existingColumnNamesInDbSet.end())
            newColumns.push_back(col);
        }


    if (SUCCESS != AlterTable(ecdb, table, newColumns))
        return ERROR;

    return CreateTriggers(ecdb, table, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
        
        if (columnToAdd->IsOnlyColumnOfPrimaryKeyConstraint())
            {
            ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to add column %s as primary column to table %s.", columnToAdd->GetName().c_str(),
                                            table.GetName().c_str());
            return ERROR;
            }

        Utf8String ddl(alterDdlTemplate);
        if (SUCCESS != AppendColumnDdl(ddl, *columnToAdd))
            return ERROR;

        //append FK constraints, if defined for this column
        for (DbConstraint const* constraint : table.GetConstraints())
            {
            if (constraint->GetType() == DbConstraint::Type::PrimaryKey)
                {
                ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to add column (%s) as primary column for an existing table.", ddl.c_str());
                return ERROR;
                }

            BeAssert(constraint->GetType() == DbConstraint::Type::ForeignKey);
            ForeignKeyDbConstraint const* fkConstraint = static_cast<ForeignKeyDbConstraint const*> (constraint);
            if (!fkConstraint->IsValid())
                return ERROR;

            if (fkConstraint->GetFkColumns().size() != 1 || fkConstraint->GetFkColumns()[0] != columnToAdd)
                continue;
            
            if (SUCCESS != AppendForeignKeyToColumnDdl(ddl, *fkConstraint, *columnToAdd))
                return ERROR;
            }

        if (BE_SQLITE_OK != ecdb.ExecuteDdl(ddl.c_str()))
            {
            ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to add new column %s. Error message: %s", ddl.c_str(), ecdb.GetLastError().c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
        if (col->GetPersistenceType() == PersistenceType::Physical)
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
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::CreateIndex(ECDbCR ecdb, DbIndex const& index, Utf8StringCR ddl)
    {
    if (index.GetTable().GetType() == DbTable::Type::Virtual)
        {
        BeAssert(false && "Must not call this method for indexes on virtual tables");
        return ERROR;
        }

    if (BE_SQLITE_OK != ecdb.ExecuteDdl(ddl.c_str()))
        {
        ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to create index %s on table %s. Error: %s [%s]", index.GetName().c_str(), index.GetTable().GetName().c_str(),
                                       ecdb.GetLastError().c_str(), ddl.c_str());

        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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

    Utf8StringCR tableName = index.GetTable().GetName();
    Utf8String columnsDdl;
    AppendColumnNamesToDdl(columnsDdl, index.GetColumns());

    ddl.append("INDEX [").append(index.GetName()).append("] ON [").append(tableName).append("](").append(columnsDdl).append(")");
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
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::GenerateIndexWhereClause(Utf8StringR whereClause, ECDbCR ecdb, DbIndex const& index)
    {
    auto buildECClassIdFilter = [] (Utf8StringR filterSqlExpression, StorageDescription const& desc, DbTable const& table, DbColumn const& classIdColumn, bool polymorphic)
        {
        if (table.GetType() == DbTable::Type::Virtual)
            return SUCCESS; //-> noop

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

    // implicit indexes should be able to create unique index if classid is included
    if (index.IsAutoGenerated() && index.GetIsUnique())
        {
        const auto& cols = index.GetColumns();
        const bool indexIncludeECClassId = std::find_if(cols.begin(), cols.end(), [] (DbColumn const* c)
            {
            return c->GetKind() == DbColumn::Kind::ECClassId;
            }) != cols.end() && cols.size() > 1;

        if (indexIncludeECClassId)
            return SUCCESS;
    }
    BeAssert(index.HasClassId());

    ECClassCP ecclass = ecdb.Schemas().GetClass(index.GetClassId());
    if (ecclass == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    //during schema import always use MainDbMap
    ClassMap const* classMap = ecdb.Schemas().Main().GetClassMap(*ecclass);
    if (classMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    StorageDescription const& storageDescription = classMap->GetStorageDescription();
    if (index.AppliesToSubclassesIfPartial() && storageDescription.HasMultipleNonVirtualHorizontalPartitions() && classMap->GetClass().GetRelationshipClassCP() == nullptr)
        {
        ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Index %s cannot be created for ECClass '%s' because the ECClass has subclasses in other tables and the index is defined to apply to subclasses.",
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
    if (index.IsAutoGenerated() && !index.GetIsUnique())
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
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::CreateTriggers(ECDbCR ecdb, DbTable const& table, bool failIfExists)
    {
    for (DbTrigger const* trigger : table.GetTriggers())
        {
        if (TriggerExists(ecdb, *trigger))
            {
            if (failIfExists)
                {
                ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Trigger %s already exists on table %s.", trigger->GetName().c_str(), trigger->GetTable().GetName().c_str());
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
            ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to create trigger %s on table %s. Error: %s", trigger->GetName().c_str(), trigger->GetTable().GetName().c_str(),
                                                          ecdb.GetLastError().c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
bool DbSchemaPersistenceManager::TriggerExists(ECDbCR ecdb, DbTrigger const& trigger)
    {
    CachedStatementPtr stmt = ecdb.GetImpl().GetCachedSqliteStatement("select NULL from main.sqlite_master WHERE type='trigger' and name=?");
    if (stmt == nullptr)
        {
        BeAssert(false);
        return false;
        }

    stmt->BindText(1, trigger.GetName(), Statement::MakeCopy::No);
    return BE_SQLITE_ROW == stmt->Step();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbSchemaPersistenceManager::RunPragmaTableInfo(std::vector<SqliteColumnInfo>& colInfos, ECDbCR ecdb, Utf8StringCR tableName, Utf8CP tableSpace)
    {
    if (Utf8String::IsNullOrEmpty(tableSpace))
        {
        std::vector<Utf8String> tableSpaces;
        if (SUCCESS != DbUtilities::GetTableSpaces(tableSpaces, ecdb))
            return ERROR;

        for (Utf8StringCR existingTableSpace : tableSpaces)
            {
            if (SUCCESS != RunPragmaTableInfo(colInfos, ecdb, tableName, existingTableSpace.c_str()))
                return ERROR;

            if (!colInfos.empty()) // table found, no need to continue in other table spaces
                return SUCCESS;
            }

        return SUCCESS;
        }

    colInfos.clear();

    CachedStatementPtr stmt = ecdb.GetImpl().GetCachedSqliteStatement(Utf8PrintfString("PRAGMA [%s].table_info('%s')", tableSpace, tableName.c_str()).c_str());
    if (stmt == nullptr)
        return ERROR;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        BeAssert(BeStringUtilities::StricmpAscii(stmt->GetColumnName(1), "name") == 0);
        Utf8String colName(stmt->GetValueText(1));

        BeAssert(BeStringUtilities::StricmpAscii(stmt->GetColumnName(2), "type") == 0);
        Utf8String colTypeName;
        if (!stmt->IsColumnNull(2))
            colTypeName.assign(stmt->GetValueText(2));

        DbColumn::Type colType = DbColumn::Type::Any;
        if (!colTypeName.empty())
            {
            if (colTypeName.ContainsI("long") ||
                colTypeName.ContainsI("int"))
                colType = DbColumn::Type::Integer;
            else if (colTypeName.ContainsI("char") ||
                     colTypeName.ContainsI("clob") ||
                     colTypeName.ContainsI("text"))
                colType = DbColumn::Type::Text;
            else if (colTypeName.ContainsI("blob") ||
                     colTypeName.ContainsI("binary"))
                colType = DbColumn::Type::Blob;
            else if (colTypeName.ContainsI("real") ||
                     colTypeName.ContainsI("floa") ||
                     colTypeName.ContainsI("doub"))
                colType = DbColumn::Type::Real;
            else if (colTypeName.ContainsI("date") ||
                     colTypeName.ContainsI("timestamp"))
                colType = DbColumn::Type::TimeStamp;
            else if (colTypeName.ContainsI("bool"))
                colType = DbColumn::Type::Boolean;
            }

        BeAssert(BeStringUtilities::StricmpAscii(stmt->GetColumnName(3), "notnull") == 0);
        const bool colIsNotNull = stmt->GetValueBoolean(3);

        BeAssert(BeStringUtilities::StricmpAscii(stmt->GetColumnName(4), "dflt_value") == 0);
        Utf8String colDefaultValue;
        if (!stmt->IsColumnNull(4))
            colDefaultValue.assign(stmt->GetValueText(4));

        BeAssert(BeStringUtilities::StricmpAscii(stmt->GetColumnName(5), "pk") == 0);
        const int pkOrdinal = stmt->GetValueInt(5); //PK column ordinals returned by this pragma are 1-based as 0 indicates "not a PK col"

        colInfos.push_back(SqliteColumnInfo(colName, colType, pkOrdinal, colIsNotNull, colDefaultValue));
        }

    return SUCCESS;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE