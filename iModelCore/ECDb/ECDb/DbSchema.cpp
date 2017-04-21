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
DbTable* DbSchema::CreateTable(Utf8StringCR name, DbTable::Type tableType, PersistenceType persType, ECClassId exclusiveRootClassId, DbTable const* primaryTable)
    {
    if (tableType == DbTable::Type::Existing)
        {
        if (name.empty())
            {
            BeAssert(false && "Existing table name cannot be null or empty");
            return nullptr;
            }

        if (!m_ecdb.TableExists(name.c_str()))
            {
            LOG.errorv("Table '%s' specified in ClassMap custom attribute must exist if MapStrategy is ExistingTable.", name.c_str());
            return nullptr;
            }
        }

    Utf8String finalName;
    if (!name.empty())
        {
        if (IsTableNameInUse(name))
            {
            BeAssert(false && "Table with same name already exists");
            return nullptr;
            }

        finalName.assign(name);
        }
    else
        {
        do
            {
            m_nameGenerator.Generate(finalName);
            } while (IsTableNameInUse(finalName));
        }

    DbTableId tableId;
    m_ecdb.GetECDbImplR().GetSequence(IdSequences::Key::TableId).GetNextValue(tableId);
    return CreateTable(tableId, finalName, tableType, persType, exclusiveRootClassId, primaryTable);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbTable* DbSchema::CreateTable(DbTableId tableId, Utf8StringCR name, DbTable::Type tableType, PersistenceType persType, ECClassId exclusiveRootClassId, DbTable const* primaryTable)
    {
    if (name.empty() || !tableId.IsValid())
        {
        BeAssert(false && "Table name cannot be empty, table id must be valid");
        return nullptr;
        }

    std::unique_ptr<DbTable> table(std::unique_ptr<DbTable>(new DbTable(tableId, name, *this, persType, tableType, exclusiveRootClassId, primaryTable)));
    if (tableType == DbTable::Type::Existing)
        table->GetEditHandleR().EndEdit(); //we do not want this table to be editable;

    DbTable* tableP = table.get();
    m_tableMapByName[tableP->GetName()] = std::move(table);
    m_tableMapById[tableId] = tableP->GetName();
    return tableP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbSchema::SynchronizeExistingTables()
    {
    SyncTableCache();

    bvector<DbTable const*> tables;
    for (bpair<Utf8String, DbTableId> const& tableKey : GetExistingTableMap())
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
            if (table->CreateColumn(*addColumn, itor->second->GetType(), DbColumn::Kind::DataColumn, PersistenceType::Physical) == nullptr)
                {
                BeAssert("Failed to create column");
                return ERROR;
                }
            }

        table->GetEditHandleR().EndEdit();
        if (UpdateTable(*table) != SUCCESS)
            return SUCCESS;
        }

   
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
bool DbSchema::IsTableNameInUse(Utf8StringCR tableName) const
    {
    SyncTableCache();
    return m_tableMapByName.find(tableName) != m_tableMapByName.end();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbTable const* DbSchema::FindTable(Utf8CP name) const
    {
    SyncTableCache();
    auto itor = m_tableMapByName.find(name);
    if (itor != m_tableMapByName.end())
        {
        if (itor->second == nullptr)
            {
            //OnDemand loading
            DbTable* table;
            if (LoadTable(itor->first, table) == ERROR)
                return nullptr;

            return table;
            }

        return itor->second.get();
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbTable const* DbSchema::FindTable(DbTableId id) const
    {
    SyncTableCache();
    auto itor = m_tableMapById.find(id);
    if (itor != m_tableMapById.end())
        {
        return FindTable(itor->second.c_str());
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbTable* DbSchema::FindTableP(Utf8CP name) const
    {
    SyncTableCache();
    auto itor = m_tableMapByName.find(name);
    if (itor != m_tableMapByName.end())
        {
        if (itor->second == nullptr)
            {
            //OnDemand loading
            DbTable* table;
            if (LoadTable(itor->first, table) == ERROR)
                return nullptr;

            return table;
            }

        return itor->second.get();
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbIndex* DbSchema::CreateIndex(DbTable& table, Nullable<Utf8String> const& indexName, bool isUnique, std::vector<DbColumn const*> const& columns, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId classId, bool applyToSubclassesIfPartial)
    {    
    if (columns.empty())
        {
        BeAssert(false && "Index must have at least one column defined.");
        return nullptr;
        }

    Utf8String generatedIndexName;
    if (!indexName.IsNull())
        generatedIndexName.assign(indexName.Value());
    else
        {
        do
            {
            m_nameGenerator.Generate(generatedIndexName);
            } while (m_usedIndexNames.find(generatedIndexName.c_str()) != m_usedIndexNames.end() || IsTableNameInUse(generatedIndexName.c_str()));
        }

    DbIndexId id;
    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetSequence(IdSequences::Key::IndexId).GetNextValue(id))
        {
        BeAssert(false);
        return nullptr;
        }

    return CreateIndex(id, table, generatedIndexName, isUnique, columns, addIsNotNullWhereExp, isAutoGenerated, classId, applyToSubclassesIfPartial);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbIndex* DbSchema::CreateIndex(DbTable& table, Nullable<Utf8String> const& indexName, bool isUnique, std::vector<Utf8CP> const& columnNames, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId classId, bool applyToSubclassesIfPartial)
    {
    if (columnNames.empty())
        return nullptr;

    std::vector<DbColumn const*> columns;
    for (Utf8CP colName : columnNames)
        {
        DbColumn const* col = table.FindColumn(colName);
        if (col == nullptr)
            {
            BeAssert(false && "Failed to find index column");
            return nullptr;
            }

        columns.push_back(col);
        }

    return CreateIndex(table, indexName, isUnique, columns, addIsNotNullWhereExp, isAutoGenerated, classId, applyToSubclassesIfPartial);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//--------------------------------------------------------------------------------------
BentleyStatus DbSchema::LoadIndexes() const
    {
    if (m_indexesLoaded)
        return SUCCESS;

    m_indexesLoaded = true;
    m_indexes.clear();
    m_usedIndexNames.clear();

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT I.Id, T.Name, I.Name, I.IsUnique, I.AddNotNullWhereExp, I.IsAutoGenerated, I.ClassId, I.AppliesToSubclassesIfPartial FROM " TABLE_ECIndex " I INNER JOIN ec_Table T ON T.Id = I.TableId");
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

        CachedStatementPtr indexColStmt = m_ecdb.GetCachedStatement("SELECT C.Name FROM " TABLE_ECIndexColumn " I INNER JOIN ec_Column C ON C.Id = I.ColumnId WHERE I.IndexId = ? ORDER BY I.Ordinal");
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

        if (nullptr ==  const_cast<DbSchema*>(this)->CreateIndex(id, *table, Utf8String(name), isUnique, columns, addNotNullWhereExp, isAutoGenerated, classId, appliesToSubclassesIfPartial))
            return ERROR;
        }

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
std::vector<std::unique_ptr<DbIndex>> const& DbSchema::GetIndexes() const
    {
    if (!m_indexesLoaded && LoadIndexes() == ERROR) 
        { 
        BeAssert(false); 
        }  
    
    return m_indexes;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbIndex* DbSchema::CreateIndex(DbIndexId id, DbTable& table, Utf8StringCR indexName, bool isUnique, std::vector<DbColumn const*> const& columns, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId classId, bool applyToSubclassesIfPartial)
    {
    BeAssert(!columns.empty());
    if (!m_indexesLoaded && LoadIndexes() == ERROR)
        return nullptr;

    auto it = m_usedIndexNames.find(indexName.c_str());
    if (it != m_usedIndexNames.end())
        {
        for (std::unique_ptr<DbIndex>& index : m_indexes)
            {
            if (index->GetName().EqualsIAscii(indexName))
                {
                if (&index->GetTable() == &table && index->GetIsUnique() == isUnique && index->IsAddColumnsAreNotNullWhereExp() == addIsNotNullWhereExp &&
                    index->IsAutoGenerated() == isAutoGenerated)
                    {
                    std::set<DbColumn const*> s1(index->GetColumns().begin(), index->GetColumns().end());
                    std::set<DbColumn const*> s2(columns.begin(), columns.end());
                    std::vector<DbColumn const*> v3;
                    std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), std::back_inserter(v3));

                    if (v3.size() == s1.size() && v3.size() == s2.size())
                        return index.get();
                    }
                }
            }
        m_ecdb.GetECDbImplR().GetIssueReporter().Report("Index with name '%s' already defined in the ECDb file.", indexName.c_str());
        return nullptr;
        }

    std::unique_ptr<DbIndex> index(new DbIndex(id, table, indexName, isUnique, columns, addIsNotNullWhereExp, isAutoGenerated, classId, applyToSubclassesIfPartial));
    DbIndex* indexP = index.get();
    m_indexes.push_back(std::move(index));

    m_usedIndexNames.insert(indexP->GetName().c_str());
    return indexP;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbSchema::SaveOrUpdateTables() const
    {
    //Following return the list of table and there id from db
    bmap<Utf8String, DbTableId, CompareIUtf8Ascii> persistedTableMap = GetPersistedTableMap();
    for (DbTable const* table : GetCachedTables())
        {
        // This would be null in case a table is not loaded yet and if its not loaded then we do not need to update it
        if (table != nullptr)
            {
            if (table->Validate(true) == ERROR)
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
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        04/2017
//---------------------------------------------------------------------------------------
DbTable* DbSchema::CreateJoinedTable(DbTable const& baseTable, Utf8CP joinedTableName, ECN::ECClassId exclusiveRootClassId)
    {
    if (baseTable.GetType() != DbTable::Type::Primary)
        {
        BeAssert(false && "Base table must be primary or joined table");
        return nullptr;
        }

    if (baseTable.GetPersistenceType() == PersistenceType::Virtual)
        {
        BeAssert(false && "Base table must not be virtual");
        return nullptr;
        }

    if (baseTable.GetDerivedTables().size() == 1 && baseTable.GetDerivedTables().front()->GetType() == DbTable::Type::Overflow)
        {
        BeAssert(false && "Base table have overflow derive table. Derive table can only be of one type");
        return nullptr;
        }

    if (baseTable.Validate(true) == ERROR)
        {
        return nullptr;
        }

    return CreateTable(joinedTableName, DbTable::Type::Joined, PersistenceType::Physical, exclusiveRootClassId, &baseTable);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        04/2017
//---------------------------------------------------------------------------------------
DbTable* DbSchema::CreateOverflowTable(DbTable const& baseTable)
    {
    if (!(baseTable.GetType() == DbTable::Type::Primary ||
          baseTable.GetType() == DbTable::Type::Joined))
        {
        BeAssert(false && "Base table must be primary or joined table");
        return nullptr;
        }

    if (baseTable.GetPersistenceType() == PersistenceType::Virtual)
        {
        BeAssert(false && "Base table must not be virtual");
        return nullptr;
        }

    if (!baseTable.GetDerivedTables().empty())
        {
        BeAssert(false && "Base table must not have any dervied table at this time");
        return nullptr;
        }

    if (baseTable.Validate(true) == ERROR)
        {
        return nullptr;
        }

    Utf8String name = baseTable.GetName();
    name.append("_Overflow");
    return CreateTable(name, DbTable::Type::Overflow, PersistenceType::Physical, ECClassId(), &baseTable);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
bmap<Utf8String, DbTableId, CompareIUtf8Ascii> DbSchema::GetPersistedTableMap() const
    {
    bmap<Utf8String, DbTableId, CompareIUtf8Ascii> persistedTableMap;
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT Name, Id FROM ec_Table");
    if (stmt == nullptr)
        {
        BeAssert(false);
        return persistedTableMap;
        }

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        persistedTableMap[stmt->GetValueText(0)] = stmt->GetValueId<DbTableId>(1);
        }

    return persistedTableMap;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
bmap<Utf8String, DbTableId, CompareIUtf8Ascii> DbSchema::GetExistingTableMap() const
    {
    bmap<Utf8String, DbTableId, CompareIUtf8Ascii> persistedTableMap;

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT Name, Id FROM ec_Table WHERE Type=" SQLVAL_DbTable_Type_Existing);
    if (stmt == nullptr)
        {
        BeAssert(false);
        return persistedTableMap;
        }

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        persistedTableMap[stmt->GetValueText(0)] = stmt->GetValueId<DbTableId>(1);
        }

    return persistedTableMap;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
bmap<Utf8String, DbColumnId, CompareIUtf8Ascii> DbSchema::GetPersistedColumnMap(DbTableId tableId) const
    {
    bmap<Utf8String, DbColumnId, CompareIUtf8Ascii> persistedColumnMap;
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT Name, Id FROM ec_Column WHERE TableId=?");
    if (stmt == nullptr)
        {
        BeAssert(false);
        return persistedColumnMap;
        }

    stmt->BindId(1, tableId);
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        persistedColumnMap[stmt->GetValueText(0)] = stmt->GetValueId<DbColumnId>(1);
        }

    return persistedColumnMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
void DbSchema::SyncTableCache() const
    {
    if (m_syncTableCacheNames)
        return;

    for (bpair<Utf8String, DbTableId> const& tableKey : GetPersistedTableMap())
        {
        if (m_tableMapByName.find(tableKey.first) == m_tableMapByName.end())
            {
            m_tableMapByName.insert(std::make_pair(tableKey.first, std::unique_ptr<DbTable>()));
            m_tableMapById.insert(bpair<DbTableId, Utf8String>(tableKey.second, tableKey.first));
            }
        }

    m_syncTableCacheNames = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
void DbSchema::Reset() const
    {
    m_nullTable = nullptr;
    m_tableMapByName.clear();
    m_tableMapById.clear();
    m_indexesLoaded = false;
    m_indexes.clear();
    m_usedIndexNames.clear();
    m_syncTableCacheNames = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
std::vector<DbTable const*> DbSchema::GetCachedTables() const
    {
    std::vector<DbTable const*> cachedTables;
    for (auto const& tableKey : m_tableMapByName)
        {
        if (tableKey.second != nullptr)
            {
            if (tableKey.second->GetType() == DbTable::Type::Joined)
                cachedTables.push_back(tableKey.second.get());
            else
                cachedTables.insert(cachedTables.begin(), tableKey.second.get());
            }
        }

    return cachedTables;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbSchema::InsertTable(DbTable const& table) const
    {
    if (!table.IsNullTable() && !table.IsValid())
        {
        BeAssert(false && "Table to insert is not valid");
        return ERROR;
        }

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("INSERT INTO ec_Table(Id,Name,Type,IsVirtual,ExclusiveRootClassId,ParentTableId) VALUES(?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindId(1, table.GetId());
    stmt->BindText(2, table.GetName().c_str(), Statement::MakeCopy::No);
    stmt->BindInt(3, Enum::ToInt(table.GetType()));
    stmt->BindBoolean(4, table.GetPersistenceType() == PersistenceType::Virtual);
    if (table.HasExclusiveRootECClass())
        stmt->BindId(5, table.GetExclusiveRootECClassId());

    DbTable const* parentTable = table.GetBaseTable();
    if (parentTable != nullptr)
        stmt->BindId(6, parentTable->GetId());

    DbResult stat = stmt->Step();
    if (stat != BE_SQLITE_DONE)
        return ERROR;

    bmap<DbColumn const*, int> primaryKeys;
    int i = 0;
    if (PrimaryKeyDbConstraint const* pkConstraint = table.GetPrimaryKeyConstraint())
        {
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
    if (!table.IsNullTable() && !table.IsValid())
        {
        BeAssert(false && "Table to insert is not valid");
        return ERROR;
        }

    bmap<Utf8String, DbColumnId, CompareIUtf8Ascii> persistedColumnMap = GetPersistedColumnMap(table.GetId());
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("UPDATE ec_Table SET Name=?,Type=?,IsVirtual=?,ParentTableId=? WHERE Id=?");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindId(5, table.GetId());
    stmt->BindText(1, table.GetName().c_str(), Statement::MakeCopy::No);
    stmt->BindInt(2, Enum::ToInt(table.GetType()));
    stmt->BindBoolean(3, table.GetPersistenceType() == PersistenceType::Virtual);
    DbTable const* parentTable = table.GetBaseTable();
    if (parentTable != nullptr)
        stmt->BindId(4, parentTable->GetId());
    else
        stmt->BindNull(4);

    DbResult stat = stmt->Step();
    if (stat != BE_SQLITE_DONE)
        return ERROR;

    bmap<DbColumn const*, int> primaryKeys;
    int i = 0;
    if (PrimaryKeyDbConstraint const* pkConstraint = table.GetPrimaryKeyConstraint())
        {
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
// @bsimethod                                                    Krischan.Eberle  08/2015
//---------------------------------------------------------------------------------------
BentleyStatus DbSchema::CreateOrUpdateIndexes() const
    {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("DELETE FROM " TABLE_ECIndex);
    if (stmt == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (stmt->Step() != BE_SQLITE_DONE)
        return ERROR;

    stmt = nullptr;

    bmap<Utf8String, DbIndex const*, CompareIUtf8Ascii> comparableIndexDefs;
    for (std::unique_ptr<DbIndex> const& indexPtr : GetIndexes())
        {
        DbIndex& index = *indexPtr;
        if (index.GetColumns().empty())
            {
            BeAssert(false && "Index definition is not valid");
            return ERROR;
            }

        //drop index first if it exists, as we always have to recreate them to make sure the class id filter is up-to-date
        Utf8String dropIndexSql;
        dropIndexSql.Sprintf("DROP INDEX [%s]", index.GetName().c_str());
        m_ecdb.TryExecuteSql(dropIndexSql.c_str());

        //indexes on virtual tables are ignored
        if (index.GetTable().GetPersistenceType() == PersistenceType::Physical)
            {
            Utf8String ddl, comparableIndexDef;
            if (SUCCESS != DbSchemaPersistenceManager::BuildCreateIndexDdl(ddl, comparableIndexDef, m_ecdb, index))
                return ERROR;

            auto it = comparableIndexDefs.find(comparableIndexDef);
            if (it != comparableIndexDefs.end())
                {
                Utf8CP errorMessage = "Index '%s'%s on table '%s' has the same definition as the already existing index '%s'%s. ECDb does not create this index.";

                Utf8String provenanceStr;
                if (index.HasClassId())
                    {
                    ECClassCP provenanceClass = m_ecdb.Schemas().GetClass(index.GetClassId());
                    if (provenanceClass == nullptr)
                        {
                        BeAssert(false);
                        return ERROR;
                        }
                    provenanceStr.Sprintf(" [Created for ECClass %s]", provenanceClass->GetFullName());
                    }

                DbIndex const* existingIndex = it->second;
                Utf8String existingIndexProvenanceStr;
                if (existingIndex->HasClassId())
                    {
                    ECClassCP provenanceClass = m_ecdb.Schemas().GetClass(existingIndex->GetClassId());
                    if (provenanceClass == nullptr)
                        {
                        BeAssert(false);
                        return ERROR;
                        }
                    existingIndexProvenanceStr.Sprintf(" [Created for ECClass %s]", provenanceClass->GetFullName());
                    }

                if (!index.IsAutoGenerated())
                    LOG.warningv(errorMessage, index.GetName().c_str(), provenanceStr.c_str(), index.GetTable().GetName().c_str(),
                                               existingIndex->GetName().c_str(), existingIndexProvenanceStr.c_str());
                else
                    {
                    if (LOG.isSeverityEnabled(NativeLogging::LOG_DEBUG))
                        LOG.debugv(errorMessage,
                                    index.GetName().c_str(), provenanceStr.c_str(), index.GetTable().GetName().c_str(),
                                    existingIndex->GetName().c_str(), existingIndexProvenanceStr.c_str());
                    }

                continue;
                }

            comparableIndexDefs[comparableIndexDef] = &index;

            if (BE_SQLITE_OK != m_ecdb.ExecuteSql(ddl.c_str()))
                {
                m_ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to create index %s on table %s. Error: %s", index.GetName().c_str(), index.GetTable().GetName().c_str(),
                                                                m_ecdb.GetLastError().c_str());
                BeAssert(false && "Failed to create index");
                return ERROR;
                }
            }

        //populates the ec_Index table (even for indexes on virtual tables, as they might be necessary
        //if further schema imports introduce subclasses of abstract classes (which map to virtual tables))
        if (SUCCESS != InsertIndex(index))
            return ERROR;
        }

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbSchema::UpdateColumn(DbColumn const& column, int columnOrdinal, int primaryKeyOrdinal) const
    {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("UPDATE ec_Column SET Name=?, Type=?, IsVirtual=?, Ordinal=?, NotNullConstraint=?, UniqueConstraint=?, CheckConstraint=?, DefaultConstraint=?, CollationConstraint=?, OrdinalInPrimaryKey=?, ColumnKind=? WHERE Id=?");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindId(12, column.GetId());
    stmt->BindText(1, column.GetName().c_str(), Statement::MakeCopy::No);
    stmt->BindInt(2, Enum::ToInt(column.GetType()));
    stmt->BindBoolean(3, column.GetPersistenceType() == PersistenceType::Virtual);
    stmt->BindInt64(4, columnOrdinal);
    stmt->BindBoolean(5, column.GetConstraints().HasNotNullConstraint());
    stmt->BindBoolean(6, column.GetConstraints().HasUniqueConstraint());

    if (!column.GetConstraints().GetCheckConstraint().empty())
        stmt->BindText(7, column.GetConstraints().GetCheckConstraint().c_str(), Statement::MakeCopy::No);

    if (!column.GetConstraints().GetDefaultValueConstraint().empty())
        stmt->BindText(8, column.GetConstraints().GetDefaultValueConstraint().c_str(), Statement::MakeCopy::No);

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
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("INSERT INTO ec_Column (Id, TableId, Name, Type, IsVirtual, Ordinal, NotNullConstraint, UniqueConstraint, CheckConstraint, DefaultConstraint, CollationConstraint, OrdinalInPrimaryKey, ColumnKind) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindId(1, column.GetId());
    stmt->BindId(2, column.GetTable().GetId());
    stmt->BindText(3, column.GetName().c_str(), Statement::MakeCopy::No);
    stmt->BindInt(4, Enum::ToInt(column.GetType()));
    stmt->BindBoolean(5, column.GetPersistenceType() == PersistenceType::Virtual);
    stmt->BindInt64(6, columnOrdinal);
    stmt->BindBoolean(7, column.GetConstraints().HasNotNullConstraint());
    stmt->BindBoolean(8, column.GetConstraints().HasUniqueConstraint());

    if (!column.GetConstraints().GetCheckConstraint().empty())
        stmt->BindText(9, column.GetConstraints().GetCheckConstraint().c_str(), Statement::MakeCopy::No);

    if (!column.GetConstraints().GetDefaultValueConstraint().empty())
        stmt->BindText(10, column.GetConstraints().GetDefaultValueConstraint().c_str(), Statement::MakeCopy::No);

    stmt->BindInt(11, Enum::ToInt(column.GetConstraints().GetCollation()));
    if (primaryKeyOrdinal > -1)
        stmt->BindInt(12, primaryKeyOrdinal);

    stmt->BindInt(13, Enum::ToInt(column.GetKind()));

    return stmt->Step() == BE_SQLITE_DONE ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbSchema::InsertIndex(DbIndex const& index) const
    {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("INSERT INTO " TABLE_ECIndex "(Id,TableId,Name,IsUnique,AddNotNullWhereExp,IsAutoGenerated,ClassId,AppliesToSubclassesIfPartial) VALUES(?,?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindId(1, index.GetId());
    stmt->BindId(2, index.GetTable().GetId());
    stmt->BindText(3, index.GetName().c_str(), Statement::MakeCopy::No);
    stmt->BindBoolean(4, index.GetIsUnique());
    stmt->BindBoolean(5, index.IsAddColumnsAreNotNullWhereExp());

    stmt->BindBoolean(6, index.IsAutoGenerated());
    if (index.HasClassId())
        stmt->BindId(7, index.GetClassId());

    stmt->BindBoolean(8, index.AppliesToSubclassesIfPartial());

    if (BE_SQLITE_DONE != stmt->Step())
        {
        LOG.errorv("Failed to insert index metadata into " TABLE_ECIndex " for index %s (Id: %s): %s",
                   index.GetName().c_str(), index.GetId().ToString().c_str(), m_ecdb.GetLastError().c_str());
        return ERROR;
        }

    stmt = nullptr; //free resources

    CachedStatementPtr indexColStmt = m_ecdb.GetCachedStatement("INSERT INTO " TABLE_ECIndexColumn "(Id,IndexId,ColumnId,Ordinal) VALUES(?,?,?,?)");
    if (indexColStmt == nullptr)
        return ERROR;

    int i = 0;
    for (DbColumn const* col : index.GetColumns())
        {
        BeInt64Id id;
        if (m_ecdb.GetECDbImplR().GetSequence(IdSequences::Key::IndexColumnId).GetNextValue(id))
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
            LOG.errorv("Failed to insert index column metadata into " TABLE_ECIndexColumn " for index %s (Id: %s) and column %s (Id: %s): %s",
                       index.GetName().c_str(), index.GetId().ToString().c_str(), col->GetName().c_str(), col->GetId().ToString().c_str(), m_ecdb.GetLastError().c_str());
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
BentleyStatus DbSchema::LoadColumns(DbTable& table) const
    {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT Id, Name, Type, IsVirtual, NotNullConstraint, UniqueConstraint, CheckConstraint, DefaultConstraint, CollationConstraint, OrdinalInPrimaryKey, ColumnKind FROM ec_Column WHERE TableId = ? ORDER BY Ordinal");
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
    int sharedColumnCount = 0;
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

        if (Enum::Contains(columnKind, DbColumn::Kind::SharedDataColumn))
            sharedColumnCount++;
        }

    if (!pkColumns.empty())
        {
        if (SUCCESS != table.CreatePrimaryKeyConstraint(pkColumns, &pkOrdinals))
            return SUCCESS;
        }

    table.InitializeSharedColumnNameGenerator(sharedColumnCount);

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbSchema::LoadTable(Utf8StringCR name, DbTable*& tableP) const
    {
    tableP = nullptr;
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT t.Id, t.Type, t.IsVirtual, t.ExclusiveRootClassId, parentT.Name FROM ec_Table t LEFT JOIN ec_Table parentT ON t.ParentTableId = parentT.Id WHERE t.Name = ?");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindText(1, name.c_str(), Statement::MakeCopy::No);
    if (stmt->Step() != BE_SQLITE_ROW)
        return ERROR;

    DbTableId id = stmt->GetValueId<DbTableId>(0);
    DbTable::Type tableType = Enum::FromInt<DbTable::Type>(stmt->GetValueInt(1));
    PersistenceType persistenceType = stmt->GetValueBoolean(2) ? PersistenceType::Virtual : PersistenceType::Physical;
    ECClassId exclusiveRootClassId;
    if (!stmt->IsColumnNull(3))
        exclusiveRootClassId = stmt->GetValueId<ECClassId>(3);

    Utf8CP parentTableName = stmt->GetValueText(4);

    DbTable const* parentTable = nullptr;
    if (!Utf8String::IsNullOrEmpty(parentTableName))
        {
        parentTable = FindTable(parentTableName);
        BeAssert(parentTable != nullptr && "Failed to find parent table of joined table");
        BeAssert(DbTable::Type::Joined == tableType && "Expecting JoinedTable");
        }

    DbTable* table = const_cast<DbSchema*>(this)->CreateTable(id, name.c_str(), tableType, persistenceType, exclusiveRootClassId, parentTable);
    if (table == nullptr)
        {
        BeAssert(false && "Failed to create table definition");
        return ERROR;
        }

    const bool canEdit = table->GetEditHandle().CanEdit();
    if (!canEdit)
        table->GetEditHandleR().BeginEdit();

    table->SetId(id);
    if (LoadColumns(*table) != SUCCESS)
        return ERROR;

    if (!canEdit)
        table->GetEditHandleR().EndEdit();

    tableP = table;
    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static
DbSchema::EntityType DbSchema::GetEntityType(ECDbCR ecdb, Utf8CP name)
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT type FROM sqlite_master WHERE name=?");
    BeAssert(stmt != nullptr);
    stmt->BindText(1, name, Statement::MakeCopy::No);
    if (stmt->Step() == BE_SQLITE_ROW)
        {
        if (BeStringUtilities::StricmpAscii(stmt->GetValueText(0), "table") == 0)
            return EntityType::Table;

        if (BeStringUtilities::StricmpAscii(stmt->GetValueText(0), "view") == 0)
            return EntityType::View;

        if (BeStringUtilities::StricmpAscii(stmt->GetValueText(0), "index") == 0)
            return EntityType::Index;

        if (BeStringUtilities::StricmpAscii(stmt->GetValueText(0), "trigger") == 0)
            return EntityType::Trigger;
        }

    return EntityType::None;
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
            m_nullTable = const_cast<DbSchema*>(this)->CreateTable(DBSCHEMA_NULLTABLENAME, DbTable::Type::Primary, PersistenceType::Virtual, ECClassId(), nullptr);

        if (m_nullTable != nullptr && m_nullTable->GetEditHandleR().CanEdit())
            m_nullTable->GetEditHandleR().EndEdit();
        }

    BeAssert(m_nullTable != nullptr);
    return m_nullTable;
    }

//****************************************************************************************
//DbTable
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   05/2016
//---------------------------------------------------------------------------------------
DbTable::DbTable(DbTableId id, Utf8StringCR name, DbSchema& dbSchema, PersistenceType type, Type tableType, ECN::ECClassId exclusiveRootClass, DbTable const* baseTable)
    : m_id(id), m_name(name), m_dbSchema(dbSchema), m_sharedColumnNameGenerator("sc%d"), m_persistenceType(type), m_type(tableType), m_exclusiveRootECClassId(exclusiveRootClass),
      m_pkConstraint(nullptr), m_classIdColumn(nullptr), m_baseTable(baseTable)
    {
    if (IsDerivedTable() && baseTable != nullptr)
        {
        const_cast<DbTable*>(baseTable)->m_derviedTables.push_back(this);
        }

    //!Assert must be called after above
    BeAssert(Validate(true) == SUCCESS);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan   04/2017
//---------------------------------------------------------------------------------------
BentleyStatus DbTable::Validate(bool bAssert) const
    {
    if (IsDerivedTable() && GetBaseTable() == nullptr)
        {
        BeAssert(!bAssert && "For dervied tables parent must not be null");
        return ERROR;
        }

    if (GetType() == Type::Primary && GetPersistenceType() == PersistenceType::Physical)
        {
        int overflow = 0;
        int joined = 0;
        for (DbTable const* derivedTable : GetDerivedTables())
            {
            if (derivedTable->GetType() == Type::Joined)
                joined++;
            else if (derivedTable->GetType() == Type::Overflow)
                overflow++;
            }

        if (overflow > 0 && joined > 0)
            {
            BeAssert(!bAssert && "Primary table can only have either one or more joined Tables or a single overflow table but not both");
            return ERROR;
            }

        if (overflow > 1 && joined == 0)
            {
            BeAssert(!bAssert && "Primary table can only have single dervied overflow table");
            return ERROR;
            }
        }

    if (GetBaseTable() != nullptr)
        {
        if (GetBaseTable()->GetType() != Type::Existing)
            {
            BeAssert(!bAssert && "Existing table cannot be use as base tables");
            return ERROR;
            }
        if (GetBaseTable()->GetType() != Type::Overflow)
            {
            BeAssert(!bAssert && "Overflow table cannot be use as base tables");
            return ERROR;
            }

        if (GetPersistenceType() == PersistenceType::Virtual)
            {
            BeAssert(!bAssert && "Derived table cannot be virtual");
            return ERROR;
            }

        if (GetBaseTable()->GetPersistenceType() == PersistenceType::Virtual)
            {
            BeAssert(!bAssert && "Derived table cannot have base virtual tables");
            return ERROR;
            }

        if (!IsDerivedTable())
            {
            BeAssert(!bAssert && "Base table can only be specified for derived tables");
            return ERROR;
            }
        
        if (GetType() == Type::Overflow && GetBaseTable()->GetType() != Type::Overflow)
            {
            BeAssert(!bAssert && "Overflow table cannot have base table that is of type existing");
            return ERROR;
            }

        if (GetType() == Type::Joined)
            {
            if (GetBaseTable()->GetType() != Type::Primary)
                {
                BeAssert(!bAssert && "JoinedTable can only have Primary as base table");
                return ERROR;
                }
            if (GetDerivedTables().size() > 1)
                {
                BeAssert(!bAssert && "Joined Table can only have single derived overflow table");
                return ERROR;
                }

            }
        if (GetType() == Type::Overflow && !GetDerivedTables().empty())
            {
            BeAssert(!bAssert && "Ovverflow table cannot have derived tables");
            return ERROR;
            }
        }


    return SUCCESS;
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
// @bsimethod                          muhammad.zaighum                           01/2015
//---------------------------------------------------------------------------------------
BentleyStatus DbTable::CreateTrigger(Utf8CP triggerName, DbTrigger::Type type, Utf8CP condition, Utf8CP body)
    {
    if (!IsOwnedByECDb())
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
        IssueReporter const& issues = m_dbSchema.GetECDb().GetECDbImplR().GetIssueReporter();
        if (m_type == Type::Existing)
            issues.Report("Cannot add columns to the existing table '%s' not owned by ECDb.", m_name.c_str());
        else
            {
            BeAssert(false && "Cannot add columns to read-only table.");
            issues.Report("Cannot add columns to the table '%s'. Table is not in edit mode.", m_name.c_str());
            }

        return nullptr;
        }

    PersistenceType resolvePersistenceType = persistenceType;
    if (GetPersistenceType() == PersistenceType::Virtual)
        resolvePersistenceType = PersistenceType::Virtual;

    if (!id.IsValid())
        m_dbSchema.GetECDb().GetECDbImplR().GetSequence(IdSequences::Key::ColumnId).GetNextValue(id);

    std::shared_ptr<DbColumn> newColumn = std::make_shared<DbColumn>(id, *this, colName, type, kind, resolvePersistenceType);
    DbColumn* newColumnP = newColumn.get();
    m_columns[newColumn->GetName().c_str()] = newColumn;

    if (position < 0)
        m_orderedColumns.push_back(newColumnP);
    else
        m_orderedColumns.insert(m_orderedColumns.begin() + (size_t) position, newColumnP);

    if (Enum::Contains(kind, DbColumn::Kind::ECClassId))
        m_classIdColumn = newColumnP;

    return newColumnP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbTable::DeleteColumn(DbColumn& col)
    {
    if (GetEditHandleR().AssertNotInEditMode())
        return ERROR;

    for (std::unique_ptr<DbConstraint>& constraint : m_constraints)
        {
        switch (constraint->GetType())
            {
                case DbConstraint::Type::ForeignKey:
                {
                ForeignKeyDbConstraint* fkc = static_cast<ForeignKeyDbConstraint*>(constraint.get());
                fkc->Remove(col.GetName().c_str(), nullptr);
                break;
                }

                case DbConstraint::Type::PrimaryKey:
                {
                PrimaryKeyDbConstraint const* pkc = static_cast<PrimaryKeyDbConstraint const*>(constraint.get());
                if (pkc->Contains(col))
                    {
                    BeAssert(false && "Cannot delete a column from a PK constraint");
                    return ERROR;
                    }

                break;
                }

                default:
                    BeAssert(false);
                    return ERROR;
            }
        }

    m_columns.erase(col.GetName().c_str());
    auto columnsAreEqual = [&col] (DbColumn const* column) { return column == &col; };
    m_orderedColumns.erase(std::find_if(m_orderedColumns.begin(), m_orderedColumns.end(), columnsAreEqual));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2016
//---------------------------------------------------------------------------------------
DbColumn* DbTable::CreateSharedColumn(DbColumn::Type colType)
    {
    Utf8String generatedName;
    m_sharedColumnNameGenerator.Generate(generatedName);
    BeAssert(FindColumn(generatedName.c_str()) == nullptr);

    return CreateColumn(generatedName, DbColumn::Type::Any, DbColumn::Kind::SharedDataColumn, PersistenceType::Physical);

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
BentleyStatus DbTable::GetFilteredColumnList(std::vector<DbColumn const*>& columns, PersistenceType persistenceType) const
    {
    for (DbColumn const* column : m_orderedColumns)
        {
        if (column->GetPersistenceType() == persistenceType)
            columns.push_back(column);
        }

    return columns.empty() ? ERROR : SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbTable::GetFilteredColumnList(std::vector<DbColumn const*>& columns, DbColumn::Kind kind) const
    {
    for (DbColumn const* column : m_orderedColumns)
        {
        if (Enum::Intersects(column->GetKind(), kind))
            columns.push_back(column);
        }

    return columns.empty() ? ERROR : SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbColumn const* DbTable::GetFilteredColumnFirst(DbColumn::Kind kind) const
    {
    for (DbColumn const* column : m_orderedColumns)
        {
        if (Enum::Intersects(column->GetKind(), kind))
            return column;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
bool DbTable::IsNullTable() const
    {
    return this == m_dbSchema.GetNullTable();
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
bool DbColumn::IsUnique() const
    {
    return m_constraints.HasUniqueConstraint() || IsOnlyColumnOfPrimaryKeyConstraint();
    }

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
        return BentleyStatus::ERROR;

    m_kind = kind;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbColumn::MakeNonVirtual()
    {
    if (m_table.GetEditHandleR().AssertNotInEditMode())
        return ERROR;

    if (m_persistenceType == PersistenceType::Physical)
        return SUCCESS;

    if (m_table.GetPersistenceType() == PersistenceType::Virtual)
        {
        BeAssert(false && "Virtual table cannot have persistence column");
        return ERROR;
        }

    m_persistenceType = PersistenceType::Physical;
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

    if (&this->GetForeignKeyTable() != &GetForeignKeyTable())
        return false;

    if (&this->GetReferencedTable() != &GetReferencedTable())
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



/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//static
DbTable* TableMapper::FindOrCreateTable(DbSchema& dbSchema, Utf8StringCR tableName, DbTable::Type tableType, bool isVirtual, Utf8StringCR primaryKeyColumnName, ECN::ECClassId exclusiveRootClassId, DbTable const* primaryTable)
    {
    BeAssert(!primaryKeyColumnName.empty() && "should always be set (either to user value or default value) by this time");

    DbTable* table = dbSchema.FindTableP(tableName.c_str());
    if (table != nullptr)
        {
        if (table->GetType() != tableType)
            {
            std::function<Utf8CP(bool)> toStr = [] (bool val) { return val ? "true" : "false"; };
            LOG.warningv("Multiple classes are mapped to the table %s although the classes require mismatching table metadata: "
                         "Metadata IsMappedToExistingTable: Expected=%s - Actual=%s. Actual value is ignored.",
                         tableName.c_str(),
                         toStr(tableType == DbTable::Type::Existing), toStr(!table->IsOwnedByECDb()));
            BeAssert(false && "ECDb uses a table for two classes although the classes require mismatching table metadata.");
            }

        if (table->HasExclusiveRootECClass())
            {
            BeAssert(table->GetExclusiveRootECClassId() != exclusiveRootClassId);
            dbSchema.GetECDb().GetECDbImplR().GetIssueReporter().Report("Table %s is exclusively used by the ECClass with Id %s and therefore "
                                                                        "cannot be used by other ECClasses which are no subclass of the mentioned ECClass.",
                                                                        tableName.c_str(), table->GetExclusiveRootECClassId().ToString().c_str());
            return nullptr;
            }

        if (exclusiveRootClassId.IsValid())
            {
            BeAssert(table->GetExclusiveRootECClassId() != exclusiveRootClassId);
            dbSchema.GetECDb().GetECDbImplR().GetIssueReporter().Report("The ECClass with Id %s requests exclusive use of the table %s, "
                                                                        "but it is already used by some other ECClass.",
                                                                        exclusiveRootClassId.ToString().c_str(), tableName.c_str());
            return nullptr;
            }

        return table;
        }

    if (tableType != DbTable::Type::Existing)
        return CreateTableForOtherStrategies(dbSchema, tableName, tableType, isVirtual, primaryKeyColumnName, exclusiveRootClassId, primaryTable);

    return CreateTableForExistingTableStrategy(dbSchema, tableName, primaryKeyColumnName, exclusiveRootClassId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle       11/2016
//---------------------------------------------------------------------------------------
//static
DbTable* TableMapper::CreateTableForOtherStrategies(DbSchema& dbSchema, Utf8StringCR tableName, DbTable::Type tableType, bool isVirtual, Utf8StringCR primaryKeyColumnName, ECN::ECClassId exclusiveRootClassId, DbTable const* primaryTable)
    {
    DbTable* table = dbSchema.CreateTable(tableName.c_str(), tableType, isVirtual ? PersistenceType::Virtual : PersistenceType::Physical, exclusiveRootClassId, primaryTable);
    
    DbColumn* pkColumn = table->CreateColumn(primaryKeyColumnName, DbColumn::Type::Integer, DbColumn::Kind::ECInstanceId, PersistenceType::Physical);
    if (table->GetPersistenceType() == PersistenceType::Physical)
        {
        std::vector<DbColumn*> pkColumns {pkColumn};
        if (SUCCESS != table->CreatePrimaryKeyConstraint(pkColumns))
            return nullptr;
        }

    //! We always create a virtual ECClassId column and later change it persistenceType to Persisted if required to.
    // Index is created on it later in FinishTableDefinition
    DbColumn* classIdColumn = table->CreateColumn(Utf8String(COL_ECClassId), DbColumn::Type::Integer, 1, DbColumn::Kind::ECClassId, PersistenceType::Virtual);
    if (classIdColumn == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    classIdColumn->GetConstraintsR().SetNotNullConstraint();

    return table;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static
DbTable* TableMapper::CreateTableForExistingTableStrategy(DbSchema& dbSchema, Utf8StringCR existingTableName, Utf8StringCR primaryKeyColName, ECClassId exclusiveRootClassId)
    {
    BeAssert(!existingTableName.empty());
    DbTable* table = dbSchema.CreateTable(existingTableName, DbTable::Type::Existing, PersistenceType::Physical, exclusiveRootClassId, nullptr);
    if (table == nullptr)
        return nullptr;

    bvector<SqliteColumnInfo> existingColumnInfos;
    if (SUCCESS != DbSchemaPersistenceManager::RunPragmaTableInfo(existingColumnInfos, dbSchema.GetECDb(), existingTableName))
        {
        BeAssert(false && "Failed to get column informations");
        return nullptr;
        }

    if (!table->GetEditHandle().CanEdit())
        table->GetEditHandleR().BeginEdit();

    DbColumn* idColumn = nullptr;
    std::vector<DbColumn*> pkColumns;
    std::vector<size_t> pkOrdinals;
    for (SqliteColumnInfo const& colInfo : existingColumnInfos)
        {
        DbColumn* column = table->CreateColumn(colInfo.GetName(), colInfo.GetType(), DbColumn::Kind::DataColumn, PersistenceType::Physical);
        if (column == nullptr)
            {
            BeAssert(false && "Failed to create column");
            return nullptr;
            }

        if (!colInfo.GetDefaultConstraint().empty())
            column->GetConstraintsR().SetDefaultValueExpression(colInfo.GetDefaultConstraint().c_str());

        if (colInfo.IsNotNull())
            column->GetConstraintsR().SetNotNullConstraint();

        if (colInfo.GetPrimaryKeyOrdinal() > 0)
            {
            pkColumns.push_back(column);
            pkOrdinals.push_back((size_t) (colInfo.GetPrimaryKeyOrdinal() - 1));
            }

        if (column->GetName().EqualsIAscii(primaryKeyColName))
            idColumn = column;
        }

    if (!pkColumns.empty())
        {
        if (pkColumns.size() > 1)
            {
            LOG.errorv("Multi-column PK not supported for MapStrategy 'ExistingTable'. Table: %s", table->GetName().c_str());
            return nullptr;
            }

        if (SUCCESS != table->CreatePrimaryKeyConstraint(pkColumns, &pkOrdinals))
            return nullptr;
        }

    if (idColumn != nullptr)
        idColumn->SetKind(DbColumn::Kind::ECInstanceId);
    else
        {
        LOG.errorv(ECDBSYS_PROP_ECInstanceId " column '%s' does not exist in table '%s' which was specified in ClassMap custom attribute together with ExistingTable MapStrategy.",
                   primaryKeyColName.c_str(), table->GetName().c_str());
        return nullptr;
        }

    DbColumn* column = table->CreateColumn(Utf8String(COL_ECClassId), DbColumn::Type::Integer, 1, DbColumn::Kind::ECClassId, PersistenceType::Virtual);
    if (column == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    table->GetEditHandleR().EndEdit(); //we do not want this table to be editable;
    return table;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE