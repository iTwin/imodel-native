/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Performance/PropertiesTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "PerformanceTestFixture.h"

#include <Bentley/BeTimeUtilities.h>
using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct PerformancePropertiesTest   : PerformanceTestFixture 
{
    void AddElementsOneAtATime(ECClassP classA, Utf8StringCR propertyName, bmap<Utf8String, double>& results)
        {
        Utf8Char timerName[256];
        BeStringUtilities::Snprintf (timerName, "Adding 10000 array elements one at a time to %ls", propertyName.c_str());
        StopWatch timer(timerName, false);
        StandaloneECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
        timer.Start();
        for (int i = 0; i < 10000; i++)
            instanceA->AddArrayElements(propertyName.c_str(), 1);
        timer.Stop();

        PERFORMANCELOG.infov("%s - %lf", timerName, timer.GetElapsedSeconds());
        results[Utf8String(timerName)] = timer.GetElapsedSeconds();
        }
    void AddElementsOnce(ECClassP classA, Utf8StringCR propertyName, bmap<Utf8String, double>& results)
        {
        Utf8Char timerName[256];
        BeStringUtilities::Snprintf (timerName, "Adding 10000 array elements at once to %ls", propertyName.c_str());
        StopWatch timer(timerName, false);
        StandaloneECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
        timer.Start();
        instanceA->AddArrayElements(propertyName.c_str(), 10000);
        timer.Stop();

        PERFORMANCELOG.infov("ls - %lf", timerName, timer.GetElapsedSeconds());
        results[Utf8String(timerName)] = timer.GetElapsedSeconds();
        }
};

TEST_F(PerformancePropertiesTest, AddArrayElements)
    {
    ECSchemaPtr schemaA;
    ECSchema::CreateSchema (schemaA, "SchemaA", 1, 0);
    ECClassP classA;
    schemaA->CreateClass (classA, "ClassA");

    ArrayECPropertyP prop;
    classA->CreateArrayProperty (prop, "IntArray", PRIMITIVETYPE_Integer);
    classA->CreateArrayProperty (prop, "StringArray", PRIMITIVETYPE_String);
    classA->CreateArrayProperty (prop, "BoolArray", PRIMITIVETYPE_Boolean);

    ECClassP struct1;
    schemaA->CreateClass (struct1, "Struct");
    struct1->SetIsStruct (true);
    PrimitiveECPropertyP primProp;
    struct1->CreatePrimitiveProperty (primProp, "String", PRIMITIVETYPE_String);

    classA->CreateArrayProperty (prop, "StructArray", struct1);
    bmap<Utf8String, double> results;
    AddElementsOneAtATime(classA, "IntArray", results);
    AddElementsOneAtATime(classA, "StringArray", results);
    AddElementsOneAtATime(classA, "BoolArray", results);
    AddElementsOneAtATime(classA, "StructArray", results);
    AddElementsOnce(classA, "IntArray", results);
    AddElementsOnce(classA, "StringArray", results);
    AddElementsOnce(classA, "BoolArray", results);
    AddElementsOnce(classA, "StructArray", results);

    LogResultsToFile(results);
    }

END_BENTLEY_ECN_TEST_NAMESPACE