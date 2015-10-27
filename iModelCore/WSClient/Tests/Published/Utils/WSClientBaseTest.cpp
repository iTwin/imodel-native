/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Utils/WSClientBaseTest.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "WSClientBaseTest.h"
#include <DgnClientFx/DgnClientFxCommon.h>
#include <DgnClientFx/DgnClientFxL10N.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
USING_NAMESPACE_WSCLIENT_UNITTESTS

void WSClientBaseTest::SetUp()
    {
    InitLogging();
    DgnClientFxCommon::SetApplicationPathsProvider(&m_pathProvider);
    InitLibraries();
    }

void WSClientBaseTest::TearDown()
    {
    DgnClientFxCommon::SetApplicationPathsProvider(nullptr);
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

    BeFileName sqlangFile;
    BeTest::GetHost().GetFrameworkSqlangFiles(sqlangFile);
    L10N::SqlangFiles sqlangFiles(sqlangFile);
    DgnClientFxL10N::ReInitialize(sqlangFiles, sqlangFiles);
    }

void WSClientBaseTest::InitLogging()
    {
    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
    NativeLogging::LoggingConfig::SetMaxMessageSize(100000);

    NativeLogging::LoggingConfig::SetSeverity("BeAssert", BentleyApi::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity("ECDb", BentleyApi::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity("ECDbMap", BentleyApi::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity("ECObjectsNative", BentleyApi::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_DGNCLIENTFX_UTILS_HTTP, BentleyApi::NativeLogging::LOG_INFO);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_DGNCLIENTFX_UTILS_THREADING, BentleyApi::NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_WSCACHE, BentleyApi::NativeLogging::LOG_DEBUG);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_WSCLIENT, BentleyApi::NativeLogging::LOG_DEBUG);
    }
