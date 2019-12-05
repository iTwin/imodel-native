/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "TestsHelper.h"
#include "Utils/MockHttpHandler.h"
#include "Mocks/BuddiProviderMock.h"
#include "Mocks/AuthHandlerProviderMock.h"
#include "../../../Licensing/Providers/EntitlementProvider.h"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

class EntitlementProviderTests : public ::testing::Test
    {
    private:
        std::shared_ptr<MockHttpHandler>                    m_httpHandlerMock;
        std::shared_ptr<BuddiProviderMock>                  m_buddiMock;
        std::shared_ptr<AuthHandlerProviderMock>            m_authProviderMock;
        std::shared_ptr<EntitlementProvider>                m_entitlementProvider;
    public:
        EntitlementProviderTests();
        static void SetUpTestCase();

        EntitlementProvider& GetEntitlementProvider() const;

        MockHttpHandler& GetMockHttpHandler() const;
        std::shared_ptr<MockHttpHandler> GetMockHttpHandlerPtr() const;

        AuthHandlerProviderMock&                 GetAuthHandlerProviderMock() const;
        std::shared_ptr<AuthHandlerProviderMock> GetAuthHandlerProviderMockPtr() const;

        BuddiProviderMock& GetMockBuddi() const;
        Utf8String MockEntitlementUrl();

        void TearDown();
    };
