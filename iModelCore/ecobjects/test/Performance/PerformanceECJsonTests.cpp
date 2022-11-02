/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "PerformanceTestFixture.h"

#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct PerformanceTestsECJson : PerformanceTestFixture
{

void SerializeSchemaToJson(ECSchemaPtr schema, bool useDescription = true)
    {
    size_t repeats = PerformanceTestFixture::MinimumRepeats;
    std::vector<double> results; 

    for(size_t i = 0; i < repeats; i++)
        {
        StopWatch serializationTimer("Serialization to JSON", true);
        Json::Value schemaJson;
        EXPECT_TRUE(schema->WriteToJsonValue(schemaJson));
        serializationTimer.Stop();
        results.push_back(serializationTimer.GetElapsedSeconds());

        if(i == 0)
            { //after the first run, recalculate number of repeats
            repeats = CalculateNumberOfRepeats(results[0]);
            }
        }

    double median;
    Utf8String resultJson = GenerateResultJson(results, median);
    LOGPERFDB(TEST_FIXTURE_NAME, (useDescription ? schema->GetDescription().c_str() : schema->GetFullSchemaName().c_str()), median, resultJson.c_str());
    }

void SerializeBisSchemaToJson(WString schemaPath)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(false, true);
    schemaContext->AddSchemaPath(GetAssetsGDataPath({L"ECSchemas", L"ECDb"}).c_str());
    schemaContext->AddSchemaPath(GetAssetsGDataPath({L"ECSchemas", L"Dgn"}).c_str());
    schemaContext->AddSchemaPath(GetAssetsGDataPath({L"ECSchemas", L"Domain"}).c_str());
    ECSchemaPtr schema;
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlFile(schema, schemaPath.c_str(), *schemaContext)) << schemaPath.c_str();

    SerializeSchemaToJson(schema, false);
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceTestsECJson, SerializeFlatSchemasToJson)
    {
    SerializeSchemaToJson(GenerateSchema20000Classes1PropsPerClass());
    SerializeSchemaToJson(GenerateSchema2000Classes10PropsPerClass());
    SerializeSchemaToJson(GenerateSchema100Classes200PropsPerClass());
    SerializeSchemaToJson(GenerateSchema10Classes2000PropsPerClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceTestsECJson, SerializeDeepHierarchySchemasToJson)
    {
    SerializeSchemaToJson(GenerateSchema10Root15Deep3Mixin5PropsAndOverrides());
    SerializeSchemaToJson(GenerateSchema300Root3Deep200Props());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceTestsECJson, SerializeWithCustomAttributes)
    {
    SerializeSchemaToJson(GenerateSchema50Root5Deep3Mixin5Props());
    }


//-------------------------------------------------------------------------------------
//* @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceTestsECJson, SerializeBisSchemas)
    {

    SerializeBisSchemaToJson(GetStandardsPath(L"Units.01.00.07.ecschema.xml"));
    SerializeBisSchemaToJson(GetStandardsPath(L"Formats.01.00.00.ecschema.xml"));

    SerializeBisSchemaToJson(GetAssetsGSchemaPath(L"Dgn", L"BisCore.ecschema.xml"));

    SerializeBisSchemaToJson(GetAssetsGSchemaPath(L"Domain", L"ProcessFunctional.ecschema.xml"));
    SerializeBisSchemaToJson(GetAssetsGSchemaPath(L"Domain", L"ProcessPhysical.ecschema.xml"));

    SerializeBisSchemaToJson(GetAssetsGSchemaPath(L"Domain", L"CifBridge.ecschema.xml"));
    SerializeBisSchemaToJson(GetAssetsGSchemaPath(L"Domain", L"CifCommon.ecschema.xml"));
    SerializeBisSchemaToJson(GetAssetsGSchemaPath(L"Domain", L"CifGeometricRules.ecschema.xml"));
    SerializeBisSchemaToJson(GetAssetsGSchemaPath(L"Domain", L"CifHydraulicAnalysis.ecschema.xml"));
    SerializeBisSchemaToJson(GetAssetsGSchemaPath(L"Domain", L"CifHydraulicResults.ecschema.xml"));
    SerializeBisSchemaToJson(GetAssetsGSchemaPath(L"Domain", L"CifQuantityTakeoffs.ecschema.xml"));
    SerializeBisSchemaToJson(GetAssetsGSchemaPath(L"Domain", L"CifRail.ecschema.xml"));
    SerializeBisSchemaToJson(GetAssetsGSchemaPath(L"Domain", L"CifRoads.ecschema.xml"));
    SerializeBisSchemaToJson(GetAssetsGSchemaPath(L"Domain", L"CifSubsurface.ecschema.xml"));
    SerializeBisSchemaToJson(GetAssetsGSchemaPath(L"Domain", L"CifSubsurfaceConflictAnalysis.ecschema.xml"));
    SerializeBisSchemaToJson(GetAssetsGSchemaPath(L"Domain", L"CifUnits.ecschema.xml"));
    }

END_BENTLEY_ECN_TEST_NAMESPACE
