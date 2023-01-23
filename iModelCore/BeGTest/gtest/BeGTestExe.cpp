/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32)
    // WIP_BEFILENAME_PORTABILITY - need better way of setting temp directory
    #include <windows.h>
    #include <stdio.h>
    #include <assert.h>
#endif

#include <future>
#include <thread>
#include <chrono>
#include <iostream>

#include <Bentley/BeThread.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/Desktop/FileSystem.h>
#include <Bentley/Logging.h>

USING_NAMESPACE_BENTLEY_LOGGING

struct GtestOptions
    {
    bool umdh           = false; // Whether or not to perform Memory Leak analysis.  Windows-only
    int timeout         = 1800; // in seconds.  The default is 1800 seconds (30 minutes)
    Utf8String gtest_filter;  // String containing the semi-colon separated list of gtest filters.  Can be passed verbatim to gtest.
    Utf8String bsitools_path; // Absolute path of bsitools directory
    };

static void TryParseInput(int argc, char** argv, GtestOptions& options)
    {
    for (int i = 0; i < argc; i++)
        {
        if (0 == strcmp(argv[i], "--umdh"))
            options.umdh = true;
        else // All other options content input after a `=`.
            {
            bvector<Utf8String> tokens;
            BeStringUtilities::Split(argv[i], "=", nullptr, tokens);

            // Index 0 contains the param
            // Index 1 contains the param input
            if (0 == strcmp(tokens[0].c_str(), "--gtest_filter"))
                options.gtest_filter = tokens[1];
            else if (0 == strcmp(tokens[0].c_str(), "--timeout"))
                options.timeout = atoi(tokens[1].c_str());
            else if (0 == strcmp(tokens[0].c_str(), "--bsitools_path"))
                options.bsitools_path = tokens[1];
            }

        }
    }

