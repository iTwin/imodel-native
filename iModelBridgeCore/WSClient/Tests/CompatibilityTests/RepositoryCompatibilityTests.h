/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/RepositoryCompatibilityTests.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

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
