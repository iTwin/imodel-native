/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbSchema.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
#define EC_INDEX_TableName "ec_Index"
#define EC_INDEXCOLUMN_TableName "ec_IndexColumn"

//****************************************************************************************
//ExistingColumn
//****************************************************************************************
struct ExistingColumn
    {
    private:
        DbColumn::Type m_type;
        Utf8String m_name;
        int m_pkordinal;
        bool m_isnotnull;
        Utf8String m_defaultConstraint;
    public:
        ExistingColumn(Utf8CP name, DbColumn::Type type, int pkordinal, bool isnotnull, Utf8CP defaultConstraint)
            :m_type(type), m_name(name), m_pkordinal(pkordinal), m_defaultConstraint(defaultConstraint), m_isnotnull(isnotnull)
            {}
        ExistingColumn(ExistingColumn const&& rhs)
            :m_type(std::move(rhs.m_type)), m_name(std::move(rhs.m_name)), m_pkordinal(std::move(rhs.m_pkordinal)), m_defaultConstraint(std::move(rhs.m_defaultConstraint)), m_isnotnull(std::move(rhs.m_isnotnull))
            {}
        ExistingColumn(ExistingColumn const& rhs)
            :m_type(rhs.m_type), m_name(rhs.m_name), m_pkordinal(rhs.m_pkordinal), m_defaultConstraint(rhs.m_defaultConstraint), m_isnotnull(rhs.m_isnotnull)
            {}
        ExistingColumn& operator = (ExistingColumn const&& rhs)
            {
            if (this != &rhs)
                {
                m_type = std::move(rhs.m_type);
                m_name = std::move(rhs.m_name);
                m_pkordinal = std::move(rhs.m_pkordinal);
                m_defaultConstraint = std::move(rhs.m_defaultConstraint);
                m_isnotnull = std::move(rhs.m_isnotnull);
                }

            return *this;
            }
        ExistingColumn& operator = (ExistingColumn const& rhs)
            {
            if (this != &rhs)
                {
                m_type = rhs.m_type;
                m_name = rhs.m_name;
                m_pkordinal = rhs.m_pkordinal;
                m_defaultConstraint = rhs.m_defaultConstraint;
                m_isnotnull = rhs.m_isnotnull;
                }

            return *this;
            }
        DbColumn::Type GetType() const { return m_type; }
        Utf8StringCR GetName() const { return m_name; }
        int GetPrimaryKeyOrdinal() const { return m_pkordinal; }
        bool IsNotNull() const { return m_isnotnull; }
        Utf8StringCR GetDefault() const { return m_defaultConstraint; }
        static BentleyStatus GetColumns(std::vector<ExistingColumn>& columns, DbCR db, Utf8CP existingTableName)
            {
            columns.clear();
            Utf8String sql;
            sql.Sprintf("PRAGMA table_info('%s')", existingTableName);
            Statement stmt;
            if (stmt.Prepare(db, sql.c_str()) != BE_SQLITE_OK)
                return BentleyStatus::ERROR;

            while (stmt.Step() == BE_SQLITE_ROW)
                {
                BeAssert(BeStringUtilities::StricmpAscii(stmt.GetColumnName(1), "name") == 0);
                Utf8CP colName = stmt.GetValueText(1);

                BeAssert(BeStringUtilities::StricmpAscii(stmt.GetColumnName(2), "type") == 0);
                Utf8String colTypeName(stmt.GetValueText(2));
                colTypeName.ToLower();
                colTypeName.Trim();

                BeAssert(BeStringUtilities::StricmpAscii(stmt.GetColumnName(3), "notnull") == 0);
                const bool colIsNotNull = stmt.GetValueInt(3) == 1;

                BeAssert(BeStringUtilities::StricmpAscii(stmt.GetColumnName(4), "dflt_value") == 0);
                Utf8CP colDefaultValue = stmt.GetValueText(4);

                BeAssert(BeStringUtilities::StricmpAscii(stmt.GetColumnName(5), "pk") == 0);
                const int pkOrdinal = stmt.GetValueInt(5) ; //PK column ordinals returned by this pragma are 1-based as 0 indicates "not a PK col"

                DbColumn::Type colType = DbColumn::Type::Any;
                if (colTypeName.rfind("long") != Utf8String::npos ||
                    colTypeName.rfind("int") != Utf8String::npos)
                    colType = DbColumn::Type::Integer;
                else if (colTypeName.rfind("char") != Utf8String::npos ||
                         colTypeName.rfind("clob") != Utf8String::npos ||
                         colTypeName.rfind("text") != Utf8String::npos)
                    colType = DbColumn::Type::Text;
                else if (colTypeName.rfind("blob") != Utf8String::npos ||
                         colTypeName.rfind("binary") != Utf8String::npos)
                    colType = DbColumn::Type::Blob;
                else if (colTypeName.rfind("real") != Utf8String::npos ||
                         colTypeName.rfind("floa") != Utf8String::npos ||
                         colTypeName.rfind("doub") != Utf8String::npos)
                    colType = DbColumn::Type::Real;
                else if (colTypeName.rfind("date") != Utf8String::npos ||
                         colTypeName.rfind("timestamp") != Utf8String::npos)
                    colType = DbColumn::Type::TimeStamp;
                else if (colTypeName.rfind("bool") != Utf8String::npos)
                    colType = DbColumn::Type::Boolean;

                columns.push_back(ExistingColumn(colName, colType, pkOrdinal, colIsNotNull, colDefaultValue));
                }

            return SUCCESS;
            }
    };

