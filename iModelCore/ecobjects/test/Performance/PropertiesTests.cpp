/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "PerformanceTestFixture.h"

#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct PerformancePropertiesTest : PerformanceTestFixture 
{
    void AddElementsOneAtATime(ECClassP classA, Utf8StringCR propertyName, Utf8String testcaseName, Utf8String testName);
    void AddElementsOnce(ECClassP classA, Utf8StringCR propertyName, Utf8String testcaseName, Utf8String testName);
};

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
void PerformancePropertiesTest::AddElementsOneAtATime(ECClassP classA, Utf8StringCR propertyName, Utf8String testcaseName, Utf8String testName)
    {
    Utf8Char timerName[256];
    BeStringUtilities::Snprintf(timerName, "Adding 10000 array elements one at a time to %ls", propertyName.c_str());
    StopWatch timer(timerName, false);
    StandaloneECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    timer.Start();
    for (int i = 0; i < 10000; i++)
        instanceA->AddArrayElements(propertyName.c_str(), 1);
    timer.Stop();

    PERFORMANCELOG.infov("%s - %lf", timerName, timer.GetElapsedSeconds());
    LOGTODB(testcaseName.c_str(), testName.c_str(), timer.GetElapsedSeconds(), 10000, Utf8PrintfString("Adding array elements one at a time: %s", propertyName.c_str()).c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
void PerformancePropertiesTest::AddElementsOnce(ECClassP classA, Utf8StringCR propertyName, Utf8String testcaseName, Utf8String testName)
    {
    Utf8Char timerName[256];
    BeStringUtilities::Snprintf(timerName, "Adding 10000 array elements at once to %ls", propertyName.c_str());
    StopWatch timer(timerName, false);
    StandaloneECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    timer.Start();
    instanceA->AddArrayElements(propertyName.c_str(), 10000);
    timer.Stop();

    PERFORMANCELOG.infov("ls - %lf", timerName, timer.GetElapsedSeconds());
    LOGTODB(testcaseName.c_str(), testName.c_str(), timer.GetElapsedSeconds(), 10000, Utf8PrintfString("Adding array elements at once: %s", propertyName.c_str()).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformancePropertiesTest, AddArrayElements)
    {
    ECSchemaPtr schemaA;
    ECSchema::CreateSchema(schemaA, "SchemaA", "ts", 1, 0, 0);
    ECEntityClassP classA;
    schemaA->CreateEntityClass(classA, "ClassA");

    PrimitiveArrayECPropertyP prop;
    classA->CreatePrimitiveArrayProperty(prop, "IntArray", PRIMITIVETYPE_Integer);
    classA->CreatePrimitiveArrayProperty(prop, "StringArray", PRIMITIVETYPE_String);
    classA->CreatePrimitiveArrayProperty(prop, "BoolArray", PRIMITIVETYPE_Boolean);

    ECStructClassP struct1;
    schemaA->CreateStructClass(struct1, "Struct");
    PrimitiveECPropertyP primProp;
    struct1->CreatePrimitiveProperty(primProp, "String", PRIMITIVETYPE_String);

    StructArrayECPropertyP structArrayProp;
    classA->CreateStructArrayProperty(structArrayProp, "StructArray", *struct1);
    AddElementsOneAtATime(classA, "IntArray", TEST_DETAILS);
    AddElementsOneAtATime(classA, "StringArray", TEST_DETAILS);
    AddElementsOneAtATime(classA, "BoolArray", TEST_DETAILS);
    AddElementsOneAtATime(classA, "StructArray",  TEST_DETAILS);
    AddElementsOnce(classA, "IntArray",  TEST_DETAILS);
    AddElementsOnce(classA, "StringArray",  TEST_DETAILS);
    AddElementsOnce(classA, "BoolArray",  TEST_DETAILS);
    AddElementsOnce(classA, "StructArray",  TEST_DETAILS);
    }

END_BENTLEY_ECN_TEST_NAMESPACE