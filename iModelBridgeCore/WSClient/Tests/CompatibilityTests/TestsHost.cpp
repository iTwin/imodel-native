/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "TestsHost.h"

#include <BeSQLite/L10N.h>

#include <Bentley/Tasks/AsyncResult.h>
#include <Bentley/Tasks/AsyncTask.h>

#include "../UnitTests/Published/Utils/TestAppPathProvider.h"

#include "Logging/GenericLogProviderActivator.h"
#include "Logging/Logging.h"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_WSCLIENT_UNITTESTS

#define LOGGING_OPTION_CONFIG_FILE L"CONFIG_FILE"
#define LOGGING_CONFIG_FILE_NAME L"logging.config.xml"

std::shared_ptr<TestAppPathProvider> s_pathProvider;
Utf8String s_errorLog;

TestsHost::TestsHost(BeFileNameCR programPath, BeFileNameCR workDir, int logLevel)
    {
    m_programDir = programPath.GetDirectoryName();

    m_outputDir = workDir;
    if (workDir.empty())
        m_outputDir = m_programDir;

    m_outputDir.AppendToPath(L"WSCTWorkDir").AppendSeparator();

    s_pathProvider = std::make_shared<TestAppPathProvider>(m_programDir, m_outputDir);

    SetupTestEnvironment();
    InitLibraries();
    InitLogging(logLevel);
    }

RefCountedPtr<TestsHost> TestsHost::Create(BeFileNameCR programPath, BeFileNameCR workDir, int logLevel)
    {
    return new TestsHost(programPath, workDir, logLevel);
    }

void TestsHost::SetupTestEnvironment()
    {
    BeFileName::EmptyAndRemoveDirectory(m_outputDir);
    BeFileName::CreateNewDirectory(m_outputDir);

    BeFileName tempDir = s_pathProvider->GetTemporaryDirectory();
    BeFileName::EmptyAndRemoveDirectory(tempDir);
    BeFileName::CreateNewDirectory(tempDir);
    }

static BeFileName getSqlangFile()
    {
    BeFileName testsSqlangFile = s_pathProvider->GetAssetsRootDirectory();
    testsSqlangFile.AppendToPath(L"sqlang\\WSClient_en.sqlang.db3");
    return testsSqlangFile;
    }

void TestsHost::InitLibraries()
    {
    HttpClient::Initialize(s_pathProvider->GetAssetsRootDirectory());
    CachingDataSource::Initialize(s_pathProvider->GetAssetsRootDirectory());
    BeSQLiteLib::Initialize(s_pathProvider->GetTemporaryDirectory());
    BeSQLite::EC::ECDb::Initialize(s_pathProvider->GetTemporaryDirectory(), &s_pathProvider->GetAssetsRootDirectory());

    L10N::SqlangFiles sqlangFiles(getSqlangFile());
    BeSQLite::L10N::Initialize(sqlangFiles);
    }

void TestsHost::InitLogging(int logLevel)
    {
    if (NativeLogging::LoggingConfig::IsProviderActive())
        NativeLogging::LoggingConfig::DeactivateProvider();

    bool silent = logLevel == 0;

    // Log LOG_ERROR as test failures. We should consider LOG_WARNING as failures in future as well.
    GenericLogProviderActivator::Activate([=] (NativeLogging::SEVERITY sev, WCharCP msg)
        {
        if (sev >= NativeLogging::LOG_ERROR)
            s_errorLog += Utf8String(msg);

        if (!silent)
            fwprintf(stdout, msg);
        });

    NativeLogging::LoggingConfig::SetSeverity("BeAssert", NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity("BeSQLite", NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity("ECDb", NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity("ECDbMap", NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity("ECObjectsNative", NativeLogging::LOG_WARNING);

    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_HTTP, NativeLogging::LOG_INFO);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_TASKS, NativeLogging::LOG_WARNING);

    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_WSCACHE, NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_WSCLIENT, NativeLogging::LOG_WARNING);

    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_WSCCTESTS, NativeLogging::LOG_INFO);
    }

Utf8String& TestsHost::GetErrorLog()
    {
    return s_errorLog;
    }

void* TestsHost::_InvokeP(char const* function, void* args)
    {
    return nullptr;
    }

void TestsHost::_GetDocumentsRoot(BeFileName& path)
    {
    path = s_pathProvider->GetDocumentsDirectory();
    }

void TestsHost::_GetDgnPlatformAssetsDirectory(BeFileName& path)
    {
    path = m_programDir;
    }

void TestsHost::_GetOutputRoot(BeFileName& path)
    {
    path = m_outputDir;
    }

void TestsHost::_GetTempDir(BeFileName& path)
    {
    path = s_pathProvider->GetTemporaryDirectory();
    }

void TestsHost::_GetFrameworkSqlangFiles(BeFileName& path)
    {
    path = getSqlangFile();
    }
