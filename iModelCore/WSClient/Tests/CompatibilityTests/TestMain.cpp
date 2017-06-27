/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/TestMain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


#include <WebServices/Client/WSError.h>
#include "Parser/ArgumentParser.h"
#include "RepositoryCompatibilityTests.h"
#include "TestsHost.h"

#include <Windows.h>
BeFileName GetProgramPath()
    {
    WChar buffer[1024];
    int bytes = GetModuleFileName(nullptr, buffer, 1024);
    return BeFileName(WString(buffer, bytes));
    }

extern "C"
int main(int argc, char** argv)
    {
    int logLevel = 1;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    int parseStatus = ArgumentParser::Parse(argc, argv, logLevel, workDir, testData, &std::cerr, &std::cout);
    RepositoryCompatibilityTests::SetTestData(testData);

    auto host = TestsHost::Create(GetProgramPath(), workDir, logLevel);
    BeTest::Initialize(*host);


    ::testing::InitGoogleMock(&argc, argv);

    int runStatus = RUN_ALL_TESTS();

    if (runStatus != 0)
        return runStatus;
    return parseStatus;
    }
