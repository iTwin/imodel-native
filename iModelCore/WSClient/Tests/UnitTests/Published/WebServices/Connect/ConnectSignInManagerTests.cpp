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
#include "MockImsClient.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace ::testing;

#ifdef USE_GTEST
void ConnectSignInManagerTests::SetUp()
    {
    BaseMockHttpHandlerTest::SetUp();

    m_imsClient = std::make_shared<StubImsClient>();
    m_buddiClient = std::make_shared<StubBuddiClient>();
    m_secureStore = std::make_shared<StubSecureStore>();
    m_localState = StubLocalState();

    ConnectAuthenticationPersistence::CustomInitialize(&m_localState, m_secureStore);
    StubUrlProviderEnvironment(UrlProvider::Environment::Dev);
    }

void ConnectSignInManagerTests::StubUrlProviderEnvironment(UrlProvider::Environment env)
    {
    UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, &m_localState, m_buddiClient);
    }

TEST_F(ConnectSignInManagerTests, GetAuthenticationHandler_UrlProviderProduction_SetsValidateCertificateForAllRequests)
    {
    StubUrlProviderEnvironment(UrlProvider::Environment::Release);
    auto manager = ConnectSignInManager::Create(m_imsClient,  &m_localState, m_secureStore);
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
    auto manager = ConnectSignInManager::Create(m_imsClient, &m_localState, m_secureStore);
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
    auto manager = ConnectSignInManager::Create(m_imsClient, &m_localState, m_secureStore);
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
    auto imsClient = std::make_shared<NiceMock<MockImsClient>>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    Credentials creds("Foo", "Boo");
    SamlTokenPtr identityToken = StubSamlToken();

    EXPECT_CALL(*imsClient, RequestToken(creds, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken))));
    ASSERT_TRUE(manager->SignInWithCredentials(creds)->GetResult().IsSuccess());

    auto authHandler1 = manager->GetAuthenticationHandler("https://foo.com", GetHandlerPtr());
    auto authHandler2 = manager->GetAuthenticationHandler("https://foo.com", GetHandlerPtr());

    EXPECT_CALL(*imsClient, RequestToken(*identityToken, _, _)).Times(1)
        .WillRepeatedly(Return(CreateCompletedAsyncTask( SamlTokenResult::Success(StubSamlToken()))));
    GetHandler().ForAnyRequest(StubImsTokenHttpResponse());

    HttpRequest("https://foo.com/a", "GET", authHandler1).Perform();
    HttpRequest("https://foo.com/b", "GET", authHandler2).Perform();
    }

TEST_F(ConnectSignInManagerTests, GetAuthenticationHandler_TwoRequestsSentInParaleleUsingDifferentAuthHandlersWithSameServer_OnlyOneTokenRequestSent)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    Credentials creds("Foo", "Boo");
    SamlTokenPtr identityToken = StubSamlToken();

    EXPECT_CALL(*imsClient, RequestToken(creds, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken))));
    ASSERT_TRUE(manager->SignInWithCredentials(creds)->GetResult().IsSuccess());

    AsyncTestCheckpoint checkpoint;
    GetHandler().ForAnyRequest([&] (HttpRequestCR request)
        {
        checkpoint.CheckinAndWait();
        return StubHttpResponse();
        });

    auto authHandler = manager->GetAuthenticationHandler("https://foo.com", GetHandlerPtr());

    EXPECT_CALL(*imsClient, RequestToken(*identityToken, _, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken()))));

    auto t1 = HttpRequest("https://foo.com/a", "GET", authHandler).PerformAsync();
    auto t2 = HttpRequest("https://foo.com/b", "GET", authHandler).PerformAsync();

    checkpoint.WaitUntilReached();
    checkpoint.Continue();

    t1->Wait();
    t2->Wait();
    }

TEST_F(ConnectSignInManagerTests, GetUserInfo_NotSignedIn_ReturnsEmpty)
    {
    auto manager = ConnectSignInManager::Create(m_imsClient, &m_localState, m_secureStore);

    auto info = manager->GetUserInfo();

    EXPECT_EQ("", info.firstName);
    EXPECT_EQ("", info.lastName);
    EXPECT_EQ("", info.userId);
    EXPECT_EQ("", info.username);
    }

