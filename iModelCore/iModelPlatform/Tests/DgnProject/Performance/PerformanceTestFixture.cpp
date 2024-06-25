/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PerformanceTestFixture.h"
#include "TestSchemaHelper.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE
//static member initialization
Utf8CP const PerformanceTestFixture::TEST_CLASS_NAME = "PIPE_Extra";
Utf8CP const PerformanceTestFixture::TEST_SCHEMA_NAME = "TestSchema";
Utf8CP const PerformanceTestFixture::TEST_COMPLEX_SCHEMA_NAME = "ComplexTestSchema";
const int PerformanceTestFixture::TESTCLASS_INSTANCE_COUNT = 20000;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestFixture::ImportSchema
(
ECSchemaReadContextR schemaContext, 
ECSchemaR testSchema,
DgnDbR project
)
    {
    StopWatch stopwatch ("PerformanceTestFixture::ImportSchema", true);
    SchemaStatus status = project.ImportSchemas (schemaContext.GetCache().GetSchemas());
    stopwatch.Stop();
    ASSERT_EQ(SchemaStatus::Success, status);

    PERFORMANCELOG.infov (L"PerformanceTestFixture::ImportSchema> Importing ECSchema '%ls' into DgnDb file took %.4lf ms.", 
        testSchema.GetFullSchemaName ().c_str (),
        stopwatch.GetElapsedSeconds() * 1000);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestFixture::ImportComplexTestSchema
(
ECSchemaPtr& testSchema,
DgnDbR project
)
    {
    ECSchemaReadContextPtr schemaReadContext = ECN::ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(project.GetSchemaLocater());

    BeFileName searchDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDir);
    searchDir.AppendToPath(L"ECSchemas").AppendToPath(L"Dgn");
    schemaReadContext->AddSchemaPath(searchDir.GetName());
    BeFileName searchDir2;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDir2);
    searchDir2.AppendToPath(L"ECSchemas").AppendToPath(L"ECDb");
    schemaReadContext->AddSchemaPath(searchDir2.GetName());
    testSchema = TestSchemaHelper::CreateComplexTestSchema (schemaReadContext);
    ASSERT_TRUE (schemaReadContext.IsValid ());

    ImportSchema(*schemaReadContext, *testSchema, project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestFixture::ImportTestSchema
(
ECN::ECSchemaPtr& testSchema,
DgnDbR project,
int numIntProperties,
int numStringProperties
)
    {
    ECSchemaReadContextPtr schemaReadContext = ECN::ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(project.GetSchemaLocater());

    BeFileName searchDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDir);
    searchDir.AppendToPath(L"ECSchemas").AppendToPath(L"Dgn");
    schemaReadContext->AddSchemaPath(searchDir.GetName());
    BeFileName searchDir2;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDir2);
    searchDir2.AppendToPath(L"ECSchemas").AppendToPath(L"ECDb");
    schemaReadContext->AddSchemaPath(searchDir2.GetName());

    testSchema = TestSchemaHelper::CreateTestSchema (schemaReadContext, numIntProperties, numStringProperties);
    ASSERT_TRUE (schemaReadContext.IsValid ());

    ImportSchema(*schemaReadContext, *testSchema, project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void PerformanceTestFixture::AssignRandomECValue 
(
ECValue& ecValue, 
ECPropertyCR ecProperty
)
    {
    ecValue.Clear();

    PrimitiveType type = ecProperty.GetAsPrimitiveProperty ()->GetType ();
    srand ((uint32_t)BeTimeUtilities::QueryMillisecondsCounter ());
    int randomNumber = rand ();
    switch (type)
        {
        case PRIMITIVETYPE_String: 
            {
            Utf8String text;
            text.Sprintf ("Sample text with random number: %d", randomNumber);
            ecValue.SetUtf8CP(text.c_str (), true); 
            }
            break;

        case PRIMITIVETYPE_Integer: 
            {
            ecValue.SetInteger(randomNumber); 
            }
            break;

        case PRIMITIVETYPE_Long: 
            {
            const int32_t intMax = std::numeric_limits<int32_t>::max ();
            const int64_t longValue = static_cast<int64_t> (intMax) + randomNumber;
            ecValue.SetLong(longValue); 
            }
            break;

        case PRIMITIVETYPE_Double: 
            {
            ecValue.SetDouble(randomNumber * PI);
            }
            break;

        case PRIMITIVETYPE_DateTime: 
            {
            DateTime utcTime = DateTime::GetCurrentTimeUtc ();
            ecValue.SetDateTime(utcTime); 
            }
            break;

        case PRIMITIVETYPE_Boolean: 
            {
            ecValue.SetBoolean(randomNumber % 2 != 0); 
            }
            break;

        case PRIMITIVETYPE_Point2d: 
            {
            DPoint2d point2d;
            point2d.x=randomNumber * 1.0;
            point2d.y=randomNumber * 1.8;
            ecValue.SetPoint2d(point2d);
            break;
            }
        case PRIMITIVETYPE_Point3d:
            {
            DPoint3d point3d;
            point3d.x=randomNumber * 1.0;
            point3d.y=randomNumber * 1.8;
            point3d.z=randomNumber * 2.9;
            ecValue.SetPoint3d(point3d);
            break;
            }

        default:
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestFixture::LogResultsToFile(bmap<Utf8String, double> results)
    {
    FILE* logFile=NULL;

    Utf8String dateTime = DateTime::GetCurrentTime().ToString();
    BeFileName dir;
    BeTest::GetHost().GetOutputRoot (dir);
    dir.AppendToPath (L"DgnECPerformanceResults.csv");

    bool existingFile = dir.DoesPathExist();

    fopen_s(&logFile, dir.GetNameUtf8().c_str(), "a+"); 
    PERFORMANCELOG.infov (L"CSV Results filename: %ls\n", dir.GetName());

    if (!existingFile)
        fprintf (logFile, "Date, Test Description, Baseline, Time (secs)\n");

    FOR_EACH(T_TimerResultPair const& pair, results)
        {
        fprintf (logFile, "%s, %s,, %.4f\n", dateTime.c_str(), pair.first.c_str(), pair.second);
        }

    fclose (logFile);

    }


END_DGNDB_UNIT_TESTS_NAMESPACE

