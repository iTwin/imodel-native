/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Tests/UnitTests/Published/Utils/LogFileHelperTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    BeFileName testDir = logFilesDir;
    BeFileName logFilePath;

    for (int i = 1; i <= 5; i++)
        {
        logFilePath.Sprintf(L"%stestLogFile%d.csv", testDir.c_str(), i);
        if (BeFileStatus::Success != logFile.Create(logFilePath))
            {
            return ERROR;
            }
        logFile.Close();
        }

    for (int i = 1; i <= 3; i++)
        {
        logFilePath.Sprintf(L"%stestNonLogFile%d.txt", testDir.c_str(), i);
        if (BeFileStatus::Success != logFile.Create(logFilePath))
            {
            return ERROR;
            }
        logFile.Close();
        }

    return SUCCESS;
    }

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
