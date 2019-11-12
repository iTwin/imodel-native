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

static wchar_t const* s_configFileName = L"StructuralTestFixtureTests.logging.config.xml";

/*---------------------------------------------------------------------------------**//**
* This class knows how data files are linked into the Product/ECObjectsXXXTests directory structure.
* @bsiclass                                     Vytautas.Valiukonis             08/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct BeGTestHost : RefCounted<BeTest::Host>
    {
    BeFileName m_programPath;
    WString m_programName;

    BeGTestHost(char const* argv0)
        {
        WString programFullPath = WString(argv0, true);
        m_programPath = BeFileName(BeFileName::DevAndDir, programFullPath.c_str());
        m_programPath.BeGetFullPathName();
        m_programName = BeFileName::GetFileNameWithoutExtension(programFullPath.c_str());
        }

    virtual void _GetDocumentsRoot(BeFileName& path) override { path = m_programPath; }
    virtual void _GetDgnPlatformAssetsDirectory(BeFileName& path) override { path = m_programPath; }
    virtual void _GetTempDir(BeFileName& path) override
        {
#if defined (BENTLEY_WIN32)
        // use the standard Windows temporary directory
        wchar_t tempPathW[MAX_PATH];
        ::GetTempPathW(_countof(tempPathW), tempPathW);
        path.SetName(tempPathW);
        path.AppendSeparator();
#else
        path.SetName(WString(getenv("tmp")).c_str());
        path.AppendSeparator();
#endif
        path.AppendToPath(m_programName.c_str());
        path.AppendSeparator();
        }

    virtual void _GetOutputRoot(BeFileName& path) override
        {
        _GetTempDir(path);
#if defined (USE_GTEST)
        if (::testing::UnitTest::GetInstance() && ::testing::UnitTest::GetInstance()->current_test_info())
            {
            BentleyApi::WString testCaseName;
            BeStringUtilities::Utf8ToWChar(testCaseName, ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name());
            path.AppendToPath(testCaseName.c_str());
            path.AppendSeparator();

            BentleyApi::WString testName;
            BeStringUtilities::Utf8ToWChar(testName, ::testing::UnitTest::GetInstance()->current_test_info()->name());
            path.AppendToPath(testName.c_str());
            path.AppendSeparator();
            }
#endif
        }
    virtual void* _InvokeP(const char *, void *) override { BeAssert(false); return nullptr; }

    void _GetFrameworkSqlangFiles(BeFileName& path) override
        {
        GetDgnPlatformAssetsDirectory(path);
        path.AppendToPath(L"sqlang");
        path.AppendToPath(L"DgnPlatform_en.sqlang.db3");
        }

    static RefCountedPtr<BeGTestHost> Create(char const* progDir) { return new BeGTestHost(progDir); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vytautas.Valiukonis             08/17
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getEnv(BeFileName& fn, WCharCP envname)
    {
    wchar_t filepath[MAX_PATH];
    if ((0 == GetEnvironmentVariableW(envname, filepath, MAX_PATH)))
        return ERROR;
    fn.SetName(filepath);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vytautas.Valiukonis             08/17
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getLogConfigurationFilename(BeFileName& configFile, char const* argv0)
    {
    if (SUCCESS == getEnv(configFile, L"BEGTEST_LOGGING_CONFIG"))
        {
        if (BeFileName::DoesPathExist(configFile))
            {
            wprintf(L"BeGTest configuring logging with %s (Set by BEGTEST_LOGGING_CONFIG environment variable.)\n", configFile.GetName());
            return SUCCESS;
            }
        }

    configFile = BeFileName(BeFileName::DevAndDir, WString(argv0, true).c_str());
    configFile.AppendToPath(s_configFileName);
    configFile.BeGetFullPathName();
    if (BeFileName::DoesPathExist(configFile))
        {
        wprintf(L"BeGTest configuring logging using %s. Override by setting BEGTEST_LOGGING_CONFIG in environment.\n", configFile.GetName());
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vytautas.Valiukonis             08/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void initLogging(char const* argv0)
    {
    BeFileName configFile;
    if (SUCCESS == getLogConfigurationFilename(configFile, argv0))
        {
        NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, configFile);
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
        }
    else
        {
        wprintf(L"Logging.config.xml not found. BeGTest configuring default logging using console provider.\n");
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
        NativeLogging::LoggingConfig::SetSeverity(L"Performance", NativeLogging::LOG_TRACE);
        }
    }

/*---------------------------------------------------------------------------------**//**
* This is the TestListener that can act on certain events
* @bsiclass                                     Vytautas.Valiukonis             08/17
+---------------+---------------+---------------+---------------+---------------+------*/
class BeGTestListener : public ::testing::EmptyTestEventListener
    {
    virtual void OnTestProgramEnd(const ::testing::UnitTest& unit_test)
        {
        if (unit_test.failed_test_count() == 0)
            fprintf(stdout, "\n\nAll test(s) passed (count=%d)\n", unit_test.successful_test_count());
        else
            fprintf(stderr, "\n\n *** %d test(s) failed ***\n", unit_test.failed_test_count());
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vytautas.Valiukonis             08/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct GtestFailureHandler : BeTest::IFailureHandler
    {
    virtual void _OnAssertionFailure(WCharCP msg) THROW_SPECIFIER(CharCP) { FAIL() << msg; }
    virtual void _OnUnexpectedResult(WCharCP msg) THROW_SPECIFIER(CharCP) { FAIL() << msg; }
    virtual void _OnFailureHandled() { ; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vytautas.Valiukonis             08/17
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" int main(int argc, char **argv)
    {
    ::testing::InitGoogleTest(&argc, argv);

    //listener with test failure output
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new BeGTestListener);

    initLogging(argv[0]);

    auto hostPtr = BeGTestHost::Create(argv[0]);

    BeTest::Initialize(*hostPtr);

    BeTest::SetRunningUnderGtest();

    //GtestFailureHandler gtestFailureHandler;
    // NEEDSWORK  BeTest::SetIFailureHandler(gtestFailureHandler);

    if (::testing::GTEST_FLAG(filter).empty() || ::testing::GTEST_FLAG(filter) == "*")
        {
        // use ignore lists if the user did not specify any filters on the command line
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

    int errors = RUN_ALL_TESTS();

    return errors;
    }
