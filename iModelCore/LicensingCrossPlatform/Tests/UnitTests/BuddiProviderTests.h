/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/BuddiProviderTests.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "TestsHelper.h"
#include "Utils/MockHttpHandler.h"
#include "Mocks/AuthHandlerProviderMock.h"
#include "../../Licensing/Providers/BuddiProvider.h"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

class BuddiProviderTests : public ::testing::Test
    {
    private:
        std::shared_ptr<MockHttpHandler>                    m_httpHandlerMock;
        std::shared_ptr<AuthHandlerProviderMock>            m_authProviderMock;
        std::shared_ptr<BuddiProvider>                      m_buddiProvider;
    public:
        BuddiProviderTests();
        static void SetUpTestCase();

        BuddiProvider& GetBuddiProvider() const;

        MockHttpHandler& GetMockHttpHandler() const;
        std::shared_ptr<MockHttpHandler> GetMockHttpHandlerPtr() const;

        //AuthHandlerProviderMock&                 GetAuthHandlerProviderMock() const;
        //std::shared_ptr<AuthHandlerProviderMock> GetAuthHandlerProviderMockPtr() const;

        void TearDown();
    };
