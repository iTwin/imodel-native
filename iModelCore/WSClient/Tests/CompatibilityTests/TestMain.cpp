/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/


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
    BeTest::SetAssertionFailureHandler([] (wchar_t const*)
        {
        FAIL();
        });

    ::testing::InitGoogleMock(&argc, argv);

    if (::testing::GTEST_FLAG(filter).empty() || ::testing::GTEST_FLAG(filter) == "*")
        ::testing::GTEST_FLAG(filter) = "*RepositoryCompatibilityTests*";

    int runStatus = RUN_ALL_TESTS();

    HttpClient::Uninitialize();

    if (runStatus != 0)
        return runStatus;
    return parseStatus;
    }
