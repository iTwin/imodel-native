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

//****************************************************************************************
//DbSchema
//****************************************************************************************

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
        if (FindTable(name))
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
            } while (FindTable(finalName.c_str()));
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
    m_tables[tableP->GetName().c_str()] = std::move(table);
    return tableP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbTable* DbSchema::CreateTableAndColumnsForExistingTableMapStrategy(Utf8CP existingTableName)
    {
    DbTable* table = CreateTable(existingTableName, DbTable::Type::Existing, PersistenceType::Persisted, nullptr);
    if (table == nullptr)
        return nullptr;

    Utf8String sql;
    sql.Sprintf("PRAGMA table_info('%s')", existingTableName);
    Statement stmt;
    if (stmt.Prepare(m_ecdb, sql.c_str()) != BE_SQLITE_OK)
        {
        BeAssert(false && "Failed to prepare statement");
        return nullptr;
        }

    if (!table->GetEditHandle().CanEdit())
        table->GetEditHandleR().BeginEdit();

    std::vector<DbColumn*> pkColumns;
    std::vector<size_t> pkOrdinals;
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
        const int pkOrdinal = stmt.GetValueInt(5) - 1; //PK column ordinals returned by this pragma are 1-based as 0 indicates "not a PK col"

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

        DbColumn* column = table->CreateColumn(colName, colType, DbColumn::Kind::DataColumn, PersistenceType::Persisted);
        if (column == nullptr)
            {
            BeAssert(false && "Failed to create column");
            return nullptr;
            }

        if (!Utf8String::IsNullOrEmpty(colDefaultValue))
            column->GetConstraintsR().SetDefaultValueExpression(colDefaultValue);

        if (colIsNotNull)
            column->GetConstraintsR().SetNotNullConstraint();
        
        if (pkOrdinal >= 0)
            {
            pkColumns.push_back(column);
            pkOrdinals.push_back((size_t) pkOrdinal);
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
    auto itor = m_tables.find(name);
    if (itor != m_tables.end())
        return itor->second.get();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
DbTable* DbSchema::FindTableP(Utf8CP name) const
    {
    auto itor = m_tables.find(name);
    if (itor != m_tables.end())
        return itor->second.get();

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
//---------------------------------------------------------------------------------------
DbIndex* DbSchema::CreateIndex(DbIndexId id, DbTable& table, Utf8CP indexName, bool isUnique, std::vector<DbColumn const*> const& columns, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId classId, bool applyToSubclassesIfPartial)
    {
    BeAssert(!columns.empty());

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
void DbSchema::Reset()
    {
    m_tables.clear(); 
    m_nullTable = nullptr;
    m_indexes.clear();
    m_usedIndexNames.clear();
    m_dbMappings.Reset(); 
    m_loadState = LoadState::NotLoaded;
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

//****************************************************************************************
//DbMappings
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
PropertyDbMapping::Path const* DbMappings::FindPropertyPath(ECN::ECPropertyId rootPropertyId, Utf8CP accessString) const
    {
    auto it = m_propertyPathsByRootPropertyId.find(rootPropertyId);
    if (it == m_propertyPathsByRootPropertyId.end())
        return nullptr;

    auto it2 = it->second.find(accessString);
    if (it2 == it->second.end())
        return nullptr;

    return it2->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
PropertyDbMapping::Path const* DbMappings::FindPropertyPath(PropertyPathId propertyPathId) const
    {
    auto itor = m_propertyPaths.find(propertyPathId);
    if (itor == m_propertyPaths.end())
        return nullptr;

    return itor->second.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ClassDbMapping const* DbMappings::FindClassMapping(ClassMapId id) const
    {
    auto itor = m_classMappings.find(id);
    if (itor == m_classMappings.end())
        return nullptr;

    return itor->second.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
std::vector<ClassDbMapping const*> const* DbMappings::FindClassMappings(ECN::ECClassId id) const
    {
    auto itor = m_classMappingsByClassId.find(id);
    if (itor != m_classMappingsByClassId.end())
        return &itor->second;

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
PropertyDbMapping::Path* DbMappings::AddPropertyPath(PropertyPathId id, ECN::ECPropertyId rootPropertyId, Utf8CP accessString)
    {
    if (FindPropertyPath(rootPropertyId, accessString) != nullptr)
        {
        BeAssert(false && "PropertyPath already exist");
        return nullptr;
        }


    if (m_propertyPaths.find(id) != m_propertyPaths.end())
        {
        BeAssert(false && "PropertyPath with same id already exists");
        return nullptr;
        }

    std::unique_ptr<PropertyDbMapping::Path> pp(new PropertyDbMapping::Path(id, rootPropertyId, accessString));
    PropertyDbMapping::Path* ppP = pp.get();

    m_propertyPathsByRootPropertyId[pp->GetRootPropertyId()][pp->GetAccessString().c_str()] = ppP;
    m_propertyPaths[id] = std::move(pp);
    return ppP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
PropertyDbMapping::Path* DbMappings::CreatePropertyPath(ECN::ECPropertyId rootPropertyId, Utf8CP accessString)
    {
    BeBriefcaseBasedId id;
    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetPropertyPathIdSequence().GetNextValue(id))
        {
        BeAssert(false);
        return nullptr;
        }

    return AddPropertyPath(PropertyPathId(id.GetValue()), rootPropertyId, accessString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ClassDbMapping* DbMappings::AddClassMapping(ClassMapId id, ECN::ECClassId classId, ECDbMapStrategy const& mapStrategy, ClassMapId baseClassMapId)
    {
    if (m_classMappings.find(id) != m_classMappings.end())
        {
        BeAssert(false && "ClassDbMapping with same id already exists");
        return nullptr;
        }

    std::unique_ptr<ClassDbMapping> cm(new ClassDbMapping(*this, id, classId, mapStrategy, baseClassMapId));
    ClassDbMapping* cmP = cm.get();

    std::vector<ClassDbMapping const*>& vect = m_classMappingsByClassId[classId];
    if (!cm->GetBaseClassMappingId().IsValid())
        vect.insert(vect.begin(), cmP);
    else
        vect.push_back(cmP);

    m_classMappings[id] = std::move(cm);
    return cmP;

    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ClassDbMapping* DbMappings::CreateClassMapping(ECN::ECClassId classId, ECDbMapStrategy const& mapStrategy, ClassMapId baseClassMapId)
    {
    BeAssert(!baseClassMapId.IsValid()|| FindClassMapping(baseClassMapId) != nullptr);

    BeBriefcaseBasedId id;
    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetClassMapIdSequence().GetNextValue(id))
        {
        BeAssert(false);
        return nullptr;
        }

    return AddClassMapping(ClassMapId(id.GetValue()), classId, mapStrategy, baseClassMapId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
void DbMappings::Reset()
    {
    m_propertyPaths.clear();
    m_propertyPathsByRootPropertyId.clear();
    m_classMappings.clear();
    m_classMappingsByClassId.clear();
    }


//****************************************************************************************
//ClassMapping
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
void ClassDbMapping::GetPropertyMappings(std::vector<PropertyDbMapping const*>& propertyMaps, bool onlyLocal) const
    {
    if (!onlyLocal && m_baseClassMappingId.IsValid())
        {
        ClassDbMapping const* baseClassMapping = m_dbMappings.FindClassMapping(m_baseClassMappingId);
        BeAssert(baseClassMapping != nullptr);
        baseClassMapping->GetPropertyMappings(propertyMaps, onlyLocal);
        }

    propertyMaps.erase(
        std::remove_if(
            propertyMaps.begin(),
            propertyMaps.end(), [] (PropertyDbMapping const* minfo)
        {
        const DbColumn::Kind kind = minfo->ExpectingSingleColumn()->GetKind();
        return kind != DbColumn::Kind::DataColumn && kind != DbColumn::Kind::SharedDataColumn;
        }),
        propertyMaps.end());

    for (std::unique_ptr<PropertyDbMapping> const& localPropertyMap : m_localPropertyMaps)
        propertyMaps.push_back(localPropertyMap.get());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
PropertyDbMapping const* ClassDbMapping::FindPropertyMapping(ECN::ECPropertyId rootPropertyId, Utf8CP accessString) const
    {
    std::vector<PropertyDbMapping const*> propMappings;
    GetPropertyMappings(propMappings, false);
    for (PropertyDbMapping const* pm : propMappings)
        {
        if (pm->GetPropertyPath().GetRootPropertyId() == rootPropertyId && pm->GetPropertyPath().GetAccessString() == accessString)
            return pm;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
PropertyDbMapping const* ClassDbMapping::FindPropertyMapping(Utf8CP accessString) const
    {
    std::vector<PropertyDbMapping const*> propMappings;
    GetPropertyMappings(propMappings, false);

    for (PropertyDbMapping const* pm : propMappings)
        {
        if (pm->GetPropertyPath().GetAccessString() == accessString)
            return pm;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
PropertyDbMapping* ClassDbMapping::CreatePropertyMapping(PropertyDbMapping::Path const& propertyPath)
    {
    std::unique_ptr<PropertyDbMapping> pm(new PropertyDbMapping(*this, propertyPath));
    PropertyDbMapping* pmP = pm.get();
    m_localPropertyMaps.push_back(std::move(pm));
    return pmP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
PropertyDbMapping* ClassDbMapping::CreatePropertyMapping(ECN::ECPropertyId rootPropertyId, Utf8CP accessString, std::vector<DbColumn const*> const& columns)
    {
    PropertyDbMapping::Path const* propertyPath = m_dbMappings.FindPropertyPath(rootPropertyId, accessString);
    if (propertyPath == nullptr)
        propertyPath = m_dbMappings.CreatePropertyPath(rootPropertyId, accessString);

    PropertyDbMapping* prop = CreatePropertyMapping(*propertyPath);
    BeAssert(prop != nullptr);
    for (DbColumn const* column : columns)
        prop->GetColumnsR().push_back(const_cast<DbColumn*>(column));

    return prop;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ClassDbMapping const* ClassDbMapping::GetBaseClassMapping() const
    {
    if (!m_baseClassMappingId.IsValid())
        return nullptr;

    return m_dbMappings.FindClassMapping(m_baseClassMappingId);
    }


END_BENTLEY_SQLITE_EC_NAMESPACE