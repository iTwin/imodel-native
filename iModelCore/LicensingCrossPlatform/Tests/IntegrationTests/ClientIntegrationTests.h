/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "TestsHelper.h"
#include "../UnitTests/Utils/MockHttpHandler.h"

#include <Licensing/Client.h>
#include "../../Licensing/ClientImpl.h"

#include <BeHttp/HttpClient.h>

USING_NAMESPACE_BENTLEY_LICENSING_INTEGRATION_TESTS

#define TEST_PRODUCT_ID      "2545"

class ClientIntegrationTests : public ::testing::Test
    {
public:
    ClientIntegrationTests();
    static void SetUpTestCase();

    BeFileName GetLicensingDbPathIntegration() const;

    Licensing::ClientPtr          CreateTestClient(bool signIn, Utf8StringCR productId = TEST_PRODUCT_ID) const;
    Licensing::ClientImplPtr          CreateTestClientImpl(bool signIn, Utf8StringCR productId = TEST_PRODUCT_ID) const;

    void TearDown();
    };
