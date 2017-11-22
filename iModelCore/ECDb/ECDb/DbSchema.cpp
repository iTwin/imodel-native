/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbSchema.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SqlNames.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//****************************************************************************************
//DbSchema
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbTable const* DbSchema::FindTable(Utf8StringCR name) const
    {
    DbTable const* table = m_tables.Get(name);
    if (table != nullptr)
        return table;

    return LoadTable(name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbTable const* DbSchema::FindTable(DbTableId id) const
    {
    if (!id.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    DbTable const* table = m_tables.Get(id);
    if (table != nullptr)
        return table;

    return LoadTable(id);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbTable* DbSchema::FindTableP(Utf8StringCR name) const
    {
    DbTable const* table = m_tables.Get(name);
    if (table != nullptr)
        return const_cast<DbTable*>(table);

    return LoadTable(name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbTable* DbSchema::AddTable(DbTableSpace const& dbNamespace, Utf8StringCR name, DbTable::Type tableType, ECClassId exclusiveRootClassId)
    {
    return m_tables.Add(DbTableId(), dbNamespace, name, tableType, exclusiveRootClassId, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbTable* DbSchema::AddTable(DbTableSpace const& dbNamespace, Utf8StringCR name, DbTable::Type tableType, ECClassId exclusiveRootClassId, DbTable const& parentTable)
    {
    return m_tables.Add(DbTableId(), dbNamespace, name, tableType, exclusiveRootClassId, &parentTable);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbSchema::SynchronizeExistingTables()
    {
    PERFLOG_START("ECDb", "Schema import> Synchronize existing tables");

    bvector<DbTable const*> tables;
    for (bpair<Utf8String, DbTableId> const& tableKey : DbSchemaPersistenceManager::GetTableDefNamesAndIds(m_ecdb,"Type=" SQLVAL_DbTable_Type_Existing))
        {
        Utf8StringCR tableName = tableKey.first;
        DbTable* table = FindTableP(tableName);
        bset<Utf8StringCP, CompareIUtf8Ascii> oldColumnList;
        bmap<Utf8StringCP, SqliteColumnInfo const*, CompareIUtf8Ascii> newColumnList;
        std::vector<SqliteColumnInfo> dbColumnList;
        if (SUCCESS != DbSchemaPersistenceManager::RunPragmaTableInfo(dbColumnList, m_ecdb, tableName, table->GetTableSpace().GetName().c_str()))
            {
            BeAssert(false);
            return ERROR;
            }

        for (SqliteColumnInfo const& dbColumn : dbColumnList)
            newColumnList[&dbColumn.GetName()] = &dbColumn;

        for (DbColumn const* dbColumn : table->GetColumns())
            {
            if (dbColumn->GetPersistenceType() == PersistenceType::Physical)
                oldColumnList.insert(&dbColumn->GetName());
            }

        //Compute how table has changed
        bset<Utf8StringCP, CompareIUtf8Ascii> added;
        bset<Utf8StringCP, CompareIUtf8Ascii> deleted;
        for (Utf8StringCP oldColumn : oldColumnList)
            {
            if (newColumnList.find(oldColumn) != newColumnList.end())
                continue;

            deleted.insert(oldColumn);
            }

        for (bpair<Utf8StringCP, SqliteColumnInfo const*> const& kvPair : newColumnList)
            {
            if (oldColumnList.find(kvPair.first) != oldColumnList.end())
                continue;

            added.insert(kvPair.first);
            }

        if (!deleted.empty())
            {
            BeAssert("Existing table changed without map knowing about it");
            return ERROR;
            }

        if (!table->GetEditHandle().CanEdit())
            table->GetEditHandleR().BeginEdit();

        for (Utf8StringCP addColumnName : added)
            {
            auto itor = newColumnList.find(addColumnName);
            if (table->AddColumn(*addColumnName, itor->second->GetType(), DbColumn::Kind::Default, PersistenceType::Physical) == nullptr)
                {
                BeAssert("Failed to create column");
                return ERROR;
                }
            }

        table->GetEditHandleR().EndEdit();
        if (UpdateTable(*table) != SUCCESS)
            return ERROR;
        }

    PERFLOG_FINISH("ECDb", "Schema import> Synchronize existing tables");
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbSchema::SaveOrUpdateTables() const
    {
    if (SUCCESS != m_tableSpaceManager.PersistNewTableSpaces())
        return ERROR;

    //Following return the list of table and there id from db
    bmap<Utf8String, DbTableId, CompareIUtf8Ascii> existingTables = DbSchemaPersistenceManager::GetTableDefNamesAndIds(m_ecdb);
    for (DbTable const* table : m_tables.GetTablesInDependencyOrder())
        {
        // This would be null in case a table is not loaded yet and if its not loaded then we do not need to update it
        if (table == nullptr)
            continue;

        if (SUCCESS != table->GetLinkNode().Validate())
            return ERROR;

        auto itor = existingTables.find(table->GetName());
        if (itor == existingTables.end())
            {
            if (InsertTable(*table) != SUCCESS)
                return ERROR;
            }
        else
            {
            if (UpdateTable(*table) != SUCCESS)
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
void DbSchema::ClearCache() const
    {
    m_nullTable = nullptr;
    m_tables.ClearCache();
    m_tableSpaceManager.ClearCache();
    m_indexDefsAreLoaded = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbSchema::InsertTable(DbTable const& table) const
    {
    if (!IsNullTable(table) && !table.IsValid())
        {
        BeAssert(false && "Table to insert is not valid");
        return ERROR;
        }

    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO " TABLE_Table "(ParentTableId,Name,TableSpaceId,Type,ExclusiveRootClassId) VALUES(?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    DbTable::LinkNode const* parentNode = table.GetLinkNode().GetParent();
    if (parentNode != nullptr)
        stmt->BindId(1, parentNode->GetTable().GetId());

    stmt->BindText(2, table.GetName(), Statement::MakeCopy::No);

    DbTableSpace const& tableSpace = table.GetTableSpace();
    if (!tableSpace.GetId().IsValid())
        {
        BeAssert(false && "TableSpace must be persisted before persisting new table def");
        return ERROR;
        }

    if (!tableSpace.IsMain())
        stmt->BindId(3, tableSpace.GetId());

    stmt->BindInt(4, Enum::ToInt(table.GetType()));

    if (table.HasExclusiveRootECClass())
        stmt->BindId(5, table.GetExclusiveRootECClassId());


    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    const DbTableId tableId = DbSchemaPersistenceManager::GetLastInsertedId<DbTableId>(m_ecdb);
    if (!tableId.IsValid())
        return ERROR;

    const_cast<DbTable&>(table).SetId(tableId);

    bmap<DbColumn const*, int> primaryKeys;
    if (PrimaryKeyDbConstraint const* pkConstraint = table.GetPrimaryKeyConstraint())
        {
        int i = 0;
        for (DbColumn const* pkCol : pkConstraint->GetColumns())
            {
            primaryKeys[pkCol] = i++;
            }
        }

    int columnOrdinal = 0;
    for (DbColumn const* column : table.GetColumns())
        {
        auto it = primaryKeys.find(column);
        const int primaryKeyOrdinal = it == primaryKeys.end() ? -1 : it->second;
        if (InsertColumn(*column, columnOrdinal, primaryKeyOrdinal) == ERROR)
            return ERROR;

        columnOrdinal++;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbSchema::UpdateTable(DbTable const& table) const
    {
    if (!IsNullTable(table) && !table.IsValid())
        {
        BeAssert(false && "Table to insert is not valid");
        return ERROR;
        }

    bmap<DbColumn const*, int> primaryKeys;
    if (PrimaryKeyDbConstraint const* pkConstraint = table.GetPrimaryKeyConstraint())
        {
        int i = 0;
        for (DbColumn const* pkCol : pkConstraint->GetColumns())
            {
            primaryKeys[pkCol] = i++;
            }
        }

    int existingColumnOrdinal = 0;
    bmap<Utf8String, DbColumnId, CompareIUtf8Ascii> persistedColumnMap = DbSchemaPersistenceManager::GetColumnNamesAndIds(m_ecdb, table.GetId());
    int newColumnOrdinal = (int) persistedColumnMap.size();
    for (DbColumn const* column : table.GetColumns())
        {
        auto it = primaryKeys.find(column);
        const int primaryKeyOrdinal = it == primaryKeys.end() ? -1 : it->second;
        auto itor = persistedColumnMap.find(column->GetName());
        if (itor == persistedColumnMap.end())
            {
            if (InsertColumn(*column, newColumnOrdinal, primaryKeyOrdinal) != SUCCESS)
                return ERROR;

            newColumnOrdinal++;
            }
        else
            {
            if (UpdateColumn(*column, existingColumnOrdinal, primaryKeyOrdinal) != SUCCESS)
                return ERROR;

            existingColumnOrdinal++;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbSchema::InsertColumn(DbColumn const& column, int columnOrdinal, int primaryKeyOrdinal) const
    {
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO ec_Column(TableId,Name,Type,IsVirtual,Ordinal,NotNullConstraint,UniqueConstraint,CheckConstraint,DefaultConstraint,CollationConstraint,OrdinalInPrimaryKey,ColumnKind) VALUES (?,?,?,?,?,?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindId(1, column.GetTable().GetId());
    stmt->BindText(2, column.GetName(), Statement::MakeCopy::No);
    stmt->BindInt(3, Enum::ToInt(column.GetType()));
    stmt->BindBoolean(4, column.GetPersistenceType() == PersistenceType::Virtual);
    stmt->BindInt64(5, columnOrdinal);
    stmt->BindBoolean(6, column.GetConstraints().HasNotNullConstraint());
    stmt->BindBoolean(7, column.GetConstraints().HasUniqueConstraint());

    if (!column.GetConstraints().GetCheckConstraint().empty())
        stmt->BindText(8, column.GetConstraints().GetCheckConstraint(), Statement::MakeCopy::No);

    if (!column.GetConstraints().GetDefaultValueConstraint().empty())
        stmt->BindText(9, column.GetConstraints().GetDefaultValueConstraint(), Statement::MakeCopy::No);

    stmt->BindInt(10, Enum::ToInt(column.GetConstraints().GetCollation()));
    if (primaryKeyOrdinal > -1)
        stmt->BindInt(11, primaryKeyOrdinal);

    stmt->BindInt(12, Enum::ToInt(column.GetKind()));

    if (BE_SQLITE_DONE != stmt->Step())
        return ERROR;

    const DbColumnId colId = DbSchemaPersistenceManager::GetLastInsertedId<DbColumnId>(m_ecdb);
    if (!colId.IsValid())
        return ERROR;

    const_cast<DbColumn&>(column).SetId(colId);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbSchema::UpdateColumn(DbColumn const& column, int columnOrdinal, int primaryKeyOrdinal) const
    {
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("UPDATE ec_Column SET Name=?, Type=?, IsVirtual=?, Ordinal=?, NotNullConstraint=?, UniqueConstraint=?, CheckConstraint=?, DefaultConstraint=?, CollationConstraint=?, OrdinalInPrimaryKey=?, ColumnKind=? WHERE Id=?");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindId(12, column.GetId());
    stmt->BindText(1, column.GetName(), Statement::MakeCopy::No);
    stmt->BindInt(2, Enum::ToInt(column.GetType()));
    stmt->BindBoolean(3, column.GetPersistenceType() == PersistenceType::Virtual);
    stmt->BindInt64(4, columnOrdinal);
    stmt->BindBoolean(5, column.GetConstraints().HasNotNullConstraint());
    stmt->BindBoolean(6, column.GetConstraints().HasUniqueConstraint());

    if (!column.GetConstraints().GetCheckConstraint().empty())
        stmt->BindText(7, column.GetConstraints().GetCheckConstraint(), Statement::MakeCopy::No);

    if (!column.GetConstraints().GetDefaultValueConstraint().empty())
        stmt->BindText(8, column.GetConstraints().GetDefaultValueConstraint(), Statement::MakeCopy::No);

    stmt->BindInt(9, Enum::ToInt(column.GetConstraints().GetCollation()));
    if (primaryKeyOrdinal > -1)
        stmt->BindInt(10, primaryKeyOrdinal);

    stmt->BindInt(11, Enum::ToInt(column.GetKind()));

    return stmt->Step() == BE_SQLITE_DONE ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbSchema::LoadColumns(DbTable& table) const
    {
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("SELECT Id,Name,Type,IsVirtual,NotNullConstraint,UniqueConstraint,CheckConstraint,DefaultConstraint,CollationConstraint,OrdinalInPrimaryKey,ColumnKind FROM ec_Column WHERE TableId=? ORDER BY Ordinal");
    if (stmt == nullptr)
        return ERROR;

    DbResult stat = stmt->BindId(1, table.GetId());
    if (stat != BE_SQLITE_OK)
        return ERROR;

    const int notNullColIx = 4;
    const int uniqueColIx = 5;
    const int checkColIx = 6;
    const int defaultValueColIx = 7;
    const int collationColIx = 8;
    const int pkOrdinalColIx = 9;
    const int kindColIx = 10;

    std::vector<DbColumn*> pkColumns;
    std::vector<size_t> pkOrdinals;
    uint32_t sharedColumnCount = 0;
    while (BE_SQLITE_ROW == stmt->Step())
        {
        DbColumnId id = stmt->GetValueId<DbColumnId>(0);
        Utf8CP name = stmt->GetValueText(1);
        Nullable<DbColumn::Type> type = DbSchemaPersistenceManager::ToDbColumnType(stmt->GetValueInt(2));
        const PersistenceType persistenceType = stmt->GetValueBoolean(3) ? PersistenceType::Virtual : PersistenceType::Physical;

        const bool hasNotNullConstraint = stmt->GetValueBoolean(notNullColIx);
        const bool hasUniqueConstraint = stmt->GetValueBoolean(uniqueColIx);
        Utf8CP constraintCheck = !stmt->IsColumnNull(checkColIx) ? stmt->GetValueText(checkColIx) : nullptr;
        Utf8CP constraintDefault = !stmt->IsColumnNull(defaultValueColIx) ? stmt->GetValueText(defaultValueColIx) : nullptr;
        Nullable<DbColumn::Constraints::Collation> collationConstraint = DbSchemaPersistenceManager::ToDbColumnCollation(stmt->GetValueInt(collationColIx));

        int primaryKeyOrdinal = stmt->IsColumnNull(pkOrdinalColIx) ? -1 : stmt->GetValueInt(pkOrdinalColIx);
        Nullable<DbColumn::Kind> columnKind = DbSchemaPersistenceManager::ToDbColumnKind(stmt->GetValueInt(kindColIx));

        if (type.IsNull() || collationConstraint.IsNull() || columnKind.IsNull())
            {
            LOG.errorv("Failed to load information for column '%s.%s' from " TABLE_Column ". The ECDb file might have been created by a new version of the software.",
                       table.GetName().c_str(),name);
            return ERROR;
            }

        DbColumn* column = table.AddColumn(id, Utf8String(name), type.Value(), columnKind.Value(), persistenceType);
        if (column == nullptr)
            {
            BeAssert(false);
            return SUCCESS;
            }

        if (hasNotNullConstraint)
            column->GetConstraintsR().SetNotNullConstraint();

        if (hasUniqueConstraint)
            column->GetConstraintsR().SetUniqueConstraint();

        column->GetConstraintsR().SetCollation(collationConstraint.Value());

        if (!Utf8String::IsNullOrEmpty(constraintCheck))
            column->GetConstraintsR().SetCheckConstraint(constraintCheck);

        if (!Utf8String::IsNullOrEmpty(constraintDefault))
            column->GetConstraintsR().SetDefaultValueExpression(constraintDefault);

        if (primaryKeyOrdinal >= 0)
            {
            pkColumns.push_back(column);
            pkOrdinals.push_back((size_t) primaryKeyOrdinal);
            }

        if (columnKind == DbColumn::Kind::SharedData)
            sharedColumnCount++;
        }

    if (!pkColumns.empty())
        {
        if (SUCCESS != table.AddPrimaryKeyConstraint(pkColumns, &pkOrdinals))
            return SUCCESS;
        }

    if (table.GetType() != DbTable::Type::Existing && table.GetType() != DbTable::Type::Virtual)
        table.InitializeSharedColumnNameGenerator(sharedColumnCount);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbTable* DbSchema::LoadTable(DbTableId tableId) const
    {
    if (!tableId.IsValid())
        return nullptr;

    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("SELECT Name FROM " TABLE_Table " WHERE Id=?");
    if (stmt == nullptr)
        return nullptr;

    stmt->BindId(1, tableId);
    if (stmt->Step() != BE_SQLITE_ROW)
        return nullptr;

    Utf8String tableName(stmt->GetValueText(0));
    stmt = nullptr;

    return FindTableP(tableName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbTable* DbSchema::LoadTable(Utf8StringCR name) const
    {
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("SELECT Id,TableSpaceId,Type,ExclusiveRootClassId,ParentTableId FROM " TABLE_Table " WHERE Name=?");
    if (stmt == nullptr)
        return nullptr;

    if (BE_SQLITE_OK != stmt->BindText(1, name, Statement::MakeCopy::No) || BE_SQLITE_ROW != stmt->Step())
        return nullptr;

    DbTableId id = stmt->GetValueId<DbTableId>(0);

    DbTableSpaceId tableSpaceId = stmt->GetValueId<DbTableSpaceId>(1);
    DbTableSpace const* tableSpace = m_tableSpaceManager.LoadTableSpace(tableSpaceId);
    if (tableSpace == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    BeAssert(!stmt->IsColumnNull(2) && "Type column is expected to have NOT NULL constraint");
    Nullable<DbTable::Type> tableType = DbSchemaPersistenceManager::ToDbTableType(stmt->GetValueInt(2));
    if (tableType.IsNull())
        {
        LOG.errorv("Failed to load information about table '%s'. It has a type not supported by this software. The file might have been used with newer versions of the software.",
                                         name.c_str());
        return nullptr;
        }

    ECClassId exclusiveRootClassId = !stmt->IsColumnNull(3) ? stmt->GetValueId<ECClassId>(3) : ECClassId();

    DbTableId parentTableId;
    if (!stmt->IsColumnNull(4))
        {
        BeAssert((tableType == DbTable::Type::Joined || tableType == DbTable::Type::Overflow) && "Expecting joined or overflow table if parent table id is not null");
        parentTableId = stmt->GetValueId<DbTableId>(4);
        }

    stmt = nullptr; // release statement so that it can be used to load the parent table.

    DbTable const* parentTable = parentTableId.IsValid() ? FindTable(parentTableId) : nullptr;
    BeAssert(!parentTableId.IsValid() || parentTable != nullptr && "Failed to find parent table");

    DbTable* table = const_cast<TableCollection&>(m_tables).Add(id, *tableSpace, name, tableType.Value(), exclusiveRootClassId, parentTable);
    if (table == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    const bool canEdit = table->GetEditHandle().CanEdit();
    if (!canEdit)
        table->GetEditHandleR().BeginEdit();

    if (LoadColumns(*table) != SUCCESS)
        return nullptr;

    if (!canEdit)
        table->GetEditHandleR().EndEdit();

    return table;
    }


//-------------------------------------------------------------------------------------- -
// @bsimethod                                                Krischan.Eberle        11/2017
//--------------------------------------------------------------------------------------
BentleyStatus DbSchema::RecreateTempTables(bool& hasTempTables) const
    {
    BeMutexHolder lock(m_ecdb.GetImpl().GetMutex());
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("SELECT t.Name FROM " TABLE_Table " t JOIN " TABLE_TableSpace " ts ON t.TableSpaceId=ts.Id WHERE t.Type<>" SQLVAL_DbTable_Type_Existing " AND ts.Name='" TABLESPACE_Temp "'");
    if (stmt == nullptr)
        return ERROR;

    hasTempTables = false;
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        DbTable const* table = FindTable(stmt->GetValueText(0));
        if (table == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        hasTempTables = true;
        BeAssert(table->GetTableSpace().IsTemp());
        if (DbSchemaPersistenceManager::TableExists(m_ecdb, table->GetName().c_str(), TABLESPACE_Temp))
            continue;

        if (DbSchemaPersistenceManager::CreateOrUpdateTableResult::Created != DbSchemaPersistenceManager::CreateOrUpdateTable(m_ecdb, *table))
            return ERROR;
        }

    if (!hasTempTables)
        return SUCCESS;

    return RecreateTempIndexes();
    }


//-------------------------------------------------------------------------------------- -
// @bsimethod                                                Krischan.Eberle        11/2017
//--------------------------------------------------------------------------------------
BentleyStatus DbSchema::RecreateTempIndexes() const
    {
    std::vector<std::pair<DbTable*, std::unique_ptr<DbIndex>>> indexes;
    if (LoadIndexDefs(indexes, " INNER JOIN " TABLE_TableSpace " TS ON TS.Id=T.TableSpaceId WHERE T.Type<>" SQLVAL_DbTable_Type_Existing " AND TS.Name='" TABLESPACE_Temp "'"))
        return ERROR;

    for (std::pair<DbTable*, std::unique_ptr<DbIndex>> const& pair : indexes)
        {
        DbIndex const& index = *pair.second;

        if (DbSchemaPersistenceManager::IndexExists(m_ecdb, index.GetName().c_str(), TABLESPACE_Temp))
            continue;

        Utf8String ddl,unused;
        if (SUCCESS != DbSchemaPersistenceManager::BuildCreateIndexDdl(ddl,unused, m_ecdb, index))
            return ERROR;

        if (SUCCESS != DbSchemaPersistenceManager::CreateIndex(m_ecdb, index, ddl))
            return ERROR;
        }

    return SUCCESS;
    }

// -------------------------------------------------------------------------------------- -
// @bsimethod                                                    Affan.Khan        09/2014
//--------------------------------------------------------------------------------------
BentleyStatus DbSchema::LoadIndexDefs() const
    {
    if (m_indexDefsAreLoaded)
        return SUCCESS;

    std::vector<std::pair<DbTable*, std::unique_ptr<DbIndex>>> indexes;
    if (LoadIndexDefs(indexes, nullptr))
        return ERROR;

    for (std::pair<DbTable*, std::unique_ptr<DbIndex>>& pair : indexes)
        {
        pair.first->AddIndexDef(std::move(pair.second));
        }

    m_indexDefsAreLoaded = true;
    return SUCCESS;
    }

//-------------------------------------------------------------------------------------- -
// @bsimethod                                                Krischan.Eberle        11/2017
//--------------------------------------------------------------------------------------
BentleyStatus DbSchema::LoadIndexDefs(std::vector<std::pair<DbTable*, std::unique_ptr<DbIndex>>>& indexDefs, Utf8CP sqlWhereOrJoinClause) const
    {
    Utf8CP sql = "SELECT I.Id, T.Name, I.Name, I.IsUnique, I.AddNotNullWhereExp, I.IsAutoGenerated, I.ClassId, I.AppliesToSubclassesIfPartial FROM " TABLE_Index " I INNER JOIN " TABLE_Table " T ON T.Id = I.TableId";

    CachedStatementPtr stmt;
    if (Utf8String::IsNullOrEmpty(sqlWhereOrJoinClause))
        stmt = GetECDb().GetImpl().GetCachedSqliteStatement(sql);
    else
        {
        Utf8String extendedSql(sql);
        extendedSql.append(sqlWhereOrJoinClause);
        stmt = GetECDb().GetImpl().GetCachedSqliteStatement(extendedSql.c_str());
        }

    if (stmt == nullptr)
        return ERROR;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        BeInt64Id id = stmt->GetValueId<BeInt64Id>(0);
        Utf8CP tableName = stmt->GetValueText(1);
        Utf8CP name = stmt->GetValueText(2);
        bool isUnique = stmt->GetValueBoolean(3);
        bool addNotNullWhereExp = stmt->GetValueBoolean(4);
        bool isAutoGenerated = stmt->GetValueBoolean(5);
        ECClassId classId = !stmt->IsColumnNull(6) ? stmt->GetValueId<ECClassId>(6) : ECClassId();
        bool appliesToSubclassesIfPartial = stmt->GetValueBoolean(7);

        DbTable* table = FindTableP(tableName);
        if (table == nullptr)
            {
            BeAssert(false && "Failed to find table");
            return ERROR;
            }

        CachedStatementPtr indexColStmt = GetECDb().GetImpl().GetCachedSqliteStatement("SELECT C.Name FROM " TABLE_IndexColumn " I INNER JOIN ec_Column C ON C.Id = I.ColumnId WHERE I.IndexId = ? ORDER BY I.Ordinal");
        if (indexColStmt == nullptr)
            return ERROR;

        indexColStmt->BindId(1, id);
        std::vector<DbColumn const*> columns;
        while (indexColStmt->Step() == BE_SQLITE_ROW)
            {
            Utf8CP columnName = indexColStmt->GetValueText(0);
            DbColumn const* col = table->FindColumn(columnName);
            if (col == nullptr)
                return ERROR;

            columns.push_back(col);
            }

        indexDefs.push_back(std::make_pair(table, std::make_unique<DbIndex>(*table, Utf8String(name), isUnique, columns, addNotNullWhereExp, isAutoGenerated, classId, appliesToSubclassesIfPartial)));
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbSchema::PersistIndexDef(DbIndex const& index) const
    {
    CachedStatementPtr stmt = GetECDb().GetImpl().GetCachedSqliteStatement("INSERT INTO " TABLE_Index "(TableId,Name,IsUnique,AddNotNullWhereExp,IsAutoGenerated,ClassId,AppliesToSubclassesIfPartial) VALUES(?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindId(1, index.GetTable().GetId());
    stmt->BindText(2, index.GetName(), Statement::MakeCopy::No);
    stmt->BindBoolean(3, index.GetIsUnique());
    stmt->BindBoolean(4, index.IsAddColumnsAreNotNullWhereExp());

    stmt->BindBoolean(5, index.IsAutoGenerated());
    if (index.HasClassId())
        stmt->BindId(6, index.GetClassId());

    stmt->BindBoolean(7, index.AppliesToSubclassesIfPartial());

    if (BE_SQLITE_DONE != stmt->Step())
        {
        m_ecdb.GetImpl().Issues().Report("Failed to persist definition for index %s on table %s: %s",
                                         index.GetName().c_str(), index.GetTable().GetName().c_str(), GetECDb().GetLastError().c_str());
        return ERROR;
        }

    const BeInt64Id indexId = DbSchemaPersistenceManager::GetLastInsertedId<BeInt64Id>(m_ecdb);
    if (!indexId.IsValid())
        return ERROR;

    stmt = nullptr; //free resources

    CachedStatementPtr indexColStmt = GetECDb().GetImpl().GetCachedSqliteStatement("INSERT INTO " TABLE_IndexColumn "(IndexId,ColumnId,Ordinal) VALUES(?,?,?)");
    if (indexColStmt == nullptr)
        return ERROR;

    int i = 0;
    for (DbColumn const* col : index.GetColumns())
        {
        if (BE_SQLITE_OK != indexColStmt->BindId(1, indexId) ||
            BE_SQLITE_OK != indexColStmt->BindId(2, col->GetId()) ||
            BE_SQLITE_OK != indexColStmt->BindInt(3, i))
            {
            BeAssert(false);
            return ERROR;
            }

        if (BE_SQLITE_DONE != indexColStmt->Step())
            {
            m_ecdb.GetImpl().Issues().Report("Failed to persist definition for index %s on table %s. Could not persist index column information for column %s: %s.",
                       index.GetName().c_str(), index.GetTable().GetName().c_str(), col->GetName().c_str(), m_ecdb.GetLastError().c_str());
            return ERROR;
            }

        indexColStmt->Reset();
        indexColStmt->ClearBindings();
        i++;
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//--------------------------------------------------------------------------------------
DbTable const* DbSchema::GetNullTable() const
    {
    if (m_nullTable == nullptr)
        {
        m_nullTable = FindTableP(DBSCHEMA_NULLTABLENAME);
        if (m_nullTable == nullptr)
            m_nullTable = const_cast<DbSchema*>(this)->AddTable(m_tableSpaceManager.GetMain(), DBSCHEMA_NULLTABLENAME, DbTable::Type::Virtual, ECClassId());

        if (m_nullTable != nullptr && m_nullTable->GetEditHandleR().CanEdit())
            m_nullTable->GetEditHandleR().EndEdit();
        }

    BeAssert(m_nullTable != nullptr);
    return m_nullTable;
    }

//****************************************************************************************
//DbSchema::TableCollection
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   07/2017
//---------------------------------------------------------------------------------------
DbTable const* DbSchema::TableCollection::Get(Utf8StringCR name) const
    {
    auto it = m_tableMapByName.find(name); 
    if (it == m_tableMapByName.end())
        return nullptr;

    return it->second.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   07/2017
//---------------------------------------------------------------------------------------
DbTable const* DbSchema::TableCollection::Get(DbTableId tableId) const
    {
    auto it = m_cacheById.find(tableId);
    if (it == m_cacheById.end())
        return nullptr;

    return it->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   07/2017
//---------------------------------------------------------------------------------------
DbTable* DbSchema::TableCollection::Add(DbTableId tableId, DbTableSpace const& dbNamespace, Utf8StringCR name, DbTable::Type tableType, ECClassId exclusiveRootClassId, DbTable const* parentTable)
    {
    if (name.empty())
        {
        BeAssert(false && "TableCollection::Add expects table name to be not empty.");
        return nullptr;
        }

    if (Contains(name))
        {
        BeAssert(false && "Table with same name already exists");
        return nullptr;
        }

    if (parentTable != nullptr && !dbNamespace.GetName().EqualsIAscii(parentTable->GetTableSpace().GetName()))
        {
        BeAssert(false && "Child table is expected to have same TableSpace as parent table");
        return nullptr;
        }

    std::unique_ptr<DbTable> table = std::make_unique<DbTable>(m_ecdb, tableId, dbNamespace, name, tableType, exclusiveRootClassId, parentTable);
    if (tableType == DbTable::Type::Existing)
        table->GetEditHandleR().EndEdit(); //we do not want this table to be editable;

    DbTable* tableP = table.get();
    m_tableMapByName[tableP->GetName()] = std::move(table);

    if (tableId.IsValid())
        m_cacheById[tableId] = tableP;

    return tableP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
std::vector<DbTable const*> DbSchema::TableCollection::GetTablesInDependencyOrder() const
    {
    std::vector<DbTable const*> cachedTables;
    std::vector<DbTable const*> cachedTablesJoined;
    std::vector<DbTable const*> cachedTablesOverflow;

    for (auto const& tableKey : m_tableMapByName)
        {
        if (tableKey.second != nullptr)
            {
            if (tableKey.second->GetType() == DbTable::Type::Joined)
                cachedTablesJoined.push_back(tableKey.second.get());
            else if (tableKey.second->GetType() == DbTable::Type::Overflow)
                cachedTablesOverflow.push_back(tableKey.second.get());
            else
                cachedTables.insert(cachedTables.begin(), tableKey.second.get());
            }
        }

    cachedTables.insert(cachedTables.end(), cachedTablesJoined.begin(), cachedTablesJoined.end());
    cachedTables.insert(cachedTables.end(), cachedTablesOverflow.begin(), cachedTablesOverflow.end());
    return cachedTables;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2017
//---------------------------------------------------------------------------------------
void DbSchema::TableCollection::Remove(Utf8StringCR tableName) const
    {
    auto it = m_tableMapByName.find(tableName);
    if (it == m_tableMapByName.end())
        return;

    if (it->second != nullptr)
        {
        auto cacheIt = m_cacheById.find(it->second->GetId());
        m_cacheById.erase(cacheIt);
        }

    m_tableMapByName.erase(it);
    }





//****************************************************************************************
//DbSchema::TableSpaceManager
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   11/2017
//---------------------------------------------------------------------------------------
DbTableSpace const& DbSchema::TableSpaceManager::GetMain() const
    {
    if (m_mainTableSpace != nullptr)
        return *m_mainTableSpace;

    FindOrAddTableSpace(TABLESPACE_Main);
    BeAssert(m_mainTableSpace != nullptr);
    return *m_mainTableSpace;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   11/2017
//---------------------------------------------------------------------------------------
DbTableSpace const& DbSchema::TableSpaceManager::GetTemp() const
    {
    if (m_tempTableSpace != nullptr)
        return *m_tempTableSpace;

    FindOrAddTableSpace(TABLESPACE_Temp);
    BeAssert(m_tempTableSpace != nullptr);
    return *m_tempTableSpace;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   11/2017
//---------------------------------------------------------------------------------------
DbTableSpace const* DbSchema::TableSpaceManager::FindOrAddTableSpace(Utf8CP name) const
    {
    auto it = m_tableSpaces.find(name);
    if (it != m_tableSpaces.end())
        return it->second.get();

    DbTableSpaceId id = LookupTableSpaceId(name);
    if (!id.IsValid())
        return AddTableSpace(name); // add a new namespace

    return AddTableSpace(id, name); //add a an existing namespace
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   11/2017
//---------------------------------------------------------------------------------------
DbTableSpace const* DbSchema::TableSpaceManager::AddTableSpace(DbTableSpaceId id, Utf8CP name) const
    {
    if (m_tableSpaces.find(name) != m_tableSpaces.end())
        return nullptr;

    DbTableSpace const* ts = nullptr;
    
    {
    std::unique_ptr<DbTableSpace> tsPtr = std::make_unique<DbTableSpace>(id, name);
    ts = tsPtr.get();
    m_tableSpaces[ts->GetName().c_str()] = std::move(tsPtr);
    }

    //cache the known table space objects for quicker access and a particular order for persistence
    if (ts->IsMain())
        {
        m_tableSpacesOrdered.insert(m_tableSpacesOrdered.begin(), ts);
        m_mainTableSpace = ts;
        }
    else if (ts->IsTemp())
        {
        if (m_tableSpacesOrdered.empty() || !m_tableSpacesOrdered[0]->IsMain())
            m_tableSpacesOrdered.insert(m_tableSpacesOrdered.begin(), ts);
        else
            m_tableSpacesOrdered.insert(m_tableSpacesOrdered.begin() + 1, ts);

        m_tempTableSpace = ts;
        }
    else
        m_tableSpacesOrdered.push_back(ts);

    return ts;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   11/2017
//---------------------------------------------------------------------------------------
DbTableSpace const* DbSchema::TableSpaceManager::LoadTableSpace(DbTableSpaceId id) const
    {
    if (!id.IsValid())
        return &GetMain(); //main table space is not persisted in ec_Table to save some disk space

    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("SELECT Name FROM " TABLE_TableSpace " WHERE Id=?");
    if (stmt == nullptr)
        return nullptr;

    stmt->BindId(1, id);

    if (BE_SQLITE_ROW != stmt->Step())
        return nullptr;

    Utf8CP name = stmt->GetValueText(0);

    auto it = m_tableSpaces.find(name);
    if (it != m_tableSpaces.end())
        return it->second.get();

    return AddTableSpace(id, name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   11/2017
//---------------------------------------------------------------------------------------
DbTableSpaceId DbSchema::TableSpaceManager::LookupTableSpaceId(Utf8CP name) const
    {
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("SELECT Id FROM " TABLE_TableSpace " WHERE Name=?");
    if (stmt == nullptr)
        return DbTableSpaceId();

    stmt->BindText(1, name, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return DbTableSpaceId();

    return stmt->GetValueId<DbTableSpaceId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle        11/2017
//---------------------------------------------------------------------------------------
BentleyStatus DbSchema::TableSpaceManager::PersistNewTableSpaces() const
    {
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO " TABLE_TableSpace "(Name) VALUES(?)");
    if (stmt == nullptr)
        return ERROR;

    for (DbTableSpace const* tableSpace : m_tableSpacesOrdered)
        {
        if (tableSpace->GetId().IsValid())
            continue; //if it has id, it was already persisted

        stmt->BindText(1, tableSpace->GetName(), Statement::MakeCopy::No);

        if (BE_SQLITE_DONE != stmt->Step())
            return ERROR;

        stmt->Reset();
        stmt->ClearBindings();

        const DbTableSpaceId id = DbSchemaPersistenceManager::GetLastInsertedId<DbTableSpaceId>(m_ecdb);
        if (!id.IsValid())
            return ERROR;

        const_cast<DbTableSpace&>(*tableSpace).SetId(id);
        }

    return SUCCESS;
    }

//****************************************************************************************
//DbTableSpace
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   11/2017
//---------------------------------------------------------------------------------------
DbTableSpace::DbTableSpace(DbTableSpaceId id, Utf8CP name) : m_id(id), m_name(name)
    {
    if (m_name.empty() || m_name.EqualsIAscii(TABLESPACE_Main))
        {
        m_type = Type::Main;
        if (m_name.empty())
            m_name.assign(TABLESPACE_Main);

        return;
        }

    if (m_name.EqualsIAscii(TABLESPACE_Temp))
        {
        m_type = Type::Temp;
        return;
        }

    m_type = Type::Attached;
    }


//****************************************************************************************
//DbTable
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   05/2016
//---------------------------------------------------------------------------------------
DbTable::DbTable(ECDbCR ecdb, DbTableId id, DbTableSpace const& dbNamespace, Utf8StringCR name, Type type, ECN::ECClassId exclusiveRootClass, DbTable const* parentTable)
    : m_ecdb(ecdb), m_id(id), m_tableSpace(dbNamespace), m_name(name), m_type(type), m_exclusiveRootECClassId(exclusiveRootClass), m_linkNode(*this, parentTable)
    {
    if (m_type != Type::Existing && m_type != Type::Virtual)
        m_sharedColumnNameGenerator = DbSchemaNameGenerator(GetSharedColumnNamePrefix(m_type));

    BeAssert(m_linkNode.Validate() == SUCCESS);
    BeAssert(m_linkNode.GetParent() == nullptr || m_linkNode.GetParent()->GetTable().GetTableSpace().GetName().EqualsIAscii(m_tableSpace.GetName()));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2017
//---------------------------------------------------------------------------------------
bool DbTable::operator==(DbTable const& rhs) const
    {
    if (this == &rhs)
        return true;

    if (m_id.IsValid() && rhs.m_id.IsValid())
        return m_id == rhs.m_id;

    return m_name.EqualsIAscii(rhs.m_name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   05/2016
//---------------------------------------------------------------------------------------
BentleyStatus DbTable::AddPrimaryKeyConstraint(std::vector<DbColumn*> const& pkColumns, std::vector<size_t> const* pkOrdinals)
    {
    if (GetEditHandleR().AssertNotInEditMode())
        return ERROR;

    //if ordinals are passed and PK consists of more than one column, order the columns first
    std::vector<DbColumn*> const* orderedPkColumnsP = &pkColumns;

    std::vector<DbColumn*> orderedPkColumns;
    if (pkColumns.size() > 1 && pkOrdinals != nullptr)
        {
        for (size_t pkOrdinal : *pkOrdinals)
            {
            orderedPkColumns.push_back(pkColumns[pkOrdinal]);
            }

        orderedPkColumnsP = &orderedPkColumns;
        }

    std::unique_ptr<PrimaryKeyDbConstraint> pkConstraint = PrimaryKeyDbConstraint::Create(*this, *orderedPkColumnsP);
    if (pkConstraint == nullptr)
        return ERROR;

    m_pkConstraint = std::move(pkConstraint);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ForeignKeyDbConstraint const* DbTable::AddForeignKeyConstraint(DbColumn const& fkColumn, DbColumn const& referencedColumn, ForeignKeyDbConstraint::ActionType onDeleteAction, ForeignKeyDbConstraint::ActionType onUpdateAction)
    {
    if (GetEditHandleR().AssertNotInEditMode())
        return nullptr;

    if (fkColumn.GetPersistenceType() == PersistenceType::Virtual || referencedColumn.GetPersistenceType() == PersistenceType::Virtual)
        {
        BeAssert(false && "Cannot create FK constraint on virtual columns");
        return nullptr;
        }

    std::unique_ptr<ForeignKeyDbConstraint> constraint (new ForeignKeyDbConstraint(*this, fkColumn, referencedColumn, onDeleteAction, onUpdateAction));
    ForeignKeyDbConstraint* constraintP = constraint.get();
    m_constraints.push_back(std::move(constraint));

    //! remove the fk constraint if already exist due to another relationship on same column
    constraintP->RemoveIfDuplicate();

    return constraintP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
std::vector<DbConstraint const*> DbTable::GetConstraints() const
    {
    std::vector<DbConstraint const*> constraints;
    for (auto const& constraint : m_constraints)
        constraints.push_back(constraint.get());

    return constraints;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                          muhammad.zaighum                           01/2015
//---------------------------------------------------------------------------------------
BentleyStatus DbTable::AddTrigger(Utf8StringCR triggerName, DbTrigger::Type type, Utf8StringCR condition, Utf8StringCR body)
    {
    if (m_type == Type::Existing)
        {
        BeAssert(false);
        return ERROR;
        }

    if (m_triggers.find(triggerName.c_str()) == m_triggers.end())
        {
        std::unique_ptr<DbTrigger> trigger = std::make_unique<DbTrigger>(triggerName, *this, type, condition, body);
        DbTrigger* triggerP = trigger.get();
        m_triggers[triggerP->GetName().c_str()] = std::move(trigger);
        return SUCCESS;
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
std::vector<DbTrigger const*> DbTable::GetTriggers()const
    {
    std::vector<DbTrigger const*> triggers;
    for (auto &trigger : m_triggers)
        {
        triggers.push_back(trigger.second.get());
        }
    return triggers;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbTable::RemoveConstraint(DbConstraint const& constraint)
    {
    for (auto itor = m_constraints.begin(); itor != m_constraints.end(); ++itor)
        {
        if (itor->get() == &constraint)
            {
            m_constraints.erase(itor);
            return SUCCESS;
            }
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbColumn* DbTable::AddColumn(DbColumnId id, Utf8StringCR colName, DbColumn::Type type, int position, DbColumn::Kind kind, PersistenceType persistenceType)
    {
    if (colName.empty())
        {
        BeAssert(false && "DbTable::AddColumn cannot be called if column name is null or empty");
        return nullptr;
        }

    if (FindColumn(colName.c_str()) != nullptr)
        {
        BeAssert(false && "DbTable::AddColumn> Column with specified name already exist");
        return nullptr;
        }

    if (!GetEditHandleR().CanEdit())
        {
        IssueReporter const& issues = m_ecdb.GetImpl().Issues();
        if (m_type == Type::Existing)
            issues.Report("Cannot add columns to the existing table '%s' not owned by ECDb.", m_name.c_str());
        else
            {
            BeAssert(false && "Cannot add columns to read-only table.");
            issues.Report("Cannot add columns to the table '%s'. Table is not in edit mode.", m_name.c_str());
            }

        return nullptr;
        }

    if (m_type == Type::Virtual)
        {
        //!Force column to be virtual
        persistenceType = PersistenceType::Virtual;
        }

    std::shared_ptr<DbColumn> newColumn = std::make_shared<DbColumn>(id, *this, colName, type, kind, persistenceType);
    DbColumn* newColumnP = newColumn.get();
    m_columns[newColumn->GetName().c_str()] = newColumn;

    if (position < 0)
        m_orderedColumns.push_back(newColumnP);
    else
        m_orderedColumns.insert(m_orderedColumns.begin() + (size_t) position, newColumnP);

    if (kind == DbColumn::Kind::ECClassId)
        m_classIdColumn = newColumnP;

    return newColumnP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2016
//---------------------------------------------------------------------------------------
DbColumn* DbTable::AddSharedColumn()
    {
    Utf8String generatedName;
    m_sharedColumnNameGenerator.Generate(generatedName);
    BeAssert(FindColumn(generatedName.c_str()) == nullptr);
    return AddColumn(generatedName, DbColumn::Type::Any, DbColumn::Kind::SharedData, PersistenceType::Physical);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbColumn const* DbTable::FindColumn(Utf8CP name) const
    {
    auto itor = m_columns.find(name);
    if (itor != m_columns.end())
        return itor->second.get();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbColumn* DbTable::FindColumnP(Utf8CP name) const
    {
    auto itor = m_columns.find(name);
    if (itor != m_columns.end())
        return itor->second.get();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
std::vector<DbColumn const*> DbTable::FindAll(PersistenceType persistenceType) const
    {
    std::vector<DbColumn const*> columns;
    for (DbColumn const* column : m_orderedColumns)
        {
        if (column->GetPersistenceType() == persistenceType)
            columns.push_back(column);
        }

    return columns;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
std::vector<DbColumn const*> DbTable::FindAll(DbColumn::Kind kind) const
    {
    std::vector<DbColumn const*> columns;
    for (DbColumn const* column : m_orderedColumns)
        {
        if (column->GetKind() == kind)
            columns.push_back(column);
        }

    return columns;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbColumn const* DbTable::FindFirst(DbColumn::Kind kind) const
    {
    for (DbColumn const* column : m_orderedColumns)
        {
        if (column->GetKind() == kind)
            return column;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  05/2017
//---------------------------------------------------------------------------------------
Utf8CP DbTable::GetSharedColumnNamePrefix(Type tableType)
    {
    switch (tableType)
        {
            case Type::Primary:
                return "ps";

            case Type::Joined:
                return "js";

            case Type::Overflow:
                return "os";

            default:
                BeAssert(false && "Must not be called for this table type");
                return nullptr;
        }
    }

//****************************************************************************************
//DbTable::LinkNode
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    05/2017
//---------------------------------------------------------------------------------------
DbTable::LinkNode::LinkNode(DbTable const& table, DbTable const* parent) : m_table(table)
    {
    if (parent != nullptr)
        {
        m_parent = &parent->GetLinkNode();
        const_cast<LinkNode*>(m_parent)->m_children.push_back(this);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    05/2017
//---------------------------------------------------------------------------------------
BentleyStatus DbTable::LinkNode::Validate() const
    {
    switch (m_table.GetType())
        {
            case Type::Primary:
            {
            if (m_parent != nullptr)
                {
                BeAssert(false && "Primary table must not have a parent table node");
                return ERROR;
                }

            if (!m_children.empty())
                {
                Type type = m_children[0]->m_table.GetType();
                DbTableSpace const& tableSpace = m_children[0]->m_table.GetTableSpace();

                for (size_t i = 1; i < m_children.size(); i++)
                    {
                    if (m_children[i]->m_table.GetType() != type || m_children[i]->m_table.GetTableSpace() != tableSpace)
                        {
                        BeAssert(false && "All sibling tables must be of the same type");
                        return ERROR;
                        }
                    }
                }
            break;
            }
            case Type::Existing:
            {
            if (m_parent != nullptr || !m_children.empty())
                {
                BeAssert(false && "'Existing' table must neither have parent nor child table nodes");
                return ERROR;
                }

            break;
            }

            case Type::Joined:
            {
            if (m_parent == nullptr || m_parent->m_table.GetType() != Type::Primary || m_parent->m_table.GetTableSpace() != m_table.GetTableSpace())
                {
                BeAssert(false && "Joined table must have a parent table and it must be of type 'Primary' and parent and joined table must be in same table space");
                return ERROR;
                }

            if (!m_children.empty())
                {
                if (m_children.size() > 1 || m_children[0]->m_table.GetType() != Type::Overflow)
                    {
                    BeAssert(false && "Joined table can only have a single child table at most and it must be an overflow table");
                    return ERROR;
                    }
                }

            break;
            }

            case Type::Overflow:
            {
            if (m_parent == nullptr || m_parent->m_table.GetTableSpace() != m_table.GetTableSpace())
                {
                BeAssert(false && "Overflow table must have a parent table and both must be in the same table space");
                return ERROR;
                }

            if (!m_children.empty())
                {
                BeAssert(false && "Overflow table must not have child tables");
                return ERROR;
                }

            break;
            }

            case Type::Virtual:
            {
            if (m_parent != nullptr)
                {
                BeAssert(false && "Virtual table must have a parent table");
                return ERROR;
                }

            if (!m_children.empty())
                {
                BeAssert(false && "Virtual table must not have child tables");
                return ERROR;
                }

            break;
            }
            default:
                BeAssert(false);
                break;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    05/2017
//---------------------------------------------------------------------------------------
DbTable::LinkNode const* DbTable::LinkNode::FindOverflowTable() const
    {
    BeAssert(m_table.GetType() != Type::Overflow);
    if (m_children.empty())
        return nullptr;

    LinkNode const* nextNode = m_children[0];
    return nextNode->m_table.GetType() == Type::Overflow ? nextNode : nullptr;
    }

//****************************************************************************************
//DbTable::EditHandle
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        11/2014
//---------------------------------------------------------------------------------------
bool DbTable::EditHandle::BeginEdit()
    {
    if (CanEdit())
        {
        BeAssert(false && "Already in edit mode");
        return false;
        }

    m_canEdit = true;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        11/2014
//---------------------------------------------------------------------------------------
bool DbTable::EditHandle::EndEdit()
    {
    if (!CanEdit())
        {
        BeAssert(false && "Not in edit mode");
        return false;
        }

    m_canEdit = false;
    return true;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        11/2014
//---------------------------------------------------------------------------------------
bool DbTable::EditHandle::AssertNotInEditMode()
    {
    if (!CanEdit())
        {
        BeAssert(false && "Require object to be in edit mode. Call editHandle.BeginEdit() to enable it");
        return true;
        }

    return false;
    }


//****************************************************************************************
//DbColumn
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2017
//---------------------------------------------------------------------------------------
bool DbColumn::operator==(DbColumn const& rhs) const
    {
    if (this == &rhs)
        return true;

    if (m_id.IsValid() && rhs.m_id.IsValid())
        return m_id == rhs.m_id;

    return m_name.EqualsIAscii(rhs.m_name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   05/2016
//---------------------------------------------------------------------------------------
bool DbColumn::IsUnique() const { return m_constraints.HasUniqueConstraint() || IsOnlyColumnOfPrimaryKeyConstraint(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   05/2016
//---------------------------------------------------------------------------------------
bool DbColumn::IsOnlyColumnOfPrimaryKeyConstraint() const
    {
    return m_pkConstraint != nullptr && m_pkConstraint->GetColumns().size() == 1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        2/2017
//---------------------------------------------------------------------------------------
int DbColumn::DeterminePosition() const
    {
    bvector<DbColumn const*> const& columns = GetTable().GetColumns();
    for (size_t i = 0; i < columns.size(); i++)
        {
        if (columns[i] == this)
            return (int) i;
        }

    BeAssert(false && "Column must exist in the table");
    return -1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbColumn::SetKind(Kind kind)
    {
    if (GetTableR().GetEditHandleR().AssertNotInEditMode())
        return ERROR;

    m_kind = kind;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//--------------------------------------------------------------------------------------
//static 
bool DbColumn::IsCompatible(DbColumn::Type target, DbColumn::Type source)
    {
    switch (source)
        {
            case DbColumn::Type::Any:
                switch (target)
                    {
                        case DbColumn::Type::Any:
                            return true;
                    }
                break;
            case DbColumn::Type::Blob:
                switch (target)
                    {
                        case DbColumn::Type::Any:
                        case DbColumn::Type::Blob:
                            return true;
                    }
                break;
            case DbColumn::Type::Boolean:
                switch (target)
                    {
                        case DbColumn::Type::Any:
                        case DbColumn::Type::Boolean:
                            return true;
                    }
                break;
            case DbColumn::Type::TimeStamp:
                switch (target)
                    {
                        case DbColumn::Type::Any:
                        case DbColumn::Type::TimeStamp:
                            return true;
                    }
                break;
            case DbColumn::Type::Real:
                switch (target)
                    {
                        case DbColumn::Type::Any:
                        case DbColumn::Type::Real:
                            return true;
                    }
                break;
            case DbColumn::Type::Integer:
                switch (target)
                    {
                        case DbColumn::Type::Any:
                        case DbColumn::Type::Integer:
                            return true;
                    }
                break;
            case DbColumn::Type::Text:
                switch (target)
                    {
                        case DbColumn::Type::Any:
                        case DbColumn::Type::Text:
                            return true;
                    }
                break;
        }

    return false;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   02/2016
//---------------------------------------------------------------------------------------
//static 
Utf8CP DbColumn::TypeToSql(DbColumn::Type colType)
    {
    switch (colType)
        {
            case DbColumn::Type::Any:
            case DbColumn::Type::Blob:
                return "BLOB";
            case DbColumn::Type::Boolean:
                return "BOOLEAN";
            case DbColumn::Type::TimeStamp:
                return "TIMESTAMP";
            case DbColumn::Type::Real:
                return "REAL";
            case DbColumn::Type::Integer:
                return "INTEGER";
            case DbColumn::Type::Text:
                return "TEXT";

            default:
                BeAssert(false && "Adjust ColumnTypeToSql for new column type");
                return nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static 
Utf8CP DbColumn::Constraints::CollationToSql(DbColumn::Constraints::Collation collation)
    {
    switch (collation)
        {
        case Collation::Unset:
            return "";

        case Collation::Binary:
            return "COLLATE BINARY";

        case Collation::NoCase:
            return "COLLATE NOCASE";

        case Collation::RTrim:
            return "COLLATE RTRIM";

        default:
            BeAssert(false && "Unhandled value of Enum Collation");
            return "";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static 
bool DbColumn::Constraints::TryParseCollationString(Collation& collation, Utf8StringCR str)
    {
    if (str.empty())
        {
        collation = Collation::Unset;
        return true;
        }

    if (str.EqualsIAscii("Binary"))
        collation = Collation::Binary;
    else if (str.EqualsIAscii("NoCase"))
        collation = Collation::NoCase;
    else if (str.EqualsIAscii("RTrim"))
        collation = Collation::RTrim;
    else
        return false;

    return true;
    }

//****************************************************************************************
//DbColumn::CreateParams
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberlen        12/2016
//---------------------------------------------------------------------------------------
void DbColumn::CreateParams::Assign(Utf8StringCR colName, bool colNameIsFromPropertyMapCA, bool addNotNullConstraint, bool addUniqueConstraint, DbColumn::Constraints::Collation collation)
    {
    m_columnName.assign(colName);
    m_columnNameIsFromPropertyMapCA = colNameIsFromPropertyMapCA;
    m_addNotNullConstraint = addNotNullConstraint;
    m_addUniqueConstraint = addUniqueConstraint;
    m_collation = collation;
    }

//****************************************************************************************
//PrimaryKeyDbConstraint
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  05/2016
//---------------------------------------------------------------------------------------
std::unique_ptr<PrimaryKeyDbConstraint> PrimaryKeyDbConstraint::Create(DbTable const& table, std::vector<DbColumn*> const& columns)
    {
    if (columns.empty())
        {
        BeAssert(false && "PK must at least have one column");
        return nullptr;
        }

    std::set<Utf8CP, CompareIUtf8Ascii> uniqueColNames;
    std::unique_ptr<PrimaryKeyDbConstraint> pkConstraint(new PrimaryKeyDbConstraint(table));
    for (DbColumn* col : columns)
        {
        if (uniqueColNames.find(col->GetName().c_str()) != uniqueColNames.end())
            {
            BeAssert(false && "Duplicate columns in PK constraint");
            return nullptr;
            }

        if (col->GetPersistenceType() == PersistenceType::Virtual)
            {
            BeAssert(false && "Virtual columns are not allowed in PK constraint");
            return nullptr;
            }

        pkConstraint->m_columns.push_back(col);
        col->SetIsPrimaryKeyColumn(*pkConstraint);
        }

    return pkConstraint;
    }


//****************************************************************************************
//ForeignKeyDbConstraint
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ForeignKeyDbConstraint::ForeignKeyDbConstraint(DbTable const& fkTable, DbColumn const& fkColumn, DbColumn const& referencedColumn, ForeignKeyDbConstraint::ActionType onDeleteAction, ForeignKeyDbConstraint::ActionType onUpdateAction)
    : DbConstraint(Type::ForeignKey, fkTable), m_onDeleteAction(onDeleteAction), m_onUpdateAction(onUpdateAction)
    {
    m_fkColumns.push_back(&fkColumn);
    m_referencedTableColumns.push_back(&referencedColumn);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
//static
Utf8CP ForeignKeyDbConstraint::ActionTypeToSql(ActionType actionType)
    {
    switch (actionType)
        {
            case ActionType::Cascade:
                return "CASCADE";
            case ActionType::NoAction:
                return "NO ACTION";
            case ActionType::Restrict:
                return "RESTRICT";
            case ActionType::SetDefault:
                return "SET DEFAULT";
            case ActionType::SetNull:
                return "SET NULL";
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static
BentleyStatus ForeignKeyDbConstraint::TryParseActionType(ActionType& actionType, Nullable<Utf8String> const& str)
    {
    if (str.IsNull())
        {
        actionType = ActionType::NotSpecified;
        return SUCCESS;
        }

    BeAssert(!str.Value().empty());

    if (str.Value().EqualsIAscii("Cascade"))
        {
        actionType = ActionType::Cascade;
        return SUCCESS;
        }

    if (str.Value().EqualsIAscii("SetNull") || str.Value().EqualsIAscii("SET NULL"))
        {
        actionType = ActionType::SetNull;
        return SUCCESS;
        }

    if (str.Value().EqualsIAscii("SetDefault") || str.Value().EqualsIAscii("SET DEAULT"))
        {
        actionType = ActionType::SetDefault;
        return SUCCESS;
        }

    if (str.Value().EqualsIAscii("Restrict"))
        {
        actionType = ActionType::Restrict;
        return SUCCESS;
        }

    if (str.Value().EqualsIAscii("NoAction") || str.Value().EqualsIAscii("NO ACTION"))
        {
        actionType = ActionType::NoAction;
        return SUCCESS;
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ForeignKeyDbConstraint::Remove(Utf8CP fkColumnName, Utf8CP referencedTableColumnName)
    {
    bool cont;
    bool hasFkColumnName = !Utf8String::IsNullOrEmpty(fkColumnName);
    bool hasReferencedTableColumnName = !Utf8String::IsNullOrEmpty(referencedTableColumnName);
    size_t size = m_fkColumns.size();
    do
        {
        cont = false;
        for (size_t i = 0; i < m_fkColumns.size(); i++)
            {
            if (hasFkColumnName && !hasReferencedTableColumnName)
                {
                if (m_fkColumns[i]->GetName() == fkColumnName)
                    {
                    Remove(i);
                    cont = true;
                    break;
                    }
                }
            else if (!hasFkColumnName && hasReferencedTableColumnName)
                {
                if (m_referencedTableColumns[i]->GetName() == referencedTableColumnName)
                    {
                    Remove(i);
                    cont = true;
                    break;
                    }
                }
            else if (hasFkColumnName && hasReferencedTableColumnName)
                {
                if (m_fkColumns[i]->GetName() == fkColumnName && m_referencedTableColumns[i]->GetName() == referencedTableColumnName)
                    {
                    Remove(i);
                    cont = true;
                    break;
                    }
                }
            }
        } while (cont);

        return size != m_fkColumns.size() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ForeignKeyDbConstraint::Remove(size_t index)
    {
    if (index >= m_fkColumns.size())
        return ERROR;

    m_fkColumns.erase(m_fkColumns.begin() + index);
    m_referencedTableColumns.erase(m_referencedTableColumns.begin() + index);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
void ForeignKeyDbConstraint::RemoveIfDuplicate()
    {
    if (IsDuplicate())
        const_cast<DbTable&>(GetForeignKeyTable()).RemoveConstraint(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   03/2016
//---------------------------------------------------------------------------------------
bool ForeignKeyDbConstraint::IsValid() const
    {
    return GetReferencedTable().IsValid() && !m_fkColumns.empty() && m_fkColumns.size() == m_referencedTableColumns.size();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
bool ForeignKeyDbConstraint::IsDuplicate() const
    {
    for (DbConstraint const* constraint : GetForeignKeyTable().GetConstraints())
        {
        if (constraint->GetType() == DbConstraint::Type::ForeignKey)
            {
            ForeignKeyDbConstraint const* fkConstraint = static_cast<ForeignKeyDbConstraint const*>(constraint);
            if (fkConstraint == this)
                continue;

            if (Equals(*fkConstraint))
                return true;
            }
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
bool ForeignKeyDbConstraint::Equals(ForeignKeyDbConstraint const& rhs) const
    {
    if (&rhs == this)
        return true;

    if (rhs.m_onDeleteAction != m_onDeleteAction)
        return false;

    if (rhs.m_onUpdateAction != m_onUpdateAction)
        return false;

    if (rhs.m_fkColumns.size() != m_fkColumns.size())
        return false;

    if (&rhs.GetForeignKeyTable() != &GetForeignKeyTable())
        return false;

    if (&rhs.GetReferencedTable() != &GetReferencedTable())
        return false;

    std::set<DbColumn const*> rhsFkColumns = std::set<DbColumn const*>(rhs.m_fkColumns.begin(), rhs.m_fkColumns.end());
    std::set<DbColumn const*> rhsReferencedTableColumns = std::set<DbColumn const*>(rhs.m_referencedTableColumns.begin(), rhs.m_referencedTableColumns.end());
    for (DbColumn const* col : m_fkColumns)
        {
        if (rhsFkColumns.find(col) == rhsFkColumns.end())
            return false;
        }

    for (DbColumn const* col : m_referencedTableColumns)
        {
        if (rhsReferencedTableColumns.find(col) == rhsReferencedTableColumns.end())
            return false;
        }

    return true;
    }



END_BENTLEY_SQLITE_EC_NAMESPACE