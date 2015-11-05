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

ECDB_TYPEDEFS(TableMap);
ECDB_TYPEDEFS(SqlChange);
ECDB_TYPEDEFS(IClassMap);
ECDB_TYPEDEFS(ECDbSqlColumn);
ECDB_TYPEDEFS(PropertyMapPoint);

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                              Ramanujam.Raman      07/2015
//=======================================================================================
struct SqlChange : NonCopyableClass
{
private:
    Changes::Change const& m_sqlChange;
    Utf8String m_tableName;
    int m_indirect;
    int m_nCols;
    DbOpcode m_dbOpcode;
    bset<int> m_primaryKeyColumnIndices;

public:
    SqlChange(Changes::Change const& change);

    Changes::Change const& GetChange() const { return m_sqlChange; }
    Utf8StringCR GetTableName() const { return m_tableName; }
    DbOpcode GetDbOpcode() const { return m_dbOpcode; }
    int GetIndirect() const { return m_indirect; }
    int GetNCols() const { return m_nCols; }
    bool IsPrimaryKeyColumn(int index) const { return m_primaryKeyColumnIndices.find(index) != m_primaryKeyColumnIndices.end(); }

    void GetValues(DbValue& oldValue, DbValue& newValue, int columnIndex) const;
    void GetValueIds(ECInstanceId& oldInstanceId, ECInstanceId& newInstanceId, int idColumnIndex) const;

    DbValue GetValue(int columnIndex) const;
    int64_t GetValueInt64(int columnIndex) const;
    ECInstanceId GetValueId(int columnIndex) const;
};

//=======================================================================================
// @bsiclass                                              Ramanujam.Raman      07/2015
//=======================================================================================
struct TableMap : NonCopyableClass
{
private:
    ECDbR m_ecdb;
    Utf8String m_tableName;
    bmap<Utf8String, int> m_columnIndexByName;
    bvector<ECN::ECClassId> m_fkeyRelClassIds;
    bool m_isMapped = false;

    ECN::ECClassId m_primaryClassId = (ECN::ECClassId) - 1;
    Utf8String m_instanceIdColumnName;
    int  m_instanceIdColumnIndex = -1;
    Utf8String m_primaryClassIdColumnName;
    int  m_primaryClassIdColumnIndex = -1;

    void InitColumnIndexByName(ECDbR ecdb);
    bool QueryIdColumn(Utf8StringR idColumnName, ECDbR ecdb, Utf8StringCR tableName, int columnKind) const;
    bool QueryInstanceIdColumn(Utf8StringR idColumnName, ECDbR ecdb, Utf8StringCR tableName) const;
    bool QueryClassIdColumn(Utf8StringR idColumnName, ECDbR ecdb, Utf8StringCR tableName) const;
    ECN::ECClassId QueryClassId(ECDbR ecdb, Utf8StringCR tableName) const;
    void QueryForeignKeyRelClassIds(bvector<ECN::ECClassId>& fkeyRelClassIds, ECDbR ecdb, Utf8StringCR tableName) const;

public:
    TableMap(ECDbR ecdb) : m_ecdb(ecdb) {}
    ~TableMap() {}

    void Initialize(ECDbR ecDb, Utf8StringCR tableName);
    Utf8StringCR GetTableName() const { return m_tableName; }

    ECN::ECClassId GetPrimaryClassId() const { return m_primaryClassId; }
    bvector<ECN::ECClassId> const& GetForeignKeyRelClassIds() const { return m_fkeyRelClassIds; }

    Utf8StringCR GetInstanceIdColumnName() const { return m_instanceIdColumnName; }
    int GetInstanceIdColumnIndex() const { return m_instanceIdColumnIndex; }
    Utf8StringCR GetPrimaryClassIdColumnName() const { return m_primaryClassIdColumnName; }
    int GetPrimaryClassIdColumnIndex() const { return m_primaryClassIdColumnIndex; }

    bool GetIsMapped() const { return m_isMapped; }
    int GetColumnIndexByName(Utf8StringCR columnName) const;

    DbDupValue GetValue(Utf8StringCR columnName, ECInstanceId instanceId) const;
    int64_t GetValueInt64(Utf8StringCR columnName, ECInstanceId instanceId) const;
    bool ContainsInstance(ECInstanceId instanceId) const;
};

//=======================================================================================
// @bsiclass                                              Ramanujam.Raman      10/2015
//=======================================================================================
struct InstancesTable : NonCopyableClass
{
private:
    ChangeSummaryCR m_changeSummary;
    ECDbR m_ecdb;
    Utf8String m_instancesTableNameNoPrefix;
    mutable Statement m_instancesTableInsert;
    mutable Statement m_instancesTableUpdate;
    mutable Statement m_instancesTableSelect;

    const int m_nameSuffix;

    void CreateTable();
    void PrepareStatements();
    void FinalizeStatements();
    void ClearTable();

    void Insert(ECN::ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect, Utf8StringCR tableName);
    void Update(ECN::ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect);

public:
    InstancesTable(ChangeSummaryCR changeSummary, int nameSuffix);
    ~InstancesTable() { Free(); }

    void Initialize();
    void Free();

    ECDbR GetDb() const { return m_ecdb; }
    ChangeSummaryCR GetChangeSummary() const { return m_changeSummary; }

    Utf8String GetName() const;
    Utf8StringCR GetNameNoPrefix() const { return m_instancesTableNameNoPrefix; }
    int GetNameSuffix() const { return m_nameSuffix; }

    void InsertOrUpdate(ChangeSummary::InstanceCR changeInstance);
    ChangeSummary::Instance QueryInstance(ECN::ECClassId classId, ECInstanceId instanceId) const;
    ECN::ECClassId QueryClassId(Utf8StringCR tableName, ECInstanceId instanceId) const;
    bool ContainsInstance(ECN::ECClassId classId, ECInstanceId instanceId) const;
};

