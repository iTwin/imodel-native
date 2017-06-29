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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
ClassMap const* TestHelper::GetClassMap(Utf8CP schemaNameOrAlias, Utf8CP className) const
    {
    ECClassId classId = m_ecdb.Schemas().GetClassId(schemaNameOrAlias, className, SchemaLookupMode::AutoDetect);
    if (!classId.IsValid())
        return nullptr;

    return GetClassMap(classId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
PropertyMap const* TestHelper::GetPropertyMap(AccessString const& propAccessString) const
    {
    ClassMap const* classMap = GetClassMap(propAccessString.m_schemaNameOrAlias.c_str(), propAccessString.m_className.c_str());
    if (classMap == nullptr)
        return nullptr;

    return classMap->GetPropertyMap(propAccessString.m_propAccessString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
Column const* TestHelper::GetPropertyMapColumn(AccessString const& propAccessString) const
    {
    PropertyMap const* propMap = GetPropertyMap(propAccessString);
    if (propMap == nullptr)
        return nullptr;

    if (propMap->GetColumns().size() == 1)
        return propMap->GetColumns()[0];

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<Column const*> const& TestHelper::GetPropertyMapColumns(AccessString const& propAccessString) const
    {
    PropertyMap const* propMap = GetPropertyMap(propAccessString);
    if (propMap == nullptr)
        return PropertyMap::EmptyColumnList();

    return propMap->GetColumns();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
Column const* TestHelper::GetColumn(Utf8CP tableName, Utf8CP columnName) const
    {
    Table const* table = GetTable(tableName);
    if (table == nullptr)
        return nullptr;

    return table->GetColumn(columnName);
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
// @bsimethod                                   Krischan.Eberle                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<Utf8String> TestHelper::GetIndexNamesForTable(Utf8StringCR tableName) const
    {
    std::vector<Utf8String> indexNames;

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, "SELECT name FROM sqlite_master WHERE type='index' AND tbl_name=? ORDER BY name"))
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

//*************************************************************************************


//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
ClassMap const* TestHelper::DbSchemaCache::GetClassMap(ECClassId classId) const
    {
    auto itor = m_classMaps.find(classId);
    if (itor != m_classMaps.end())
        return itor->second.get();

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(
        R"sql(SELECT pp.AccessString, t.Name, col.Name
                FROM ec_ClassMap cm
                INNER JOIN ec_PropertyMap pm ON pm.ClassId = cm.ClassId
                INNER JOIN ec_PropertyPath pp ON pp.Id = pm.PropertyPathId
                INNER JOIN ec_Column col ON col.Id = pm.ColumnId
                INNER JOIN ec_Table t ON t.Id = col.TableId
                WHERE cm.ClassId=? ORDER BY pp.AccessString, t.Name, col.Ordinal)sql");
    if (stmt == nullptr)
        return nullptr;

    stmt->BindId(1, classId);

    std::unique_ptr<ClassMap> cm = std::make_unique<ClassMap>(classId);
    bmap<Utf8CP, PropertyMap*> compoundPropMaps;

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        Utf8String propAccessString(stmt->GetValueText(0));
        Utf8CP tableName = stmt->GetValueText(1);
        Utf8CP colName = stmt->GetValueText(2);

        Table const* table = GetTable(Utf8String(tableName));
        if (table == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }

        Column const* column = table->GetColumn(colName);
        if (column == nullptr)
            {
            BeAssert(false);
            return nullptr;
            }

        PropertyMap* propMap = cm->GetPropertyMapR(propAccessString);
        if (propMap == nullptr)
            propMap = cm->AddPropertyMap(std::make_unique<PropertyMap>(*cm, propAccessString));

        propMap->AddColumn(*column);

        bvector<Utf8String> tokens;
        BeStringUtilities::Split(propAccessString.c_str(), ".", tokens);
        if (tokens.size() > 1)
            {

            }
        }

    if (cm->GetPropertyMaps().empty())
        return nullptr;

    ClassMap const* cmCP = cm.get();
    m_classMaps[classId] = std::move(cm);
    return cmCP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
Table const* TestHelper::DbSchemaCache::GetTable(Utf8StringCR tableName) const
    {
    auto it = m_tables.find(tableName);
    if (it != m_tables.end())
        return it->second.get();

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(
          R"sql(SELECT t.Id, t.Type, parent.Name, t.ExclusiveRootClassId
                FROM ec_Table t LEFT JOIN ec_Table parent ON parent.Id = t.ParentTableId
                WHERE t.Name=?)sql");
    
    if (stmt == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindText(1, tableName, Statement::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return nullptr;

    BeInt64Id tableId = stmt->GetValueId<BeInt64Id>(0);

    Table::Type type = (Table::Type) stmt->GetValueInt(1);
    Utf8CP parentTableName = nullptr;
    if (!stmt->IsColumnNull(2))
        parentTableName = stmt->GetValueText(2);

    ECClassId exclusiveRootClassId;
    if (!stmt->IsColumnNull(3))
        exclusiveRootClassId = stmt->GetValueId<ECClassId>(3);

    std::unique_ptr<Table> table = std::make_unique<Table>(tableName.c_str(), type, parentTableName, exclusiveRootClassId);

    //now load columns
    stmt = m_ecdb.GetCachedStatement(
        R"sql(SELECT Name, Type, IsVirtual, NotNullConstraint, UniqueConstraint, CheckConstraint, DefaultConstraint, 
                     CollationConstraint, ColumnKind, OrdinalInPrimaryKey
              FROM ec_Column WHERE TableId=? ORDER BY Ordinal;)sql");

    if (stmt == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    stmt->BindId(1, tableId);
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        Utf8CP columnName = stmt->GetValueText(0);
        Column::Type colType = (Column::Type) stmt->GetValueInt(1);
        Virtual isVirtual = stmt->GetValueBoolean(2) ? Virtual::Yes : Virtual::No;
        bool notNull = stmt->GetValueBoolean(3);
        bool unique = stmt->GetValueBoolean(4);
        Utf8CP checkConstraint = nullptr;
        if (!stmt->IsColumnNull(5))
            checkConstraint = stmt->GetValueText(5);
        Utf8CP defaultConstraint = nullptr;
        if (!stmt->IsColumnNull(6))
            defaultConstraint = stmt->GetValueText(6);
        Column::Collation collation = Column::Collation::Unset;
        if (!stmt->IsColumnNull(7))
            collation = (Column::Collation) stmt->GetValueInt(7);

        Column::Kind kind = Column::Kind::Unknown;
        if (!stmt->IsColumnNull(8))
            kind = (Column::Kind) stmt->GetValueInt(8);

        Nullable<uint32_t> ordinalInPk;
        if (!stmt->IsColumnNull(9))
            ordinalInPk = (uint32_t) stmt->GetValueInt64(9);

        table->AddColumn(std::make_unique<Column>(*table, columnName, colType, isVirtual, notNull, unique,
                                                  checkConstraint, defaultConstraint, collation, kind, ordinalInPk));
        }

    if (table->GetColumns().empty())
        {
        BeAssert(false && "Failed to read columns of table");
        return nullptr;
        }

    Table const* tableCP = table.get();
    m_tables[tableName] = std::move(table);
    return tableCP;
    }

END_ECDBUNITTESTS_NAMESPACE