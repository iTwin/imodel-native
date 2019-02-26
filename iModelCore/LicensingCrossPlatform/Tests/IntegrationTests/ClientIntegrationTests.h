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


#include "../../Licensing/Providers/IBuddiProvider.h"
#include "../../Licensing/Providers/IPolicyProvider.h"
#include "../../Licensing/Providers/IUlasProvider.h"
#include <BeHttp/HttpClient.h>

USING_NAMESPACE_BENTLEY_LICENSING_INTEGRATION_TESTS

class ClientIntegrationTests : public ::testing::Test
    {
    private:
        //std::shared_ptr<IHttpHandler>    m_handler;
        //std::shared_ptr<IBuddiProvider>  m_buddiProvider;
        //std::shared_ptr<IPolicyProvider> m_policyProvider;
        //std::shared_ptr<IUlasProvider>   m_ulasProvider;
    public:
        ClientIntegrationTests();
        static void SetUpTestCase();

        //IHttpHandler& GetHandler() const;
        //std::shared_ptr<IHttpHandler> GetHandlerPtr() const;

        //IBuddiProvider& GetBuddiProvider() const;
        //std::shared_ptr<IBuddiProvider>  GetBuddiProviderPtr() const;
        //IPolicyProvider& GetPolicyProvider() const;
        //std::shared_ptr<IPolicyProvider> GetPolicyProviderPtr() const;
        //IUlasProvider& GetUlasProvider() const;
        //std::shared_ptr<IUlasProvider>   GetUlasProviderPtr() const;

        void TearDown();
    };
