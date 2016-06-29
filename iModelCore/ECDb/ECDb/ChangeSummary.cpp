/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ChangeSummary.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

#define CHANGED_TABLES_TEMP_PREFIX "temp."
#define CHANGED_INSTANCES_TABLE_BASE_NAME "ec_ChangedInstances"
#define CHANGED_VALUES_TABLE_BASE_NAME "ec_ChangedValues"

USING_NAMESPACE_BENTLEY_EC

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

    template <class T_Id> T_Id GetValueId(int columnIndex) const
        {
        DbValue value = GetValue(columnIndex);
        if (!value.IsValid() || value.IsNull())
            return T_Id();
        return value.GetValueId<T_Id>();
        }
};

//=======================================================================================
// @bsiclass                                              Ramanujam.Raman      07/2015
//=======================================================================================
struct TableMapDetail : NonCopyableClass
{
private:
    ECDbCR m_ecdb;
    bool m_isMapped = false;
    DbTable const* m_dbTable = nullptr;
    Utf8String m_tableName;
    bmap<Utf8String, int> m_columnIndexByName;
    bvector<ECN::ECClassId> m_fkeyRelClassIds;
    ECN::ECClassId m_primaryClassId;
    bmap<Utf8String, ChangeSummary::ColumnMap> m_columnMapByAccessString;
    ChangeSummary::ColumnMap m_emptyColumnMap;

    void Initialize(Utf8StringCR tableName);
    void Initialize(ECN::ECClassCR ecClass);
    void InitColumnIndexByName();
    void InitSystemColumnMaps();
    void InitPropertyColumnMaps(ClassMap const& classMap);
    void InitForeignKeyRelClassMaps();

    ECN::ECClassId QueryClassId() const;
    
    void AddColumnMapsForProperty(PropertyMapCR propertyMap);

public:
    TableMapDetail(ECDbCR ecdb, Utf8StringCR tableName) : m_ecdb(ecdb)
        {
        Initialize(tableName);
        }

    TableMapDetail(ECDbCR ecdb, ECN::ECClassCR ecClass) : m_ecdb(ecdb)
        {
        Initialize(ecClass);
        }

    ~TableMapDetail() {}

    DbTable const* GetDbTable() const { return m_dbTable; }
    Utf8StringCR GetTableName() const { return m_tableName; }
    bool IsMapped() const { return m_isMapped; }

    int GetColumnIndexByName(Utf8StringCR columnName) const;

    bool ContainsColumn(Utf8CP propertyAccessString) const;
    ChangeSummary::ColumnMap const& GetColumn(Utf8CP propertyAccessString) const;

    bool ContainsECClassIdColumn() const { return ContainsColumn("ECClassId"); }
    ChangeSummary::ColumnMap const& GetECClassIdColumn() const;
    ECN::ECClassId GetECClassId() const;

    ChangeSummary::ColumnMap const& GetECInstanceIdColumn() const { return GetColumn("ECInstanceId"); }

    bvector<ECN::ECClassId> const& GetMappedForeignKeyRelationshipClasses() const { return m_fkeyRelClassIds; }

    DbDupValue QueryValue(Utf8StringCR columnName, ECInstanceId instanceId) const;

    bool QueryInstance(ECInstanceId instanceId) const;
    };

//=======================================================================================
// @bsiclass                                              Ramanujam.Raman      10/2015
//=======================================================================================
struct InstancesTable : NonCopyableClass
{
private:
    ChangeSummaryCR m_changeSummary;
    ECDbCR m_ecdb;
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

    ECDbCR GetDb() const { return m_ecdb; }
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
    ECDbCR m_ecdb;
    InstancesTable const& m_instancesTable;
    Utf8String m_valuesTableNameNoPrefix;
    mutable Statement m_valuesTableInsert;

    void CreateTable();
    void ClearTable();

    void PrepareStatements();
    void FinalizeStatements();

public:
    ValuesTable(InstancesTableCR instancesTable);
    ~ValuesTable() { Free(); }

    void Initialize();
    void Free();

    Utf8String GetName() const;
    Utf8StringCR GetNameNoPrefix() const { return m_valuesTableNameNoPrefix; }

