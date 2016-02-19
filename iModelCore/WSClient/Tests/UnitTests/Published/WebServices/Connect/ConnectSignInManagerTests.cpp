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

TEST_F(ConnectSignInManagerTests, GetAuthenticationHandler_TwoRequestsSentUsingDifferentAuthHandlersWithSameServer_TokenReusedForSecondRequest)
    {
    ConnectAuthenticationPersistence::GetShared()->SetToken(StubSamlToken());
    ConnectSignInManager manager;
    auto authHandler1 = manager.GetAuthenticationHandler("https://foo.com", GetHandlerPtr());
    auto authHandler2 = manager.GetAuthenticationHandler("https://foo.com", GetHandlerPtr());

    int tokenRequestCount = 0;
    GetHandler().ForAnyRequest([&] (HttpRequestCR request)
        {
        if (request.GetUrl().find("/IssueEx") != Utf8String::npos)
            tokenRequestCount++;

        return StubImsTokenHttpResponse();
        });

    HttpRequest("https://foo.com/a", "GET", authHandler1).Perform();
    EXPECT_EQ(1, tokenRequestCount);

    HttpRequest("https://foo.com/b", "GET", authHandler2).Perform();
    EXPECT_EQ(1, tokenRequestCount);
    }

TEST_F(ConnectSignInManagerTests, GetAuthenticationHandler_TwoRequestsSentInParaleleUsingDifferentAuthHandlersWithSameServer_OnlyOneTokenRequestSent)
    {
    ConnectAuthenticationPersistence::GetShared()->SetToken(StubSamlToken());
    auto authHandler = ConnectSignInManager().GetAuthenticationHandler("https://foo.com", GetHandlerPtr());

    AsyncTestCheckpoint checkpoint;

    int tokenRequestCount = 0;
    GetHandler().ForAnyRequest([&] (HttpRequestCR request)
        {
        if (request.GetUrl().find("/IssueEx") != Utf8String::npos)
            tokenRequestCount ++;

        checkpoint.CheckinAndWait();

        return StubHttpResponse();
        });

    auto t1 = HttpRequest("https://foo.com/a", "GET", authHandler).PerformAsync();
    auto t2 = HttpRequest("https://foo.com/b", "GET", authHandler).PerformAsync();

    checkpoint.WaitUntilReached();
    checkpoint.Continue();

    t1->Wait();
    t2->Wait();

    EXPECT_EQ(1, tokenRequestCount);
    }
#endif