//=======================================================================================
// @bsiclass                                              Ramanujam.Raman      10/2015
//=======================================================================================
struct ValuesTable : NonCopyableClass
{
private:
    ChangeSummaryCR m_changeSummary;
    ECDbR m_ecdb;
    InstancesTableCR m_instancesTable;
    Utf8String m_valuesTableNameNoPrefix;
    mutable Statement m_valuesTableInsert;

    void CreateTable();
    void ClearTable();

    void PrepareStatements();
    void FinalizeStatements();
    void BindStatement(Statement& statement, ECN::ECClassId classId, ECInstanceId instanceId, Utf8StringCR accessString, DbValue const& oldValue, DbValue const& newValue);
    void BindStatement(Statement& statement, ECN::ECClassId classId, ECInstanceId instanceId, Utf8StringCR accessString, int64_t oldValue, int64_t newValue);

public:
    ValuesTable(InstancesTableCR instancesTable);
    ~ValuesTable() { Free(); }

    void Initialize();
    void Free();

    Utf8String GetName() const;
    Utf8StringCR GetNameNoPrefix() const { return m_valuesTableNameNoPrefix; }

    void Insert(ECN::ECClassId classId, ECInstanceId instanceId, Utf8StringCR accessString, DbValue const& oldValue, DbValue const& newValue);
    void Insert(ECN::ECClassId classId, ECInstanceId instanceId, Utf8StringCR accessString, int64_t oldValue, int64_t newValue);
};

//=======================================================================================
// @bsiclass                                              Ramanujam.Raman      10/2015
//=======================================================================================
struct ChangeExtractor : NonCopyableClass
{
enum class ExtractOption
    {
    InstancesOnly = 1,
    RelationshipInstancesOnly = 2
    };

typedef bmap<Utf8String, TableMapP> TableMapByName;

private:
    ChangeSummaryCR m_changeSummary;
    ECDbR m_ecdb;
    mutable TableMapByName m_tableMapByName;
    InstancesTableR m_instancesTable;
    ValuesTableR m_valuesTable;

    ExtractOption m_extractOption;
    SqlChangeCP m_sqlChange;
    TableMapCP m_tableMap;

    static IClassMapCP GetClassMap(ECDbR ecdb, ECN::ECClassId classId);
   
    TableMapCP GetTableMap(Utf8StringCR tableName) const;
    void AddTableToMap(Utf8StringCR tableName) const;
    void FreeTableMap();

    int64_t GetValueInt64FromChangeOrTable(Utf8StringCR columnName, ECInstanceId instanceId) const;
    ECN::ECClassId GetClassIdFromChangeOrTable(ECInstanceId instanceId) const;
    bool ChangeAffectsClass(IClassMapCR classMap) const;
    bool ChangeAffectsProperty(PropertyMapCR propertyMap) const;
    int GetFirstColumnIndex(PropertyMapCP propertyMap) const;

    BentleyStatus ExtractFromSqlChangeSet(ChangeSet& changeSet, ExtractOption extractOption);
    BentleyStatus ExtractFromSqlChange(SqlChangeCR sqlChange, ExtractOption extractOption);

    void ExtractNonInlineStructInstance(IClassMapCR primaryClassMap, ECInstanceId instanceId);
    bool GetStructArrayParentFromChange(ECClassId& parentClassId, ECInstanceId& parentInstanceId, ECInstanceId instanceId);

    void ExtractInstance(IClassMapCR primaryClassMap, ECInstanceId instanceId);

    void ExtractRelInstance(IClassMapCR classMap, ECInstanceId relInstanceId);
    void ExtractRelInstanceInEndTable(RelationshipClassEndTableMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECClassId thisEndClassId);
    void ExtractRelInstanceInLinkTable(RelationshipClassLinkTableMapCR relClassMap, ECInstanceId relInstanceId);
    void GetRelEndInstanceKeys(ECInstanceKey& oldInstanceKey, ECInstanceKey& newInstanceKey, RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd) const;
    ECN::ECClassId GetRelEndClassId(RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd, ECInstanceId endInstanceId) const;
    static ECN::ECClassId GetRelEndClassIdFromRelClass(ECN::ECRelationshipClassCP relClass, ECN::ECRelationshipEnd relEnd);

    void RecordInstance(IClassMapCR classMap, ECInstanceId instanceId, DbOpcode dbOpcode);
    void RecordRelInstance(IClassMapCR classMap, ECInstanceId instanceId, DbOpcode dbOpcode, ECInstanceKeyCR oldSourceKey, ECInstanceKeyCR newSourceKey, ECInstanceKeyCR oldTargetKey, ECInstanceKeyCR newTargetKey);
    void RecordStructArrayParentInstance(IClassMapCR classMap, ECInstanceId instanceId, DbOpcode dbOpcode);
    void RecordPropertyValue(ChangeSummary::InstanceCR instance, PropertyMapCR propertyMap);
    void RecordColumnValue(ChangeSummary::InstanceCR instance, Utf8StringCR columnName, Utf8StringCR accessString);
    
public:
    ChangeExtractor(ChangeSummaryCR changeSummary, InstancesTableR instancesTable, ValuesTableR valuesTable) : m_changeSummary(changeSummary), m_ecdb(m_changeSummary.GetDb()), m_instancesTable(instancesTable), m_valuesTable(valuesTable) {}
    ~ChangeExtractor() { FreeTableMap(); }

    BentleyStatus ExtractFromSqlChangeSet(ChangeSet& changeSet);
};


