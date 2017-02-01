/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/Utils/WSClientBaseTest.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "WSClientBaseTest.h"
#include <DgnClientFx/DgnClientFxCommon.h>
#include <DgnClientFx/DgnClientFxL10N.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_WSCLIENT_UNITTESTS

std::shared_ptr<TestAppPathProvider>  WSClientBaseTest::s_pathProvider;

void WSClientBaseTest::SetUp()
    {
    BeTest::SetFailOnAssert(true);
    SetUpTestCase();
    }

void WSClientBaseTest::TearDown()
    {
    TearDownTestCase();
    }

void WSClientBaseTest::SetUpTestCase()
    {
    s_pathProvider = std::make_shared<TestAppPathProvider>();
    InitLogging();
    DgnClientFxCommon::SetApplicationPathsProvider(s_pathProvider.get());
    InitLibraries();
    }

void WSClientBaseTest::TearDownTestCase()
    {
    DgnClientFxCommon::SetApplicationPathsProvider(nullptr);
    s_pathProvider = nullptr;
    }

// *** NEEDS WORK: This is a work-around. The WSClient tests seem to require localized strings from the DgnClientFx sqlang db.
// ***              However, these tests do not initialize DgnClientUi properly and so they cannot call DgnClientFxL10N::GetDefaultFrameworkSqlangFiles.
// ***              This function is an attempt to replicate what DgnClientFxL10N::GetDefaultFrameworkSqlangFiles does. Use this until
// ***              we can figure out the right way to fix this.
static BeFileName getDgnClientFxSqlangFile()
    {
    BeFileName frameworkSqlangFile;
    BeTest::GetHost().GetFrameworkSqlangFiles(frameworkSqlangFile);

    BeFileName dgnClientFxSqlangFile = frameworkSqlangFile.GetDirectoryName();
    //dgnClientFxSqlangFile.AppendToPath(L"platform");
#if defined (NDEBUG)
    dgnClientFxSqlangFile.AppendToPath(L"DgnClientFx_en.sqlang.db3");
#else
    dgnClientFxSqlangFile.AppendToPath(L"DgnClientFx_pseudo.sqlang.db3");
#endif
    
    return dgnClientFxSqlangFile;
    }

void WSClientBaseTest::InitLibraries()
    {
    BeFileName::CreateNewDirectory(s_pathProvider->GetTemporaryDirectory());

    BeSQLiteLib::Initialize(s_pathProvider->GetTemporaryDirectory());
    BeSQLite::EC::ECDb::Initialize(s_pathProvider->GetTemporaryDirectory(), &s_pathProvider->GetAssetsRootDirectory());

    L10N::SqlangFiles sqlangFiles(getDgnClientFxSqlangFile());
    DgnClientFxL10N::ReInitialize(sqlangFiles, sqlangFiles);

    HttpClient::Initialize(s_pathProvider->GetAssetsRootDirectory());
    }

#if defined(BENTLEY_WIN32)
static wchar_t const* s_configFileName = L"logging.config.xml";
#endif

void WSClientBaseTest::InitLogging()
    {
#if defined(BENTLEY_WIN32) // _wgetenv is non-portable, and s_configFileName is carved out above...
    BeFileName loggingConfigFile(_wgetenv(L"WSCLIENT_TEST_LOGGING_CONFIG_FILE"));
    if (!BeFileName::DoesPathExist(loggingConfigFile))
        {
        loggingConfigFile.AssignOrClear(s_pathProvider->GetAssetsRootDirectory());
        loggingConfigFile.AppendToPath(s_configFileName);
        }

    if (BeFileName::DoesPathExist(loggingConfigFile))
        {
        NativeLogging::LoggingConfig::SetMaxMessageSize(10000);
        NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, loggingConfigFile.c_str());
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
        }
    else
        {
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
        NativeLogging::LoggingConfig::SetMaxMessageSize(100000);

        NativeLogging::LoggingConfig::SetSeverity("ECDb", BentleyApi::NativeLogging::LOG_ERROR);
        NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_HTTP, BentleyApi::NativeLogging::LOG_INFO);
        NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_TASKS, BentleyApi::NativeLogging::LOG_WARNING);
        NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_WSCACHE, BentleyApi::NativeLogging::LOG_WARNING);
        NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_WSCLIENT, BentleyApi::NativeLogging::LOG_WARNING);
        }    
#endif
    }