//****************************************************************************************
//DbSchema
//****************************************************************************************
bool DbSchema::IsTableNameInUse(Utf8CP tableName) const
    {
    SyncTableCache();
    return m_tableMapByName.find(tableName) != m_tableMapByName.end();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbTable* DbSchema::CreateTable(Utf8CP name, DbTable::Type tableType, PersistenceType persType, DbTable const* primaryTable)
    {
    if (tableType == DbTable::Type::Existing)
        {
        if (Utf8String::IsNullOrEmpty(name))
            {
            BeAssert(false && "Existing table name cannot be null or empty");
            return nullptr;
            }

        if (!m_ecdb.TableExists(name))
            {
            LOG.errorv("Table '%s' specified in ClassMap custom attribute must exist if MapStrategy is ExistingTable.", name);
            return nullptr;
            }
        }

    Utf8String finalName;
    if (!Utf8String::IsNullOrEmpty(name))
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
            } while (IsTableNameInUse(finalName.c_str()));
        }

    BeBriefcaseBasedId tableId;
    m_ecdb.GetECDbImplR().GetTableIdSequence().GetNextValue(tableId);
    return CreateTable(DbTableId(tableId.GetValue()), finalName.c_str(), tableType, persType, primaryTable);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbTable* DbSchema::CreateTable(DbTableId tableId, Utf8CP name, DbTable::Type tableType, PersistenceType persType, DbTable const* primaryTable)
    {
    if (Utf8String::IsNullOrEmpty(name) || !tableId.IsValid())
        {
        BeAssert(false && "Table name cannot be empty, table id must be valid");
        return nullptr;
        }

    std::unique_ptr<DbTable> table(std::unique_ptr<DbTable>(new DbTable(tableId, name, *this, persType, tableType, primaryTable)));
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

    std::vector<DbTable*> tables;
    for (auto& tableKey : GetExistingTableMap())
        {
        Utf8CP tableName = tableKey.first.c_str();
        DbTable* table = FindTableP(tableName);
        std::set<Utf8CP, CompareIUtf8Ascii> oldColumnList;
        std::map<Utf8CP, ExistingColumn*, CompareIUtf8Ascii> newColumnList;
        std::vector<ExistingColumn> dbColumnList;
        if (ExistingColumn::GetColumns(dbColumnList, m_ecdb, tableName) == ERROR)
            {
            BeAssert(false && "Failed to get column informations");
            return ERROR;
            }

        for (auto& dbColumn : dbColumnList)
            newColumnList[dbColumn.GetName().c_str()] = &dbColumn;

        for (auto dbColumn : table->GetColumns())
            {
            if (dbColumn->GetPersistenceType() == PersistenceType::Persisted)
                oldColumnList.insert(dbColumn->GetName().c_str());
            }

        //Compute how table have changed
        std::set<Utf8CP, CompareIUtf8Ascii> added;
        std::set<Utf8CP, CompareIUtf8Ascii> deleted;
        for (auto oldColumn : oldColumnList)
            {
            if (newColumnList.find(oldColumn) != newColumnList.end())
                continue;

            deleted.insert(oldColumn);
            }

        for (auto const& newColumnKey : newColumnList)
            {
            if (oldColumnList.find(newColumnKey.first) != oldColumnList.end())
                continue;

            added.insert(newColumnKey.first);
            }

        if (!deleted.empty())
            {
            BeAssert("Existing table changed without map knowing about it");
            return ERROR;
            }

        if (!table->GetEditHandle().CanEdit())
            table->GetEditHandleR().BeginEdit();

        for (Utf8CP addColumn : added)
            {
            auto itor = newColumnList.find(addColumn);
            if (table->CreateColumn(addColumn, itor->second->GetType(), DbColumn::Kind::DataColumn, PersistenceType::Persisted) == nullptr)
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
DbTable* DbSchema::CreateTableAndColumnsForExistingTableMapStrategy(Utf8CP existingTableName)
    {
    DbTable* table = CreateTable(existingTableName, DbTable::Type::Existing, PersistenceType::Persisted, nullptr);
    if (table == nullptr)
        return nullptr;

    std::vector<ExistingColumn> existingColumns;
    if (ExistingColumn::GetColumns(existingColumns, m_ecdb, existingTableName) == ERROR)
        {
        BeAssert(false && "Failed to get column informations");
        return nullptr;
        }

    if (!table->GetEditHandle().CanEdit())
        table->GetEditHandleR().BeginEdit();

    std::vector<DbColumn*> pkColumns;
    std::vector<size_t> pkOrdinals;
    for (ExistingColumn const& col : existingColumns)
        {
        DbColumn* column = table->CreateColumn(col.GetName().c_str(), col.GetType(), DbColumn::Kind::DataColumn, PersistenceType::Persisted);
        if (column == nullptr)
            {
            BeAssert(false && "Failed to create column");
            return nullptr;
            }

        if (!col.GetDefault().empty())
            column->GetConstraintsR().SetDefaultValueExpression(col.GetDefault().c_str());

        if (col.IsNotNull())
            column->GetConstraintsR().SetNotNullConstraint();

        if (col.GetPrimaryKeyOrdinal() > 0)
            {
            pkColumns.push_back(column);
            pkOrdinals.push_back(static_cast<size_t>(col.GetPrimaryKeyOrdinal() - 1));
            }
        }

    if (!pkColumns.empty())
        {
        if (SUCCESS != table->CreatePrimaryKeyConstraint(pkColumns, &pkOrdinals))
            return nullptr;
        }

    table->GetEditHandleR().EndEdit(); //we do not want this table to be editable;
    return table;
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
DbIndex* DbSchema::CreateIndex(DbTable& table, Utf8CP indexName, bool isUnique, std::vector<DbColumn const*> const& columns, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId classId, bool applyToSubclassesIfPartial)
    {    
    if (columns.empty())
        {
        BeAssert(false && "Index must have at least one column defined.");
        return nullptr;
        }

    Utf8String generatedIndexName;
    if (Utf8String::IsNullOrEmpty(indexName))
        {
        do
            {
            m_nameGenerator.Generate(generatedIndexName);
            } while (m_usedIndexNames.find(generatedIndexName.c_str()) != m_usedIndexNames.end() || IsTableNameInUse(generatedIndexName.c_str()));

            indexName = generatedIndexName.c_str();
        }

    BeBriefcaseBasedId id;
    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetIndexIdSequence().GetNextValue(id))
        {
        BeAssert(false);
        return nullptr;
        }

    return CreateIndex(DbIndexId(id.GetValue()), table, indexName, isUnique, columns, addIsNotNullWhereExp, isAutoGenerated, classId, applyToSubclassesIfPartial);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbIndex* DbSchema::CreateIndex(DbTable& table, Utf8CP indexName, bool isUnique, std::vector<Utf8CP> const& columnNames, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId classId, bool applyToSubclassesIfPartial)
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

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT I.Id, T.Name, I.Name, I.IsUnique, I.AddNotNullWhereExp, I.IsAutoGenerated, I.ClassId, I.AppliesToSubclassesIfPartial FROM " EC_INDEX_TableName " I INNER JOIN ec_Table T ON T.Id = I.TableId");
    if (stmt == nullptr)
        return ERROR;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        DbIndexId id = stmt->GetValueId<DbIndexId>(0);
        Utf8CP tableName = stmt->GetValueText(1);
        Utf8CP name = stmt->GetValueText(2);
        bool isUnique = stmt->GetValueInt(3) == 1;
        bool addNotNullWhereExp = stmt->GetValueInt(4) == 1;
        bool isAutoGenerated = stmt->GetValueInt(5) == 1;
        ECClassId classId = !stmt->IsColumnNull(6) ? stmt->GetValueId<ECClassId>(6) : ECClassId();
        bool appliesToSubclassesIfPartial = stmt->GetValueInt(7) == 1;

        DbTable* table = FindTableP(tableName);
        if (table == nullptr)
            {
            BeAssert(false && "Failed to find table");
            return ERROR;
            }

        CachedStatementPtr indexColStmt = m_ecdb.GetCachedStatement("SELECT C.Name FROM " EC_INDEXCOLUMN_TableName " I INNER JOIN ec_Column C ON C.Id = I.ColumnId WHERE I.IndexId = ? ORDER BY I.Ordinal");
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

        if (nullptr ==  const_cast<DbSchema*>(this)->CreateIndex(id, *table, name, isUnique, columns, addNotNullWhereExp, isAutoGenerated, classId, appliesToSubclassesIfPartial))
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
DbIndex* DbSchema::CreateIndex(DbIndexId id, DbTable& table, Utf8CP indexName, bool isUnique, std::vector<DbColumn const*> const& columns, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId classId, bool applyToSubclassesIfPartial)
    {
    BeAssert(!columns.empty());
    if (!m_indexesLoaded && LoadIndexes() == ERROR)
        return nullptr;

    auto it = m_usedIndexNames.find(indexName);
    if (it != m_usedIndexNames.end())
        {
        for (std::unique_ptr<DbIndex>& index : m_indexes)
            {
            if (index->GetName() == indexName)
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
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Index with name '%s' already defined in the ECDb file.", indexName);
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
    std::map<Utf8String, DbTableId, CompareIUtf8Ascii> persistedTableMap = GetPersistedTableMap();
    for (DbTable const* table : GetCachedTables())
        {
        // This would be null in case a table is not loaded yet and if its not loaded then we do not need to update it
        if (table != nullptr)
            {
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
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
std::map<Utf8String, DbTableId, CompareIUtf8Ascii> DbSchema::GetPersistedTableMap() const
    {
    std::map<Utf8String, DbTableId, CompareIUtf8Ascii> persistedTableMap;
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
std::map<Utf8String, DbTableId, CompareIUtf8Ascii> DbSchema::GetExistingTableMap() const
    {
    std::map<Utf8String, DbTableId, CompareIUtf8Ascii> persistedTableMap;
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(SqlPrintfString("SELECT Name, Id FROM ec_Table WHERE Type = %d", Enum::ToInt(DbTable::Type::Existing)).GetUtf8CP());
    
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
std::map<Utf8String, DbColumnId, CompareIUtf8Ascii> DbSchema::GetPersistedColumnMap(DbTableId tableId) const
    {
    std::map<Utf8String, DbColumnId, CompareIUtf8Ascii> persistedColumnMap;
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT Name, Id FROM ec_Column WHERE TableId = ?");
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

    for (auto const& tableKey : GetPersistedTableMap())
        {
        if (m_tableMapByName.find(tableKey.first) == m_tableMapByName.end())
            {
            m_tableMapByName.insert(std::make_pair(tableKey.first, std::unique_ptr<DbTable>()));
            m_tableMapById.insert(std::make_pair(tableKey.second, tableKey.first));
            }
        }

    m_syncTableCacheNames = true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
void DbSchema::Reset()
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
        if (tableKey.second != nullptr)
            {
            if (tableKey.second->GetType() == DbTable::Type::Joined)
                cachedTables.push_back(tableKey.second.get());
            else
                cachedTables.insert(cachedTables.begin(), tableKey.second.get());
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

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("INSERT INTO ec_Table(Id, Name, Type, IsVirtual, PrimaryTableId) VALUES (?, ?, ?, ?, ?)");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindId(1, table.GetId());
    stmt->BindText(2, table.GetName().c_str(), Statement::MakeCopy::No);
    stmt->BindInt(3, Enum::ToInt(table.GetType()));
    stmt->BindInt(4, table.GetPersistenceType() == PersistenceType::Virtual ? 1 : 0);
    if (auto primaryTable = table.GetParentOfJoinedTable())
        stmt->BindId(5, primaryTable->GetId());
    else
        stmt->BindNull(5);

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

    std::map<Utf8String, DbColumnId, CompareIUtf8Ascii> persistedColumnMap = GetPersistedColumnMap(table.GetId());
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("UPDATE ec_Table SET Name =?, Type =?, IsVirtual=?, PrimaryTableId=? WHERE Id = ?");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindId(5, table.GetId());
    stmt->BindText(1, table.GetName().c_str(), Statement::MakeCopy::No);
    stmt->BindInt(2, Enum::ToInt(table.GetType()));
    stmt->BindInt(3, table.GetPersistenceType() == PersistenceType::Virtual ? 1 : 0);
    if (auto primaryTable = table.GetParentOfJoinedTable())
        stmt->BindId(4, primaryTable->GetId());
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
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("DELETE FROM ec_Index");
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
        if (index.GetTable().GetPersistenceType() == PersistenceType::Persisted)
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
                    ECClassCP provenanceClass = m_ecdb.Schemas().GetECClass(index.GetClassId());
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
                    ECClassCP provenanceClass = m_ecdb.Schemas().GetECClass(existingIndex->GetClassId());
                    if (provenanceClass == nullptr)
                        {
                        BeAssert(false);
                        return ERROR;
                        }
                    existingIndexProvenanceStr.Sprintf(" [Created for ECClass %s]", provenanceClass->GetFullName());
                    }

                if (!index.IsAutoGenerated())
                    m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Warning, errorMessage,
                                                                    index.GetName().c_str(), provenanceStr.c_str(), index.GetTable().GetName().c_str(),
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
                m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to create index %s on table %s. Error: %s", index.GetName().c_str(), index.GetTable().GetName().c_str(),
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
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("UPDATE ec_Column SET Name =?, Type =?, IsVirtual=?, Ordinal=?, NotNullConstraint=?, UniqueConstraint=?, CheckConstraint=?, DefaultConstraint=?, CollationConstraint=?, OrdinalInPrimaryKey=?, ColumnKind=? WHERE Id = ?");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindId(12, column.GetId());
    stmt->BindText(1, column.GetName().c_str(), Statement::MakeCopy::No);
    stmt->BindInt(2, Enum::ToInt(column.GetType()));
    stmt->BindInt(3, column.GetPersistenceType() == PersistenceType::Virtual ? 1 : 0);
    stmt->BindInt64(4, columnOrdinal);
    stmt->BindInt(5, column.GetConstraints().HasNotNullConstraint() ? 1 : 0);
    stmt->BindInt(6, column.GetConstraints().HasUniqueConstraint() ? 1 : 0);

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
    stmt->BindInt(5, column.GetPersistenceType() == PersistenceType::Virtual ? 1 : 0);
    stmt->BindInt64(6, columnOrdinal);
    stmt->BindInt(7, column.GetConstraints().HasNotNullConstraint() ? 1 : 0);
    stmt->BindInt(8, column.GetConstraints().HasUniqueConstraint() ? 1 : 0);

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
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("INSERT INTO " EC_INDEX_TableName "(Id,TableId,Name,IsUnique,AddNotNullWhereExp,IsAutoGenerated,ClassId,AppliesToSubclassesIfPartial) VALUES(?,?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindId(1, index.GetId());
    stmt->BindId(2, index.GetTable().GetId());
    stmt->BindText(3, index.GetName().c_str(), Statement::MakeCopy::No);
    stmt->BindInt(4, index.GetIsUnique() ? 1 : 0);
    stmt->BindInt(5, index.IsAddColumnsAreNotNullWhereExp() ? 1 : 0);

    stmt->BindInt(6, index.IsAutoGenerated() ? 1 : 0);
    if (index.HasClassId())
        stmt->BindId(7, index.GetClassId());

    stmt->BindInt(8, index.AppliesToSubclassesIfPartial() ? 1 : 0);

    DbResult stat = stmt->Step();
    if (stat != BE_SQLITE_DONE)
        {
        LOG.errorv("Failed to insert index metadata into " EC_INDEX_TableName " for index %s (Id: %s): %s",
                   index.GetName().c_str(), index.GetId().ToString().c_str(), m_ecdb.GetLastError().c_str());
        return ERROR;
        }

    CachedStatementPtr indexColStmt = m_ecdb.GetCachedStatement("INSERT INTO " EC_INDEXCOLUMN_TableName "(IndexId,ColumnId,Ordinal) VALUES(?,?,?)");
    if (indexColStmt == nullptr)
        return ERROR;

    int i = 0;
    for (DbColumn const* col : index.GetColumns())
        {
        indexColStmt->BindId(1, index.GetId());
        indexColStmt->BindId(2, col->GetId());
        indexColStmt->BindInt(3, i);

        stat = indexColStmt->Step();
        if (stat != BE_SQLITE_DONE)
            {
            LOG.errorv("Failed to insert index column metadata into " EC_INDEXCOLUMN_TableName " for index %s (Id: %s) and column %s (Id: %s): %s",
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
    while (BE_SQLITE_ROW == stmt->Step())
        {
        DbColumnId id = stmt->GetValueId<DbColumnId>(0);
        Utf8CP name = stmt->GetValueText(1);
        const DbColumn::Type type = Enum::FromInt<DbColumn::Type>(stmt->GetValueInt(2));
        const PersistenceType persistenceType = IsTrue(stmt->GetValueInt(3)) ? PersistenceType::Virtual : PersistenceType::Persisted;

        const bool hasNotNullConstraint = IsTrue(stmt->GetValueInt(notNullColIx));
        const bool hasUniqueConstraint = IsTrue(stmt->GetValueInt(uniqueColIx));
        Utf8CP constraintCheck = !stmt->IsColumnNull(checkColIx) ? stmt->GetValueText(checkColIx) : nullptr;
        Utf8CP constraintDefault = !stmt->IsColumnNull(defaultValueColIx) ? stmt->GetValueText(defaultValueColIx) : nullptr;
        const DbColumn::Constraints::Collation collationConstraint = Enum::FromInt<DbColumn::Constraints::Collation>(stmt->GetValueInt(collationColIx));

        int primaryKeyOrdinal = stmt->IsColumnNull(pkOrdinalColIx) ? -1 : stmt->GetValueInt(pkOrdinalColIx);
        const DbColumn::Kind columnKind = Enum::FromInt<DbColumn::Kind>(stmt->GetValueInt(kindColIx));

        DbColumn* column = table.CreateColumn(id, name, type, columnKind, persistenceType);
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
        }

    if (!pkColumns.empty())
        {
        if (SUCCESS != table.CreatePrimaryKeyConstraint(pkColumns, &pkOrdinals))
            return SUCCESS;
        }

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbSchema::LoadTable(Utf8StringCR name, DbTable*& tableP) const
    {
    tableP = nullptr;
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT A.Id, A.Type, A.IsVirtual, B.Name BaseTableName FROM ec_Table A LEFT JOIN ec_Table B ON A.PrimaryTableId = B.Id WHERE A.Name = ?");
    if (stmt == nullptr)
        return ERROR;

    stmt->BindText(1, name.c_str(), Statement::MakeCopy::No);
    if (stmt->Step() != BE_SQLITE_ROW)
        return ERROR;

    DbTableId id = stmt->GetValueId<DbTableId>(0);
    DbTable::Type tableType = Enum::FromInt<DbTable::Type>(stmt->GetValueInt(1));
    PersistenceType persistenceType = stmt->GetValueInt(2) == 1 ? PersistenceType::Virtual : PersistenceType::Persisted;
    Utf8CP primaryTableName = stmt->GetValueText(3);

    DbTable const* primaryTable = nullptr;
    if (!Utf8String::IsNullOrEmpty(primaryTableName))
        {
        primaryTable = FindTable(primaryTableName);
        BeAssert(primaryTable != nullptr && "Failed to find primary table");
        BeAssert(DbTable::Type::Joined == tableType && "Expecting JoinedTable");
        }

    DbTable* table = const_cast<DbSchema*>(this)->CreateTable(id, name.c_str(), tableType, persistenceType, primaryTable);
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
            m_nullTable = const_cast<DbSchema*>(this)->CreateTable(DBSCHEMA_NULLTABLENAME, DbTable::Type::Primary, PersistenceType::Virtual, nullptr);

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
// @bsimethod                                               Krischan.Eberle 02/2016
//---------------------------------------------------------------------------------------
BentleyStatus DbTable::EnsureMinimumNumberOfSharedColumns()
    {
    if (m_minimumSharedColumnCount == ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT)
        return SUCCESS; // no min count specified -> nothing to do

    int existingSharedColCount = 0;
    for (DbColumn const* col : m_orderedColumns)
        {
        if (col->IsShared())
            existingSharedColCount++;
        }

    const int neededSharedColumns = m_minimumSharedColumnCount - existingSharedColCount;
    if (neededSharedColumns <= 0)
        return SUCCESS;

    for (int i = 0; i < neededSharedColumns; i++)
        {
        if (CreateSharedColumn() == nullptr)
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle 02/2016
//---------------------------------------------------------------------------------------
BentleyStatus DbTable::SetMinimumSharedColumnCount(int minimumSharedColumnCount)
    {
    //can only by one ECClass of this table
    if (minimumSharedColumnCount == ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT ||
        (m_minimumSharedColumnCount != ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT && minimumSharedColumnCount != m_minimumSharedColumnCount))
        {
        BeAssert(false && "Cannot modify MinimumSharedColumnCount on an DbTable if it has been set already.");
        return ERROR;
        }

    m_minimumSharedColumnCount = minimumSharedColumnCount;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                          muhammad.zaighum                           01/2015
//---------------------------------------------------------------------------------------
BentleyStatus DbTable::CreateTrigger(Utf8CP triggerName, DbTrigger::Type type, Utf8CP condition, Utf8CP body)
    {
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
DbColumn* DbTable::CreateColumn(DbColumnId id, Utf8CP name, DbColumn::Type type, int position, DbColumn::Kind kind, PersistenceType persistenceType)
    {
    if (!GetEditHandleR().CanEdit())
        {
        IssueReporter const& issues = m_dbSchema.GetECDb().GetECDbImplR().GetIssueReporter();
        if (m_type == Type::Existing)
            issues.Report(ECDbIssueSeverity::Error, "Cannot add columns to the existing table '%s' not owned by ECDb.", m_name.c_str());
        else
            {
            BeAssert(false && "Cannot add columns to read-only table.");
            issues.Report(ECDbIssueSeverity::Error, "Cannot add columns to the table '%s'. Table is not in edit mode.", m_name.c_str());
            }

        return nullptr;
        }

    PersistenceType resolvePersistenceType = persistenceType;
    if (GetPersistenceType() == PersistenceType::Virtual)
        resolvePersistenceType = PersistenceType::Virtual;

    if (!id.IsValid())
        {
        BeBriefcaseBasedId columnId;
        m_dbSchema.GetECDb().GetECDbImplR().GetColumnIdSequence().GetNextValue(columnId);
        id = DbColumnId(columnId.GetValue());
        }

    std::shared_ptr<DbColumn> newColumn = nullptr;
    if (!Utf8String::IsNullOrEmpty(name))
        {
        if (FindColumn(name))
            {
            BeAssert(false && "Column name already exist");
            return nullptr;
            }

        newColumn = std::make_shared<DbColumn>(id, *this, name, type, kind, resolvePersistenceType);
        }
    else
        {
        Utf8String generatedName;
        do
            {
            m_columnNameGenerator.Generate(generatedName);
            } while (FindColumn(generatedName.c_str()));

            newColumn = std::make_shared<DbColumn>(id, *this, generatedName.c_str(), type, kind, resolvePersistenceType);
        }

    if (position < 0)
        m_orderedColumns.push_back(newColumn.get());
    else
        m_orderedColumns.insert(m_orderedColumns.begin() + (size_t) position, newColumn.get());

    m_columns[newColumn->GetName().c_str()] = newColumn;

    for (auto& eh : m_columnEvents)
        eh(ColumnEvent::Created, *newColumn);

    return newColumn.get();
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
        if (constraint->GetType() == DbConstraint::Type::ForeignKey)
            {
            ForeignKeyDbConstraint* fkc = static_cast<ForeignKeyDbConstraint*>(constraint.get());
            fkc->Remove(col.GetName().c_str(), nullptr);
            }
        else if (constraint->GetType() == DbConstraint::Type::PrimaryKey)
            {
            PrimaryKeyDbConstraint const* pkc = static_cast<PrimaryKeyDbConstraint const*>(constraint.get());
            if (pkc->Contains(col))
                {
                BeAssert(false && "Cannot delete a column from a PK constraint");
                return ERROR;
                }
            }
        }

    for (auto& eh : m_columnEvents)
        eh(ColumnEvent::Deleted, col);

    m_columns.erase(col.GetName().c_str());
    auto columnsAreEqual = [&col] (DbColumn const* column) { return column == &col; };
    m_orderedColumns.erase(std::find_if(m_orderedColumns.begin(), m_orderedColumns.end(), columnsAreEqual));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
std::weak_ptr<DbColumn> DbTable::FindColumnWeakPtr(Utf8CP name) const
    {
    auto itor = m_columns.find(name);
    if (itor != m_columns.end())
        {
        return itor->second;
        }

    return std::weak_ptr<DbColumn>();
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
    for (auto column : m_orderedColumns)
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
    for (auto column : m_orderedColumns)
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
bool DbTable::TryGetECClassIdColumn(DbColumn const*& classIdCol) const
    {
    if (!m_isClassIdColumnCached)
        {
        m_classIdColumn = GetFilteredColumnFirst(DbColumn::Kind::ECClassId);
        m_isClassIdColumnCached = true;
        }

    classIdCol = m_classIdColumn;
    return m_classIdColumn != nullptr;
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
// @bsimethod                                               Affan.Khan 02/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbColumn::MakePersisted(DbColumn& column)
    {
    if (column.GetTableR().GetEditHandleR().AssertNotInEditMode())
        return ERROR;

    if (column.GetPersistenceType() == PersistenceType::Persisted)
        return SUCCESS;

    if (column.GetTable().GetPersistenceType() == PersistenceType::Virtual)
        {
        BeAssert(false && "Virtual table cannot have persistence column");
        return ERROR;
        }

    column.m_persistenceType = PersistenceType::Persisted;
    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   05/2016
//---------------------------------------------------------------------------------------
bool DbColumn::IsOnlyColumnOfPrimaryKeyConstraint() const
    {
    return m_pkConstraint != nullptr && m_pkConstraint->GetColumns().size() == 1;
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
//static 
DbColumn::Type DbColumn::PrimitiveTypeToColumnType(ECN::PrimitiveType type)
    {
    switch (type)
        {
            case ECN::PrimitiveType::PRIMITIVETYPE_Binary:
            case ECN::PrimitiveType::PRIMITIVETYPE_IGeometry:;
                return DbColumn::Type::Blob;
            case ECN::PrimitiveType::PRIMITIVETYPE_Boolean:
                return DbColumn::Type::Boolean;
            case ECN::PrimitiveType::PRIMITIVETYPE_DateTime:
                return DbColumn::Type::TimeStamp;
            case ECN::PrimitiveType::PRIMITIVETYPE_Double:
                return DbColumn::Type::Real;

            case ECN::PrimitiveType::PRIMITIVETYPE_Integer:
            case ECN::PrimitiveType::PRIMITIVETYPE_Long:
                return DbColumn::Type::Integer;

            case ECN::PrimitiveType::PRIMITIVETYPE_String:
                return DbColumn::Type::Text;
        }

    BeAssert(false && "Type not supported");
    return DbColumn::Type::Any;
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
Utf8CP DbColumn::KindToString(Kind columnKind)
    {
    switch (columnKind)
        {
            case Kind::ECInstanceId:
                return "ECInstanceId";
            case Kind::ECClassId:
                return "ECClassId";
            case Kind::SourceECInstanceId:
                return "SourceECInstanceId";
            case Kind::SourceECClassId:
                return "SourceECClassId";
            case Kind::TargetECInstanceId:
                return "TargetECInstanceId";
            case Kind::TargetECClassId:
                return "TargetECClassId";
            case Kind::DataColumn:
                return "DataColumn";
            case Kind::SharedDataColumn:
                return "SharedDataColumn";
            default:
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
        case Collation::Default:
            return "";

        case Collation::Binary:
            return "BINARY";

        case Collation::NoCase:
            return "NOCASE";

        case Collation::RTrim:
            return "RTRIM";

        default:
            BeAssert(false && "Unhandled value of Enum Collation");
            return "";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static 
bool DbColumn::Constraints::TryParseCollationString(Collation& collation, Utf8CP str)
    {
    if (Utf8String::IsNullOrEmpty(str))
        {
        collation = Collation::Default;
        return true;
        }

    if (BeStringUtilities::StricmpAscii(str, "Binary") == 0)
        collation = Collation::Binary;
    else if (BeStringUtilities::StricmpAscii(str, "NoCase") == 0)
        collation = Collation::NoCase;
    else if (BeStringUtilities::StricmpAscii(str, "RTrim") == 0)
        collation = Collation::RTrim;
    else
        return false;

    return true;
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
ForeignKeyDbConstraint::ActionType ForeignKeyDbConstraint::ToActionType(Utf8CP str)
    {
    if (BeStringUtilities::StricmpAscii(str, "Cascade") == 0)
        return ActionType::Cascade;

    if (BeStringUtilities::StricmpAscii(str, "SetNull") == 0 || BeStringUtilities::StricmpAscii(str, "SET NULL") == 0)
        return ActionType::SetNull;

    if (BeStringUtilities::StricmpAscii(str, "SetDefault") == 0 || BeStringUtilities::StricmpAscii(str, "SET DEAULT") == 0)
        return ActionType::SetDefault;

    if (BeStringUtilities::StricmpAscii(str, "Restrict") == 0)
        return ActionType::Restrict;

    if (BeStringUtilities::StricmpAscii(str, "NoAction") == 0 || BeStringUtilities::StricmpAscii(str, "NO ACTION") == 0)
        return ActionType::NoAction;

    return ActionType::NotSpecified;
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

END_BENTLEY_SQLITE_EC_NAMESPACE