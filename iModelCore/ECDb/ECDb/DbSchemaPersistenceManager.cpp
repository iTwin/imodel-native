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
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
bool DbMapSaveContext::IsAlreadySaved(ClassMapCR classMap) const
    {
    return m_savedClassMaps.find(classMap.GetClass().GetId()) != m_savedClassMaps.end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
void DbMapSaveContext::BeginSaving(ClassMapCR classMap)
    {
    if (IsAlreadySaved(classMap))
        return;

    m_savedClassMaps[classMap.GetClass().GetId()] = &classMap;
    m_editStack.push(&classMap);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
void DbMapSaveContext::EndSaving(ClassMapCR classMap)
    {
    if (m_editStack.top() == &classMap)
        {
        m_editStack.pop();
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
DbClassMapSaveContext::DbClassMapSaveContext(DbMapSaveContext& ctx)
    :m_classMapContext(ctx), m_classMap(*ctx.GetCurrent())
    {
    BeAssert(ctx.GetCurrent() != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbClassMapSaveContext::InsertPropertyMap(ECPropertyId rootPropertyId, Utf8CP accessString, DbColumnId columnId)
    {
    PropertyPathId propertyPathId;
    if (m_classMapContext.TryGetPropertyPathId(propertyPathId, rootPropertyId, accessString, true) != SUCCESS)
        return ERROR;
    ECDbCR ecdb = GetMapSaveContext().GetECDb();
    CachedStatementPtr stmt = ecdb.GetCachedStatement("INSERT INTO ec_PropertyMap(ClassMapId, PropertyPathId, ColumnId) VALUES (?,?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }

    stmt->BindId(1, m_classMap.GetId());
    stmt->BindId(2, propertyPathId);
    stmt->BindId(3, columnId);
    if (stmt->Step() != BE_SQLITE_DONE)
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbMapSaveContext::InsertClassMap(ClassMapId& classMapId, ECClassId classId, ECDbMapStrategy const& mapStrategy, ClassMapId baseClassMapId)
    {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("INSERT INTO ec_ClassMap(Id, ParentId, ClassId, MapStrategy, MapStrategyOptions, MapStrategyMinSharedColumnCount, MapStrategyAppliesToSubclasses) VALUES (?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }

    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetClassMapIdSequence().GetNextValue(classMapId))
        {
        BeAssert(false);
        return ERROR;
        }

    stmt->BindId(1, classMapId);
    if (!baseClassMapId.IsValid())
        stmt->BindNull(2);
    else
        stmt->BindId(2, baseClassMapId);

    stmt->BindId(3, classId);
    stmt->BindInt(4, Enum::ToInt(mapStrategy.GetStrategy()));
    stmt->BindInt(5, Enum::ToInt(mapStrategy.GetOptions()));
    const int minSharedColCount = mapStrategy.GetMinimumSharedColumnCount();
    if (minSharedColCount != ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT)
        stmt->BindInt(6, minSharedColCount);

    stmt->BindInt(7, mapStrategy.AppliesToSubclasses() ? 1 : 0);

    const DbResult stat = stmt->Step();
    if (stat != BE_SQLITE_DONE)
        {
        BeAssert(false && "Failed to save classmap");
        return ERROR;
        }

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbMapSaveContext::TryGetPropertyPathId(PropertyPathId& id, ECN::ECPropertyId rootPropertyid, Utf8CP accessString, bool addIfDoesNotExist)
    {
    auto stmt = m_ecdb.GetCachedStatement("SELECT Id FROM ec_PropertyPath  WHERE RootPropertyId =? AND AccessString = ?");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to prepare statement");
        return ERROR;
        }
    stmt->BindId(1, rootPropertyid);
    stmt->BindText(2, accessString, Statement::MakeCopy::No);

    if (stmt->Step() == BE_SQLITE_ROW)
        {
        id = stmt->GetValueId<PropertyPathId>(0);
        return SUCCESS;
        }

    if (!addIfDoesNotExist)
        return ERROR;

    if (m_ecdb.GetECDbImplR().GetPropertyPathIdSequence().GetNextValue(id) != BE_SQLITE_OK)
        {
        BeAssert(false);
        return ERROR;
        }

    stmt = m_ecdb.GetCachedStatement("INSERT INTO ec_PropertyPath(Id, RootPropertyId, AccessString) VALUES(?,?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to prepare statement");
        return ERROR;
        }
    stmt->BindId(1, id);
    stmt->BindId(2, rootPropertyid);
    stmt->BindText(3, accessString, Statement::MakeCopy::No);
    if (stmt->Step() != BE_SQLITE_DONE)
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
std::vector<DbColumn const*> const* DbClassMapLoadContext::FindColumnByAccessString(Utf8CP accessString) const
    {
    auto itor = m_columnByAccessString.find(accessString);
    if (itor != m_columnByAccessString.end())
        return &(itor->second);

    return nullptr;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbClassMapLoadContext::ReadPropertyMaps(DbClassMapLoadContext& ctx, ECDbCR ecdb)
    {
    if (!ctx.GetClassMapId().IsValid())
        return ERROR;

    ctx.m_columnByAccessString.clear();
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT  T.Name TableName, C.Name ColumnName , A.AccessString"
                                                      " FROM ec_PropertyMap P"
                                                      "     INNER JOIN ec_Column C ON C.Id = P.ColumnId"
                                                      "     INNER JOIN ec_Table T ON T.Id = C.TableId"
                                                      "     INNER JOIN ec_PropertyPath A ON A.Id = P.PropertyPathId"
                                                      " WHERE P.ClassMapId = ? ORDER BY T.Name");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }

    stmt->BindId(1, ctx.GetClassMapId());
    DbSchema const& dbSchema = ecdb.GetECDbImplR().GetECDbMap().GetDbSchema();

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        Utf8CP  tableName = stmt->GetValueText(0);
        Utf8CP  columName = stmt->GetValueText(1);
        Utf8CP  accessString = stmt->GetValueText(2);

        DbTable const* table = dbSchema.FindTable(tableName);
        if (table == nullptr)
            return ERROR;

        DbColumn const* column = table->FindColumn(columName);
        if (column == nullptr)
            return ERROR;

        ctx.m_columnByAccessString[accessString].push_back(column);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbClassMapLoadContext::SetBaseClassMap(ClassMapCR classMap)
    {
    if (classMap.GetClass().GetId() != m_baseClassId)
        {
        BeAssert(false);
        return ERROR;
        }

    m_baseClassMap = &classMap;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbClassMapLoadContext::Load(DbClassMapLoadContext& loadContext, ECDbCR ecdb, ECN::ECClassId classId)
    {
    loadContext.m_isValid = false;
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT A.Id, b.ClassId, A.MapStrategy, A.MapStrategyOptions, A.MapStrategyMinSharedColumnCount, A.MapStrategyAppliesToSubclasses FROM ec_ClassMap A LEFT JOIN  ec_ClassMap B ON B.Id = A.ParentId  WHERE A.ClassId = ?");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }
    stmt->BindId(1, classId);
    if (stmt->Step() != BE_SQLITE_ROW)
        return ERROR;

    loadContext.m_baseClassMap = nullptr;
    loadContext.m_classMapId = stmt->GetValueId<ClassMapId>(0);
    loadContext.m_baseClassId = stmt->IsColumnNull(1) ? ECN::ECClassId() : stmt->GetValueId<ECN::ECClassId>(1);
    const int minSharedColCount = stmt->IsColumnNull(4) ? ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT : stmt->GetValueInt(5);
    if (SUCCESS != loadContext.m_mapStrategy.Assign(Enum::FromInt<ECDbMapStrategy::Strategy>(stmt->GetValueInt(2)),
                                                    Enum::FromInt<ECDbMapStrategy::Options>(stmt->GetValueInt(3)),
                                                    minSharedColCount,
                                                    stmt->GetValueInt(5) == 1))
        {
        BeAssert(false && "Found invalid persistence values for ECDbMapStrategy");
        return ERROR;
        }

    if (DbClassMapLoadContext::ReadPropertyMaps(loadContext, ecdb) != SUCCESS)
        return ERROR;

    loadContext.m_isValid = true;
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
    std::vector<DbConstraint const*> tableConstraints = table.GetConstraints();
    for (DbConstraint const* tableConstraint : tableConstraints)
        {
        ddl.append(", ");

        if (tableConstraint->GetType() == DbConstraint::Type::PrimaryKey)
            {
            PrimaryKeyDbConstraint const* pkConstraint = static_cast<PrimaryKeyDbConstraint const*>(tableConstraint);
            if (pkConstraint->GetColumns().empty())
                {
                BeAssert(false && "PK constraint must at least have one column");
                return ERROR;
                }

            ddl.append("PRIMARY KEY(");
            AppendColumnNamesToDdl(ddl, pkConstraint->GetColumns());
            ddl.append(")");
            }
        else if (tableConstraint->GetType() == DbConstraint::Type::ForeignKey)
            {
            ForeignKeyDbConstraint const* fkConstraint = static_cast<ForeignKeyDbConstraint const*>(tableConstraint);
            if (SUCCESS != AppendForeignKeyDdl(ddl, *fkConstraint))
                return ERROR;
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

    for (DbColumn const* col : table.GetColumns())
        {
        if (namesOfExistingColumnsSet.find(col->GetName()) == namesOfExistingColumnsSet.end())
            return true; //new column
        }

    //no columns were added. So difference in columns means that columns was deleted -> which also means that the table changed.
    return table.GetColumns().size() != namesOfExistingColumns.size();
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
            if (indexCol->GetConstraint().IsNotNull())
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

    DbColumn::Constraint const& colConstraint = col.GetConstraint();
    if (colConstraint.IsNotNull())
        ddl.append(" NOT NULL");

    if (colConstraint.IsUnique())
        ddl.append(" UNIQUE");

    if (colConstraint.GetCollation() != DbColumn::Constraint::Collation::Default)
        ddl.append(" COLLATE ").append(DbColumn::Constraint::CollationToSql(colConstraint.GetCollation()));

    if (!colConstraint.GetDefaultExpression().empty())
        ddl.append(" DEFAULT(").append(colConstraint.GetDefaultExpression()).append(")");

    if (!colConstraint.GetCheckExpression().empty())
        ddl.append(" CHECK('").append(colConstraint.GetCheckExpression()).append("')");

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
                  (int) DbColumn::Kind::NonRelSystemColumn == 3 &&
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