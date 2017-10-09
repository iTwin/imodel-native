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
DbTable const* DbSchema::FindTable(Utf8CP name) const
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
    DbTable const* table = m_tables.Get(id);
    if (table != nullptr)
        return table;

    return LoadTable(id);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbTable* DbSchema::FindTableP(Utf8CP name) const
    {
    DbTable const* table = m_tables.Get(name);
    if (table != nullptr)
        return const_cast<DbTable*>(table);

    return LoadTable(name);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbTable* DbSchema::CreateTable(Utf8StringCR name, DbTable::Type tableType, ECClassId exclusiveRootClassId, DbTable const* parentTable)
    {
    if (name.empty())
        {
        BeAssert(false && "CreateTable expects table name to be not empty.");
        return nullptr;
        }

    if (m_tables.Contains(name))
        {
        BeAssert(false && "Table with same name already exists");
        return nullptr;
        }

    DbTableId tableId;
    m_ecdb.GetImpl().GetSequence(IdSequences::Key::TableId).GetNextValue(tableId);
    return m_tables.Add(tableId, name, tableType, exclusiveRootClassId, parentTable);
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
        DbTable* table = FindTableP(tableName.c_str());
        bset<Utf8StringCP, CompareIUtf8Ascii> oldColumnList;
        bmap<Utf8StringCP, SqliteColumnInfo const*, CompareIUtf8Ascii> newColumnList;
        bvector<SqliteColumnInfo> dbColumnList;
        if (SUCCESS != DbSchemaPersistenceManager::RunPragmaTableInfo(dbColumnList, m_ecdb, tableName))
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

        for (Utf8StringCP addColumn : added)
            {
            auto itor = newColumnList.find(addColumn);
            if (table->CreateColumn(*addColumn, itor->second->GetType(), DbColumn::Kind::Default, PersistenceType::Physical) == nullptr)
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
    //Following return the list of table and there id from db
    bmap<Utf8String, DbTableId, CompareIUtf8Ascii> persistedTableMap = DbSchemaPersistenceManager::GetTableDefNamesAndIds(m_ecdb);
    for (DbTable const* table : m_tables.GetTablesInDependencyOrder())
        {
        // This would be null in case a table is not loaded yet and if its not loaded then we do not need to update it
        if (table == nullptr)
            continue;

        if (SUCCESS != table->GetLinkNode().Validate())
            return ERROR;

        auto itor = persistedTableMap.find(table->GetName());
        if (itor == persistedTableMap.end())
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

    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO ec_Table(Id,Name,Type,ExclusiveRootClassId,ParentTableId) VALUES(?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindId(1, table.GetId());
    stmt->BindText(2, table.GetName(), Statement::MakeCopy::No);
    stmt->BindInt(3, Enum::ToInt(table.GetType()));
    if (table.HasExclusiveRootECClass())
        stmt->BindId(4, table.GetExclusiveRootECClassId());

    DbTable::LinkNode const* parentNode = table.GetLinkNode().GetParent();
    if (parentNode != nullptr)
        stmt->BindId(5, parentNode->GetTable().GetId());

    DbResult stat = stmt->Step();
    if (stat != BE_SQLITE_DONE)
        return ERROR;

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

    bmap<Utf8String, DbColumnId, CompareIUtf8Ascii> persistedColumnMap = DbSchemaPersistenceManager::GetColumnNamesAndIds(m_ecdb, table.GetId());
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("UPDATE ec_Table SET Name=?,Type=?,ParentTableId=? WHERE Id=?");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindText(1, table.GetName(), Statement::MakeCopy::No);
    stmt->BindInt(2, Enum::ToInt(table.GetType()));
    DbTable::LinkNode const* parentNode = table.GetLinkNode().GetParent();
    if (parentNode != nullptr)
        stmt->BindId(3, parentNode->GetTable().GetId());
    else
        stmt->BindNull(3);

    stmt->BindId(4, table.GetId());

    DbResult stat = stmt->Step();
    if (stat != BE_SQLITE_DONE)
        return ERROR;

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
BentleyStatus DbSchema::InsertColumn(DbColumn const& column, int columnOrdinal, int primaryKeyOrdinal) const
    {
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO ec_Column(Id, TableId, Name, Type, IsVirtual, Ordinal, NotNullConstraint, UniqueConstraint, CheckConstraint, DefaultConstraint, CollationConstraint, OrdinalInPrimaryKey, ColumnKind) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindId(1, column.GetId());
    stmt->BindId(2, column.GetTable().GetId());
    stmt->BindText(3, column.GetName(), Statement::MakeCopy::No);
    stmt->BindInt(4, Enum::ToInt(column.GetType()));
    stmt->BindBoolean(5, column.GetPersistenceType() == PersistenceType::Virtual);
    stmt->BindInt64(6, columnOrdinal);
    stmt->BindBoolean(7, column.GetConstraints().HasNotNullConstraint());
    stmt->BindBoolean(8, column.GetConstraints().HasUniqueConstraint());

    if (!column.GetConstraints().GetCheckConstraint().empty())
        stmt->BindText(9, column.GetConstraints().GetCheckConstraint(), Statement::MakeCopy::No);

    if (!column.GetConstraints().GetDefaultValueConstraint().empty())
        stmt->BindText(10, column.GetConstraints().GetDefaultValueConstraint(), Statement::MakeCopy::No);

    stmt->BindInt(11, Enum::ToInt(column.GetConstraints().GetCollation()));
    if (primaryKeyOrdinal > -1)
        stmt->BindInt(12, primaryKeyOrdinal);

    stmt->BindInt(13, Enum::ToInt(column.GetKind()));

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
        const DbColumn::Type type = Enum::FromInt<DbColumn::Type>(stmt->GetValueInt(2));
        const PersistenceType persistenceType = stmt->GetValueBoolean(3) ? PersistenceType::Virtual : PersistenceType::Physical;

        const bool hasNotNullConstraint = stmt->GetValueBoolean(notNullColIx);
        const bool hasUniqueConstraint = stmt->GetValueBoolean(uniqueColIx);
        Utf8CP constraintCheck = !stmt->IsColumnNull(checkColIx) ? stmt->GetValueText(checkColIx) : nullptr;
        Utf8CP constraintDefault = !stmt->IsColumnNull(defaultValueColIx) ? stmt->GetValueText(defaultValueColIx) : nullptr;
        const DbColumn::Constraints::Collation collationConstraint = Enum::FromInt<DbColumn::Constraints::Collation>(stmt->GetValueInt(collationColIx));

        int primaryKeyOrdinal = stmt->IsColumnNull(pkOrdinalColIx) ? -1 : stmt->GetValueInt(pkOrdinalColIx);
        const DbColumn::Kind columnKind = Enum::FromInt<DbColumn::Kind>(stmt->GetValueInt(kindColIx));

        DbColumn* column = table.CreateColumn(id, Utf8String(name), type, columnKind, persistenceType);
        if (column == nullptr)
            {
            BeAssert(false);
            return SUCCESS;
            }

        if (hasNotNullConstraint)
            column->GetConstraintsR().SetNotNullConstraint();

        if (hasUniqueConstraint)
            column->GetConstraintsR().SetUniqueConstraint();

        column->GetConstraintsR().SetCollation(collationConstraint);

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
        if (SUCCESS != table.CreatePrimaryKeyConstraint(pkColumns, &pkOrdinals))
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
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("SELECT Name, Type, ExclusiveRootClassId, ParentTableId FROM ec_Table WHERE Id=?");
    if (stmt == nullptr)
        return nullptr;

    stmt->BindId(1, tableId);
    if (stmt->Step() != BE_SQLITE_ROW)
        return nullptr;

    Utf8CP name = stmt->GetValueText(0);
    DbTable::Type tableType = Enum::FromInt<DbTable::Type>(stmt->GetValueInt(1));
    ECClassId exclusiveRootClassId;
    if (!stmt->IsColumnNull(2))
        exclusiveRootClassId = stmt->GetValueId<ECClassId>(2);

    DbTable const* parentTable = nullptr;
    if (!stmt->IsColumnNull(3))
        {
        BeAssert((DbTable::Type::Joined == tableType || DbTable::Type::Overflow == tableType) && "Expecting joined or overflow table if parent table id is not null");
        DbTableId parentTableId = stmt->GetValueId<DbTableId>(3);
        parentTable = FindTable(parentTableId);
        BeAssert(parentTable != nullptr && "Failed to find parent table");
        }

    DbTable* table = const_cast<TableCollection&>(m_tables).Add(tableId, name, tableType, exclusiveRootClassId, parentTable);

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

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbTable* DbSchema::LoadTable(Utf8StringCR name) const
    {
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("SELECT Id, Type, ExclusiveRootClassId, ParentTableId FROM ec_Table WHERE Name=?");
    if (stmt == nullptr)
        return nullptr;

    stmt->BindText(1, name, Statement::MakeCopy::No);
    if (stmt->Step() != BE_SQLITE_ROW)
        return nullptr;

    DbTableId id = stmt->GetValueId<DbTableId>(0);
    DbTable::Type tableType = Enum::FromInt<DbTable::Type>(stmt->GetValueInt(1));
    ECClassId exclusiveRootClassId;
    if (!stmt->IsColumnNull(2))
        exclusiveRootClassId = stmt->GetValueId<ECClassId>(2);

    DbTable const* parentTable = nullptr;
    if (!stmt->IsColumnNull(3))
        {
        BeAssert((DbTable::Type::Joined == tableType || DbTable::Type::Overflow == tableType) && "Expecting joined or overflow table if parent table id is not null");
        DbTableId parentTableId = stmt->GetValueId<DbTableId>(3);
        parentTable = FindTable(parentTableId);
        BeAssert(parentTable != nullptr && "Failed to find parent table");
        }

    DbTable* table = const_cast<TableCollection&>(m_tables).Add(id, name, tableType, exclusiveRootClassId, parentTable);
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

// -------------------------------------------------------------------------------------- -
// @bsimethod                                                    Affan.Khan        09/2014
//--------------------------------------------------------------------------------------
BentleyStatus DbSchema::LoadIndexDefs() const
    {
    if (m_indexDefsAreLoaded)
        return SUCCESS;

    CachedStatementPtr stmt = GetECDb().GetImpl().GetCachedSqliteStatement("SELECT I.Id, T.Name, I.Name, I.IsUnique, I.AddNotNullWhereExp, I.IsAutoGenerated, I.ClassId, I.AppliesToSubclassesIfPartial FROM " TABLE_Index " I INNER JOIN ec_Table T ON T.Id = I.TableId");
    if (stmt == nullptr)
        return ERROR;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        DbIndexId id = stmt->GetValueId<DbIndexId>(0);
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

        if (nullptr == table->AddIndexDef(id, Utf8String(name), isUnique, columns, addNotNullWhereExp, isAutoGenerated, classId, appliesToSubclassesIfPartial))
            return ERROR;
        }

    m_indexDefsAreLoaded = true;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbSchema::PersistIndexDef(DbIndex const& index) const
    {
    CachedStatementPtr stmt = GetECDb().GetImpl().GetCachedSqliteStatement("INSERT INTO " TABLE_Index "(Id,TableId,Name,IsUnique,AddNotNullWhereExp,IsAutoGenerated,ClassId,AppliesToSubclassesIfPartial) VALUES(?,?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindId(1, index.GetId());
    stmt->BindId(2, index.GetTable().GetId());
    stmt->BindText(3, index.GetName(), Statement::MakeCopy::No);
    stmt->BindBoolean(4, index.GetIsUnique());
    stmt->BindBoolean(5, index.IsAddColumnsAreNotNullWhereExp());

    stmt->BindBoolean(6, index.IsAutoGenerated());
    if (index.HasClassId())
        stmt->BindId(7, index.GetClassId());

    stmt->BindBoolean(8, index.AppliesToSubclassesIfPartial());

    if (BE_SQLITE_DONE != stmt->Step())
        {
        m_ecdb.GetImpl().Issues().Report("Failed to persist definition for index %s on table %s: %s",
                                         index.GetName().c_str(), index.GetTable().GetName().c_str(), GetECDb().GetLastError().c_str());
        return ERROR;
        }

    stmt = nullptr; //free resources

    CachedStatementPtr indexColStmt = GetECDb().GetImpl().GetCachedSqliteStatement("INSERT INTO " TABLE_IndexColumn "(Id,IndexId,ColumnId,Ordinal) VALUES(?,?,?,?)");
    if (indexColStmt == nullptr)
        return ERROR;

    int i = 0;
    for (DbColumn const* col : index.GetColumns())
        {
        BeInt64Id id;
        if (GetECDb().GetImpl().GetSequence(IdSequences::Key::IndexColumnId).GetNextValue(id))
            return ERROR;

        if (BE_SQLITE_OK != indexColStmt->BindId(1, id) ||
            BE_SQLITE_OK != indexColStmt->BindId(2, index.GetId()) ||
            BE_SQLITE_OK != indexColStmt->BindId(3, col->GetId()) ||
            BE_SQLITE_OK != indexColStmt->BindInt(4, i))
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
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static
DbSchema::EntityType DbSchema::GetEntityType(ECDbCR ecdb, Utf8CP name)
    {
    CachedStatementPtr stmt = ecdb.GetImpl().GetCachedSqliteStatement("SELECT type FROM sqlite_master WHERE name=?");
    BeAssert(stmt != nullptr);
    stmt->BindText(1, name, Statement::MakeCopy::No);
    if (stmt->Step() == BE_SQLITE_ROW)
        {
        Utf8CP entityType = stmt->GetValueText(0);
        if (BeStringUtilities::StricmpAscii(entityType, "table") == 0)
            return EntityType::Table;

        if (BeStringUtilities::StricmpAscii(entityType, "view") == 0)
            return EntityType::View;

        if (BeStringUtilities::StricmpAscii(entityType, "index") == 0)
            return EntityType::Index;

        if (BeStringUtilities::StricmpAscii(entityType, "trigger") == 0)
            return EntityType::Trigger;
        }

    return EntityType::None;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     07/2017
//---------------------------------------------------------------------------------------
//static
bool DbSchema::ExistsInDb(ECDbCR ecdb, Utf8CP dbObjectName)
    {
    CachedStatementPtr stmt = ecdb.GetImpl().GetCachedSqliteStatement("SELECT NULL FROM sqlite_master WHERE name=? COLLATE NOCASE");
    if (stmt == nullptr)
        {
        BeAssert(stmt != nullptr);
        return false;
        }

    stmt->BindText(1, dbObjectName, Statement::MakeCopy::No);
    return stmt->Step() == BE_SQLITE_ROW;
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
            m_nullTable = const_cast<DbSchema*>(this)->CreateTable(DBSCHEMA_NULLTABLENAME, DbTable::Type::Virtual, ECClassId(), nullptr);

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
    auto it = m_tableNamesById.find(tableId);
    if (it == m_tableNamesById.end())
        return nullptr;

    return Get(*it->second);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   07/2017
//---------------------------------------------------------------------------------------
DbTable* DbSchema::TableCollection::Add(DbTableId tableId, Utf8StringCR name, DbTable::Type tableType, ECClassId exclusiveRootClassId, DbTable const* parentTable)
    {
    if (name.empty() || !tableId.IsValid())
        {
        BeAssert(false && "Table name cannot be empty, table id must be valid");
        return nullptr;
        }

    std::unique_ptr<DbTable> table = std::make_unique<DbTable>(m_ecdb, tableId, name, tableType, exclusiveRootClassId, parentTable);
    if (tableType == DbTable::Type::Existing)
        table->GetEditHandleR().EndEdit(); //we do not want this table to be editable;

    DbTable* tableP = table.get();
    m_tableMapByName[tableP->GetName()] = std::move(table);
    m_tableNamesById[tableP->GetId()] = &tableP->GetName();
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
        m_tableNamesById.erase(m_tableNamesById.find(it->second->GetId()));

    m_tableMapByName.erase(it);
    }

//****************************************************************************************
//DbTable
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   05/2016
//---------------------------------------------------------------------------------------
DbTable::DbTable(ECDbCR ecdb, DbTableId id, Utf8StringCR name, Type tableType, ECN::ECClassId exclusiveRootClass, DbTable const* parentTable)
    : m_ecdb(ecdb), m_id(id), m_name(name), m_type(tableType), m_exclusiveRootECClassId(exclusiveRootClass), m_linkNode(*this, parentTable)
    {
    if (tableType != Type::Existing && tableType != Type::Virtual)
        m_sharedColumnNameGenerator = DbSchemaNameGenerator(GetSharedColumnNamePrefix(tableType));

    BeAssert(m_linkNode.Validate() == SUCCESS);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   05/2016
//---------------------------------------------------------------------------------------
BentleyStatus DbTable::CreatePrimaryKeyConstraint(std::vector<DbColumn*> const& pkColumns, std::vector<size_t> const* pkOrdinals)
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
ForeignKeyDbConstraint const* DbTable::CreateForeignKeyConstraint(DbColumn const& fkColumn, DbColumn const& referencedColumn, ForeignKeyDbConstraint::ActionType onDeleteAction, ForeignKeyDbConstraint::ActionType onUpdateAction)
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
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbIndex* DbTable::AddIndexDef(DbIndexId id, Utf8StringCR indexName, bool isUnique, std::vector<DbColumn const*> const& columns, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId classId, bool applyToSubclassesIfPartial)
    {
    std::unique_ptr<DbIndex> index = std::make_unique<DbIndex>(id, *this, indexName, isUnique, columns, addIsNotNullWhereExp, isAutoGenerated, classId, applyToSubclassesIfPartial);
    DbIndex* indexP = index.get();
    m_indexes.push_back(std::move(index));
    return indexP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                          muhammad.zaighum                           01/2015
//---------------------------------------------------------------------------------------
BentleyStatus DbTable::CreateTrigger(Utf8CP triggerName, DbTrigger::Type type, Utf8CP condition, Utf8CP body)
    {
    if (m_type == Type::Existing)
        {
        BeAssert(false);
        return ERROR;
        }

    if (m_triggers.find(triggerName) == m_triggers.end())
        {
        std::unique_ptr<DbTrigger> trigger = std::unique_ptr<DbTrigger>(new DbTrigger(triggerName, *this, type, condition, body));
        DbTrigger* triggerP = trigger.get();
        m_triggers[triggerP->GetName()] = std::move(trigger);
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
DbColumn* DbTable::CreateColumn(DbColumnId id, Utf8StringCR colName, DbColumn::Type type, int position, DbColumn::Kind kind, PersistenceType persistenceType)
    {
    if (colName.empty())
        {
        BeAssert(false && "DbTable::CreateColumn cannot be called if column name is null or empty");
        return nullptr;
        }

    if (FindColumn(colName.c_str()) != nullptr)
        {
        BeAssert(false && "DbTable::CreateColumn> Column with specified name already exist");
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

    if (GetType() == DbTable::Type::Virtual)
        {
        //!Force column to be virtual
        persistenceType = PersistenceType::Virtual;
        }

    if (!id.IsValid())
        m_ecdb.GetImpl().GetSequence(IdSequences::Key::ColumnId).GetNextValue(id);

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
DbColumn* DbTable::CreateSharedColumn()
    {
    Utf8String generatedName;
    m_sharedColumnNameGenerator.Generate(generatedName);
    BeAssert(FindColumn(generatedName.c_str()) == nullptr);
    return CreateColumn(generatedName, DbColumn::Type::Any, DbColumn::Kind::SharedData, PersistenceType::Physical);
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
const std::vector<DbColumn const*> DbTable::FindAll(PersistenceType persistenceType) const
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
const  std::vector<DbColumn const*> DbTable::FindAll(DbColumn::Kind kind) const
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
    switch (GetType())
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
                Type type = m_children[0]->GetType();
                for (size_t i = 1; i < m_children.size(); i++)
                    {
                    if (m_children[i]->GetType() != type)
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
            if (m_parent == nullptr || m_parent->GetType() != Type::Primary)
                {
                BeAssert(false && "Joined table must have a parent table and it must be of type 'Primary'");
                return ERROR;
                }

            if (!m_children.empty())
                {
                if (m_children.size() > 1 || m_children[0]->GetType() != Type::Overflow)
                    {
                    BeAssert(false && "Joined table can only have a single child table at most and it must be an overflow table");
                    return ERROR;
                    }
                }

            break;
            }

            case Type::Overflow:
            {
            if (m_parent == nullptr)
                {
                BeAssert(false && "Overflow table must have a parent table");
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
    BeAssert(GetType() != Type::Overflow);
    if (m_children.empty())
        return nullptr;

    LinkNode const* nextNode = m_children[0];
    return nextNode->GetType() == Type::Overflow ? nextNode : nullptr;
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
    for (auto constraint : GetForeignKeyTable().GetConstraints())
        {
        if (constraint->GetType() == DbConstraint::Type::ForeignKey)
            {
            auto fkConstraint = static_cast<ForeignKeyDbConstraint const*>(constraint);
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
    for (auto col : m_fkColumns)
        {
        if (rhsFkColumns.find(col) == rhsFkColumns.end())
            return false;
        }

    for (auto col : m_referencedTableColumns)
        {
        if (rhsReferencedTableColumns.find(col) == rhsReferencedTableColumns.end())
            return false;
        }

    return true;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE