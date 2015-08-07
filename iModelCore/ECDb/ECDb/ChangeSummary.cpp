/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ChangeSummary.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

#define CHANGE_SUMMARY_TABLE_PREFIX "temp."
#define CHANGE_SUMMARY_TABLE_NAME_NOPREFIX "ec_ChangeSummary"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

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
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::ClassMap::AddColumn(Utf8CP accessString, Utf8CP columnName, int columnIndex)
    {
    ColumnMap columnMap(accessString, columnName, columnIndex);
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
        "SELECT ec_Column.Name, ec_PropertyPath.AccessString"
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

        bmap<Utf8String, int>::const_iterator iter = columnIndexByName.find(columnName);
        BeAssert(iter != columnIndexByName.end());
        AddColumn(accessString, columnName, iter->second);
        }
    BeAssert(BE_SQLITE_DONE == result);
    
    // Struct arrays require special treatment - add additional columns to the map
    if (m_isStruct && columnIndexByName.find("ParentECInstanceId") != columnIndexByName.end())
        {
        bmap<Utf8String, int>::const_iterator iter;

        iter = columnIndexByName.find("ParentECInstanceId");
        BeAssert(iter != columnIndexByName.end());
        AddColumn("ParentECInstanceId", "ParentECInstanceId", iter->second);

        iter = columnIndexByName.find("ECPropertyPathId");
        BeAssert(iter != columnIndexByName.end());
        AddColumn("ECPropertyPathId", "ECPropertyPathId", iter->second);

        iter = columnIndexByName.find("ECArrayIndex");
        BeAssert(iter != columnIndexByName.end());
        AddColumn("ECArrayIndex", "ECArrayIndex", iter->second);
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
        " WHERE ec_Table.Name = ? AND ec_Column.IsVirtual = 0 AND ec_Column.UserData = ?");
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
        " WHERE ec_Table.Name = ? AND ec_Column.IsVirtual = 0 AND ec_Column.UserData = 1");
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
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::Initialize()
    {
    if (IsValid())
        return;

    CreateChangeSummaryTable();
    PrepareChangeSummaryTableStatements();
    m_isValid = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::Free()
    {
    if (!IsValid())
        return;

    FinalizeChangeSummaryTableStatements();
    DestroyChangeSummaryTable();
    m_isValid = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::~ChangeSummary()
    {
    Free();
    FreeTableMap();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
Utf8String ChangeSummary::GetChangeSummaryTableName() const
    {
    Utf8String tableName = CHANGE_SUMMARY_TABLE_PREFIX CHANGE_SUMMARY_TABLE_NAME_NOPREFIX;
    return tableName;
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
bool ChangeSummary::SqlChangeHasUpdatesForClass(SqlChange const& sqlChange, ChangeSummary::ClassMap const& classMap) const
    {
    ColumnMapByAccessString const& columnMapByAccessString = classMap.GetColumnMapByAccessString();
    for (ColumnMapByAccessString::const_iterator it = columnMapByAccessString.begin(); it != columnMapByAccessString.end(); it++)
        {
        ColumnMap const& columnMap = it->second;

        int columnIndex = columnMap.GetColumnIndex();
        Utf8CP accessString = columnMap.GetAccessString();

        if (0 == ::strcmp(accessString, "ECInstanceId"))
            continue;

        DbValue dbValue = sqlChange.GetChange().GetValue(columnIndex, Changes::Change::Stage::Old);
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

    ECInstanceId otherEndOldId;
    if (dbOpcode == DbOpcode::Delete || dbOpcode == DbOpcode::Update)
        {
        DbValue oldDbVal = sqlChange.GetChange().GetValue(otherEndIdColumnIndex, Changes::Change::Stage::Old);
        if (oldDbVal.IsValid() && !oldDbVal.IsNull())
            otherEndOldId = oldDbVal.GetValueId<ECInstanceId>();
        }

    ECInstanceId otherEndNewId;
    if (dbOpcode == DbOpcode::Insert || dbOpcode == DbOpcode::Update)
        {
        DbValue newDbVal = sqlChange.GetChange().GetValue(otherEndIdColumnIndex, Changes::Change::Stage::New);
        if (newDbVal.IsValid() && !newDbVal.IsNull())
            otherEndNewId = newDbVal.GetValueId<ECInstanceId>();
        }

    DbOpcode relDbOpcode;
    if (!otherEndOldId.IsValid() && otherEndNewId.IsValid())
        relDbOpcode = DbOpcode::Insert;
    else if (otherEndOldId.IsValid() && !otherEndNewId.IsValid())
        relDbOpcode = DbOpcode::Delete;
    else if (otherEndOldId.IsValid() && otherEndNewId.IsValid() && SqlChangeHasUpdatesForClass(sqlChange, classMap))
        relDbOpcode = DbOpcode::Update;
    else
        return;

    ECChange ecChange(classMap.GetClassId(), instanceId, relDbOpcode, sqlChange.GetIndirect());
    RecordInChangeSummaryTable(ecChange);
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
        ECChange ecChange(parentClassId, parentInstanceId, DbOpcode::Update, sqlChange.GetIndirect());
        RecordInChangeSummaryTable(ecChange);
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
    ECChange ecChange(classMap.GetClassId(), instanceId, dbOpcode, sqlChange.GetIndirect());

    if (dbOpcode == DbOpcode::Insert || dbOpcode == DbOpcode::Delete)
        RecordInChangeSummaryTable(ecChange);
    else if (SqlChangeHasUpdatesForClass(sqlChange, classMap))
        RecordInChangeSummaryTable(ecChange);
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
void ChangeSummary::CreateChangeSummaryTable()
    {
    BeAssert(!m_ecdb.TableExists(CHANGE_SUMMARY_TABLE_PREFIX CHANGE_SUMMARY_TABLE_NAME_NOPREFIX));

    DbResult result = m_ecdb.CreateTable(CHANGE_SUMMARY_TABLE_PREFIX CHANGE_SUMMARY_TABLE_NAME_NOPREFIX, "[Id] INTEGER NOT NULL PRIMARY KEY, [ClassId] INTEGER not null, [InstanceId] INTEGER not null, [DbOpcode] INTEGER not null, [IsIndirect] INTEGER not null");
    BeAssert(result == BE_SQLITE_OK);

    Utf8CP sqlIdx1 = "CREATE UNIQUE INDEX " CHANGE_SUMMARY_TABLE_PREFIX "idx_" CHANGE_SUMMARY_TABLE_NAME_NOPREFIX "_id ON [" CHANGE_SUMMARY_TABLE_NAME_NOPREFIX "] (ClassId, InstanceId)";
    result = m_ecdb.ExecuteSql(sqlIdx1);
    BeAssert(result == BE_SQLITE_OK);

    Utf8CP sqlIdx2 = "CREATE INDEX " CHANGE_SUMMARY_TABLE_PREFIX "idx_" CHANGE_SUMMARY_TABLE_NAME_NOPREFIX "_op ON [" CHANGE_SUMMARY_TABLE_NAME_NOPREFIX "](DbOpcode)";
    result = m_ecdb.ExecuteSql(sqlIdx2);
    BeAssert(result == BE_SQLITE_OK);

    Utf8CP sqlIdx3 = "CREATE INDEX " CHANGE_SUMMARY_TABLE_PREFIX "idx_" CHANGE_SUMMARY_TABLE_NAME_NOPREFIX "_ind ON [" CHANGE_SUMMARY_TABLE_NAME_NOPREFIX "](IsIndirect)";
    result = m_ecdb.ExecuteSql(sqlIdx3);
    BeAssert(result == BE_SQLITE_OK);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::DestroyChangeSummaryTable()
    {
    BeAssert(m_ecdb.TableExists(CHANGE_SUMMARY_TABLE_PREFIX CHANGE_SUMMARY_TABLE_NAME_NOPREFIX));

    Utf8CP sql = "DROP TABLE " CHANGE_SUMMARY_TABLE_PREFIX CHANGE_SUMMARY_TABLE_NAME_NOPREFIX;
    DbResult result = m_ecdb.ExecuteSql(sql);
    BeAssert(result == BE_SQLITE_OK);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::PrepareChangeSummaryTableStatements()
    {
    Utf8String changeSummaryTableName = GetChangeSummaryTableName();
    BeAssert(m_ecdb.TableExists(changeSummaryTableName.c_str()));

    DbResult result;

    BeAssert(!m_changeSummaryTableInsert.IsPrepared());
    Utf8PrintfString insertSql("INSERT INTO %s (ClassId,InstanceId,DbOpcode,IsIndirect) VALUES(?1,?2,?3,?4)", changeSummaryTableName.c_str());
    result = m_changeSummaryTableInsert.Prepare(m_ecdb, insertSql.c_str());
    BeAssert(result == BE_SQLITE_OK);

    BeAssert(!m_changeSummaryTableUpdate.IsPrepared());
    Utf8PrintfString updateSql("UPDATE %s SET DbOpcode=?3,IsIndirect=?4 WHERE ClassId=?1 AND InstanceId=?2", changeSummaryTableName.c_str());
    result = m_changeSummaryTableUpdate.Prepare(m_ecdb, updateSql.c_str());
    BeAssert(result == BE_SQLITE_OK);

    BeAssert(!m_changeSummaryTableSelect.IsPrepared());
    Utf8PrintfString selectSql("SELECT DbOpcode, IsIndirect FROM %s WHERE ClassId=?1 AND InstanceId=?2", changeSummaryTableName.c_str());
    result = m_changeSummaryTableSelect.Prepare(m_ecdb, selectSql.c_str());
    BeAssert(result == BE_SQLITE_OK);

    BeAssert(!m_changeSummaryTableDelete.IsPrepared());
    Utf8PrintfString deleteSql("DELETE FROM %s WHERE ClassId=?1 AND InstanceId=?2", changeSummaryTableName.c_str());
    result = m_changeSummaryTableDelete.Prepare(m_ecdb, deleteSql.c_str());
    BeAssert(result == BE_SQLITE_OK);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::FinalizeChangeSummaryTableStatements()
    {
    m_changeSummaryTableInsert.Finalize();
    m_changeSummaryTableUpdate.Finalize();
    m_changeSummaryTableSelect.Finalize();
    m_changeSummaryTableDelete.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::RecordInChangeSummaryTable(ECChange const& ecChange)
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

    ECClassId classId = ecChange.GetECClassId();
    ECInstanceId instanceId = ecChange.GetECInstanceId();
    DbOpcode dbOpcode = ecChange.GetDbOpcode();
    int indirect = ecChange.GetIndirect();
    
    DbOpcode foundDbOpcode;
    bool found = SelectInChangeSummaryTable(foundDbOpcode, classId, instanceId);

    if (!found)
        {
        InsertInChangeSummaryTable(classId, instanceId, dbOpcode, indirect);
        return;
        }

    if (foundDbOpcode == DbOpcode::Update && dbOpcode != DbOpcode::Update)
        {
        UpdateInChangeSummaryTable(classId, instanceId, dbOpcode, indirect);
        return;
        }

    BeAssert(dbOpcode == DbOpcode::Update); // This is the only legal possibility. 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeSummary::SelectInChangeSummaryTable(DbOpcode& dbOpcode, ECClassId classId, ECInstanceId instanceId)
    {
    BindChangeSummaryTableStatement(m_changeSummaryTableSelect, classId, instanceId);

    DbResult result = m_changeSummaryTableSelect.Step();

    if (result == BE_SQLITE_ROW)
        {
        dbOpcode = (DbOpcode) m_changeSummaryTableSelect.GetValueInt(0);
        return true;
        }

    BeAssert(result == BE_SQLITE_DONE);
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::InsertInChangeSummaryTable(ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect)
    {
    BindChangeSummaryTableStatement(m_changeSummaryTableInsert, classId, instanceId, dbOpcode, indirect);

    DbResult result = m_changeSummaryTableInsert.Step();
    BeAssert(result == BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::DeleteInChangeSummaryTable(ECClassId classId, ECInstanceId instanceId)
    {
    BindChangeSummaryTableStatement(m_changeSummaryTableDelete, classId, instanceId);

    DbResult result = m_changeSummaryTableDelete.Step();
    BeAssert(result == BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::UpdateInChangeSummaryTable(ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect)
    {
    BindChangeSummaryTableStatement(m_changeSummaryTableUpdate, classId, instanceId, dbOpcode, indirect);

    DbResult result = m_changeSummaryTableUpdate.Step();
    BeAssert(result == BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::BindChangeSummaryTableStatement(Statement& statement, ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect)
    {
    BindChangeSummaryTableStatement(statement, classId, instanceId);
    statement.BindInt(3, (int) dbOpcode);
    statement.BindInt(4, indirect);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeSummary::BindChangeSummaryTableStatement(Statement& statement, ECClassId classId, ECInstanceId instanceId)
    {
    statement.Reset();
    statement.BindInt64(1, classId);
    statement.BindId(2, instanceId);
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
        
    printf("InstanceId;ClassId;ClassName;DbOpcode;IsIndirect\n");

    for (ChangeSummary::Iterator::const_iterator const& entry : MakeIterator())
        {
        ECClassId classId = entry.GetClassId();

        ECN::ECClassCP ecClass = m_ecdb.Schemas().GetECClass(classId);
        BeAssert(ecClass != nullptr);
        Utf8String className(ecClass->GetFullName());

        DbOpcode opCode = entry.GetDbOpcode();
        Utf8String opCodeStr;
        if (opCode == DbOpcode::Insert)
            opCodeStr = "Insert";
        else if (opCode == DbOpcode::Update)
            opCodeStr = "Update";
        else /* if (opCode = DbOpcode::Delete) */
            opCodeStr = "Delete";

        printf("%" PRId64 ";%" PRId64 ";%s;%s;%s\n", entry.GetInstanceId().GetValueUnchecked(), (int64_t) classId, className.c_str(), opCodeStr.c_str(), entry.GetIsIndirect() ? "Yes" : "No");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeSummary::GetColumnValueFromChangeOrTable(int64_t& value, SqlChange const& sqlChange, Utf8CP tableName, Utf8CP columnName, int columnIndex, Utf8CP instanceIdColumnName, ECInstanceId instanceId) const
    {
    if (columnIndex < 0)
        return false;

    DbOpcode dbOpcode = sqlChange.GetDbOpcode();

    if (dbOpcode == DbOpcode::Insert || dbOpcode == DbOpcode::Delete)
        return GetColumnValueFromChange(value, sqlChange, columnIndex);

    /* if (dbOpcode == DbOpcode::Update) */
    return QueryColumnValueFromTable(value, tableName, columnName, instanceIdColumnName, instanceId);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeSummary::GetColumnValueFromChange(int64_t& value, SqlChange const& sqlChange, int columnIndex) const
    {
    DbValue dbValue = sqlChange.GetChange().GetValue(columnIndex, (sqlChange.GetDbOpcode() == DbOpcode::Insert) ? Changes::Change::Stage::New : Changes::Change::Stage::Old);
    if (!dbValue.IsValid() || dbValue.IsNull())
        return false;

    value = dbValue.GetValueInt64();
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeSummary::QueryColumnValueFromTable(int64_t& value, Utf8CP tableName, Utf8CP columnName, Utf8CP instanceIdColumnName, ECInstanceId instanceId) const
    {
    Utf8PrintfString ecSql("SELECT %s FROM %s WHERE %s=?", columnName, tableName, instanceIdColumnName);
    CachedStatementPtr statement = m_ecdb.GetCachedStatement(ecSql.c_str());
    BeAssert(statement.IsValid());

    statement->BindId(1, instanceId);

    DbResult result = statement->Step();

    if (BE_SQLITE_ROW == result)
        {
        value = statement->GetValueInt64(0);
        return true;
        }

    BeAssert(result == BE_SQLITE_DONE);
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeSummary::IsInstanceDeleted(ECInstanceId instanceId, TableMap const& tableMap) const
    {
    int64_t dummyId;
    bool found = QueryColumnValueFromTable(dummyId, tableMap.GetTableName(), tableMap.GetECInstanceIdColumnName(), tableMap.GetECInstanceIdColumnName(), instanceId);
    return !found;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ECClassId ChangeSummary::GetClassIdFromChangeOrTable(SqlChange const& sqlChange, TableMap const& tableMap, ECInstanceId instanceId) const
    {
    int64_t value = -1;
    GetColumnValueFromChangeOrTable(value, sqlChange, tableMap.GetTableName(), tableMap.GetECClassIdColumnName(), tableMap.GetECClassIdColumnIndex(), tableMap.GetECInstanceIdColumnName(), instanceId);
    return (ECClassId) value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ECInstanceId ChangeSummary::GetInstanceIdFromChange(SqlChange const& sqlChange, TableMap const& tableMap) const
    {
    int64_t value;
    GetColumnValueFromChange(value, sqlChange, tableMap.GetECInstanceIdColumnIndex());
    return ECInstanceId(value);
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

    int64_t value = -1;
    GetColumnValueFromChangeOrTable(value, sqlChange, classMap.GetTableName(), classMap.GetColumnName("ParentECInstanceId"), parentECInstanceIdColumnIndex, ecInstanceIdColumnName, instanceId);
    if (value <= 0)
        return false;
    parentInstanceId = ECInstanceId(value);

    int64_t propertyPathId = -1;
    GetColumnValueFromChangeOrTable(propertyPathId, sqlChange, classMap.GetTableName(), classMap.GetColumnName("ECPropertyPathId"), ecPropertyPathIdColumnIndex, ecInstanceIdColumnName, instanceId);
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
ChangeSummary::Iterator::const_iterator ChangeSummary::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString("SELECT ClassId,InstanceId,DbOpcode,IsIndirect FROM " CHANGE_SUMMARY_TABLE_PREFIX CHANGE_SUMMARY_TABLE_NAME_NOPREFIX);
        m_db->GetCachedStatement(m_stmt, sqlString.c_str());
        m_params.Bind(*m_stmt);
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
int ChangeSummary::Iterator::QueryCount() const
    {
    Utf8String sqlString = MakeSqlString("SELECT COUNT(*) FROM " CHANGE_SUMMARY_TABLE_PREFIX CHANGE_SUMMARY_TABLE_NAME_NOPREFIX);

    Statement sql;
    sql.Prepare(*m_db, sqlString.c_str());

    return ((BE_SQLITE_ROW != sql.Step()) ? 0 : sql.GetValueInt(0));
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
