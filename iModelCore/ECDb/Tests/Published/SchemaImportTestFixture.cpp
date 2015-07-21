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
void SchemaImportTestFixture::AssertSchemaImport(ECDb* ecdb, SchemaImportTestItem const& testItem, Utf8CP ecdbFileName) const
    {
    ECDb localECDb;

    ECDb* ecdbP = ecdb != nullptr ? ecdb : &localECDb;

    ASSERT_EQ (BE_SQLITE_OK, ECDbTestUtility::CreateECDb(*ecdbP, nullptr, WString(ecdbFileName, BentleyCharEncoding::Utf8).c_str()));

    auto schemaCache = ECDbTestUtility::ReadECSchemaFromString(testItem.m_schemaXml.c_str());
    ASSERT_TRUE(schemaCache != nullptr) << testItem.m_assertMessage.c_str();

    if (!testItem.m_expectedToSucceed)
        BeTest::SetFailOnAssert(false);

        {
        ASSERT_EQ(testItem.m_expectedToSucceed, SUCCESS == ecdbP->Schemas().ImportECSchemas(*schemaCache)) << testItem.m_assertMessage.c_str();
        }

    BeTest::SetFailOnAssert(true);
    }

END_ECDBUNITTESTS_NAMESPACE