int ChangeSummary::s_count = 0;
IsChangedInstanceSqlFunction* ChangeSummary::s_isChangedInstanceSqlFunction = nullptr;

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
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
int64_t SqlChange::GetValueInt64(int columnIndex) const
    {
    DbValue value = GetValue(columnIndex);
    if (!value.IsValid() || value.IsNull())
        return -1;
    return value.GetValueInt64();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ECInstanceId SqlChange::GetValueId(int columnIndex) const
    {
    return ECInstanceId(GetValueInt64(columnIndex));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
ECClassId TableMap::QueryClassId(ECDbR ecdb, Utf8StringCR tableName) const
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement(
        "SELECT DISTINCT ec_Class.Id"
        " FROM ec_Class"
        " JOIN ec_ClassMap ON ec_Class.Id = ec_ClassMap.ClassId"
        " JOIN ec_PropertyMap ON ec_ClassMap.Id = ec_PropertyMap.ClassMapId"
        " JOIN ec_Column ON ec_PropertyMap.ColumnId = ec_Column.Id"
        " JOIN ec_Table ON ec_Table.Id = ec_Column.TableId"
        " WHERE ec_Table.Name = :tableName"
        " AND (ec_ClassMap.MapStrategy != :sourceTableStrategy AND ec_ClassMap.MapStrategy != :targetTableStrategy)"
        " AND ec_Column.IsVirtual = 0"
        " AND (ec_Column.ColumnKind & " COLUMNKIND_ECINSTANCEID_SQLVAL " = " COLUMNKIND_ECINSTANCEID_SQLVAL")");
    BeAssert(stmt.IsValid());

    stmt->BindText(stmt->GetParameterIndex(":tableName"), tableName.c_str(), Statement::MakeCopy::No);
    stmt->BindInt(stmt->GetParameterIndex(":sourceTableStrategy"), (int) ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable);
    stmt->BindInt(stmt->GetParameterIndex(":targetTableStrategy"), (int) ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable);

    DbResult result = stmt->Step();
    if (result != BE_SQLITE_ROW)
        {
        BeAssert(false);
        return -1;
        }

    ECClassId ecClassId = (ECClassId) stmt->GetValueInt64(0);
    BeAssert(ecClassId > 0);

    BeAssert(BE_SQLITE_DONE == stmt->Step()); // There should be only one primary class mapped to a table (if there is no ecClassId column)    
    return ecClassId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool TableMap::QueryInstanceIdColumn(Utf8StringR idColumnName, ECDbR ecdb, Utf8StringCR tableName) const
    {
    return QueryIdColumn(idColumnName, ecdb, tableName, 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool TableMap::QueryClassIdColumn(Utf8StringR idColumnName, ECDbR ecdb, Utf8StringCR tableName) const
    {
    return QueryIdColumn(idColumnName, ecdb, tableName, 2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool TableMap::QueryIdColumn(Utf8StringR idColumnName, ECDbR ecdb, Utf8StringCR tableName, int columnKind) const
    {
    CachedStatementPtr statement = ecdb.GetCachedStatement(
        "SELECT ec_Column.Name"
        " FROM ec_Column"
        " JOIN ec_Table ON ec_Table.Id = ec_Column.TableId"
        " WHERE ec_Table.Name = ?1 AND ec_Column.IsVirtual = 0 AND (ec_Column.ColumnKind & ?2 = ?2)");
    BeAssert(statement.IsValid());

    statement->BindText(1, tableName, Statement::MakeCopy::No);
    statement->BindInt(2, columnKind);

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
void TableMap::QueryForeignKeyRelClassIds(bvector<ECClassId>& fkeyRelClassIds, ECDbR ecdb, Utf8StringCR tableName) const
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement(
        "SELECT DISTINCT ec_Class.Id"
        " FROM ec_Class"
        " JOIN ec_ClassMap ON ec_Class.Id = ec_ClassMap.ClassId"
        " JOIN ec_PropertyMap ON ec_ClassMap.Id = ec_PropertyMap.ClassMapId"
        " JOIN ec_Column ON ec_PropertyMap.ColumnId = ec_Column.Id"
        " JOIN ec_Table ON ec_Table.Id = ec_Column.TableId"
        " WHERE ec_Table.Name = :tableName AND"
        "       ec_Class.IsRelationship = 1 AND"
        "       (ec_ClassMap.MapStrategy = :targetTableStrategy OR ec_ClassMap.MapStrategy = :sourceTableStrategy) AND"
        "       ec_Column.IsVirtual = 0 AND"
        "       (ec_Column.ColumnKind & " COLUMNKIND_ECINSTANCEID_SQLVAL " = " COLUMNKIND_ECINSTANCEID_SQLVAL")");
    BeAssert(stmt.IsValid());

    stmt->BindText(stmt->GetParameterIndex(":tableName"), tableName, Statement::MakeCopy::No);
    stmt->BindInt(stmt->GetParameterIndex(":sourceTableStrategy"), (int) ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable);
    stmt->BindInt(stmt->GetParameterIndex(":targetTableStrategy"), (int) ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable);

    DbResult result;
    while ((result = stmt->Step()) == BE_SQLITE_ROW)
        {
        fkeyRelClassIds.push_back((ECClassId) stmt->GetValueInt64(0));
        }
    BeAssert(result == BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void TableMap::Initialize(ECDbR ecdb, Utf8StringCR tableName)
    {
    m_tableName = tableName;
    InitColumnIndexByName(ecdb);

    if (!QueryInstanceIdColumn(m_instanceIdColumnName, ecdb, tableName))
        return;
    
    m_instanceIdColumnIndex = GetColumnIndexByName(m_instanceIdColumnName);
    BeAssert(m_instanceIdColumnIndex >= 0);

    if (QueryClassIdColumn(m_primaryClassIdColumnName, ecdb, tableName))
        m_primaryClassIdColumnIndex = GetColumnIndexByName(m_primaryClassIdColumnName);
    else
        m_primaryClassId = QueryClassId(ecdb, tableName);

    QueryForeignKeyRelClassIds(m_fkeyRelClassIds, ecdb, tableName);

    m_isMapped = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void TableMap::InitColumnIndexByName(ECDbR ecdb)
    {
    bvector<Utf8String> columnNames;
    ecdb.GetColumns(columnNames, m_tableName.c_str());

    for (int ii = 0; ii < (int) columnNames.size(); ii++)
        m_columnIndexByName[columnNames[ii]] = ii;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
int TableMap::GetColumnIndexByName(Utf8StringCR columnName) const
    {
    bmap<Utf8String, int>::const_iterator iter = m_columnIndexByName.find(columnName);
    return (iter != m_columnIndexByName.end()) ? iter->second : -1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
DbDupValue TableMap::GetValue(Utf8StringCR columnName, ECInstanceId instanceId) const
    {
    Utf8PrintfString ecSql("SELECT %s FROM %s WHERE %s=?", columnName.c_str(), m_tableName.c_str(), m_instanceIdColumnName.c_str());
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
int64_t TableMap::GetValueInt64(Utf8StringCR columnName, ECInstanceId instanceId) const
    {
    DbDupValue value = GetValue(columnName, instanceId);
    if (!value.IsValid() || value.IsNull())
        return -1;
    return value.GetValueInt64();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool TableMap::ContainsInstance(ECInstanceId instanceId) const
    {
    return GetValueInt64(m_instanceIdColumnName, instanceId) > 0;
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
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void InstancesTable::FinalizeStatements()
    {
    m_instancesTableInsert.Finalize();
    m_instancesTableUpdate.Finalize();
    m_instancesTableSelect.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void InstancesTable::Insert(ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect, Utf8StringCR tableName)
    {
    m_instancesTableInsert.Reset();
    m_instancesTableInsert.BindInt64(1, classId);
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
    m_instancesTableUpdate.BindInt64(1, classId);
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

    BeAssert(dbOpcode == DbOpcode::Update); // This is the only legal possibility. 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::Instance InstancesTable::QueryInstance(ECClassId classId, ECInstanceId instanceId) const
    {
    m_instancesTableSelect.Reset();
    m_instancesTableSelect.BindInt64(1, classId);
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
    m_instancesTableSelect.BindInt64(1, classId);
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
        return (ECClassId) -1;

    return (ECClassId) stmt->GetValueInt64(0);
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
void ValuesTable::Insert(ECN::ECClassId classId, ECInstanceId instanceId, Utf8StringCR accessString, DbValue const& oldValue, DbValue const& newValue)
    {
    BindStatement(m_valuesTableInsert, classId, instanceId, accessString, oldValue, newValue);

    DbResult result = m_valuesTableInsert.Step();
    BeAssert(result == BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ValuesTable::Insert(ECN::ECClassId classId, ECInstanceId instanceId, Utf8StringCR accessString, int64_t oldValue, int64_t newValue)
    {
    BindStatement(m_valuesTableInsert, classId, instanceId, accessString, oldValue, newValue);

    DbResult result = m_valuesTableInsert.Step();
    BeAssert(result == BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ValuesTable::BindStatement(Statement& statement, ECN::ECClassId classId, ECInstanceId instanceId, Utf8StringCR accessString, DbValue const& oldValue, DbValue const& newValue)
    {
    statement.Reset();
    statement.BindInt64(1, (int64_t) classId);
    statement.BindId(2, instanceId);
    statement.BindText(3, accessString.c_str(), Statement::MakeCopy::No);

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
void ValuesTable::BindStatement(Statement& statement, ECN::ECClassId classId, ECInstanceId instanceId, Utf8StringCR accessString, int64_t oldValue, int64_t newValue)
    {
    statement.Reset();
    statement.BindInt64(1, (int64_t) classId);
    statement.BindId(2, instanceId);
    statement.BindText(3, accessString.c_str(), Statement::MakeCopy::No);

    if (oldValue > 0)
        statement.BindInt64(4, oldValue);
    else
        statement.BindNull(4);

    if (newValue > 0)
        statement.BindInt64(5, newValue);
    else
        statement.BindNull(5);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
// static
IClassMapCP ChangeExtractor::GetClassMap(ECDbR ecdb, ECClassId classId)
    {
    ECN::ECClassCP ecClass = ecdb.Schemas().GetECClass(classId);
    if (ecClass == nullptr)
        {
        BeAssert(false && "Couldn't determine the class corresponding to the change.");
        return nullptr;
        }

    IClassMapCP classMap = ecdb.GetECDbImplR().GetECDbMap().GetClassMap(*ecClass);
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

    return m_tableMapByName[tableName.c_str()];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::AddTableToMap(Utf8StringCR tableName) const
    {
    TableMapP tableMap = new TableMap(m_ecdb);
    tableMap->Initialize(m_ecdb, tableName);
    m_tableMapByName[tableName.c_str()] = tableMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::FreeTableMap()
    {
    for (TableMapByName::const_iterator iter = m_tableMapByName.begin(); iter != m_tableMapByName.end(); iter++)
        delete iter->second;
    m_tableMapByName.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
int64_t ChangeExtractor::GetValueInt64FromChangeOrTable(Utf8StringCR columnName, ECInstanceId instanceId) const
    {
    DbOpcode dbOpcode = m_sqlChange->GetDbOpcode();

    if (dbOpcode == DbOpcode::Insert || dbOpcode == DbOpcode::Delete)
        return m_sqlChange->GetValueInt64(m_tableMap->GetColumnIndexByName(columnName));

    /* if (dbOpcode == DbOpcode::Update) */
    return m_tableMap->GetValueInt64(columnName, instanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ECClassId ChangeExtractor::GetClassIdFromChangeOrTable(ECInstanceId instanceId) const
    {
    ECClassId classId = m_tableMap->GetPrimaryClassId();
    if (classId > 0)
        return classId; // There is only one primary class for the table. 

    int64_t value = GetValueInt64FromChangeOrTable(m_tableMap->GetPrimaryClassIdColumnName(), instanceId);
    return (ECClassId) value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeExtractor::ChangeAffectsClass(IClassMapCR classMap) const
    {
    BeAssert(m_sqlChange->GetDbOpcode() == DbOpcode::Update);
    for (PropertyMapCP propertyMap : classMap.GetPropertyMaps())
        {
        if (ChangeAffectsProperty(*propertyMap))
            return true;
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
bool ChangeExtractor::ChangeAffectsProperty(PropertyMapCR propertyMap) const
    {
    PropertyMapToInLineStructCP inlineStructMap = dynamic_cast<PropertyMapToInLineStructCP> (&propertyMap);
    if (inlineStructMap != nullptr)
        {
        for (PropertyMapCP childPropertyMap : inlineStructMap->GetChildren())
            {
            if (ChangeAffectsProperty(*childPropertyMap))
                return true;
            }
        return false;
        }

    std::vector<ECDbSqlColumnCP> columns;
    propertyMap.GetColumns(columns);

    for (ECDbSqlColumnCP column : columns)
        {
        Utf8StringCR columnName = column->GetName();
        int columnIndex = m_tableMap->GetColumnIndexByName(columnName);

        if (m_sqlChange->IsPrimaryKeyColumn(columnIndex))
            continue;

        DbValue dbValue = m_sqlChange->GetChange().GetValue(columnIndex, Changes::Change::Stage::New);
        if (dbValue.IsValid() && !dbValue.IsNull())
            return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
int ChangeExtractor::GetFirstColumnIndex(PropertyMapCP propertyMap) const
    {
    if (!propertyMap)
        return -1;

    ECDbSqlColumnCP firstColumn = propertyMap->GetFirstColumn();
    if (!firstColumn)
        return -1;

    return m_tableMap->GetColumnIndexByName(firstColumn->GetName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeExtractor::ExtractFromSqlChangeSet(ChangeSet& changeSet)
    {
    // Pass 1
    BentleyStatus status = ExtractFromSqlChangeSet(changeSet, ExtractOption::InstancesOnly);
    if (SUCCESS != status)
        return status;

    // Pass 2
    return ExtractFromSqlChangeSet(changeSet, ExtractOption::RelationshipInstancesOnly);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeExtractor::ExtractFromSqlChangeSet(ChangeSet& changeSet, ExtractOption extractOption)
    {
    Changes changes(changeSet);
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
BentleyStatus ChangeExtractor::ExtractFromSqlChange(SqlChangeCR sqlChange, ExtractOption extractOption)
    {
    m_extractOption = extractOption;
    m_sqlChange = &sqlChange;
    m_tableMap = GetTableMap(m_sqlChange->GetTableName());
    BeAssert(m_tableMap != nullptr);

    if (!m_tableMap->GetIsMapped())
        {
        LOG.infov("ChangeSummary skipping table %s since it's not mapped", m_tableMap->GetTableName().c_str());
        return SUCCESS;
        }

    ECInstanceId primaryInstanceId = m_sqlChange->GetValueId(m_tableMap->GetInstanceIdColumnIndex());
    BeAssert(primaryInstanceId.IsValid());

    if (m_sqlChange->GetDbOpcode() == DbOpcode::Update && !m_tableMap->ContainsInstance(primaryInstanceId))
        {
        // Note: The instance doesn't exist anymore, and has been deleted in future change to the Db.
        // Processing updates requires that the instance is still available in the Db to extract sufficient EC information, 
        // especially since a SqlChangeSet records only the updated columns but not the entire row. 
        BeAssert(false && "SqlChangeSet does not span all modifications made to the Db");
        return ERROR;
        }

    ECClassId primaryClassId = GetClassIdFromChangeOrTable(primaryInstanceId);
    IClassMapCP primaryClassMap = GetClassMap(m_ecdb, primaryClassId);
    if (primaryClassMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    IClassMap::Type primaryMapType = primaryClassMap->GetClassMapType();
    BeAssert(primaryMapType != IClassMap::Type::Unmapped && "Unexpected case"); // TODO: Need to find if this is possible. 

    if (primaryMapType == IClassMap::Type::Unmapped)
        return SUCCESS;

    if (primaryMapType == IClassMap::Type::SecondaryTable)
        {
        ExtractNonInlineStructInstance(*primaryClassMap, primaryInstanceId);
        return SUCCESS;
        }

    if (primaryMapType == IClassMap::Type::Class && m_extractOption == ExtractOption::InstancesOnly)
        {
        ExtractInstance(*primaryClassMap, primaryInstanceId);
        return SUCCESS;
        }

    if (m_extractOption == ExtractOption::RelationshipInstancesOnly)
        {
        ExtractRelInstance(*primaryClassMap, primaryInstanceId);
        return SUCCESS;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::ExtractNonInlineStructInstance(IClassMapCR primaryClassMap, ECInstanceId instanceId)
    {
    // Row in struct array table - record that the parent instance has changed
    ECClassId parentClassId;
    ECInstanceId parentInstanceId;
    bool isStructArray = GetStructArrayParentFromChange(parentClassId, parentInstanceId, instanceId);
    if (isStructArray)
        {
        IClassMapCP parentClassMap = GetClassMap(m_ecdb, parentClassId);
        BeAssert(parentClassMap != nullptr);
        bool isParentRel = parentClassMap->IsRelationshipClassMap();
        if ((isParentRel && m_extractOption == ExtractOption::RelationshipInstancesOnly) || (!isParentRel && m_extractOption == ExtractOption::InstancesOnly))
            {
            RecordStructArrayParentInstance(*parentClassMap, parentInstanceId, DbOpcode::Update);
            // TODO: Record struct array values also - need to add ParentECInstanceId, ECArrayIndex columns.
            }
        return;
        }

    // Instance in struct table!!!
    if (m_extractOption == ExtractOption::InstancesOnly)
        ExtractInstance(primaryClassMap, instanceId);
    else /* if (m_extractOption == ExtractOption::RelationshipInstancesOnly) */
        ExtractRelInstance(primaryClassMap, instanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeExtractor::GetStructArrayParentFromChange(ECClassId& parentClassId, ECInstanceId& parentInstanceId, ECInstanceId instanceId)
    {
    int parentInstanceIdColumnIndex = m_tableMap->GetColumnIndexByName("ParentECInstanceId");
    if (parentInstanceIdColumnIndex < 0)
        return false;

    int propertyPathIdColumnIndex = m_tableMap->GetColumnIndexByName("ECPropertyPathId");
    BeAssert(propertyPathIdColumnIndex >= 0);

    int64_t value = GetValueInt64FromChangeOrTable("ParentECInstanceId", instanceId);
    if (value <= 0)
        return false;
    parentInstanceId = ECInstanceId(value);

    int64_t propertyPathId = GetValueInt64FromChangeOrTable("ECPropertyPathId", instanceId);
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
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::ExtractInstance(IClassMapCR primaryClassMap, ECInstanceId instanceId)
    {
    DbOpcode dbOpcode = m_sqlChange->GetDbOpcode();
    if (dbOpcode == DbOpcode::Update && !ChangeAffectsClass(primaryClassMap))
        return;

    RecordInstance(primaryClassMap, instanceId, dbOpcode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::ExtractRelInstance(IClassMapCR classMap, ECInstanceId relInstanceId)
    {
    IClassMap::Type type = classMap.GetClassMapType();
    if (type == IClassMap::Type::RelationshipLinkTable)
        {
        RelationshipClassLinkTableMapCP relClassMap = dynamic_cast<RelationshipClassLinkTableMapCP> (&classMap);
        BeAssert(relClassMap != nullptr);
        ExtractRelInstanceInLinkTable(*relClassMap, relInstanceId);
        return;
        }

    bvector<ECClassId> const& relClassIds = m_tableMap->GetForeignKeyRelClassIds();
    for (ECClassId relClassId : relClassIds)
        {
        RelationshipClassEndTableMapCP relClassMap = dynamic_cast<RelationshipClassEndTableMapCP> (GetClassMap(m_ecdb, relClassId));
        BeAssert(relClassMap != nullptr);

        ExtractRelInstanceInEndTable(*relClassMap, relInstanceId, classMap.GetClass().GetId());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::ExtractRelInstanceInEndTable(RelationshipClassEndTableMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECClassId thisEndClassId)
    {
    // Check that this end of the relationship matches the actual class found. 
    ECN::ECRelationshipEnd thisEnd = relClassMap.GetThisEnd();
    if (!relClassMap.GetConstraintMap(thisEnd).ClassIdMatchesConstraint(thisEndClassId))
        return;

    ECInstanceKey thisEndInstanceKey(thisEndClassId, relInstanceId);

    ECInstanceKey oldOtherEndInstanceKey, newOtherEndInstanceKey;
    ECN::ECRelationshipEnd otherEnd = (thisEnd == ECRelationshipEnd_Source) ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
    GetRelEndInstanceKeys(oldOtherEndInstanceKey, newOtherEndInstanceKey, relClassMap, relInstanceId, otherEnd);

    if (!newOtherEndInstanceKey.IsValid() && !oldOtherEndInstanceKey.IsValid())
        return;

    // Check if the other end of the relationship matches the actual class found. 
    if (newOtherEndInstanceKey.IsValid() && !relClassMap.GetConstraintMap(otherEnd).ClassIdMatchesConstraint(newOtherEndInstanceKey.GetECClassId()))
        return;
    if (oldOtherEndInstanceKey.IsValid() && !relClassMap.GetConstraintMap(otherEnd).ClassIdMatchesConstraint(oldOtherEndInstanceKey.GetECClassId()))
        return;

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

    RecordRelInstance(relClassMap, thisEndInstanceKey.GetECInstanceId(), relDbOpcode, *oldSourceInstanceKey, *newSourceInstanceKey, *oldTargetInstanceKey, *newTargetInstanceKey);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::ExtractRelInstanceInLinkTable(RelationshipClassLinkTableMapCR relClassMap, ECInstanceId relInstanceId)
    {
    DbOpcode dbOpcode = m_sqlChange->GetDbOpcode();
    if (dbOpcode == DbOpcode::Update && !ChangeAffectsClass(relClassMap))
        return;

    ECInstanceKey oldSourceInstanceKey, newSourceInstanceKey;
    GetRelEndInstanceKeys(oldSourceInstanceKey, newSourceInstanceKey, relClassMap, relInstanceId, ECRelationshipEnd_Source);

    ECInstanceKey oldTargetInstanceKey, newTargetInstanceKey;
    GetRelEndInstanceKeys(oldTargetInstanceKey, newTargetInstanceKey, relClassMap, relInstanceId, ECRelationshipEnd_Target);

    RecordRelInstance(relClassMap, relInstanceId, dbOpcode, oldSourceInstanceKey, newSourceInstanceKey, oldTargetInstanceKey, newTargetInstanceKey);
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
        BeAssert(newClassId > 0);
        newInstanceKey = ECInstanceKey(newClassId, newEndInstanceId);
        }

    if (oldEndInstanceId.IsValid())
        {
        ECClassId oldClassId = GetRelEndClassId(relClassMap, relInstanceId, relEnd, oldEndInstanceId);
        BeAssert(oldClassId > 0);
        oldInstanceKey = ECInstanceKey(oldClassId, oldEndInstanceId);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
ECN::ECClassId ChangeExtractor::GetRelEndClassId(RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd, ECInstanceId endInstanceId) const
    {
    PropertyMapCP classIdPropMap = relClassMap.GetConstraintECClassIdPropMap(relEnd);
    if (!classIdPropMap)
        {
        BeAssert(false);
        return (ECClassId) -1;
        }

    ECDbSqlColumnCP classIdColumn = classIdPropMap->GetFirstColumn();
    BeAssert(classIdColumn != nullptr);

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
    bool endIsInOneClass = (classIdColumn->GetPersistenceType() == PersistenceType::Virtual);
    if (endIsInOneClass)
        {
        // TODO: dynamic_cast<PropertyMapRelationshipConstraintClassId const*> (propMap)->GetDefaultConstraintECClassId()
        // should work, but doesn't for link tables - need to check with Krischan/Affan. 
        return GetRelEndClassIdFromRelClass(relClassMap.GetClass().GetRelationshipClassCP(), relEnd);
        }

    // Case #2: End is in only one table    
    ECDbSqlColumnCP tableClassIdColumn = nullptr;
    classIdPropMap->GetTable()->TryGetECClassIdColumn(tableClassIdColumn);

    bool endIsInOneTable = (classIdColumn == tableClassIdColumn);
    if (endIsInOneTable)
        {
        Utf8StringCR endTableName = classIdColumn->GetTable().GetName();

        // Search in all changes
        ECClassId classId = m_instancesTable.QueryClassId(endTableName, endInstanceId);
        if (classId > 0)
            return classId;

        // Search in the end table
        classId = (ECClassId) GetTableMap(endTableName)->GetValueInt64(classIdColumn->GetName(), endInstanceId);
        BeAssert(classId > 0);

        return classId;
        }

    // Case #3: End could be in many tables
    Utf8StringCR classIdColumnName = classIdColumn->GetName();
    int classIdColumnIndex = m_tableMap->GetColumnIndexByName(classIdColumn->GetName());
    BeAssert(classIdColumnIndex >= 0);

    ECClassId classId = (ECClassId) GetValueInt64FromChangeOrTable(classIdColumnName, relInstanceId);
    BeAssert(classId > 0);

    return classId;
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
        return (ECClassId) -1;
        }

    ECRelationshipConstraintR endConsraint = (relEnd == ECRelationshipEnd_Source) ? relClass->GetSource() : relClass->GetTarget();
    bvector<ECClassP> endClasses = endConsraint.GetClasses();
    if (endClasses.size() != 1)
        {
        BeAssert(false && "Multiple classes at end. Cannot pick something arbitrary");
        return (ECClassId) -1;
        }

    ECClassId classId = endClasses[0]->GetId();
    BeAssert(classId > 0);

    return classId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::RecordInstance(IClassMapCR classMap, ECInstanceId instanceId, DbOpcode dbOpcode)
    {
    ChangeSummary::Instance instance(m_changeSummary, classMap.GetClass().GetId(), instanceId, dbOpcode, m_sqlChange->GetIndirect(), m_tableMap->GetTableName());
    m_instancesTable.InsertOrUpdate(instance);

    for (PropertyMapCP propertyMap : classMap.GetPropertyMaps())
        RecordPropertyValue(instance, *propertyMap);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::RecordRelInstance(IClassMapCR classMap, ECInstanceId instanceId, DbOpcode dbOpcode, ECInstanceKeyCR oldSourceKey, ECInstanceKeyCR newSourceKey, ECInstanceKeyCR oldTargetKey, ECInstanceKeyCR newTargetKey)
    {
    RecordInstance(classMap, instanceId, dbOpcode);

    ECClassId classId = classMap.GetClass().GetId();

    m_valuesTable.Insert(classId, instanceId, "SourceECClassId", oldSourceKey.IsValid() ? oldSourceKey.GetECClassId() : -1, newSourceKey.IsValid() ? newSourceKey.GetECClassId() : -1);
    m_valuesTable.Insert(classId, instanceId, "SourceECInstanceId", oldSourceKey.IsValid() ? oldSourceKey.GetECInstanceId().GetValue() : -1, newSourceKey.IsValid() ? newSourceKey.GetECInstanceId().GetValue() : -1);
    m_valuesTable.Insert(classId, instanceId, "TargetECClassId", oldTargetKey.IsValid() ? oldTargetKey.GetECClassId() : -1, newTargetKey.IsValid() ? newTargetKey.GetECClassId() : -1);
    m_valuesTable.Insert(classId, instanceId, "TargetECInstanceId", oldTargetKey.IsValid() ? oldTargetKey.GetECInstanceId().GetValue() : -1, newTargetKey.IsValid() ? newTargetKey.GetECInstanceId().GetValue() : -1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::RecordStructArrayParentInstance(IClassMapCR classMap, ECInstanceId instanceId, DbOpcode dbOpcode)
    {
    ChangeSummary::Instance instance(m_changeSummary, classMap.GetClass().GetId(), instanceId, dbOpcode, m_sqlChange->GetIndirect(), m_tableMap->GetTableName());
    m_instancesTable.InsertOrUpdate(instance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::RecordPropertyValue(ChangeSummary::InstanceCR instance, PropertyMapCR propertyMap)
    {
    PropertyMapToInLineStructCP inlineStructMap = dynamic_cast<PropertyMapToInLineStructCP> (&propertyMap);
    if (inlineStructMap != nullptr)
        {
        for (PropertyMapCP childPropertyMap : inlineStructMap->GetChildren())
            RecordPropertyValue(instance, *childPropertyMap);
        return;
        }

    Utf8String accessString = propertyMap.GetPropertyAccessString();
    std::vector<ECDbSqlColumnCP> columns;
    propertyMap.GetColumns(columns);

    PropertyMapPointCP pointMap = dynamic_cast<PropertyMapPointCP> (&propertyMap);
    if (pointMap != nullptr)
        {
        BeAssert(columns.size() == (pointMap->Is3d() ? 3 : 2));
        for (int ii = 0; ii < columns.size(); ii++)
            {
            Utf8PrintfString childAccessString("%s.%s", accessString.c_str(), (ii == 0) ? "X" : ((ii == 1) ? "Y" : "Z"));
            Utf8StringCR columnName = columns[ii]->GetName();
            RecordColumnValue(instance, columnName, childAccessString);
            }
        return;
        }

    PropertyMapSystem const* systemMap = dynamic_cast<PropertyMapSystem const*> (&propertyMap);
    if (systemMap != nullptr)
        return;

    // PropertyMapToColumn, PropertyMapArrayOfPrimitives
    if (columns.size() == 1)
        {
        RecordColumnValue(instance, columns[0]->GetName(), accessString);
        return;
        }

    BeAssert(nullptr != dynamic_cast<PropertyMapToTable const*> (&propertyMap));
    BeAssert(columns.size() == 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::RecordColumnValue(ChangeSummary::InstanceCR instance, Utf8StringCR columnName, Utf8StringCR accessString)
    {
    ECN::ECClassId classId = instance.GetClassId();
    ECInstanceId instanceId = instance.GetInstanceId();
    DbOpcode dbOpcode = instance.GetDbOpcode();

    int columnIndex = m_tableMap->GetColumnIndexByName(columnName);

    DbValue oldValue(nullptr), newValue(nullptr);
    m_sqlChange->GetValues(oldValue, newValue, columnIndex);

    bool hasOldValue = oldValue.IsValid() && !oldValue.IsNull();
    bool hasNewValue = newValue.IsValid() && !newValue.IsNull();

    DbDupValue oldDupValue(nullptr), newDupValue(nullptr); // declared early 'cos these need to be held in memory
    if (dbOpcode != m_sqlChange->GetDbOpcode())
        {
        /*
        * Note: In the case of FKEY relationships, an insert or delete can be caused by an update
        * to the table. In these cases, all the old or new values necessary may not be part of
        * change record since update records only changed values.
        * We make an attempt to retrieve these values from the current state of the Db.
        */
        BeAssert(dbOpcode == DbOpcode::Insert || dbOpcode == DbOpcode::Delete);
        BeAssert(m_sqlChange->GetDbOpcode() == DbOpcode::Update);

        if (dbOpcode == DbOpcode::Insert && !hasNewValue)
            {
            newDupValue = std::move(m_tableMap->GetValue(columnName, instanceId));
            newValue = DbValue(newDupValue.GetSqlValueP());
            hasNewValue = newValue.IsValid() && !newValue.IsNull();
            }
        else if (dbOpcode == DbOpcode::Delete && !hasOldValue)
            {
            oldDupValue = std::move(m_tableMap->GetValue(columnName, instanceId));
            oldValue = DbValue(oldDupValue.GetSqlValueP());
            hasOldValue = oldValue.IsValid() && !oldValue.IsNull();
            }
        }

    if (hasOldValue || hasNewValue) // Do not persist entirely empty fields
        m_valuesTable.Insert(classId, instanceId, accessString, oldValue, newValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     09/2015
//---------------------------------------------------------------------------------------
ChangeSummary::ChangeSummary(ECDbR ecdb) : m_ecdb(ecdb)
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
    Free();
    s_count--;
    if (s_count == 0)
        UnregisterSqlFunctions();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummary::FromSqlChangeSet(ChangeSet& changeSet)
    {
    Initialize();
    return m_changeExtractor->ExtractFromSqlChangeSet(changeSet);
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
    return m_valuesTable->GetName();
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
    BeAssert(IsValid());

	if (IsValid())
	   {
       SetupValuesTableSelectStatement(accessString);
       DbResult result = m_valuesTableSelect->Step();
       if (result == BE_SQLITE_ROW)
           return std::move(m_valuesTableSelect->GetDbValue(0));
       BeAssert(result == BE_SQLITE_DONE);
	   }
	
    DbDupValue invalidValue(nullptr);
    return std::move(invalidValue);
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
            return std::move(m_valuesTableSelect->GetDbValue(1));
        BeAssert(result == BE_SQLITE_DONE);
        }

    DbDupValue invalidValue(nullptr);
    return std::move(invalidValue);
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
    ECClassId classId = (ECN::ECClassId) m_sql->GetValueInt64(0);
    ECInstanceId instanceId = m_sql->GetValueId<ECInstanceId>(1);
    DbOpcode dbOpcode = (DbOpcode) m_sql->GetValueInt(2);
    int indirect = m_sql->GetValueInt(3);
    Utf8String tableName = m_sql->GetValueText(4);

    ChangeSummary::Instance instance(m_changeSummary, classId, instanceId, dbOpcode, indirect, tableName);
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
        Utf8PrintfString sql("SELECT ClassId,InstanceId,DbOpcode,Indirect,TableName FROM %s", tableName.c_str());
        Utf8String sqlString = MakeSqlString(sql.c_str());
        m_db->GetCachedStatement(m_stmt, sqlString.c_str());
        m_params.Bind(*m_stmt);
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
    Utf8String instancesTableName = GetInstancesTableName();
    if (isPolymorphic)
        {
        sql = Utf8PrintfString(
            " WITH RECURSIVE"
            "    DerivedClasses(ClassId) AS ("
            "        VALUES(:baseClassId)"
            "        UNION "
            "        SELECT ec_BaseClass.ClassId FROM ec_BaseClass, DerivedClasses WHERE ec_BaseClass.BaseClassId=DerivedClasses.ClassId"
            "        )"
            " SELECT ClassId,InstanceId,DbOpcode,Indirect,TableName"
            " FROM %s", instancesTableName.c_str());
        whereClause = " WHERE ClassId IN DerivedClasses";
        }
    else
        {
        sql = Utf8PrintfString("SELECT ClassId,InstanceId,DbOpcode,Indirect,TableName FROM %s", instancesTableName.c_str());
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
        Utf8String tableName = stmt->GetValueText(4);

        ChangeSummary::Instance instance(*this, classId, instanceId, dbOpcode, indirect, tableName);
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
    int res = changeSummary->ContainsInstance(classId, instanceId) ? 1 : 0;
    ctx.SetResultInt(res);
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
