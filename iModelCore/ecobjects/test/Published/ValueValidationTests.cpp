/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ValueValidationTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

BEGIN_BENTLEY_ECN_TEST_NAMESPACE
using namespace BentleyApi::ECN;

struct ValueValidationTests : ECTestFixture { };

//---------------------------------------------------------------------------------------//
// Verify that the minimum and maximum values are read
// @bsimethod                             Prasanna.Prakash                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
void VerifyMaxAndMinValues(ECPropertyIterable properties)
    {
    for (ECPropertyIterable::const_iterator propertyIterator = properties.begin(); propertyIterator != properties.end(); ++propertyIterator)
        {
        ECPropertyP     testProperty = *propertyIterator;
        
        if (0 == testProperty->GetName().compare("StringWithRange"))
            continue;

        ASSERT_TRUE(testProperty->IsMaximumValueDefined()) << "Failed to read the MaximumValue for " << testProperty->GetName();
        ASSERT_TRUE(testProperty->IsMinimumValueDefined()) << "Failed to read the MinimumValue for " << testProperty->GetName();
        }
    }

//---------------------------------------------------------------------------------------//
// Verify that ECProperties Minimum and Maximum values are read while deserializing
// @bsimethod                             Prasanna.Prakash                       01/2016
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(ValueValidationTests, ECPropertyHasMinAndMaxValues)
    {
    ECSchemaReadContextPtr   context = ECSchemaReadContext::CreateContext();

    ECSchemaPtr inputSchema;
    SchemaReadStatus readInputSchemaStatus = ECSchema::ReadFromXmlFile(inputSchema, ECTestFixture::GetTestDataPath(L"RangeTestSchema.01.01.ecschema.xml").c_str(), *context);
    ASSERT_EQ(SchemaReadStatus::Success, readInputSchemaStatus) << "Failed to load the RangeTest schema.";

    ECClassP        inputTestClass = inputSchema->GetClassP("TestClass");
    ASSERT_NE(inputTestClass, nullptr) << "Failed to load the test class";

    ECPropertyIterable inputSchemaProperties = inputTestClass->GetProperties();
    VerifyMaxAndMinValues(inputSchemaProperties);
    
    WString outputSchemaString;
    SchemaWriteStatus writeStatus = inputSchema->WriteToXmlString(outputSchemaString);
    ASSERT_EQ(SchemaWriteStatus::Success, writeStatus) << "Failed to write the output schema as string";
    
    ECSchemaPtr outputSchema;
    SchemaReadStatus readOutputSchemaStatus = ECSchema::ReadFromXmlString(outputSchema, outputSchemaString.c_str(), *context);
    ASSERT_EQ(SchemaReadStatus::Success, readOutputSchemaStatus) << "Failed to load the output schema file";

    ECClassP        outputTestClass = outputSchema->GetClassP("TestClass");
    ASSERT_NE(outputTestClass, nullptr) << "Failed to load the test class";

    ECPropertyIterable outputSchemaProperties = outputTestClass->GetProperties();
    VerifyMaxAndMinValues(outputSchemaProperties);
    }

END_BENTLEY_ECN_TEST_NAMESPACE