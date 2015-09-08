/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ChangeSummary.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

#define CHANGED_TABLES_TEMP_PREFIX "temp."
#define CHANGED_INSTANCES_TABLE_BASE_NAME "ec_ChangedInstances"
#define CHANGED_VALUES_TABLE_BASE_NAME "ec_ChangedValues"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

int ChangeSummary::s_count = 0;
IsChangedInstanceSqlFunction* ChangeSummary::s_isChangedInstanceSqlFunction = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::SqlChange::SqlChange(Changes::Change const& change) : m_sqlChange(change)
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
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::ClassMap::AddColumn(Utf8CP accessString, Utf8CP columnName, int columnIndex, int columnType)
    {
    ColumnMap columnMap(accessString, columnName, columnIndex, columnType);
    m_columnMapByAccessString[accessString] = columnMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeSummary::ClassMap::GetIsForeignKeyRelationship() const
    {
    ECDbMapStrategy::Strategy mapStrategy = (ECDbMapStrategy::Strategy) m_mapStrategy;
    return (ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable == mapStrategy || ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable == mapStrategy);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
Utf8CP ChangeSummary::ClassMap::GetColumnName(Utf8CP accessString) const
    {
    ColumnMapByAccessString::const_iterator iter = m_columnMapByAccessString.find(accessString);
    if (iter != m_columnMapByAccessString.end())
        return iter->second.GetColumnName();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
int ChangeSummary::ClassMap::GetColumnIndex(Utf8CP accessString) const
    {
    ColumnMapByAccessString::const_iterator iter = m_columnMapByAccessString.find(accessString);
    if (iter != m_columnMapByAccessString.end())
        return iter->second.GetColumnIndex();

    return -1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeSummary::ClassMap::Initialize(ECDbR ecdb, ECClassId classId, Utf8CP tableName, bmap<Utf8String, int> const& columnIndexByName)
    {
    m_classId = classId;
    m_tableName = tableName;
    m_mapStrategy = (int) ECDbMapStrategy::Strategy::NotMapped;

    CachedStatementPtr statement = ecdb.GetCachedStatement(
        "SELECT ec_Class.Name, ec_Class.IsRelationship, ec_Class.IsStruct, ec_ClassMap.MapStrategy"
        " FROM ec_Class "
        " JOIN ec_ClassMap ON ec_ClassMap.ClassId = ec_Class.Id"
        " WHERE ec_Class.Id=?");
    BeAssert(statement.IsValid());

    statement->BindInt64(1, (int64_t) classId);

    DbResult result = statement->Step();
    BeAssert(result == BE_SQLITE_ROW);
    if (result == BE_SQLITE_DONE)
        {
        BeAssert(false && "A class exists that isn't in the map table. Didn't expect this to be possible");
        return false;
        }

    m_className = statement->GetValueText(0);
    m_isRelationship = statement->GetValueInt(1) > 0;
    m_isStruct = statement->GetValueInt(2) > 0;
    m_mapStrategy = statement->GetValueInt(3);

    if (m_mapStrategy == (int) ECDbMapStrategy::Strategy::NotMapped)
        return false;

    statement = ecdb.GetCachedStatement(
        "SELECT ec_Column.Name, ec_PropertyPath.AccessString, ec_Column.Type"
        " FROM ec_Column"
        " JOIN ec_PropertyMap ON ec_PropertyMap.ColumnId = ec_Column.Id"
        " JOIN ec_PropertyPath ON ec_PropertyMap.PropertyPathId = ec_PropertyPath.Id"
        " JOIN ec_ClassMap ON ec_ClassMap.Id = ec_PropertyMap.ClassMapId"
        " JOIN ec_Class ON ec_Class.Id = ec_ClassMap.ClassId"
        " JOIN ec_Table ON ec_Table.Id = ec_Column.TableId"
        " WHERE ec_Column.IsVirtual=0 AND ec_Table.Name=? AND ec_Class.Id=?");
    BeAssert(statement.IsValid());

    statement->BindText(1, tableName, Statement::MakeCopy::No);
    statement->BindInt64(2, (int64_t) classId);

    while (BE_SQLITE_ROW == (result = statement->Step()))
        {
        Utf8CP columnName = statement->GetValueText(0);
        Utf8CP accessString = statement->GetValueText(1);
        int columnType = statement->GetValueInt(2);

        bmap<Utf8String, int>::const_iterator iter = columnIndexByName.find(columnName);
        BeAssert(iter != columnIndexByName.end());
        AddColumn(accessString, columnName, iter->second, columnType);
        }
    BeAssert(BE_SQLITE_DONE == result);
    
    // Struct arrays require special treatment - add additional columns to the map
    if (m_isStruct && columnIndexByName.find("ParentECInstanceId") != columnIndexByName.end())
        {
        bmap<Utf8String, int>::const_iterator iter;

        iter = columnIndexByName.find("ParentECInstanceId");
        BeAssert(iter != columnIndexByName.end());
        AddColumn("ParentECInstanceId", "ParentECInstanceId", iter->second, (int) ECDbSqlColumn::Type::Long);

        iter = columnIndexByName.find("ECPropertyPathId");
        BeAssert(iter != columnIndexByName.end());
        AddColumn("ECPropertyPathId", "ECPropertyPathId", iter->second, (int) ECDbSqlColumn::Type::Integer);

        iter = columnIndexByName.find("ECArrayIndex");
        BeAssert(iter != columnIndexByName.end());
        AddColumn("ECArrayIndex", "ECArrayIndex", iter->second, (int) ECDbSqlColumn::Type::Integer);
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeSummary::TableMap::QueryInstanceIdColumnFromMap(Utf8StringR idColumnName, ECDbR ecdb, Utf8CP tableName) const
    {
    return QueryIdColumnFromMap(idColumnName, ecdb, tableName, 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeSummary::TableMap::QueryClassIdColumnFromMap(Utf8StringR idColumnName, ECDbR ecdb, Utf8CP tableName) const
    {
    return QueryIdColumnFromMap(idColumnName, ecdb, tableName, 2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeSummary::TableMap::QueryIdColumnFromMap(Utf8StringR idColumnName, ECDbR ecdb, Utf8CP tableName, int userData) const
    {
    CachedStatementPtr statement = ecdb.GetCachedStatement(
        "SELECT ec_Column.Name"
        " FROM ec_Column"
        " JOIN ec_Table ON ec_Table.Id = ec_Column.TableId"
        " WHERE ec_Table.Name = ? AND ec_Column.IsVirtual = 0 AND ec_Column.KnownColumn = ?");
    BeAssert(statement.IsValid());

    statement->BindText(1, tableName, Statement::MakeCopy::No);
    statement->BindInt(2, userData);

    DbResult result = statement->Step();
    if (result == BE_SQLITE_DONE)
        return false;

    BeAssert(result == BE_SQLITE_ROW);
    idColumnName = statement->GetValueText(0);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::TableMap::Initialize(ECDbR ecdb, Utf8CP tableName)
    {
    m_tableName = tableName;

    bvector<Utf8String> columnNames;
    ecdb.GetColumns(columnNames, tableName);

    bmap<Utf8String, int> columnIndexByName;
    for (int ii = 0; ii < (int) columnNames.size(); ii++)
        columnIndexByName[columnNames[ii]] = ii;

    if (!QueryInstanceIdColumnFromMap(m_ecInstanceIdColumnName, ecdb, tableName))
        return;
    bmap<Utf8String, int>::const_iterator iter = columnIndexByName.find(m_ecInstanceIdColumnName);
    BeAssert(iter != columnIndexByName.end());
    m_ecInstanceIdColumnIndex = iter->second;
    BeAssert(m_ecInstanceIdColumnIndex >= 0);

    if (QueryClassIdColumnFromMap(m_ecClassIdColumnName, ecdb, tableName))
        {
        bmap<Utf8String, int>::const_iterator iter = columnIndexByName.find(m_ecClassIdColumnName);
        BeAssert(iter != columnIndexByName.end());
        m_ecClassIdColumnIndex = iter->second;
        }
    
    CachedStatementPtr statement = ecdb.GetCachedStatement(
        "SELECT DISTINCT ec_Class.Id"
        " FROM ec_Class"
        " JOIN ec_ClassMap ON ec_Class.Id = ec_ClassMap.ClassId"
        " JOIN ec_PropertyMap ON ec_ClassMap.Id = ec_PropertyMap.ClassMapId"
        " JOIN ec_Column ON ec_PropertyMap.ColumnId = ec_Column.Id"
        " JOIN ec_Table ON ec_Table.Id = ec_Column.TableId"
        " WHERE ec_Table.Name = ? AND ec_Column.IsVirtual = 0 AND ec_Column.KnownColumn = 1");
    BeAssert(statement.IsValid());

    statement->BindText(1, tableName, Statement::MakeCopy::No);

    DbResult result;
    while (BE_SQLITE_ROW == (result = statement->Step()))
        {
        ECClassId classId = (ECClassId) statement->GetValueInt64(0);

        ClassMapP classMap = new ClassMap();
        bool isMapped = classMap->Initialize(ecdb, classId, tableName, columnIndexByName);
        if (!isMapped)
            {
            delete classMap;
            continue;
            }
        
        m_classMaps[classId] = classMap;
        }
    BeAssert(BE_SQLITE_DONE == result);

    for (ClassMapById::const_iterator iter = m_classMaps.begin(); iter != m_classMaps.end(); iter++)
        {
        statement = ecdb.GetCachedStatement("SELECT ec_BaseClass.BaseClassId FROM ec_BaseClass WHERE ec_BaseClass.ClassId = ?");
        BeAssert(statement.IsValid());

        statement->BindInt64(1, iter->first);

        bvector<ClassMapCP> baseClassMaps;
        while (BE_SQLITE_ROW == (result = statement->Step()))
            {
            ECClassId baseClassId = (ECClassId) statement->GetValueInt64(0);

            ClassMapById::const_iterator baseIter = m_classMaps.find(baseClassId);
            if (baseIter == m_classMaps.end())
                continue; // Either unmapped, or not in the same table. 

            baseClassMaps.push_back(baseIter->second);
            }
        BeAssert(BE_SQLITE_DONE == result);

        m_classMaps[iter->first]->SetBaseClassMaps(baseClassMaps);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::TableMap::FreeClassMap()
    {
    for (ClassMapById::const_iterator iter = m_classMaps.begin(); iter != m_classMaps.end(); iter++)
        delete iter->second;
    m_classMaps.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::TableMap::~TableMap()
    {
    FreeClassMap();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     09/2015
//---------------------------------------------------------------------------------------
ChangeSummary::ChangeSummary(ECDbR ecdb) : m_ecdb(ecdb)
    {
    SetupChangedTableNames();

    if (s_count == 0)
        RegisterSqlFunctions(m_ecdb);
    s_count++;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     09/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::SetupChangedTableNames()
    {
    m_instancesTableNameNoPrefix = Utf8PrintfString(CHANGED_INSTANCES_TABLE_BASE_NAME "_%d", s_count);
    m_valuesTableNameNoPrefix = Utf8PrintfString(CHANGED_VALUES_TABLE_BASE_NAME "_%d", s_count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::Initialize()
    {
    if (IsValid())
        return;

    CreateInstancesTable();
    PrepareInstancesTableStatements();

    CreateValuesTable();
    PrepareValuesTableStatements();

    m_isValid = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::Free()
    {
    if (!IsValid())
        return;

    FinalizeInstancesTableStatements();
    ClearInstancesTable();

    FinalizeValuesTableStatements();
    ClearValuesTable();

    m_isValid = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::~ChangeSummary()
    {
    Free();
    FreeTableMap();

    s_count--;
    if (s_count == 0)
        UnregisterSqlFunctions();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::TableMapCP ChangeSummary::GetTableMap(Utf8CP tableName) const
    {
    if (m_tableMapByName.find(tableName) == m_tableMapByName.end())
        {
        AddTableToMap(tableName);
        BeAssert(m_tableMapByName.find(tableName) != m_tableMapByName.end());
        }

    return m_tableMapByName[tableName];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::AddTableToMap(Utf8CP tableName) const
    {
    TableMapP tableMap = new TableMap();
    tableMap->Initialize (m_ecdb, tableName);
    m_tableMapByName[tableName] = tableMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::FreeTableMap()
    {
    for (TableMapByName::const_iterator iter = m_tableMapByName.begin(); iter != m_tableMapByName.end(); iter++)
        delete iter->second;
    m_tableMapByName.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
DbDupValue ChangeSummary::GetValueFromTable(Utf8CP tableName, Utf8CP columnName, Utf8CP instanceIdColumnName, ECInstanceId instanceId) const
    {
    Utf8PrintfString ecSql("SELECT %s FROM %s WHERE %s=?", columnName, tableName, instanceIdColumnName);
    CachedStatementPtr statement = m_ecdb.GetCachedStatement(ecSql.c_str());
    BeAssert(statement.IsValid());

    statement->BindId(1, instanceId);

    DbResult result = statement->Step();
    if (BE_SQLITE_ROW == result)
        return statement->GetDbValue(0);

    BeAssert(result == BE_SQLITE_DONE);
    return std::move(DbDupValue(nullptr));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
DbValue ChangeSummary::GetValueFromChange(SqlChange const& sqlChange, int columnIndex) const
    {
    return sqlChange.GetChange().GetValue(columnIndex, (sqlChange.GetDbOpcode() == DbOpcode::Insert) ? Changes::Change::Stage::New : Changes::Change::Stage::Old);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
int64_t ChangeSummary::GetValueInt64FromTable(Utf8CP tableName, Utf8CP columnName, Utf8CP instanceIdColumnName, ECInstanceId instanceId) const
    {
    DbDupValue value = GetValueFromTable(tableName, columnName, instanceIdColumnName, instanceId);
    if (!value.IsValid() || value.IsNull())
        return -1;
    return value.GetValueInt64();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
int64_t ChangeSummary::GetValueInt64FromChange(SqlChange const& sqlChange, int columnIndex) const
    {
    DbValue value = GetValueFromChange(sqlChange, columnIndex);
    if (!value.IsValid() || value.IsNull())
        return -1;
    return value.GetValueInt64();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
int64_t ChangeSummary::GetValueInt64FromChangeOrTable(SqlChange const& sqlChange, Utf8CP tableName, Utf8CP columnName, int columnIndex, Utf8CP instanceIdColumnName, ECInstanceId instanceId) const
    {
    if (columnIndex < 0)
        return -1;

    DbOpcode dbOpcode = sqlChange.GetDbOpcode();

    if (dbOpcode == DbOpcode::Insert || dbOpcode == DbOpcode::Delete)
        return GetValueInt64FromChange(sqlChange, columnIndex);

    /* if (dbOpcode == DbOpcode::Update) */
    return GetValueInt64FromTable(tableName, columnName, instanceIdColumnName, instanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ECClassId ChangeSummary::GetClassIdFromChangeOrTable(SqlChange const& sqlChange, TableMap const& tableMap, ECInstanceId instanceId) const
    {
    int64_t value = GetValueInt64FromChangeOrTable(sqlChange, tableMap.GetTableName(), tableMap.GetECClassIdColumnName(), tableMap.GetECClassIdColumnIndex(), tableMap.GetECInstanceIdColumnName(), instanceId);
    return (ECClassId) value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ECInstanceId ChangeSummary::GetInstanceIdFromChange(SqlChange const& sqlChange, TableMap const& tableMap) const
    {
    int64_t value = GetValueInt64FromChange(sqlChange, tableMap.GetECInstanceIdColumnIndex());
    return ECInstanceId(value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeSummary::IsInstanceDeleted(ECInstanceId instanceId, TableMap const& tableMap) const
    {
    int64_t dummyId = GetValueInt64FromTable(tableMap.GetTableName(), tableMap.GetECInstanceIdColumnName(), tableMap.GetECInstanceIdColumnName(), instanceId);
    return dummyId <= 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeSummary::GetStructArrayParentFromChange(ECClassId& parentClassId, ECInstanceId& parentInstanceId, SqlChange const& sqlChange, ClassMap const& classMap, ECInstanceId instanceId)
    {
    int parentECInstanceIdColumnIndex = classMap.GetColumnIndex("ParentECInstanceId");
    if (parentECInstanceIdColumnIndex < 0)
        return false;

    int ecPropertyPathIdColumnIndex = classMap.GetColumnIndex("ECPropertyPathId");
    BeAssert(ecPropertyPathIdColumnIndex >= 0);

    Utf8CP ecInstanceIdColumnName = classMap.GetColumnName("ECInstanceId");
    BeAssert(ecInstanceIdColumnName != nullptr);

    int64_t value = GetValueInt64FromChangeOrTable(sqlChange, classMap.GetTableName(), classMap.GetColumnName("ParentECInstanceId"), parentECInstanceIdColumnIndex, ecInstanceIdColumnName, instanceId);
    if (value <= 0)
        return false;
    parentInstanceId = ECInstanceId(value);

    int64_t propertyPathId = GetValueInt64FromChangeOrTable(sqlChange, classMap.GetTableName(), classMap.GetColumnName("ECPropertyPathId"), ecPropertyPathIdColumnIndex, ecInstanceIdColumnName, instanceId);
    BeAssert(propertyPathId > 0);

    Utf8CP sql = "SELECT ec_Property.ClassId, ec_Property.IsArray "
        "FROM ec_Property "
        "JOIN ec_PropertyPath ON ec_Property.Id = ec_PropertyPath.RootPropertyId "
        "WHERE ec_PropertyPath.Id = ?";

    CachedStatementPtr statement = m_ecdb.GetCachedStatement(sql);
    BeAssert(statement.IsValid());

    statement->BindInt64(1, propertyPathId);

    DbResult result = statement->Step();
    BeAssert(result == BE_SQLITE_ROW);
    BeAssert(statement->GetValueInt(1) > 0 && "Expected a struct array property");

    parentClassId = (ECClassId) statement->GetValueInt64(0);
    BeAssert(parentClassId > 0);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeSummary::SqlChangeHasUpdatesForClass(SqlChange const& sqlChange, ChangeSummary::ClassMap const& classMap) const
    {
    BeAssert(sqlChange.GetDbOpcode() == DbOpcode::Update);

    ColumnMapByAccessString const& columnMapByAccessString = classMap.GetColumnMapByAccessString();
    for (ColumnMapByAccessString::const_iterator it = columnMapByAccessString.begin(); it != columnMapByAccessString.end(); it++)
        {
        ColumnMap const& columnMap = it->second;

        int columnIndex = columnMap.GetColumnIndex();
        
        if (sqlChange.IsPrimaryKeyColumn(columnIndex))
            continue;

        DbValue dbValue = sqlChange.GetChange().GetValue(columnIndex, Changes::Change::Stage::New);
        if (dbValue.IsValid() && !dbValue.IsNull())
            return true;
        }

    for (ClassMapCP baseClassMap : classMap.GetBaseClassMaps())
        {
        if (SqlChangeHasUpdatesForClass(sqlChange, *baseClassMap))
            return true;
        }
    
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::ProcessSqlChangeForForeignKeyMap(SqlChange const& sqlChange, ChangeSummary::ClassMap const& classMap, ECInstanceId instanceId)
    {
    ECDbMapStrategy::Strategy mapStrategy = (ECDbMapStrategy::Strategy) classMap.GetMapStrategy();

    Utf8CP otherEndIdAccessString = (ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable == mapStrategy) ? "TargetECInstanceId" : "SourceECInstanceId";
    int otherEndIdColumnIndex = classMap.GetColumnIndex(otherEndIdAccessString);
    BeAssert(otherEndIdColumnIndex >= 0);

    DbOpcode dbOpcode = sqlChange.GetDbOpcode();

    DbOpcode relDbOpcode;
    if (dbOpcode == DbOpcode::Insert)
        {
        DbValue newValue = sqlChange.GetChange().GetValue(otherEndIdColumnIndex, Changes::Change::Stage::New);
        if (!newValue.IsValid() || newValue.IsNull())
            return;
        relDbOpcode = DbOpcode::Insert;
        }
    else if (dbOpcode == DbOpcode::Delete)
        {
        DbValue oldValue = sqlChange.GetChange().GetValue(otherEndIdColumnIndex, Changes::Change::Stage::Old);
        if (!oldValue.IsValid() || oldValue.IsNull())
            return;
        relDbOpcode = DbOpcode::Delete;
        }
    else /* if (dbOpcode == DbOpcode::Update) */
        {
        DbValue oldValue = sqlChange.GetChange().GetValue(otherEndIdColumnIndex, Changes::Change::Stage::Old);
        DbValue newValue = sqlChange.GetChange().GetValue(otherEndIdColumnIndex, Changes::Change::Stage::New);

        ECInstanceId otherEndOldId;
        if (oldValue.IsValid() && !oldValue.IsNull())
            otherEndOldId = oldValue.GetValueId<ECInstanceId>();

        ECInstanceId otherEndNewId;
        if (newValue.IsValid() && !newValue.IsNull())
            otherEndNewId = newValue.GetValueId<ECInstanceId>();

        if (!otherEndOldId.IsValid() && !otherEndNewId.IsValid())
            return; // No valid FKEY relationship
        else if (!otherEndOldId.IsValid() && otherEndNewId.IsValid())
            relDbOpcode = DbOpcode::Insert;
        else /* if (otherEndOldId.IsValid()) */
            {
            if (newValue.IsValid() && newValue.IsNull())
                relDbOpcode = DbOpcode::Delete;
            else if (SqlChangeHasUpdatesForClass(sqlChange, classMap))
                relDbOpcode = DbOpcode::Update;
            else
                return; // No changes
            }
        }

    ChangeSummary::Instance instance(*this, classMap.GetClassId(), instanceId, relDbOpcode, sqlChange.GetIndirect());
    RecordInInstancesTable(instance);
    RecordInValuesTable(instance, sqlChange, classMap);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::ProcessSqlChangeForStructMap(SqlChange const& sqlChange, ChangeSummary::ClassMap const& classMap, ECInstanceId instanceId)
    {
    ECClassId parentClassId;
    ECInstanceId parentInstanceId;
    if (GetStructArrayParentFromChange(parentClassId, parentInstanceId, sqlChange, classMap, instanceId))
        {
        ChangeSummary::Instance instance(*this, parentClassId, parentInstanceId, DbOpcode::Update, sqlChange.GetIndirect());
        RecordInInstancesTable(instance);
        // TODO: Support recording changes to struct arrays in changed values table. 
        return;
        }
        
    ProcessSqlChangeForClassMap(sqlChange, classMap, instanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::ProcessSqlChangeForClassMap(SqlChange const& sqlChange, ChangeSummary::ClassMap const& classMap, ECInstanceId instanceId)
    {
    DbOpcode dbOpcode = sqlChange.GetDbOpcode();
    if (dbOpcode == DbOpcode::Update && !SqlChangeHasUpdatesForClass(sqlChange, classMap))
        return;

    ChangeSummary::Instance instance(*this, classMap.GetClassId(), instanceId, dbOpcode, sqlChange.GetIndirect());
    RecordInInstancesTable(instance);
    RecordInValuesTable(instance, sqlChange, classMap);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummary::ProcessSqlChangeForTable(SqlChange const& sqlChange, TableMap const& tableMap)
    {
    if (!tableMap.GetIsMapped())
        {
        LOG.infov("ChangeSummary skipping table %s since it's not mapped", tableMap.GetTableName());
        return SUCCESS;
        }

    ECInstanceId instanceId = GetInstanceIdFromChange(sqlChange, tableMap);
    BeAssert(instanceId.IsValid());

    if (sqlChange.GetDbOpcode() == DbOpcode::Update && IsInstanceDeleted(instanceId, tableMap))
        {
        // Note: The instance doesn't exist anymore, and has been deleted in future change to the Db.
        // Processing updates requires that the instance is still available in the Db to extract sufficient EC information, 
        // especially since a SqlChangeSet records only the updated columns but not the entire row. 
        BeAssert(false && "SqlChangeSet does not span all modifications made to the Db");
        return ERROR;
        }

    ECClassId classId = GetClassIdFromChangeOrTable(sqlChange, tableMap, instanceId);
    
    ClassMapById const& classMaps = tableMap.GetClassMaps();
    for (ClassMapById::const_iterator iter = classMaps.begin(); iter != classMaps.end(); iter++)
        {
        ClassMap const& classMap = *(iter->second);

        if (classMap.GetIsForeignKeyRelationship())
            {
            ProcessSqlChangeForForeignKeyMap(sqlChange, classMap, instanceId); // Special processing of foreign key relationships
            continue;
            }
        
        if ((classId > 0 && classId == classMap.GetClassId()) || classId < 0)
            {
            if (classMap.GetIsStruct())
                ProcessSqlChangeForStructMap(sqlChange, classMap, instanceId); // Special processing of struct array tables
            else
                ProcessSqlChangeForClassMap(sqlChange, classMap, instanceId); // Normal processing of classes
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummary::FromSqlChangeSet(ChangeSet& changeSet)
    {
    Initialize();

    Changes changes(changeSet);
    for (Changes::Change const& change : changes)
        {
        SqlChange sqlChange(change);

        TableMapCP tableMap = GetTableMap(sqlChange.GetTableName());
        BeAssert(tableMap != nullptr);

        BentleyStatus status = ProcessSqlChangeForTable(sqlChange, *tableMap);
        if (SUCCESS != status)
            return status;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
Utf8String ChangeSummary::GetInstancesTableName() const
    {
    Utf8PrintfString tableName(CHANGED_TABLES_TEMP_PREFIX "%s", m_instancesTableNameNoPrefix.c_str());
    return tableName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::CreateInstancesTable()
    {
    Utf8String tableName = GetInstancesTableName();
    if (m_ecdb.TableExists(tableName.c_str()))
        return;

    DbResult result = m_ecdb.CreateTable(tableName.c_str(),
        "[ClassId] INTEGER not null, [InstanceId] INTEGER not null, [DbOpcode] INTEGER not null, [Indirect] INTEGER not null, PRIMARY KEY (ClassId, InstanceId)");
    BeAssert(result == BE_SQLITE_OK);

    Utf8PrintfString sqlIdx("CREATE INDEX " CHANGED_TABLES_TEMP_PREFIX "idx_%s_op ON [%s](DbOpcode)", m_instancesTableNameNoPrefix.c_str(), m_instancesTableNameNoPrefix.c_str());
    result = m_ecdb.ExecuteSql(sqlIdx.c_str());
    BeAssert(result == BE_SQLITE_OK);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::ClearInstancesTable()
    {
    Utf8String tableName = GetInstancesTableName();
    BeAssert(m_ecdb.TableExists(tableName.c_str()));

    Utf8PrintfString sqlDeleteAll("DELETE FROM %s", tableName.c_str());
    DbResult result = m_ecdb.ExecuteSql(sqlDeleteAll.c_str());
    BeAssert(result == BE_SQLITE_OK);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::PrepareInstancesTableStatements()
    {
    Utf8String instancesTableName = GetInstancesTableName();
    BeAssert(m_ecdb.TableExists(instancesTableName.c_str()));

    DbResult result;

    BeAssert(!m_instancesTableInsert.IsPrepared());
    Utf8PrintfString insertSql("INSERT INTO %s (ClassId,InstanceId,DbOpcode,Indirect) VALUES(?1,?2,?3,?4)", instancesTableName.c_str());
    result = m_instancesTableInsert.Prepare(m_ecdb, insertSql.c_str());
    BeAssert(result == BE_SQLITE_OK);

    BeAssert(!m_instancesTableUpdate.IsPrepared());
    Utf8PrintfString updateSql("UPDATE %s SET DbOpcode=?3,Indirect=?4 WHERE ClassId=?1 AND InstanceId=?2", instancesTableName.c_str());
    result = m_instancesTableUpdate.Prepare(m_ecdb, updateSql.c_str());
    BeAssert(result == BE_SQLITE_OK);

    BeAssert(!m_instancesTableSelect.IsPrepared());
    Utf8PrintfString selectSql("SELECT DbOpcode, Indirect FROM %s WHERE ClassId=?1 AND InstanceId=?2", instancesTableName.c_str());
    result = m_instancesTableSelect.Prepare(m_ecdb, selectSql.c_str());
    BeAssert(result == BE_SQLITE_OK);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::FinalizeInstancesTableStatements()
    {
    m_instancesTableInsert.Finalize();
    m_instancesTableUpdate.Finalize();
    m_instancesTableSelect.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::InsertInInstancesTable(ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect)
    {
    BindInstancesTableStatement(m_instancesTableInsert, classId, instanceId, dbOpcode, indirect);

    DbResult result = m_instancesTableInsert.Step();
    BeAssert(result == BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::UpdateInInstancesTable(ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect)
    {
    BindInstancesTableStatement(m_instancesTableUpdate, classId, instanceId, dbOpcode, indirect);

    DbResult result = m_instancesTableUpdate.Step();
    BeAssert(result == BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::BindInstancesTableStatement(Statement& statement, ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect) const
    {
    BindInstancesTableStatement(statement, classId, instanceId);
    statement.BindInt(3, (int) dbOpcode);
    statement.BindInt(4, indirect);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::BindInstancesTableStatement(Statement& statement, ECClassId classId, ECInstanceId instanceId) const
    {
    statement.Reset();
    statement.BindInt64(1, classId);
    statement.BindId(2, instanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::Instance ChangeSummary::SelectInInstancesTable(ECClassId classId, ECInstanceId instanceId) const
    {
    BindInstancesTableStatement(m_instancesTableSelect, classId, instanceId);

    DbResult result = m_instancesTableSelect.Step();
    if (result == BE_SQLITE_ROW)
        {
        ChangeSummary::Instance instance(*this, classId, instanceId, (DbOpcode) m_instancesTableSelect.GetValueInt(0), m_instancesTableSelect.GetValueInt(1));
        return instance;
        }

    BeAssert(result == BE_SQLITE_DONE);

    ChangeSummary::Instance invalidInstance;
    return invalidInstance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeSummary::HasInstance(ECClassId classId, ECInstanceId instanceId) const
    {
    BindInstancesTableStatement(m_instancesTableSelect, classId, instanceId);

    DbResult result = m_instancesTableSelect.Step();
    BeAssert(result == BE_SQLITE_ROW || BE_SQLITE_DONE);

    return result == BE_SQLITE_ROW;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::Instance ChangeSummary::GetInstance(ECClassId classId, ECInstanceId instanceId) const
    {
    return SelectInInstancesTable(classId, instanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::RecordInInstancesTable(ChangeSummary::Instance const& instance)
    {
    /*
    * Note: Struct arrays are the only known case where a previous
    * change to the change table can be overridden by a later change.
    *
    * Here's the logic to consolidate new changes with the ones
    * previously found:
    *
    * not-found    + new:*       = Insert new entry
    *
    * found:UPDATE + new:INSERT  = Update existing entry to INSERT
    * found:UPDATE + new:DELETE  = Update existing entry to DELETE
    *
    * found:UPDATE + new:UPDATE  = Keep existing entry
    * found:INSERT + new:UPDATE  = Keep existing entry
    * found:DELETE + new:UPDATE  = Keep existing entry
    *
    * <all other cases can never happen>
    */

    ECClassId classId = instance.GetClassId();
    ECInstanceId instanceId = instance.GetInstanceId();
    DbOpcode dbOpcode = instance.GetDbOpcode();
    int indirect = instance.GetIndirect();

    ChangeSummary::Instance foundInstance = SelectInInstancesTable(classId, instanceId);
    if (!foundInstance.IsValid())
        {
        InsertInInstancesTable(classId, instanceId, dbOpcode, indirect);
        return;
        }

    if (foundInstance.GetDbOpcode() == DbOpcode::Update && dbOpcode != DbOpcode::Update)
        {
        UpdateInInstancesTable(classId, instanceId, dbOpcode, indirect);
        return;
        }

    BeAssert(dbOpcode == DbOpcode::Update); // This is the only legal possibility. 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::InsertInValuesTable(ECN::ECClassId classId, ECInstanceId instanceId, Utf8CP accessString, DbValue const& oldValue, DbValue const& newValue)
    {
    BindValuesTableStatement(m_valuesTableInsert, classId, instanceId, accessString, oldValue, newValue);

    DbResult result = m_valuesTableInsert.Step();
    BeAssert(result == BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::BindValuesTableStatement(Statement& statement, ECN::ECClassId classId, ECInstanceId instanceId, Utf8CP accessString, DbValue const& oldValue, DbValue const& newValue)
    {
    statement.Reset();
    statement.BindInt64(1, (int64_t) classId);
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
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::RecordInValuesTable(ChangeSummary::Instance const& instance, SqlChange const& sqlChange, ChangeSummary::ClassMap const& classMap)
    {
    ECN::ECClassId classId = instance.GetClassId();
    ECInstanceId instanceId = instance.GetInstanceId();
    DbOpcode dbOpcode = instance.GetDbOpcode();

    ColumnMapByAccessString const& columnMapByAccessString = classMap.GetColumnMapByAccessString();

    for (ColumnMapByAccessString::const_iterator it = columnMapByAccessString.begin(); it != columnMapByAccessString.end(); it++)
        {
        ColumnMap const& columnMap = it->second;
        int columnIndex = columnMap.GetColumnIndex();
        Utf8CP accessString = it->first.c_str();

        // Filter out duplicates - these are possible for the id columns that get repeated in sub classes. 
        if (instance.HasValue(accessString))
            continue;

        DbValue oldValue(nullptr);
        if (dbOpcode == DbOpcode::Delete || dbOpcode == DbOpcode::Update)
            oldValue = sqlChange.GetChange().GetValue(columnIndex, Changes::Change::Stage::Old);

        DbValue newValue(nullptr);
        if (dbOpcode == DbOpcode::Insert || dbOpcode == DbOpcode::Update)
            newValue = sqlChange.GetChange().GetValue(columnIndex, Changes::Change::Stage::New);

        bool hasOldValue = oldValue.IsValid() && !oldValue.IsNull();
        bool hasNewValue = newValue.IsValid() && !newValue.IsNull();

        DbDupValue oldDupValue(nullptr), newDupValue(nullptr); // declared early 'cos these need to be held in memory
        if (dbOpcode != sqlChange.GetDbOpcode())
            {
            /*
            * Note: In the case of FKEY relationships, an insert or delete can be caused by an update
            * to the table. In these cases, all the old or new values necessary may not be part of
            * change record since update records only changed values.
            * We make an attempt to retrieve these values from the current state of the Db.
            */
            BeAssert(dbOpcode == DbOpcode::Insert || dbOpcode == DbOpcode::Delete);
            BeAssert(sqlChange.GetDbOpcode() == DbOpcode::Update);

            if (dbOpcode == DbOpcode::Insert && !hasNewValue)
                {
                newDupValue = std::move(GetValueFromTable(classMap.GetTableName(), columnMap.GetColumnName(), classMap.GetColumnName("ECInstanceId"), instanceId));
                newValue = DbValue(newDupValue.GetSqlValueP());
                hasNewValue = newValue.IsValid() && !newValue.IsNull();
                }
            else if (dbOpcode == DbOpcode::Delete && !hasOldValue)
                {
                oldDupValue = std::move(GetValueFromTable(classMap.GetTableName(), columnMap.GetColumnName(), classMap.GetColumnName("ECInstanceId"), instanceId));
                oldValue = DbValue(oldDupValue.GetSqlValueP());
                hasOldValue = oldValue.IsValid() && !oldValue.IsNull();
                }
            }

        if (!hasOldValue && !hasNewValue)
            continue; // Do not persist entirely empty fields

        InsertInValuesTable(classId, instanceId, accessString, oldValue, newValue);
        }

    for (ClassMapCP baseClassMap : classMap.GetBaseClassMaps())
        {
        RecordInValuesTable(instance, sqlChange, *baseClassMap);
        }
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
        
    printf("InstanceId;ClassId;ClassName;DbOpcode;Indirect\n");

    for (ChangeSummary::InstanceIterator::const_iterator const& iEntry : MakeInstanceIterator())
        {
        ChangeSummary::Instance instance = iEntry.GetInstance();

        ECClassId classId = instance.GetClassId();
        ECInstanceId instanceId = instance.GetInstanceId();
        int indirect = instance.GetIndirect();

        ECN::ECClassCP ecClass = m_ecdb.Schemas().GetECClass(classId);
        BeAssert(ecClass != nullptr);
        Utf8String className(ecClass->GetFullName());

        DbOpcode opCode = instance.GetDbOpcode();
        Utf8String opCodeStr;
        if (opCode == DbOpcode::Insert)
            opCodeStr = "Insert";
        else if (opCode == DbOpcode::Update)
            opCodeStr = "Update";
        else /* if (opCode = DbOpcode::Delete) */
            opCodeStr = "Delete";

        printf("%" PRId64 ";%" PRId64 ";%s;%s;%s\n", instanceId.GetValueUnchecked(), (int64_t) classId, className.c_str(), opCodeStr.c_str(), indirect > 0 ? "Yes" : "No");

        for (ChangeSummary::ValueIterator::const_iterator const& vEntry : instance.MakeValueIterator(*this))
            {
            Utf8CP accessString = vEntry.GetAccessString();
            DbDupValue oldValue = vEntry.GetOldValue();
            DbDupValue newValue = vEntry.GetNewValue();

            Utf8String oldValueStr = oldValue.Format(0);
            Utf8String newValueStr = newValue.Format(0);

            printf("\t%s;%s;%s\n", accessString, oldValueStr.c_str(), newValueStr.c_str());
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
Utf8String ChangeSummary::GetValuesTableName() const
    {
    Utf8PrintfString tableName(CHANGED_TABLES_TEMP_PREFIX "%s", m_valuesTableNameNoPrefix.c_str());
    return tableName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::CreateValuesTable()
    {
    Utf8String tableName = GetValuesTableName();
    if (m_ecdb.TableExists(tableName.c_str()))
        return;

    Utf8PrintfString sql("[Id] INTEGER NOT NULL, [ClassId] INTEGER NOT NULL, [InstanceId] INTEGER NOT NULL, [AccessString] TEXT not null, [OldValue] BLOB, [NewValue] BLOB,  "
                         "PRIMARY KEY ([Id]), FOREIGN KEY ([ClassId],[InstanceId]) REFERENCES %s ON DELETE CASCADE ON UPDATE NO ACTION", m_instancesTableNameNoPrefix.c_str());

    DbResult result = m_ecdb.CreateTable(tableName.c_str(), sql.c_str());
    BeAssert(result == BE_SQLITE_OK);

    Utf8PrintfString sqlIdx("CREATE UNIQUE INDEX " CHANGED_TABLES_TEMP_PREFIX "idx_%s_AccessStrUnique ON [%s] (ClassId, InstanceId, AccessString)", m_valuesTableNameNoPrefix.c_str(), m_valuesTableNameNoPrefix.c_str());
    result = m_ecdb.ExecuteSql(sqlIdx.c_str());
    BeAssert(result == BE_SQLITE_OK);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::ClearValuesTable()
    {
    Utf8String tableName = GetValuesTableName();
    BeAssert(m_ecdb.TableExists(tableName.c_str()));

    Utf8PrintfString sqlDeleteAll("DELETE FROM %s", tableName.c_str());
    DbResult result = m_ecdb.ExecuteSql(sqlDeleteAll.c_str());
    BeAssert(result == BE_SQLITE_OK);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::PrepareValuesTableStatements()
    {
    Utf8String valuesTableName = GetValuesTableName();
    BeAssert(m_ecdb.TableExists(valuesTableName.c_str()));

    BeAssert(!m_valuesTableInsert.IsPrepared());
    Utf8PrintfString insertSql("INSERT INTO %s (ClassId,InstanceId,AccessString,OldValue,NewValue) VALUES(?1,?2,?3,?4,?5)", valuesTableName.c_str());
    DbResult result = m_valuesTableInsert.Prepare(m_ecdb, insertSql.c_str());
    BeAssert(result == BE_SQLITE_OK);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::FinalizeValuesTableStatements()
    {
    m_valuesTableInsert.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::Instance& ChangeSummary::Instance::operator=(ChangeSummary::Instance const& other)
    {
    m_classId = other.m_classId;
    m_instanceId = other.m_instanceId;
    m_dbOpcode = other.m_dbOpcode;
    m_indirect = other.m_indirect;
    m_changeSummary = other.m_changeSummary;
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

        m_valuesTableSelect->BindInt64(1, (int64_t) m_classId);
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
	if (!IsValid())
	   {
       BeAssert(false);
       DbDupValue invalidValue(nullptr);
       return std::move(invalidValue);
	   }
	
    SetupValuesTableSelectStatement(accessString);

    DbResult result = m_valuesTableSelect->Step();
    if (result == BE_SQLITE_ROW)
        return std::move(m_valuesTableSelect->GetDbValue(0));

    BeAssert(result == BE_SQLITE_DONE);

    DbDupValue invalidValue(nullptr);
    return std::move(invalidValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     08/2015
//---------------------------------------------------------------------------------------
DbDupValue ChangeSummary::Instance::GetNewValue(Utf8CP accessString) const
    {
    SetupValuesTableSelectStatement(accessString);

    DbResult result = m_valuesTableSelect->Step();
    if (result == BE_SQLITE_ROW)
        return std::move(m_valuesTableSelect->GetDbValue(1));

    BeAssert(result == BE_SQLITE_DONE);

    DbDupValue invalidValue(nullptr);
    return std::move(invalidValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     08/2015
//---------------------------------------------------------------------------------------
bool ChangeSummary::Instance::HasValue(Utf8CP accessString) const
    {
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
    ECClassId classId = (ECN::ECClassId) m_sql->GetValueInt64(0);
    ECInstanceId instanceId = m_sql->GetValueId<ECInstanceId>(1);
    DbOpcode dbOpcode = (DbOpcode) m_sql->GetValueInt(2);
    int indirect = m_sql->GetValueInt(3);

    ChangeSummary::Instance instance(m_changeSummary, classId, instanceId, dbOpcode, indirect);
    return instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::InstanceIterator::const_iterator ChangeSummary::InstanceIterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String tableName = m_changeSummary.GetInstancesTableName();
        Utf8PrintfString sql("SELECT ClassId,InstanceId,DbOpcode,Indirect FROM %s", tableName.c_str());
        Utf8String sqlString = MakeSqlString(sql.c_str());
        m_db->GetCachedStatement(m_stmt, sqlString.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_changeSummary.GetDb(), m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
int ChangeSummary::InstanceIterator::QueryCount() const
    {
    Utf8String tableName = m_changeSummary.GetInstancesTableName();
    Utf8PrintfString sql("SELECT COUNT(*) FROM %s", tableName.c_str());
    Utf8String sqlString = MakeSqlString(sql.c_str());

    CachedStatementPtr stmt;
    m_db->GetCachedStatement(stmt, sqlString.c_str());
    BeAssert(stmt.IsValid());

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

        m_stmt->BindInt64(1, m_classId);
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

    stmt->BindInt64(1, m_classId);
    stmt->BindId(2, m_instanceId);

    return ((BE_SQLITE_ROW != stmt->Step()) ? 0 : stmt->GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     08/2015
//---------------------------------------------------------------------------------------
Utf8String ChangeSummary::ConstructWhereInClause(int queryDbOpcodes) const
    {
    Utf8String whereInStr;
    if (queryDbOpcodes & QueryDbOpcode::Insert)
        {
        Utf8PrintfString addStr("%d", (int) DbOpcode::Insert);
        whereInStr.append(addStr);
        }
    if (queryDbOpcodes & QueryDbOpcode::Update)
        {
        if (!whereInStr.empty())
            whereInStr.append(",");
        Utf8PrintfString addStr("%d", (int) DbOpcode::Update);
        whereInStr.append(addStr);
        }
    if (queryDbOpcodes & QueryDbOpcode::Delete)
        {
        if (!whereInStr.empty())
            whereInStr.append(",");
        Utf8PrintfString addStr("%d", (int) DbOpcode::Delete);
        whereInStr.append(addStr);
        }

    BeAssert(!whereInStr.empty());

    return Utf8PrintfString("DbOpcode IN (%s)", whereInStr.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     08/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::QueryByClass(bmap<ECInstanceId, ChangeSummary::Instance>& changes, ECN::ECClassId classId, bool isPolymorphic /*= true*/, int queryDbOpcodes /*= QueryDbOpcode::All*/) const
    {
    if (!IsValid())
        {
        BeAssert(false);
        return;
        }

    Utf8String sql, whereClause;
    Utf8String tableName = GetInstancesTableName();
    if (isPolymorphic)
        {
        sql = Utf8PrintfString(
            " WITH RECURSIVE"
            "    DerivedClasses(ClassId) AS ("
            "        VALUES(:baseClassId)"
            "        UNION "
            "        SELECT ec_BaseClass.ClassId FROM ec_BaseClass, DerivedClasses WHERE ec_BaseClass.BaseClassId=DerivedClasses.ClassId"
            "        )"
            " SELECT ClassId,InstanceId,DbOpcode,Indirect"
            " FROM %s", tableName.c_str());
        whereClause = " WHERE ClassId IN DerivedClasses";
        }
    else
        {
        sql = Utf8PrintfString("SELECT ClassId,InstanceId,DbOpcode,Indirect FROM %s", tableName.c_str());
        }

    if (queryDbOpcodes != QueryDbOpcode::All)
        {
        Utf8String whereInClause = ConstructWhereInClause(queryDbOpcodes);
        
        if (whereClause.empty())
            whereClause.append(" WHERE ");
        else
            whereClause.append(" AND ");

        whereClause.append(whereInClause);
        }

    sql.append(whereClause);

    CachedStatementPtr stmt;
    m_ecdb.GetCachedStatement(stmt, sql.c_str());
    BeAssert(stmt.IsValid());

    int baseClassIdx = stmt->GetParameterIndex(":baseClassId");
    stmt->BindInt64(baseClassIdx, (int64_t) classId);

    DbResult result;
    while ((result = stmt->Step()) == BE_SQLITE_ROW)
        {
        ECClassId classId = (ECClassId) stmt->GetValueInt64(0);
        ECInstanceId instanceId = stmt->GetValueId<ECInstanceId>(1);
        DbOpcode dbOpcode = (DbOpcode) stmt->GetValueInt(2);
        int indirect = stmt->GetValueInt(3);

        ChangeSummary::Instance instance(m_ecdb, classId, instanceId, dbOpcode, indirect);
        changes[instanceId] = instance;
        }
    BeAssert(result == BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
// static
void ChangeSummary::RegisterSqlFunctions(ECDbR ecdb)
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
void ChangeSummary::UnregisterSqlFunctions()
    {
    if (!s_isChangedInstanceSqlFunction)
        return;

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
    ECClassId classId = (ECClassId) args[1].GetValueInt64();
    ECInstanceId instanceId = args[2].GetValueId<ECInstanceId>();

    /*
     * TODO: Instead of returning a bool we can return a change id (need to set one up)
     * Alternately, we can setup ECSQL mappings with the change table and entirely
     * avoid some of these custom functions. This needs more investigation if and when 
     * the use cases demand it. 
     */
    int res = changeSummary->HasInstance(classId, instanceId) ? 1 : 0;
    ctx.SetResultInt(res);
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
