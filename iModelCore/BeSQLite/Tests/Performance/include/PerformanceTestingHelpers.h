/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Performance/include/PerformanceTestingHelpers.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

/*
    How to use the PerformanceTestFixture?

    The main purpose of this fixture is to allow saving results to Db for further Analysis and Benchmarking.

    This is what you need to do to use it:
    1. In your file, include header file like this:
        #include <UnitTests/BackDoor/PerformanceTestingHelper/PerformanceTestingHelpers.h>
    2. Now when you want to write a Time to the db, add following line:
        LOGTODB(TEST_DETAILS, m_InsertTime, "Sql Insert time", 1000);
       It automatically adds Test case name and Test name. You will need to provide time in seconds and then some additional details and number of operations. Last two arguments are optional.
    6. All entries go into PerformanceResult.Db which is located in your out's build\BeGTest\run\Output\. Look in the PerformanceTestRun table.

Notes:
- You can us PERFORMANCELOG to log info about your tests anywhere.
- You can add additional info about a test in writeToDb(). You can also pass the number of times an action is perfromed to it.
- If same test is executed twice, it gets a new TestRun number so that you can run multiple times and rationalize your result.
- You can also get start, end and increment numbers from the database if you want to run the same test multiple times with different counters.
- Fixture has a basic method to open a Db. More methods will be added.
- The PerformanceResult.db is created on the fly. If it is there, the existing copy is used and data is appended to the Table.
*/
#pragma once
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/DateTime.h>
#include <Logging/bentleylogging.h>

#define PERFORMANCELOG (*NativeLogging::LoggingManager::GetLogger (L"Performance"))
#define LOGTODB PerformanceResultRecorder::writeResults
// Get Test name details
#if defined (USE_GTEST)
    #define TEST_DETAILS ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(), ::testing::UnitTest::GetInstance()->current_test_info()->name()
#else
    #define TEST_DETAILS GetTestCaseNameA(), GetTestNameA()
#endif


//=======================================================================================
// @bsiclass                                                Majd.Uddin     05/2015
//=======================================================================================
struct PerformanceResultRecorder
{

public:
    PerformanceResultRecorder();
    static void writeResults(Utf8String testcaseName, Utf8String testName, double timeInSeconds, Utf8String testDescription = "", int opCount = -1);
    
};


