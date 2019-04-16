/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Client/WSError.h>
#include "Parser/TestData.h"
#include <Bentley/BeTest.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

struct RepositoryCompatibilityTests : ::testing::TestWithParam<TestRepositories>
    {
    static void SetTestData(const bvector<TestRepositories>& testData);

    void SetUp();
    void TearDown();
    };
