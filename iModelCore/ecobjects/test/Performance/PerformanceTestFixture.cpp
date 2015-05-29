/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Performance/PerformanceTestFixture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "PerformanceTestFixture.h"

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestFixture::LogResultsToFile(bmap<Utf8String, double> results)
    {
    FILE* logFile=NULL;

    BeFileName dir;
    BeTest::GetHost().GetOutputRoot (dir);
    WCharCP processorArchitecture = (8 == sizeof(void*)) ? L"Winx64" : L"Winx86";
    dir.AppendToPath(processorArchitecture);
    dir.AppendToPath(L"TestResults");
    if (!dir.DoesPathExist())
        BeFileName::CreateNewDirectory (dir.c_str());

    dir.AppendToPath (L"ECObjectsPerformanceResults.csv");

    bool existingFile = dir.DoesPathExist();

    logFile = fopen(dir.GetNameUtf8().c_str(), "a+"); 
    PERFORMANCELOG.infov (L"CSV Results filename: %ls\n", dir.GetName());

    if (!existingFile)
        fprintf (logFile, "Date, Test Description, Baseline, Time (secs)\n");

    Utf8String dateTime = DateTime::GetCurrentTime().ToUtf8String();
    FOR_EACH(T_TimerResultPair const& pair, results)
        {
        fprintf (logFile, "%s, %s,, %.4f\n", dateTime.c_str(), pair.first.c_str(), pair.second);
        }

    fclose (logFile);

    }

END_BENTLEY_ECN_TEST_NAMESPACE
