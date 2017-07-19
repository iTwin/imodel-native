/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ChangeSummaryImpl.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SqlNames.h"
#include "ChangeSummaryImpl.h"

#define CHANGED_TABLES_TEMP_PREFIX "temp."
#define CHANGED_INSTANCES_TABLE_BASE_NAME "ec_ChangedInstances"
#define CHANGED_VALUES_TABLE_BASE_NAME "ec_ChangedValues"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

int ChangeSummary::s_count = 0;
IsChangedInstanceSqlFunction* ChangeSummary::s_isChangedInstanceSqlFunction = nullptr;
const Utf8String ChangeIterator::RowEntry::s_emptyString = "";
const Utf8String ChangeIterator::ColumnEntry::s_emptyString = "";

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
TableMap::TableMap(ECDbCR ecdb, Utf8StringCR tableName) : m_ecdb(ecdb)
    {
    Initialize(tableName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
// static 
TableMapPtr TableMap::Create(ECDbCR ecdb, Utf8StringCR tableName)
    {
    return new TableMap(ecdb, tableName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void TableMap::Initialize(Utf8StringCR tableName)
    {
    DbSchema const& dbSchema = m_ecdb.Schemas().GetDbMap().GetDbSchema();
    m_tableName = tableName;

    DbTable const* dbTable = dbSchema.FindTable(tableName.c_str());
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
void TableMap::InitColumnIndexByName()
    {
    bvector<Utf8String> columnNames;
    m_ecdb.GetColumns(columnNames, m_tableName.c_str());

    for (int ii = 0; ii < (int) columnNames.size(); ii++)
        m_columnIndexByName[columnNames[ii]] = ii;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
void TableMap::InitSystemColumnMaps()
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
ECClassId TableMap::QueryClassId() const
    {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(
        "SELECT DISTINCT ec_Class.Id FROM ec_Class "
        "JOIN ec_ClassMap ON ec_Class.Id = ec_ClassMap.ClassId "
        "JOIN ec_PropertyMap ON ec_ClassMap.ClassId = ec_PropertyMap.ClassId "
        "JOIN ec_Column ON ec_PropertyMap.ColumnId = ec_Column.Id "
        "JOIN ec_Table ON ec_Table.Id = ec_Column.TableId "
        "WHERE ec_Table.Name = :tableName AND "
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
ECN::ECClassId TableMap::GetClassId() const
    {
    if (ContainsECClassIdColumn())
        {
        BeAssert(false && "Table can map to multiple classes");
        return ECClassId();
        }

    return m_primaryClassId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
DbDupValue TableMap::QueryValueFromDb(Utf8StringCR physicalColumnName, ECInstanceId instanceId) const
    {
    Utf8PrintfString ecSql("SELECT %s FROM %s WHERE %s=?", physicalColumnName.c_str(), m_tableName.c_str(), GetIdColumn().GetName().c_str());
    CachedStatementPtr statement = m_ecdb.GetCachedStatement(ecSql.c_str());
    BeAssert(statement.IsValid());

    statement->BindId(1, instanceId);

    DbResult result = statement->Step();
    if (BE_SQLITE_ROW == result)
        return statement->GetDbValue(0);

    BeAssert(result == BE_SQLITE_DONE);
    return DbDupValue(nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
int TableMap::GetColumnIndexByName(Utf8StringCR columnName) const
    {
    bmap<Utf8String, int>::const_iterator iter = m_columnIndexByName.find(columnName);
    return (iter != m_columnIndexByName.end()) ? iter->second : -1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool TableMap::QueryInstance(ECInstanceId instanceId) const
    {
    DbDupValue value = QueryValueFromDb(GetIdColumn().GetName(), instanceId);
    if (!value.IsValid() || value.IsNull())
        return false;

    return value.GetValueId<ECInstanceId>().IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
TableClassMap const* TableMap::GetTableClassMap(ECClassCR ecClass) const
    {
    auto iter = m_tableClassMapsById.find(ecClass.GetId());
    if (iter != m_tableClassMapsById.end())
        return iter->second.get();

    TableClassMapPtr classMap = TableClassMap::Create(m_ecdb, *this, ecClass);
    m_tableClassMapsById[ecClass.GetId()] = classMap;

    return classMap.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
TableClassMap::TableClassMap(ECDbCR ecdb, TableMap const& tableMap, ECN::ECClassCR ecClass) : m_ecdb(ecdb), m_tableMap(tableMap), m_class(ecClass)
    {
    Initialize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
TableClassMap::~TableClassMap()
    {
    FreeEndTableRelationshipMaps();
    FreeColumnMaps();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void TableClassMap::FreeColumnMaps()
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
// static 
TableClassMapPtr TableClassMap::Create(ECDbCR ecdb, TableMap const& tableMap, ECN::ECClassCR ecClass)
    { 
    return new TableClassMap(ecdb, tableMap, ecClass); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void TableClassMap::Initialize()
    {
    m_classMap = m_ecdb.Schemas().GetDbMap().GetClassMap(m_class);
    if (!m_classMap)
        return;

    InitEndTableRelationshipMaps();
    InitPropertyColumnMaps();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2017
//---------------------------------------------------------------------------------------
void TableClassMap::InitEndTableRelationshipMaps()
    {
    SearchPropertyMapVisitor navVisitor(PropertyMap::Type::Navigation, false /*=recurseIntoCompoundTypes*/);
    m_classMap->GetPropertyMaps().AcceptVisitor(navVisitor);
    for (PropertyMap const* propertyMap : navVisitor.Results())
        {
        NavigationPropertyMap const& navPropertyMap = propertyMap->GetAs<NavigationPropertyMap>();

        NavigationPropertyMap::IdPropertyMap const& idPropertyMap = navPropertyMap.GetIdPropertyMap();
        DbColumn const& idColumn = idPropertyMap.GetColumn();

        if (idColumn.GetTable().GetId() != m_tableMap.GetDbTable()->GetId())
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
void TableClassMap::FreeEndTableRelationshipMaps()
    {
    for (TableClassMap::EndTableRelationshipMap* map: m_endTableRelMaps)
        delete map;
    m_endTableRelMaps.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
void TableClassMap::InitPropertyColumnMaps()
    {
    BeAssert(m_classMap != nullptr);

    SearchPropertyMapVisitor visitor(PropertyMap::Type::SingleColumnData, true /*=recurseIntoCompoundTypes*/);
    m_classMap->GetPropertyMaps().AcceptVisitor(visitor);
    for (PropertyMap const* propertyMap : visitor.Results())
        {
        SingleColumnDataPropertyMap const& singleColumnMap = propertyMap->GetAs<SingleColumnDataPropertyMap>();

        DbColumn const& column = singleColumnMap.GetColumn();
        if (column.GetTable().GetId() != m_tableMap.GetDbTable()->GetId() || column.IsVirtual())
            continue; // Skip properties that don't belong to, or not written to the current table. 
        
        int columnIndex = m_tableMap.GetColumnIndexByName(column.GetName());
        m_columnMapByAccessString[singleColumnMap.GetAccessString()] = new ColumnMap(column.GetName(), columnIndex);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
bool TableClassMap::ContainsColumn(Utf8CP propertyAccessString) const
    {
    return m_columnMapByAccessString.find(propertyAccessString) != m_columnMapByAccessString.end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
SqlChange::SqlChange(Changes::Change const& change) : m_sqlChange(change)
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
void SqlChange::GetValues(DbValue& oldValue, DbValue& newValue, int columnIndex) const
    {
    DbOpcode dbOpcode = GetDbOpcode();

    if (dbOpcode == DbOpcode::Delete || dbOpcode == DbOpcode::Update)
        oldValue = GetChange().GetValue(columnIndex, Changes::Change::Stage::Old);

    if (dbOpcode == DbOpcode::Insert || dbOpcode == DbOpcode::Update)
        newValue = GetChange().GetValue(columnIndex, Changes::Change::Stage::New);

    return;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void SqlChange::GetValueIds(ECInstanceId& oldInstanceId, ECInstanceId& newInstanceId, int idColumnIndex) const
    {
    oldInstanceId = newInstanceId = ECInstanceId();

    DbValue oldValue(nullptr), newValue(nullptr);
    GetValues(oldValue, newValue, idColumnIndex);

    if (oldValue.IsValid() && !oldValue.IsNull())
        oldInstanceId = oldValue.GetValueId<ECInstanceId>();

    if (newValue.IsValid() && !newValue.IsNull())
        newInstanceId = newValue.GetValueId<ECInstanceId>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
DbValue SqlChange::GetValue(int columnIndex) const
    {
    return GetChange().GetValue(columnIndex, (GetDbOpcode() == DbOpcode::Insert) ? Changes::Change::Stage::New : Changes::Change::Stage::Old);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
InstancesTable::InstancesTable(ChangeSummaryCR changeSummary, int nameSuffix) : m_changeSummary(changeSummary), m_ecdb(changeSummary.GetDb()), m_nameSuffix(nameSuffix)
    {
    m_instancesTableNameNoPrefix = Utf8PrintfString(CHANGED_INSTANCES_TABLE_BASE_NAME "_%d", m_nameSuffix);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void InstancesTable::Initialize()
    {
    CreateTable();
    PrepareStatements();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void InstancesTable::Free()
    {
    FinalizeStatements();
    ClearTable();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
Utf8String InstancesTable::GetName() const
    {
    Utf8PrintfString tableName(CHANGED_TABLES_TEMP_PREFIX "%s", GetNameNoPrefix().c_str());
    return tableName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void InstancesTable::CreateTable()
    {
    Utf8String tableName = GetName();
    if (m_ecdb.TableExists(tableName.c_str()))
        return;

    DbResult result = m_ecdb.CreateTable(tableName.c_str(),
                                         "[ClassId] INTEGER not null, [InstanceId] INTEGER not null, [DbOpcode] INTEGER not null, [Indirect] INTEGER not null, [TableName] TEXT not null, PRIMARY KEY (ClassId, InstanceId)");
    BeAssert(result == BE_SQLITE_OK);

    Utf8PrintfString sqlIdx("CREATE INDEX " CHANGED_TABLES_TEMP_PREFIX "idx_%s_op ON [%s](DbOpcode)", m_instancesTableNameNoPrefix.c_str(), m_instancesTableNameNoPrefix.c_str());
    result = m_ecdb.ExecuteSql(sqlIdx.c_str());
    BeAssert(result == BE_SQLITE_OK);

    Utf8PrintfString sqlIdx2("CREATE INDEX " CHANGED_TABLES_TEMP_PREFIX "idx_%s_table ON [%s](TableName,InstanceId)", m_instancesTableNameNoPrefix.c_str(), m_instancesTableNameNoPrefix.c_str());
    result = m_ecdb.ExecuteSql(sqlIdx2.c_str());
    BeAssert(result == BE_SQLITE_OK);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void InstancesTable::ClearTable()
    {
    Utf8String tableName = GetName();
    BeAssert(m_ecdb.TableExists(tableName.c_str()));

    Utf8PrintfString sqlDeleteAll("DELETE FROM %s", tableName.c_str());
    DbResult result = m_ecdb.ExecuteSql(sqlDeleteAll.c_str());
    BeAssert(result == BE_SQLITE_OK);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void InstancesTable::PrepareStatements()
    {
    Utf8String instancesTableName = GetName();
    BeAssert(m_ecdb.TableExists(instancesTableName.c_str()));

    DbResult result;

    BeAssert(!m_instancesTableInsert.IsPrepared());
    Utf8PrintfString insertSql("INSERT INTO %s (ClassId,InstanceId,DbOpcode,Indirect,TableName) VALUES(?1,?2,?3,?4,?5)", instancesTableName.c_str());
    result = m_instancesTableInsert.Prepare(m_ecdb, insertSql.c_str());
    BeAssert(result == BE_SQLITE_OK);

    BeAssert(!m_instancesTableUpdate.IsPrepared());
    Utf8PrintfString updateSql("UPDATE %s SET DbOpcode=?3,Indirect=?4 WHERE ClassId=?1 AND InstanceId=?2", instancesTableName.c_str());
    result = m_instancesTableUpdate.Prepare(m_ecdb, updateSql.c_str());
    BeAssert(result == BE_SQLITE_OK);

    BeAssert(!m_instancesTableSelect.IsPrepared());
    Utf8PrintfString selectSql("SELECT DbOpcode, Indirect,TableName FROM %s WHERE ClassId=?1 AND InstanceId=?2", instancesTableName.c_str());
    result = m_instancesTableSelect.Prepare(m_ecdb, selectSql.c_str());
    BeAssert(result == BE_SQLITE_OK);

    BeAssert(!m_instancesTableDelete.IsPrepared());
    Utf8PrintfString deleteSql("DELETE FROM %s WHERE ClassId=?1 AND InstanceId=?2", instancesTableName.c_str());
    result = m_instancesTableDelete.Prepare(m_ecdb, deleteSql.c_str());
    BeAssert(result == BE_SQLITE_OK);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void InstancesTable::FinalizeStatements()
    {
    m_instancesTableInsert.Finalize();
    m_instancesTableUpdate.Finalize();
    m_instancesTableSelect.Finalize();
    m_instancesTableDelete.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void InstancesTable::Insert(ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect, Utf8StringCR tableName)
    {
    m_instancesTableInsert.Reset();
    m_instancesTableInsert.BindId(1, classId);
    m_instancesTableInsert.BindId(2, instanceId);
    m_instancesTableInsert.BindInt(3, (int) dbOpcode);
    m_instancesTableInsert.BindInt(4, indirect);
    m_instancesTableInsert.BindText(5, tableName, Statement::MakeCopy::No);

    DbResult result = m_instancesTableInsert.Step();
    BeAssert(result == BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void InstancesTable::Update(ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect)
    {
    m_instancesTableUpdate.Reset();
    m_instancesTableUpdate.BindId(1, classId);
    m_instancesTableUpdate.BindId(2, instanceId);
    m_instancesTableUpdate.BindInt(3, (int) dbOpcode);
    m_instancesTableUpdate.BindInt(4, indirect);

    DbResult result = m_instancesTableUpdate.Step();
    BeAssert(result == BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void InstancesTable::InsertOrUpdate(ChangeSummary::InstanceCR instance)
    {
    /*
    * Here's the logic to consolidate new changes with the ones
    * previously found:
    *
    * not-found    + new:*       = Insert new entry
    *
    * found:UPDATE + new:INSERT  = Update existing entry to INSERT
    * found:UPDATE + new:DELETE  = Update existing entry to DELETE
    * 
    * <all other cases keep existing entry>
    */

    ECClassId classId = instance.GetClassId();
    ECInstanceId instanceId = instance.GetInstanceId();
    DbOpcode dbOpcode = instance.GetDbOpcode();
    int indirect = instance.GetIndirect();

    ChangeSummary::Instance foundInstance = QueryInstance(classId, instanceId);
    if (!foundInstance.IsValid())
        {
        Insert(classId, instanceId, dbOpcode, indirect, instance.GetTableName());
        return;
        }

    if (foundInstance.GetDbOpcode() == DbOpcode::Update && dbOpcode != DbOpcode::Update)
        {
        Update(classId, instanceId, dbOpcode, indirect);
        return;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void InstancesTable::Delete(ECClassId classId, ECInstanceId instanceId)
    {
    m_instancesTableDelete.Reset();
    m_instancesTableDelete.BindId(1, classId);
    m_instancesTableDelete.BindId(2, instanceId);

    DbResult result = m_instancesTableDelete.Step();
    BeAssert(result == BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::Instance InstancesTable::QueryInstance(ECClassId classId, ECInstanceId instanceId) const
    {
    m_instancesTableSelect.Reset();
    m_instancesTableSelect.BindId(1, classId);
    m_instancesTableSelect.BindId(2, instanceId);

    DbResult result = m_instancesTableSelect.Step();
    if (result == BE_SQLITE_ROW)
        {
        ChangeSummary::Instance instance(m_changeSummary, classId, instanceId, (DbOpcode) m_instancesTableSelect.GetValueInt(0), m_instancesTableSelect.GetValueInt(1), Utf8String(m_instancesTableSelect.GetValueText(2)));
        return instance;
        }

    BeAssert(result == BE_SQLITE_DONE);

    ChangeSummary::Instance invalidInstance;
    return invalidInstance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool InstancesTable::ContainsInstance(ECClassId classId, ECInstanceId instanceId) const
    {
    m_instancesTableSelect.Reset();
    m_instancesTableSelect.BindId(1, classId);
    m_instancesTableSelect.BindId(2, instanceId);

    DbResult result = m_instancesTableSelect.Step();
    BeAssert(result == BE_SQLITE_ROW || BE_SQLITE_DONE);

    return result == BE_SQLITE_ROW;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
ECClassId InstancesTable::QueryClassId(Utf8StringCR tableName, ECInstanceId instanceId) const
    {
    Utf8String instancesTableName = GetName();

    Utf8PrintfString sql("SELECT ClassId FROM %s WHERE TableName=?1 AND InstanceId=?2", instancesTableName.c_str());
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(sql.c_str());
    BeAssert(stmt.IsValid());

    stmt->BindText(1, tableName, Statement::MakeCopy::No);
    stmt->BindId(2, instanceId);

    DbResult result = stmt->Step();
    if (result != BE_SQLITE_ROW)
        return ECClassId();

    return stmt->GetValueId<ECClassId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
ValuesTable::ValuesTable(InstancesTable const& instancesTable) : m_instancesTable(instancesTable), m_changeSummary(instancesTable.GetChangeSummary()), m_ecdb(instancesTable.GetDb())
    {
    m_valuesTableNameNoPrefix = Utf8PrintfString(CHANGED_VALUES_TABLE_BASE_NAME "_%d", instancesTable.GetNameSuffix());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ValuesTable::Initialize()
    {
    CreateTable();
    PrepareStatements();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ValuesTable::Free()
    {
    FinalizeStatements();
    ClearTable();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
Utf8String ValuesTable::GetName() const
    {
    Utf8PrintfString tableName(CHANGED_TABLES_TEMP_PREFIX "%s", GetNameNoPrefix().c_str());
    return tableName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ValuesTable::CreateTable()
    {
    Utf8String valuesTableName = GetName();
    if (m_ecdb.TableExists(valuesTableName.c_str()))
        return;

    Utf8PrintfString sql("[Id] INTEGER NOT NULL, [ClassId] INTEGER NOT NULL, [InstanceId] INTEGER NOT NULL, [AccessString] TEXT not null, [OldValue] BLOB, [NewValue] BLOB,  "
                         "PRIMARY KEY ([Id]), FOREIGN KEY ([ClassId],[InstanceId]) REFERENCES %s ON DELETE CASCADE ON UPDATE NO ACTION", m_instancesTable.GetNameNoPrefix().c_str());

    DbResult result = m_ecdb.CreateTable(valuesTableName.c_str(), sql.c_str());
    BeAssert(result == BE_SQLITE_OK);

    Utf8PrintfString sqlIdx("CREATE UNIQUE INDEX " CHANGED_TABLES_TEMP_PREFIX "idx_%s_AccessStrUnique ON [%s] (ClassId, InstanceId, AccessString)", m_valuesTableNameNoPrefix.c_str(), m_valuesTableNameNoPrefix.c_str());
    result = m_ecdb.ExecuteSql(sqlIdx.c_str());
    BeAssert(result == BE_SQLITE_OK);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ValuesTable::ClearTable()
    {
    Utf8String tableName = GetName();
    BeAssert(m_ecdb.TableExists(tableName.c_str()));

    Utf8PrintfString sqlDeleteAll("DELETE FROM %s", tableName.c_str());
    DbResult result = m_ecdb.ExecuteSql(sqlDeleteAll.c_str());
    BeAssert(result == BE_SQLITE_OK);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ValuesTable::PrepareStatements()
    {
    Utf8String valuesTableName = GetName();
    BeAssert(m_ecdb.TableExists(valuesTableName.c_str()));

    BeAssert(!m_valuesTableInsert.IsPrepared());
    Utf8PrintfString insertSql("INSERT INTO %s (ClassId,InstanceId,AccessString,OldValue,NewValue) VALUES(?1,?2,?3,?4,?5)", valuesTableName.c_str());
    DbResult result = m_valuesTableInsert.Prepare(m_ecdb, insertSql.c_str());
    BeAssert(result == BE_SQLITE_OK);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ValuesTable::FinalizeStatements()
    {
    m_valuesTableInsert.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ValuesTable::Insert(ECN::ECClassId classId, ECInstanceId instanceId, Utf8CP accessString, DbValue const& oldValue, DbValue const& newValue)
    {
    Statement& statement = m_valuesTableInsert;
    statement.Reset();
    statement.BindId(1, classId);
    statement.BindId(2, instanceId);
    statement.BindText(3, accessString, Statement::MakeCopy::No);

    if (oldValue.IsValid())
        statement.BindDbValue(4, oldValue);
    else
        statement.BindNull(4);

    if (newValue.IsValid())
        statement.BindDbValue(5, newValue);
    else
        statement.BindNull(5);

    DbResult result = statement.Step();
    BeAssert(result == BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ValuesTable::Insert(ECN::ECClassId classId, ECInstanceId instanceId, Utf8CP accessString, ECClassId oldValue, ECClassId newValue)
    {
    Statement& statement = m_valuesTableInsert;

    statement.Reset();
    statement.BindId(1, classId);
    statement.BindId(2, instanceId);
    statement.BindText(3, accessString, Statement::MakeCopy::No);

    if (oldValue.IsValid())
        statement.BindId(4, oldValue);
    else
        statement.BindNull(4);

    if (newValue.IsValid())
        statement.BindId(5, newValue);
    else
        statement.BindNull(5);

    DbResult result = statement.Step();
    BeAssert(result == BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ValuesTable::Insert(ECN::ECClassId classId, ECInstanceId instanceId, Utf8CP accessString, ECInstanceId oldValue, ECInstanceId newValue)
    {
    Statement& statement = m_valuesTableInsert;

    statement.Reset();
    statement.BindId(1, classId);
    statement.BindId(2, instanceId);
    statement.BindText(3, accessString, Statement::MakeCopy::No);

    if (oldValue.IsValid())
        statement.BindId(4, oldValue);
    else
        statement.BindNull(4);

    if (newValue.IsValid())
        statement.BindId(5, newValue);
    else
        statement.BindNull(5);

    DbResult result = statement.Step();
    BeAssert(result == BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
ChangeExtractor::ChangeExtractor(ChangeSummary const& changeSummary, InstancesTable& instancesTable, ValuesTable& valuesTable) 
    : m_changeSummary(changeSummary), m_ecdb(m_changeSummary.GetDb()), m_instancesTable(instancesTable), m_valuesTable(valuesTable) 
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
int ChangeExtractor::GetFirstColumnIndex(PropertyMap const* propertyMap, ChangeIterator::RowEntry const& rowEntry) const
    {
    if (propertyMap == nullptr)
        return -1;

    GetColumnsPropertyMapVisitor columnsDisp(PropertyMap::Type::All, /* doNotSkipSystemPropertyMaps */ true);
    propertyMap->AcceptVisitor(columnsDisp);
    if (columnsDisp.GetColumns().size() != 1)
        return -1;

    return rowEntry.GetTableMap()->GetColumnIndexByName(columnsDisp.GetColumns()[0]->GetName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
BentleyStatus ChangeExtractor::FromChangeSet(IChangeSet& changeSet, bool includeRelationshipInstances)
    {
    // Pass 1
    BentleyStatus status = FromChangeSet(changeSet, ExtractOption::InstancesOnly);
    if (SUCCESS != status || !includeRelationshipInstances)
        return status;

    // Pass 2
    return FromChangeSet(changeSet, ExtractOption::RelationshipInstancesOnly);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
BentleyStatus ChangeExtractor::FromChangeSet(IChangeSet& changeSet, ExtractOption extractOption)
    {
    ChangeIterator iter(m_ecdb, changeSet);
    for (ChangeIterator::RowEntry const& rowEntry : iter)
        {
        if (!rowEntry.IsMapped())
            {
            LOG.warningv("ChangeSet includes changes to unmapped table %s", rowEntry.GetTableName().c_str());
            continue; // There are tables which are just not mapped to EC that we simply don't care about (e.g., be_Prop table)
            }

        if (rowEntry.GetTableName().StartsWith("ec_"))
            {
            rowEntry.GetSqlChange()->GetChange().Dump(m_ecdb, false, 1);
            LOG.errorv("ChangeSet includes changes to an EC system table, and that may represent an ECSchema change. Change summaries are not reliable.", rowEntry.GetTableName().c_str());
            BeAssert(false && "ChangeSet includes changes to an EC system table, and that may represent an ECSchema change. Change summaries are not reliable.");
            return ERROR;
            }

        ECClassCP primaryClass = rowEntry.GetPrimaryClass();
        ECInstanceId primaryInstanceId = rowEntry.GetPrimaryInstanceId();
        if (primaryClass == nullptr || !primaryInstanceId.IsValid())
            {
            LOG.errorv("Could not determine the primary instance corresponding to a change to table %s", rowEntry.GetTableName().c_str());
            BeAssert(false && "Could not determine the primary instance corresponding to a change.");
            return ERROR;
            }

        if (extractOption == ExtractOption::InstancesOnly && !primaryClass->IsRelationshipClass())
            {
            ExtractInstance(rowEntry);
            continue;
            }
        
        if (extractOption == ExtractOption::RelationshipInstancesOnly)
            {
            ExtractRelInstances(rowEntry);
            continue;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void ChangeExtractor::ExtractInstance(ChangeIterator::RowEntry const& rowEntry)
    {
    ChangeSummary::Instance instance(m_changeSummary, rowEntry.GetPrimaryClass()->GetId(), rowEntry.GetPrimaryInstanceId(), rowEntry.GetDbOpcode(), rowEntry.GetIndirect(), rowEntry.GetTableName());
    bool recordOnlyIfUpdatedProperties = (rowEntry.GetDbOpcode() == DbOpcode::Update);
    RecordInstance(instance, rowEntry, recordOnlyIfUpdatedProperties);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void ChangeExtractor::ExtractRelInstances(ChangeIterator::RowEntry const& rowEntry)
    {
    ECClassCP primaryClass = rowEntry.GetPrimaryClass();

    ClassMap const* classMap = m_ecdb.Schemas().GetDbMap().GetClassMap(*primaryClass);
    BeAssert(classMap != nullptr);

    ClassMap::Type type = classMap->GetType();
    if (type == ClassMap::Type::RelationshipLinkTable)
        {
        RelationshipClassLinkTableMap const& relClassMap =  classMap->GetAs<RelationshipClassLinkTableMap>();

        ExtractRelInstanceInLinkTable(rowEntry, relClassMap);
        return;
        }

    TableClassMap const* tableClassMap = rowEntry.GetTableMap()->GetTableClassMap(*primaryClass);
    BeAssert(tableClassMap != nullptr);

    for (TableClassMap::EndTableRelationshipMap const* endTableRelMap : tableClassMap->GetEndTableRelationshipMaps())
        {
        ExtractRelInstanceInEndTable(rowEntry, *endTableRelMap);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void ChangeExtractor::ExtractRelInstanceInLinkTable(ChangeIterator::RowEntry const& rowEntry, RelationshipClassLinkTableMap const& relClassMap)
    {
    ChangeSummary::Instance instance(m_changeSummary, rowEntry.GetPrimaryClass()->GetId(), rowEntry.GetPrimaryInstanceId(), rowEntry.GetDbOpcode(), rowEntry.GetIndirect(), rowEntry.GetTableName());

    ECInstanceKey oldSourceInstanceKey, newSourceInstanceKey;
    GetRelEndInstanceKeys(oldSourceInstanceKey, newSourceInstanceKey, rowEntry, relClassMap, instance.GetInstanceId(), ECRelationshipEnd_Source);

    ECInstanceKey oldTargetInstanceKey, newTargetInstanceKey;
    GetRelEndInstanceKeys(oldTargetInstanceKey, newTargetInstanceKey, rowEntry, relClassMap, instance.GetInstanceId(), ECRelationshipEnd_Target);

    RecordRelInstance(instance, rowEntry, oldSourceInstanceKey, newSourceInstanceKey, oldTargetInstanceKey, newTargetInstanceKey);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
ECN::ECClassId ChangeExtractor::GetClassIdFromColumn(TableMap const& tableMap, DbColumn const& classIdColumn, ECInstanceId instanceId) const
    {
    // Search in all changes
    ECClassId classId = m_instancesTable.QueryClassId(tableMap.GetTableName(), instanceId);
    if (classId.IsValid())
        return classId;

    // Search in table itself
    classId = tableMap.QueryValueId<ECClassId>(classIdColumn.GetName(), instanceId);
    BeAssert(classId.IsValid());
    
    return classId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::ExtractRelInstanceInEndTable(ChangeIterator::RowEntry const& rowEntry, TableClassMap::EndTableRelationshipMap const& endTableRelMap)
    {
    // Check if the other end was/is valid to determine if there's really a relationship that was inserted/updated/deleted
    ColumnMap const& otherEndColumnMap = endTableRelMap.m_relatedInstanceIdColumnMap;
    ECInstanceId oldOtherEndInstanceId, newOtherEndInstanceId;
    rowEntry.GetSqlChange()->GetValueIds(oldOtherEndInstanceId, newOtherEndInstanceId, otherEndColumnMap.GetIndex());
    if (!oldOtherEndInstanceId.IsValid() && !newOtherEndInstanceId.IsValid())
        return;

    // Evaluate the relationship information
    ECN::ECClassId relClassId = endTableRelMap.m_relationshipClassId;
    if (!relClassId.IsValid())
        {
        ColumnMap const& relClassIdColumnMap = endTableRelMap.m_relationshipClassIdColumnMap;

        relClassId = rowEntry.GetClassIdFromChangeOrTable(relClassIdColumnMap.GetName().c_str(), rowEntry.GetPrimaryInstanceId());
        BeAssert(relClassId.IsValid());
        }
    ECInstanceId relInstanceId = rowEntry.GetPrimaryInstanceId();
    
    ECN::ECClassCP relClass = m_ecdb.Schemas().GetClass(relClassId);
    BeAssert(relClass != nullptr);
    RelationshipClassEndTableMap const* relClassMap = dynamic_cast<RelationshipClassEndTableMap const*> (m_ecdb.Schemas().GetDbMap().GetClassMap(*relClass));
    BeAssert(relClassMap != nullptr);
    
    // Setup this end of the relationship (Note: EndInstanceId = RelationshipInstanceId)
    ECN::ECClassId thisEndClassId = rowEntry.GetPrimaryClass()->GetId();
    ECInstanceKey thisEndInstanceKey(thisEndClassId, relInstanceId);
    ECN::ECRelationshipEnd thisEnd = relClassMap->GetForeignEnd();

    // Setup other end of relationship
    RelationshipClassEndTableMap::Partition const* firstPartition = relClassMap->GetPartitionView().GetPhysicalPartitions().front();
    if (!firstPartition)
        return;
    ECN::ECRelationshipEnd otherEnd = (thisEnd == ECRelationshipEnd_Source) ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
    DbColumn const* otherEndClassIdColumnCP = firstPartition->GetConstraintECClassId(otherEnd);
    if (otherEndClassIdColumnCP == nullptr)
        {
        BeAssert(false && "Need to adjust code when constraint ecclassid column is nullptr");
        return;
        }

    DbColumn const& otherEndClassIdColumn = *otherEndClassIdColumnCP;

    ECClassId oldOtherEndClassId, newOtherEndClassId;
    if (otherEndClassIdColumn.IsVirtual())
        {
        // The table at the end contains a single class only - just use the relationship to get the end class
        oldOtherEndClassId = newOtherEndClassId = GetRelEndClassIdFromRelClass(relClass->GetRelationshipClassCP(), otherEnd);
        }
    else
        {
        TableMap const* otherEndTableMap = rowEntry.GetChangeIterator().GetTableMap(otherEndClassIdColumn.GetTable().GetName());
        BeAssert(otherEndTableMap != nullptr);

        if (newOtherEndInstanceId.IsValid())
            newOtherEndClassId = GetClassIdFromColumn(*otherEndTableMap, otherEndClassIdColumn, newOtherEndInstanceId);
        if (oldOtherEndInstanceId.IsValid())
            oldOtherEndClassId = GetClassIdFromColumn(*otherEndTableMap, otherEndClassIdColumn, oldOtherEndInstanceId);
        }

    ECInstanceKey oldOtherEndInstanceKey, newOtherEndInstanceKey;
    if (newOtherEndInstanceId.IsValid())
        newOtherEndInstanceKey = ECInstanceKey(newOtherEndClassId, newOtherEndInstanceId);
    if (oldOtherEndInstanceId.IsValid())
        oldOtherEndInstanceKey = ECInstanceKey(oldOtherEndClassId, oldOtherEndInstanceId);

    // Setup the change instance of the relationship
    DbOpcode relDbOpcode;
    if (newOtherEndInstanceKey.IsValid() && !oldOtherEndInstanceKey.IsValid())
        relDbOpcode = DbOpcode::Insert;
    else if (!newOtherEndInstanceKey.IsValid() && oldOtherEndInstanceKey.IsValid())
        relDbOpcode = DbOpcode::Delete;
    else /* if (newOtherEndInstanceKey.IsValid() && oldOtherEndInstanceKey.IsValid()) */
        relDbOpcode = DbOpcode::Update;

    ECInstanceKeyCP oldSourceInstanceKey, newSourceInstanceKey, oldTargetInstanceKey, newTargetInstanceKey;
    ECInstanceKey invalidKey;
    oldSourceInstanceKey = newSourceInstanceKey = oldTargetInstanceKey = newTargetInstanceKey = nullptr;
    if (thisEnd == ECRelationshipEnd_Source)
        {
        oldSourceInstanceKey = (relDbOpcode != DbOpcode::Insert) ? &thisEndInstanceKey : &invalidKey;
        oldTargetInstanceKey = &oldOtherEndInstanceKey;
        newSourceInstanceKey = (relDbOpcode != DbOpcode::Delete) ? &thisEndInstanceKey : &invalidKey;
        newTargetInstanceKey = &newOtherEndInstanceKey;
        }
    else
        {
        oldSourceInstanceKey = &oldOtherEndInstanceKey;
        oldTargetInstanceKey = (relDbOpcode != DbOpcode::Insert) ? &thisEndInstanceKey : &invalidKey;
        newSourceInstanceKey = &newOtherEndInstanceKey;
        newTargetInstanceKey = (relDbOpcode != DbOpcode::Delete) ? &thisEndInstanceKey : &invalidKey;
        }

    ChangeSummary::Instance instance(m_changeSummary, relClassId, relInstanceId, relDbOpcode, rowEntry.GetIndirect(), rowEntry.GetTableName());

    RecordRelInstance(instance, rowEntry, *oldSourceInstanceKey, *newSourceInstanceKey, *oldTargetInstanceKey, *newTargetInstanceKey);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
bool ChangeExtractor::RecordRelInstance(ChangeSummary::InstanceCR instance, ChangeIterator::RowEntry const& rowEntry, ECInstanceKeyCR oldSourceKey, ECInstanceKeyCR newSourceKey, ECInstanceKeyCR oldTargetKey, ECInstanceKeyCR newTargetKey)
    {
    bool recordOnlyIfUpdatedProperties = false; // Even if any of the properties of the relationship is not updated, the relationship needs to be recorded since the source/target keys would have changed (to get here)
    RecordInstance(instance, rowEntry, recordOnlyIfUpdatedProperties);

    ECClassId classId = instance.GetClassId();
    ECInstanceId instanceId = instance.GetInstanceId();

    m_valuesTable.Insert(classId, instanceId, ECDBSYS_PROP_SourceECClassId, oldSourceKey.IsValid() ? oldSourceKey.GetClassId() : ECClassId(), newSourceKey.IsValid() ? newSourceKey.GetClassId() : ECClassId());
    m_valuesTable.Insert(classId, instanceId, ECDBSYS_PROP_SourceECInstanceId, oldSourceKey.IsValid() ? oldSourceKey.GetInstanceId() : ECInstanceId(), newSourceKey.IsValid() ? newSourceKey.GetInstanceId() : ECInstanceId());
    m_valuesTable.Insert(classId, instanceId, ECDBSYS_PROP_TargetECClassId, oldTargetKey.IsValid() ? oldTargetKey.GetClassId() : ECClassId(), newTargetKey.IsValid() ? newTargetKey.GetClassId() : ECClassId());
    m_valuesTable.Insert(classId, instanceId, ECDBSYS_PROP_TargetECInstanceId, oldTargetKey.IsValid() ? oldTargetKey.GetInstanceId() : ECInstanceId(), newTargetKey.IsValid() ? newTargetKey.GetInstanceId() : ECInstanceId());

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void ChangeExtractor::RecordInstance(ChangeSummary::InstanceCR instance, ChangeIterator::RowEntry const& rowEntry, bool recordOnlyIfUpdatedProperties)
    {
    ECN::ECClassId classId = instance.GetClassId();
    ECInstanceId instanceId = instance.GetInstanceId();

    bool removeIfNotUpdatedProperties = false;
    if (recordOnlyIfUpdatedProperties)
        removeIfNotUpdatedProperties = !m_instancesTable.ContainsInstance(classId, instanceId);

    m_instancesTable.InsertOrUpdate(instance);

    bool updatedProperties = false;
    ECN::ECClassCP ecClass = m_ecdb.Schemas().GetClass(classId);
    for (ChangeIterator::ColumnEntry const& columnEntry : rowEntry.MakeColumnIterator(*ecClass))
        {
        if (columnEntry.IsPrimaryKeyColumn())
            continue;  // Primary key columns need not be included in the values table

        if (RecordValue(instance, columnEntry))
            updatedProperties = true;
        }

    if (removeIfNotUpdatedProperties && !updatedProperties)
        {
        // If recording an update for the first time, and none of the properties have really been updated, remove record of the updated instance
        m_instancesTable.Delete(classId, instanceId);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
bool ChangeExtractor::RecordValue(ChangeSummary::InstanceCR instance, ChangeIterator::ColumnEntry const& columnEntry)
    {
    DbOpcode dbOpcode = instance.GetDbOpcode();

    DbDupValue oldValue(nullptr);
    if (dbOpcode != DbOpcode::Insert)
        oldValue = columnEntry.GetValue(Changes::Change::Stage::Old);

    DbDupValue newValue(nullptr);
    if (dbOpcode != DbOpcode::Delete)
        newValue = columnEntry.GetValue(Changes::Change::Stage::New);

    bool hasOldValue = oldValue.IsValid() && !oldValue.IsNull();
    bool hasNewValue = newValue.IsValid() && !newValue.IsNull();

    if (!hasOldValue && !hasNewValue) // Do not persist entirely empty fields
        return false;

    m_valuesTable.Insert(instance.GetClassId(), instance.GetInstanceId(), columnEntry.GetPropertyAccessString().c_str(), oldValue, newValue);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::GetRelEndInstanceKeys(ECInstanceKey& oldInstanceKey, ECInstanceKey& newInstanceKey, ChangeIterator::RowEntry const& rowEntry, RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd) const
    {
    oldInstanceKey = newInstanceKey = ECInstanceKey();

    int instanceIdColumnIndex = GetFirstColumnIndex(relClassMap.GetConstraintECInstanceIdPropMap(relEnd), rowEntry);
    BeAssert(instanceIdColumnIndex >= 0);

    ECInstanceId oldEndInstanceId, newEndInstanceId;
    rowEntry.GetSqlChange()->GetValueIds(oldEndInstanceId, newEndInstanceId, instanceIdColumnIndex);

    if (newEndInstanceId.IsValid())
        {
        ECClassId newClassId = GetRelEndClassId(rowEntry, relClassMap, relInstanceId, relEnd, newEndInstanceId);
        BeAssert(newClassId.IsValid());
        newInstanceKey = ECInstanceKey(newClassId, newEndInstanceId);
        }

    if (oldEndInstanceId.IsValid())
        {
        ECClassId oldClassId = GetRelEndClassId(rowEntry, relClassMap, relInstanceId, relEnd, oldEndInstanceId);
        BeAssert(oldClassId.IsValid());
        oldInstanceKey = ECInstanceKey(oldClassId, oldEndInstanceId);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
ECN::ECClassId ChangeExtractor::GetRelEndClassId(ChangeIterator::RowEntry const& rowEntry, RelationshipClassMapCR relationshipClassMap, ECInstanceId relationshipInstanceId, ECN::ECRelationshipEnd relEnd, ECInstanceId relEndInstanceId) const
    {
    ConstraintECClassIdPropertyMap const* classIdPropMap = relationshipClassMap.GetConstraintECClassIdPropMap(relEnd);
    if (classIdPropMap == nullptr)
        {
        BeAssert(false);
        return ECClassId();
        }

    GetColumnsPropertyMapVisitor columnsDisp(PropertyMap::Type::All, /* doNotSkipSystemPropertyMaps */ true);
    classIdPropMap->AcceptVisitor(columnsDisp);
    if (columnsDisp.GetColumns().size() != 1)
        {
        BeAssert(false);
        return ECClassId();
        }

    DbColumn const* classIdColumn = columnsDisp.GetColumns()[0];

    /*
    * There are various cases that need to be considered to resolve the constrained ECClassId at the end of a relationship:
    *
    * 1. By definition the end can point to only one class, and the ECClassId is implicit.
    *    + We use the mapping/relationship class to obtain the class id in this case.
    *    + We detect this case by checking if the ClassId constraint column is not persisted (i.e., PersistenceType = "Virtual")
    *
    * 2. By definition the end can point to only one table, and a ECClassId column in that "end" table stores the ECClassId.
    *    + We use the end instance id to search the ECClassId from the end table.
    *    + Since the end instance id is a different change record than the one currently being processed, we need to first search
    *      for ECClassId in all changes, and subsequently the DbTable itself.
    *    + Note that the end table could be the relationship table itself in case of foreign key constraints (e.g., Element's ParentId)
    *    + We detect this case by checking if the ClassId constraint column is persisted, and is setup as the primary "ECClassId" column
    *      of the table that contains it. Note that the latter check differentiates case #3.
    *
    * 3. By definition the end can point to many tables, and a Source/Target ECClassId column in the relationship table stores the ECClassId
    *    + We use the relationship instance id to search the Source/Target ECClassId column in the relationship table
    *    + Even if the Source/Target ECClassId is in the same row as the currently processed change, it may not be part of the
    *      change if it wasn't modified. Therefore we need to first search for the column in the current change, and subsequently the DbTable.
    *    + Note that like the previous case, the end table could be the same as the relationship table.
    *    + We detect this case by checking if the ClassId constraint column is persisted, and is *not* the primary "ECClassId" column of the
    *      table that contains it.
    *
    */

    // Case #1: End can point to only one class
    const bool endIsInOneClass = (classIdColumn->GetPersistenceType() == PersistenceType::Virtual);
    if (endIsInOneClass)
        {
        // TODO: dynamic_cast<PropertyMapRelationshipConstraintClassId const*> (propMap)->GetDefaultConstraintECClassId()
        // should work, but doesn't for link tables - need to check with Krischan/Affan. 
        return GetRelEndClassIdFromRelClass(relationshipClassMap.GetClass().GetRelationshipClassCP(), relEnd);
        }

    // Case #2: End is in only one table (Note: not in the current table the row belongs to, but some OTHER end table)
    const bool endIsInOneTable = classIdPropMap->GetTables().size() == 1;
    if (endIsInOneTable)
        {
        Utf8StringCR endTableName = classIdColumn->GetTable().GetName();

        // Search in all changes
        ECClassId classId = m_instancesTable.QueryClassId(endTableName, relEndInstanceId);
        if (classId.IsValid())
            return classId;

        // Search in the end table
        classId = rowEntry.GetChangeIterator().GetTableMap(endTableName)->QueryValueId<ECClassId>(classIdColumn->GetName(), relEndInstanceId);
        BeAssert(classId.IsValid());
        return classId;
        }

    // Case #3: End could be in many tables
    Utf8StringCR classIdColumnName = classIdColumn->GetName();
    ECClassId classId = rowEntry.GetClassIdFromChangeOrTable(classIdColumnName.c_str(), relationshipInstanceId);
    BeAssert(classId.IsValid());
    return classId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
bool ChangeExtractor::ClassIdMatchesConstraint(ECN::ECClassId relClassId, ECN::ECRelationshipEnd end, ECN::ECClassId candidateClassId) const
    {
    bmap<ECN::ECClassId, LightweightCache::RelationshipEnd> const& constraintClassIds = m_ecdb.Schemas().GetDbMap().GetLightweightCache().GetConstraintClassesForRelationshipClass(relClassId);
    auto it = constraintClassIds.find(candidateClassId);
    if (it == constraintClassIds.end())
        return false;

    const LightweightCache::RelationshipEnd requiredEnd = end == ECRelationshipEnd::ECRelationshipEnd_Source ? LightweightCache::RelationshipEnd::Source : LightweightCache::RelationshipEnd::Target;
    const LightweightCache::RelationshipEnd actualEnd = it->second;
    return actualEnd == LightweightCache::RelationshipEnd::Both || actualEnd == requiredEnd;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
// static
ECN::ECClassId ChangeExtractor::GetRelEndClassIdFromRelClass(ECN::ECRelationshipClassCP relClass, ECN::ECRelationshipEnd relEnd)
    {
    if (relClass == nullptr)
        {
        BeAssert(false);
        return ECClassId();
        }

    ECRelationshipConstraintR endConstraint = (relEnd == ECRelationshipEnd_Source) ? relClass->GetSource() : relClass->GetTarget();
    ECRelationshipConstraintClassList const& endClasses = endConstraint.GetConstraintClasses();
    if (endClasses.size() != 1)
        {
        BeAssert(false && "Multiple classes at end. Cannot pick something arbitrary");
        return ECClassId();
        }

    ECClassId classId = endClasses[0]->GetId();
    BeAssert(classId.IsValid());
    return classId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     09/2015
//---------------------------------------------------------------------------------------
ChangeSummary::ChangeSummary(ECDbCR ecdb) : m_ecdb(ecdb)
    {
    if (s_count == 0)
        RegisterSqlFunctions(m_ecdb);
    s_count++;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::Initialize()
    {
    if (IsValid())
        return;

    m_instancesTable = new InstancesTable(*this, s_count);
    m_instancesTable->Initialize();

    m_valuesTable = new ValuesTable(*m_instancesTable);
    m_valuesTable->Initialize();

    m_changeExtractor = new ChangeExtractor(*this, *m_instancesTable, *m_valuesTable);

    m_isValid = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::Free()
    {
    if (!IsValid())
        return;

    delete m_instancesTable;
    m_instancesTable = nullptr;

    delete m_valuesTable;
    m_valuesTable = nullptr;

    delete m_changeExtractor;
    m_changeExtractor = nullptr;

    m_isValid = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::~ChangeSummary()
    {
    s_count--;
    if (s_count == 0)
        UnregisterSqlFunctions(m_ecdb);
    Free();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummary::FromChangeSet(IChangeSet& changeSet, ChangeSummary::Options const& options)
    {
    Initialize();
    return m_changeExtractor->FromChangeSet(changeSet, options.GetIncludeRelationshipInstances());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
Utf8String ChangeSummary::GetInstancesTableName() const
    {
    return m_instancesTable->GetName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeSummary::ContainsInstance(ECClassId classId, ECInstanceId instanceId) const
    {
    return m_instancesTable->ContainsInstance(classId, instanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::Instance ChangeSummary::GetInstance(ECClassId classId, ECInstanceId instanceId) const
    {
    return m_instancesTable->QueryInstance(classId, instanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
Utf8String ChangeSummary::FormatInstanceIdStr(ECInstanceId id) const
    {
    if (!id.IsValid())
        return "NULL";

    const uint64_t idVal = id.GetValue();

    uint32_t briefcaseId = (uint32_t) ((idVal << 40) & 0xffffff);
    uint64_t localId = (uint64_t) (0xffffffffffLL & idVal);

    Utf8PrintfString idStr("%" PRIu32 ":%" PRIu64, briefcaseId, localId);
    return idStr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
Utf8String ChangeSummary::FormatClassIdStr(ECClassId id) const
    {
    if (!id.IsValid())
        return "NULL";

    ECN::ECClassCP ecClass = m_ecdb.Schemas().GetClass(id);
    BeAssert(ecClass != nullptr);

    Utf8PrintfString idStr("%s:%" PRIu64, ecClass->GetFullName(), id.GetValue());
    return idStr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::Dump() const
    {
    if (!IsValid())
        {
        BeAssert(false);
        printf("Invalid ChangeSummary");
        return;
        }
        
    printf("\tBriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect\n");
    printf("\t\tAccessString;OldValue;NewValue\n");

    for (ChangeSummary::InstanceIterator::const_iterator const& iEntry : MakeInstanceIterator())
        {
        ChangeSummary::Instance instance = iEntry.GetInstance();

        Utf8String classIdStr = FormatClassIdStr(instance.GetClassId());
        Utf8String instanceIdStr = FormatInstanceIdStr(instance.GetInstanceId());

        int indirect = instance.GetIndirect();

        DbOpcode opCode = instance.GetDbOpcode();
        Utf8String opCodeStr;
        if (opCode == DbOpcode::Insert)
            opCodeStr = "Insert";
        else if (opCode == DbOpcode::Update)
            opCodeStr = "Update";
        else /* if (opCode = DbOpcode::Delete) */
            opCodeStr = "Delete";

        printf("\t%s;%s;%s;%s\n", instanceIdStr.c_str(), classIdStr.c_str(), opCodeStr.c_str(), indirect > 0 ? "Yes" : "No");

        for (ChangeSummary::ValueIterator::const_iterator const& vEntry : instance.MakeValueIterator())
            {
            Utf8String accessString = vEntry.GetAccessString();
            DbDupValue oldValue = vEntry.GetOldValue();
            DbDupValue newValue = vEntry.GetNewValue();

            Utf8String oldValueStr, newValueStr;
            if (accessString.Contains(ECDBSYS_PROP_ECInstanceId))
                {
                oldValueStr = FormatInstanceIdStr(oldValue.GetValueId<ECInstanceId>());
                newValueStr = FormatInstanceIdStr(newValue.GetValueId<ECInstanceId>());
                }
            else if (accessString.Contains(ECDBSYS_PROP_ECClassId))
                {
                oldValueStr = FormatClassIdStr(oldValue.GetValueId<ECClassId>());
                newValueStr = FormatClassIdStr(newValue.GetValueId<ECClassId>());
                }
            else
                {
                oldValueStr = oldValue.Format(0);
                newValueStr = newValue.Format(0);
                }
            
            printf("\t\t%s;%s;%s\n", accessString.c_str(), oldValueStr.c_str(), newValueStr.c_str());
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
Utf8String ChangeSummary::GetValuesTableName() const
    {
    return m_valuesTable->GetName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
// static
BentleyStatus ChangeSummary::GetMappedPrimaryTable(Utf8StringR tableName, bool& isTablePerHierarcy, ECN::ECClassCR ecClass, ECDbCR ecdb)
    {
    // TODO: This functionality needs to be moved to some publicly available ECDb mapping utility. 
    ClassMapCP classMap = ecdb.Schemas().GetDbMap().GetClassMap(ecClass);
    if (!classMap)
        return ERROR;

    DbTable& table = classMap->GetPrimaryTable();
    if (!table.IsValid())
        return ERROR;

    tableName = classMap->GetPrimaryTable().GetName();
    isTablePerHierarcy = classMap->GetMapStrategy().IsTablePerHierarchy();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::Instance& ChangeSummary::Instance::operator=(InstanceCR other)
    {
    m_classId = other.m_classId;
    m_instanceId = other.m_instanceId;
    m_dbOpcode = other.m_dbOpcode;
    m_indirect = other.m_indirect;
    m_changeSummary = other.m_changeSummary;
    m_tableName = other.m_tableName;
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::Instance::SetupValuesTableSelectStatement(Utf8CP accessString) const
    {
    if (!m_valuesTableSelect.IsValid())
        {
        Utf8String tableName = m_changeSummary->GetValuesTableName();
        Utf8PrintfString sql("SELECT OldValue, NewValue FROM %s WHERE ClassId=? AND InstanceId=? AND AccessString=?", tableName.c_str());
        m_valuesTableSelect = m_changeSummary->GetDb().GetCachedStatement(sql.c_str());
        BeAssert(m_valuesTableSelect.IsValid());

        m_valuesTableSelect->BindId(1, m_classId);
        m_valuesTableSelect->BindId(2, m_instanceId);
        }

    m_valuesTableSelect->Reset();
    m_valuesTableSelect->BindText(3, accessString, Statement::MakeCopy::No);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     08/2015
//---------------------------------------------------------------------------------------
DbDupValue ChangeSummary::Instance::GetOldValue(Utf8CP accessString) const
    {
    BeAssert(IsValid());

    if (IsValid())
       {
       SetupValuesTableSelectStatement(accessString);
       DbResult result = m_valuesTableSelect->Step();
       if (result == BE_SQLITE_ROW)
           return m_valuesTableSelect->GetDbValue(0);
       BeAssert(result == BE_SQLITE_DONE);
       }
    
    DbDupValue invalidValue(nullptr);
    return invalidValue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     08/2015
//---------------------------------------------------------------------------------------
DbDupValue ChangeSummary::Instance::GetNewValue(Utf8CP accessString) const
    {
    BeAssert(IsValid());

    if (IsValid())
        {
        SetupValuesTableSelectStatement(accessString);
        DbResult result = m_valuesTableSelect->Step();
        if (result == BE_SQLITE_ROW)
            return m_valuesTableSelect->GetDbValue(1);
        BeAssert(result == BE_SQLITE_DONE);
        }

    DbDupValue invalidValue(nullptr);
    return invalidValue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     08/2015
//---------------------------------------------------------------------------------------
bool ChangeSummary::Instance::ContainsValue(Utf8CP accessString) const
    {
    if (!IsValid())
        {
        BeAssert(false);
        return false;
        }

    SetupValuesTableSelectStatement(accessString);

    DbResult result = m_valuesTableSelect->Step();
    BeAssert(result == BE_SQLITE_DONE || result == BE_SQLITE_ROW);

    return (result == BE_SQLITE_ROW);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::Instance ChangeSummary::InstanceIterator::Entry::GetInstance() const
    {
    ECClassId classId = m_sql->GetValueId<ECClassId>(0);
    ECInstanceId instanceId = m_sql->GetValueId<ECInstanceId>(1);
    DbOpcode dbOpcode = (DbOpcode) m_sql->GetValueInt(2);
    int indirect = m_sql->GetValueInt(3);
    Utf8String tableName = m_sql->GetValueText(4);

    ChangeSummary::Instance instance(m_changeSummary, classId, instanceId, dbOpcode, indirect, tableName);
    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ChangeSummary::InstanceIterator::MakeSelectStatement(Utf8CP columns) const
    {
    return m_options.ToSelectStatement(columns, m_changeSummary);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ChangeSummary::InstanceIterator::Options::ToSelectStatement(Utf8CP columns, ChangeSummary const& summary) const
    {
    if (IsEmpty())
        {
        Utf8String sql("SELECT ");
        sql.append(columns);
        sql.append(" FROM ");
        sql.append(summary.GetInstancesTableName());

        return sql;
        }

    Utf8String sql(
            " WITH RECURSIVE"
            "    DerivedClasses(ClassId) AS ("
            "        VALUES(:baseClassId)"
            "        UNION "
            "        SELECT ec_ClassHasBaseClasses.ClassId FROM ec_ClassHasBaseClasses, DerivedClasses WHERE ec_ClassHasBaseClasses.BaseClassId=DerivedClasses.ClassId"
            "        )"
            " SELECT ClassId,InstanceId,DbOpcode,Indirect,TableName"
            " FROM ");
    sql.append(summary.GetInstancesTableName());
    sql.append(" WHERE ClassId IN DerivedClasses");
    if (QueryDbOpcode::All != m_opcodes)
        {
        sql.append(" AND ");
        sql.append(summary.ConstructWhereInClause(m_opcodes));
        }

    return sql;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeSummary::InstanceIterator::Options::Bind(Statement& stmt) const
    {
    if (!IsEmpty())
        stmt.BindId(stmt.GetParameterIndex(":baseClassId"), m_classId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::InstanceIterator::const_iterator ChangeSummary::InstanceIterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString(MakeSelectStatement("ClassId,InstanceId,DbOpcode,Indirect,TableName").c_str());
        m_db->GetCachedStatement(m_stmt, sqlString.c_str());
        m_params.Bind(*m_stmt);
        m_options.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_changeSummary, m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::InstanceIterator::const_iterator ChangeSummary::InstanceIterator::end() const
    { 
    return Entry(m_changeSummary, m_stmt.get(), false); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
int ChangeSummary::InstanceIterator::QueryCount() const
    {
    Utf8String sqlString = MakeSqlString(MakeSelectStatement("count(*)").c_str());

    CachedStatementPtr stmt;
    m_db->GetCachedStatement(stmt, sqlString.c_str());
    BeAssert(stmt.IsValid());

    m_options.Bind(*stmt);

    return ((BE_SQLITE_ROW != stmt->Step()) ? 0 : stmt->GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::ValueIterator::const_iterator ChangeSummary::ValueIterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String tableName = m_changeSummary.GetValuesTableName();
        Utf8PrintfString sql("SELECT AccessString,OldValue,NewValue FROM %s WHERE ClassId=? AND InstanceId=?", tableName.c_str());
        Utf8String sqlString = MakeSqlString(sql.c_str(), true);
        DbResult result = m_db->GetCachedStatement(m_stmt, sqlString.c_str());
        BeAssert(BE_SQLITE_OK == result);

        m_stmt->BindId(1, m_classId);
        m_stmt->BindId(2, m_instanceId);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
int ChangeSummary::ValueIterator::QueryCount() const
    {
    Utf8String tableName = m_changeSummary.GetValuesTableName();
    Utf8PrintfString sql("SELECT COUNT(*) FROM %s WHERE ClassId=? AND InstanceId=?", tableName.c_str());
    Utf8String sqlString = MakeSqlString(sql.c_str());

    CachedStatementPtr stmt;
    m_db->GetCachedStatement(stmt, sqlString.c_str());
    BeAssert(stmt.IsValid());

    stmt->BindId(1, m_classId);
    stmt->BindId(2, m_instanceId);

    return ((BE_SQLITE_ROW != stmt->Step()) ? 0 : stmt->GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     08/2015
//---------------------------------------------------------------------------------------
Utf8String ChangeSummary::ConstructWhereInClause(QueryDbOpcode queryDbOpcodes) const
    {
    Utf8String whereInStr;
    if (QueryDbOpcode::None != (queryDbOpcodes & QueryDbOpcode::Insert))
        {
        Utf8PrintfString addStr("%d", Enum::ToInt(DbOpcode::Insert));
        whereInStr.append(addStr);
        }
    if (QueryDbOpcode::None != (queryDbOpcodes & QueryDbOpcode::Update))
        {
        if (!whereInStr.empty())
            whereInStr.append(",");

        Utf8PrintfString addStr("%d", Enum::ToInt(DbOpcode::Update));
        whereInStr.append(addStr);
        }
    if (QueryDbOpcode::None != (queryDbOpcodes & QueryDbOpcode::Delete))
        {
        if (!whereInStr.empty())
            whereInStr.append(",");
        Utf8PrintfString addStr("%d", Enum::ToInt(DbOpcode::Delete));
        whereInStr.append(addStr);
        }

    BeAssert(!whereInStr.empty());

    return Utf8PrintfString("DbOpcode IN (%s)", whereInStr.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     08/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::QueryByClass(bmap<ECInstanceId, ChangeSummary::Instance>& changes, ECN::ECClassId classId, bool isPolymorphic /*= true*/, QueryDbOpcode queryDbOpcodes /*= QueryDbOpcode::All*/) const
    {
    if (!IsValid())
        {
        BeAssert(false);
        return;
        }

    InstanceIterator::Options options(classId, isPolymorphic, queryDbOpcodes);
    for (auto& entry : MakeInstanceIterator(options))
        {
        changes[entry.GetInstanceId()] = entry.GetInstance();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
// static
void ChangeSummary::RegisterSqlFunctions(ECDbCR ecdb)
    {
    if (s_isChangedInstanceSqlFunction)
        return;

    s_isChangedInstanceSqlFunction = new IsChangedInstanceSqlFunction();
    int status = ecdb.AddFunction(*s_isChangedInstanceSqlFunction);
    BeAssert(status == 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
// static
void ChangeSummary::UnregisterSqlFunctions(ECDbCR ecdb)
    {
    if (!s_isChangedInstanceSqlFunction)
        return;

    ecdb.RemoveFunction(*s_isChangedInstanceSqlFunction);
    delete s_isChangedInstanceSqlFunction;
    s_isChangedInstanceSqlFunction = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     08/2015
// @e Example
//     SELECT el.ECInstanceId
//     JOIN dgn.ElementGeom elg USING dgn.ElementOwnsGeom
//     WHERE IsChangedInstance(elg.GetClassId(), elg.ECInstanceId)
//---------------------------------------------------------------------------------------
void IsChangedInstanceSqlFunction::_ComputeScalar(ScalarFunction::Context& ctx, int nArgs, DbValue* args)
    {
    if (nArgs != 3 || args[0].IsNull() || args[1].IsNull() || args[2].IsNull())
        {
        ctx.SetResultError("Arguments to IsChangedInstance must be (changeSummary, " ECDBSYS_PROP_ECClassId ", " ECDBSYS_PROP_ECInstanceId ")", -1);
        return;
        }

    ChangeSummary const* changeSummary = (ChangeSummary const*) args[0].GetValueInt64();
    ECClassId classId = (ECClassId) args[1].GetValueUInt64();
    ECInstanceId instanceId = args[2].GetValueId<ECInstanceId>();

    /*
     * TODO: Instead of returning a bool we can return a change id (need to set one up)
     * Alternately, we can setup ECSQL mappings with the change table and entirely
     * avoid some of these custom functions. This needs more investigation if and when 
     * the use cases demand it. 
     */
    int res = changeSummary->ContainsInstance(classId, instanceId) ? 1 : 0;
    ctx.SetResultInt(res);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ChangeIterator(ECDbCR ecdb, IChangeSet& changeSet) : m_ecdb(ecdb), m_changes(changeSet.GetChanges())
    {
    InitTableMap();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ChangeIterator(ECDbCR ecdb, Changes const& changes) : m_ecdb(ecdb), m_changes(changes)
    {
    InitTableMap();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::~ChangeIterator()
    {
    FreeTableMap();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void ChangeIterator::InitTableMap()
    {
    BeAssert(m_tableMapByName == nullptr);
    m_tableMapByName = new TableMapByName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
TableMap const* ChangeIterator::GetTableMap(Utf8StringCR tableName) const
    {
    if (m_tableMapByName->find(tableName.c_str()) == m_tableMapByName->end())
        {
        AddTableToMap(tableName);
        BeAssert(m_tableMapByName->find(tableName.c_str()) != m_tableMapByName->end());
        }

    return (*m_tableMapByName)[tableName].get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void ChangeIterator::AddTableToMap(Utf8StringCR tableName) const
    {
    TableMapPtr tableMap = TableMap::Create(m_ecdb, tableName);
    BeAssert(tableMap.IsValid());

    (*m_tableMapByName)[tableName] = tableMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void ChangeIterator::FreeTableMap()
    {
    if (m_tableMapByName)
        {
        delete m_tableMapByName;
        m_tableMapByName = nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::RowEntry ChangeIterator::begin() const
    {
    return ChangeIterator::RowEntry(*this, m_changes.begin());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::RowEntry ChangeIterator::end() const
    {
    return ChangeIterator::RowEntry(*this, m_changes.end());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::RowEntry::RowEntry(ChangeIterator const& iterator, Changes::Change const& change) : m_ecdb(iterator.GetDb()), m_iterator(iterator), m_change(change)
    {
    Initialize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::RowEntry::~RowEntry()
    {
    FreeSqlChange();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void ChangeIterator::RowEntry::Reset()
    {
    m_sqlChange = nullptr;
    m_tableMap = nullptr;
    m_primaryInstanceId = ECInstanceId();
    m_primaryClass = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void ChangeIterator::RowEntry::Initialize()
    {
    Reset();
    if (!m_change.IsValid())
        return;

    InitSqlChange();

    m_tableMap = m_iterator.GetTableMap(m_sqlChange->GetTableName());
    BeAssert(m_tableMap != nullptr);

    if (m_tableMap->IsMapped())
        InitPrimaryInstance();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
bool ChangeIterator::RowEntry::IsMapped() const
    {
    return m_primaryClass != nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void ChangeIterator::RowEntry::InitPrimaryInstance()
    {
    BeAssert(m_tableMap->IsMapped());

    m_primaryInstanceId = m_sqlChange->GetValueId<ECInstanceId>(m_tableMap->GetIdColumn().GetIndex());
    BeAssert(m_primaryInstanceId.IsValid());

    if (m_sqlChange->GetDbOpcode() == DbOpcode::Update && !m_tableMap->QueryInstance(m_primaryInstanceId))
        {
        // Note: The instance doesn't exist anymore, and has been deleted in future change to the Db.
        // Processing updates requires that the instance is still available in the Db to extract sufficient EC information, 
        // especially since a SqlChangeSet records only the updated columns but not the entire row. 
        BeAssert(false && "SqlChangeSet does not span all modifications made to the Db");
        return;
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
    const DbOpcode dbOpcode = m_sqlChange->GetDbOpcode();
    if (dbOpcode == DbOpcode::Insert || dbOpcode == DbOpcode::Delete)
        return m_sqlChange->GetValueId<ECClassId>(m_tableMap->GetColumnIndexByName(classIdColumnName));

    /* if (dbOpcode == DbOpcode::Update) */
    return m_tableMap->QueryValueId<ECClassId>(classIdColumnName, instanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void ChangeIterator::RowEntry::InitSqlChange()
    {
    FreeSqlChange();
    m_sqlChange = new SqlChange(m_change);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
void ChangeIterator::RowEntry::FreeSqlChange()
    {
    if (!m_sqlChange)
        return;
    delete m_sqlChange;
    m_sqlChange = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
Utf8StringCR ChangeIterator::RowEntry::GetTableName() const
    {
    if (m_tableMap == nullptr)
        {
        BeAssert(false);
        return s_emptyString;
        }

    return m_tableMap->GetTableName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
bool ChangeIterator::RowEntry::IsPrimaryTable() const
    {
    if (m_tableMap == nullptr)
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
ChangeIterator::ColumnIterator ChangeIterator::RowEntry::MakeColumnIterator(ECClassCR ecClass) const
    {
    return ChangeIterator::ColumnIterator(*this, &ecClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ColumnIterator ChangeIterator::RowEntry::MakePrimaryColumnIterator() const
    {
    return ChangeIterator::ColumnIterator(*this, GetPrimaryClass());
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

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ColumnEntry::ColumnEntry(ColumnIterator const& columnIterator, bmap<Utf8String, ColumnMap*>::const_iterator columnMapIterator)
    : m_columnIterator(columnIterator), m_ecdb(columnIterator.GetDb()), m_sqlChange(columnIterator.GetSqlChange()), m_columnMapIterator(columnMapIterator), m_isValid(true)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ColumnEntry::ColumnEntry(ColumnIterator const& columnIterator) : m_columnIterator(columnIterator), m_ecdb(columnIterator.GetDb()), m_isValid(false)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
Utf8StringCR ChangeIterator::ColumnEntry::GetPropertyAccessString() const
    {
    if (!m_isValid)
        {
        BeAssert(false);
        return s_emptyString;
        }

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

    RowEntry const& rowEntry = m_columnIterator.GetRowEntry();
    return rowEntry.GetTableMap()->QueryValueFromDb(columnMap->GetName(), rowEntry.GetPrimaryInstanceId());
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

END_BENTLEY_SQLITE_EC_NAMESPACE
