/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "TestsHelper.h"

#include <Licensing/SaasClient.h>
#include "../../../Licensing/SaasClientImpl.h"

#include <BeHttp/HttpClient.h>

USING_NAMESPACE_BENTLEY_LICENSING_INTEGRATION_TESTS

#define TEST_PRODUCT_ID      "2545"

class SaasClientIntegrationTests : public ::testing::Test
    {
public:
    SaasClientIntegrationTests();
    static void SetUpTestCase();

    BeFileName GetLicensingDbPathIntegration();

    Licensing::SaasClientPtr      CreateTestSaasClient(int productId = std::atoi(TEST_PRODUCT_ID)) const;
    Licensing::SaasClientImplPtr      CreateTestSaasClientImpl(int productId = std::atoi(TEST_PRODUCT_ID)) const;

    void TearDown();
    };
