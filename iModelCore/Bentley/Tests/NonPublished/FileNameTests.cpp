/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeFileName.h>
#include <Bentley/Desktop/FileSystem.h>
#if defined(_WIN32) && !defined(BENTLEY_WINRT)
#include <Windows.h>
#endif
#include <stdio.h>
#if defined(_WIN32) && !defined(BENTLEY_WINRT)
#include <Shlwapi.h>
#endif
#define BUFFER_SIZE 200
#define BUFSIZE 65536

#if defined(_WIN32) && !defined(BENTLEY_WINRT)

//-------------------------------------------------------------------------------------------
// @bsimethod                                          Farhad.Kabir                    11/16
//-------------------------------------------------------------------------------------------
TEST(BeFileName_Tests, GetCurrentWorkingDirectory)
    {
    WString currentDirectory;
    TCHAR currentDirectoryExpected[BUFFER_SIZE];
    // Get the current working directory
    GetCurrentDirectory(BUFFER_SIZE, currentDirectoryExpected);
    BeFileName bnew(currentDirectoryExpected);
    ASSERT_EQ(BeFileNameStatus::Success, Desktop::FileSystem::GetCwd(currentDirectory));
    EXPECT_STREQ(bnew.GetName(), currentDirectory.c_str());
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                          Farhad.Kabir                    11/16
//-------------------------------------------------------------------------------------------
TEST(BeFileName_Tests, BeGetTempPath)
    {
    BeFileName bf1(L"");
    //BeFileName  bf2(L"/dir1/dir2/.txt");
    std::wstring strTempPath;
    WChar wchPath[MAX_PATH];
    if (GetTempPathW(MAX_PATH, wchPath))
        strTempPath = wchPath;
    ASSERT_EQ(BeFileNameStatus::Success, Desktop::FileSystem::BeGetTempPath(bf1));
    EXPECT_STREQ(strTempPath.c_str(), bf1.GetName()) << bf1.GetName();
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                          Farhad.Kabir                    11/16
//-------------------------------------------------------------------------------------------
TEST(BeFileName_Tests, BeGetTempFileName)
    {
    TCHAR lpPathBuffer[BUFSIZE] = TEXT("");
    BeFileName befile(lpPathBuffer);
    BeFileName fileName(L"");
    WCharCP prefix = L"DEMO";
    ASSERT_EQ(BeFileNameStatus::Success, Desktop::FileSystem::BeGetTempFileName(fileName, befile, prefix));
    WCharCP expectedFileName = fileName.GetName();
    EXPECT_TRUE(PathFileExistsW(expectedFileName));
    }

#endif
