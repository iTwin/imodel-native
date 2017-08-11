/*--------------------------------------------------------------------------------------+
|
|     $Source: test/TestFixture/TestFixture.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>
#include "../ECObjectsTestPCH.h"

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

#define EC_ASSERT_SUCCESS(actual) ASSERT_EQ(ECObjectsStatus::Success, actual)
#define EC_EXPECT_SUCCESS(actual) EXPECT_EQ(ECObjectsStatus::Success, actual)

struct ECTestFixture : ::testing::Test
    {
protected:
    ECTestFixture();
    
public:

    static WString GetTestDataPath(WCharCP fileName);
    static WString GetTempDataPath(WCharCP fileName);
    static Utf8String GetDateTime();
    };

struct ECTestUtility
    {
    static void ExpectSchemaDeserializationFailure(Utf8CP schemaXml, ECN::SchemaReadStatus expectedError = ECN::SchemaReadStatus::InvalidECSchemaXml);
    static void AssertSchemaDeserializationFailure(Utf8CP schemaXml, ECN::SchemaReadStatus expectedError = ECN::SchemaReadStatus::InvalidECSchemaXml);
    };

END_BENTLEY_ECN_TEST_NAMESPACE