TEST_F(ConnectSignInManagerTests, GetUserInfo_SignedInWithToken_ReturnsValuesFromToken)
    {
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

    m_imsClient->stubToken = std::make_shared<SamlToken>(tokenStr);
    auto manager = ConnectSignInManager::Create(m_imsClient, &m_localState, m_secureStore);
    ASSERT_TRUE(manager->SignInWithToken(StubSamlToken())->GetResult().IsSuccess());

    auto info = manager->GetUserInfo();

    EXPECT_EQ("ValueA", info.firstName);
    EXPECT_EQ("ValueB", info.lastName);
    EXPECT_EQ("ValueC", info.userId);
    EXPECT_EQ("ValueD", info.username);
    }

TEST_F(ConnectSignInManagerTests, GetUserInfo_SignedInWithTokenAndSignedOut_ReturnsEmpty)
    {
    auto manager = ConnectSignInManager::Create(m_imsClient, &m_localState, m_secureStore);

    GetHandler().ForFirstRequest(StubImsTokenHttpResponse(*StubSamlToken()));
    ASSERT_TRUE(manager->SignInWithToken(StubSamlToken())->GetResult().IsSuccess());
    manager->SignOut();

    auto info = manager->GetUserInfo();

    EXPECT_EQ("", info.firstName);
    EXPECT_EQ("", info.lastName);
    EXPECT_EQ("", info.userId);
    EXPECT_EQ("", info.username);
    }

TEST_F(ConnectSignInManagerTests, GetTokenProvider_NotSignedIn_TokenProviderDoesNotReturnAnyTokens)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(m_imsClient, &m_localState, m_secureStore);
    auto provider = manager->GetTokenProvider("https://foo.com");

    EXPECT_EQ(nullptr, provider->GetToken());
    EXPECT_EQ(nullptr, provider->UpdateToken());
    }

TEST_F(ConnectSignInManagerTests, GetTokenProvider_SignedInAndSecondCall_ReturnsSameProvider)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    SamlTokenPtr signInToken = StubSamlToken();
    SamlTokenPtr identityToken = StubSamlToken();

    EXPECT_CALL(*imsClient, RequestToken(*signInToken, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken))));
    ASSERT_TRUE(manager->SignInWithToken(signInToken)->GetResult().IsSuccess());

    auto provider1 = manager->GetTokenProvider("https://foo.com");
    auto provider2 = manager->GetTokenProvider("https://foo.com");
    EXPECT_NE(nullptr, provider1);
    EXPECT_EQ(provider1, provider2);
    }

TEST_F(ConnectSignInManagerTests, GetTokenProvider_SignedInWithToken_UpdateTokenRetrievesDelegationTokenUsingIdentityToken)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    SamlTokenPtr signInToken = StubSamlToken();
    SamlTokenPtr identityToken = StubSamlToken();
    SamlTokenPtr delegationToken = StubSamlToken();

    EXPECT_CALL(*imsClient, RequestToken(*signInToken, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken))));
    ASSERT_TRUE(manager->SignInWithToken(signInToken)->GetResult().IsSuccess());

    auto provider = manager->GetTokenProvider("https://foo.com");

    EXPECT_EQ(nullptr, provider->GetToken());

    EXPECT_CALL(*imsClient, RequestToken(*identityToken, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(delegationToken))));
    EXPECT_EQ(delegationToken, provider->UpdateToken());
    }

TEST_F(ConnectSignInManagerTests, GetTokenProvider_SignedInWithTokenAndGetTokenCalledMultipleTimesAfterUpdate_ReturnsSameToken)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    SamlTokenPtr signInToken = StubSamlToken();
    SamlTokenPtr identityToken = StubSamlToken();
    SamlTokenPtr delegationToken = StubSamlToken();

    EXPECT_CALL(*imsClient, RequestToken(*signInToken, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken))));
    ASSERT_TRUE(manager->SignInWithToken(signInToken)->GetResult().IsSuccess());

    auto provider = manager->GetTokenProvider("https://foo.com");

    EXPECT_CALL(*imsClient, RequestToken(*identityToken, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(delegationToken))));
    EXPECT_EQ(delegationToken, provider->UpdateToken());

    auto token1 = provider->GetToken();
    auto token2 = provider->GetToken();
    EXPECT_EQ(delegationToken, token1);
    EXPECT_EQ(token1, token2);
    }

