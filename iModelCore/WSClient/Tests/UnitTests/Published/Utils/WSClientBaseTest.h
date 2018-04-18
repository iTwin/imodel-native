/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/Utils/WSClientBaseTest.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "WebServicesUnitTests.h"
#include "TestAppPathProvider.h"

BEGIN_WSCLIENT_UNITTESTS_NAMESPACE

#define LOGGER_NAMESPACE_WSCLIENT_TESTS "WSClient.Tests"
#define TESTLOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_WSCLIENT_TESTS))

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSClientBaseTest : ::testing::Test
    {
    private:
        static std::shared_ptr<TestAppPathProvider> s_pathProvider;

    private:
        static void InitLibraries();
        static void InitLogging();

    public:
        virtual void SetUp() override;
        virtual void TearDown() override;
        static void SetUpTestCase();
        static void TearDownTestCase();
    };

END_WSCLIENT_UNITTESTS_NAMESPACE