    void Insert(ECN::ECClassId classId, ECInstanceId instanceId, Utf8CP accessString, DbValue const& oldValue, DbValue const& newValue);
    void Insert(ECN::ECClassId classId, ECInstanceId instanceId, Utf8CP accessString, ECN::ECClassId oldId, ECN::ECClassId newId);
    void Insert(ECN::ECClassId classId, ECInstanceId instanceId, Utf8CP accessString, ECInstanceId oldId, ECInstanceId newId);
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

typedef bmap<Utf8String, ChangeSummary::TableMapPtr> TableMapByName;

private:
    ChangeSummaryCR m_changeSummary;
    ECDbCR m_ecdb;
    mutable TableMapByName m_tableMapByName;
    InstancesTable& m_instancesTable;
    ValuesTable& m_valuesTable;

    ExtractOption m_extractOption;
    SqlChange const* m_sqlChange;
    ChangeSummary::TableMap const* m_tableMap;

    static ClassMapCP GetClassMap(ECDbCR, ECN::ECClassId);
   
    ChangeSummary::TableMap const* GetTableMap(Utf8StringCR tableName) const;
    void AddTableToMap(Utf8StringCR tableName) const;
    void FreeTableMap();

    ECN::ECClassId GetClassIdFromChangeOrTable(Utf8CP classIdColumnName, ECInstanceId instanceId) const;
    bool ChangeAffectsClass(ClassMapCR classMap) const;
    bool ChangeAffectsProperty(PropertyMapCR propertyMap) const;
    int GetFirstColumnIndex(PropertyMapCP propertyMap) const;

    BentleyStatus ExtractFromSqlChanges(Changes& sqlChanges, ExtractOption extractOption);
    BentleyStatus ExtractFromSqlChange(SqlChange const& sqlChange, ExtractOption extractOption);

    void ExtractInstance(ClassMapCR primaryClassMap, ECInstanceId instanceId);

    void ExtractRelInstance(ClassMapCR classMap, ECInstanceId relInstanceId);
    void ExtractRelInstanceInEndTable(RelationshipClassEndTableMap const& relClassMap, ECInstanceId relInstanceId, ECN::ECClassId foreignEndClassId);
    void ExtractRelInstanceInLinkTable(RelationshipClassLinkTableMap const& relClassMap, ECInstanceId relInstanceId);
    void GetRelEndInstanceKeys(ECInstanceKey& oldInstanceKey, ECInstanceKey& newInstanceKey, RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd) const;
    ECN::ECClassId GetRelEndClassId(RelationshipClassMapCR relClassMap, ECInstanceId relInstanceId, ECN::ECRelationshipEnd relEnd, ECInstanceId endInstanceId) const;
    static ECN::ECClassId GetRelEndClassIdFromRelClass(ECN::ECRelationshipClassCP relClass, ECN::ECRelationshipEnd relEnd);

    void RecordInstance(ClassMapCR classMap, ECInstanceId instanceId, DbOpcode dbOpcode);
    void RecordRelInstance(ClassMapCR classMap, ECInstanceId instanceId, DbOpcode dbOpcode, ECInstanceKeyCR oldSourceKey, ECInstanceKeyCR newSourceKey, ECInstanceKeyCR oldTargetKey, ECInstanceKeyCR newTargetKey);
    void RecordPropertyValue(ChangeSummary::InstanceCR instance, PropertyMapCR propertyMap);
    void RecordColumnValue(ChangeSummary::InstanceCR instance, Utf8StringCR columnName, Utf8CP accessString);
    
public:
    ChangeExtractor(ChangeSummaryCR changeSummary, InstancesTableR instancesTable, ValuesTableR valuesTable) : m_changeSummary(changeSummary), m_ecdb(m_changeSummary.GetDb()), m_instancesTable(instancesTable), m_valuesTable(valuesTable) {}
    ~ChangeExtractor() { FreeTableMap(); }

