/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Utils/WSClientBaseTest.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "WebServicesUnitTests.h"
#include "TestAppPathProvider.h"

BEGIN_WSCLIENT_UNITTESTS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSClientBaseTest : ::testing::Test
    {
    private:
        TestAppPathProvider m_pathProvider;

    private:
        void InitLibraries();
        void InitLogging();

    public:
        virtual void SetUp() override;
        virtual void TearDown() override;
        static void SetUpTestCase();
        static void TearDownTestCase();
    };

END_WSCLIENT_UNITTESTS_NAMESPACE
