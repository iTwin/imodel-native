/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/TestUtils/TestsHost.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestsHost.h"

#include <MobileDgn/MobileDgnCommon.h>
#include <MobileDgn/MobileDgnL10N.h>

#include "../../UnitTests/Published/Utils/TestAppPathProvider.h"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_MOBILEDGN
USING_NAMESPACE_WSCLIENT_UNITTESTS

#define LOGGING_OPTION_CONFIG_FILE L"CONFIG_FILE"
#define LOGGING_CONFIG_FILE_NAME L"logging.config.xml"

std::shared_ptr<TestAppPathProvider> s_pathProvider;

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

void TestsHost::InitLibraries()
    {
    MobileDgnCommon::SetApplicationPathsProvider(s_pathProvider.get());

    BeSQLiteLib::Initialize(s_pathProvider->GetTemporaryDirectory());
    BeSQLite::EC::ECDb::Initialize(s_pathProvider->GetTemporaryDirectory(), &s_pathProvider->GetAssetsRootDirectory());

    L10N::SqlangFiles sqlangFiles(BeFileName(s_pathProvider->GetAssetsRootDirectory()).AppendToPath(L"sqlang\\platform\\MobileDgn_en.sqlang.db3"));
    MobileDgnL10N::ReInitialize(sqlangFiles, sqlangFiles);
    }

void TestsHost::InitLogging(int logLevel)
    {
    if (NativeLogging::LoggingConfig::IsProviderActive())
        NativeLogging::LoggingConfig::DeactivateProvider();

    BeFileName configFile = m_programDir;
    configFile.AppendToPath(LOGGING_CONFIG_FILE_NAME);
    if (configFile.DoesPathExist())
        {
        NativeLogging::LoggingConfig::SetOption(LOGGING_OPTION_CONFIG_FILE, configFile);
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
        return;
        }

    if (logLevel == 0)
        return;

    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
    NativeLogging::LoggingConfig::SetMaxMessageSize(100000);

    NativeLogging::LoggingConfig::SetSeverity("BeAssert", Bentley::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity("BeSQLite", Bentley::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity("ECDb", Bentley::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity("ECDbMap", Bentley::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity("ECObjectsNative", Bentley::NativeLogging::LOG_WARNING);

    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_MOBILEDGN_UTILS_HTTP, Bentley::NativeLogging::LOG_INFO);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_MOBILEDGN_UTILS_THREADING, Bentley::NativeLogging::LOG_WARNING);

    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_WSCACHE, Bentley::NativeLogging::LOG_INFO);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_WSCLIENT, Bentley::NativeLogging::LOG_WARNING);
    }

void* TestsHost::_InvokeP(char const* function_and_args)
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
