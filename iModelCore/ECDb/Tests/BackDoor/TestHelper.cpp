/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/TestHelper.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/TestHelper.h"
#include "PublicAPI/BackDoor/ECDb/ECDbTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

#define SQL_SELECTCLAUSE_ecColumn "c.Name,c.Type,c.IsVirtual,c.NotNullConstraint,c.UniqueConstraint,c.CheckConstraint,c.DefaultConstraint,c.CollationConstraint,c.ColumnKind,c.OrdinalInPrimaryKey"

BEGIN_ECDBUNITTESTS_NAMESPACE

//************************************************************************************
// TestHelper
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus TestHelper::RunSchemaImport(SchemaItem const& schema, Utf8CP fileName /*= nullptr*/)
    {
    ECDb ecdb;
    if (BE_SQLITE_OK != ECDbTestFixture::CreateECDb(ecdb, fileName))
        return ERROR;

    return TestHelper(ecdb).ImportSchema(schema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus TestHelper::RunSchemaImport(std::vector<SchemaItem> const& schemas, Utf8CP fileName /*= nullptr*/)
    {
    ECDb ecdb;
    if (BE_SQLITE_OK != ECDbTestFixture::CreateECDb(ecdb, fileName))
        return ERROR;

    return TestHelper(ecdb).ImportSchemas(schemas);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestHelper::ImportSchemas(std::vector<SchemaItem> const& schemas) const
    {
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    //not all ECDb schemas are included in the ECDb file by default. So add the path to the ECDb
    //XML files to the search paths
    BeFileName ecdbSchemaSearchPath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(ecdbSchemaSearchPath);
    ecdbSchemaSearchPath.AppendToPath(L"ECSchemas").AppendToPath(L"ECDb");
    context->AddSchemaPath(ecdbSchemaSearchPath);

    context->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    for (SchemaItem const& schema : schemas)
        {
        if (SUCCESS != ECDbTestFixture::ReadECSchema(context, m_ecdb, schema))
            return ERROR;
        }

    Savepoint sp(const_cast<ECDb&>(m_ecdb), "ECSchema Import");
    if (SUCCESS == m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()))
        {
        sp.Commit();
        return SUCCESS;
        }

    sp.Cancel();
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestHelper::ImportSchema(SchemaItem const& testItem) const
    {
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();

    //not all ECDb schemas are included in the ECDb file by default. So add the path to the ECDb
    //XML files to the search paths
    BeFileName ecdbSchemaSearchPath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(ecdbSchemaSearchPath);
    ecdbSchemaSearchPath.AppendToPath(L"ECSchemas").AppendToPath(L"ECDb");
    context->AddSchemaPath(ecdbSchemaSearchPath);

    context->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    if (SUCCESS != ECDbTestFixture::ReadECSchema(context, m_ecdb, testItem))
        return ERROR;

    Savepoint sp(const_cast<ECDb&>(m_ecdb), "ECSchema Import");
    if (SUCCESS == m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()))
        {
        sp.Commit();
        return SUCCESS;
        }

    sp.Cancel();
    return ERROR;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                     03/17
//+---------------+---------------+---------------+---------------+---------------+------
DbResult TestHelper::ExecuteNonSelectECSql(Utf8CP ecsql) const
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, ecsql))
        return BE_SQLITE_ERROR;

    LOG.debugv("ECSQL %s -> SQL %s", ecsql, stmt.GetNativeSql());
    return stmt.Step();
    }

//---------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                     03/17
//+---------------+---------------+---------------+---------------+---------------+------
DbResult TestHelper::ExecuteInsertECSql(ECInstanceKey& key, Utf8CP ecsql) const
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, ecsql))
        return BE_SQLITE_ERROR;

    LOG.debugv("ECSQL %s -> SQL %s", ecsql, stmt.GetNativeSql());
    return stmt.Step(key);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
