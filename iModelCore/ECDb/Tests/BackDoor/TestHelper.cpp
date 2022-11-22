/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/TestHelper.h"
#include "PublicAPI/BackDoor/ECDb/ECDbTestFixture.h"
#include <Bentley/BeTextFile.h>

USING_NAMESPACE_BENTLEY_EC

#define SQL_SELECTCLAUSE_ecColumn "c.Name,c.Type,c.IsVirtual,c.NotNullConstraint,c.UniqueConstraint,c.CheckConstraint,c.DefaultConstraint,c.CollationConstraint,c.ColumnKind,c.OrdinalInPrimaryKey"

BEGIN_ECDBUNITTESTS_NAMESPACE

//************************************************************************************
// TestHelper
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus TestHelper::RunSchemaImport(SchemaItem const& schema, Utf8CP fileName /*= nullptr*/)
    {
    ECDb ecdb;
    if (BE_SQLITE_OK != ECDbTestFixture::CreateECDb(ecdb, fileName))
        return ERROR;

    auto rc = TestHelper(ecdb).ImportSchema(schema);
    if (rc == BE_SQLITE_OK)
        ecdb.SaveChanges();
    else
        ecdb.AbandonChanges();

    return rc;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus TestHelper::RunSchemaImport(std::vector<SchemaItem> const& schemas, Utf8CP fileName /*= nullptr*/)
    {
    ECDb ecdb;
    if (BE_SQLITE_OK != ECDbTestFixture::CreateECDb(ecdb, fileName))
        return ERROR;

    auto rc = TestHelper(ecdb).ImportSchemas(schemas);
    if (rc == BE_SQLITE_OK)
        ecdb.SaveChanges();
    else
        ecdb.AbandonChanges();

    return rc;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus TestHelper::RunSchemaImportOneAtATime(std::vector<SchemaItem> const& schemas, Utf8CP fileName /*= nullptr*/)
    {
    ECDb ecdb;
    if (BE_SQLITE_OK != ECDbTestFixture::CreateECDb(ecdb, fileName))
        return ERROR;

    for (auto& schema : schemas)
        {
        if (TestHelper(ecdb).ImportSchema(schema, SchemaManager::SchemaImportOptions::None) != SUCCESS)
            {
            ecdb.AbandonChanges();
            return ERROR;
            }
        }
    ecdb.SaveChanges();
    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestHelper::ImportSchema(SchemaItem const& testItem, SchemaManager::SchemaImportOptions options) const
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
    if (SUCCESS == m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas(), options))
        {
        sp.Commit();
        return SUCCESS;
        }

    sp.Cancel();
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestHelper::ImportSchema(ECN::ECSchemaCP testSchema, SchemaManager::SchemaImportOptions options) const
    {
    Savepoint sp(const_cast<ECDb&>(m_ecdb), "ECSchema Import");
    if (SUCCESS == m_ecdb.Schemas().ImportSchemas(bvector<ECN::ECSchemaCP>{testSchema}, options))
        {
        sp.Commit();
        return SUCCESS;
        }

    sp.Cancel();
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestHelper::ImportSchema(ECN::ECSchemaPtr testSchema, SchemaManager::SchemaImportOptions options) const
    {
    return ImportSchema(testSchema.get(), options);
    }


//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String TestHelper::ECSqlToSql(Utf8CP ecsql) const
    {
    ECSqlStatement stmt; 
    if (ECSqlStatus::Success == stmt.Prepare(m_ecdb, ecsql))
        {
        return Utf8String(stmt.GetNativeSql());
        }

    return Utf8String();
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult TestHelper::ExecuteECSql(Utf8CP ecsql) const
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, ecsql))
        return BE_SQLITE_ERROR;

    return stmt.Step();
    }


//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
JsonValue TestHelper::ExecuteSelectECSql(Utf8CP ecsql) const
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, ecsql))
        return JsonValue();
    
    return ExecutePreparedECSql(stmt);
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult TestHelper::ExecuteInsertECSql(ECInstanceKey& key, Utf8CP ecsql) const
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, ecsql))
        return BE_SQLITE_ERROR;

    return stmt.Step(key);
    }


