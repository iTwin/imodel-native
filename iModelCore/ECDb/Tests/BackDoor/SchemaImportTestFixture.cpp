/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/BackDoor/SchemaImportTestFixture.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/SchemaImportTestFixture.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaImportTestFixture::AssertSchemaImport(std::vector<SchemaItem> const& testItems, Utf8CP ecdbFileName) const
    {
    for (SchemaItem const& testItem : testItems)
        {
        AssertSchemaImport(testItem, ecdbFileName);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaImportTestFixture::AssertSchemaImport(SchemaItem const& testItem, Utf8CP ecdbFileName) const
    {
    ECDb localECDb;
    bool asserted = false;
    AssertSchemaImport(localECDb, asserted, testItem, ecdbFileName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaImportTestFixture::AssertSchemaImport(ECDbR ecdb, bool& asserted, SchemaItem const& testItem, Utf8CP ecdbFileName) const
    {
    asserted = true;
    ASSERT_EQ(BE_SQLITE_OK, CreateECDb(ecdb, ecdbFileName));
    AssertSchemaImport(asserted, ecdb, testItem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaImportTestFixture::AssertSchemaImport(bool& asserted, ECDbCR ecdb, SchemaItem const& testItem) const
    {
    asserted = true;
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(ecdb.GetSchemaLocater());
    BeTest::SetFailOnAssert(false);
    const BentleyStatus deserializeStat = ReadECSchemaFromString(context, ecdb, testItem);
    BeTest::SetFailOnAssert(true);
    if (SUCCESS != deserializeStat)
        {
        ASSERT_FALSE(testItem.m_expectedToSucceed) << "ECSchema deserialization failed";
        asserted = false;
        return;
        }

    Savepoint sp(const_cast<ECDbR>(ecdb), "ECSchema Import");
    BentleyStatus schemaImportStatus = ecdb.Schemas().ImportECSchemas(context->GetCache().GetSchemas());
    if (schemaImportStatus == SUCCESS)
        sp.Commit();
    else
        sp.Cancel();

    ASSERT_EQ(testItem.m_expectedToSucceed, SUCCESS == schemaImportStatus) << testItem.m_assertMessage.c_str();

    if (SUCCESS == schemaImportStatus)
        ASSERT_EQ(testItem.m_expectedToSucceed, !HasDataCorruptingMappingIssues(ecdb)) << testItem.m_assertMessage.c_str();

    asserted = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan Khan                       02/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool SchemaImportTestFixture::HasDataCorruptingMappingIssues(ECDbCR ecdb)
    {
    EXPECT_TRUE(ecdb.IsDbOpen());

    if (!ecdb.IsDbOpen())
        return true;

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb,
                                     R"sql(
        SELECT ec_Schema.Name, ec_Class.Name, ec_Table.Name, ec_Column.Name, 
        '[' || GROUP_CONCAT(mappedpropertyschema.Alias || ':' || mappedpropertyclass.Name || '.' || ec_PropertyPath.AccessString,'|') || ']'
        FROM ec_PropertyMap
        INNER JOIN ec_Column ON ec_Column.Id=ec_PropertyMap.ColumnId
        INNER JOIN ec_Class ON ec_Class.Id=ec_PropertyMap.ClassId
        INNER JOIN ec_Schema ON ec_Schema.Id=ec_Class.SchemaId
        INNER JOIN ec_PropertyPath ON ec_PropertyPath.Id=ec_PropertyMap.PropertyPathId
        INNER JOIN ec_Table ON ec_Table.Id=ec_Column.TableId
        INNER JOIN ec_Property ON ec_Property.Id=ec_PropertyPath.RootPropertyId
        INNER JOIN ec_Class mappedpropertyclass ON mappedpropertyclass.Id=ec_Property.ClassId
        INNER JOIN ec_Schema mappedpropertyschema ON mappedpropertyschema.Id=mappedpropertyclass.SchemaId
        WHERE ec_Column.IsVirtual=0 AND (ec_Column.ColumnKind & 128=128)
        GROUP BY ec_PropertyMap.ClassId, ec_PropertyMap.ColumnId HAVING COUNT(*)>1)sql"))
        {
        EXPECT_TRUE(false) << ecdb.GetLastError().c_str();
        return true;
        }
    
    bool hasError = false;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        hasError = true;
        LOG.errorv("ECClass '%s:%s' with invalid mapping. Multiple properties map to the same column: %s:%s | %s", stmt.GetValueText(0),
                   stmt.GetValueText(1), stmt.GetValueText(2), stmt.GetValueText(3), stmt.GetValueText(4));
        }

    return hasError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/16
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaImportTestFixture::AssertColumnCount(ECDbCR ecdb, std::vector<std::pair<Utf8String, int>> const& testItems, Utf8CP scenario)
    {
    for (std::pair<Utf8String, int> const& kvPair : testItems)
        {
        Utf8CP tableName = kvPair.first.c_str();
        const int expectedColCount = kvPair.second;
        bvector<Utf8String> colNames;
        ASSERT_TRUE(ecdb.GetColumns(colNames, tableName)) << tableName << " Scenario: " << scenario;
        ASSERT_EQ(expectedColCount, colNames.size()) << tableName << " Scenario: " << scenario;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/15
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaImportTestFixture::AssertIndexExists(ECDbCR ecdb, Utf8CP indexName, bool expectedToExist)
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT NULL FROM sqlite_master WHERE name=? AND type='index'"));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, indexName, Statement::MakeCopy::No));
    if (expectedToExist)
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Index " << indexName << " does not exist unexpectedly";
    else
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Index " << indexName << " does exist unexpectedly";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaImportTestFixture::AssertIndex(ECDbCR ecdb, Utf8CP indexName, bool isUnique, Utf8CP tableName, std::vector<Utf8CP> const& columns, Utf8CP whereClause)
    {
    Utf8String expectedDdl("CREATE ");
    if (isUnique)
        expectedDdl.append("UNIQUE ");

    expectedDdl.append("INDEX [").append(indexName).append("] ON [").append(tableName).append("](");
    bool isFirstColumn = true;
    for (Utf8CP column : columns)
        {
        if (!isFirstColumn)
            expectedDdl.append(", ");

        expectedDdl.append("[").append(column).append("]");
        isFirstColumn = false;
        }

    expectedDdl.append(")");
    if (!Utf8String::IsNullOrEmpty (whereClause))
        expectedDdl.append(" WHERE ").append(whereClause);
    
    Utf8String actualDdl = RetrieveDdl(ecdb, indexName, "index");
    ASSERT_FALSE(actualDdl.empty());
    ASSERT_STRCASEEQ(expectedDdl.c_str(), actualDdl.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::vector<SchemaImportTestFixture::IndexInfo> SchemaImportTestFixture::RetrieveIndicesForTable(ECDbCR ecdb, Utf8CP tableName)
    {
    std::vector<SchemaImportTestFixture::IndexInfo> indices;

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT name, sql FROM sqlite_master WHERE type='index' AND tbl_name=? ORDER BY name"))
        {
        BeAssert(false && "Preparation failed");
        return indices;
        }

    if (BE_SQLITE_OK != stmt.BindText(1, tableName, Statement::MakeCopy::No))
        {
        BeAssert(false && "Preparation failed");
        return indices;
        }

    while (BE_SQLITE_ROW == stmt.Step())
        {
        indices.push_back(SchemaImportTestFixture::IndexInfo(stmt.GetValueText(0), tableName, stmt.GetValueText(1)));
        }

    return indices;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaImportTestFixture::AssertForeignKey(bool expectedToHaveForeignKey, ECDbCR ecdb, Utf8CP tableName, Utf8CP foreignKeyColumnName)
    {
    Utf8String ddl = RetrieveDdl(ecdb, tableName);
    ASSERT_FALSE(ddl.empty());

    Utf8String fkSearchString;
    if (Utf8String::IsNullOrEmpty(foreignKeyColumnName))
        fkSearchString = "FOREIGN KEY(";
    else
        fkSearchString.Sprintf("FOREIGN KEY([%s]", foreignKeyColumnName);

    ASSERT_EQ(expectedToHaveForeignKey, ddl.find(fkSearchString) != ddl.npos) << "Table: " << tableName << " Expected FK column name: " << foreignKeyColumnName << " Actual complete DDL: " << ddl.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  12/16
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaImportTestFixture::AssertForeignKeyDdl(ECDbCR ecdb, Utf8CP tableName, Utf8CP foreignKeyDdl)
    {
    Utf8String ddl = RetrieveDdl(ecdb, tableName);
    ASSERT_FALSE(ddl.empty());
    ASSERT_TRUE(ddl.find(foreignKeyDdl) != ddl.npos) << "Table: " << tableName << " Expected FK DDL: " << foreignKeyDdl << " Actual complete DDL: " << ddl.c_str();
    }


//---------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         02/15
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbMappingTestFixture::TryGetMapStrategyInfo(MapStrategyInfo& stratInfo, ECDbCR ecdb, ECN::ECClassId classId) const
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT MapStrategy, ShareColumnsMode, SharedColumnCount, SharedColumnCountPerOverflowTable, JoinedTableInfo FROM ec_ClassMap WHERE ClassId=?");
    EXPECT_TRUE(stmt != nullptr);

    stmt->BindId(1, classId);
    if (BE_SQLITE_ROW == stmt->Step())
        {
        int parameterIx = 0;
        stratInfo.m_strategy = (MapStrategyInfo::Strategy) stmt->GetValueInt(parameterIx);
        parameterIx++;
        stratInfo.m_tphInfo.m_sharedColumnsMode = stmt->IsColumnNull(parameterIx) ? MapStrategyInfo::TablePerHierarchyInfo::ShareColumnsMode::No : (MapStrategyInfo::TablePerHierarchyInfo::ShareColumnsMode) stmt->GetValueInt(parameterIx);
        parameterIx++;
        stratInfo.m_tphInfo.m_sharedColumnCount = stmt->IsColumnNull(parameterIx) ? -1 : stmt->GetValueInt(parameterIx);
        parameterIx++;
        stratInfo.m_tphInfo.m_sharedColumnCountPerOverflowTable = stmt->IsColumnNull(parameterIx) ? -1 : stmt->GetValueInt(parameterIx);
        parameterIx++;
        stratInfo.m_tphInfo.m_joinedTableInfo = stmt->IsColumnNull(parameterIx) ? MapStrategyInfo::JoinedTableInfo::None : (MapStrategyInfo::JoinedTableInfo) stmt->GetValueInt(parameterIx);
        return true;
        }

    return false;
    }




END_ECDBUNITTESTS_NAMESPACE

