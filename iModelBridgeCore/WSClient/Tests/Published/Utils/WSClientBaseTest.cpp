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

    //L10N::SqlangFiles sqlangFiles(BeFileName(m_pathProvider.GetAssetsRootDirectory()).AppendToPath(L"sqlang\\BeGTest_en-US.sqlang.db3"));
    MobileDgnL10N::ReInitialize(MobileDgnL10N::GetDefaultFrameworkSqlangFiles(), MobileDgnL10N::GetDefaultFrameworkSqlangFiles());
    }

void WSClientBaseTest::InitLogging()
    {
    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
    NativeLogging::LoggingConfig::SetMaxMessageSize(100000);

    NativeLogging::LoggingConfig::SetSeverity("BeAssert", BentleyApi::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity("ECDb", BentleyApi::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity("ECDbMap", BentleyApi::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity("ECObjectsNative", BentleyApi::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_MOBILEDGN_UTILS_HTTP, BentleyApi::NativeLogging::LOG_INFO);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_MOBILEDGN_UTILS_THREADING, BentleyApi::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_WSCACHE, BentleyApi::NativeLogging::LOG_DEBUG);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_WSCLIENT, BentleyApi::NativeLogging::LOG_DEBUG);
    }
