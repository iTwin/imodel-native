/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/WSClientBaseTest.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../BackDoor/PublicAPI/BackDoor/WebServices/iModelHub/BackDoor.h"
#include "TestAppPathProvider.h"

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE

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

END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