    BentleyStatus ExtractFromSqlChanges(Changes& changes, bool includeRelationshipInstances);
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
void TableMapDetail::Initialize(Utf8StringCR tableName)
    {
    DbSchema const& dbSchema = m_ecdb.GetECDbImplR().GetECDbMap().GetDbSchema();
    BeAssert(dbSchema.GetLoadState() != DbSchema::LoadState::NotLoaded);

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
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void TableMapDetail::Initialize(ECN::ECClassCR ecClass)
    {
    ClassMapCP classMap = m_ecdb.GetECDbImplR().GetECDbMap().GetClassMap(ecClass);
    if (!classMap)
        {
        m_isMapped = false;
        return;
        }

    Initialize(classMap->GetPrimaryTable().GetName());
    InitPropertyColumnMaps(*classMap);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void TableMapDetail::InitColumnIndexByName()
    {
    bvector<Utf8String> columnNames;
    m_ecdb.GetColumns(columnNames, m_tableName.c_str());

    for (int ii = 0; ii < (int) columnNames.size(); ii++)
        m_columnIndexByName[columnNames[ii]] = ii;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
void TableMapDetail::InitSystemColumnMaps()
    {
    DbColumn const* instanceIdColumn = m_dbTable->GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId);
    Utf8StringCR instanceIdColumnName = instanceIdColumn->GetName();
    int instanceIdColumnIndex = GetColumnIndexByName(instanceIdColumnName);
    BeAssert(instanceIdColumnIndex >= 0);
    m_columnMapByAccessString["ECInstanceId"] = ChangeSummary::ColumnMap(instanceIdColumnName, instanceIdColumnIndex, true /* isSystemColumn */);

    DbColumn const* classIdColumn = m_dbTable->GetFilteredColumnFirst(DbColumn::Kind::ECClassId);
    if (classIdColumn->GetPersistenceType() != PersistenceType::Virtual)
        {
        Utf8StringCR classIdColumnName = classIdColumn->GetName();
        int classIdColumnIndex = GetColumnIndexByName(classIdColumnName);
        m_columnMapByAccessString["ECClassId"] = ChangeSummary::ColumnMap(classIdColumnName, classIdColumnIndex, true /* isSystemColumn */);
        }
    else
        {
        m_primaryClassId = QueryClassId();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
void TableMapDetail::InitPropertyColumnMaps(ClassMap const& classMap)
    {
    for (PropertyMapCP propertyMap : classMap.GetPropertyMaps())
        {
        // TODO: MapsToTable() doesn't seem to work
        DbTable const* table = propertyMap->GetSingleTable();
        if (!table || table->GetId() != m_dbTable->GetId())
            continue;

        AddColumnMapsForProperty(*propertyMap);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
int TableMapDetail::GetColumnIndexByName(Utf8StringCR columnName) const
    {
    bmap<Utf8String, int>::const_iterator iter = m_columnIndexByName.find(columnName);
    return (iter != m_columnIndexByName.end()) ? iter->second : -1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
bool TableMapDetail::ContainsColumn(Utf8CP propertyAccessString) const
    {
    bmap<Utf8String, ChangeSummary::ColumnMap>::const_iterator iter = m_columnMapByAccessString.find(propertyAccessString);
    return iter != m_columnMapByAccessString.end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
ChangeSummary::ColumnMap const& TableMapDetail::GetColumn(Utf8CP propertyAccessString) const
    {
    bmap<Utf8String, ChangeSummary::ColumnMap>::const_iterator iter = m_columnMapByAccessString.find(propertyAccessString);
    BeAssert(iter != m_columnMapByAccessString.end());
    return iter->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
ChangeSummary::ColumnMap const& TableMapDetail::GetECClassIdColumn() const
    {
    if (!ContainsColumn("ECClassId"))
        {
        BeAssert(false && "Table can map to multiple classes");
        return m_emptyColumnMap;
        }

    return GetColumn("ECClassId");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
ECN::ECClassId TableMapDetail::GetECClassId() const
    {
    if (ContainsColumn("ECClassId"))
        {
        BeAssert(false && "Table can map to multiple classes");
        return ECClassId();
        }

    return m_primaryClassId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
void TableMapDetail::AddColumnMapsForProperty(PropertyMapCR propertyMap)
    {
    SystemPropertyMap const* systemMap = dynamic_cast<SystemPropertyMap const*> (&propertyMap);
    if (systemMap != nullptr)
        return;

    StructPropertyMap const* inlineStructMap = dynamic_cast<StructPropertyMap const*> (&propertyMap);
    if (inlineStructMap != nullptr)
        {
        for (PropertyMapCP childPropertyMap : inlineStructMap->GetChildren())
            AddColumnMapsForProperty(*childPropertyMap);
        return;
        }

    Utf8String accessString(propertyMap.GetPropertyAccessString());
    std::vector<DbColumn const*> columns;
    propertyMap.GetColumns(columns);

    PointPropertyMap const* pointMap = dynamic_cast<PointPropertyMap const*> (&propertyMap);
    if (pointMap != nullptr)
        {
        BeAssert(columns.size() == (pointMap->Is3d() ? 3 : 2));
        for (int ii = 0; ii < (int) columns.size(); ii++)
            {
            Utf8PrintfString childAccessString("%s.%s", accessString.c_str(), (ii == 0) ? "X" : ((ii == 1) ? "Y" : "Z"));

            Utf8StringCR columnName = columns[ii]->GetName();
            int columnIndex = GetColumnIndexByName(columnName);

            m_columnMapByAccessString[childAccessString] = ChangeSummary::ColumnMap(columnName, columnIndex, false /* isSystemColumn */);
            }
        return;
        }

    // SingleColumnPropertyMap - PrimitiveArrayPropertyMap, StructArrayJsonPropertyMap
    BeAssert(columns.size() == 1);

    Utf8StringCR columnName = columns[0]->GetName();
    int columnIndex = GetColumnIndexByName(columnName);

    m_columnMapByAccessString[accessString] = ChangeSummary::ColumnMap(columnName, columnIndex, false /* isSystemColumn */);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
ECClassId TableMapDetail::QueryClassId() const
    {
    Utf8String sql;
    sql.Sprintf("SELECT DISTINCT ec_Class.Id"
                " FROM ec_Class"
                " JOIN ec_ClassMap ON ec_Class.Id = ec_ClassMap.ClassId"
                " JOIN ec_PropertyMap ON ec_ClassMap.Id = ec_PropertyMap.ClassMapId"
                " JOIN ec_Column ON ec_PropertyMap.ColumnId = ec_Column.Id"
                " JOIN ec_Table ON ec_Table.Id = ec_Column.TableId"
                " WHERE ec_Table.Name = :tableName"
                " AND (ec_ClassMap.MapStrategy != :sourceTableStrategy AND ec_ClassMap.MapStrategy != :targetTableStrategy)"
                " AND ec_Column.IsVirtual = 0"
                " AND (ec_Column.ColumnKind & %d = %d)",
                Enum::ToInt(DbColumn::Kind::ECInstanceId), Enum::ToInt(DbColumn::Kind::ECInstanceId));

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(sql.c_str());
    BeAssert(stmt.IsValid());

    stmt->BindText(stmt->GetParameterIndex(":tableName"), m_tableName.c_str(), Statement::MakeCopy::No);
    stmt->BindInt(stmt->GetParameterIndex(":sourceTableStrategy"), (int) ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable);
    stmt->BindInt(stmt->GetParameterIndex(":targetTableStrategy"), (int) ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable);

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
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void TableMapDetail::InitForeignKeyRelClassMaps()
    {
    Utf8String sql;
    sql.Sprintf("SELECT DISTINCT ec_Class.Id"
                " FROM ec_Class"
                " JOIN ec_ClassMap ON ec_Class.Id = ec_ClassMap.ClassId"
                " JOIN ec_PropertyMap ON ec_ClassMap.Id = ec_PropertyMap.ClassMapId"
                " JOIN ec_Column ON ec_PropertyMap.ColumnId = ec_Column.Id"
                " JOIN ec_Table ON ec_Table.Id = ec_Column.TableId"
                " WHERE ec_Table.Name = :tableName AND"
                "       ec_Class.Type=%d AND"
                "       (ec_ClassMap.MapStrategy = :targetTableStrategy OR ec_ClassMap.MapStrategy = :sourceTableStrategy) AND"
                "       ec_Column.IsVirtual = 0 AND"
                "       (ec_Column.ColumnKind & %d = %d )",
                Enum::ToInt(ECClassType::Relationship),
                Enum::ToInt(DbColumn::Kind::ECInstanceId), Enum::ToInt(DbColumn::Kind::ECInstanceId));

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(sql.c_str());
    BeAssert(stmt.IsValid());

    stmt->BindText(stmt->GetParameterIndex(":tableName"), m_tableName, Statement::MakeCopy::No);
    stmt->BindInt(stmt->GetParameterIndex(":sourceTableStrategy"), (int) ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable);
    stmt->BindInt(stmt->GetParameterIndex(":targetTableStrategy"), (int) ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable);

    DbResult result;
    while ((result = stmt->Step()) == BE_SQLITE_ROW)
        {
        m_fkeyRelClassIds.push_back((ECClassId) stmt->GetValueUInt64(0));
        }
    BeAssert(result == BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
DbDupValue TableMapDetail::QueryValue(Utf8StringCR columnName, ECInstanceId instanceId) const
    {
    Utf8PrintfString ecSql("SELECT %s FROM %s WHERE %s=?", columnName.c_str(), m_tableName.c_str(), GetECInstanceIdColumn().GetName().c_str());
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
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool TableMapDetail::QueryInstance(ECInstanceId instanceId) const
    {
    DbDupValue value = QueryValue(GetECInstanceIdColumn().GetName(), instanceId);
    if (!value.IsValid() || value.IsNull())
        return false;

    return value.GetValueId<ECInstanceId>().IsValid();
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
// static
ClassMapCP ChangeExtractor::GetClassMap(ECDbCR ecdb, ECClassId classId)
    {
    ECN::ECClassCP ecClass = ecdb.Schemas().GetECClass(classId);
    if (ecClass == nullptr)
        {
        BeAssert(false && "Couldn't determine the class corresponding to the change.");
        return nullptr;
        }

    ClassMapCP classMap = ecdb.GetECDbImplR().GetECDbMap().GetClassMap(*ecClass);
    return classMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
ChangeSummary::TableMapCP ChangeExtractor::GetTableMap(Utf8StringCR tableName) const
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
    ChangeSummary::TableMapPtr tableMap = ChangeSummary::TableMap::Create(m_ecdb, tableName);
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
        return m_sqlChange->GetValueId<ECClassId>(m_tableMap->GetDetail()->GetColumnIndexByName(classIdColumnName));

    /* if (dbOpcode == DbOpcode::Update) */
    return m_tableMap->QueryValueId<ECClassId>(classIdColumnName, instanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
bool ChangeExtractor::ChangeAffectsClass(ClassMapCR classMap) const
    {
    DbTable const* dbTable = m_tableMap->GetDetail()->GetDbTable();
    for (PropertyMapCP propertyMap : classMap.GetPropertyMaps())
        {
        if (propertyMap->GetType() == PropertyMap::Type::ECClassId)
            continue;

        // TODO: MapsToTable() doesn't seem to work
        DbTable const* table = propertyMap->GetSingleTable();
        if (!table || table->GetId() != dbTable->GetId())
            continue;

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
    StructPropertyMap const* inlineStructMap = dynamic_cast<StructPropertyMap const*> (&propertyMap);
    if (inlineStructMap != nullptr)
        {
        for (PropertyMapCP childPropertyMap : inlineStructMap->GetChildren())
            {
            if (ChangeAffectsProperty(*childPropertyMap))
                return true;
            }
        return false;
        }

    std::vector<DbColumn const*> columns;
    propertyMap.GetColumns(columns);

    for (DbColumn const* column : columns)
        {
        Utf8StringCR columnName = column->GetName();
        int columnIndex = m_tableMap->GetDetail()->GetColumnIndexByName(columnName);

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

    DbColumn const* firstColumn = propertyMap->GetSingleColumn();
    if (!firstColumn)
        return -1;

    return m_tableMap->GetDetail()->GetColumnIndexByName(firstColumn->GetName());
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

    ECInstanceId primaryInstanceId = m_sqlChange->GetValueId<ECInstanceId>(m_tableMap->GetECInstanceIdColumn().GetIndex());
    BeAssert(primaryInstanceId.IsValid());

    if (m_sqlChange->GetDbOpcode() == DbOpcode::Update && !m_tableMap->GetDetail()->QueryInstance(primaryInstanceId))
        {
        // Note: The instance doesn't exist anymore, and has been deleted in future change to the Db.
        // Processing updates requires that the instance is still available in the Db to extract sufficient EC information, 
        // especially since a SqlChangeSet records only the updated columns but not the entire row. 
        BeAssert(false && "SqlChangeSet does not span all modifications made to the Db");
        return ERROR;
        }

    ECClassId primaryClassId;
    if (m_tableMap->ContainsECClassIdColumn())
        primaryClassId = GetClassIdFromChangeOrTable(m_tableMap->GetECClassIdColumn().GetName().c_str(), primaryInstanceId);
    else
        primaryClassId = m_tableMap->GetECClassId();

    ClassMapCP primaryClassMap = GetClassMap(m_ecdb, primaryClassId);
    if (primaryClassMap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    DbTable const* ecDbSqlTable = m_tableMap->GetDetail()->GetDbTable();
    BeAssert(ecDbSqlTable != nullptr);

    DbTable::Type tableType = ecDbSqlTable->GetType();
    BeAssert(tableType == DbTable::Type::Primary || tableType == DbTable::Type::Existing || tableType == DbTable::Type::Joined);

    if (m_extractOption == ExtractOption::InstancesOnly && !primaryClassMap->IsRelationshipClassMap())
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
void ChangeExtractor::ExtractInstance(ClassMapCR primaryClassMap, ECInstanceId instanceId)
    {
    DbOpcode dbOpcode = m_sqlChange->GetDbOpcode();
    if (dbOpcode == DbOpcode::Update && !ChangeAffectsClass(primaryClassMap))
        return;

    RecordInstance(primaryClassMap, instanceId, dbOpcode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::ExtractRelInstance(ClassMapCR classMap, ECInstanceId relInstanceId)
    {
    ClassMap::Type type = classMap.GetType();
    if (type == ClassMap::Type::RelationshipLinkTable)
        {
        RelationshipClassLinkTableMap const* relClassMap = dynamic_cast<RelationshipClassLinkTableMap const*> (&classMap);
        BeAssert(relClassMap != nullptr);
        ExtractRelInstanceInLinkTable(*relClassMap, relInstanceId);
        return;
        }

    bvector<ECClassId> const& relClassIds = m_tableMap->GetDetail()->GetMappedForeignKeyRelationshipClasses();
    for (ECClassId relClassId : relClassIds)
        {
        RelationshipClassEndTableMap const* relClassMap = dynamic_cast<RelationshipClassEndTableMap const*> (GetClassMap(m_ecdb, relClassId));
        BeAssert(relClassMap != nullptr);

        ExtractRelInstanceInEndTable(*relClassMap, relInstanceId, classMap.GetClass().GetId());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::ExtractRelInstanceInEndTable(RelationshipClassEndTableMap const& relClassMap, ECInstanceId relInstanceId, ECN::ECClassId foreignEndClassId)
    {
    // Check that this end of the relationship matches the actual class found. 
    ECN::ECRelationshipEnd foreignEnd = relClassMap.GetForeignEnd();
    if (!relClassMap.GetConstraintMap(foreignEnd).ClassIdMatchesConstraint(foreignEndClassId))
        return;

    ECInstanceKey foreignEndInstanceKey(foreignEndClassId, relInstanceId);

    ECInstanceKey oldReferencedEndInstanceKey, newReferencedEndInstanceKey;
    ECN::ECRelationshipEnd referencedEnd = (foreignEnd == ECRelationshipEnd_Source) ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
    GetRelEndInstanceKeys(oldReferencedEndInstanceKey, newReferencedEndInstanceKey, relClassMap, relInstanceId, referencedEnd);

    if (!newReferencedEndInstanceKey.IsValid() && !oldReferencedEndInstanceKey.IsValid())
        return;

    // Check if the other end of the relationship matches the actual class found. 
    if (newReferencedEndInstanceKey.IsValid() && !relClassMap.GetConstraintMap(referencedEnd).ClassIdMatchesConstraint(newReferencedEndInstanceKey.GetECClassId()))
        return;
    if (oldReferencedEndInstanceKey.IsValid() && !relClassMap.GetConstraintMap(referencedEnd).ClassIdMatchesConstraint(oldReferencedEndInstanceKey.GetECClassId()))
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

    RecordRelInstance(relClassMap, foreignEndInstanceKey.GetECInstanceId(), relDbOpcode, *oldSourceInstanceKey, *newSourceInstanceKey, *oldTargetInstanceKey, *newTargetInstanceKey);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     07/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::ExtractRelInstanceInLinkTable(RelationshipClassLinkTableMap const& relClassMap, ECInstanceId relInstanceId)
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
    PropertyMapCP classIdPropMap = relClassMap.GetConstraintECClassIdPropMap(relEnd);
    if (!classIdPropMap)
        {
        BeAssert(false);
        return ECClassId();
        }

    DbColumn const* classIdColumn = classIdPropMap->GetSingleColumn();
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
    DbColumn const* tableClassIdColumn = nullptr;
    classIdPropMap->GetTable()->TryGetECClassIdColumn(tableClassIdColumn);

    bool endIsInOneTable = (classIdColumn == tableClassIdColumn);
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
    int classIdColumnIndex = m_tableMap->GetDetail()->GetColumnIndexByName(classIdColumnName);
    BeAssert(classIdColumnIndex >= 0);

    ECClassId classId = GetClassIdFromChangeOrTable(classIdColumnName.c_str(), relInstanceId);
    BeAssert(classId.IsValid());
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
        return ECClassId();
        }

    ECRelationshipConstraintR endConstraint = (relEnd == ECRelationshipEnd_Source) ? relClass->GetSource() : relClass->GetTarget();
    ECRelationshipConstraintClassList const& endClasses = endConstraint.GetConstraintClasses();
    if (endClasses.size() != 1)
        {
        BeAssert(false && "Multiple classes at end. Cannot pick something arbitrary");
        return ECClassId();
        }

    ECClassId classId = endClasses[0]->GetClass().GetId();
    BeAssert(classId.IsValid());
    return classId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::RecordInstance(ClassMapCR classMap, ECInstanceId instanceId, DbOpcode dbOpcode)
    {
    Utf8StringCR tableName = m_tableMap->GetTableName();
    ChangeSummary::Instance instance(m_changeSummary, classMap.GetClass().GetId(), instanceId, dbOpcode, m_sqlChange->GetIndirect(), tableName);
    m_instancesTable.InsertOrUpdate(instance);

    DbTable const* dbTable = m_tableMap->GetDetail()->GetDbTable();
    for (PropertyMapCP propertyMap : classMap.GetPropertyMaps())
        {
        // TODO: MapsToTable() doesn't seem to work
        DbTable const* table = propertyMap->GetSingleTable();
        if (!table || table->GetId() != dbTable->GetId())
            continue;

        RecordPropertyValue(instance, *propertyMap);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::RecordRelInstance(ClassMapCR classMap, ECInstanceId instanceId, DbOpcode dbOpcode, ECInstanceKeyCR oldSourceKey, ECInstanceKeyCR newSourceKey, ECInstanceKeyCR oldTargetKey, ECInstanceKeyCR newTargetKey)
    {
    RecordInstance(classMap, instanceId, dbOpcode);

    ECClassId classId = classMap.GetClass().GetId();

    m_valuesTable.Insert(classId, instanceId, "SourceECClassId", oldSourceKey.IsValid() ? oldSourceKey.GetECClassId() : ECClassId(), newSourceKey.IsValid() ? newSourceKey.GetECClassId() : ECClassId());
    m_valuesTable.Insert(classId, instanceId, "SourceECInstanceId", oldSourceKey.IsValid() ? oldSourceKey.GetECInstanceId() : ECInstanceId(), newSourceKey.IsValid() ? newSourceKey.GetECInstanceId() : ECInstanceId());
    m_valuesTable.Insert(classId, instanceId, "TargetECClassId", oldTargetKey.IsValid() ? oldTargetKey.GetECClassId() : ECClassId(), newTargetKey.IsValid() ? newTargetKey.GetECClassId() : ECClassId());
    m_valuesTable.Insert(classId, instanceId, "TargetECInstanceId", oldTargetKey.IsValid() ? oldTargetKey.GetECInstanceId() : ECInstanceId(), newTargetKey.IsValid() ? newTargetKey.GetECInstanceId() : ECInstanceId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::RecordPropertyValue(ChangeSummary::InstanceCR instance, PropertyMapCR propertyMap)
    {
    SystemPropertyMap const* systemMap = dynamic_cast<SystemPropertyMap const*> (&propertyMap);
    if (systemMap != nullptr)
        return;

    StructPropertyMap const* inlineStructMap = dynamic_cast<StructPropertyMap const*> (&propertyMap);
    if (inlineStructMap != nullptr)
        {
        for (PropertyMapCP childPropertyMap : inlineStructMap->GetChildren())
            RecordPropertyValue(instance, *childPropertyMap);
        return;
        }

    Utf8CP accessString = propertyMap.GetPropertyAccessString();
    std::vector<DbColumn const*> columns;
    propertyMap.GetColumns(columns);

    PointPropertyMap const* pointMap = dynamic_cast<PointPropertyMap const*> (&propertyMap);
    if (pointMap != nullptr)
        {
        BeAssert(columns.size() == (pointMap->Is3d() ? 3 : 2));
        for (int ii = 0; ii < (int) columns.size(); ii++)
            {
            Utf8String childAccessString;
            childAccessString.Sprintf("%s.%s", accessString, (ii == 0) ? "X" : ((ii == 1) ? "Y" : "Z"));
            Utf8StringCR columnName = columns[ii]->GetName();
            RecordColumnValue(instance, columnName, childAccessString.c_str());
            }
        return;
        }

    // SingleColumnPropertyMap - PrimitiveArrayPropertyMap, StructArrayJsonPropertyMap
    BeAssert(columns.size() == 1);
    RecordColumnValue(instance, columns[0]->GetName(), accessString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     10/2015
//---------------------------------------------------------------------------------------
void ChangeExtractor::RecordColumnValue(ChangeSummary::InstanceCR instance, Utf8StringCR columnName, Utf8CP accessString)
    {
    ECN::ECClassId classId = instance.GetClassId();
    ECInstanceId instanceId = instance.GetInstanceId();
    DbOpcode dbOpcode = instance.GetDbOpcode();

    int columnIndex = m_tableMap->GetDetail()->GetColumnIndexByName(columnName);

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
            newDupValue = m_tableMap->QueryValue(columnName, instanceId);
            newValue = DbValue(newDupValue.GetSqlValueP());
            hasNewValue = newValue.IsValid() && !newValue.IsNull();
            }
        else if (dbOpcode == DbOpcode::Delete && !hasOldValue)
            {
            oldDupValue = m_tableMap->QueryValue(columnName, instanceId);
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

    // Ensure the ECDb mapping constructs are initialized
    DbSchema& dbSchema = m_ecdb.GetECDbImplR().GetECDbMap().GetDbSchemaR();
    if (!Enum::Contains(dbSchema.GetLoadState(), DbSchema::LoadState::Core))
        {
        if (DbSchemaPersistenceManager::Load(dbSchema, m_ecdb, DbSchema::LoadState::Core) != SUCCESS)
            {
            BeAssert(false);
            return ERROR;
            }
        }

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

        for (ChangeSummary::ValueIterator::const_iterator const& vEntry : instance.MakeValueIterator(*this))
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
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
ChangeSummary::TableMap::TableMap(ECDbCR ecdb, Utf8StringCR tableName)
    {
    m_impl = new TableMapDetail(ecdb, tableName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
ChangeSummary::TableMap::TableMap(ECDbCR ecdb, ECN::ECClassCR ecClass)
    {
    m_impl = new TableMapDetail(ecdb, ecClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
ChangeSummary::TableMap::~TableMap()
    {
    delete m_impl;
    m_impl = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
bool ChangeSummary::TableMap::IsMapped() const
    {
    return m_impl->IsMapped();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
Utf8StringCR ChangeSummary::TableMap::GetTableName() const
    {
    return m_impl->GetTableName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
bool ChangeSummary::TableMap::ContainsColumn(Utf8CP propertyAccessString) const
    {
    return m_impl->ContainsColumn(propertyAccessString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
ChangeSummary::ColumnMap const& ChangeSummary::TableMap::GetColumn(Utf8CP propertyAccessString) const
    { 
    return m_impl->GetColumn(propertyAccessString); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
bool ChangeSummary::TableMap::ContainsECClassIdColumn() const
    {
    return m_impl->ContainsECClassIdColumn();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
ChangeSummary::ColumnMap const& ChangeSummary::TableMap::GetECClassIdColumn() const
    {
    return m_impl->GetECClassIdColumn();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
ECN::ECClassId ChangeSummary::TableMap::GetECClassId() const
    {
    return m_impl->GetECClassId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
ChangeSummary::ColumnMap const& ChangeSummary::TableMap::GetECInstanceIdColumn() const
    {
    return m_impl->GetECInstanceIdColumn();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     04/2016
//---------------------------------------------------------------------------------------
DbDupValue ChangeSummary::TableMap::QueryValue(Utf8StringCR columnName, ECInstanceId instanceId) const
    {
    return m_impl->QueryValue(columnName, instanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
//static 
ChangeSummary::TableMapPtr ChangeSummary::GetPrimaryTableMap(ECDbCR ecdb, ECN::ECClassCR cls)
    {
    return ChangeSummary::TableMap::Create(ecdb, cls);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