#if defined(BENTLEY_WIN32)
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SpawnProcessWin32 (char *command, DWORD &returnCode)
    {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    bool ret = false;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Check exit code
    DWORD dwExitCode = 0;

    // Start the child process.
    if( !CreateProcessA (NULL,      // No module name (use command line).
                         command,    // Command line.
                         NULL,       // Process handle not inheritable.
                         NULL,       // Thread handle not inheritable.
                         FALSE,      // Set handle inheritance to FALSE.
                         0,          // No creation flags.
                         NULL,       // Use parent's environment block.
                         NULL,       // Use parent's starting directory.
                         &si,        // Pointer to STARTUPINFO structure.
                         &pi )       // Pointer to PROCESS_INFORMATION structure.
       )
        {
        DWORD error = GetLastError ();
        printf ("error == %d\n", error);
        goto exit;
        }

    // Wait until child process exits.
    WaitForSingleObject (pi.hProcess, INFINITE);

    GetExitCodeProcess (pi.hProcess, &dwExitCode);

    if(dwExitCode == STILL_ACTIVE)
        {
        // Process did not terminate -> force it
        TerminateProcess (pi.hProcess, 0); // Zero is the exit code in this example
        returnCode = 0;
        }
    else
        {
        returnCode = dwExitCode;
        }

    // Close process and thread handles.
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

    ret = true;

exit:
    return ret;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP WinGetEnv(const char * name)
    {
    const DWORD buffSize = 65535;
    static char buffer[buffSize];
    if (GetEnvironmentVariableA(name, buffer, buffSize))
        return buffer;

    return 0;
    }
#endif  // BENTLEY_WIN32

/*---------------------------------------------------------------------------------**//**
* This class knows how data files are linked into the Product/BeGTest directory structure.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct BeGTestHost : RefCounted<BeTest::Host>
    {
    BeFileName m_programPath;

    BeGTestHost (char const* progDir)
        {
        m_programPath = BeFileName (BeFileName::DevAndDir, WString(progDir, true).c_str());
        if ( m_programPath.IsEmpty() ) // We get progdir from argv[0] , if we execute it from CWD argv[0] is blank. in which case BeFileName is not able to resolve full path. So creating path ".\"
            {
            m_programPath.AppendString(L".");
            m_programPath.AppendSeparator();
            }
        m_programPath.BeGetFullPathName ();
        }

    void GetRunRoot (BeFileName& path)
        {
        path = m_programPath;
        path.AppendToPath (L"run");
        }

    void _GetDocumentsRoot (BeFileName& path) override
        {
        path = m_programPath;
        path.AppendToPath (L"Assets");
        path.AppendToPath (L"Documents");
        path.AppendSeparator ();
        }

    void _GetDgnPlatformAssetsDirectory (BeFileName& path) override
        {
        path = m_programPath;
        path.AppendToPath (L"Assets");
        path.AppendSeparator ();
        }

    void _GetOutputRoot (BeFileName& path) override
        {
        GetRunRoot (path);
        path.AppendToPath (L"Output");
        path.AppendSeparator ();
        }

    void _GetTempDir (BeFileName& path) override
        {
        GetRunRoot (path);
        path.AppendToPath (L"Temp");
        path.AppendSeparator ();
        }

    void*  _InvokeP (char const* request, void* arg) override
        {
        if (0==strncmp (request, "foo", 3))
            {
            return new Utf8String ("foo");
            }
        NativeLogging::CategoryLogger("BeTest").errorv ("Unknown upcall %hs", request);
        return NULL;
        }

    static RefCountedPtr<BeGTestHost> Create (char const* progDir) {return new BeGTestHost (progDir);}
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void initLogging ()
    {
#if defined (BENTLEY_WIN32)
    Logging::SetLogger(&ConsoleLogger::GetLogger());
#endif
    }

static void recreateDir(BeFileNameCR dirName)
    {
    BeFileName::EmptyAndRemoveDirectory (dirName.c_str());
    BeFileName::CreateNewDirectory (dirName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* This is the TestListener that can act on certain events
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
class BeGTestListener : public ::testing::EmptyTestEventListener
    {
    Utf8String m_currTestCaseName;
    Utf8String m_currTestName;

    virtual void OnTestStart(const ::testing::TestInfo& test_info) override
        {
        m_currTestCaseName = test_info.test_case_name();
        m_currTestName = test_info.name();

        // Remove the temp directory at the start of every test to simulate the transitory nature of temp on some platforms.
        BeFileName dirName;
        BeTest::GetHost().GetTempDir(dirName);
        recreateDir(dirName);
        }

    virtual void OnTestEnd(const ::testing::TestInfo& test_info) override
        {
        // In case a test disabled asserts and forget to re-enable them!
        if (!BeTest::GetFailOnAssert())
            FAIL() << "This test called BeTest::SetFailOnAssert(false) and failed to call it again with true.";
        BeTest::SetFailOnAssert(true);
        }

    // Called after a failed assertion or a SUCCEED() invocation.
    virtual void OnTestPartResult(const ::testing::TestPartResult& test_part_result) { }

    virtual void OnTestProgramEnd(const ::testing::UnitTest& unit_test) override
        {
        fprintf (stderr, "\n\n%d tests passed, %d tests failed, %d tests were disabled\n", unit_test.successful_test_count(), unit_test.failed_test_count(), unit_test.disabled_test_count());
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static int runAllTestsWithTimeout(uint64_t timeoutInSeconds)
    {
    std::promise<bool> promisedFinished;
    auto futureResult = promisedFinished.get_future();
    int testResult = 0;
    bool didComplete = false;

    std::thread ([&]
        {
        BeThreadUtilities::SetCurrentThreadName("Test Runner Thread");
        BeTest::setS_mainThreadId(BeThreadUtilities::GetCurrentThreadId());
        testResult = RUN_ALL_TESTS();
        didComplete = true;
        promisedFinished.set_value(true);
        }).detach();

    EXPECT_TRUE(std::future_status::ready == futureResult.wait_for(std::chrono::seconds(timeoutInSeconds)));

    if (!didComplete)
        {
        printf("\n");
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        printf("ERROR: Test suite exceeded %" PRIu64 " second(s) and was aborted.\n", timeoutInSeconds);
        printf("       Errors reported above may either be due to the termination of the test,\n");
        printf("       or may be clues for the hang and/or deadlock.\n");
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        printf("\n");

        return 1;
        }

    return testResult;
    }

#if defined(BENTLEY_WIN32)
static void runUmdh(BeGTestHost& host, GtestOptions options)
    {
    char strCommand[1024];
    bool spawnRet;
    DWORD retCode;

    RUN_ALL_TESTS();

    BeFileName outdirName;
    host.GetOutputRoot(outdirName);
    outdirName.PopDir();
    WString currentDirectory(outdirName.GetName());
    //set _NT_SYMBOL_PATH
    printf("%d\n", SetEnvironmentVariableW(L"_NT_SYMBOL_PATH", currentDirectory.c_str()));
    printf("Symbols path is   :   %s\n", WinGetEnv("_NT_SYMBOL_PATH"));

    Utf8String gflagsSet;
    BeStringUtilities::WCharToUtf8(gflagsSet, host.m_programPath.c_str());

    BeFileName gflagsPath(options.bsitools_path);
    gflagsPath.AppendToPath(L"gflags.exe");

    Utf8String setGflags;
    BeStringUtilities::WCharToUtf8(setGflags, gflagsPath.c_str());
    setGflags.append(" -i ").append(gflagsSet).append(" +ust");

    sprintf_s(strCommand, sizeof(strCommand), setGflags.c_str());
    spawnRet = SpawnProcessWin32(strCommand, retCode);

    //first snapshot
    BeFileName pathSnapshotPath(currentDirectory);
    pathSnapshotPath.AppendToPath(L"FirstSnapshot.log");

    BeFileName umdhPath(options.bsitools_path);
        umdhPath.AppendToPath(L"umdh.exe");

    Utf8String umdhPathJoin;
    BeStringUtilities::WCharToUtf8(umdhPathJoin, umdhPath.c_str());

    Utf8String pathSnapshot1;
    BeStringUtilities::WCharToUtf8(pathSnapshot1, pathSnapshotPath.c_str());

    bvector<Utf8CP> snapshot1 = {umdhPathJoin.c_str(), " -p:%ld -f:", pathSnapshot1.c_str()  };
    Utf8String generateSnapshot1 =  BeStringUtilities::Join(snapshot1);

    // For debugging purposes, take initial snapshot of memory
    sprintf_s(strCommand, sizeof(strCommand), generateSnapshot1.c_str(), GetCurrentProcessId());
    spawnRet = SpawnProcessWin32(strCommand, retCode);

    for (int i = 0; i < 5; i++)
        RUN_ALL_TESTS();

    // Set the path comparison at the same level as the output root
    BeFileName pathComparison(outdirName);
    if (options.gtest_filter.empty())
        pathComparison.AppendToPath(L"Comparison.log");
    else
        {
        // If filename has '/' in it then replace it with '-'
        if (options.gtest_filter.Contains("/"))
            {
            Utf8String fileName = options.gtest_filter.c_str();
            fileName.ReplaceAll("/", "-");
            pathComparison.AppendSeparator()
                .AppendUtf8(fileName.c_str())
                .append(L".log");
            }
        else
            pathComparison.AppendSeparator()
                .AppendUtf8(options.gtest_filter.c_str())
                .append(L".log");
        }

    BeFileName snapshotPath(outdirName);
    snapshotPath.AppendToPath(L"SecondSnapshot.log");

    Utf8String pathSnapshot2;
    BeStringUtilities::WCharToUtf8(pathSnapshot2, snapshotPath.c_str());
    Utf8String pathComparison2;
    BeStringUtilities::WCharToUtf8(pathComparison2, pathComparison.c_str());

    Utf8String generateSnapshot2(umdhPathJoin.c_str());
    generateSnapshot2.append(" -p:%ld -f:").append(pathSnapshot2.c_str());

    // For debugging purposes, take terminal snapshot of memory
    sprintf_s(strCommand, sizeof(strCommand), generateSnapshot2.c_str(), GetCurrentProcessId());
    spawnRet = SpawnProcessWin32(strCommand, retCode);

    Utf8String generateComparisonLog(umdhPathJoin.c_str());
    generateComparisonLog.append(" -d ").append(pathSnapshot1.c_str())
        .append(" ").append(pathSnapshot2.c_str())
        .append(" -f:").append(pathComparison2.c_str());

    // Now take a diff of the two dumps
    sprintf_s(strCommand, sizeof(strCommand), generateComparisonLog.c_str());
    spawnRet = SpawnProcessWin32(strCommand, retCode);

    printf(strCommand, "\n");

    Utf8String gflagsDisable;
    BeStringUtilities::WCharToUtf8(gflagsDisable, gflagsPath.c_str());
    gflagsDisable.append(" -i ").append(gflagsSet.c_str()).append(" -ust");

    sprintf_s(strCommand, sizeof(strCommand), gflagsDisable.c_str());
    spawnRet = SpawnProcessWin32(strCommand, retCode);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C"
int main(int argc, char **argv)
    {
    GtestOptions options;
    TryParseInput(argc, argv, options);

    auto hostPtr = BeGTestHost::Create(argv[0]);

    //  Make sure output directies exist (and remove any results hanging around from a previous run)
    BeFileName dirName;
    hostPtr->GetOutputRoot(dirName);
    recreateDir(dirName);
    hostPtr->GetTempDir(dirName);
    recreateDir(dirName);

    //  Finish initializing libraries
    BeTest::Initialize(*hostPtr);
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    //listener with test failure output
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new BeGTestListener);

    initLogging();

    //  Configure the test runner
    BeTest::SetRunningUnderGtest();

    BeTest::SetAssertionFailureHandler([](WCharCP msg)
        {
        FAIL() << msg;
        });

    if (::testing::GTEST_FLAG(filter).empty() || ::testing::GTEST_FLAG(filter) == "*")
        { // use ignore lists if the user did not specify any filters on the command line
        Utf8String toignore, torun;
        BeTest::LoadFilters(toignore, torun);
        bastring filters;
        if (!toignore.empty())
            filters.assign("-").append(toignore);
        if (!torun.empty())
            {
            if (!filters.empty())
                filters.append(":");
            filters.assign("+").append(torun);
            }
        if (!filters.empty())
            ::testing::GTEST_FLAG(filter) = filters.c_str();
        }

    // When umdh is used there is an entirely different code path.
    if (options.umdh)
        {
        #if defined(BENTLEY_WIN32)
            runUmdh(*hostPtr, options);
            return 0; // Return without any errors reported.
        #else
            printf("Running with umdh is only supported on Windows platforms.");
            return 0;
        #endif
        }

    // Always run tests with a timeout unless "-1" is provided
    int errors = 0;
    if (-1 != options.timeout)
        errors = runAllTestsWithTimeout(options.timeout);
    else
        errors = RUN_ALL_TESTS(); //run tests without timeout

    return errors;
    }
