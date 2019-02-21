/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/PolicyProviderTests.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "TestsHelper.h"
#include "Utils/MockHttpHandler.h"
#include "Mocks/BuddiProviderMock.h"
#include "Mocks/AuthHandlerProviderMock.h"
#include "../../../Licensing/Providers/PolicyProvider.h"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

class PolicyProviderTests : public ::testing::Test
    {
    private:
        std::shared_ptr<MockHttpHandler>                    m_handlerMock;
        std::shared_ptr<BuddiProviderMock>                  m_buddiMock;
        std::shared_ptr<AuthHandlerProviderMock>            m_authMock;
        std::shared_ptr<PolicyProvider>                     m_policyProvider;
    public:
        PolicyProviderTests();
        static void SetUpTestCase();

        PolicyProvider& GetPolicyProvider() const;

        MockHttpHandler& GetMockHttp() const;
        std::shared_ptr<MockHttpHandler> GetHandlerPtr() const;

        BuddiProviderMock& GetMockBuddi() const;
        Utf8String MockEntitlementUrl();

        void TearDown();
    };
