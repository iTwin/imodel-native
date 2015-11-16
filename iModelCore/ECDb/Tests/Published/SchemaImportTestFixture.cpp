/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/SchemaImportTestFixture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "SchemaImportTestFixture.h"

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
    ASSERT_EQ (BE_SQLITE_OK, ECDbTestUtility::CreateECDb(ecdb, nullptr, WString(ecdbFileName, BentleyCharEncoding::Utf8).c_str()));

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

    for (Utf8StringCR schemaXml : testItem.m_schemaXmlList)
        {
        ASSERT_EQ(SUCCESS, ECDbTestUtility::ReadECSchemaFromString(context, schemaXml.c_str())) << testItem.m_assertMessage.c_str() << " " << schemaXml.c_str();
        }

    if (!testItem.m_expectedToSucceed)
        BeTest::SetFailOnAssert(false);

        {
        ASSERT_EQ(testItem.m_expectedToSucceed, SUCCESS == ecdb.Schemas().ImportECSchemas(context->GetCache ())) << testItem.m_assertMessage.c_str();
        asserted = false;
        }

    BeTest::SetFailOnAssert(true);
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

    expectedDdl.append("INDEX [").append(indexName).append("] ON [").append(tableName).append("] (");
    bool isFirstColumn = true;
    for (Utf8CP column : columns)
        {
        if (!isFirstColumn)
            expectedDdl.append(",");

        expectedDdl.append("[").append(column).append("]");
        isFirstColumn = false;
        }

    expectedDdl.append(")");
    if (!Utf8String::IsNullOrEmpty (whereClause))
        expectedDdl.append(" WHERE ").append(whereClause);

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT sql FROM sqlite_master WHERE name=?"));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, indexName, Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Index " << indexName << " does not exist unexpectedly";

    ASSERT_STRCASEEQ(expectedDdl.c_str(), stmt.GetValueText(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::vector<SchemaImportTestFixture::IndexInfo> SchemaImportTestFixture::RetrieveIndicesForTable(ECDbCR ecdb, Utf8CP tableName)
    {
    std::vector<SchemaImportTestFixture::IndexInfo> indices;

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT name, sql FROM sqlite_master WHERE type='index' AND tbl_name=?"))
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

    return std::move(indices);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaImportTestFixture::AssertForeignKey(bool expectedToHaveForeignKey, ECDbCR ecdb, Utf8CP tableName, Utf8CP foreignKeyColumnName)
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT sql FROM sqlite_master WHERE name=?"));

    stmt.BindText(1, tableName, Statement::MakeCopy::No);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    Utf8String ddl(stmt.GetValueText(0));

    //only one row expected
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    Utf8String fkSearchString;
    if (Utf8String::IsNullOrEmpty(foreignKeyColumnName))
        fkSearchString = "FOREIGN KEY (";
    else
        fkSearchString.Sprintf("FOREIGN KEY ([%s", foreignKeyColumnName);

    ASSERT_EQ(expectedToHaveForeignKey, ddl.find(fkSearchString) != ddl.npos);
    }


//---------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         02/15
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbMappingTestFixture::TryGetPersistedMapStrategy(PersistedMapStrategy& strategy, ECDbCR ecdb, ECN::ECClassId classId) const
    {
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT MapStrategy, MapStrategyOptions, MapStrategyAppliesToSubclasses FROM ec_ClassMap WHERE ClassId = ?");
    EXPECT_TRUE(stmt != nullptr);

    stmt->BindInt64(1, classId);
    if (BE_SQLITE_ROW == stmt->Step())
        {
        const PersistedMapStrategy::Strategy strat = (PersistedMapStrategy::Strategy) stmt->GetValueInt(0);
        const PersistedMapStrategy::Options options = (PersistedMapStrategy::Options) stmt->GetValueInt(1);
        const bool appliesToSubclasses = stmt->GetValueInt(2) == 1;
        strategy = PersistedMapStrategy(strat, options, appliesToSubclasses);
        return true;
        }

    return false;
    }

END_ECDBUNITTESTS_NAMESPACE

