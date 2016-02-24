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

    Connect::Initialize(StubClientInfo(), GetHandlerPtr());
    ConnectAuthenticationPersistence::CustomInitialize(&m_localState, m_secureStore);
    StubUrlProviderEnvironment(UrlProvider::Environment::Dev);
    }

void ConnectSignInManagerTests::TearDown()
    {
    Connect::Uninintialize();
    }

void ConnectSignInManagerTests::StubUrlProviderEnvironment(UrlProvider::Environment env)
    {
    UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, &m_localState, m_client);
    }

TEST_F(ConnectSignInManagerTests, GetAuthenticationHandler_UrlProviderProduction_SetsValidateCertificateForAllRequests)
    {
    StubUrlProviderEnvironment(UrlProvider::Environment::Release);
    auto manager = ConnectSignInManager::Create(&m_localState, m_secureStore);
    auto authHandler = manager->GetAuthenticationHandler("https://foo.com", GetHandlerPtr());

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
    auto manager = ConnectSignInManager::Create(&m_localState, m_secureStore);
    auto authHandler = manager->GetAuthenticationHandler("https://foo.com", GetHandlerPtr());

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
    auto manager = ConnectSignInManager::Create(&m_localState, m_secureStore);
    auto authHandler = manager->GetAuthenticationHandler("https://foo.com", GetHandlerPtr());

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
    auto manager = ConnectSignInManager::Create(&m_localState, m_secureStore);

    GetHandler().ForFirstRequest(StubImsTokenHttpResponse());
    ASSERT_TRUE(manager->SignInWithCredentials({"Foo", "Boo"})->GetResult().IsSuccess());
    EXPECT_EQ(1, GetHandler().GetRequestsPerformed());

    auto authHandler1 = manager->GetAuthenticationHandler("https://foo.com", GetHandlerPtr());
    auto authHandler2 = manager->GetAuthenticationHandler("https://foo.com", GetHandlerPtr());

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
    auto manager = ConnectSignInManager::Create(&m_localState, m_secureStore);

    GetHandler().ForFirstRequest(StubImsTokenHttpResponse());
    ASSERT_TRUE(manager->SignInWithCredentials({"Foo", "Boo"})->GetResult().IsSuccess());
    EXPECT_EQ(1, GetHandler().GetRequestsPerformed());

    auto authHandler = manager->GetAuthenticationHandler("https://foo.com", GetHandlerPtr());

    AsyncTestCheckpoint checkpoint;

    int tokenRequestCount = 0;
    GetHandler().ForAnyRequest([&] (HttpRequestCR request)
        {
        if (request.GetUrl().find("/IssueEx") != Utf8String::npos)
            tokenRequestCount++;

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

TEST_F(ConnectSignInManagerTests, GetUserInfo_NotSignedIn_ReturnsEmpty)
    {
    auto manager = ConnectSignInManager::Create(&m_localState, m_secureStore);

    auto info = manager->GetUserInfo();

    EXPECT_EQ("", info.firstName);
    EXPECT_EQ("", info.lastName);
    EXPECT_EQ("", info.userId);
    EXPECT_EQ("", info.username);
    }

TEST_F(ConnectSignInManagerTests, GetUserInfo_SignedIn_ReturnsValuesFromToken)
    {
    auto manager = ConnectSignInManager::Create(&m_localState, m_secureStore);

    auto tokenStr =
        R"(<saml:Assertion MajorVersion="1" MinorVersion="1" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion">
            <saml:Conditions 
                NotBefore   ="2016-02-24T10:48:17.584Z"
                NotOnOrAfter="2016-02-24T10:48:17.584Z">
            </saml:Conditions>
            <ds:Signature xmlns:ds="http://www.w3.org/2000/09/xmldsig#">
                <KeyInfo xmlns="http://www.w3.org/2000/09/xmldsig#">
                    <X509Data>
                        <X509Certificate>TestCert</X509Certificate>
                    </X509Data>
                </KeyInfo>
            </ds:Signature>
            <saml:AttributeStatement>
                <saml:Attribute AttributeName="givenname">
                    <saml:AttributeValue>ValueA</saml:AttributeValue>
                </saml:Attribute>
                <saml:Attribute AttributeName="surname">
                    <saml:AttributeValue>ValueB</saml:AttributeValue>
                </saml:Attribute>
                <saml:Attribute AttributeName="userid">
                    <saml:AttributeValue>ValueC</saml:AttributeValue>
                </saml:Attribute>
                <saml:Attribute AttributeName="name">
                    <saml:AttributeValue>ValueD</saml:AttributeValue>
                </saml:Attribute>
            </saml:AttributeStatement>
        </saml:Assertion>)";

    auto token = std::make_shared<SamlToken>(tokenStr);
    ASSERT_TRUE(manager->SignInWithToken(token)->GetResult().IsSuccess());

    auto info = manager->GetUserInfo();

    EXPECT_EQ("ValueA", info.firstName);
    EXPECT_EQ("ValueB", info.lastName);
    EXPECT_EQ("ValueC", info.userId);
    EXPECT_EQ("ValueD", info.username);
    }

TEST_F(ConnectSignInManagerTests, GetUserInfo_SignedInWithToken_ReturnsValuesFromToken)
    {
    auto manager = ConnectSignInManager::Create(&m_localState, m_secureStore);

    auto tokenStr =
        R"(<saml:Assertion MajorVersion="1" MinorVersion="1" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion">
            <saml:Conditions 
                NotBefore   ="2016-02-24T10:48:17.584Z"
                NotOnOrAfter="2016-02-24T10:48:17.584Z">
            </saml:Conditions>
            <ds:Signature xmlns:ds="http://www.w3.org/2000/09/xmldsig#">
                <KeyInfo xmlns="http://www.w3.org/2000/09/xmldsig#">
                    <X509Data>
                        <X509Certificate>TestCert</X509Certificate>
                    </X509Data>
                </KeyInfo>
            </ds:Signature>
            <saml:AttributeStatement>
                <saml:Attribute AttributeName="givenname">
                    <saml:AttributeValue>ValueA</saml:AttributeValue>
                </saml:Attribute>
                <saml:Attribute AttributeName="surname">
                    <saml:AttributeValue>ValueB</saml:AttributeValue>
                </saml:Attribute>
                <saml:Attribute AttributeName="userid">
                    <saml:AttributeValue>ValueC</saml:AttributeValue>
                </saml:Attribute>
                <saml:Attribute AttributeName="name">
                    <saml:AttributeValue>ValueD</saml:AttributeValue>
                </saml:Attribute>
            </saml:AttributeStatement>
        </saml:Assertion>)";

    auto token = std::make_shared<SamlToken>(tokenStr);
    ASSERT_TRUE(manager->SignInWithToken(token)->GetResult().IsSuccess());

    auto info = manager->GetUserInfo();

    EXPECT_EQ("ValueA", info.firstName);
    EXPECT_EQ("ValueB", info.lastName);
    EXPECT_EQ("ValueC", info.userId);
    EXPECT_EQ("ValueD", info.username);
    }

TEST_F(ConnectSignInManagerTests, GetUserInfo_SignedInWithTokenAndSignedOut_ReturnsEmpty)
    {
    auto manager = ConnectSignInManager::Create(&m_localState, m_secureStore);

    ASSERT_TRUE(manager->SignInWithToken(StubSamlToken())->GetResult().IsSuccess());
    manager->SignOut();

    auto info = manager->GetUserInfo();

    EXPECT_EQ("", info.firstName);
    EXPECT_EQ("", info.lastName);
    EXPECT_EQ("", info.userId);
    EXPECT_EQ("", info.username);
    }

#endif
