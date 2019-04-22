/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/ClientTests.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "TestsHelper.h"
#include "Utils/MockHttpHandler.h"

#include "Mocks/LicensingDbMock.h"
#include "Mocks/PolicyProviderMock.h"
#include "Mocks/UlasProviderMock.h"
#include <WebServices/Connect/ConnectSignInManager.h>
#include "../../../Licensing/ClientImpl.h"

USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

#define TEST_PRODUCT_ID         "2545"

class ClientTests : public ::testing::Test
    {
    private:
        std::shared_ptr<MockHttpHandler>         m_handler;
        std::shared_ptr<LicensingDbMock>         m_licensingDbMock;
        std::shared_ptr<PolicyProviderMock>      m_policyProviderMock;
        std::shared_ptr<UlasProviderMock>        m_ulasProviderMock;

    public:
        ClientTests();
        static void SetUpTestCase();

        MockHttpHandler&                         GetHandler() const { return *m_handler; }
        std::shared_ptr<MockHttpHandler>         GetHandlerPtr() const { return m_handler; }
        PolicyProviderMock&                      GetPolicyProviderMock() const { return *m_policyProviderMock; }
        std::shared_ptr<PolicyProviderMock>      GetPolicyProviderMockPtr() const { return m_policyProviderMock; }
        UlasProviderMock&                        GetUlasProviderMock() const { return *m_ulasProviderMock; }
        std::shared_ptr<UlasProviderMock>        GetUlasProviderMockPtr() const { return m_ulasProviderMock; }
        LicensingDbMock&                         GetLicensingDbMock() const { return *m_licensingDbMock; }
        std::shared_ptr<LicensingDbMock>         GetLicensingDbMockPtr() const { return m_licensingDbMock; }

        Licensing::ClientImplPtr                 CreateTestClient(WebServices::ConnectSignInManager::UserInfo userInfo, Utf8StringCR productId = TEST_PRODUCT_ID, WebServices::ClientInfoPtr clientInfo = nullptr) const;

        void TearDown();
    };
