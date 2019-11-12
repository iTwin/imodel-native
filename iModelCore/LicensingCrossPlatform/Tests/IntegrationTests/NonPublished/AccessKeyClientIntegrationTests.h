/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "TestsHelper.h"

#include <Licensing/AccessKeyClient.h>
#include "../../../Licensing/AccessKeyClientImpl.h"

#include <BeHttp/HttpClient.h>

USING_NAMESPACE_BENTLEY_LICENSING_INTEGRATION_TESTS

#define TEST_PRODUCT_ID      "2545"
#define TEST_VALID_ACCESSKEY "3469AD8D095A53F3CBC9A905A8FF8926"

class AccessKeyClientIntegrationTests : public ::testing::Test
    {
    public:
        AccessKeyClientIntegrationTests();
        static void SetUpTestCase();

        BeFileName GetLicensingDbPathIntegration() const;

        Licensing::AccessKeyClientPtr CreateTestClient(Utf8StringCR productId = TEST_PRODUCT_ID, Utf8StringCR accessKey = TEST_VALID_ACCESSKEY) const;
        Licensing::AccessKeyClientPtr CreateTestClientWithUltimate(Utf8StringCR ultimateId, Utf8StringCR productId = TEST_PRODUCT_ID, Utf8StringCR accessKey = TEST_VALID_ACCESSKEY) const;
        Licensing::AccessKeyClientImplPtr CreateTestClientImpl(Utf8StringCR productId = TEST_PRODUCT_ID, Utf8StringCR accessKey = TEST_VALID_ACCESSKEY) const;

        void TearDown();
    };
