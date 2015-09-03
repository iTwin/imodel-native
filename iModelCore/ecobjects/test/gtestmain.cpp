/*--------------------------------------------------------------------------------------+
|
|     $Source: test/gtestmain.cpp $
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
#include <Logging/bentleylogging.h>

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
        m_programPath.BeGetFullPathName ();
        m_programName = BeFileName::GetFileNameWithoutExtension (programFullPath.c_str());
        }

    virtual void _GetDocumentsRoot (BeFileName& path) override              {path = m_programPath;}
    virtual void _GetDgnPlatformAssetsDirectory (BeFileName& path) override {path = m_programPath;}
    virtual void _GetTempDir (BeFileName& path) override                    {path = m_programPath;}

    virtual void _GetOutputRoot (BeFileName& path) override
        {
#if defined (BENTLEY_WIN32)
        // use the standard Windows temporary directory
        wchar_t tempPathW[MAX_PATH];    
        ::GetTempPathW (_countof(tempPathW), tempPathW);
        path.SetName (tempPathW);
        path.AppendSeparator ();
#else
        path.SetName (WString(getenv("tmp")).c_str());
        path.AppendSeparator ();
#endif
        path.AppendToPath (m_programName.c_str());
        path.AppendSeparator ();
        }

    virtual void*  _InvokeP (const char *,void *) override {BeAssert(false); return nullptr;}

    static RefCountedPtr<BeGTestHost> Create (char const* progDir) {return new BeGTestHost (progDir);}
    };

static wchar_t const* s_configFileName = L"logging.config.xml";

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
            fprintf (stdout, "\n\nAll test(s) passed\n");
        else
            fprintf (stderr, "\n\n *** %d test(s) failed ***\n", unit_test.failed_test_count());
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct GtestFailureHandler : BeTest::IFailureHandler
    {
    virtual void _OnAssertionFailure (WCharCP msg) THROW_SPECIFIER(CharCP) {FAIL() << msg;}
    virtual void _OnUnexpectedResult (WCharCP msg) THROW_SPECIFIER(CharCP) {FAIL() << msg;}
    virtual void _OnFailureHandled() {;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" 
int main (int argc, char **argv) 
    {
    ::testing::InitGoogleTest (&argc, argv);

    //listener with test failure output
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new BeGTestListener);

    initLogging (argv[0]);

    auto hostPtr = BeGTestHost::Create(argv[0]);

    BeFileName outRoot;                 // make sure output directory exists
    hostPtr->GetOutputRoot (outRoot);
    BeFileName::CreateNewDirectory (outRoot.c_str());
    
    BeTest::Initialize (*hostPtr);

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

    int errors = RUN_ALL_TESTS();

    return errors;
    }
