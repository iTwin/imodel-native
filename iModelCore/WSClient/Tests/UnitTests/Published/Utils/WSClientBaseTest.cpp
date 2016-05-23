/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/Utils/WSClientBaseTest.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "WSClientBaseTest.h"
#include <MobileDgn/MobileDgnCommon.h>
#include <MobileDgn/MobileDgnL10N.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_MOBILEDGN
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS
USING_NAMESPACE_WSCLIENT_UNITTESTS

BeFileName s_l10nSubPath(L"sqlang\\BeGTest_en-US.sqlang.db3");
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
    MobileDgnCommon::SetApplicationPathsProvider(s_pathProvider.get());
    InitLibraries();
    }

void WSClientBaseTest::TearDownTestCase()
    {
    MobileDgnCommon::SetApplicationPathsProvider(nullptr);
    s_pathProvider = nullptr;
    }

void WSClientBaseTest::InitLibraries()
    {
    BeFileName::CreateNewDirectory(s_pathProvider->GetTemporaryDirectory());

    BeSQLiteLib::Initialize(s_pathProvider->GetTemporaryDirectory());
    BeSQLite::EC::ECDb::Initialize(s_pathProvider->GetTemporaryDirectory(), &s_pathProvider->GetAssetsRootDirectory());

    L10N::SqlangFiles sqlangFiles(BeFileName(s_pathProvider->GetAssetsRootDirectory()).AppendToPath(s_l10nSubPath));
    MobileDgnL10N::ReInitialize(sqlangFiles, sqlangFiles);
    }

void WSClientBaseTest::InitLogging()
    {
    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
    NativeLogging::LoggingConfig::SetMaxMessageSize(100000);

    NativeLogging::LoggingConfig::SetSeverity("BeAssert", Bentley::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity("ECDb", Bentley::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity("ECDbMap", Bentley::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity("ECObjectsNative", Bentley::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_MOBILEDGN_UTILS_HTTP, Bentley::NativeLogging::LOG_INFO);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_MOBILEDGN_UTILS_THREADING, Bentley::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_WSCACHE, Bentley::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_WSCLIENT, Bentley::NativeLogging::LOG_WARNING);
    }

void WSClientBaseTest::SetL10NSubPath(BeFileNameCR subPath)
    {
    s_l10nSubPath = subPath;
    }
