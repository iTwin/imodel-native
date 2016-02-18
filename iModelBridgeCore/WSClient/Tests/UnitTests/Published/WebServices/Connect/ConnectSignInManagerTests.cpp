/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/ConnectSignInManagerTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConnectSignInManagerTests.h"
#include <WebServices/Connect/ConnectSignInManager.h>
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/Connect.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace ::testing;

#ifdef USE_GTEST
void ConnectSignInManagerTests::SetUp()
    {
    m_client = std::make_shared<StubBuddiClient>();
    m_secureStore = std::make_shared<StubSecureStore>();
    m_localState = StubLocalState();

    Connect::Initialize (StubClientInfo (), GetHandlerPtr ());
    ConnectAuthenticationPersistence::CustomInitialize(&m_localState, m_secureStore);
    StubUrlProviderEnvironment(UrlProvider::Environment::Dev);
    }

void ConnectSignInManagerTests::TearDown()
    {
    Connect::Uninintialize ();
    }

void ConnectSignInManagerTests::StubUrlProviderEnvironment(UrlProvider::Environment env)
    {
    UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, &m_localState, m_client);
    }

TEST_F(ConnectSignInManagerTests, GetAuthenticationHandler_UrlProviderProduction_SetsValidateCertificateForAllRequests)
    {
    StubUrlProviderEnvironment(UrlProvider::Environment::Release);
    auto authHandler = ConnectSignInManager().GetAuthenticationHandler("https://foo.com", GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForFirstRequest([=] (HttpRequestCR request)
        {
        EXPECT_TRUE(request.GetValidateCertificate());
        return StubHttpResponse();
        });

    HttpRequest("https://foo.com", "GET", authHandler).Perform();
    }

TEST_F(ConnectSignInManagerTests, GetAuthenticationHandler_UrlProviderQa_DoesNotSetSetValidateCertificateForRequests)
    {
    StubUrlProviderEnvironment(UrlProvider::Environment::Qa);
    auto authHandler = ConnectSignInManager().GetAuthenticationHandler("https://foo.com", GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForFirstRequest([=] (HttpRequestCR request)
        {
        EXPECT_FALSE(request.GetValidateCertificate());
        return StubHttpResponse();
        });

    HttpRequest("https://foo.com", "GET", authHandler).Perform();
    }

TEST_F(ConnectSignInManagerTests, GetAuthenticationHandler_UrlProviderDev_DoesNotSetSetValidateCertificateForRequests)
    {
    StubUrlProviderEnvironment(UrlProvider::Environment::Dev);
     auto authHandler = ConnectSignInManager().GetAuthenticationHandler("https://foo.com", GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForFirstRequest([=] (HttpRequestCR request)
        {
        EXPECT_FALSE(request.GetValidateCertificate());
        return StubHttpResponse();
        });

    HttpRequest("https://foo.com", "GET", authHandler).Perform();
    }
#endif