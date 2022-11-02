/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "PerformanceTestFixture.h"
#include <Bentley/BeTimeUtilities.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct PerformanceTestsECXml : PerformanceTestFixture
{

void TimeBisSchema(WString schemaPath)
    {
    std::vector<WString> schemaPaths;
    schemaPaths.push_back(GetAssetsGDataPath({L"ECSchemas", L"ECDb"}));
    schemaPaths.push_back(GetAssetsGDataPath({L"ECSchemas", L"Dgn"}));
    schemaPaths.push_back(GetAssetsGDataPath({L"ECSchemas", L"Domain"}));

    TimeSchemaXmlFile(schemaPath, schemaPaths, false, true, TEST_FIXTURE_NAME);
    }

void TimeInMemorySchema(ECSchemaPtr schema)
    {
    Utf8String schemaXml;
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(schemaXml)) << schema->GetFullSchemaName().c_str();
    TimeSchemaXml(schemaXml, TEST_FIXTURE_NAME);
    }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceTestsECXml, ReadingAndWritingEC2Schemas)
    {
    std::vector<WString> referencePaths;
    referencePaths.push_back(ECTestFixture::GetAssetsDataPath({L"SeedData"}));

    TimeSchemaXmlFile(ECTestFixture::GetTestDataPath(L"OpenPlant.01.02.ecschema.xml"), referencePaths, false, true, TEST_FIXTURE_NAME);
    TimeSchemaXmlFile(ECTestFixture::GetTestDataPath(L"OpenPlant_PID.01.02.ecschema.xml"), referencePaths, false, true, TEST_FIXTURE_NAME);
    TimeSchemaXmlFile(ECTestFixture::GetTestDataPath(L"OpenPlant_3D.01.02.ecschema.xml"), referencePaths, false, true, TEST_FIXTURE_NAME);
    TimeSchemaXmlFile(ECTestFixture::GetTestDataPath(L"Bentley_Plant.06.00.ecschema.xml"), referencePaths, false, true, TEST_FIXTURE_NAME);
    TimeSchemaXmlFile(ECTestFixture::GetTestDataPath(L"CustomAttributeTest.01.00.ecschema.xml"), referencePaths, false, true, TEST_FIXTURE_NAME);
    }

//-------------------------------------------------------------------------------------
//* @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceTestsECXml, ReadingAndWritingGeneratedSchemas)
    {
    TimeInMemorySchema(GenerateSchema20000Classes1PropsPerClass());
    TimeInMemorySchema(GenerateSchema2000Classes10PropsPerClass());
    TimeInMemorySchema(GenerateSchema100Classes200PropsPerClass());
    TimeInMemorySchema(GenerateSchema10Classes2000PropsPerClass());
    TimeInMemorySchema(GenerateSchema10Root15Deep3Mixin5PropsAndOverrides());
    TimeInMemorySchema(GenerateSchema300Root3Deep200Props());
    TimeInMemorySchema(GenerateSchema50Root5Deep3Mixin5Props());
    }

//-------------------------------------------------------------------------------------
//* @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceTestsECXml, ReadingAndWritingBisSchemas)
    {
    TimeBisSchema(GetStandardsPath(L"Units.01.00.07.ecschema.xml"));
    TimeBisSchema(GetStandardsPath(L"Formats.01.00.00.ecschema.xml"));

    TimeBisSchema(GetAssetsGSchemaPath(L"Dgn", L"BisCore.ecschema.xml"));

    TimeBisSchema(GetAssetsGSchemaPath(L"Domain", L"ProcessFunctional.ecschema.xml"));
    TimeBisSchema(GetAssetsGSchemaPath(L"Domain", L"ProcessPhysical.ecschema.xml"));

    TimeBisSchema(GetAssetsGSchemaPath(L"Domain", L"CifBridge.ecschema.xml"));
    TimeBisSchema(GetAssetsGSchemaPath(L"Domain", L"CifCommon.ecschema.xml"));
    TimeBisSchema(GetAssetsGSchemaPath(L"Domain", L"CifGeometricRules.ecschema.xml"));
    TimeBisSchema(GetAssetsGSchemaPath(L"Domain", L"CifHydraulicAnalysis.ecschema.xml"));
    TimeBisSchema(GetAssetsGSchemaPath(L"Domain", L"CifHydraulicResults.ecschema.xml"));
    TimeBisSchema(GetAssetsGSchemaPath(L"Domain", L"CifQuantityTakeoffs.ecschema.xml"));
    TimeBisSchema(GetAssetsGSchemaPath(L"Domain", L"CifRail.ecschema.xml"));
    TimeBisSchema(GetAssetsGSchemaPath(L"Domain", L"CifRoads.ecschema.xml"));
    TimeBisSchema(GetAssetsGSchemaPath(L"Domain", L"CifSubsurface.ecschema.xml"));
    TimeBisSchema(GetAssetsGSchemaPath(L"Domain", L"CifSubsurfaceConflictAnalysis.ecschema.xml"));
    TimeBisSchema(GetAssetsGSchemaPath(L"Domain", L"CifUnits.ecschema.xml"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceTestsECXml, ReadingAndWritingInstance)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    TimeInstance(L"ECRules.01.00.ecschema.xml", L"RuleSet.xml", schemaContext, TEST_FIXTURE_NAME);
    TimeInstance(L"OpenPlant_3D.01.02.ecschema.xml", L"OpenPlant_3D_Instance.xml", schemaContext, TEST_FIXTURE_NAME);
    }

END_BENTLEY_ECN_TEST_NAMESPACE
