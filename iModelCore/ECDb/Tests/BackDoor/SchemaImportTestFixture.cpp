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
    BentleyStatus schemaImportStatus = ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas());
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
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, SchemaManager::GetValidateDbMappingSql()))
        {
        EXPECT_TRUE(false) << ecdb.GetLastError().c_str();
        return true;
        }
    
    bool hasError = false;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        hasError = true;
        LOG.errorv("ECClass '%s:%s' with invalid mapping: %s. Table name: %s - %s", stmt.GetValueText(0),
                   stmt.GetValueText(2), stmt.GetValueText(5), stmt.GetValueText(3), stmt.GetValueText(6));
        }

    return hasError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle               05/17
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaImportTestFixture::AssertColumnNames(ECDbCR ecdb, Utf8CP tableName, std::vector<Utf8String> const& expectedColumnNames, Utf8CP scenario)
    {
    bvector<Utf8String> actualColNames;
    ASSERT_TRUE(ecdb.GetColumns(actualColNames, tableName)) << tableName << " Scenario: " << scenario;
    ASSERT_EQ(expectedColumnNames.size(), actualColNames.size()) << tableName << " Scenario: " << scenario;
    std::sort(actualColNames.begin(), actualColNames.end());

    std::vector<Utf8String> expectedColNamesSorted {expectedColumnNames};
    std::sort(expectedColNamesSorted.begin(), expectedColNamesSorted.end());
    for (size_t i = 0; i < expectedColNamesSorted.size(); i++)
        {
        ASSERT_STRCASEEQ(expectedColNamesSorted[0].c_str(), actualColNames[0].c_str()) << tableName << " Scenario: " << scenario;
        }
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
bool DbMappingTestFixture::TryGetMapStrategyInfo(MapStrategyInfo& stratInfo, ECDbCR ecdb, ECN::ECClassId classId) const
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT MapStrategy, ShareColumnsMode, MaxSharedColumnsBeforeOverflow, JoinedTableInfo FROM ec_ClassMap WHERE ClassId=?");
    EXPECT_TRUE(stmt != nullptr);

    stmt->BindId(1, classId);
    if (BE_SQLITE_ROW == stmt->Step())
        {
        int parameterIx = 0;
        stratInfo.m_strategy = (MapStrategyInfo::Strategy) stmt->GetValueInt(parameterIx);
        parameterIx++;
        stratInfo.m_tphInfo.m_sharedColumnsMode = stmt->IsColumnNull(parameterIx) ? MapStrategyInfo::TablePerHierarchyInfo::ShareColumnsMode::No : (MapStrategyInfo::TablePerHierarchyInfo::ShareColumnsMode) stmt->GetValueInt(parameterIx);
        parameterIx++;
        stratInfo.m_tphInfo.m_maxSharedColumnsBeforeOverflow = stmt->IsColumnNull(parameterIx) ? -1 : stmt->GetValueInt(parameterIx);
        parameterIx++;
        stratInfo.m_tphInfo.m_joinedTableInfo = stmt->IsColumnNull(parameterIx) ? MapStrategyInfo::JoinedTableInfo::None : (MapStrategyInfo::JoinedTableInfo) stmt->GetValueInt(parameterIx);
        return true;
        }

    return false;
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                 03/17
//+---------------+---------------+---------------+---------------+---------------+------
void DbMappingTestFixture::AssertPropertyMapping(ECDbCR ecdb, PropertyAccessString const& accessString, std::map<Utf8String, ColumnInfo> const& expectedColumnInfosByAccessString) const
    {
    std::map<Utf8String, ColumnInfo> actualColInfos;
    ASSERT_TRUE(TryGetColumnInfo(actualColInfos, ecdb, accessString)) << accessString.ToString().c_str();
    ASSERT_EQ(expectedColumnInfosByAccessString.size(), actualColInfos.size()) << accessString.ToString().c_str();
    for (std::pair<Utf8String, ColumnInfo> const& expectedColInfo : expectedColumnInfosByAccessString)
        {
        Utf8StringCR expectedAccessString = expectedColInfo.first;
        auto it = actualColInfos.find(expectedAccessString);
        ASSERT_TRUE(it != actualColInfos.end()) << expectedAccessString.c_str() << " for " << accessString.ToString().c_str();
        ColumnInfo const& actualColInfo = it->second;
        ASSERT_EQ(expectedColInfo.second, actualColInfo) << "Expected: " << expectedColInfo.second.ToString().c_str() << " Actual: " << actualColInfo.ToString().c_str() << " " << expectedAccessString.c_str() << " for " << accessString.ToString().c_str();
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                 03/17
//+---------------+---------------+---------------+---------------+---------------+------
void DbMappingTestFixture::AssertPropertyMapping(ECDbCR ecdb, PropertyAccessString const& accessString, std::vector<ColumnInfo> const& expectedColInfos) const
    {
    std::vector<ColumnInfo> actualColInfos;
    ASSERT_TRUE(TryGetColumnInfo(actualColInfos, ecdb, accessString)) << accessString.ToString().c_str();
    ASSERT_EQ(expectedColInfos.size(), actualColInfos.size()) << accessString.ToString().c_str();
    for (size_t i = 0; i < expectedColInfos.size(); i++)
        {
        ASSERT_EQ(expectedColInfos[i], actualColInfos[i]) << "Expected: " << expectedColInfos[i].ToString().c_str() << " Actual: " << actualColInfos[i].ToString().c_str() << " " <<  accessString.ToString().c_str();
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                 03/17
//+---------------+---------------+---------------+---------------+---------------+------
bool DbMappingTestFixture::TryGetColumnInfo(std::map<Utf8String, ColumnInfo>& colInfosPerAccessString, ECDbCR ecdb, PropertyAccessString const& accessString) const
    {
    ECN::ECClassId classId = ecdb.Schemas().GetClassId(accessString.m_schemaNameOrAlias, accessString.m_className, SchemaLookupMode::AutoDetect);
    EXPECT_TRUE(classId.IsValid());

    CachedStatementPtr stmt = ecdb.GetCachedStatement(
        //can return multiple rows for same prop and same column in case of inherited prop. Therefore using DISTINCT
        R"sql(
        SELECT pp.AccessString, t.Name, c.Name, c.IsVirtual FROM ec_Table t
                     INNER JOIN ec_Column c ON t.Id=c.TableId
                     INNER JOIN ec_PropertyMap pm ON pm.ColumnId=c.Id
                     INNER JOIN ec_PropertyPath pp ON pp.Id=pm.PropertyPathId
                WHERE pm.ClassId=? AND instr(pp.AccessString,?) = 1 ORDER BY pp.AccessString,t.Name,c.Name
        )sql");

    if (stmt == nullptr)
        {
        EXPECT_TRUE(stmt != nullptr) << ecdb.GetLastError().c_str();
        return false;
        }

    EXPECT_EQ(BE_SQLITE_OK, stmt->BindId(1, classId));
    EXPECT_EQ(BE_SQLITE_OK, stmt->BindText(2, accessString.m_propAccessString, Statement::MakeCopy::No));
    while (BE_SQLITE_ROW == stmt->Step())
        {
        Utf8String actualPropAccessString(stmt->GetValueText(0));
        BeAssert(colInfosPerAccessString.find(actualPropAccessString) == colInfosPerAccessString.end());
        colInfosPerAccessString.insert(std::make_pair(actualPropAccessString, ColumnInfo(stmt->GetValueText(1), stmt->GetValueText(2), stmt->GetValueInt(3) != 0)));
        }

    return !colInfosPerAccessString.empty();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                 03/17
//+---------------+---------------+---------------+---------------+---------------+------
bool DbMappingTestFixture::TryGetColumnInfo(std::vector<ColumnInfo>& colInfos, ECDbCR ecdb, PropertyAccessString const& accessString) const
    {
    ECN::ECClassId classId = ecdb.Schemas().GetClassId(accessString.m_schemaNameOrAlias, accessString.m_className, SchemaLookupMode::AutoDetect);
    EXPECT_TRUE(classId.IsValid());

    CachedStatementPtr stmt = ecdb.GetCachedStatement(
        //can return multiple rows for same prop and same column in case of inherited prop. Therefore using DISTINCT
        R"sql(
        SELECT t.Name, c.Name, c.IsVirtual FROM ec_Table t
                     INNER JOIN ec_Column c ON t.Id=c.TableId
                     INNER JOIN ec_PropertyMap pm ON pm.ColumnId=c.Id
                     INNER JOIN ec_PropertyPath pp ON pp.Id=pm.PropertyPathId
                WHERE pm.ClassId=? AND pp.AccessString LIKE ? COLLATE NOCASE ORDER BY t.Name,c.Name
        )sql");

    if (stmt == nullptr)
        {
        EXPECT_TRUE(stmt != nullptr) << ecdb.GetLastError().c_str();
        return false;
        }

    EXPECT_EQ(BE_SQLITE_OK, stmt->BindId(1, classId));
    EXPECT_EQ(BE_SQLITE_OK, stmt->BindText(2, accessString.m_propAccessString, Statement::MakeCopy::No));
    while (BE_SQLITE_ROW == stmt->Step())
        {
        colInfos.push_back(ColumnInfo(stmt->GetValueText(0), stmt->GetValueText(1), stmt->GetValueInt(2) != 0));
        }

    return !colInfos.empty();
    }

END_ECDBUNITTESTS_NAMESPACE

