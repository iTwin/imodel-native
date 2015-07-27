/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/Performance/PerformanceTestFixture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTestFixture.h"
#include "TestSchemaHelper.h"

USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE
//static member initialization
Utf8CP const PerformanceTestFixture::TEST_CLASS_NAME = "PIPE_Extra";
Utf8CP const PerformanceTestFixture::TEST_SCHEMA_NAME = "TestSchema";
Utf8CP const PerformanceTestFixture::TEST_COMPLEX_SCHEMA_NAME = "ComplexTestSchema";
const int PerformanceTestFixture::TESTCLASS_INSTANCE_COUNT = 20000;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Carole.MacDonald        07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestFixture::ImportSchema
(
ECSchemaReadContextR schemaContext, 
ECSchemaR testSchema,
DgnDbTestDgnManager tdm
)
    {
    DgnDbR project = *tdm.GetDgnProjectP();

    StopWatch stopwatch ("PerformanceTestFixture::ImportSchema", true);
    auto stat = project.Schemas ().ImportECSchemas (schemaContext.GetCache ());
    stopwatch.Stop();
    ASSERT_EQ (SUCCESS, stat);

    PERFORMANCELOG.infov (L"PerformanceTestFixture::ImportSchema> Importing ECSchema '%ls' into DgnDb file took %.4lf ms.", 
        testSchema.GetFullSchemaName ().c_str (),
        stopwatch.GetElapsedSeconds() * 1000);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Carole.MacDonald        07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestFixture::ImportComplexTestSchema
(
ECSchemaPtr& testSchema,
DgnDbTestDgnManager tdm
)
    {
    ECSchemaReadContextPtr schemaReadContext = nullptr;
    testSchema = TestSchemaHelper::CreateComplexTestSchema (schemaReadContext);
    ASSERT_TRUE (schemaReadContext.IsValid ());

    ImportSchema(*schemaReadContext, *testSchema, tdm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Carole.MacDonald        07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestFixture::ImportTestSchema
(
ECN::ECSchemaPtr& testSchema,
DgnDbTestDgnManager tdm,
int numIntProperties,
int numStringProperties
)
    {
    ECSchemaReadContextPtr schemaReadContext = nullptr;
    testSchema = TestSchemaHelper::CreateTestSchema (schemaReadContext, numIntProperties, numStringProperties);
    ASSERT_TRUE (schemaReadContext.IsValid ());

    ImportSchema(*schemaReadContext, *testSchema, tdm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
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

        case PRIMITIVETYPE_Point2D: 
            {
            DPoint2d point2d;
            point2d.x=randomNumber * 1.0;
            point2d.y=randomNumber * 1.8;
            ecValue.SetPoint2D(point2d);
            break;
            }
        case PRIMITIVETYPE_Point3D:
            {
            DPoint3d point3d;
            point3d.x=randomNumber * 1.0;
            point3d.y=randomNumber * 1.8;
            point3d.z=randomNumber * 2.9;
            ecValue.SetPoint3D(point3d);
            break;
            }

        default:
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestFixture::LogResultsToFile(bmap<Utf8String, double> results)
    {
    FILE* logFile=NULL;

    Utf8String dateTime = DateTime::GetCurrentTime().ToUtf8String();
    BeFileName dir;
    BeTest::GetHost().GetOutputRoot (dir);
    dir.AppendToPath (L"DgnECPerformanceResults.csv");

    bool existingFile = dir.DoesPathExist();

    logFile = fopen(dir.GetNameUtf8().c_str(), "a+"); 
    PERFORMANCELOG.infov (L"CSV Results filename: %ls\n", dir.GetName());

    if (!existingFile)
        fprintf (logFile, "Date, Test Description, Baseline, Time (secs)\n");

    FOR_EACH(T_TimerResultPair const& pair, results)
        {
        fprintf (logFile, "%s, %s,, %.4f\n", dateTime.c_str(), pair.first.c_str(), pair.second);
        }

    fclose (logFile);

    }

/*---------------------------------------------------------------------------------**//**
*@bsimethod                                            Majd.Uddin          05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestingFrameWork::setCounters()
{
    openDb();
    Statement selectStatement;
    Utf8CP selectQuery = "Select * from Counters"; //nullptr;
    //selectQuery = "Select * from Counters";

    DbResult selectPreparetResult = selectStatement.Prepare(m_Db, selectQuery);
    EXPECT_EQ(selectPreparetResult, DbResult::BE_SQLITE_OK);

    if (selectStatement.Step() == DbResult::BE_SQLITE_ROW) // the table should have only one row
    {
        m_startNum = selectStatement.GetValueInt(0);
        m_endNum = selectStatement.GetValueInt(1);
        m_increment = selectStatement.GetValueInt(2);
    }
    else
        ASSERT_TRUE(false) << "Setting Counters failed for Performance Test";
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Adeel.Shoukat          07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestingFrameWork::openDb()
{
    // This is hard coded name of the ECDb that stores performance results
    WString dbName(L"PerformanceTest.ecdb");

    if (m_Db.IsDbOpen())
        m_Db.CloseDb();

    DbResult dbOpenStat;
    BeFileName dir;
    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot(outputDir);
    BeSQLiteLib::Initialize(outputDir);
    BeTest::GetHost().GetDocumentsRoot(dir);
    dir.AppendToPath(L"DgnDb");
    dir.AppendToPath(dbName.c_str());

    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    temporaryDir.AppendToPath(dbName.c_str());

    if (dir.DoesPathExist())
    {
        if (!temporaryDir.DoesPathExist())
        {
            if (BeFileName::BeCopyFile(dir, temporaryDir) != BeFileNameStatus::Success)
            {
                ASSERT_FALSE(true) << "Cannot Copy " << dbName.c_str();
            }
            dbOpenStat = m_Db.OpenBeSQLiteDb(temporaryDir.GetNameUtf8().c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
            ASSERT_EQ(BE_SQLITE_OK, dbOpenStat);
        }
        else
        {
            dbOpenStat = m_Db.OpenBeSQLiteDb(temporaryDir.GetNameUtf8().c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
            ASSERT_EQ(BE_SQLITE_OK, dbOpenStat);
        }
    }
}

/*---------------------------------------------------------------------------------**//**
*@bsimethod                                            Majd.Uddin          05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PerformanceTestingFrameWork::writeTodb(StopWatch &timerCount, Utf8String testName, Utf8String testDescription, int opCount)
{
    return writeTodb(timerCount.GetElapsedSeconds(), testName, testDescription, opCount);
}
/*---------------------------------------------------------------------------------**//**
* Writes time to the datbase along with test name and description
* Time is to be sent in seconds.
*@bsimethod                                            Majd.Uddin          05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PerformanceTestingFrameWork::writeTodb(double timeInSeconds, Utf8String testName, Utf8String testDescription, int opCount)
{
    int performanceCount = 0;
    int testRunCount = 0;
    int lastRunCount = 0;
    DbResult selectPreparetResult, insertPreparetResult;
    Statement insertStatement, selectStatement;
    Utf8CP insertQuery = nullptr;
    Utf8CP selectQuery = nullptr;
    Utf8CP testNameInDb;
    openDb();
    selectQuery = "Select * from PerformanceTestRun";
    insertQuery = "INSERT INTO PerformanceTestRun(PerformanceTestRun,TestName,ExecutionTime,DescriptionOfTest,opCount) VALUES(?,?,?,?,?)";
    selectPreparetResult = selectStatement.Prepare(m_Db, selectQuery);
    EXPECT_EQ(selectPreparetResult, DbResult::BE_SQLITE_OK);

    while (selectStatement.Step() == DbResult::BE_SQLITE_ROW)
    {
        lastRunCount = testRunCount;
        testNameInDb = selectStatement.GetValueText(1);
        if (testNameInDb == testName)
        {
            testRunCount = selectStatement.GetValueInt(0);
            if (testRunCount >= lastRunCount)
            {
                performanceCount = testRunCount;
            }
        }
    }
    performanceCount = performanceCount + 1;
    insertPreparetResult = insertStatement.Prepare(m_Db, insertQuery);
    insertStatement.BindInt(1, performanceCount);
    insertStatement.BindText(2, testName, Statement::MakeCopy::No);
    insertStatement.BindDouble(3, timeInSeconds);
    insertStatement.BindText(4, testDescription, Statement::MakeCopy::No);
    insertStatement.BindInt(5, opCount);
    if (insertPreparetResult == DbResult::BE_SQLITE_OK)
    {
        DbResult stepResult = insertStatement.Step();
        if (stepResult == DbResult::BE_SQLITE_DONE)
        {
            DbResult ChangesResult = m_Db.SaveChanges();

            if (ChangesResult == DbResult::BE_SQLITE_OK)
            {
                return true;
            }
        }
    }
    return false;
}



END_DGNDB_UNIT_TESTS_NAMESPACE

