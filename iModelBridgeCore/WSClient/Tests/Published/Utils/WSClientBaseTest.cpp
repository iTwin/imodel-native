/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Utils/WSClientBaseTest.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

void WSClientBaseTest::SetUp()
    {
    InitLogging();
    MobileDgnCommon::SetApplicationPathsProvider(&m_pathProvider);
    InitLibraries();
    }

void WSClientBaseTest::TearDown()
    {
    MobileDgnCommon::SetApplicationPathsProvider(nullptr);
    }

void WSClientBaseTest::SetUpTestCase()
    {}

void WSClientBaseTest::TearDownTestCase()
    {}

void WSClientBaseTest::InitLibraries()
    {
    BeFileName::CreateNewDirectory(m_pathProvider.GetTemporaryDirectory());

    BeSQLiteLib::Initialize(m_pathProvider.GetTemporaryDirectory());
    BeSQLite::EC::ECDb::Initialize(m_pathProvider.GetTemporaryDirectory(), &m_pathProvider.GetAssetsRootDirectory());

    L10N::SqlangFiles sqlangFiles(BeFileName(m_pathProvider.GetAssetsRootDirectory()).AppendToPath(L"sqlang\\BeGTest_en-US.sqlang.db3"));
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
