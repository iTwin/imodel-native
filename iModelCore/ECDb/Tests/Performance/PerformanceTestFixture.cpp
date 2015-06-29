/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Performance/PerformanceTestFixture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTestFixture.h"
#include "TestSchemaHelper.h"
#include "DbLogHelper.h"
const int performanceTestRunCount = 5;
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
PerformanceTestFixture::PerformanceTestFixture ()
    {
    }

//******* Setup ***********

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestFixture::SetUp ()
    {
    //establish standard schema search paths (they are in the applications dir)
    BeFileName applicationSchemaDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (applicationSchemaDir);

    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);
    ECDb::Initialize (temporaryDir, &applicationSchemaDir);

    InitializeTestDb ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestFixture::TearDown ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestFixture::CreateEmptyDb
(
ECDbR db,
BeFileNameCR dbPath
)
    {
    // Setup DgnDb file
    if (BeFileName::DoesPathExist (dbPath.GetName ()))
        {
        // Delete any previously created file
        BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile (dbPath.GetName ());
        ASSERT_TRUE (BeFileNameStatus::Success == fileDeleteStatus) << "Could not delete preexisting test dgndb file '" << dbPath.GetName () << "'.";
        }

    DbResult stat = db.CreateNewDb (dbPath.GetNameUtf8 ().c_str ());
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Creation of test DgnDb file failed.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestFixture::SetTestDbPath 
(
BeFileNameCR dbPath
)
    {
    m_testDbPath = dbPath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void PerformanceTestFixture::ImportSchema
(
ECDbR testDb,
ECSchemaPtr schema,
ECSchemaReadContextPtr ecSchemaReadContext
)
    {
    StopWatch stopwatch ("PerformanceTestFixture::ImportSchema", true);
    auto stat = testDb. Schemas ().ImportECSchemas (ecSchemaReadContext->GetCache ());
    stopwatch.Stop();
    ASSERT_EQ (SUCCESS, stat);
    
    LOG.infov (L"PerformanceTestFixture::ImportSchema> Importing ECSchema '%ls' into DgnDb file took %.4lf ms.", 
        schema->GetFullSchemaName ().c_str (),
        stopwatch.GetElapsedSeconds() * 1000);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void PerformanceTestFixture::InsertTestData
(
ECDbR testDb,
ECClassCP ecClass,
int instanceCount
)
    {
    ASSERT_TRUE (ecClass != nullptr);
    StandaloneECEnablerP instanceEnabler = ecClass->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (instanceEnabler != nullptr);

    ECInstanceInserter inserter (testDb, *ecClass);
    if (!inserter.IsValid ())
        {
        return;
        }

    StopWatch stopwatch ("PerformanceTestFixture::InsertTestData", true);
    for (int i = 0; i < instanceCount; ++i)
        {
        InsertTestInstance (*ecClass, *instanceEnabler, inserter);
        }

    stopwatch.Stop();
    LOG.infov (L"PerformanceTestFixture::InsertTestData> Inserting %d rows of random data into class '%ls' in DgnDb file took %.4lf ms.", 
        instanceCount,
        ecClass->GetName ().c_str (),
        stopwatch.GetElapsedSeconds() * 1000);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void PerformanceTestFixture::InsertTestInstance
(
ECClassCR testClass,
StandaloneECEnablerR instanceEnabler,
ECInstanceInserter const& inserter
)
    {
    IECInstancePtr instance = instanceEnabler.CreateInstance ();
    ECValue value;
    for (ECPropertyCP ecProperty : testClass.GetProperties(true))
        {
        if (ecProperty->GetIsPrimitive ())
            {
            AssignRandomECValue (value, *ecProperty);
            instance->SetValue (ecProperty->GetName().c_str(), value);
            }
        }

    ECInstanceKey ecInstanceKey;
    auto insertStatus = inserter.Insert (ecInstanceKey, *instance);
    ASSERT_EQ (SUCCESS, insertStatus);
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
            WString text;
            text.Sprintf (L"Sample text with random number: %d", randomNumber);
            ecValue.SetString(text.c_str (), true); 
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
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void PerformanceTestFixture::ReadECSchemaFromFile
(
ECSchemaPtr& ecSchema, 
ECSchemaReadContextPtr& ecSchemaReadContext, 
WCharCP ecSchemaFileName
)
    {
    // Construct the path to the sample schema
    BeFileName ecSchemaPath;
    BeTest::GetHost().GetDocumentsRoot (ecSchemaPath);
    ecSchemaPath.AppendToPath (L"DgnDb");
    ecSchemaPath.AppendToPath (L"ECDb");
    ecSchemaPath.AppendToPath (L"Schemas");
    BeFileName ecSchemaFile (ecSchemaPath);
    ecSchemaFile.AppendToPath (ecSchemaFileName);
    ASSERT_TRUE (BeFileName::DoesPathExist (ecSchemaFile.GetName()));
    
    // Read the sample schema
    ecSchemaReadContext = ECSchemaReadContext::CreateContext ();

    ecSchemaReadContext->AddSchemaPath (ecSchemaPath.GetName());
    SchemaReadStatus ecSchemaStatus = ECSchema::ReadFromXmlFile (ecSchema, ecSchemaFile.GetName(), *ecSchemaReadContext);
    ASSERT_EQ (ecSchemaStatus, SCHEMA_READ_STATUS_Success);
    ASSERT_TRUE (ecSchema.IsValid());
    }

typedef bpair<Utf8String, double> T_TimerResultPair;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Carole.MacDonald         04/14
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void PerformanceTestFixture::LogResultsToFile
(
bmap<Utf8String, double> results
)
    {
    FILE* logFile=nullptr;

    BeFileName dir;
    BeTest::GetHost().GetOutputRoot (dir);
    WCharCP processorArchitecture = (8 == sizeof(void*)) ? L"Winx64" : L"Winx86";
    dir.AppendToPath(processorArchitecture);
    dir.AppendToPath(L"TestResults");
    if (!dir.DoesPathExist())
        BeFileName::CreateNewDirectory (dir.c_str());

    dir.AppendToPath (L"ECDbPerformanceResults.csv");

    bool existingFile = dir.DoesPathExist();

    logFile = fopen(dir.GetNameUtf8().c_str(), "a+"); 
    LOG.infov (L"CSV Results filename: %ls\n", dir.GetName());

    if (!existingFile)
        fprintf (logFile, "Date, Test Description, Baseline, Time (secs)\n");

    Utf8String dateTime = DateTime::GetCurrentTime().ToUtf8String();
    for (T_TimerResultPair const& pair : results)
        {
        fprintf (logFile, "%s, %s,, %.4f\n", dateTime.c_str(), pair.first.c_str(), pair.second);
        }

    fclose (logFile);

    DbLogHelper logHelper;
    logHelper.LogResults(results);
    }
//******* Test Helpers ***********
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Adeel.Shoukat          07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestFixture::OpenTestDb(ECDbR testDb, Db::OpenMode openMode, DefaultTxn defaultTransactionMode) const
{
    DbResult stat = testDb.OpenBeSQLiteDb(m_testDbPath.GetNameUtf8().c_str(), Db::OpenParams(openMode, defaultTransactionMode));
    ASSERT_EQ(BE_SQLITE_OK, stat);// << L"Opening DgnDb file with mode '" << openMode << L"' failed.";
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Adeel.Shoukat          07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestingFrameWork::openDb()
{
    // This is hard coded name of the ECDb that stores performance results
    WString dbName(L"PerformanceTest.ecdb");
    if (m_Db.IsDbOpen())
    {
        stmt.Finalize();
        m_Db.CloseDb();
    }
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
*@bsimethod                                            Adeel.Shoukat          07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool PerformanceTestingFrameWork::writeTodb(StopWatch &timerCount, Utf8String testName, Utf8String testDescription)
{
    return writeTodb(timerCount.GetElapsedSeconds(), testName, testDescription);
}
/*---------------------------------------------------------------------------------**//**
* Writes time to the datbase along with test name and description
* Time is to be sent in seconds.
*@bsimethod                                            Adeel.Shoukat          07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool PerformanceTestingFrameWork::writeTodb(double timeInSeconds, Utf8String testName, Utf8String testDescription)
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
    insertQuery = "INSERT INTO PerformanceTestRun(PerformanceTestRun,TestName,ExecutionTime,DescriptionOfTest) VALUES(?,?,?,?)";
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
END_ECDBUNITTESTS_NAMESPACE