TEST_F(ConnectSignInManagerTests, GetTokenProvider_DelegationAndIdentityTokenRequestFailsDueToAuthentication_CallsExpirationHandlerAndReturnsNull)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    SamlTokenPtr signInToken = StubSamlToken();
    SamlTokenPtr identityToken = StubSamlToken();
    SamlTokenPtr delegationToken = StubSamlToken();

    EXPECT_CALL(*imsClient, RequestToken(*signInToken, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken))));
    ASSERT_TRUE(manager->SignInWithToken(signInToken)->GetResult().IsSuccess());

    int count = 0;
    manager->SetTokenExpiredHandler([&]
        {
        count++;
        });

    auto provider = manager->GetTokenProvider("https://foo.com");

    HttpError error(ConnectionStatus::OK, HttpStatus::Unauthorized);
    EXPECT_CALL(*imsClient, RequestToken(*identityToken, _, _)).Times(2).WillRepeatedly(Return(CreateCompletedAsyncTask(SamlTokenResult::Error(error))));
    EXPECT_EQ(nullptr, provider->UpdateToken());
    EXPECT_EQ(1, count);
    }

TEST_F(ConnectSignInManagerTests, GetTokenProvider_TokenRequestFailsDueToConnectionError_ReturnsNullWithoutCallingExpirationHandler)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    SamlTokenPtr signInToken = StubSamlToken();
    SamlTokenPtr identityToken = StubSamlToken();
    SamlTokenPtr delegationToken = StubSamlToken();

    EXPECT_CALL(*imsClient, RequestToken(*signInToken, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken))));
    ASSERT_TRUE(manager->SignInWithToken(signInToken)->GetResult().IsSuccess());

    int count = 0;
    manager->SetTokenExpiredHandler([&]
        {
        count++;
        });

    auto provider = manager->GetTokenProvider("https://foo.com");

    HttpError error(ConnectionStatus::CouldNotConnect, HttpStatus::None);
    EXPECT_CALL(*imsClient, RequestToken(*identityToken, _, _)).Times(1).WillRepeatedly(Return(CreateCompletedAsyncTask(SamlTokenResult::Error(error))));
    EXPECT_EQ(nullptr, provider->UpdateToken());
    EXPECT_EQ(0, count);
    }

TEST_F(ConnectSignInManagerTests, GetTokenProvider_TokenDelegationFailsButIdentityTokenRenewalSucceeds_GetsNewDelegationToken)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    SamlTokenPtr signInToken = StubSamlToken();
    SamlTokenPtr identityToken = StubSamlToken();
    SamlTokenPtr newIdentityToken = StubSamlToken();
    SamlTokenPtr delegationToken = StubSamlToken();

    InSequence seq;
    EXPECT_CALL(*imsClient, RequestToken(*signInToken, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken))));
    ASSERT_TRUE(manager->SignInWithToken(signInToken)->GetResult().IsSuccess());

    int count = 0;
    manager->SetTokenExpiredHandler([&]
        {
        count++;
        });

    auto provider = manager->GetTokenProvider("https://foo.com");

    HttpError error(ConnectionStatus::OK, HttpStatus::Unauthorized);
    EXPECT_CALL(*imsClient, RequestToken(*identityToken, Not(Utf8String()), _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Error(error))));
    EXPECT_CALL(*imsClient, RequestToken(*identityToken, Utf8String(), _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(newIdentityToken))));
    EXPECT_CALL(*imsClient, RequestToken(*newIdentityToken, Not(Utf8String()), _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(delegationToken))));

    EXPECT_EQ(delegationToken, provider->UpdateToken());
    EXPECT_EQ(0, count);
    }

#endif
