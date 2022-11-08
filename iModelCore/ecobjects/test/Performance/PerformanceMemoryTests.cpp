/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "PerformanceTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ECSchemaMemoryTests : PerformanceTestFixture
{
void TestSchema(WString schemaPath)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(false, true);
    schemaContext->AddSchemaPath(GetAssetsGDataPath({L"ECSchemas", L"ECDb"}).c_str());
    schemaContext->AddSchemaPath(GetAssetsGDataPath({L"ECSchemas", L"Dgn"}).c_str());
    schemaContext->AddSchemaPath(GetAssetsGDataPath({L"ECSchemas", L"Domain"}).c_str());

    MeasureSchemaMemory(schemaPath, schemaContext, TEST_FIXTURE_NAME);
    }
};

TEST_F(ECSchemaMemoryTests, SizeOfObjects)
    {
    BeJsDocument ecSizes;
    ecSizes["ECSchema"] = (int) sizeof(ECSchema);
    ecSizes["ECClass"] = (int) sizeof(ECClass);
    ecSizes["ECEntityClass"] = (int) sizeof(ECEntityClass);
    ecSizes["ECCustomAttributeClass"] = (int) sizeof(ECCustomAttributeClass);
    ecSizes["ECStructClass"] = (int) sizeof(ECStructClass);
    ecSizes["ECRelationshipClass"] = (int) sizeof(ECRelationshipClass);
    ecSizes["ECRelationshipConstraint"] = (int) sizeof(ECRelationshipConstraint);
    ecSizes["ECProperty"] = (int) sizeof(ECProperty);
    ecSizes["PrimitiveECProperty"] = (int) sizeof(PrimitiveECProperty);
    ecSizes["StructECProperty"] = (int) sizeof(StructECProperty);
    ecSizes["PrimitiveArrayECProperty"] = (int) sizeof(PrimitiveArrayECProperty);
    ecSizes["StructArrayECProperty"] = (int) sizeof(StructArrayECProperty);
    ecSizes["NavigationECProperty"] = (int) sizeof(NavigationECProperty);
    ecSizes["ArrayECProperty"] = (int) sizeof(ArrayECProperty);
    ecSizes["ECEnumeration"] = (int) sizeof(ECEnumeration);
    ecSizes["ECEnumerator"] = (int) sizeof(ECEnumerator);
    ecSizes["KindOfQuantity"] = (int) sizeof(KindOfQuantity);
    ecSizes["ECUnit"] = (int) sizeof(ECUnit);
    ecSizes["UnitSystem"] = (int) sizeof(UnitSystem);
    ecSizes["Phenomenon"] = (int) sizeof(Phenomenon);
    ecSizes["ECFormat"] = (int) sizeof(ECFormat);
    ecSizes["NamedFormat"] = (int) sizeof(NamedFormat);
    ecSizes["IECCustomAttributeContainer"] = (int) sizeof(IECCustomAttributeContainer);

    size_t totalSize = 0;
    ecSizes.ForEachArrayMemberValue([&](BeJsValue::ArrayIndex, BeJsValue size)
        {
        totalSize += size.asUInt64();
        return false;
        });

    LOGPERFDB(TEST_DETAILS, "sizeof for all EC structs", (double)totalSize, ecSizes.Stringify().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECSchemaMemoryTests, BisSchemasUnits) { TestSchema(GetStandardsPath(L"Units.01.00.07.ecschema.xml")); }
TEST_F(ECSchemaMemoryTests, BisSchemasFormats) { TestSchema(GetStandardsPath(L"Formats.01.00.00.ecschema.xml")); }

TEST_F(ECSchemaMemoryTests, BisSchemasBisCore) { TestSchema(GetAssetsGSchemaPath(L"Dgn", L"BisCore.ecschema.xml")); }

TEST_F(ECSchemaMemoryTests, BisSchemasProcessFunctional) { TestSchema(GetAssetsGSchemaPath(L"Domain", L"ProcessFunctional.ecschema.xml")); }
TEST_F(ECSchemaMemoryTests, BisSchemasProcessPhysical) { TestSchema(GetAssetsGSchemaPath(L"Domain", L"ProcessPhysical.ecschema.xml")); }

TEST_F(ECSchemaMemoryTests, BisSchemasCifBridge) { TestSchema(GetAssetsGSchemaPath(L"Domain", L"CifBridge.ecschema.xml")); }
TEST_F(ECSchemaMemoryTests, BisSchemasCifCommon) { TestSchema(GetAssetsGSchemaPath(L"Domain", L"CifCommon.ecschema.xml")); }
TEST_F(ECSchemaMemoryTests, BisSchemasCifGeometricRules) { TestSchema(GetAssetsGSchemaPath(L"Domain", L"CifGeometricRules.ecschema.xml")); }
TEST_F(ECSchemaMemoryTests, BisSchemasCifHydraulicAnalysis) { TestSchema(GetAssetsGSchemaPath(L"Domain", L"CifHydraulicAnalysis.ecschema.xml")); }
TEST_F(ECSchemaMemoryTests, BisSchemasCifHydraulicResults) { TestSchema(GetAssetsGSchemaPath(L"Domain", L"CifHydraulicResults.ecschema.xml")); }
TEST_F(ECSchemaMemoryTests, BisSchemasCifQuantityTakeoffs) { TestSchema(GetAssetsGSchemaPath(L"Domain", L"CifQuantityTakeoffs.ecschema.xml")); }
TEST_F(ECSchemaMemoryTests, BisSchemasCifRail) { TestSchema(GetAssetsGSchemaPath(L"Domain", L"CifRail.ecschema.xml")); }
TEST_F(ECSchemaMemoryTests, BisSchemasCifRoads) { TestSchema(GetAssetsGSchemaPath(L"Domain", L"CifRoads.ecschema.xml")); }
TEST_F(ECSchemaMemoryTests, BisSchemasCifSubsurface) { TestSchema(GetAssetsGSchemaPath(L"Domain", L"CifSubsurface.ecschema.xml")); }
TEST_F(ECSchemaMemoryTests, BisSchemasCifSubsurfaceConflictAnalysis) { TestSchema(GetAssetsGSchemaPath(L"Domain", L"CifSubsurfaceConflictAnalysis.ecschema.xml")); }
TEST_F(ECSchemaMemoryTests, BisSchemasCifUnits) { TestSchema(GetAssetsGSchemaPath(L"Domain", L"CifUnits.ecschema.xml")); }

TEST_F(ECSchemaMemoryTests, SyntheticSchemasMemoryTest1) { MeasureSchemaMemoryUsage(PerformanceTestFixture::GenerateSchema20000Classes1PropsPerClass, TEST_FIXTURE_NAME); }
TEST_F(ECSchemaMemoryTests, SyntheticSchemasMemoryTest2) { MeasureSchemaMemoryUsage(PerformanceTestFixture::GenerateSchema2000Classes10PropsPerClass, TEST_FIXTURE_NAME); }
TEST_F(ECSchemaMemoryTests, SyntheticSchemasMemoryTest3) { MeasureSchemaMemoryUsage(PerformanceTestFixture::GenerateSchema100Classes200PropsPerClass, TEST_FIXTURE_NAME); }
TEST_F(ECSchemaMemoryTests, SyntheticSchemasMemoryTest4) { MeasureSchemaMemoryUsage(PerformanceTestFixture::GenerateSchema10Classes2000PropsPerClass, TEST_FIXTURE_NAME); }
TEST_F(ECSchemaMemoryTests, SyntheticSchemasMemoryTest5) { MeasureSchemaMemoryUsage(PerformanceTestFixture::GenerateSchema10Root15Deep3Mixin5PropsAndOverrides, TEST_FIXTURE_NAME); }
TEST_F(ECSchemaMemoryTests, SyntheticSchemasMemoryTest6) { MeasureSchemaMemoryUsage(PerformanceTestFixture::GenerateSchema300Root3Deep200Props, TEST_FIXTURE_NAME); }
TEST_F(ECSchemaMemoryTests, SyntheticSchemasMemoryTest7) { MeasureSchemaMemoryUsage(PerformanceTestFixture::GenerateSchema50Root5Deep3Mixin5Props, TEST_FIXTURE_NAME); }


END_BENTLEY_ECN_TEST_NAMESPACE
