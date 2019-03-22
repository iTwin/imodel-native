/*--------------------------------------------------------------------------------------+
|
|  $Source: BentleyTest/TestRunner/TestRunner.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <vector>
#include <BentleyTest/Common/FileIO.h>
#include <time.h>
#include <BentleyTest/Misc/ArgParser.h>

#ifdef TOOLSUBS_AVAILABLE_FROM_GTEST
/*-----------------------------------------------------------------------
This is all the patching mechanism for Pagalloc.  
-----------------------------------------------------------------------*/
#include <DgnPlatform\Tools\papatch.h>
#include <RmgrTools\Tools\memutil.h>
#endif

typedef int (*RunTestsImport)(int, char **, HINSTANCE, char const* outFolder);
typedef int (*HostInitialize)(int, char **, char const* outFolder);
typedef int (*HostShutdown)();

std::vector <char const*> g_defaultHostDlls;
std::vector <char const*> g_dgnplatformTestDlls;
std::vector <char const*> g_dgnviewTestDlls;

WCharCP SEP_STR = L"**************************************************\n";

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void testrunner_initializeDefaults () 
    {
    // Technically this list can be loaded dynamically by figuring out what dlls 
    //      are in the testrunner. But for now hard-coding this here is sufficient.
    g_defaultHostDlls.push_back ("DgnPlatformTestHost.dll");
    g_defaultHostDlls.push_back ("DgnViewTestHost.dll");

    g_dgnplatformTestDlls.push_back ("DgnPlatformTest_Published.dll");
    g_dgnplatformTestDlls.push_back ("DgnPlatformTest_NonPublished.dll");
    g_dgnplatformTestDlls.push_back ("DgnPlatformTest_Scenario.dll");
    g_dgnplatformTestDlls.push_back ("DgnPlatformTest_Regression.dll");

    g_dgnviewTestDlls.push_back ("DgnViewTest_SmokeTest.dll");
    g_dgnviewTestDlls.push_back ("DgnViewTest_Longevity.dll");
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void testrunner_logq (wchar_t const* fmt, ...)
    {
    va_list argp;
    va_start(argp, fmt);

    int length = _vscwprintf (fmt, argp) + 1;
    wchar_t* buffer = new wchar_t[length];
    vswprintf (buffer, fmt, argp);

    va_end(argp);
    wprintf (L"%ls", buffer);

    delete [] buffer;
    }

#ifdef UNUSED_FUNCTION
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void testrunner_log (wchar_t const* fmt, ...)
    {
    va_list argp;
    va_start(argp, fmt);

    int length = _vscwprintf (fmt, argp) + 1;
    wchar_t* buffer = new wchar_t[length];
    vswprintf (buffer, fmt, argp);

    va_end(argp);
    wprintf (L"[TestRunner]: %s", buffer);

    delete [] buffer;
    }
#endif

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void testrunner_log (char const* fmt, ...)
    {
    va_list argp;
    va_start(argp, fmt);

    int length = _vscprintf (fmt, argp) + 1;
    char* buffer = new char[length];
    vsprintf (buffer, fmt, argp);

    va_end(argp);
    printf ("[TestRunner]: %s", buffer);

    delete [] buffer;
    }

/*================================================================================**//**
* @bsiclass                                                     Kevin.Nyman     05/11
+===============+===============+===============+===============+===============+======*/
struct TestRunnerSettings : ArgParser
{
// ArgParser
protected: virtual void _InitializeArgumentMap () override;
public: virtual void _PrintUsage () override;
public: virtual void _PrintExamples () override;
public: virtual void ProcessParsedArguments () override;

// TestRunnerSettings
public: TestRunnerSettings ();

public: bool m_isPagallocOn;
public: bool m_disableForceHighAddress;
public: WString m_outputFolder;
public: int m_timesToRun;
};

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
TestRunnerSettings::TestRunnerSettings ()
    {
    m_isPagallocOn = false;
    m_disableForceHighAddress = true;

    InitializeArgumentMap();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void TestRunnerSettings::_InitializeArgumentMap ()
    {
    ArgInfoMap& argMap = m_argMap;
    std::string tmp = getenv ("tmp");
    tmp += "\\DgnPlatformTest\\";
    WString outFolderDefault = WString (tmp.c_str());

#ifdef TOOLSUBS_AVAILABLE_FROM_GTEST
    argMap["disable_high_address"]  = ArgInfo (ArgInfo::BOOL_TYPE, L"mem_utils", L"no", L"Disables memutil_forceHighAddress. Using high address space will cause *.pdb information to not be available with line numbers, method names etc. If you are running any type of coverage tool or memleak detection with stack traces, you should disable the force high address.", (void*)&m_disableForceHighAddress);
    argMap["pagalloc"]         = ArgInfo (ArgInfo::BOOL_TYPE, L"pagalloc", L"no", L"Enables pagalloc. MS_PAGALLOC=1 env var works also. Forwarded to sub dlls.", (void*)&m_isPagallocOn);
#endif
    argMap["out_folder"]       = ArgInfo (ArgInfo::STRING_TYPE, L"testrunner", outFolderDefault.c_str(), L"Output location for all test output.", (void*)&m_outputFolder);
    argMap["times_to_run"]     = ArgInfo (ArgInfo::INT_TYPE, L"testrunner", L"1", L"Number of times to run the testdll.", (void*)&m_timesToRun); 
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void TestRunnerSettings::ProcessParsedArguments ()
    {
    if (m_isPagallocOn)
        testrunner_log ("TestRunner detected --pagalloc. Turning pagalloc on.\n");

    if (!m_isPagallocOn && NULL != getenv("MS_PAGALLOC") && 0 != atoi(getenv("MS_PAGALLOC")))
        {
        testrunner_log ("TestRunner detected MS_PAGALLOC=%d. Turning pagalloc on.\n", atoi(getenv("MS_PAGALLOC")));
        m_isPagallocOn = true;
        }

#ifdef TOOLSUBS_AVAILABLE_FROM_GTEST
    if (m_isPagallocOn)
        paPatch_patchCRuntimeMemFuncs();

    bool disableForceHigh = false;
#if defined(_X86_)
    if (m_isPagallocOn)
        {
        disableForceHigh = true;
        testrunner_log ("WARNING!!! Not turning on memutil_forceHighAddress because running in x86 with pagalloc.\n");
        }
#endif
    if (!(m_disableForceHighAddress || disableForceHigh))
        {
        testrunner_log ("WARNING!!! Turning on memutil_forceHighAddress!!! This will cause *.dlls to load in non default address space and most likely will cause issues with pdb and line numbers. If you are running coverage or leak detection you will most likely get stack traces with missing method names and line numbers. It can be disabled by passing --disable_high_address option\n");
        memutil_forceHighAddress (false);
        }
#endif
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void TestRunnerSettings::_PrintUsage ()
    {
    testrunner_logq (SEP_STR); 
    testrunner_logq (L"TestRunner.exe Usage \n");
    testrunner_logq (L"  TestRunner.exe loads a host dll and a plugin dll that contains tests.\n");
    testrunner_logq (L"  TestRunner.exe [HOST_NAME.DLL] [--out_folder=OUTFOLDER] [--times_to_run=1] [--pagalloc] [--help] --testdll=TEST_NAME.DLL ARG1 ARG2 ... ARGN \n");
    testrunner_logq (L"      Args in [] are optional.\n");
    testrunner_logq (L"      HOST_NAME.DLL defaults to DgnPlatformTestHost.dll\n");
    testrunner_logq (L"      HOST_NAME.DLL can be 'NoHost' - Host dll will not be loaded and NULL will be passed into TestDll for host.\n");
    testrunner_logq (L"      --help found anywhere in the command line will show this help. Forwarded to sub dlls.\n");
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void TestRunnerSettings::_PrintExamples()
    {
    testrunner_logq (L"\n\n\n");
    testrunner_logq (L"  Examples:\n");
    testrunner_logq (L"      TestRunner.exe\n");
    testrunner_logq (L"          Load the default host, DgnPlatformTestHost.dll. All default tests run.\n"); 
    testrunner_logq (L"      TestRunner.exe NoHost --testdll=TestDllThatDoesntNeedHost.dll\n");
    testrunner_logq (L"          Do not load a host dll. Invoke a dll that doesn't need one.\n"); 
    testrunner_logq (L"      TestRunner.exe DgnViewTestHost.dll\n");
    testrunner_logq (L"          Load the DgnViewTestHost.dll. All default tests run.\n"); 
    testrunner_logq (L"      TestRunner.exe DgnPlatformTestHost.dll --testdll=DgnPlatformTest_Published.dll --gtest_filter=*ModelInfo*\n");
    testrunner_logq (L"          Load the DgnPlatformTest Host and the published tests for it, passing --gtest_filter=*ModelInfo* as the argument to the test dll.\n\n"); 
    testrunner_logq (L"      TestRunner.exe DgnPlatformTestHost.dll --testdll=DgnPlatformTest_Published.dll --gtest_filter=*ModelInfo* --testdll=DgnPlatformTest_NonPublished.dll --gtest_filter=*DgnModel*\n");
    testrunner_logq (L"          Load the DgnPlatformTest Host, pass --gtest_filter=*ModelInfo* into the published tests, and --gtest_filter=*DgnModel* into the nonpublished tests.\n\n");
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      08/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void testrunner_getLastErrorMsg(std::string& outErrorString, char const* lpszFunction)
    {
    LPVOID lpMsgBuf;
    char buffer[1024];
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    sprintf (buffer, "ERROR: %s failed with error %d: %s", lpszFunction, dw, lpMsgBuf);
    outErrorString = buffer;

    LocalFree(lpMsgBuf);
    }

/*================================================================================**//**
* @bsiclass                                                     KevinNyman      03/10
+===============+===============+===============+===============+===============+======*/
struct HostLoader
{
std::string         m_hostName;
HostInitialize      m_hostInitialize;
HostShutdown        m_hostShutdown;
HINSTANCE           m_hinstanceLib;

HostLoader () 
    : m_hinstanceLib(NULL)
    , m_hostInitialize(NULL)
    , m_hostShutdown(NULL)
    {m_hostName = "";}

~HostLoader ()                      {FreeHandlesIfNecessary();}

HINSTANCE GetDllHandle () const     {return m_hinstanceLib;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     10/11
+---------------+---------------+---------------+---------------+---------------+------*/
int Shutdown ()                     
    {
    if (NULL == m_hinstanceLib)
        return SUCCESS;
    return m_hostShutdown ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      03/10
+---------------+---------------+---------------+---------------+---------------+------*/
int Initialize (int argc, char* argv[], char const* outfolder)
    {
    if (NULL == m_hinstanceLib)
        return SUCCESS;
    return m_hostInitialize (argc, argv, outfolder);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      03/10
+---------------+---------------+---------------+---------------+---------------+------*/
void FreeHandlesIfNecessary ()
    {
    if (NULL != m_hinstanceLib)
        {
        FreeLibrary(m_hinstanceLib);
        m_hinstanceLib = NULL;
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LoadHost (char const* hostToLoad)
    {
    m_hostName = hostToLoad;
    if (0 == strcmpi ("nohost", hostToLoad))
        {
        testrunner_log ("NOTICE: 'NoHost' provided - Not using a host dll.\n");
        return SUCCESS;
        }
    
    testrunner_log ("Attempting to load '%s' as a host.\n", m_hostName.c_str());

    m_hinstanceLib = LoadLibrary (m_hostName.c_str());
    if (NULL == m_hinstanceLib)
        {
        std::string msg;
        testrunner_getLastErrorMsg (msg, "LoadLibrary");
        testrunner_log ("Error: unable to load '%s':\n    %s", m_hostName.c_str(), msg.c_str());
        return ERROR;
        }

    m_hostInitialize = (HostInitialize)GetProcAddress(m_hinstanceLib, "InitializeHost");
    if (NULL == m_hostInitialize)
        {
        testrunner_log ("Error: '%s' does not have InitializeHost() function which is required for host dlls.\n", m_hostName.c_str());
        FreeHandlesIfNecessary ();
        return ERROR;
        }

    m_hostShutdown = (HostShutdown)GetProcAddress(m_hinstanceLib, "ShutdownHost");
    if (NULL == m_hostShutdown)
        {
        testrunner_log ("Error: '%s' does not have ShutdownHost() function which is required for host dlls.\n", m_hostName.c_str());
        FreeHandlesIfNecessary ();
        return ERROR;
        }
    testrunner_log ("Host loaded successfully.\n");
    return SUCCESS;
    }
}; // HostLoader

/*================================================================================**//**
* @bsiclass                                                     KevinNyman      03/10
+===============+===============+===============+===============+===============+======*/
struct TestSuite
{
std::string name;

private: char** m_argv;
private: char** m_cpyArgv;
private: std::string m_outputLocation;
private: int m_dllReturnCode;
public: std::vector<std::string> args;

public: TestSuite ()
            : m_dllReturnCode (0),
              m_argv (NULL)
            {}
public: ~TestSuite ()                                   {FreeArgV();}

public: void SetResult (int result)                     {m_dllReturnCode = result;}
public: void SetOutputLocation (CharCP location)        {m_outputLocation = location;}
public: void AddArg (char const* arg)                   {args.push_back (arg);}

public: int GetResult () const                          {return m_dllReturnCode;}
public: std::string GetOutputLocation () const          {return m_outputLocation;}
public: int GetArgC () const                            {return (int)args.size();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
public: std::string GetFullArgString () 
    {
    std::string result;
    size_t i, sz = args.size();
    for (i = 0; i < sz; ++i)
        {
        if (i > 0)
            result += " ";
        result += args[i];
        }
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
public: char** GetArgV()
    {
    if (NULL == m_argv)
        {
        m_argv = new char*[args.size()];
        m_cpyArgv = new char*[args.size()];
        size_t i, sz = args.size();
        for (i = 0; i < sz; ++i)
            {
            size_t size = args[i].length() + 1;
            m_argv[i] = new char[size];
            m_cpyArgv[i] = m_argv[i];
            strcpy (m_argv[i], args[i].c_str());
            }
        }
    return m_cpyArgv;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
public: void FreeArgV()
    {
    if (m_argv)
        {
        size_t i, sz = args.size();
        for (i = 0; i < sz; i++)
            {
            delete[] m_argv[i];
            }
        delete[] m_argv;
        delete[] m_cpyArgv;
        m_argv= NULL;
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
public: void AddArgIfNotPresent (char const* arg)
    {
    size_t i, sz = args.size();
    for (i = 0; i < sz; ++i)
        if (0 == strcmpi (args[i].c_str(), arg))
            return; 
    AddArg (arg);
    }
}; // TestSuite

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      05/10
+---------------+---------------+---------------+---------------+---------------+------*/
static WString testrunner_requestOutDirBackupIfNecessary (wchar_t const* path, wchar_t const* name, int groupId, int pass, int backup = -1)
    {
    const int SZ = 1024;
    wchar_t buffer[SZ];

    if (-1 == backup)
        swprintf (buffer, L"%ls%ls/Run%d/", path, name, pass);
    else
        swprintf (buffer, L"%ls%ls/Run%d_Backup%d/", path, name, pass, backup);
    WString tempPath = buffer;
    size_t pos;
    while (std::string::npos != (pos = tempPath.find(L"/")))
        tempPath.replace (pos, 1, L"\\");
    return tempPath;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      05/10
+---------------+---------------+---------------+---------------+---------------+------*/
static WString testrunner_requestOutDirTimeIfNecessary (wchar_t const* path, wchar_t const* name, int groupId, int pass, int backup = -1)
    {
    const int SZ = 1024;
    wchar_t buffer[SZ];
    wchar_t timeBuffer[SZ];
    time_t t;
    time (&t);

    wcsftime (timeBuffer, SZ-1,L"%m_%d_%y_%H_%M_%S", localtime (&t));
    swprintf (buffer, L"%ls%ls/Run%d_%ls/", path, name, pass, timeBuffer);
    WString tempPath = buffer;
    size_t pos;
    while (std::string::npos != (pos = tempPath.find(L"/")))
        tempPath.replace (pos, 1, L"\\");
    return tempPath;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      05/10
+---------------+---------------+---------------+---------------+---------------+------*/
static WString testrunner_requestOutDir (wchar_t const* path, wchar_t const* name, int groupId, int pass, int backup, bool useTimeAsBackup)
    {
    if (useTimeAsBackup)
        return testrunner_requestOutDirTimeIfNecessary (path, name, groupId, pass, backup);
    return testrunner_requestOutDirBackupIfNecessary (path, name, groupId, pass, backup);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
static WString testrunner_makeOutputDirForTestGroup (wchar_t const* path, wchar_t const* name, int groupId, int pass, bool shouldDelete, bool useTimeAsBackup)
    {
    WString tempPath = testrunner_requestOutDir (path, name, groupId, pass, -1, useTimeAsBackup);
    int backupCount = 0;
    // We must clear the folder if there is already data there.
    //  If we cannot delete the directory - then we need to create an alternative directory.
    if (shouldDelete)
        {
        bool hasFilesLeft = false;
        bool hasDirsLeft = false;
        WString rootDir = tempPath;
        rootDir += L"..\\";
        DeleteDirectoryFiles (hasFilesLeft, hasDirsLeft, rootDir.c_str());
        }
    while (DirExists (tempPath.c_str()))
        {
        if (shouldDelete)
            {
            bool hasFilesLeft = false;
            bool hasDirsLeft = false;
            DeleteDirectoryFiles (hasFilesLeft, hasDirsLeft, tempPath.c_str());
            if (hasFilesLeft)
                {
                WString newPath = testrunner_requestOutDir (path, name, groupId, pass, backupCount, useTimeAsBackup);
                testrunner_log ("Warning!!!: Failed to remove files in %s which is used as the output dir, attempting to use alternative directory %s.\n", tempPath.c_str(), newPath.c_str());
                tempPath = newPath;
                backupCount++;
                }
            else
                break;
            }
        else
            {
            WString newPath = testrunner_requestOutDir (path, name, groupId, pass, backupCount, useTimeAsBackup);
            tempPath = newPath;
            backupCount++;
            }
        }

    WString wpath;
    size_t i, sz = tempPath.size();
    for (i = 0; i < sz; ++i)
        {
        wpath += tempPath[i];
        }
    CreateDirectoryRecursive (wpath.c_str(), false);
    return tempPath;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      04/10
+---------------+---------------+---------------+---------------+---------------+------*/
static WString testrunner_makeOutputDirForTestGroup (char const* path, char const* name, int groupId, int pass, bool shouldDelete, bool useTimeAsBackup)
    {
    const int SZ = 1024;
    wchar_t pp[SZ];
    wchar_t nn[SZ];
    for (int i = 0; i < SZ; ++i)
        {
        pp[i] = path[i];
        if (0 == path[i])
            break;
        }

    for (int i = 0; i < SZ; ++i)
        {
        nn[i] = name[i];
        if (0 == name[i])
            break;
        }

    return testrunner_makeOutputDirForTestGroup (pp, nn, groupId, pass, shouldDelete, useTimeAsBackup);
    }

/*--------------------------------------------------------------------------------**//**
* arguments for each testdll plugin are grouped after the --testdll= * argument  
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void testrunner_loadPluginListFromArgs (std::vector<TestSuite>& suites, int argc, char **argv) 
    {
    std::string testDllArg = "--testdll=";
    for (int i = 0; i < argc; ++i)
        {
        std::string arg = std::string (argv[i]);
        if (0 == strcmpi (arg.substr (0, testDllArg.length()).c_str(), testDllArg.c_str()))
            {
            std::string testDll = arg.substr (testDllArg.length(), arg.length());
            TestSuite t;
            t.name = testDll;
            t.AddArg (t.name.c_str());       // gtest requires we pass in the executable name - so we add the dll name as the first argument.
            suites.push_back (t);
            }
        else if (suites.size()> 0)
            {
            suites[suites.size()-1].AddArg (argv[i]);
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* This is used when the user didn't specify which dll they want to run, so we will 
* try to run all of the ones for the given host dll.
* Each plugin gets a copy of the arguments that were passed into the TestRunner.
*   This is different than if the user specified which dll they want to run.
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void testrunner_loadPluginListFromDefaults (std::vector<TestSuite>& suites, char const * hostname, int argc, char **argv, bool wasHelpPresent) 
    {
    char* defaultSuite = getenv ("DPT_DEFAULT_SUITE");
    if (NULL != defaultSuite)
        {
        testrunner_log ("DPT_DEFAULT_SUITE=%s\n", defaultSuite);
            
        TestSuite t;
        t.name = defaultSuite;
        t.AddArg (t.name.c_str());       // gtest requires we pass in the executable name - so we add the dll name as the first argument.
        char* defaultSuiteArg = getenv ("DPT_DEFAULT_SUITE_ARG");
        if (NULL != defaultSuiteArg)
            {
            testrunner_log ("DPT_DEFAULT_SUITE_ARG=%s\n", defaultSuiteArg);
            t.AddArg (defaultSuiteArg);
            }
        suites.push_back (t);
        return;
        }

    testrunner_log ("No testdll provided. Queuing up all tests plugin dlls for %s host.\n", hostname);
    std::vector<char const*>const* whichGroup = &g_dgnplatformTestDlls;
    if (0 == strcmpi (hostname, "dgnviewtesthost.dll"))
        {
        whichGroup = &g_dgnviewTestDlls;
        }

    for (size_t i = 1; i < (*whichGroup).size(); ++i)
        {
        TestSuite t;
        t.name = (*whichGroup)[i];
        t.AddArg (t.name.c_str());// gtest requires we pass in the executable name - so we add the dll name as the first argument.
        suites.push_back (t);
        for (int j = 1; j < argc; ++j)
            suites[suites.size()-1].AddArg (argv[j]);

        testrunner_log (" Queued [%s] to attempt to run.\n", suites[suites.size()-1].GetFullArgString().c_str());
        if (wasHelpPresent)
            {
            testrunner_log (" Not adding other test suites since --help was provided.\n");
            break; 
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     06/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void testrunner_dumpSuitesThatWereRun (std::vector<TestSuite>& suites) 
    {
    testrunner_logq (L"\n\n\n");
    testrunner_log ("Testdlls that were run:\n");
    size_t i, sz = suites.size();
    for (i = 0; i < sz; ++i)
        testrunner_log ("    [%s], Result: %d, Output Location: [%s]\n", suites[i].GetFullArgString().c_str(), suites[i].GetResult(), suites[i].GetOutputLocation().c_str());
    if (suites.size() > 1)
        testrunner_log ("Notice!!! Multiple tests were run. This is not ideal; if there were errors in any tests they might have caused cascading problems.\n");
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus testrunner_determineHost (HostLoader& hLoader, int argc, char** argv) 
    {
    std::vector <char const*> hostOrder;

    if (argc >= 2)
        hostOrder.push_back (argv[1]);

    char* defaultHost = getenv ("DPT_DEFAULT_HOST");
    if (NULL != defaultHost)
        {
        testrunner_log ("DPT_DEFAULT_HOST='%s'; adding to default host list.\n", defaultHost);
        hostOrder.push_back (defaultHost);
        }

    size_t i, sz = g_defaultHostDlls.size();
    for (i = 0; i < sz; ++i)
        hostOrder.push_back (g_defaultHostDlls[i]);

    if (hostOrder.size() < 1)
        {
        testrunner_log ("Error: No host provided either pass in first arg as host or provide with DPT_DEFAULT_HOST env var.\n");
        testrunner_log ("   DgnPlatformTestHost.dll\n");
        testrunner_log ("   DgnViewTestHost.dll\n");
        return ERROR;
        }

    BentleyStatus overallReturnStatus = SUCCESS;
    sz = hostOrder.size();
    for (i = 0; i < sz; ++i)
        {
        if (i > 1)
            testrunner_log ("NOTICE: Falling back to default host %s.\n", hostOrder[i]);
        if (SUCCESS == (overallReturnStatus = hLoader.LoadHost (hostOrder[i])))
            break;
        }
    
    if (SUCCESS != overallReturnStatus)
        testrunner_log ("Error: Default hosts failed to load.\n");

    return overallReturnStatus;
    }

/*--------------------------------------------------------------------------------**//**
* NOTE: Cannot shutdown instance of dgnplatformhost/dgnviewhost at this time(9/22/2010).
*       Therefore cannot switch hosts from dgnplatform/dgnview without leaving other one in mem (IE DON"T DO IT).
*       Otherwise I would have made it so no args into test runner loads each host and runs all their tests for that hosts linearly.
* 
* @bsimethod                                                    KevinNyman      03/10
+---------------+---------------+---------------+---------------+---------------+------*/
int main(int argc, char* argv[])
    {
    HostLoader hLoader;
    TestRunnerSettings settings;
    if (!settings.ParseArguments (argc, argv) && !settings.m_wasHelpPresent)
       return 1; 

    testrunner_initializeDefaults ();
    int overallReturnStatus = testrunner_determineHost (hLoader, *settings.GetCountP(), settings.GetArgVP());
    if (0 != overallReturnStatus)
        {
        settings.PrintUsage ();
        return overallReturnStatus;
        }

    std::string outFolder;
    std::vector<TestSuite> suites;
    testrunner_loadPluginListFromArgs (suites, *settings.GetCountP(), settings.GetArgVP());
    
    if (0 == suites.size())
        testrunner_loadPluginListFromDefaults (suites, hLoader.m_hostName.c_str(), argc, argv, settings.m_wasHelpPresent);

    // --pagalloc and --help must be forwarded to subdlls.
    if (settings.m_isPagallocOn)
        for (size_t i = 0; i < suites.size(); ++i)
            {
            suites[i].AddArgIfNotPresent ("--pagalloc");
            if (settings.m_wasHelpPresent)
                suites[i].AddArgIfNotPresent ("--help");
            }
    
    if (settings.m_wasHelpPresent)
        {
        testrunner_log ("TestRunner detected --help. Not initializing the %s host.\n", hLoader.m_hostName.c_str());
        }
    else
        {
        testrunner_log ("Initializing host %s.\n", hLoader.m_hostName.c_str());
        char outFolderString[1024];
        settings.m_outputFolder.ConvertToLocaleChars (outFolderString, 1024);
        outFolder = outFolderString;
        if (0 != (overallReturnStatus = hLoader.Initialize (*settings.GetCountP(), settings.GetArgVP(), outFolderString)))
            {
            testrunner_log ("Error: Host failed to initialize. Stopping before loading tests.");
            return overallReturnStatus;
            }
        testrunner_log ("Finished loading host.\n");
        }

    // Iterate over and run the test suites.
    size_t i, sz = suites.size();
    for (i = 0; i < sz; ++i)
        {
        char const* pluginName = suites[i].name.c_str();
        if (i > 0)
            testrunner_log ("\n\n\n");

        testrunner_log ("Preparing to run : %s\n", suites[i].GetFullArgString().c_str());
        testrunner_logq (SEP_STR);
        
        for (int j = 0; j < settings.m_timesToRun; ++j)
            {
            if (settings.m_timesToRun > 1 && j > 0)
                {
                testrunner_logq (SEP_STR);
                testrunner_log ("Suite %s: Run %d\n", suites[i].name.c_str(), j+1);
                testrunner_logq (SEP_STR);
                testrunner_log ("* * * * * WARNING * * * * *:\n");
                testrunner_log ("   Some tests require resources to be created and used in the tests.\n");
                testrunner_log ("   These resources might get locked by the Host dll.\n");
                testrunner_log ("   Look into this if you want to run multiple times.\n");
                testrunner_logq (SEP_STR);
                }
            // hacking this here instead of creating even MORE command line arguments to testrunner.
            bool isLongevity = 0 == strcmpi(suites[i].name.c_str(), "DgnViewTest_Longevity.dll");
            if (isLongevity)
                testrunner_log ("NOTE: Longevity test suite found - not deleting out folder - using time stamp.\n");
            WString tmpoutdir = testrunner_makeOutputDirForTestGroup (outFolder.c_str(), suites[i].name.c_str(), (int)i, j, !isLongevity, isLongevity);
            std::string outdir (tmpoutdir.begin(), tmpoutdir.end());
            RunTestsImport runTestsFunc;
            HINSTANCE hinstLib = LoadLibrary(TEXT(pluginName));
            if (NULL == hinstLib)
                {
                std::string msg;
                testrunner_getLastErrorMsg (msg, "LoadLibrary");
                testrunner_log ("Error: unable to load %s:\n    %s\n", pluginName, msg.c_str());
                overallReturnStatus = 1;
                continue;
                }


            runTestsFunc = (RunTestsImport)GetProcAddress(hinstLib, "Run");
            if (NULL == runTestsFunc)
                {
                testrunner_log ("Error: %s does not have Run() function which is required to handle execution.\n", pluginName);
                FreeLibrary(hinstLib);
                overallReturnStatus = 1;
                continue;
                }

            // Run the tests.
            char **theArgv = suites[i].GetArgV();
            int testResult = 0;
            suites[i].SetOutputLocation (outdir.c_str());
            testResult = runTestsFunc (suites[i].GetArgC(), theArgv, hLoader.GetDllHandle(), outdir.c_str());

            suites[i].SetResult (testResult);
            FreeLibrary (hinstLib);

            testrunner_log ("%s returned %d.\n", pluginName, testResult);

            // Only keep track of the last "failing" test dll. (If one fails, the test suite fails etc.).
            if (0 != testResult)
                overallReturnStatus = testResult;
            }
        }
    if (!settings.m_wasHelpPresent)
        testrunner_dumpSuitesThatWereRun (suites);

    int shutdownResult = 0;
    testrunner_log ("Shutting down host %s.\n", hLoader.m_hostName.c_str());

    // We shouldn't return error if the shutdown fails unless our result is the tests passed.
    if (0 != (shutdownResult = hLoader.Shutdown ()))
        {
        testrunner_log ("Warning: Host %s return %d.\n", hLoader.m_hostName.c_str(), shutdownResult);
        overallReturnStatus = shutdownResult;
        }

    testrunner_log ("Output data can be found in %s.\n", outFolder.c_str());
    return overallReturnStatus;
    }

