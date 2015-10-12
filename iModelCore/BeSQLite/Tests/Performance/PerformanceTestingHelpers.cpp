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
}


/*---------------------------------------------------------------------------------**//**
* Writes time to a csv file
*@bsimethod                                            Majd.Uddin          10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceResultRecorder::writeResults(Utf8String testcaseName, Utf8String testName, double timeInSeconds, Utf8String testDescription, int opCount)
{
    FILE* logFile = NULL;

    BeFileName dir;
    BeTest::GetHost().GetOutputRoot(dir);
    dir.AppendToPath(L"PerformanceTestResults");
    if (!dir.DoesPathExist())
        BeFileName::CreateNewDirectory(dir.c_str());

    dir.AppendToPath(L"PerformanceResults.csv");

    bool existingFile = dir.DoesPathExist();

    logFile = fopen(dir.GetNameUtf8().c_str(), "a+");
    PERFORMANCELOG.infov(L"CSV Results filename: %ls\n", dir.GetName());

    if (!existingFile)
        fprintf(logFile, "DateTime, TestCaseName, TestName, ExecutionTime, TestDescription, opCount\n");
    Utf8String dateTime = DateTime::GetCurrentTime().ToUtf8String();
    fprintf(logFile, "%s, %s, %s, %.6lf, \"%s\", %ld\n", dateTime.c_str(), testcaseName.c_str(), testName.c_str(), timeInSeconds, testDescription.c_str(), opCount);

    fclose(logFile);
}


//#endif//def WIP_DOES_NOT_BUILD_ON_ANDROID_OR_IOS
