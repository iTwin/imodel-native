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

    auto schemaCache = ECDbTestUtility::ReadECSchemaFromString(testItem.m_schemaXml.c_str());
    ASSERT_TRUE(schemaCache != nullptr) << testItem.m_assertMessage.c_str();

    if (!testItem.m_expectedToSucceed)
        BeTest::SetFailOnAssert(false);

        {
        ASSERT_EQ(testItem.m_expectedToSucceed, SUCCESS == ecdb.Schemas().ImportECSchemas(*schemaCache)) << testItem.m_assertMessage.c_str();
        asserted = false;
        }

    BeTest::SetFailOnAssert(true);
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

END_ECDBUNITTESTS_NAMESPACE
