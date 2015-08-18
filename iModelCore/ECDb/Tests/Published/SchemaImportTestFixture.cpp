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
// @bsimethod                                   Krischan.Eberle                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaImportTestFixture::AssertSchemaImport(ECDbR ecdb, bool& asserted, TestItem const& testItem, Utf8CP ecdbFileName) const
    {
    asserted = true;
    ASSERT_EQ (BE_SQLITE_OK, ECDbTestUtility::CreateECDb(ecdb, nullptr, WString(ecdbFileName, BentleyCharEncoding::Utf8).c_str()));

    AssertSchemaImport(asserted, ecdb, testItem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaImportTestFixture::AssertSchemaImport(TestItem const& testItem, Utf8CP ecdbFileName) const
    {
    ECDb localECDb;
    bool asserted = false;
    AssertSchemaImport(localECDb, asserted, testItem, ecdbFileName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaImportTestFixture::AssertSchemaImport(bool& asserted, ECDbCR ecdb, TestItem const& testItem) const
    {
    asserted = true;
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(ecdb.GetSchemaLocater());

    ASSERT_EQ (SUCCESS, ECDbTestUtility::ReadECSchemaFromString(context, testItem.m_schemaXml.c_str())) << testItem.m_assertMessage.c_str();

    if (!testItem.m_expectedToSucceed)
        BeTest::SetFailOnAssert(false);

        {
        ASSERT_EQ(testItem.m_expectedToSucceed, SUCCESS == ecdb.Schemas().ImportECSchemas(context->GetCache ())) << testItem.m_assertMessage.c_str();
        asserted = false;
        }

    BeTest::SetFailOnAssert(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  08/15
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaImportTestFixture::AssertIndex(ECDbCR ecdb, Utf8CP indexName, bool isUnique, Utf8CP tableName, std::vector<Utf8CP> const& columns, std::vector<ECN::ECClassId> const& classIdFilter, bool negateClassIdFilter)
    {
    Utf8String whereClause;
    if (!classIdFilter.empty())
        {
        whereClause.append("ECClassId ");

        if (negateClassIdFilter)
            whereClause.append("NOT ");

        whereClause.append("IN (");

        bool isFirstClassId = true;
        for (ECN::ECClassId classId : classIdFilter)
            {
            if (!isFirstClassId)
                whereClause.append(",");

            Utf8String classIdStr;
            classIdStr.Sprintf("%lld", classId);
            whereClause.append(classIdStr);

            isFirstClassId = false;
            }
        whereClause.append(")");
        }

    AssertIndex(ecdb, indexName, isUnique, tableName, columns, whereClause.c_str());
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
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_STRCASEEQ(expectedDdl.c_str(), stmt.GetValueText(0));
    }


END_ECDBUNITTESTS_NAMESPACE