//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
JsonValue TestHelper::ExecutePreparedECSql(ECSqlStatement& stmt) const
    {
    if (!stmt.IsPrepared())
        return JsonValue();

    LOG.debugv("ECSQL: %s | SQL: %s", stmt.GetECSql(), stmt.GetNativeSql());

    JsonValue resultSet;
    JsonECSqlSelectAdapter adapter(stmt, JsonECSqlSelectAdapter::FormatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber));
    while (BE_SQLITE_ROW == stmt.Step())
        {
        Json::Value row;
        if (SUCCESS != adapter.GetRow(row))
            return JsonValue();

        resultSet.m_value.appendValue().From(row);
        }

    return resultSet;
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BeVersion TestHelper::GetOriginalECXmlVersion(Utf8CP schemaName) const
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, "SELECT OriginalECXmlVersionMajor,OriginalECXmlVersionMinor FROM meta.ECSchemaDef WHERE Name=?"))
        return BeVersion();
    
    stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No);
    if (stmt.Step() != BE_SQLITE_ROW)
        return BeVersion();

    if (stmt.IsValueNull(0))
        return BeVersion();

    return BeVersion(stmt.GetValueInt(0), stmt.GetValueInt(1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Table TestHelper::GetMappedTable(Utf8StringCR tableName) const
    {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(
        R"sql(SELECT t.Id,t.Type,parent.Name,t.ExclusiveRootClassId
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
// @bsimethod
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

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool TestHelper::TableSpaceExists(Utf8CP tableSpace) const
    {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("pragma database_list");

    while (BE_SQLITE_ROW == stmt->Step())
        {
        if (BeStringUtilities::StricmpAscii(tableSpace, stmt->GetValueText(1)) == 0)
            return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String TestHelper::GetDdl(Utf8CP entityName, Utf8CP dbSchemaName, Utf8CP entityType) const
    {
    CachedStatementPtr stmt;
    if (Utf8String::IsNullOrEmpty(dbSchemaName))
        stmt = m_ecdb.GetCachedStatement("SELECT sql FROM sqlite_master WHERE name=? COLLATE NOCASE AND type=? COLLATE NOCASE");
    else
        stmt = m_ecdb.GetCachedStatement(Utf8PrintfString("SELECT sql FROM [%s].sqlite_master WHERE name=? COLLATE NOCASE AND type=? COLLATE NOCASE", dbSchemaName).c_str());

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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<Utf8String> TestHelper::GetIndexNamesForTable(Utf8StringCR tableName, Utf8CP dbSchemaName) const
    {
    std::vector<Utf8String> indexNames;

    Utf8String sql;
    if (Utf8String::IsNullOrEmpty(dbSchemaName))
        sql.assign("SELECT name FROM sqlite_master WHERE type='index' AND tbl_name=? ORDER BY name COLLATE NOCASE");
    else
        sql.Sprintf("SELECT name FROM [%s].sqlite_master WHERE type='index' AND tbl_name=? ORDER BY name COLLATE NOCASE", dbSchemaName);

    Statement stmt;
    if (BE_SQLITE_OK != stmt.TryPrepare(m_ecdb, sql.c_str()))
        return indexNames; //e.g. if table space is not attached

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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool TestHelper::IsForeignKeyColumn(Utf8CP tableName, Utf8CP foreignKeyColumnName, Utf8CP dbSchemaName) const
    {
    Utf8String ddl = TestHelper(m_ecdb).GetDdl(tableName, dbSchemaName);

    Utf8String fkSearchString;
    fkSearchString.Sprintf("FOREIGN KEY([%s]", foreignKeyColumnName);
    return ddl.ContainsI(fkSearchString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool TestHelper::IsForeignKeyColumn(Utf8CP tableName, Utf8CP foreignKeyColumnName, Utf8CP onDeleteAction, Utf8CP onUpdateAction, Utf8CP dbSchemaName) const
    {
    Utf8String fkConstraintDdl = GetForeignKeyConstraintDdl(tableName, foreignKeyColumnName, dbSchemaName);
    if (fkConstraintDdl.empty())
        return false;

    if (!Utf8String::IsNullOrEmpty(onDeleteAction))
        {
        Utf8String actionSearchString;
        actionSearchString.Sprintf("ON DELETE %s", onDeleteAction);

        if (!fkConstraintDdl.ContainsI(actionSearchString))
            return false;
        }
    else
        {
        if (fkConstraintDdl.ContainsI("ON DELETE"))
            return false;
        }

    if (!Utf8String::IsNullOrEmpty(onUpdateAction))
        {
        Utf8String actionSearchString;
        actionSearchString.Sprintf("ON UPDATE %s", onUpdateAction);

        return fkConstraintDdl.ContainsI(actionSearchString);
        }

    return !fkConstraintDdl.ContainsI("ON UPDATE");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String TestHelper::GetForeignKeyConstraintDdl(Utf8CP tableName, Utf8CP foreignKeyColumnName, Utf8CP dbSchemaName) const
    {
    Utf8String ddl = TestHelper(m_ecdb).GetDdl(tableName, dbSchemaName);

    bvector<Utf8String> tokens;
    BeStringUtilities::Split(ddl.c_str(), ",", tokens);

    for (Utf8StringCR token : tokens)
        {
        Utf8String fkSearchString1;
        fkSearchString1.Sprintf("FOREIGN KEY([%s]", foreignKeyColumnName);
        Utf8String fkSearchString2;
        fkSearchString2.Sprintf("FOREIGN KEY(%s", foreignKeyColumnName);

        if (token.ContainsI(fkSearchString1) || token.ContainsI(fkSearchString2))
            return token;
        }

    return Utf8String();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int TestHelper::GetFrequencyCount(Utf8StringCR source, Utf8StringCR target) const
    {
    int occurrences = 0;
    size_t pos = 0;
    while ((pos = source.find(target, pos)) != std::string::npos)
        {
        occurrences++;
        pos += target.length();
        }
    return occurrences;
    }

//**************************************************************************************
// TestUtilities
//**************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
// @DMR Temp overloaded func
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus TestUtilities::ReadFile(Json::Value& json, BeFileNameCR jsonFilePath)
    {
    Utf8String jsonFileContent;
    if (SUCCESS != ReadFile(jsonFileContent, jsonFilePath))
        return ERROR;

    return ParseJson(json, jsonFileContent);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus TestUtilities::ReadFile(BeJsDocument& json, BeFileNameCR jsonFilePath)
    {
    Utf8String jsonFileContent;
    if (SUCCESS != ReadFile(jsonFileContent, jsonFilePath))
        return ERROR;

    return ParseJson(json, jsonFileContent);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus TestUtilities::ReadFile(rapidjson::Document& json, BeFileNameCR jsonFilePath)
    {
    Utf8String jsonFileContent;
    if (SUCCESS != ReadFile(jsonFileContent, jsonFilePath))
        return ERROR;

    return ParseJson(json, jsonFileContent);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus TestUtilities::ReadFile(Utf8StringR fileContent, BeFileNameCR filePath)
    {
    BeFileStatus stat = BeFileStatus::Success;
    BeTextFilePtr file = BeTextFile::Open(stat, filePath, TextFileOpenType::Read, TextFileOptions::None, TextFileEncoding::Utf8);
    if (BeFileStatus::Success != stat)
        {
        EXPECT_EQ(BeFileStatus::Success, stat) << "Reading file " << filePath.GetNameUtf8();
        return ERROR;
        }

    TextFileReadStatus readStat;
    WString line;
    while (TextFileReadStatus::Success == (readStat = file->GetLine(line)))
        {
        fileContent.append(Utf8String(line));
        }

    if (readStat != TextFileReadStatus::Eof)
        {
        EXPECT_EQ(TextFileReadStatus::Eof, readStat) << "Reading file " << filePath.GetNameUtf8();
        return ERROR;
        }

    return SUCCESS;
    }



//**************************************************************************************
// ECInstancePopulator
//**************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
void ECInstancePopulator::Populate(ECN::IECInstance& ecInstance, bool skipStructs, bool skipArrays, bool skipReadOnlyProps)
    {
    ECValue value;
    for (ECPropertyCP ecProperty : ecInstance.GetClass().GetProperties(true))
        {
        if (ecProperty->GetIsReadOnly() && skipReadOnlyProps)
            continue;

        if (!skipStructs && ecProperty->GetIsStruct())
            {
            PopulateStructValue(value, ecProperty->GetAsStructProperty()->GetType());
            CopyStruct(ecInstance, *value.GetStruct(), ecProperty->GetName().c_str());
            }
        else if (ecProperty->GetIsPrimitive())
            {
            PopulatePrimitiveValue(value, ecProperty->GetAsPrimitiveProperty()->GetType(), ecProperty);
            ecInstance.SetValue(ecProperty->GetName().c_str(), value);
            }
        else if (!skipArrays && ecProperty->GetIsArray())
            {
            ArrayECPropertyCP arrayProperty = ecProperty->GetAsArrayProperty();
            if (arrayProperty->GetIsPrimitiveArray() && arrayProperty->GetAsPrimitiveArrayProperty()->GetPrimitiveElementType() == PRIMITIVETYPE_IGeometry)
                continue;

            uint32_t arrayCount = 3;
            if (arrayCount < arrayProperty->GetMinOccurs())
                arrayCount = arrayProperty->GetMinOccurs();
            else if (arrayCount > arrayProperty->GetMaxOccurs())
                arrayCount = arrayProperty->GetMaxOccurs();

            ecInstance.AddArrayElements(ecProperty->GetName().c_str(), arrayCount);
            if (arrayProperty->GetIsStructArray())
                {
                StructArrayECPropertyCP structArrayProperty = ecProperty->GetAsStructArrayProperty();
                for (uint32_t i = 0; i < arrayCount; i++)
                    {
                    PopulateStructValue(value, structArrayProperty->GetStructElementType());
                    ecInstance.SetValue(ecProperty->GetName().c_str(), value, i);
                    }
                }
            else if (arrayProperty->GetIsPrimitiveArray())
                {
                PrimitiveArrayECPropertyCP primitiveArrayProperty = ecProperty->GetAsPrimitiveArrayProperty();
                for (uint32_t i = 0; i < arrayCount; i++)
                    {
                    PopulatePrimitiveValue(value, primitiveArrayProperty->GetPrimitiveElementType(), ecProperty);
                    ecInstance.SetValue(ecProperty->GetName().c_str(), value, i);
                    }
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
void ECInstancePopulator::PopulateStructValue(ECValueR value, ECClassCR structType)
    {
    value.Clear();
    IECInstancePtr structInst = structType.GetDefaultStandaloneEnabler()->CreateInstance();
    Populate(*structInst);
    value.SetStruct(structInst.get());
    }

//-------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-----
//static
void ECInstancePopulator::PopulatePrimitiveValue(ECValueR ecValue, PrimitiveType primitiveType, ECPropertyCP ecProperty)
    {
    ecValue.Clear();

    int randomNumber = rand();
    switch (primitiveType)
        {
            case PRIMITIVETYPE_String:
            {
            Utf8String text;
            text.Sprintf("Sample text with random number: %d", randomNumber);
            ecValue.SetUtf8CP(text.c_str(), true);
            }
            break;

            case PRIMITIVETYPE_Integer:
            {
            ecValue.SetInteger(randomNumber);
            }
            break;

            case PRIMITIVETYPE_Long:
            {
            const int32_t intMax = std::numeric_limits<int32_t>::max();
            const int64_t longValue = static_cast<int64_t> (intMax) + randomNumber;
            ecValue.SetLong(longValue);
            }
            break;

            case PRIMITIVETYPE_Double:
            {
            ecValue.SetDouble(randomNumber / 4.0);
            }
            break;

            case PRIMITIVETYPE_DateTime:
            {
            DateTime utcTime = DateTime::GetCurrentTimeUtc();
            ecValue.SetDateTime(utcTime);
            }
            break;

            case PRIMITIVETYPE_Boolean:
            {
            ecValue.SetBoolean(randomNumber % 2 != 0);
            }
            break;

            case PRIMITIVETYPE_Point2d:
            {
            DPoint2d point2d;
            point2d.x = randomNumber * 1.0;
            point2d.y = randomNumber * 1.5;
            ecValue.SetPoint2d(point2d);
            break;
            }
            case PRIMITIVETYPE_Point3d:
            {
            DPoint3d point3d;
            point3d.x = randomNumber * 1.0;
            point3d.y = randomNumber * 1.5;
            point3d.z = randomNumber * 2.0;
            ecValue.SetPoint3d(point3d);
            break;
            }

            case PRIMITIVETYPE_IGeometry:
            {
            IGeometryPtr line = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(randomNumber, randomNumber*2.0, randomNumber*3.0,
                                                                                               -randomNumber, randomNumber*(-2.0), randomNumber*(-3.0))));
            ecValue.SetIGeometry(*line);
            break;
            }

            default:
                break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
ECObjectsStatus ECInstancePopulator::CopyStruct(IECInstanceR source, ECValuesCollectionCR collection, Utf8CP baseAccessPath)
    {
    ECObjectsStatus status = ECObjectsStatus::Success;
    for (auto& propertyValue : collection)
        {
        auto pvAccessString = propertyValue.GetValueAccessor().GetPropertyName();
        auto accessString = baseAccessPath == nullptr ? pvAccessString : Utf8String(baseAccessPath) + "." + pvAccessString;

        if (propertyValue.HasChildValues())
            {
            status = CopyStruct(source, *propertyValue.GetChildValues(), accessString.c_str());
            if (status != ECObjectsStatus::Success)
                {
                return status;
                }
            continue;
            }

        auto& location = propertyValue.GetValueAccessor().DeepestLocationCR();

        //auto property = location.GetECProperty(); 
        //BeAssert(property != nullptr);
        if (location.GetArrayIndex() >= 0)
            {
            source.AddArrayElements(accessString.c_str(), 1);
            status = source.SetValue(accessString.c_str(), propertyValue.GetValue(), location.GetArrayIndex());
            }
        else
            status = source.SetValue(accessString.c_str(), propertyValue.GetValue());

        if (status != ECObjectsStatus::Success)
            {
            return status;
            }
        }
    return status;
    }
END_ECDBUNITTESTS_NAMESPACE
