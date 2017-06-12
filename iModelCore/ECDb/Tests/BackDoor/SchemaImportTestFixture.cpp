/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/BackDoor/SchemaImportTestFixture.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/SchemaImportTestFixture.h"
#include <Bentley/DateTime.h>

BEGIN_ECDBUNITTESTS_NAMESPACE


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
    
    Utf8String actualDdl = TestHelper::RetrieveDdl(ecdb, indexName, "index");
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
    Utf8String ddl = TestHelper::RetrieveDdl(ecdb, tableName);
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
    Utf8String ddl = TestHelper::RetrieveDdl(ecdb, tableName);
    ASSERT_FALSE(ddl.empty());
    ASSERT_TRUE(ddl.find(foreignKeyDdl) != ddl.npos) << "Table: " << tableName << " Expected FK DDL: " << foreignKeyDdl << " Actual complete DDL: " << ddl.c_str();
    }

//---------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         02/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DbMappingTestFixture::TryGetMapStrategyInfo(MapStrategyInfo& stratInfo, ECDbCR ecdb, ECN::ECClassId classId)
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
//static
ColumnInfo::List DbMappingTestFixture::GetColumnInfos(ECDbCR ecdb, PropertyAccessString const& accessString)
    {
    ColumnInfo::List colInfos;

    ECN::ECClassId classId = ecdb.Schemas().GetClassId(accessString.m_schemaNameOrAlias, accessString.m_className, SchemaLookupMode::AutoDetect);
    EXPECT_TRUE(classId.IsValid());

    CachedStatementPtr stmt = ecdb.GetCachedStatement(
        //can return multiple rows for same prop and same column in case of inherited prop. Therefore using DISTINCT
        //Need to wrap the pp.AccessString and the parameter in . so that we don't find cases like this:
        //A -> should not match AId, but should match Foo.A or A.Id or Foo.A.Id
        R"sql(
        SELECT pp.AccessString, t.Name, c.Name, c.IsVirtual FROM ec_Table t
                     INNER JOIN ec_Column c ON t.Id=c.TableId
                     INNER JOIN ec_PropertyMap pm ON pm.ColumnId=c.Id
                     INNER JOIN ec_PropertyPath pp ON pp.Id=pm.PropertyPathId
                WHERE pm.ClassId=?1 AND instr('.' || pp.AccessString || '.' ,'.' || ?2 || '.') = 1 ORDER BY pp.AccessString,t.Name,c.Name
        )sql");

    if (stmt == nullptr)
        {
        EXPECT_TRUE(false) << ecdb.GetLastError().c_str();
        return colInfos;
        }

    EXPECT_EQ(BE_SQLITE_OK, stmt->BindId(1, classId));
    EXPECT_EQ(BE_SQLITE_OK, stmt->BindText(2, accessString.m_propAccessString, Statement::MakeCopy::No));
    while (BE_SQLITE_ROW == stmt->Step())
        {
        colInfos.push_back(ColumnInfo(stmt->GetValueText(0), stmt->GetValueText(1), stmt->GetValueText(2), stmt->GetValueInt(3) != 0));
        }

    return colInfos;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                 05/17
//+---------------+---------------+---------------+---------------+---------------+------
bool operator==(ColumnInfo::List const& lhs, ColumnInfo::List const& rhs)
    {
    if (lhs.size() != rhs.size())
        return false;

    for (size_t i = 0; i < lhs.size(); i++)
        {
        if (lhs[i] != rhs[i])
            return false;
        }

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                 05/17
//+---------------+---------------+---------------+---------------+---------------+------
bool operator!=(ColumnInfo::List const& lhs, ColumnInfo::List const& rhs) { return !(lhs == rhs); }

//--------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                 05/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ColumnInfo::List const& colInfos, std::ostream* os)
    {
    *os << "[";
    bool isFirstItem = true;
    for (ColumnInfo const& colInfo : colInfos)
        {
        if (!isFirstItem)
            *os << ",";

        *os << colInfo.ToString().c_str();
        isFirstItem = false;
        }
    *os << "]";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                 05/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ColumnInfo const& colInfo, std::ostream* os)
    {
    *os << colInfo.ToString().c_str();
    }

END_ECDBUNITTESTS_NAMESPACE

