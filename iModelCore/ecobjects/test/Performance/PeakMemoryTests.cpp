/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "PerformanceTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

/*
The tests in this file check peak memory usage, which remains at its highest as long as a process is running. So it only makes sense to run a single
test at a time from this file.
*/
struct PeakMemoryTests : PerformanceTestFixture
{
void TestSchema(WString schemaPath, Utf8String testcaseName)
    {
    size_t initialMemory = GetPeakMemoryUsage();

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(false, true);
    schemaContext->AddSchemaPath(GetAssetsGDataPath({L"ECSchemas", L"ECDb"}).c_str());
    schemaContext->AddSchemaPath(GetAssetsGDataPath({L"ECSchemas", L"Dgn"}).c_str());
    schemaContext->AddSchemaPath(GetAssetsGDataPath({L"ECSchemas", L"Domain"}).c_str());
    ECSchemaPtr schema = nullptr;
    const SchemaReadStatus stat = ECN::ECSchema::ReadFromXmlFile(schema, schemaPath.c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, stat);
    ASSERT_TRUE(schema.IsValid());

    size_t finalMemory = GetPeakMemoryUsage();
    double delta = ((double)(finalMemory - initialMemory)) / (1024*1024);
    LOGPERFDB(testcaseName.c_str(), schema->GetFullSchemaName().c_str(), "PeakMemoryDeltaInMB", delta, "{}");
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(PeakMemoryTests, DeserializeBentleyPlant) { TestSchema(ECTestFixture::GetTestDataPath(L"Bentley_Plant.06.00.ecschema.xml").c_str(), TEST_FIXTURE_NAME); }


END_BENTLEY_ECN_TEST_NAMESPACE
