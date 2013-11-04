/*--------------------------------------------------------------------------------------+
|
|     $Source: tests/Performance/PropertiesTests.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "PerformanceTestFixture.h"

#include <Bentley/BeTimeUtilities.h>
using namespace Bentley::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct PerformancePropertiesTest   : PerformanceTestFixture 
{
    void AddElementsOneAtATime(ECClassP classA, WStringCR propertyName, bmap<Utf8String, double>& results)
        {
        wchar_t timerName[256];
        BeStringUtilities::Snwprintf (timerName, L"Adding 10000 array elements one at a time to %ls", propertyName.c_str());
        StopWatch timer(timerName, false);
        StandaloneECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
        timer.Start();
        for (int i = 0; i < 10000; i++)
            instanceA->AddArrayElements(propertyName.c_str(), 1);
        timer.Stop();

        PERFORMANCELOG.infov("%ls - %lf", timerName, timer.GetElapsedSeconds());
        results[Utf8String(timerName)] = timer.GetElapsedSeconds();
        }
    void AddElementsOnce(ECClassP classA, WStringCR propertyName, bmap<Utf8String, double>& results)
        {
        wchar_t timerName[256];
        BeStringUtilities::Snwprintf (timerName, L"Adding 10000 array elements at once to %ls", propertyName.c_str());
        StopWatch timer(timerName, false);
        StandaloneECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
        timer.Start();
        instanceA->AddArrayElements(propertyName.c_str(), 10000);
        timer.Stop();

        PERFORMANCELOG.infov("%ls - %lf", timerName, timer.GetElapsedSeconds());
        results[Utf8String(timerName)] = timer.GetElapsedSeconds();
        }
};

TEST_F(PerformancePropertiesTest, AddArrayElements)
    {
    ECSchemaPtr schemaA;
    ECSchema::CreateSchema (schemaA, L"SchemaA", 1, 0);
    ECClassP classA;
    schemaA->CreateClass (classA, L"ClassA");

    ArrayECPropertyP prop;
    classA->CreateArrayProperty (prop, L"IntArray", PRIMITIVETYPE_Integer);
    classA->CreateArrayProperty (prop, L"StringArray", PRIMITIVETYPE_String);
    classA->CreateArrayProperty (prop, L"BoolArray", PRIMITIVETYPE_Boolean);

    ECClassP struct1;
    schemaA->CreateClass (struct1, L"Struct");
    struct1->SetIsStruct (true);
    PrimitiveECPropertyP primProp;
    struct1->CreatePrimitiveProperty (primProp, L"String", PRIMITIVETYPE_String);

    classA->CreateArrayProperty (prop, L"StructArray", struct1);
    bmap<Utf8String, double> results;
    AddElementsOneAtATime(classA, L"IntArray", results);
    AddElementsOneAtATime(classA, L"StringArray", results);
    AddElementsOneAtATime(classA, L"BoolArray", results);
    AddElementsOneAtATime(classA, L"StructArray", results);
    AddElementsOnce(classA, L"IntArray", results);
    AddElementsOnce(classA, L"StringArray", results);
    AddElementsOnce(classA, L"BoolArray", results);
    AddElementsOnce(classA, L"StructArray", results);

    LogResultsToFile(results);
    }

END_BENTLEY_ECN_TEST_NAMESPACE