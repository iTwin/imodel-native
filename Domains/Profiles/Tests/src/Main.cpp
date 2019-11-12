/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley\BeTest.h>
#include <Bentley\BeFileName.h>
#include <Logging\bentleylogging.h>

static wchar_t const* s_configFileName = L"ProfilesTestFixtureTests.logging.config.xml";

/*---------------------------------------------------------------------------------**//**
* This class knows how data files are linked into the Product/ECObjectsXXXTests
* directory structure.
* @bsiclass                                                                      11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct BeGTestHost : RefCounted<BeTest::Host>
    {
private:
    BeFileName m_programPath;
    WString m_programName;

    BeGTestHost (Utf8CP pProgramPath)
        {
        WString programPath = WString (pProgramPath, true);
        m_programName = BeFileName::GetFileNameWithoutExtension (programPath.c_str());
        }

public:
    static RefCountedPtr<BeGTestHost> Create (Utf8CP pProgramPath)
        {
        return new BeGTestHost (pProgramPath);
        }

    virtual void _GetTempDir (BeFileName& path) override
        {
        // NOTE: getting path from "temp" or "tmp" is inconvenient as it is set to a different directory
        // when tests are ran from bmake (i.e. building a part to run the tests) and when the .exe is ran
        // by itself (e.g. from the IDE). Prefer to have a fixed location for viewing the bim file.
        path.SetNameUtf8 (::getenv ("LOCALAPPDATA"));
        path.AppendToPath (L"Temp");
        path.AppendToPath (m_programName.c_str());
        }

    void _GetFrameworkSqlangFiles (BeFileName& path) override
        {
        GetDgnPlatformAssetsDirectory (path);
        path.AppendToPath (L"sqlang");
        path.AppendToPath (L"DgnPlatform_en.sqlang.db3");
        }

    virtual void _GetDocumentsRoot (BeFileName& path) override { path = m_programPath; }
    virtual void _GetDgnPlatformAssetsDirectory (BeFileName& path) override { path = m_programPath; }
    virtual void _GetOutputRoot (BeFileName& path) override { _GetTempDir (path); }
    virtual void* _InvokeP (const char *, void *) override { BeAssert (false); return nullptr; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getLogConfigurationFilename (BeFileName& configFile, Utf8CP pProgramName)
    {
    configFile.SetNameUtf8 (::getenv ("BEGTEST_LOGGING_CONFIG"));
    if (BeFileName::DoesPathExist (configFile))
        {
        wprintf (L"BeGTest configuring logging with %s (Set by BEGTEST_LOGGING_CONFIG environment variable.)\n", configFile.GetName());
        return SUCCESS;
        }

    configFile = BeFileName (BeFileName::DevAndDir, WString (pProgramName, true).c_str());
    configFile.AppendToPath (s_configFileName);
    configFile.BeGetFullPathName();
    if (BeFileName::DoesPathExist (configFile))
        {
        wprintf (L"BeGTest configuring logging using %s. Override by setting BEGTEST_LOGGING_CONFIG in environment.\n", configFile.GetName());
        return SUCCESS;
        }
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void initLogging (Utf8CP pProgramName)
    {
    BeFileName configFile;
    if (SUCCESS == getLogConfigurationFilename (configFile, pProgramName))
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
* @bsiclass                                                                      11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
class BeGTestListener : public ::testing::EmptyTestEventListener
    {
    virtual void OnTestProgramEnd (const ::testing::UnitTest& unit_test)
        {
        if (unit_test.failed_test_count() == 0)
            fprintf (stdout, "\n\nAll test (s) passed (count=%d)\n", unit_test.successful_test_count());
        else
            fprintf (stderr, "\n\n *** %d test (s) failed ***\n", unit_test.failed_test_count());
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct GtestFailureHandler : BeTest::IFailureHandler
    {
    virtual void _OnAssertionFailure (WCharCP msg) THROW_SPECIFIER (CharCP) { FAIL() << msg; }
    virtual void _OnUnexpectedResult (WCharCP msg) THROW_SPECIFIER (CharCP) { FAIL() << msg; }
    virtual void _OnFailureHandled() { ; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" int main (int argc, char **argv)
    {
    ::testing::InitGoogleTest (&argc, argv);

    //listener with test failure output
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append (new BeGTestListener);

    initLogging (argv[0]);

    auto hostPtr = BeGTestHost::Create (argv[0]);
    BeTest::Initialize (*hostPtr);
    BeTest::SetRunningUnderGtest();

    if (::testing::GTEST_FLAG (filter).empty() || ::testing::GTEST_FLAG (filter) == "*")
        {
        // use ignore lists if the user did not specify any filters on the command line
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
            ::testing::GTEST_FLAG (filter) = filters.c_str();
        }

    int errors = RUN_ALL_TESTS();
    return errors;
    }