PropertyMap TestHelper::GetPropertyMap(AccessString const& propAccessString) const
    {
    const ECClassId classId = m_ecdb.Schemas().GetClassId(propAccessString.m_schemaNameOrAlias, propAccessString.m_className, SchemaLookupMode::AutoDetect);
    if (!classId.IsValid())
        return PropertyMap();

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(
        //can return multiple rows for same prop and same column in case of inherited prop. Therefore using DISTINCT
        //Need to wrap the pp.AccessString and the parameter in . so that we don't find cases like this:
        //A -> should not match AId, but should match Foo.A or A.Id or Foo.A.Id
        "SELECT t.Name," SQL_SELECTCLAUSE_ecColumn " FROM ec_Table t "
        "  INNER JOIN ec_Column c ON t.Id=c.TableId "
        "  INNER JOIN ec_PropertyMap pm ON pm.ColumnId=c.Id "
        "  INNER JOIN ec_PropertyPath pp ON pp.Id=pm.PropertyPathId "
        "WHERE pm.ClassId=?1 AND instr('.' || pp.AccessString || '.' ,'.' || ?2 || '.') = 1 ORDER BY t.Name,c.Id");

    if (stmt == nullptr)
        {
        EXPECT_TRUE(false) << m_ecdb.GetLastError().c_str();
        return PropertyMap();
        }

    EXPECT_EQ(BE_SQLITE_OK, stmt->BindId(1, classId));
    EXPECT_EQ(BE_SQLITE_OK, stmt->BindText(2, propAccessString.m_propAccessString, Statement::MakeCopy::No));

    PropertyMap propMap(classId, propAccessString.m_propAccessString);

    while (BE_SQLITE_ROW == stmt->Step())
        {
        propMap.AddColumn(GetColumnFromCurrentRow(stmt->GetValueText(0), *stmt, 1));
        }

    return propMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
Column TestHelper::GetPropertyMapColumn(AccessString const& propAccessString) const
    {
    PropertyMap propMap = GetPropertyMap(propAccessString);
    if (!propMap.IsValid())
        return Column();

    EXPECT_EQ(1, propMap.GetColumns().size());
    if (propMap.GetColumns().size() == 1)
        return propMap.GetColumns()[0];
     
    return Column();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
MapStrategyInfo TestHelper::GetMapStrategy(ECClassId classId) const
    {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT MapStrategy, ShareColumnsMode, MaxSharedColumnsBeforeOverflow, JoinedTableInfo FROM ec_ClassMap WHERE ClassId=?");
    EXPECT_TRUE(stmt != nullptr);

    MapStrategyInfo info;
    stmt->BindId(1, classId);
    if (BE_SQLITE_ROW == stmt->Step())
        {
        int parameterIx = 0;
        MapStrategy strat = (MapStrategy) stmt->GetValueInt(parameterIx);
        parameterIx++;
        MapStrategyInfo::TablePerHierarchyInfo::ShareColumnsMode shareColMode = stmt->IsColumnNull(parameterIx) ? MapStrategyInfo::TablePerHierarchyInfo::ShareColumnsMode::No : (MapStrategyInfo::TablePerHierarchyInfo::ShareColumnsMode) stmt->GetValueInt(parameterIx);
        parameterIx++;
        int maxSharedColumnsBeforeOverflow = stmt->IsColumnNull(parameterIx) ? -1 : stmt->GetValueInt(parameterIx);
        parameterIx++;
        MapStrategyInfo::JoinedTableInfo joinedTableInfo = stmt->IsColumnNull(parameterIx) ? MapStrategyInfo::JoinedTableInfo::None : (MapStrategyInfo::JoinedTableInfo) stmt->GetValueInt(parameterIx);

        return MapStrategyInfo(strat, MapStrategyInfo::TablePerHierarchyInfo(shareColMode, maxSharedColumnsBeforeOverflow, joinedTableInfo));
        }

    return MapStrategyInfo();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
Table TestHelper::GetMappedTable(Utf8StringCR tableName) const
    {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(
        R"sql(SELECT t.Id, t.Type, parent.Name, t.ExclusiveRootClassId
                FROM ec_Table t LEFT JOIN ec_Table parent ON parent.Id = t.ParentTableId
                WHERE t.Name=?)sql");

    if (stmt == nullptr)
        {
        BeAssert(false);
        return Table();
        }

    stmt->BindText(1, tableName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return Table();

    BeInt64Id tableId = stmt->GetValueId<BeInt64Id>(0);

    Table::Type type = (Table::Type) stmt->GetValueInt(1);
    Utf8CP parentTableName = nullptr;
    if (!stmt->IsColumnNull(2))
        parentTableName = stmt->GetValueText(2);

    ECClassId exclusiveRootClassId;
    if (!stmt->IsColumnNull(3))
        exclusiveRootClassId = stmt->GetValueId<ECClassId>(3);

    Table table(tableName, type, parentTableName, exclusiveRootClassId);

    //now load columns
    stmt = m_ecdb.GetCachedStatement("SELECT " SQL_SELECTCLAUSE_ecColumn " FROM ec_Column c WHERE c.TableId=? ORDER BY c.Ordinal");
    if (stmt == nullptr)
        {
        BeAssert(false);
        return Table();
        }

    stmt->BindId(1, tableId);
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        table.AddColumn(GetColumnFromCurrentRow(tableName, *stmt, 0));
        }

    return table;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
Column TestHelper::GetColumnFromCurrentRow(Utf8StringCR tableName, Statement& stmt, int columnFieldsStartIndex) const
    {
    Utf8CP columnName = stmt.GetValueText(columnFieldsStartIndex);
    Column::Type colType = (Column::Type) stmt.GetValueInt(columnFieldsStartIndex + 1);
    Virtual isVirtual = stmt.GetValueBoolean(columnFieldsStartIndex + 2) ? Virtual::Yes : Virtual::No;
    bool notNull = stmt.GetValueBoolean(columnFieldsStartIndex + 3);
    bool unique = stmt.GetValueBoolean(columnFieldsStartIndex + 4);
    Utf8CP checkConstraint = nullptr;
    if (!stmt.IsColumnNull(columnFieldsStartIndex + 5))
        checkConstraint = stmt.GetValueText(columnFieldsStartIndex + 5);
    Utf8CP defaultConstraint = nullptr;
    if (!stmt.IsColumnNull(columnFieldsStartIndex + 6))
        defaultConstraint = stmt.GetValueText(columnFieldsStartIndex + 6);
    Column::Collation collation = Column::Collation::Unset;
    if (!stmt.IsColumnNull(columnFieldsStartIndex + 7))
        collation = (Column::Collation) stmt.GetValueInt(columnFieldsStartIndex + 7);

    Column::Kind kind = Column::Kind::Default;
    if (!stmt.IsColumnNull(columnFieldsStartIndex + 8))
        kind = (Column::Kind) stmt.GetValueInt(columnFieldsStartIndex + 8);

    Nullable<uint32_t> ordinalInPk;
    if (!stmt.IsColumnNull(columnFieldsStartIndex + 9))
        ordinalInPk = (uint32_t) stmt.GetValueInt64(columnFieldsStartIndex + 9);

    return Column(tableName, columnName, colType, isVirtual, notNull, unique,
                  checkConstraint, defaultConstraint, collation, kind, ordinalInPk);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                     12/16
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String TestHelper::GetDdl(Utf8CP entityName, Utf8CP entityType) const
    {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT sql FROM sqlite_master WHERE name=? COLLATE NOCASE AND type=? COLLATE NOCASE");
    if (stmt == nullptr)
        return Utf8String();

    if (BE_SQLITE_OK != stmt->BindText(1, entityName, Statement::MakeCopy::No))
        return Utf8String();

    if (BE_SQLITE_OK != stmt->BindText(2, entityType, Statement::MakeCopy::No))
        return Utf8String();

    if (BE_SQLITE_ROW != stmt->Step())
        return Utf8String();

    return Utf8String(stmt->GetValueText(0));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<Utf8String> TestHelper::GetIndexNamesForTable(Utf8StringCR tableName) const
    {
    std::vector<Utf8String> indexNames;

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, "SELECT name FROM sqlite_master WHERE type='index' AND tbl_name=? ORDER BY name COLLATE NOCASE"))
        {
        BeAssert(false && "Preparation failed");
        return indexNames;
        }

    if (BE_SQLITE_OK != stmt.BindText(1, tableName, Statement::MakeCopy::No))
        {
        BeAssert(false && "Preparation failed");
        return indexNames;
        }

    while (BE_SQLITE_ROW == stmt.Step())
        {
        indexNames.push_back(stmt.GetValueText(0));
        }

    return indexNames;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
bool TestHelper::IsForeignKeyColumn(Utf8CP tableName, Utf8CP foreignKeyColumnName) const
    {
    Utf8String ddl = TestHelper(m_ecdb).GetDdl(tableName);

    Utf8String fkSearchString;
    fkSearchString.Sprintf("foreign key([%s]", foreignKeyColumnName);

    return ddl.ContainsI(fkSearchString);
    }

END_ECDBUNITTESTS_NAMESPACE