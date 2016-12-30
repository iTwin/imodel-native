/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ChangeSummaryImpl.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

    DbTable const* dbTable = dbSchema.FindTable(tableName.c_str());
    if (!dbTable || !dbTable->IsValid() || dbTable->IsNullTable())
        {
        m_isMapped = false;
        return;
        }

    m_isMapped = true;
    m_dbTable = dbTable;
    m_tableName = tableName;

    InitColumnIndexByName();
    InitSystemColumnMaps();
    InitForeignKeyRelClassMaps();
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
    DbColumn const* instanceIdColumn = m_dbTable->GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
    Utf8StringCR instanceIdColumnName = instanceIdColumn->GetName();
    int instanceIdColumnIndex = GetColumnIndexByName(instanceIdColumnName);
    BeAssert(instanceIdColumnIndex >= 0);
    m_instanceIdColumnMap = ColumnMap(instanceIdColumnName, instanceIdColumnIndex, false, "");

    DbColumn const* classIdColumn = m_dbTable->GetFilteredColumnFirst(DbColumn::Kind::ECClassId);
    if (classIdColumn->GetPersistenceType() != PersistenceType::Virtual)
        {
        Utf8StringCR classIdColumnName = classIdColumn->GetName();
        int classIdColumnIndex = GetColumnIndexByName(classIdColumnName);
        m_classIdColumnMap = ColumnMap(classIdColumnName, classIdColumnIndex, false, "");
        }
    else
        {
        m_primaryClassId = QueryClassId();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void TableMap::InitForeignKeyRelClassMaps()
    {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(
        "SELECT DISTINCT ec_Class.Id FROM ec_Class "
        "JOIN ec_ClassMap ON ec_Class.Id = ec_ClassMap.ClassId "
        "JOIN ec_PropertyMap ON ec_ClassMap.ClassId = ec_PropertyMap.ClassId "
        "JOIN ec_Column ON ec_PropertyMap.ColumnId = ec_Column.Id "
        "JOIN ec_Table ON ec_Table.Id = ec_Column.TableId "
        "WHERE ec_Table.Name = :tableName AND ec_Class.Type=" SQLVAL_ECClassType_Relationship " AND"
        "     (ec_ClassMap.MapStrategy = " SQLVAL_MapStrategy_ForeignKeyRelationshipInSourceTable " OR ec_ClassMap.MapStrategy = " SQLVAL_MapStrategy_ForeignKeyRelationshipInTargetTable ") AND"
        "     ec_Column.IsVirtual = " SQLVAL_False " AND"
        "     (ec_Column.ColumnKind & " SQLVAL_DbColumn_Kind_ECInstanceId "=" SQLVAL_DbColumn_Kind_ECInstanceId ")");
    BeAssert(stmt.IsValid());

    stmt->BindText(stmt->GetParameterIndex(":tableName"), m_tableName, Statement::MakeCopy::No);

    DbResult result;
    while ((result = stmt->Step()) == BE_SQLITE_ROW)
        {
        m_fkeyRelClassIds.push_back(stmt->GetValueId<ECClassId>(0));
        }
    BeAssert(result == BE_SQLITE_DONE);
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

    stmt->BindText(stmt->GetParameterIndex(":tableName"), m_tableName.c_str(), Statement::MakeCopy::No);

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
ECN::ECClassId TableMap::GetECClassId() const
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
    Utf8PrintfString ecSql("SELECT %s FROM %s WHERE %s=?", physicalColumnName.c_str(), m_tableName.c_str(), GetECInstanceIdColumn().GetPhysicalName().c_str());
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
    DbDupValue value = QueryValueFromDb(GetECInstanceIdColumn().GetPhysicalName(), instanceId);
    if (!value.IsValid() || value.IsNull())
        return false;

    return value.GetValueId<ECInstanceId>().IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
TableClassMapCP TableMap::GetTableClassMap(ECClassCR ecClass) const
    {
    auto iter = m_tableClassMapsById.find(ecClass.GetId());
    if (iter != m_tableClassMapsById.end())
        return iter->second.get();

    TableClassMapPtr classMap = TableClassMap::Create(m_ecdb, *this, ecClass);
    m_tableClassMapsById[ecClass.GetId()] = classMap;

    return classMap.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
TableClassMap::TableClassMap(ECDbCR ecdb, TableMapCR tableMap, ECN::ECClassCR ecClass) : m_ecdb(ecdb), m_tableMap(tableMap), m_class(ecClass)
    {
    Initialize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
// static 
TableClassMapPtr TableClassMap::Create(ECDbCR ecdb, TableMapCR tableMap, ECN::ECClassCR ecClass)
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

    InitPropertyColumnMaps();
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
        SingleColumnDataPropertyMap const* singleColumnMap = static_cast<SingleColumnDataPropertyMap const*>(propertyMap);

        if (singleColumnMap->GetTable().GetId() != m_tableMap.GetDbTable()->GetId())
            continue; // Skip properties that don't belong to the current table. 

        AddColumnMapsForProperty(*singleColumnMap);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
void TableClassMap::AddColumnMapsForProperty(SingleColumnDataPropertyMap const& singleColumnMap)
    {
    DbColumn const& column = singleColumnMap.GetColumn();
    bool isOverflowColumn = column.IsOverflowSlave();

    if (column.GetPersistenceType() == PersistenceType::Virtual && !isOverflowColumn)
        return; // TODO: This is to filter virtual Navigation property's RelECClassId column - needs a better check from Affan. 

    Utf8String overflowColumnName = isOverflowColumn ? column.GetName() : "";
    Utf8StringCR physicalColumnName = isOverflowColumn ? column.GetPhysicalOverflowColumn()->GetName() : column.GetName();
    int physicalColumnIndex = m_tableMap.GetColumnIndexByName(physicalColumnName);

    m_columnMapByAccessString[singleColumnMap.GetAccessString()] = ColumnMap(physicalColumnName, physicalColumnIndex, isOverflowColumn, overflowColumnName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
bool TableClassMap::ContainsColumn(Utf8CP propertyAccessString) const
    {
    bmap<Utf8String, ColumnMap>::const_iterator iter = m_columnMapByAccessString.find(propertyAccessString);
    return iter != m_columnMapByAccessString.end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
ColumnMap const& TableClassMap::GetColumn(Utf8CP propertyAccessString) const
    {
    bmap<Utf8String, ColumnMap>::const_iterator iter = m_columnMapByAccessString.find(propertyAccessString);
    BeAssert(iter != m_columnMapByAccessString.end());
    return iter->second;
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
    m_instancesTableInsert.BindText(5, tableName.c_str(), Statement::MakeCopy::No);

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

    stmt->BindText(1, tableName.c_str(), Statement::MakeCopy::No);
    stmt->BindId(2, instanceId);

    DbResult result = stmt->Step();
    if (result != BE_SQLITE_ROW)
        return ECClassId();

    return stmt->GetValueId<ECClassId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
ValuesTable::ValuesTable(InstancesTableCR instancesTable) : m_instancesTable(instancesTable), m_changeSummary(instancesTable.GetChangeSummary()), m_ecdb(instancesTable.GetDb())
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
ChangeExtractor::ChangeExtractor(ChangeSummaryCR changeSummary, InstancesTableR instancesTable, ValuesTableR valuesTable) 
    : m_changeSummary(changeSummary), m_ecdb(m_changeSummary.GetDb()), m_instancesTable(instancesTable), m_valuesTable(valuesTable) 
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
ChangeExtractor::~ChangeExtractor()
    { 
    FreeTableMap(); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
// static
ClassMapCP ChangeExtractor::GetClassMap(ECDbCR ecdb, ECClassId classId)
    {
    ECN::ECClassCP ecClass = ecdb.Schemas().GetECClass(classId);
    if (ecClass == nullptr)
        {
        BeAssert(false && "Couldn't determine the class corresponding to the change.");
        return nullptr;
        }

    ClassMapCP classMap = ecdb.Schemas().GetDbMap().GetClassMap(*ecClass);
    return classMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
TableMapCP ChangeExtractor::GetTableMap(Utf8StringCR tableName) const
    {
    if (m_tableMapByName.find(tableName.c_str()) == m_tableMapByName.end())
        {
        AddTableToMap(tableName);
        BeAssert(m_tableMapByName.find(tableName.c_str()) != m_tableMapByName.end());
        }

    return m_tableMapByName[tableName.c_str()].get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::AddTableToMap(Utf8StringCR tableName) const
    {
    TableMapPtr tableMap = TableMap::Create(m_ecdb, tableName);
    BeAssert(tableMap.IsValid());

    m_tableMapByName[tableName.c_str()] = tableMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::FreeTableMap()
    {
    m_tableMapByName.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ECClassId ChangeExtractor::GetClassIdFromChangeOrTable(Utf8CP classIdColumnName, ECInstanceId instanceId) const
    {
    const DbOpcode dbOpcode = m_sqlChange->GetDbOpcode();
    if (dbOpcode == DbOpcode::Insert || dbOpcode == DbOpcode::Delete)
        return m_sqlChange->GetValueId<ECClassId>(m_tableMap->GetColumnIndexByName(classIdColumnName));

    /* if (dbOpcode == DbOpcode::Update) */
    return m_tableMap->QueryValueId<ECClassId>(classIdColumnName, instanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
bool ChangeExtractor::RecordInstance(TableClassMapCR tableClassMap, ECInstanceId instanceId, DbOpcode dbOpcode)
    {
    Utf8StringCR tableName = m_tableMap->GetTableName();
    ECClassId classId = tableClassMap.GetClass().GetId();

    bool instanceExisted = m_instancesTable.ContainsInstance(classId, instanceId);

    ChangeSummary::Instance instance(m_changeSummary, classId, instanceId, dbOpcode, m_sqlChange->GetIndirect(), tableName);
    m_instancesTable.InsertOrUpdate(instance);

    bool updatedProperties = false;
    auto const& columnMaps = tableClassMap.GetColumnMapByAccessString();
    for (auto it = columnMaps.begin(); it != columnMaps.end(); it++)
        {
        Utf8StringCR accessString = it->first;
        ColumnMap const& columnMap = it->second;

        if (m_sqlChange->IsPrimaryKeyColumn(columnMap.GetPhysicalIndex()))
            continue; // Primary key columns are always included in an update

        if (RecordColumValue(instance, columnMap, accessString)) // Returns true if there was an update
            updatedProperties = true;
        }

    if (dbOpcode == DbOpcode::Update && !instanceExisted && !updatedProperties)
        {
        // TODO: Clean up and simplify this logic for end table cases. 
        MapStrategy mapStrategy = tableClassMap.GetClassMap()->GetMapStrategy().GetStrategy();
        bool isEndTableRel = (mapStrategy == MapStrategy::ForeignKeyRelationshipInSourceTable ||
                              mapStrategy == MapStrategy::ForeignKeyRelationshipInTargetTable);

        // If recording an update for the first time, and none of the properties have really been updated, remove record of the updated instance
        if (!isEndTableRel)
            {
            m_instancesTable.Delete(classId, instanceId);
            return false;
            }
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeExtractor::RecordColumValue(ChangeSummary::InstanceCR instance, ColumnMap const& columnMap, Utf8StringCR accessString)
    {
    ECN::ECClassId classId = instance.GetClassId();
    ECInstanceId instanceId = instance.GetInstanceId();
    
    DbValue oldValue(nullptr), newValue(nullptr);
    m_sqlChange->GetValues(oldValue, newValue, columnMap.GetPhysicalIndex());

    DbDupValue oldDupValue(nullptr), newDupValue(nullptr); // declared early 'cos these need to be held in memory
    if (columnMap.IsOverflow())
        {
        oldDupValue = ExtractOverflowValue(oldValue, columnMap);
        newDupValue = ExtractOverflowValue(newValue, columnMap);

        oldValue = DbValue(oldDupValue.GetSqlValueP());
        newValue = DbValue(newDupValue.GetSqlValueP());
        }
    
    bool hasOldValue = oldValue.IsValid() && !oldValue.IsNull();
    bool hasNewValue = newValue.IsValid() && !newValue.IsNull();
    
    // TOOD: Need to re-examine this logic - why do we need this? Also, we do hvae
    // to check the overflow case. 
    DbOpcode dbOpcode = instance.GetDbOpcode();
    if (dbOpcode != m_sqlChange->GetDbOpcode())
        {
        /*
        * Note: In the case of FKEY relationships, an insert or delete can be caused by an update
        * to the table. In these cases, all the old or new values necessary may not be part of
        * change record since update records only changed values.
        * We make an attempt to retrieve these values from the current state of the Db.
        */
        BeAssert(!columnMap.IsOverflow());
        BeAssert(dbOpcode == DbOpcode::Insert || dbOpcode == DbOpcode::Delete);
        BeAssert(m_sqlChange->GetDbOpcode() == DbOpcode::Update);

        if (dbOpcode == DbOpcode::Insert && !hasNewValue)
            {
            newValue = m_tableMap->QueryValueFromDb(columnMap.GetPhysicalName(), instanceId);
            hasNewValue = newValue.IsValid() && !newValue.IsNull();
            }
        else if (dbOpcode == DbOpcode::Delete && !hasOldValue)
            {
            oldValue = m_tableMap->QueryValueFromDb(columnMap.GetPhysicalName(), instanceId);
            hasOldValue = oldValue.IsValid() && !oldValue.IsNull();
            }
        }

    if (!hasOldValue && !hasNewValue) // Do not persist entirely empty fields
        return false;

    m_valuesTable.Insert(classId, instanceId, accessString.c_str(), oldValue, newValue);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
DbDupValue ChangeExtractor::ExtractOverflowValue(DbValue const& columnValue, ColumnMap const& columnMap)
    {
    if (!columnValue.IsValid() || columnValue.IsNull())
        return DbDupValue(nullptr);

    // TODO: Avoid using the common shared cache
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT json_extract(?,?)");
    BeAssert(stmt.IsValid());

    Utf8PrintfString extractPropExpr("$.%s", columnMap.GetOverflowName().c_str());

    stmt->BindDbValue(1, columnValue);
    stmt->BindText(2, extractPropExpr.c_str(), Statement::MakeCopy::No);

    DbResult result = stmt->Step();
    BeAssert(result == BE_SQLITE_ROW);

    return stmt->GetDbValue(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
int ChangeExtractor::GetFirstColumnIndex(PropertyMap const* propertyMap) const
    {
    if (propertyMap == nullptr)
        return -1;

    GetColumnsPropertyMapVisitor columnsDisp(PropertyMap::Type::All, /* doNotSkipSystemPropertyMaps */ true);
    propertyMap->AcceptVisitor(columnsDisp);
    if (columnsDisp.GetColumns().size() != 1)
        return -1;

    return m_tableMap->GetColumnIndexByName(columnsDisp.GetColumns()[0]->GetName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeExtractor::ExtractFromSqlChanges(Changes& changes, bool includeRelationshipInstances)
    {
    // Pass 1
    BentleyStatus status = ExtractFromSqlChanges(changes, ExtractOption::InstancesOnly);
    if (SUCCESS != status || !includeRelationshipInstances)
        return status;

    // Pass 2
    return ExtractFromSqlChanges(changes, ExtractOption::RelationshipInstancesOnly);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeExtractor::ExtractFromSqlChanges(Changes& changes, ExtractOption extractOption)
    {
    for (Changes::Change const& change : changes)
        {
        SqlChange sqlChange(change);
        BentleyStatus status = ExtractFromSqlChange(sqlChange, extractOption);
        if (SUCCESS != status)
            return status;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeExtractor::ExtractFromSqlChange(SqlChange const& sqlChange, ExtractOption extractOption)
    {
    m_extractOption = extractOption;
    m_sqlChange = &sqlChange;

    Utf8StringCR tableName = m_sqlChange->GetTableName();
    if (tableName.StartsWith("ec_"))
        {
        sqlChange.GetChange().Dump(m_ecdb, false, 1);
        BeAssert(false && "ChangeSet includes changes to the ECSchema. Change summary cannot be created.");
        return ERROR;
        }

    m_tableMap = GetTableMap(tableName);
    BeAssert(m_tableMap != nullptr);
    if (!m_tableMap->IsMapped())
        {
        LOG.infov("ChangeSummary skipping table %s since it's not mapped", m_tableMap->GetTableName().c_str());
        return SUCCESS;
        }

    ECInstanceId primaryInstanceId = m_sqlChange->GetValueId<ECInstanceId>(m_tableMap->GetECInstanceIdColumn().GetPhysicalIndex());
    BeAssert(primaryInstanceId.IsValid());

    if (m_sqlChange->GetDbOpcode() == DbOpcode::Update && !m_tableMap->QueryInstance(primaryInstanceId))
        {
        // Note: The instance doesn't exist anymore, and has been deleted in future change to the Db.
        // Processing updates requires that the instance is still available in the Db to extract sufficient EC information, 
        // especially since a SqlChangeSet records only the updated columns but not the entire row. 
        BeAssert(false && "SqlChangeSet does not span all modifications made to the Db");
        return ERROR;
        }

    ECClassId primaryClassId;
    if (m_tableMap->ContainsECClassIdColumn())
        primaryClassId = GetClassIdFromChangeOrTable(m_tableMap->GetECClassIdColumn().GetPhysicalName().c_str(), primaryInstanceId);
    else
        primaryClassId = m_tableMap->GetECClassId();

    ECN::ECClassCP primaryClass = m_ecdb.Schemas().GetECClass(primaryClassId);
    if (primaryClass == nullptr)
        {
        BeAssert(false && "Couldn't determine the class corresponding to the change.");
        return ERROR;
        }

    TableClassMapCP tableClassMap = m_tableMap->GetTableClassMap(*primaryClass);
    if (!tableClassMap->IsMapped())
        {
        BeAssert(false);
        return ERROR;
        }

    if (m_extractOption == ExtractOption::InstancesOnly && !tableClassMap->GetClassMap()->IsRelationshipClassMap())
        {
        ExtractInstance(*tableClassMap, primaryInstanceId);
        return SUCCESS;
        }
        
    if (m_extractOption == ExtractOption::RelationshipInstancesOnly)
        {
        ExtractRelInstance(*tableClassMap, primaryInstanceId);
        return SUCCESS;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::ExtractInstance(TableClassMapCR tableClassMap, ECInstanceId instanceId)
    {
    RecordInstance(tableClassMap, instanceId, m_sqlChange->GetDbOpcode());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
bool ChangeExtractor::RecordRelInstance(TableClassMapCR tableClassMap, ECInstanceId instanceId, DbOpcode dbOpcode, ECInstanceKeyCR oldSourceKey, ECInstanceKeyCR newSourceKey, ECInstanceKeyCR oldTargetKey, ECInstanceKeyCR newTargetKey)
    {
    if (!RecordInstance(tableClassMap, instanceId, dbOpcode))
        return false;

    ECClassId classId = tableClassMap.GetClass().GetId();

    m_valuesTable.Insert(classId, instanceId, "SourceECClassId", oldSourceKey.IsValid() ? oldSourceKey.GetECClassId() : ECClassId(), newSourceKey.IsValid() ? newSourceKey.GetECClassId() : ECClassId());
    m_valuesTable.Insert(classId, instanceId, "SourceECInstanceId", oldSourceKey.IsValid() ? oldSourceKey.GetECInstanceId() : ECInstanceId(), newSourceKey.IsValid() ? newSourceKey.GetECInstanceId() : ECInstanceId());
    m_valuesTable.Insert(classId, instanceId, "TargetECClassId", oldTargetKey.IsValid() ? oldTargetKey.GetECClassId() : ECClassId(), newTargetKey.IsValid() ? newTargetKey.GetECClassId() : ECClassId());
    m_valuesTable.Insert(classId, instanceId, "TargetECInstanceId", oldTargetKey.IsValid() ? oldTargetKey.GetECInstanceId() : ECInstanceId(), newTargetKey.IsValid() ? newTargetKey.GetECInstanceId() : ECInstanceId());

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::ExtractRelInstance(TableClassMapCR tableClassMap, ECInstanceId relInstanceId)
    {
    ClassMap::Type type = tableClassMap.GetClassMap()->GetType();

    if (type == ClassMap::Type::RelationshipLinkTable)
        {
        ExtractRelInstanceInLinkTable(tableClassMap, relInstanceId);
        return;
        }

    bvector<ECClassId> const& relClassIds = m_tableMap->GetMappedForeignKeyRelationshipClasses();
    for (ECClassId relClassId : relClassIds)
        {
        ECN::ECClassCP relClass = m_ecdb.Schemas().GetECClass(relClassId);
        BeAssert(relClass != nullptr);

        TableClassMapCP tableRelClassMap = m_tableMap->GetTableClassMap(*relClass);
        if (!tableRelClassMap->IsMapped())
            {
            BeAssert(false); // Can this really happen??
            continue;
            }

        ExtractRelInstanceInEndTable(*tableRelClassMap, relInstanceId, tableClassMap.GetClass().GetId());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::ExtractRelInstanceInEndTable(TableClassMapCR tableRelClassMap, ECInstanceId relInstanceId, ECN::ECClassId foreignEndClassId)
    {
    BeAssert(nullptr != dynamic_cast<RelationshipClassEndTableMap const*> (tableRelClassMap.GetClassMap()));
    RelationshipClassEndTableMap const* relClassMap = static_cast<RelationshipClassEndTableMap const*>(tableRelClassMap.GetClassMap());

    // Check that this end of the relationship matches the actual class found.
    ECClassId relClassId = relClassMap->GetClass().GetId();
    ECN::ECRelationshipEnd foreignEnd = relClassMap->GetForeignEnd();
    if (!ClassIdMatchesConstraint(relClassId, foreignEnd, foreignEndClassId))
        return;

    ECInstanceKey foreignEndInstanceKey(foreignEndClassId, relInstanceId);

    ECInstanceKey oldReferencedEndInstanceKey, newReferencedEndInstanceKey;
    ECN::ECRelationshipEnd referencedEnd = (foreignEnd == ECRelationshipEnd_Source) ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
    GetRelEndInstanceKeys(oldReferencedEndInstanceKey, newReferencedEndInstanceKey, *relClassMap, relInstanceId, referencedEnd);

    if (!newReferencedEndInstanceKey.IsValid() && !oldReferencedEndInstanceKey.IsValid())
        return;

    // Check if the other end of the relationship matches the actual class found. 
    if (newReferencedEndInstanceKey.IsValid() && !ClassIdMatchesConstraint(relClassId, referencedEnd, newReferencedEndInstanceKey.GetECClassId()))
        return;
    if (oldReferencedEndInstanceKey.IsValid() && !ClassIdMatchesConstraint(relClassId, referencedEnd, oldReferencedEndInstanceKey.GetECClassId()))
        return;

    DbOpcode relDbOpcode;
    if (newReferencedEndInstanceKey.IsValid() && !oldReferencedEndInstanceKey.IsValid())
        relDbOpcode = DbOpcode::Insert;
    else if (!newReferencedEndInstanceKey.IsValid() && oldReferencedEndInstanceKey.IsValid())
        relDbOpcode = DbOpcode::Delete;
    else /* if (newReferencedEndInstanceKey.IsValid() && oldReferencedEndInstanceKey.IsValid()) */
        relDbOpcode = DbOpcode::Update;

    ECInstanceKeyCP oldSourceInstanceKey, newSourceInstanceKey, oldTargetInstanceKey, newTargetInstanceKey;
    ECInstanceKey invalidKey;
    oldSourceInstanceKey = newSourceInstanceKey = oldTargetInstanceKey = newTargetInstanceKey = nullptr;
    if (foreignEnd == ECRelationshipEnd_Source)
        {
        oldSourceInstanceKey = (relDbOpcode != DbOpcode::Insert) ? &foreignEndInstanceKey : &invalidKey;
        oldTargetInstanceKey = &oldReferencedEndInstanceKey;
        newSourceInstanceKey = (relDbOpcode != DbOpcode::Delete) ? &foreignEndInstanceKey : &invalidKey;
        newTargetInstanceKey = &newReferencedEndInstanceKey;
        }
    else
        {
        oldSourceInstanceKey = &oldReferencedEndInstanceKey;
        oldTargetInstanceKey = (relDbOpcode != DbOpcode::Insert) ? &foreignEndInstanceKey : &invalidKey;
        newSourceInstanceKey = &newReferencedEndInstanceKey;
        newTargetInstanceKey = (relDbOpcode != DbOpcode::Delete) ? &foreignEndInstanceKey : &invalidKey;
        }

    RecordRelInstance(tableRelClassMap, foreignEndInstanceKey.GetECInstanceId(), relDbOpcode, *oldSourceInstanceKey, *newSourceInstanceKey, *oldTargetInstanceKey, *newTargetInstanceKey);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::ExtractRelInstanceInLinkTable(TableClassMapCR tableClassMap, ECInstanceId relInstanceId)
    {
    BeAssert(nullptr != dynamic_cast<RelationshipClassLinkTableMap const*> (tableClassMap.GetClassMap()));
    RelationshipClassLinkTableMap const* relClassMap = static_cast<RelationshipClassLinkTableMap const*>(tableClassMap.GetClassMap());

    ECInstanceKey oldSourceInstanceKey, newSourceInstanceKey;
    GetRelEndInstanceKeys(oldSourceInstanceKey, newSourceInstanceKey, *relClassMap, relInstanceId, ECRelationshipEnd_Source);

    ECInstanceKey oldTargetInstanceKey, newTargetInstanceKey;
    GetRelEndInstanceKeys(oldTargetInstanceKey, newTargetInstanceKey, *relClassMap, relInstanceId, ECRelationshipEnd_Target);

    RecordRelInstance(tableClassMap, relInstanceId, m_sqlChange->GetDbOpcode(), oldSourceInstanceKey, newSourceInstanceKey, oldTargetInstanceKey, newTargetInstanceKey);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::GetRelEndInstanceKeys(ECInstanceKey& oldInstanceKey, ECInstanceKey& newInstanceKey, RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd) const
    {
    oldInstanceKey = newInstanceKey = ECInstanceKey();

    int instanceIdColumnIndex = GetFirstColumnIndex(relClassMap.GetConstraintECInstanceIdPropMap(relEnd));
    BeAssert(instanceIdColumnIndex >= 0);

    ECInstanceId oldEndInstanceId, newEndInstanceId;
    m_sqlChange->GetValueIds(oldEndInstanceId, newEndInstanceId, instanceIdColumnIndex);

    if (newEndInstanceId.IsValid())
        {
        ECClassId newClassId = GetRelEndClassId(relClassMap, relInstanceId, relEnd, newEndInstanceId);
        BeAssert(newClassId.IsValid());
        newInstanceKey = ECInstanceKey(newClassId, newEndInstanceId);
        }

    if (oldEndInstanceId.IsValid())
        {
        ECClassId oldClassId = GetRelEndClassId(relClassMap, relInstanceId, relEnd, oldEndInstanceId);
        BeAssert(oldClassId.IsValid());
        oldInstanceKey = ECInstanceKey(oldClassId, oldEndInstanceId);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
ECN::ECClassId ChangeExtractor::GetRelEndClassId(RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd, ECInstanceId endInstanceId) const
    {
    ConstraintECClassIdPropertyMap const* classIdPropMap = relClassMap.GetConstraintECClassIdPropMap(relEnd);
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
        return GetRelEndClassIdFromRelClass(relClassMap.GetClass().GetRelationshipClassCP(), relEnd);
        }

    // Case #2: End is in only one table    
    const bool endIsInOneTable = classIdPropMap->GetTables().size() == 1;
    if (endIsInOneTable)
        {
        Utf8StringCR endTableName = classIdColumn->GetTable().GetName();

        // Search in all changes
        ECClassId classId = m_instancesTable.QueryClassId(endTableName, endInstanceId);
        if (classId.IsValid())
            return classId;

        // Search in the end table
        classId = GetTableMap(endTableName)->QueryValueId<ECClassId>(classIdColumn->GetName(), endInstanceId);
        BeAssert(classId.IsValid());
        return classId;
        }

    // Case #3: End could be in many tables
    Utf8StringCR classIdColumnName = classIdColumn->GetName();
    int classIdColumnIndex = m_tableMap->GetColumnIndexByName(classIdColumnName);
    BeAssert(classIdColumnIndex >= 0);

    ECClassId classId = GetClassIdFromChangeOrTable(classIdColumnName.c_str(), relInstanceId);
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

    Changes changes = changeSet.GetChanges();
    return m_changeExtractor->ExtractFromSqlChanges(changes, options.GetIncludeRelationshipInstances());
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

    ECN::ECClassCP ecClass = m_ecdb.Schemas().GetECClass(id);
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
            if (accessString.Contains("ECInstanceId"))
                {
                oldValueStr = FormatInstanceIdStr(oldValue.GetValueId<ECInstanceId>());
                newValueStr = FormatInstanceIdStr(newValue.GetValueId<ECInstanceId>());
                }
            else if (accessString.Contains("ECClassId"))
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
Utf8String ChangeSummary::InstanceIterator::Options::ToSelectStatement(Utf8CP columns, ChangeSummaryCR summary) const
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
//     WHERE IsChangedInstance(elg.GetECClassId(), elg.ECInstanceId)
//---------------------------------------------------------------------------------------
void IsChangedInstanceSqlFunction::_ComputeScalar(ScalarFunction::Context& ctx, int nArgs, DbValue* args)
    {
    if (nArgs != 3 || args[0].IsNull() || args[1].IsNull() || args[2].IsNull())
        {
        ctx.SetResultError("Arguments to IsChangedInstance must be (changeSummary, ECClassId, ECInstanceId) ", -1);
        return;
        }

    ChangeSummaryCP changeSummary = (ChangeSummaryCP) args[0].GetValueInt64();
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
TableMapCP ChangeIterator::GetTableMap(Utf8StringCR tableName) const
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
ChangeIterator::RowEntry::RowEntry(ChangeIteratorCR iterator, Changes::Change const& change) : m_ecdb(iterator.GetDb()), m_iterator(iterator), m_change(change)
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
void ChangeIterator::RowEntry::Initialize()
    {
    m_isValid = MoveToMappedChange();
    if (!m_isValid)
        return;

    m_isValid = InitPrimaryInstance();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
bool ChangeIterator::RowEntry::MoveToMappedChange()
    {
    while (m_change.IsValid())
        {
        InitSqlChange();

        Utf8StringCR tableName = m_sqlChange->GetTableName();
        if (tableName.StartsWith("ec_"))
            {
            m_sqlChange->GetChange().Dump(GetDb(), false, 1);
            BeAssert(false && "ChangeSet includes changes to the ECSchema. Change summaries are not reliable.");
            return false;
            }

        m_tableMap = m_iterator.GetTableMap(tableName);
        BeAssert(m_tableMap != nullptr);
        if (m_tableMap->IsMapped())
            return true;

        LOG.infov("ChangeSummary skipping table %s since it's not mapped", m_tableMap->GetTableName().c_str());
        ++m_change;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
bool ChangeIterator::RowEntry::InitPrimaryInstance()
    {
    m_primaryInstanceId = m_sqlChange->GetValueId<ECInstanceId>(m_tableMap->GetECInstanceIdColumn().GetPhysicalIndex());
    BeAssert(m_primaryInstanceId.IsValid());

    ECClassId primaryClassId;
    if (m_tableMap->ContainsECClassIdColumn())
        primaryClassId = GetClassIdFromChangeOrTable(m_tableMap->GetECClassIdColumn().GetPhysicalName().c_str(), m_primaryInstanceId);
    else
        primaryClassId = m_tableMap->GetECClassId();

    m_primaryClass = m_ecdb.Schemas().GetECClass(primaryClassId);
    if (m_primaryClass == nullptr)
        {
        BeAssert(false && "Couldn't determine the class corresponding to the change.");
        return false;
        }

    return true;
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
    return m_tableMap->GetDbTable()->GetName(); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
bool ChangeIterator::RowEntry::IsJoinedTable() const
    { 
    return m_tableMap->GetDbTable()->GetType() == DbTable::Type::Joined; 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
DbOpcode ChangeIterator::RowEntry::GetDbOpcode() const
    { 
    return m_sqlChange->GetDbOpcode(); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ColumnIterator ChangeIterator::RowEntry::MakeColumnIterator(ECClassCR ecClass) const
    {
    return ChangeIterator::ColumnIterator(*this, ecClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
int ChangeIterator::RowEntry::GetIndirect() const
    { 
    return m_sqlChange->GetIndirect(); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::RowEntry& ChangeIterator::RowEntry::operator++()
    {
   ++ m_change;
    Initialize();
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
DbDupValue ChangeIterator::RowEntry::QueryValueFromDb(Utf8StringCR physicalColumnName) const
    {
    return m_tableMap->QueryValueFromDb(physicalColumnName, m_primaryInstanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ColumnIterator::ColumnIterator(RowEntryCR rowEntry, ECN::ECClassCR ecClass) : m_rowEntry(rowEntry), m_ecdb(rowEntry.GetDb()), m_sqlChange(rowEntry.GetSqlChange())
    {
    m_tableClassMap = rowEntry.GetTableMap()->GetTableClassMap(ecClass);
    BeAssert(m_tableClassMap->IsMapped());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ECClassCR ChangeIterator::ColumnIterator::GetClass() const
    {
    return m_tableClassMap->GetClass();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ColumnEntry ChangeIterator::ColumnIterator::GetColumn(Utf8CP propertyAccessString) const
    {
    ColumnMapByAccessString const& columnMaps = m_tableClassMap->GetColumnMapByAccessString();
    ColumnMapByAccessString::const_iterator it = columnMaps.find(propertyAccessString);
    return ChangeIterator::ColumnEntry(*this, columnMaps, it);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ColumnEntry ChangeIterator::ColumnIterator::begin() const
    {
    ColumnMapByAccessString const& columnMaps = m_tableClassMap->GetColumnMapByAccessString();
    return ChangeIterator::ColumnEntry(*this, columnMaps, columnMaps.begin());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ColumnEntry ChangeIterator::ColumnIterator::end() const
    {
    ColumnMapByAccessString const& columnMaps = m_tableClassMap->GetColumnMapByAccessString();
    return ChangeIterator::ColumnEntry(*this, columnMaps, columnMaps.end());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
ChangeIterator::ColumnEntry::ColumnEntry(ColumnIteratorCR columnIterator, ColumnMapByAccessString const& columnMaps, ColumnMapByAccessString::const_iterator columnMapIterator)
    : m_ecdb(columnIterator.GetDb()), m_sqlChange(columnIterator.GetSqlChange()), m_columnIterator(columnIterator), m_columnMaps(columnMaps), m_columnMapIterator(columnMapIterator)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
DbDupValue ChangeIterator::ColumnEntry::ExtractOverflowValue(DbValue const& columnValue, ColumnMap const& columnMap) const
    {
    if (!columnValue.IsValid() || columnValue.IsNull())
        return DbDupValue(nullptr);

    // TODO: Avoid using the common shared cache
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT json_extract(?,?)");
    BeAssert(stmt.IsValid());

    Utf8PrintfString extractPropExpr("$.%s", columnMap.GetOverflowName().c_str());

    stmt->BindDbValue(1, columnValue);
    stmt->BindText(2, extractPropExpr.c_str(), Statement::MakeCopy::No);

    DbResult result = stmt->Step();
    BeAssert(result == BE_SQLITE_ROW);

    return stmt->GetDbValue(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
DbDupValue ChangeIterator::ColumnEntry::QueryValueFromDb() const
    {
    ColumnMap const& columnMap = m_columnMapIterator->second;
    DbDupValue value = m_columnIterator.GetRowEntry().QueryValueFromDb(columnMap.GetPhysicalName());

    if (columnMap.IsOverflow())
        return ExtractOverflowValue(value, columnMap);

    return value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
DbDupValue ChangeIterator::ColumnEntry::GetValue(Changes::Change::Stage stage) const
    {
    ColumnMap const& columnMap = m_columnMapIterator->second;

    DbValue value = m_sqlChange->GetChange().GetValue(columnMap.GetPhysicalIndex(), stage);
    if (columnMap.IsOverflow())
        return ExtractOverflowValue(value, columnMap);

    return DbDupValue(value.GetSqlValueP());        
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     12/2016
//---------------------------------------------------------------------------------------
bool ChangeIterator::ColumnEntry::IsPrimaryKeyColumn() const
    {
    ColumnMap const& columnMap = m_columnMapIterator->second;
    int idx = columnMap.GetPhysicalIndex();
    return m_sqlChange->IsPrimaryKeyColumn(idx);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
