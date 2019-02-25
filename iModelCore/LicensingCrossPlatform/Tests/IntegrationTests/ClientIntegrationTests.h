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

#include "../UnitTests/Mocks/BuddiProviderMock.h"
#include "../UnitTests/Mocks/PolicyProviderMock.h"
#include "../UnitTests/Mocks/UlasProviderMock.h"

USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

class ClientIntegrationTests : public ::testing::Test
    {
    private:
        std::shared_ptr<MockHttpHandler>    m_handler;
        std::shared_ptr<BuddiProviderMock>  m_buddiProviderMock;
        std::shared_ptr<PolicyProviderMock> m_policyProviderMock;
        std::shared_ptr<UlasProviderMock>   m_ulasProviderMock;
    public:
        ClientIntegrationTests();
        static void SetUpTestCase();

        MockHttpHandler& GetHandler() const;
        std::shared_ptr<MockHttpHandler> GetHandlerPtr() const;

        BuddiProviderMock& GetBuddiProviderMock() const;
        std::shared_ptr<BuddiProviderMock>  GetBuddiProviderMockPtr() const;
        PolicyProviderMock& GetPolicyProviderMock() const;
        std::shared_ptr<PolicyProviderMock> GetPolicyProviderMockPtr() const;
        UlasProviderMock& GetUlasProviderMock() const;
        std::shared_ptr<UlasProviderMock>   GetUlasProviderMockPtr() const;

        void TearDown();
    };
