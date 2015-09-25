/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Performance/PerformanceTestingHelpers.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//#ifdef WIP_DOES_NOT_BUILD_ON_ANDROID_OR_IOS

#include "include/PerformanceTestingHelpers.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                    Majd.Uddin                      08/15
//+---------------+---------------+---------------+---------------+---------------+------
PerformanceResultRecorder::PerformanceResultRecorder()
{
    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    BeSQLiteLib::Initialize(temporaryDir);

    EXPECT_TRUE(SetupResultDb()) << "PerformanceResults.db is not setup.";
}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Majd.Uddin                      08/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceResultRecorder::LogToDb(Utf8String testcaseName, Utf8String testName, double timeInSeconds, Utf8String testDescription, int opCount)
{
    PerformanceResultRecorder prr;

    EXPECT_TRUE (prr.writeTodb(timeInSeconds, testcaseName, testName, testDescription, opCount));
}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Majd.Uddin                      08/15
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PerformanceResultRecorder::createDb(Db& db, BeFileName dbName)
{
    if (BeFileName::DoesPathExist(dbName)) //Db is already there
        return BE_SQLITE_ERROR_FileExists;

    //create new Db and add Table
    DbResult result = db.CreateNewDb(dbName.GetNameUtf8().c_str());
    if (result != BE_SQLITE_OK)
    {
        EXPECT_TRUE(false) << "Db Creation failed. Error: " << result;
        return result;
    }
    result = db.CreateTable("PerformanceTestRun", "TestRun INT, TestCaseName TEXT, TestName TEXT, ExecutionTime DOUBLE, TestDescription TEXT, opCount INT");
    if (result != BE_SQLITE_OK)
    {
        EXPECT_TRUE(false) << "Table Creation failed. Error: " << result;
        return result;
    }
    result = db.CreateTable("Counters", "startNum INT, endNum INT, increment INT");
    if (result != BE_SQLITE_OK)
    {
        EXPECT_TRUE(false) << "Table Creation failed. Error: " << result;
        return result;
    }
    //add counters if someone wants to use it
    Statement insert;
    EXPECT_EQ(BE_SQLITE_OK, insert.Prepare(db, "INSERT INTO Counters(startNum, endNum, increment) VALUES (100, 1000, 100)"));
    EXPECT_EQ(BE_SQLITE_DONE, insert.Step());
    db.SaveChanges();
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// Sets up the Db to store Performance Results. Opens if it is there.
// Creates if it is not there and adds tables etc.
// @bsimethod                                    Majd.Uddin                      08/15
//+---------------+---------------+---------------+---------------+---------------+------
bool PerformanceResultRecorder::SetupResultDb()
{
    // Path and name for the Db
    WString dbName(L"PerformanceResults.db");
    BeFileName outPath;
    BeTest::GetHost().GetOutputRoot(outPath);
    outPath.AppendToPath(dbName.c_str());

    // Check if Db Exists
    if (BeFileName::DoesPathExist(outPath))
    {
        DbResult result = m_Db.OpenBeSQLiteDb(outPath.GetNameUtf8().c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
        if (result != BE_SQLITE_OK)
        {
            EXPECT_TRUE(false) << "Cannot open Performance Result Db. Error: " << result;
            return false;
        }
        if (m_Db.TableExists("PerformanceTestRun") && m_Db.TableExists("Counters"))
            return true;
        else
        {
            EXPECT_TRUE(false) << "Table PerformanceTestRun or Counters doens't exist";
            return false;
        }
    }
    // Create Db and Tables
    else
    {
        DbResult result = PerformanceResultRecorder::createDb(m_Db, outPath);
        if (result != BE_SQLITE_OK)
        {
            EXPECT_TRUE(false) << "PerformanceResult.db couldn't be created";
            return false;
        }
        return true;
    }
}
/*---------------------------------------------------------------------------------**//**
*@bsimethod                                            Majd.Uddin          05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PerformanceResultRecorder::getCounters(int& startNum, int& endNum, int& increment)
{
    PerformanceResultRecorder prr;
    if(prr.m_Db.TableExists("Counters"))
    { 
        Statement selectStatement;
        Utf8CP selectQuery = "Select * from Counters";

        DbResult selectPreparetResult = selectStatement.Prepare(prr.m_Db, selectQuery);
        EXPECT_EQ(selectPreparetResult, DbResult::BE_SQLITE_OK);

        if (selectStatement.Step() == DbResult::BE_SQLITE_ROW) // the table should have only one row
        {
            startNum = selectStatement.GetValueInt(0);
            endNum = selectStatement.GetValueInt(1);
            increment = selectStatement.GetValueInt(2);
            selectStatement.Finalize();
            return true;
        }
        return false;
    }
    return false;
}

/*---------------------------------------------------------------------------------**//**
*@bsimethod                                            Majd.Uddin          05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PerformanceResultRecorder::writeTodb(StopWatch &timerCount, Utf8String testcaseName, Utf8String testName, Utf8String testDescription, int opCount)
{
    return writeTodb(timerCount.GetElapsedSeconds(), testcaseName, testName, testDescription, opCount);
}
/*---------------------------------------------------------------------------------**//**
* Writes time to the datbase along with test name and description
* Time is to be sent in seconds.
*@bsimethod                                            Majd.Uddin          05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PerformanceResultRecorder::writeTodb(double timeInSeconds, Utf8String testcaseName, Utf8String testName, Utf8String testDescription, int opCount)
{
    int performanceCount = 0;
    int testRunCount = 0;
    int lastRunCount = 0;
    DbResult selectPreparetResult, insertPreparetResult;
    Statement insertStatement, selectStatement;
    Utf8CP insertQuery = nullptr;
    Utf8CP selectQuery = nullptr;
    Utf8CP testcaseNameInDb, testNameInDb, testDescriptionInDb;
    int opCountInDb;

    selectQuery = "Select * from PerformanceTestRun";
    insertQuery = "INSERT INTO PerformanceTestRun(TestRun,TestCaseName,TestName,ExecutionTime,TestDescription,opCount) VALUES(?,?,?,?,?,?)";
    selectPreparetResult = selectStatement.Prepare(m_Db, selectQuery);
    EXPECT_EQ(selectPreparetResult, DbResult::BE_SQLITE_OK);

    //Determine if an existing entry of test is there and increment it's TestRun
    while (selectStatement.Step() == DbResult::BE_SQLITE_ROW)
    {
        lastRunCount = testRunCount;
        testcaseNameInDb = selectStatement.GetValueText(1);
        testNameInDb = selectStatement.GetValueText(2);
        testDescriptionInDb = selectStatement.GetValueText(4);
        opCountInDb = selectStatement.GetValueInt(5);
        if (testNameInDb == testName && testcaseNameInDb == testcaseName && testDescriptionInDb == testDescription && opCountInDb == opCount)
        {
            testRunCount = selectStatement.GetValueInt(0);
            if (testRunCount >= lastRunCount)
            {
                performanceCount = testRunCount;
            }
        }
    }
    selectStatement.Finalize();
    performanceCount = performanceCount + 1;
    insertPreparetResult = insertStatement.Prepare(m_Db, insertQuery);
    insertStatement.BindInt(1, performanceCount);
    insertStatement.BindText(2, testcaseName, Statement::MakeCopy::No);
    insertStatement.BindText(3, testName, Statement::MakeCopy::No);
    insertStatement.BindDouble(4, timeInSeconds);
    insertStatement.BindText(5, testDescription, Statement::MakeCopy::No);
    insertStatement.BindInt(6, opCount);
    if (insertPreparetResult == DbResult::BE_SQLITE_OK)
    {
        DbResult stepResult = insertStatement.Step();
        if (stepResult == DbResult::BE_SQLITE_DONE)
        {
            insertStatement.Finalize();
            DbResult ChangesResult = m_Db.SaveChanges();
            if (ChangesResult == DbResult::BE_SQLITE_OK)
                return true;
        }
    }
    return false;

}


//---------------------------------------------------------------------------------------
// Creating a new Db for the test
// @bsimethod                                    Majd.Uddin                   06/12
//+---------------+---------------+---------------+---------------+---------------+------
BeFileName PerformanceTestFixtureBase::getDbFilePath(WCharCP dbName)
{
    BeFileName dbFileName;
    BeTest::GetHost().GetOutputRoot(dbFileName);
    dbFileName.AppendToPath(dbName);
    return dbFileName;
}

//---------------------------------------------------------------------------------------
// Creating a new Db for the test
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PerformanceTestFixtureBase::SetupDb(Db& db, WCharCP dbName)
{
    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    BeSQLiteLib::Initialize(temporaryDir);

    BeFileName dbFullName = getDbFilePath(dbName);
    if (BeFileName::DoesPathExist(dbFullName))
        BeFileName::BeDeleteFile(dbFullName);
    DbResult result = db.CreateNewDb(dbFullName.GetNameUtf8().c_str());
    EXPECT_EQ(BE_SQLITE_OK, result) << "Db Creation failed";
    return result;
}

//#endif//def WIP_DOES_NOT_BUILD_ON_ANDROID_OR_IOS
