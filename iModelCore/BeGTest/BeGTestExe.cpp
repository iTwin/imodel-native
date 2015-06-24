/*--------------------------------------------------------------------------------------+
|
|     $Source: BeGTestExe.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32)
    // WIP_BEFILENAME_PORTABILITY - need better way of setting temp directory
    #include <windows.h>
#endif

#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>
#include <BeSQLite/L10N.h>
#include <Logging/bentleylogging.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getFileNameFromEnv (BeFileName& fn, CharCP envname)
    {
    WString filepath (getenv(envname), BentleyCharEncoding::Utf8);
    if (filepath.empty())
        return ERROR;
    fn.SetName (filepath);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson
+---------------+---------------+---------------+---------------+---------------+------*/
static WCharCP getPlatform (BeFileName const& progPathIn)
    {
    BeFileName progPath(progPathIn);
    progPath.ToLower();
    if (progPath.find (L"winx86") != WString::npos)
        return L"Winx86";
    if (progPath.find (L"winx64") != WString::npos)
        return L"Winx64";
    if (progPath.find (L"linuxx86") != WString::npos)
        return L"LinuxX86";
    if (progPath.find (L"linuxx64") != WString::npos)
        return L"LinuxX64";
    if (progPath.find (L"macosx64") != WString::npos)
        return L"MacOSX64";

    printf ("%s\n", Utf8String(progPath).c_str());    
    BeAssert (false);
    return L"??";
    }

/*---------------------------------------------------------------------------------**//**
* This class knows how data files are linked into the Product/BeGTest directory structure.
* @bsimethod                                    Sam.Wilson                      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct BeGTestHost : RefCounted<BeTest::Host>
    {
    BeFileName m_programPath;

    BeGTestHost (char const* progDir)
        {
        m_programPath = BeFileName (BeFileName::DevAndDir, WString(progDir, true).c_str());
        m_programPath.BeGetFullPathName ();
        }

    void GetRunRoot (BeFileName& path)
        {
        if (getFileNameFromEnv (path, "OutRoot") != BSISUCCESS)
            {
            BeAssert(false);
            return;
            }
        path.AppendToPath (getPlatform(m_programPath));
        path.AppendToPath (L"build");
        path.AppendToPath (L"BeGTest");
        path.AppendToPath (L"run");
        }

    virtual void _GetDocumentsRoot (BeFileName& path) override
        {
        path = m_programPath;
        path.AppendToPath (L"DgnPlatformAssetsDirectory");
        path.AppendToPath (L"BeTestDocuments");
        path.AppendSeparator ();
        }

    virtual void _GetDgnPlatformAssetsDirectory (BeFileName& path) override
        {
        path = m_programPath;
        path.AppendToPath (L"DgnPlatformAssetsDirectory");
        path.AppendSeparator ();
        }

    virtual void _GetOutputRoot (BeFileName& path) override
        {
        GetRunRoot (path);
        path.AppendToPath (L"Output");
        path.AppendSeparator ();
        }

    virtual void _GetTempDir (BeFileName& path) override
        {
        GetRunRoot (path);
        path.AppendToPath (L"Temp");
        path.AppendSeparator ();
        }

    virtual void*  _InvokeP (char const* request, void* arg) override
        {
        if (0==strncmp (request, "foo", 3))
            {
            return new Utf8String ("foo");
            }
        NativeLogging::LoggingManager::GetLogger(L"BeTest")->errorv ("Unknown upcall %hs", request);
        return NULL;
        }

    static RefCountedPtr<BeGTestHost> Create (char const* progDir) {return new BeGTestHost (progDir);}
    };

static wchar_t const* s_configFileName = L"logging.config.xml";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getLogConfigurationFilename (BeFileName& configFile, char const* argv0)
    {
    if (SUCCESS == getFileNameFromEnv (configFile, "BEGTEST_LOGGING_CONFIG"))
        {
        if (BeFileName::DoesPathExist (configFile))
            {
            printf ("BeGTest configuring logging with %s (Set by BEGTEST_LOGGING_CONFIG environment variable.)\n", Utf8String(configFile.GetName()).c_str());
            return SUCCESS;
            }
        }

    configFile = BeFileName (BeFileName::DevAndDir, WString(argv0,true).c_str());
    configFile.AppendToPath (s_configFileName);
    configFile.BeGetFullPathName ();
    if (BeFileName::DoesPathExist (configFile))
        {
        printf ("BeGTest configuring logging using %s. Override by setting BEGTEST_LOGGING_CONFIG in environment.\n", Utf8String(configFile.GetName()).c_str());
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
        printf ("Logging.config.xml not found. BeGTest configuring default logging using console provider.\n");
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
    virtual void OnTestStart (const ::testing::TestInfo& test_info) override
        {
        // Zip the temp directory at the start of every test to simulate the transitory nature of temp on some platforms.
        BeFileName tempDir;
        BeTest::GetHost().GetTempDir (tempDir);
        BeFileName::EmptyAndRemoveDirectory (tempDir.c_str());
        BeFileName::CreateNewDirectory (tempDir.c_str());
        }

    virtual void OnTestProgramEnd(const ::testing::UnitTest& unit_test) override
        {
        fprintf (stderr, "\n\n%d tests passed, %d tests failed, %d tests were disabled\n", unit_test.successful_test_count(), unit_test.failed_test_count(), unit_test.disabled_test_count());
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bern.McCarty                   02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void initializeSqlangLocalization(BeGTestHost* hostP)
    {
    BeFileName appDir;
    hostP->_GetDgnPlatformAssetsDirectory(appDir);
    appDir.AppendToPath(L"sqlang");
    appDir.AppendToPath(L"BeGTest_en-US.sqlang.db3");
    // Some tests do not have language file. Skip L10N initialization because L10N assert will block the test.
    if(appDir.DoesPathExist())
        BeSQLite::L10N::Initialize(appDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct GtestFailureHandler : BeTest::IFailureHandler
    {
    virtual void _OnAssertionFailure (WCharCP msg) THROW_SPECIFIER(CharCP) {FAIL() << msg;}
    virtual void _OnUnexpectedResult (WCharCP msg) THROW_SPECIFIER(CharCP) {FAIL() << msg;}
    virtual void _OnFailureHandled() {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C"
int main (int argc, char **argv)
    {
    
    auto hostPtr = BeGTestHost::Create(argv[0]);

    //  Make sure output directies exist
    BeFileName outputDir;
    hostPtr->GetOutputRoot(outputDir);
    BeFileName::CreateNewDirectory(outputDir.c_str());

    BeFileName tempDir;
    hostPtr->GetTempDir(tempDir);
    BeFileName::CreateNewDirectory(tempDir.c_str());

    //  Finish initializing libraries
    initializeSqlangLocalization(hostPtr.get());
    BeTest::Initialize(*hostPtr);
    ::testing::InitGoogleTest (&argc, argv);
    ::testing::InitGoogleMock (&argc, argv);
    //listener with test failure output
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new BeGTestListener);

    initLogging (argv[0]);
    
    //  Configure the test runner
    BeTest::SetRunningUnderGtest ();

    GtestFailureHandler gtestFailureHandler;
    BeTest::SetIFailureHandler (gtestFailureHandler);

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

    //  Run the tests
    printf ("__START_TESTS__\n");
    int errors = RUN_ALL_TESTS();
    printf ("__END_TESTS__\n");

    return errors;
    }
