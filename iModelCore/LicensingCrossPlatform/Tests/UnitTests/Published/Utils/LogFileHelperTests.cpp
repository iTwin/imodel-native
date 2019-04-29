/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/Utils/LogFileHelperTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Licensing/Utils/LogFileHelper.h>
#include <BeSQLite/BeSQLite.h>

#include "LogFileHelperTests.h"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

BentleyStatus CreateLogFiles(BeFileNameCR logFilesDir)
    {
    BeFile logFile;

    for (int i = 1; i <= 5; i++)
        {
        BeFileName logFilePath = logFilesDir;
        WString fileName;

        fileName.Sprintf(L"testLogFile%d.csv", i);
        logFilePath.AppendToPath(fileName.GetWCharCP());

        if (BeFileStatus::Success != logFile.Create(logFilePath))
            {
            return ERROR;
            }
        logFile.Close();
        }

    for (int i = 1; i <= 3; i++)
        {
        BeFileName logFilePath = logFilesDir;
        WString fileName;

        fileName.Sprintf(L"testNonLogFile%d.txt", i);
        logFilePath.AppendToPath(fileName.GetWCharCP());

        if (BeFileStatus::Success != logFile.Create(logFilePath))
            {
            return ERROR;
            }
        logFile.Close();
        }

    return SUCCESS;
    }

// failing on linux, logFile.Create("/testLogFile1.csv") is failing
TEST_F(LogFileHelperTests, GetLogFiles_Success)
    {
    BeFileName logFilesDir;
    bvector<WString> logFiles;

    BeTest::GetHost().GetTempDir(logFilesDir);
    EXPECT_SUCCESS(CreateLogFiles(logFilesDir));

    LogFileHelper lfh;
    logFiles = lfh.GetLogFiles(Utf8String(logFilesDir));
    EXPECT_EQ(logFiles.size(), 5);
    }
