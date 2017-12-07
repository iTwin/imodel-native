/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/PerformanceECDbTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
struct PerformanceECDbTests : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsiclass                                      Krischan.Eberle       07/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECDbTests, CreateECDb)
    {
    static const int opCount = 100;

    std::vector<Utf8String> filePaths;
    for (int i = 0; i < opCount; i++)
        {
        Utf8String fileName;
        fileName.Sprintf("performance_createecdb_%d.ecdb", i);
        BeFileName filePath = BuildECDbPath(fileName.c_str());
        if (filePath.DoesPathExist())
            {
            ASSERT_EQ(BeFileNameStatus::Success, filePath.BeDeleteFile());
            }

        filePaths.push_back(filePath.GetNameUtf8());
        }
    StopWatch timer(true);
    //printf("Attach to profiler"); getchar();
    for (int i = 0; i < opCount; i++)
        {
        Utf8CP filePath = filePaths[(size_t) i].c_str();

        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, ecdb.CreateNewDb(filePath));
        }

    timer.Stop();

    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                      Krischan.Eberle       11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceECDbTests, OpenECDb)
    {
    static const int opCount = 100;

    std::vector<Utf8String> filePaths;
    for (int i = 0; i < opCount; i++)
        {
        Utf8String fileName;
        fileName.Sprintf("performance_createecdb_%d.ecdb", i);
        BeFileName filePath = BuildECDbPath(fileName.c_str());
        if (filePath.DoesPathExist())
            {
            ASSERT_EQ(BeFileNameStatus::Success, filePath.BeDeleteFile());
            }

        filePaths.push_back(filePath.GetNameUtf8());

        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, ecdb.CreateNewDb(filePath));
        }

    StopWatch timer(true);
    //printf("Attach to profiler"); getchar();
    for (int i = 0; i < opCount; i++)
        {
        Utf8CP filePath = filePaths[(size_t) i].c_str();

        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(filePath, ECDb::OpenParams(Db::OpenMode::Readonly)));
        }

    timer.Stop();

    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), opCount);
    }

END_ECDBUNITTESTS_NAMESPACE