/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/ClientIntegrationTests.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "TestsHelper.h"
#include "../UnitTests/Utils/MockHttpHandler.h"

#include <Licensing/Client.h>
#include <Licensing/AccessKeyClient.h>
#include <Licensing/SaasClient.h>

#include "../../Licensing/ClientImpl.h"
#include "../../Licensing/AccessKeyClientImpl.h"
#include "../../Licensing/SaasClientImpl.h"

#include <BeHttp/HttpClient.h>

USING_NAMESPACE_BENTLEY_LICENSING_INTEGRATION_TESTS

#define TEST_PRODUCT_ID      "2545"
#define TEST_VALID_ACCESSKEY "3469AD8D095A53F3CBC9A905A8FF8926"

class ClientIntegrationTests : public ::testing::Test
    {
    public:
        ClientIntegrationTests();
        static void SetUpTestCase();

        Licensing::ClientPtr          CreateTestClient(bool signIn, Utf8StringCR productId = TEST_PRODUCT_ID) const;
        Licensing::AccessKeyClientPtr CreateTestAccessKeyClient(Utf8StringCR productId = TEST_PRODUCT_ID, Utf8StringCR accessKey = TEST_VALID_ACCESSKEY) const;
        Licensing::SaasClientPtr      CreateTestSaasClient(int productId = std::atoi(TEST_PRODUCT_ID)) const;

        Licensing::ClientImplPtr          CreateTestClientImpl(bool signIn, Utf8StringCR productId = TEST_PRODUCT_ID) const;
        Licensing::AccessKeyClientImplPtr CreateTestAccessKeyClientImpl(Utf8StringCR productId = TEST_PRODUCT_ID, Utf8StringCR accessKey = TEST_VALID_ACCESSKEY) const;
        Licensing::SaasClientImplPtr      CreateTestSaasClientImpl(int productId = std::atoi(TEST_PRODUCT_ID)) const;

        void TearDown();
    };
