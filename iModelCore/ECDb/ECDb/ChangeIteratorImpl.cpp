/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//******************************************************************************
// ChangeIterator
//******************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ChangeIterator(ECDbCR ecdb, Changes const& changes) : m_ecdb(ecdb), m_changes(changes), m_tableMaps(std::make_unique<TableMapCollection>())
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::~ChangeIterator() {}

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::const_iterator ChangeIterator::begin() const { return RowEntry(*this, m_changes.begin()); }
//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::const_iterator ChangeIterator::end() const { return RowEntry(*this, m_changes.end()); }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::TableMap const* ChangeIterator::GetTableMap(Utf8StringCR tableName) const
    {
    ChangeIterator::TableMap const* map = m_tableMaps->Get(tableName);
    if (map != nullptr)
        return map;

    std::unique_ptr<ChangeIterator::TableMap> newMap = std::make_unique<ChangeIterator::TableMap>(m_ecdb, tableName);
    ChangeIterator::TableMap const* newMapP = newMap.get();
    m_tableMaps->Add(std::move(newMap));
    return newMapP;
    }


//******************************************************************************
// ChangeIterator::RowEntry
//******************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::RowEntry::RowEntry(ChangeIterator const& iterator, Changes::Change const& change) : m_ecdb(iterator.m_ecdb), m_iterator(iterator), m_change(change), m_sqlChange(nullptr), m_tableMap(nullptr)
    {
    Initialize();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     08/2017
//---------------------------------------------------------------------------------------
ChangeIterator::RowEntry::RowEntry(ChangeIterator::RowEntry const& other) : m_ecdb(other.m_ecdb), m_iterator(other.m_iterator), m_change(other.m_change)
    {
    *this = other;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     08/2017
//---------------------------------------------------------------------------------------
ChangeIterator::RowEntry& ChangeIterator::RowEntry::operator=(ChangeIterator::RowEntry const& other)
    {
    m_change = other.m_change;
    m_tableMap = other.m_tableMap;
    m_primaryInstanceId = other.m_primaryInstanceId;
    m_primaryClass = other.m_primaryClass;
    FreeSqlChange();
    m_sqlChange = new SqlChange(m_change);
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::RowEntry::~RowEntry() { FreeSqlChange(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void ChangeIterator::RowEntry::Initialize()
    {
    Reset();
    if (!m_change.IsValid())
        return;

    m_sqlChange = new SqlChange(m_change);

    m_tableMap = m_iterator.GetTableMap(m_sqlChange->GetTableName());
    BeAssert(m_tableMap != nullptr);

    if (m_tableMap->IsMapped())
        InitPrimaryInstance();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void ChangeIterator::RowEntry::Reset()
    {
    FreeSqlChange();
    m_tableMap = nullptr;
    m_primaryInstanceId.Invalidate();
    m_primaryClass = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void ChangeIterator::RowEntry::FreeSqlChange()
    {
    if (m_sqlChange == nullptr)
        return;

    delete m_sqlChange;
    m_sqlChange = nullptr;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void ChangeIterator::RowEntry::InitPrimaryInstance()
    {
    BeAssert(m_tableMap->IsMapped());

    m_primaryInstanceId = m_sqlChange->GetValueId<ECInstanceId>(m_tableMap->GetIdColumn().GetIndex());
    BeAssert(m_primaryInstanceId.IsValid());

    if (m_sqlChange->GetDbOpcode() == DbOpcode::Update)
        {
        CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement(Utf8PrintfString("SELECT 1 FROM %s WHERE %s=?", m_tableMap->GetTableName().c_str(),
                                                                                             m_tableMap->GetIdColumn().GetName().c_str()).c_str());

        if (stmt == nullptr)
            {
            BeAssert(false);
            return;
            }

        stmt->BindId(1, m_primaryInstanceId);
        if (BE_SQLITE_DONE == stmt->Step())
            {
            // Note: The instance doesn't exist anymore, and has been deleted in future change to the Db.
            // Processing updates requires that the instance is still available in the Db to extract sufficient EC information, 
            // especially since a SqlChangeSet records only the updated columns but not the entire row. 
            BeAssert(false && "SqlChangeSet does not span all modifications made to the Db");
            return;
            }
        }

    ECClassId primaryClassId;
    if (m_tableMap->ContainsECClassIdColumn())
        primaryClassId = GetClassIdFromChangeOrTable(m_tableMap->GetECClassIdColumn().GetName().c_str(), m_primaryInstanceId);
    else
        primaryClassId = m_tableMap->GetClassId();

    m_primaryClass = m_ecdb.Schemas().GetClass(primaryClassId);
    BeAssert(m_primaryClass != nullptr && "Couldn't determine the class corresponding to the change.");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ECN::ECClassId ChangeIterator::RowEntry::GetClassIdFromChangeOrTable(Utf8CP classIdColumnName, ECInstanceId instanceId) const
    {
    int classIdColumnIndex = m_tableMap->GetColumnIndexByName(classIdColumnName);

    ECClassId oldId, newId;
    m_sqlChange->GetValueIds<ECClassId>(oldId, newId, classIdColumnIndex);

    const DbOpcode dbOpcode = m_sqlChange->GetDbOpcode();

    if (dbOpcode == DbOpcode::Insert)
        {
        BeAssert(newId.IsValid());
        return newId;
        }

    if (dbOpcode == DbOpcode::Delete)
        {
        BeAssert(oldId.IsValid());
        return oldId;
        }

    if (dbOpcode == DbOpcode::Update)
        {
        if (newId.IsValid()) return newId;
        if (oldId.IsValid()) return oldId;
        }
    
    // The class id entry hasn't been updated at all - get it from the database itself
    ECClassId classId;
    if (SUCCESS != DbUtilities::QueryRowClassId(classId, m_ecdb, m_tableMap->GetTableName(), classIdColumnName, m_tableMap->GetIdColumn().GetName(), instanceId))
        {
        BeAssert(false && "Failed to execute SQL to query for class id of a change.");
        }

    return classId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ColumnIterator ChangeIterator::RowEntry::MakeColumnIterator(ECN::ECClassCR ecClass) const { return ChangeIterator::ColumnIterator(*this, &ecClass); }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ColumnIterator ChangeIterator::RowEntry::MakePrimaryColumnIterator() const { return ChangeIterator::ColumnIterator(*this, GetPrimaryClass()); }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
Utf8StringCR ChangeIterator::RowEntry::GetTableName() const
    {
    BeAssert(IsValid());
    return m_tableMap->GetTableName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
bool ChangeIterator::RowEntry::IsPrimaryTable() const
    {
    if (!IsValid())
        {
        BeAssert(false);
        return false;
        }

    return m_tableMap->GetDbTable()->GetType() == DbTable::Type::Primary;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
DbOpcode ChangeIterator::RowEntry::GetDbOpcode() const
    {
    BeAssert(m_sqlChange != nullptr);
    return m_sqlChange->GetDbOpcode();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
int ChangeIterator::RowEntry::GetIndirect() const
    {
    BeAssert(m_sqlChange != nullptr);
    return m_sqlChange->GetIndirect();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::RowEntry& ChangeIterator::RowEntry::operator++()
    {
    ++m_change;
    Initialize();
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                            Krischan.Eberle    07/2018
//---------------------------------------------------------------------------------------
Utf8String ChangeIterator::RowEntry::ToString() const
    {
    if (!IsValid())
        return "Invalid change";

    Utf8CP opCodeStr = nullptr;
    switch (GetDbOpcode())
        {
            case DbOpcode::Delete:
                opCodeStr = "Delete";
                break;
            case DbOpcode::Insert:
                opCodeStr = "Insert";
                break;
            case DbOpcode::Update:
                opCodeStr = "Update";
                break;
            default:
                return "Invalid Change: unknown DbOpCode";
        };

    return Utf8PrintfString("Change (Primary class: %s | Primary instance id: %s | OpCode: %s | IsIndirect: %s | Table: %s | Primary table: %s)",
                            GetPrimaryClass() != nullptr ? GetPrimaryClass()->GetFullName() : "-",
                            GetPrimaryInstanceId().IsValid() ? GetPrimaryInstanceId().ToString().c_str() : "-",
                            opCodeStr, GetIndirect() ? "yes" : "no",
                            GetTableName().c_str(), IsPrimaryTable() ? "yes" : "no");
    }

//******************************************************************************
// ChangeIterator::ColumnIterator
//******************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ColumnIterator::ColumnIterator(RowEntry const& rowEntry, ECN::ECClassCP ecClass) : m_rowEntry(rowEntry), m_ecdb(rowEntry.GetDb()), m_sqlChange(rowEntry.GetSqlChange()), m_tableClassMap(nullptr)
    {
    if (ecClass != nullptr)
        {
        m_tableClassMap = rowEntry.GetTableMap()->GetTableClassMap(*ecClass);
        BeAssert(m_tableClassMap != nullptr && m_tableClassMap->IsMapped());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ColumnEntry ChangeIterator::ColumnIterator::GetColumn(Utf8CP propertyAccessString) const
    {
    if (m_tableClassMap == nullptr)
        return ChangeIterator::ColumnEntry(*this);

    return ChangeIterator::ColumnEntry(*this, m_tableClassMap->GetColumnMapByAccessString().find(propertyAccessString));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ColumnEntry ChangeIterator::ColumnIterator::begin() const
    {
    if (m_tableClassMap == nullptr)
        return ChangeIterator::ColumnEntry(*this);

    return ChangeIterator::ColumnEntry(*this, m_tableClassMap->GetColumnMapByAccessString().begin());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ColumnEntry ChangeIterator::ColumnIterator::end() const
    {
    if (m_tableClassMap == nullptr)
        return ChangeIterator::ColumnEntry(*this);

    return ChangeIterator::ColumnEntry(*this, m_tableClassMap->GetColumnMapByAccessString().end());
    }

//******************************************************************************
// ChangeIterator::ColumnEntry
//******************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ColumnEntry::ColumnEntry(ColumnIterator const& columnIterator, bmap<Utf8String, ColumnMap*>::const_iterator columnMapIterator)
    : m_columnIterator(columnIterator), m_ecdb(columnIterator.m_ecdb), m_sqlChange(columnIterator.m_sqlChange), m_columnMapIterator(columnMapIterator), m_isValid(true)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ColumnEntry::ColumnEntry(ColumnIterator const& columnIterator) : m_columnIterator(columnIterator), m_ecdb(columnIterator.m_ecdb), m_isValid(false)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
Utf8StringCR ChangeIterator::ColumnEntry::GetPropertyAccessString() const
    {
    BeAssert(m_isValid);
    return m_columnMapIterator->first;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
DbDupValue ChangeIterator::ColumnEntry::QueryValueFromDb() const
    {
    if (!m_isValid)
        {
        BeAssert(false);
        return DbDupValue(nullptr);
        }

    ColumnMap const* columnMap = m_columnMapIterator->second;
    BeAssert(columnMap != nullptr);

    RowEntry const& rowEntry = m_columnIterator.m_rowEntry;

    ChangeIterator::TableMap const* tableMap = rowEntry.GetTableMap();
    BeAssert(tableMap != nullptr);

    Utf8PrintfString sql("SELECT %s FROM %s WHERE %s=?", columnMap->GetName().c_str(), tableMap->GetTableName().c_str(), tableMap->GetIdColumn().GetName().c_str());
    CachedStatementPtr statement = m_ecdb.GetImpl().GetCachedSqliteStatement(sql.c_str());
    BeAssert(statement.IsValid());

    statement->BindId(1, rowEntry.GetPrimaryInstanceId());

    DbResult result = statement->Step();
    if (BE_SQLITE_ROW == result)
        return statement->GetDbValue(0);

    BeAssert(result == BE_SQLITE_DONE);
    return DbDupValue(nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
DbDupValue ChangeIterator::ColumnEntry::GetValue(Changes::Change::Stage stage) const
    {
    if (!m_isValid)
        {
        BeAssert(false);
        return DbDupValue(nullptr);
        }

    ColumnMap const* columnMap = m_columnMapIterator->second;
    BeAssert(columnMap != nullptr);

    DbValue value = m_sqlChange->GetChange().GetValue(columnMap->GetIndex(), stage);
    return DbDupValue(value.GetSqlValueP());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
bool ChangeIterator::ColumnEntry::IsPrimaryKeyColumn() const
    {
    if (!m_isValid)
        {
        BeAssert(false);
        return false;
        }

    ColumnMap const* columnMap = m_columnMapIterator->second;
    BeAssert(columnMap != nullptr);

    int idx = columnMap->GetIndex();
    return m_sqlChange->IsPrimaryKeyColumn(idx);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ColumnEntry& ChangeIterator::ColumnEntry::operator++()
    {
    if (!m_isValid)
        {
        BeAssert(false);
        return *this;
        }

    m_columnMapIterator++;
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
bool ChangeIterator::ColumnEntry::operator!=(ColumnEntry const& rhs) const
    {
    return (m_isValid != rhs.m_isValid) || (m_columnMapIterator != rhs.m_columnMapIterator);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
bool ChangeIterator::ColumnEntry::operator==(ColumnEntry const& rhs) const
    {
    return (m_isValid == rhs.m_isValid) && (m_columnMapIterator == rhs.m_columnMapIterator);
    }


//******************************************************************************
// ChangeIterator::TableClassMap
//******************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::TableClassMap::~TableClassMap()
    {
    FreeEndTableRelationshipMaps();
    FreeColumnMaps();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void ChangeIterator::TableClassMap::FreeColumnMaps()
    {
    for (auto it = m_columnMapByAccessString.begin(); it != m_columnMapByAccessString.end(); it++)
        {
        ColumnMap* columnMap = it->second;
        delete columnMap;
        }
    m_columnMapByAccessString.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeIterator::TableClassMap::Initialize()
    {
    m_classMap = m_ecdb.Schemas().Main().GetClassMap(m_class);
    if (m_classMap == nullptr)
        return;

    InitEndTableRelationshipMaps();
    InitPropertyColumnMaps();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2017
//---------------------------------------------------------------------------------------
void ChangeIterator::TableClassMap::InitEndTableRelationshipMaps()
    {
    SearchPropertyMapVisitor navVisitor(PropertyMap::Type::Navigation, false /*=recurseIntoCompoundTypes*/);
    m_classMap->GetPropertyMaps().AcceptVisitor(navVisitor);
    for (PropertyMap const* propertyMap : navVisitor.Results())
        {
        NavigationPropertyMap const& navPropertyMap = propertyMap->GetAs<NavigationPropertyMap>();

        NavigationPropertyMap::IdPropertyMap const& idPropertyMap = navPropertyMap.GetIdPropertyMap();
        DbColumn const& idColumn = idPropertyMap.GetColumn();

        if (idColumn.GetTable() != *m_tableMap.GetDbTable())
            continue; // Navigation property isn't really written to this table. todo: is this even possible?

        NavigationPropertyMap::RelECClassIdPropertyMap const& relClassIdPropertyMap = navPropertyMap.GetRelECClassIdPropertyMap();
        DbColumn const& relClassIdColumn = relClassIdPropertyMap.GetColumn();

        EndTableRelationshipMap* endTableRelMap = new EndTableRelationshipMap();
        endTableRelMap->m_relatedInstanceIdColumnMap = ColumnMap(idColumn.GetName(), m_tableMap.GetColumnIndexByName(idColumn.GetName()));
        if (relClassIdColumn.IsVirtual())
            endTableRelMap->m_relationshipClassId = relClassIdPropertyMap.GetDefaultClassId();
        else
            endTableRelMap->m_relationshipClassIdColumnMap = ColumnMap(relClassIdColumn.GetName(), m_tableMap.GetColumnIndexByName(relClassIdColumn.GetName()));

        m_endTableRelMaps.push_back(endTableRelMap);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2017
//---------------------------------------------------------------------------------------
void ChangeIterator::TableClassMap::FreeEndTableRelationshipMaps()
    {
    for (TableClassMap::EndTableRelationshipMap* map : m_endTableRelMaps)
        delete map;
    m_endTableRelMaps.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
void ChangeIterator::TableClassMap::InitPropertyColumnMaps()
    {
    BeAssert(m_classMap != nullptr);

    SearchPropertyMapVisitor visitor(PropertyMap::Type::SingleColumnData, true /*=recurseIntoCompoundTypes*/);
    m_classMap->GetPropertyMaps().AcceptVisitor(visitor);
    for (PropertyMap const* propertyMap : visitor.Results())
        {
        SingleColumnDataPropertyMap const& singleColumnMap = propertyMap->GetAs<SingleColumnDataPropertyMap>();

        DbColumn const& column = singleColumnMap.GetColumn();
        if (column.GetTable() != *m_tableMap.GetDbTable() || column.IsVirtual())
            continue; // Skip properties that don't belong to, or not written to the current table. 

        int columnIndex = m_tableMap.GetColumnIndexByName(column.GetName());
        m_columnMapByAccessString[singleColumnMap.GetAccessString()] = new ColumnMap(column.GetName(), columnIndex);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
bool ChangeIterator::TableClassMap::ContainsColumn(Utf8CP propertyAccessString) const { return m_columnMapByAccessString.find(propertyAccessString) != m_columnMapByAccessString.end(); }

//******************************************************************************
// ChangeIterator::TableClassMap::EndTableRelationshipMap
//******************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     08/2017
//---------------------------------------------------------------------------------------
DbColumn const* ChangeIterator::TableClassMap::EndTableRelationshipMap::GetForeignEndClassIdColumn(ECDbCR ecdb, ECRelationshipClassCR relationshipClass) const
    {
    auto it = m_foreignEndClassIdColumnMap.find(&relationshipClass);
    if (it != m_foreignEndClassIdColumnMap.end())
        return it->second;

    auto fkView = ForeignKeyPartitionView::CreateReadonly(ecdb.Schemas().Main(), relationshipClass);
    ForeignKeyPartitionView::Partition const* firstPartition = fkView->GetPartitions(true, true).front();
    if (!firstPartition)
        return nullptr;

    DbColumn const* foreignEndClassIdColumn = firstPartition->GetFromECClassIdColumn();
    BeAssert(foreignEndClassIdColumn != nullptr);

    m_foreignEndClassIdColumnMap[&relationshipClass] = foreignEndClassIdColumn;
    return foreignEndClassIdColumn;
    }

//******************************************************************************
// ChangeIterator::TableMap
//******************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeIterator::TableMap::Initialize(Utf8StringCR tableName)
    {
    DbSchema const& dbSchema = m_ecdb.Schemas().Main().GetDbSchema();
    m_tableName = tableName;

    DbTable const* dbTable = dbSchema.FindTable(tableName);
    if (!dbTable || !dbTable->IsValid() || dbSchema.IsNullTable(*dbTable))
        {
        m_isMapped = false;
        return;
        }

    m_isMapped = true;
    m_dbTable = dbTable;

    InitColumnIndexByName();
    InitSystemColumnMaps();
    return;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeIterator::TableMap::InitColumnIndexByName()
    {
    bvector<Utf8String> columnNames;
    m_ecdb.GetColumns(columnNames, m_tableName.c_str());

    for (int ii = 0; ii < (int) columnNames.size(); ii++)
        m_columnIndexByName[columnNames[ii]] = ii;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
void ChangeIterator::TableMap::InitSystemColumnMaps()
    {
    DbColumn const* instanceIdColumn = m_dbTable->FindFirst(DbColumn::Kind::ECInstanceId);
    Utf8StringCR instanceIdColumnName = instanceIdColumn->GetName();
    int instanceIdColumnIndex = GetColumnIndexByName(instanceIdColumnName);
    BeAssert(instanceIdColumnIndex >= 0);
    m_instanceIdColumnMap = ColumnMap(instanceIdColumnName, instanceIdColumnIndex);

    DbColumn const* classIdColumn = m_dbTable->FindFirst(DbColumn::Kind::ECClassId);
    if (classIdColumn->GetPersistenceType() != PersistenceType::Virtual)
        {
        Utf8StringCR classIdColumnName = classIdColumn->GetName();
        int classIdColumnIndex = GetColumnIndexByName(classIdColumnName);
        m_classIdColumnMap = ColumnMap(classIdColumnName, classIdColumnIndex);
        }
    else
        m_primaryClassId = QueryClassId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
ECClassId ChangeIterator::TableMap::QueryClassId() const
    {
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement(
        "SELECT DISTINCT ec_Class.Id FROM main.ec_Class "
        "JOIN main.ec_ClassMap ON ec_Class.Id = ec_ClassMap.ClassId "
        "JOIN main.ec_PropertyMap ON ec_ClassMap.ClassId = ec_PropertyMap.ClassId "
        "JOIN main.ec_Column ON ec_PropertyMap.ColumnId = ec_Column.Id "
        "JOIN main.ec_Table ON ec_Table.Id = ec_Column.TableId "
        "WHERE main.ec_Table.Name = :tableName AND "
        " (ec_ClassMap.MapStrategy <> " SQLVAL_MapStrategy_ForeignKeyRelationshipInSourceTable " AND ec_ClassMap.MapStrategy <> " SQLVAL_MapStrategy_ForeignKeyRelationshipInTargetTable ") AND "
        " ec_Column.IsVirtual = " SQLVAL_False " AND "
        " (ec_Column.ColumnKind & " SQLVAL_DbColumn_Kind_ECInstanceId "=" SQLVAL_DbColumn_Kind_ECInstanceId ")");
    BeAssert(stmt.IsValid());

    stmt->BindText(stmt->GetParameterIndex(":tableName"), m_tableName, Statement::MakeCopy::No);

    DbResult result = stmt->Step();
    if (result != BE_SQLITE_ROW)
        {
        BeAssert(false);
        return ECClassId();
        }

    const ECClassId ecClassId = stmt->GetValueId<ECClassId>(0);
    BeAssert(ecClassId.IsValid());
    BeAssert(BE_SQLITE_DONE == stmt->Step()); // There should be only one primary class mapped to a table (if there is no ecClassId column)    
    return ecClassId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
ECN::ECClassId ChangeIterator::TableMap::GetClassId() const
    {
    if (ContainsECClassIdColumn())
        {
        BeAssert(false && "Table can map to multiple classes");
        return ECClassId();
        }

    return m_primaryClassId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
int ChangeIterator::TableMap::GetColumnIndexByName(Utf8StringCR columnName) const
    {
    bmap<Utf8String, int>::const_iterator iter = m_columnIndexByName.find(columnName);
    return (iter != m_columnIndexByName.end()) ? iter->second : -1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::TableClassMap const* ChangeIterator::TableMap::GetTableClassMap(ECClassCR ecClass) const
    {
    auto iter = m_tableClassMaps.find(ecClass.GetId());
    if (iter != m_tableClassMaps.end())
        return iter->second.get();

    std::unique_ptr<TableClassMap> classMap = std::make_unique<TableClassMap>(m_ecdb, *this, ecClass);
    TableClassMap* classMapP = classMap.get();

    m_tableClassMaps[ecClass.GetId()] = std::move(classMap);
    return classMapP;
    }

//******************************************************************************
// ChangeIterator::SqlChange
//******************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeIterator::SqlChange::SqlChange(Changes::Change const& change) : m_sqlChange(change)
    {
    Utf8CP tableName;
    DbResult rc = m_sqlChange.GetOperation(&tableName, &m_nCols, &m_dbOpcode, &m_indirect);
    BeAssert(rc == BE_SQLITE_OK);
    UNUSED_VARIABLE(rc);

    m_tableName = tableName;

    Byte* pcols;
    int npcols;
    m_sqlChange.GetPrimaryKeyColumns(&pcols, &npcols);
    for (int ii = 0; ii < npcols; ii++)
        {
        if (pcols[ii] > 0)
            m_primaryKeyColumnIndices.insert(ii);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeIterator::SqlChange::GetValues(DbValue& oldValue, DbValue& newValue, int columnIndex) const
    {
    DbOpcode dbOpcode = GetDbOpcode();

    if (dbOpcode == DbOpcode::Delete || dbOpcode == DbOpcode::Update)
        oldValue = GetChange().GetValue(columnIndex, Changes::Change::Stage::Old);

    if (dbOpcode == DbOpcode::Insert || dbOpcode == DbOpcode::Update)
        newValue = GetChange().GetValue(columnIndex, Changes::Change::Stage::New);

    return;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
DbValue ChangeIterator::SqlChange::GetValue(int columnIndex) const
    {
    return GetChange().GetValue(columnIndex, (GetDbOpcode() == DbOpcode::Insert) ? Changes::Change::Stage::New : Changes::Change::Stage::Old);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
