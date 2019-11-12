/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32)
    // WIP_BEFILENAME_PORTABILITY - need better way of setting temp directory
    #include <windows.h>
#endif

#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>
#include <Logging/bentleylogging.h>
#include <Bentley/Desktop/FileSystem.h>

/*---------------------------------------------------------------------------------**//**
* This class knows how data files are linked into the Product/ECObjectsXXXTests directory structure.
* @bsimethod                                    Sam.Wilson                      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct BeGTestHost : RefCounted<BeTest::Host>
    {
    BeFileName m_programPath;
    WString    m_programName;

    BeGTestHost (char const* argv0)
        {
        WString programFullPath = WString(argv0, true);
        m_programPath = BeFileName (BeFileName::DevAndDir, programFullPath.c_str());
        if (m_programPath.IsEmpty()) // We get progdir from argv[0] , if we execute it from CWD argv[0] is blank. in which case BeFileName is not able to resolve full path. So creating path ".\" 
            {
            m_programPath.AppendString(L".");
            m_programPath.AppendSeparator();
            }
        m_programPath.BeGetFullPathName ();
        m_programName = BeFileName::GetFileNameWithoutExtension (programFullPath.c_str());
        }
    void GetRunRoot (char const* argv0, BeFileName& path)
        {
        WString programFullPath = WString(argv0, true);
        BeFileName programPathCurr = BeFileName (BeFileName::DevAndDir, programFullPath.c_str());
        if ( programPathCurr.IsEmpty() ) // We get progdir from argv[0] , if we execute it from CWD argv[0] is blank. in which case BeFileName is not able to resolve full path. So creating path ".\" 
            {
            programPathCurr.AppendString(L".");
            programPathCurr.AppendSeparator();
            }
        programPathCurr.BeGetFullPathName ();
        path = programPathCurr;
        path.AppendToPath (L"run");
        }
    virtual void _GetDocumentsRoot (BeFileName& path) override              {path = m_programPath;}
    virtual void _GetDgnPlatformAssetsDirectory (BeFileName& path) override {path = m_programPath; path.AppendToPath(L"assets");}
    virtual void _GetTempDir(BeFileName& path) override
        {
        path = m_programPath;
        path.AppendToPath(L"run");
        path.AppendToPath(L"Temp");
        }

    virtual void _GetOutputRoot (BeFileName& path) override
        {
        path = m_programPath;
        path.AppendToPath(L"run");
        path.AppendToPath(L"Output");
#if defined (USE_GTEST)
        if (::testing::UnitTest::GetInstance() && ::testing::UnitTest::GetInstance()->current_test_info())
            {
            BentleyApi::WString testCaseName ;
            BeStringUtilities::Utf8ToWChar(testCaseName,::testing::UnitTest::GetInstance()->current_test_info()->test_case_name());
            path.AppendToPath(testCaseName.c_str());
            path.AppendSeparator();

            BentleyApi::WString testName; 
            BeStringUtilities::Utf8ToWChar(testName, ::testing::UnitTest::GetInstance()->current_test_info()->name());
            path.AppendToPath(testName.c_str());
            path.AppendSeparator();
            }
#endif
        }
    virtual void*  _InvokeP (const char *,void *) override {BeAssert(false); return nullptr;}

    void _GetFrameworkSqlangFiles (BeFileName& path) override
        {
        GetDgnPlatformAssetsDirectory(path);
        path.AppendToPath(L"sqlang");
        path.AppendToPath(L"DgnPlatform_en.sqlang.db3");
        }

    static RefCountedPtr<BeGTestHost> Create (char const* progDir) {return new BeGTestHost (progDir);}
    };

static wchar_t const* s_configFileName = L"DgnV8ConverterTests.logging.config.xml";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getEnv (BeFileName& fn, WCharCP envname)
    {
    wchar_t filepath[MAX_PATH];
    if ((0 == GetEnvironmentVariableW (envname, filepath, MAX_PATH)))
        return ERROR;
    fn.SetName (filepath);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getLogConfigurationFilename (BeFileName& configFile, char const* argv0)
    {
    if (SUCCESS == getEnv (configFile, L"BEGTEST_LOGGING_CONFIG"))
        {
        if (BeFileName::DoesPathExist (configFile))
            {
            wprintf (L"BeGTest configuring logging with %s (Set by BEGTEST_LOGGING_CONFIG environment variable.)\n", configFile.GetName());
            return SUCCESS;
            }
        }

    configFile = BeFileName (BeFileName::DevAndDir, WString(argv0,true).c_str());
    configFile.AppendToPath (s_configFileName);
    configFile.BeGetFullPathName ();
    if (BeFileName::DoesPathExist (configFile))
        {
        wprintf (L"BeGTest configuring logging using %s. Override by setting BEGTEST_LOGGING_CONFIG in environment.\n", configFile.GetName());
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static void initLogging (char const* argv0)
    {
    BeFileName configFile;
    if (SUCCESS == getLogConfigurationFilename(configFile, argv0))
        {
        NativeLogging::LoggingConfig::SetOption (CONFIG_OPTION_CONFIG_FILE, configFile);
        NativeLogging::LoggingConfig::ActivateProvider (NativeLogging::LOG4CXX_LOGGING_PROVIDER);
        }
    else
        {
        wprintf (L"Logging.config.xml not found. BeGTest configuring default logging using console provider.\n");
        NativeLogging::LoggingConfig::ActivateProvider (NativeLogging::CONSOLE_LOGGING_PROVIDER);
        NativeLogging::LoggingConfig::SetSeverity (L"Performance", NativeLogging::LOG_TRACE);
        }
    }

/*---------------------------------------------------------------------------------**//**
* This is the TestListener that can act on certain events
* @bsiclass                                    Majd.Uddin                      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
class BeGTestListener : public ::testing::EmptyTestEventListener
    {
    virtual void OnTestProgramEnd(const ::testing::UnitTest& unit_test)
        {
        if (unit_test.failed_test_count() == 0)
            fprintf (stdout, "\n\nAll test(s) passed (count=%d)\n", unit_test.successful_test_count());
        else
            fprintf (stderr, "\n\n *** %d test(s) failed ***\n", unit_test.failed_test_count());
        }
    };

#if defined(BENTLEY_WIN32)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Farhad.Kabir                    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool SpawnProcessWin32 (char *command, DWORD &returnCode)
    {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    bool ret = false;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

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

    // Check exit code
    DWORD dwExitCode = 0;

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
#endif // BENTLEY_WIN32

#if defined(BENTLEY_WIN32)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Farhad.Kabir                    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
char const * getTestName(int argC, bvector<CharP> argv)
    {
    for (int i = 1; i < argC; i++) 
        {
        Utf8String utf8Str(argv[i]);
        if (utf8Str.Contains("--gtest_filter"))
            {
            bvector<Utf8String> tokens;
            BeStringUtilities::Split(argv[i], "=", nullptr, tokens);

            return tokens[1].c_str();
            }
        }
    return "";
    }
bool umdh_Use(int argC, char *argv[]) 
    {
    for (int i = 1; i < argC; i++) 
        {
        if (strcmp(argv[i], "--umdh") == 0) return true;
        }
    return false;
    }
#endif
#if defined(BENTLEY_WIN32)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Farhad.Kabir                    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
const char * WinGetEnv(const char * name)
    {
    const DWORD buffSize = 65535;
    static char buffer[buffSize];
    if (GetEnvironmentVariableA(name, buffer, buffSize))
        {
        return buffer;
        }
    else
        {
        return 0;
        }
    }
int WinSetEnv(const char * name, const char * value) 
    {
    return SetEnvironmentVariableA(name, value);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" int main (int argc, char **argv) 
    {
	int no = argc;
    bvector<CharP> args;
    for (int i = 0; i < no; i++) 
        {
        args.push_back(argv[i]);
        }
    ::testing::InitGoogleTest (&argc, argv);

    //listener with test failure output
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new BeGTestListener);

    initLogging (argv[0]);

    auto hostPtr = BeGTestHost::Create(argv[0]);
    

    BeTest::Initialize (*hostPtr);

    BeTest::SetRunningUnderGtest ();
    BeTest::SetAssertionFailureHandler([](WCharCP msg) {FAIL() << msg;});

    if (::testing::GTEST_FLAG(filter).empty() || ::testing::GTEST_FLAG(filter) == "*")
        { // use ignore lists if the user did not specify any filters on the command line
        Utf8String toignore, torun;
        BeTest::LoadFilters (toignore, torun);
        bastring filters;
        if (!toignore.empty())
            filters.assign ("-").append (toignore);
        if (!torun.empty())
            {
            if (!filters.empty())
                filters.append (":");
            filters.assign ("+").append (torun);
            }
        if (!filters.empty())
            ::testing::GTEST_FLAG(filter) = filters.c_str();
        }

    int check = 0;
#if defined(BENTLEY_WIN32)   

    char strCommand[1024];
    bool spawnRet;
    DWORD retCode;
    Utf8String gflagsSet;
    Utf8String pathSnapshot1;
    Utf8String pathSnapshot2;
    Utf8String pathComparison;
    Utf8String umdhPathJoin;
    Utf8String symbolPath;
    Utf8String gflagsPathJoin;
    
    if (umdh_Use(argc, argv))
        {
        RUN_ALL_TESTS();

        BeFileName rundirPath;
        hostPtr->GetRunRoot(argv[0] , rundirPath);
        WString currentDirectory(rundirPath.GetName());

        check = 1;
        //set _NT_SYMBOL_PATH
        BeFileName pathRun(currentDirectory);
        if(!BeFileName::DoesPathExist(pathRun))
            ASSERT_TRUE(BeFileNameStatus::Success == BeFileName::CreateNewDirectory(pathRun));
        ASSERT_TRUE(BeFileName::DoesPathExist(pathRun));

        BeStringUtilities::WCharToUtf8(symbolPath, currentDirectory.c_str());
        printf("%d\n",WinSetEnv("_NT_SYMBOL_PATH", symbolPath.c_str()));
        //printf("Symbols path is   :   %s\n", WinGetEnv("_NT_SYMBOL_PATH"));

        CharCP winSdkDir = WinGetEnv("Win10SdkDir");
        CharCP  defArch = "x64";

        bvector<Utf8CP> umdhPath2 = {winSdkDir,"Debuggers\\", defArch,"\\umdh.exe" };
        bvector<Utf8CP> gflagsPath2 = {winSdkDir,"Debuggers\\", defArch,"\\gflags.exe" };
        umdhPathJoin =  BeStringUtilities::Join(umdhPath2);
        gflagsPathJoin =  BeStringUtilities::Join(gflagsPath2);
        
        WString currentDirectoryExe = currentDirectory ;
        currentDirectoryExe.AppendUtf8("\\");
        currentDirectoryExe.AppendUtf8(argv[0]);
        BeStringUtilities::WCharToUtf8(gflagsSet, currentDirectoryExe.c_str());
        bvector<Utf8CP> gflags = {gflagsPathJoin.c_str(), " -i ", gflagsSet.c_str(), " +ust"  };
        Utf8String setGflags =  BeStringUtilities::Join(gflags);

        sprintf_s(strCommand, sizeof(strCommand), setGflags.c_str());
        spawnRet = SpawnProcessWin32(strCommand, retCode);
        
        //first snapshot
        //if (!BeFileName::DoesPathExist (L"run"))

        CharP log1Name = "\\FirstSnapshot.log";
        currentDirectory.AppendUtf8(log1Name);
        
        BeStringUtilities::WCharToUtf8(pathSnapshot1, currentDirectory.c_str());

        bvector<Utf8CP> snapshot1 = {umdhPathJoin.c_str(), " -p:%ld -f:", pathSnapshot1.c_str()  };
        Utf8String generateSnapshot1 =  BeStringUtilities::Join(snapshot1);

        // For debugging purposes, take initial snapshot of memory
        sprintf_s(strCommand, sizeof(strCommand), generateSnapshot1.c_str(), GetCurrentProcessId());
        spawnRet = SpawnProcessWin32(strCommand, retCode);
        //printf(strCommand, "\n");
        ///assert (spawnRet);
        }
#endif
    int errors = 0;
    if (check == 0)
        {//  Run the tests
        errors = RUN_ALL_TESTS(); 
        }
#if defined(BENTLEY_WIN32)
    if (umdh_Use(argc, argv))
        {
        for (int i = 0; i < 5; i++)
            RUN_ALL_TESTS();

        BeFileName rundirPath;
        hostPtr->GetRunRoot(argv[0] , rundirPath);
        WString currentDirectory2(rundirPath.GetName());
        WString currentDirectory3 = currentDirectory2;
        CharP log2Name = "\\SecondSnapshot.log";
        CharP logComparisonName = "";

        CharCP testName = getTestName(no, args);
        Utf8String utf8Str(testName);
        char pathCompComm[1024];

        if (utf8Str.Equals(""))
            {
            logComparisonName = "\\Comparison.log";
            currentDirectory3.AppendUtf8(logComparisonName);
            }
        else
            {
            sprintf_s(pathCompComm, sizeof(pathCompComm), "\\%s.log", testName);
            currentDirectory3.AppendUtf8(pathCompComm);
            }

        currentDirectory2.AppendUtf8(log2Name);
        
        BeStringUtilities::WCharToUtf8(pathSnapshot2, currentDirectory2.c_str());
        BeStringUtilities::WCharToUtf8(pathComparison, currentDirectory3.c_str());

        bvector<Utf8CP> snapshot2 = {umdhPathJoin.c_str(), " -p:%ld -f:", pathSnapshot2.c_str()  };
        Utf8String generateSnapshot2 =  BeStringUtilities::Join(snapshot2);

        // For debugging purposes, take terminal snapshot of memory
        sprintf_s(strCommand, sizeof(strCommand), generateSnapshot2.c_str(), GetCurrentProcessId());
        spawnRet = SpawnProcessWin32(strCommand, retCode);
        
        //printf(strCommand, "\n");

        bvector<Utf8CP> snapshotDiff = { umdhPathJoin.c_str(), " -d ",pathSnapshot1.c_str()," ", pathSnapshot2.c_str()," -f:",pathComparison.c_str() };
        Utf8String generateComparisonLog =  BeStringUtilities::Join(snapshotDiff);
        // Now take a diff of the two dumps
        sprintf_s(strCommand, sizeof(strCommand), generateComparisonLog.c_str());
        spawnRet = SpawnProcessWin32(strCommand, retCode);
        
        printf(strCommand, "\n");

        bvector<Utf8CP> gflagsDisable = { gflagsPathJoin.c_str(), " -i ", gflagsSet.c_str(), " -ust" };
        Utf8String disGflags = BeStringUtilities::Join(gflagsDisable);

        sprintf_s(strCommand, sizeof(strCommand), disGflags.c_str());
        spawnRet = SpawnProcessWin32(strCommand, retCode);
        }
#endif

    return errors;
    }
