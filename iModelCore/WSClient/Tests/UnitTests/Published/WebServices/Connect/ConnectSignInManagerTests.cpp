/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/ConnectSignInManagerTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConnectSignInManagerTests.h"
#include <WebServices/Connect/ConnectSignInManager.h>
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include "MockImsClient.h"
#include "MockConnectionClientInterface.h"
#include "MockConnectSignInManagerListener.h"

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, Create_AfterSignIn_NoRequestsAreSent)
    {
    SamlTokenPtr token = StubSamlToken();
    auto imsClient = std::make_shared<MockImsClient>();

    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);
    EXPECT_CALL(*imsClient, RequestToken(*token, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(token))));
    ASSERT_TRUE(manager->SignInWithToken(token)->GetResult().IsSuccess());
    manager->FinalizeSignIn();

    manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);
    AsyncTasksManager::GetDefaultScheduler()->OnEmpty()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, CheckAndUpdateToken_Default_DoesNotRequestNewToken)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    manager->CheckAndUpdateToken();
    AsyncTasksManager::GetDefaultScheduler()->OnEmpty()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, CheckAndUpdateToken_AfterSignInAndTimeoutIsLargerThanTimePassed_DoesNotRequestNewToken)
    {
    SamlTokenPtr token = StubSamlToken();
    auto imsClient = std::make_shared<MockImsClient>();

    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);
    EXPECT_CALL(*imsClient, RequestToken(*token, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(token))));
    ASSERT_TRUE(manager->SignInWithToken(token)->GetResult().IsSuccess());

    manager->CheckAndUpdateToken();
    AsyncTasksManager::GetDefaultScheduler()->OnEmpty()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, CheckAndUpdateToken_AfterSignInAndConfiguredTimeout_RequestsNewToken)
    {
    SamlTokenPtr token = StubSamlToken();
    auto imsClient = std::make_shared<MockImsClient>();

    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);
    EXPECT_CALL(*imsClient, RequestToken(*token, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(token))));
    ASSERT_TRUE(manager->SignInWithToken(token)->GetResult().IsSuccess());

    ConnectSignInManager::Configuration config;
    config.identityTokenRefreshRate = 0;

    BeThreadUtilities::BeSleep(10);
    EXPECT_CALL(*imsClient, RequestToken(*token, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(token))));
    manager->Configure(config);

    BeThreadUtilities::BeSleep(10);
    EXPECT_CALL(*imsClient, RequestToken(*token, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(token))));
    manager->CheckAndUpdateToken();
    AsyncTasksManager::GetDefaultScheduler()->OnEmpty()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetAuthenticationHandler_UrlProviderProduction_SetsValidateCertificateForAllRequests)
    {
    StubUrlProviderEnvironment(UrlProvider::Environment::Release);
    auto manager = ConnectSignInManager::Create(m_imsClient, &m_localState, m_secureStore);
    auto authHandler = manager->GetAuthenticationHandler("https://foo.com", GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForFirstRequest([=] (Http::RequestCR request)
        {
        EXPECT_TRUE(request.GetValidateCertificate());
        return StubHttpResponse();
        });

    Http::Request("https://foo.com", "GET", authHandler).Perform().get();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetAuthenticationHandler_UrlProviderQa_DoesNotSetSetValidateCertificateForRequests)
    {
    StubUrlProviderEnvironment(UrlProvider::Environment::Qa);
    auto manager = ConnectSignInManager::Create(m_imsClient, &m_localState, m_secureStore);
    auto authHandler = manager->GetAuthenticationHandler("https://foo.com", GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForFirstRequest([=] (Http::RequestCR request)
        {
        EXPECT_FALSE(request.GetValidateCertificate());
        return StubHttpResponse();
        });

    Http::Request("https://foo.com", "GET", authHandler).Perform().get();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetAuthenticationHandler_UrlProviderDev_DoesNotSetSetValidateCertificateForRequests)
    {
    StubUrlProviderEnvironment(UrlProvider::Environment::Dev);
    auto manager = ConnectSignInManager::Create(m_imsClient, &m_localState, m_secureStore);
    auto authHandler = manager->GetAuthenticationHandler("https://foo.com", GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForFirstRequest([=] (Http::RequestCR request)
        {
        EXPECT_FALSE(request.GetValidateCertificate());
        return StubHttpResponse();
        });

    Http::Request("https://foo.com", "GET", authHandler).Perform().get();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
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
        .WillRepeatedly(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken()))));
    GetHandler().ForAnyRequest(StubImsTokenHttpResponse());

    Http::Request("https://foo.com/a", "GET", authHandler1).Perform().get();
    Http::Request("https://foo.com/b", "GET", authHandler2).Perform().get();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetAuthenticationHandler_TwoRequestsSentInParaleleUsingDifferentAuthHandlersWithSameServer_OnlyOneTokenRequestSent)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    Credentials creds("Foo", "Boo");
    SamlTokenPtr identityToken = StubSamlToken();

    EXPECT_CALL(*imsClient, RequestToken(creds, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken))));
    ASSERT_TRUE(manager->SignInWithCredentials(creds)->GetResult().IsSuccess());

    AsyncTestCheckpoint checkpoint;
    GetHandler().ForAnyRequest([&] (Http::RequestCR request)
        {
        checkpoint.CheckinAndWait();
        return StubHttpResponse();
        });

    auto authHandler = manager->GetAuthenticationHandler("https://foo.com", GetHandlerPtr());

    EXPECT_CALL(*imsClient, RequestToken(*identityToken, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken()))));

    auto t1 = Http::Request("https://foo.com/a", "GET", authHandler).PerformAsync();
    auto t2 = Http::Request("https://foo.com/b", "GET", authHandler).PerformAsync();

    checkpoint.WaitUntilReached();
    checkpoint.Continue();

    t1->Wait();
    t2->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetAuthenticationHandler_RequestFailsWithAuthenticationError_NewDelegationTokenWasRequested1Time)
    {
    // This test guards of too many Forbidden requests being done
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    Credentials creds("Foo", "Boo");
    SamlTokenPtr identityToken = StubSamlToken();

    EXPECT_CALL(*imsClient, RequestToken(creds, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken))));
    ASSERT_TRUE(manager->SignInWithCredentials(creds)->GetResult().IsSuccess());
    EXPECT_CALL(*imsClient, RequestToken(*identityToken, _, _)).Times(2).WillRepeatedly(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken()))));
    GetHandler().ForAnyRequest(StubHttpResponse(HttpStatus::Forbidden));

    auto authHandler = manager->GetAuthenticationHandler("https://foo.com", GetHandlerPtr());

    EXPECT_EQ(0, GetHandler().GetRequestsPerformed());
    Http::Response response = Http::Request("https://foo.com/a", "GET", authHandler).PerformAsync()->GetResult();
    EXPECT_EQ(2, GetHandler().GetRequestsPerformed());

    EXPECT_EQ(HttpStatus::Forbidden, response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetAuthenticationHandler_DefaultAuthenticationHandler_SendsWithAuthorizationString)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    Credentials creds("Foo", "Boo");
    SamlTokenPtr identityToken = StubSamlToken();
    SamlTokenPtr delegationToken = StubSamlToken();

    EXPECT_CALL(*imsClient, RequestToken(creds, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken))));
    ASSERT_TRUE(manager->SignInWithCredentials(creds)->GetResult().IsSuccess());

    GetHandler().ForAnyRequest([&] (Http::RequestCR request)
        {
        EXPECT_FALSE(delegationToken->ToAuthorizationString().empty());
        EXPECT_STREQ(("token " + delegationToken->ToAuthorizationString()).c_str(), request.GetHeaders().GetAuthorization());
        return StubHttpResponse();
        });

    auto authHandler = manager->GetAuthenticationHandler("https://foo.com", GetHandlerPtr()); // HeaderPrefix::Token defaulted

    EXPECT_CALL(*imsClient, RequestToken(*identityToken, _, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(delegationToken))));

    Http::Request("https://foo.com/a", "GET", authHandler).PerformAsync()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetAuthenticationHandler_TokenAuthenticationHandler_SendsWithAuthorizationString)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    Credentials creds("Foo", "Boo");
    SamlTokenPtr identityToken = StubSamlToken();
    SamlTokenPtr delegationToken = StubSamlToken();

    EXPECT_CALL(*imsClient, RequestToken(creds, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken))));
    ASSERT_TRUE(manager->SignInWithCredentials(creds)->GetResult().IsSuccess());

    GetHandler().ForAnyRequest([&] (Http::RequestCR request)
        {
        EXPECT_FALSE(delegationToken->ToAuthorizationString().empty());
        EXPECT_STREQ(("token " + delegationToken->ToAuthorizationString()).c_str(), request.GetHeaders().GetAuthorization());
        return StubHttpResponse();
        });

    auto authHandler = manager->GetAuthenticationHandler("https://foo.com", GetHandlerPtr(), ConnectSignInManager::HeaderPrefix::Token);

    EXPECT_CALL(*imsClient, RequestToken(*identityToken, _, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(delegationToken))));

    Http::Request("https://foo.com/a", "GET", authHandler).PerformAsync()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetAuthenticationHandler_TokenAuthenticationHandler_SendsWithSAMLAuthorizationString)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    Credentials creds("Foo", "Boo");
    SamlTokenPtr identityToken = StubSamlToken();
    SamlTokenPtr delegationToken = StubSamlToken();

    EXPECT_CALL(*imsClient, RequestToken(creds, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken))));
    ASSERT_TRUE(manager->SignInWithCredentials(creds)->GetResult().IsSuccess());

    GetHandler().ForAnyRequest([&] (Http::RequestCR request)
        {
        EXPECT_STREQ(("SAML " + delegationToken->ToAuthorizationString()).c_str(), request.GetHeaders().GetAuthorization());
        return StubHttpResponse();
        });

    auto authHandler = manager->GetAuthenticationHandler("https://foo.com", GetHandlerPtr(), ConnectSignInManager::HeaderPrefix::Saml);

    EXPECT_CALL(*imsClient, RequestToken(*identityToken, _, _)).Times(1)
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(delegationToken))));

    Http::Request("https://foo.com/a", "GET", authHandler).PerformAsync()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetUserInfo_NotSignedIn_ReturnsEmpty)
    {
    auto manager = ConnectSignInManager::Create(m_imsClient, &m_localState, m_secureStore);

    auto info = manager->GetUserInfo();

    EXPECT_EQ("", info.firstName);
    EXPECT_EQ("", info.lastName);
    EXPECT_EQ("", info.userId);
    EXPECT_EQ("", info.username);
    EXPECT_EQ("", info.organizationId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
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
                <saml:Attribute AttributeName="organizationid">
                    <saml:AttributeValue>ValueE</saml:AttributeValue>
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
    EXPECT_EQ("ValueE", info.organizationId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
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
    EXPECT_EQ("", info.organizationId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetUserInfo_InvalidToken_ReturnsEmpty)
    {
    auto manager = ConnectSignInManager::Create(m_imsClient, &m_localState, m_secureStore);

    SamlToken token(Utf8String("InvalidToken"));
    auto info = manager->ReadUserInfo(token);

    EXPECT_EQ("", info.firstName);
    EXPECT_EQ("", info.lastName);
    EXPECT_EQ("", info.userId);
    EXPECT_EQ("", info.username);
    EXPECT_EQ("", info.organizationId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, UserInfo_NoOrganizationId_IsNotComplete)
    {
    ConnectSignInManager::UserInfo info;
    info.firstName = "FirstName";
    info.lastName = "LastName";
    info.userId = "userId";
    info.username = "username";
    info.organizationId = "";

    EXPECT_FALSE(info.IsComplete());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, UserInfo_NoUserId_IsNotComplete)
    {
    ConnectSignInManager::UserInfo info;
    info.firstName = "FirstName";
    info.lastName = "LastName";
    info.userId = "";
    info.username = "username";
    info.organizationId = "OrganizationId";

    EXPECT_FALSE(info.IsComplete());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, UserInfo_AllDataSet_IsComplete)
    {
    ConnectSignInManager::UserInfo info;
    info.firstName = "FirstName";
    info.lastName = "LastName";
    info.userId = "userId";
    info.username = "username";
    info.organizationId = "OrganizationId";

    EXPECT_TRUE(info.IsComplete());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, UserInfo_NoDataSet_IsNotComplete)
    {
    ConnectSignInManager::UserInfo info;
    info.firstName = "";
    info.lastName = "";
    info.userId = "";
    info.username = "";
    info.organizationId = "";

    EXPECT_FALSE(info.IsComplete());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetUserInfo_ValidToken_ReturnsValuesFromToken)
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
                <saml:Attribute AttributeName="organizationid">
                    <saml:AttributeValue>ValueE</saml:AttributeValue>
                </saml:Attribute>
            </saml:AttributeStatement>
        </saml:Assertion>)";

    SamlTokenPtr tokenPtr = std::make_shared<SamlToken>(tokenStr);
    auto manager = ConnectSignInManager::Create(m_imsClient, &m_localState, m_secureStore);

    auto info = manager->ReadUserInfo(*tokenPtr);

    EXPECT_EQ("ValueA", info.firstName);
    EXPECT_EQ("ValueB", info.lastName);
    EXPECT_EQ("ValueC", info.userId);
    EXPECT_EQ("ValueD", info.username);
    EXPECT_EQ("ValueE", info.organizationId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetLastUsername_NoPreviousUsers_ReturnEmpty)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    EXPECT_EQ("", manager->GetLastUsername());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetLastUsername_SignedIn_ReturnsCurrentUser)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    SamlTokenPtr token = StubSamlTokenWithUser("TestUser");
    EXPECT_CALL(*imsClient, RequestToken(*token, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(token))));
    ASSERT_TRUE(manager->SignInWithToken(token)->GetResult().IsSuccess());

    EXPECT_EQ("TestUser", manager->GetLastUsername());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetLastUsername_SignedInSecondTime_ReturnsCurrentUser)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    SamlTokenPtr token = StubSamlTokenWithUser("TestUserA");
    EXPECT_CALL(*imsClient, RequestToken(*token, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(token))));
    ASSERT_TRUE(manager->SignInWithToken(token)->GetResult().IsSuccess());

    manager->SignOut();

    token = StubSamlTokenWithUser("TestUserB");
    EXPECT_CALL(*imsClient, RequestToken(*token, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(token))));
    ASSERT_TRUE(manager->SignInWithToken(token)->GetResult().IsSuccess());

    EXPECT_EQ("TestUserB", manager->GetLastUsername());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetLastUsername_SignedOut_ReturnsPreviousUser)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    SamlTokenPtr token = StubSamlTokenWithUser("TestUser");
    EXPECT_CALL(*imsClient, RequestToken(*token, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(token))));
    ASSERT_TRUE(manager->SignInWithToken(token)->GetResult().IsSuccess());

    manager->SignOut();

    EXPECT_EQ("TestUser", manager->GetLastUsername());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetTokenProvider_NotSignedIn_TokenProviderDoesNotReturnAnyTokens)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(m_imsClient, &m_localState, m_secureStore);
    auto provider = manager->GetTokenProvider("https://foo.com");

    EXPECT_EQ(nullptr, provider->GetToken());
    EXPECT_EQ(nullptr, provider->UpdateToken()->GetResult());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetTokenProvider_SignedInWithTokenAndGetToken_ReturnsNullBecauseUpdateTokenIsRequired)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    SamlTokenPtr signInToken = StubSamlToken();
    SamlTokenPtr identityToken = StubSamlToken();

    EXPECT_CALL(*imsClient, RequestToken(*signInToken, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken))));
    ASSERT_TRUE(manager->SignInWithToken(signInToken)->GetResult().IsSuccess());

    auto provider = manager->GetTokenProvider("https://foo.com");
    EXPECT_EQ(nullptr, provider->GetToken());
    EXPECT_EQ(nullptr, provider->GetToken());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetTokenProvider_SignedInWithTokenAndUpdateToken_RetrievesDelegationTokenUsingIdentityToken)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    SamlTokenPtr signInToken = StubSamlToken();
    SamlTokenPtr identityToken = StubSamlToken();
    SamlTokenPtr delegationToken = StubSamlToken();

    EXPECT_CALL(*imsClient, RequestToken(*signInToken, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken))));
    ASSERT_TRUE(manager->SignInWithToken(signInToken)->GetResult().IsSuccess());

    auto provider = manager->GetTokenProvider("https://foo.com");
    ASSERT_EQ(nullptr, provider->GetToken());

    EXPECT_CALL(*imsClient, RequestToken(*identityToken, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(delegationToken))));
    EXPECT_EQ(delegationToken, provider->UpdateToken()->GetResult());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetTokenProvider_SignedOutAndTokensRequested_ReturnsNulls)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    SamlTokenPtr identityToken = StubSamlToken();
    SamlTokenPtr delegationToken = StubSamlToken();

    EXPECT_CALL(*imsClient, RequestToken(*identityToken, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken))));
    ASSERT_TRUE(manager->SignInWithToken(identityToken)->GetResult().IsSuccess());

    // Signed-in
    auto provider = manager->GetTokenProvider("https://foo.com");
    EXPECT_CALL(*imsClient, RequestToken(*identityToken, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(delegationToken))));
    EXPECT_EQ(nullptr, provider->GetToken());
    EXPECT_EQ(delegationToken, provider->UpdateToken()->GetResult());
    EXPECT_EQ(delegationToken, provider->GetToken());

    // Signed-out
    manager->SignOut();
    EXPECT_EQ(nullptr, provider->GetToken());
    EXPECT_EQ(nullptr, provider->UpdateToken()->GetResult());
    EXPECT_EQ(nullptr, provider->GetToken());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetTokenProvider_SignedOutAndSignedInAndTokensRequested_NewDelegationTokenReturned)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    SamlTokenPtr identityToken1 = StubSamlTokenWithUser("A");
    SamlTokenPtr identityToken2 = StubSamlTokenWithUser("B");
    SamlTokenPtr delegationToken1 = StubSamlToken();
    SamlTokenPtr delegationToken2 = StubSamlToken();

    // Sign-in
    EXPECT_CALL(*imsClient, RequestToken(*identityToken1, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken1))));
    ASSERT_TRUE(manager->SignInWithToken(identityToken1)->GetResult().IsSuccess());

    // User provider
    auto provider = manager->GetTokenProvider("https://foo.com");
    EXPECT_CALL(*imsClient, RequestToken(*identityToken1, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(delegationToken1))));
    EXPECT_EQ(nullptr, provider->GetToken());
    EXPECT_EQ(delegationToken1, provider->UpdateToken()->GetResult());
    EXPECT_EQ(delegationToken1, provider->GetToken());

    // Sign-in again
    manager->SignOut();
    EXPECT_CALL(*imsClient, RequestToken(*identityToken2, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken2))));
    ASSERT_TRUE(manager->SignInWithToken(identityToken2)->GetResult().IsSuccess());

    // Use same provider
    EXPECT_CALL(*imsClient, RequestToken(*identityToken2, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(delegationToken2))));
    EXPECT_EQ(nullptr, provider->GetToken());
    EXPECT_EQ(delegationToken2, provider->UpdateToken()->GetResult());
    EXPECT_EQ(delegationToken2, provider->GetToken());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetTokenProvider_SignedOutAndSignedInAndSignedOutAndTokensRequested_NewDelegationTokenReturned)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    SamlTokenPtr identityToken1 = StubSamlTokenWithUser("A");
    SamlTokenPtr identityToken2 = StubSamlTokenWithUser("B");
    SamlTokenPtr identityToken3 = StubSamlTokenWithUser("C");
    SamlTokenPtr delegationToken = StubSamlToken();

    // Sign-in
    EXPECT_CALL(*imsClient, RequestToken(*identityToken1, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken1))));
    ASSERT_TRUE(manager->SignInWithToken(identityToken1)->GetResult().IsSuccess());
    auto provider = manager->GetTokenProvider("https://foo.com");

    EXPECT_CALL(*imsClient, RequestToken(*identityToken1, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(delegationToken))));
    EXPECT_EQ(nullptr, provider->GetToken());
    EXPECT_EQ(delegationToken, provider->UpdateToken()->GetResult());
    EXPECT_EQ(delegationToken, provider->GetToken());

    // Sign-in again
    manager->SignOut();
    EXPECT_CALL(*imsClient, RequestToken(*identityToken2, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken2))));
    ASSERT_TRUE(manager->SignInWithToken(identityToken2)->GetResult().IsSuccess());

    EXPECT_CALL(*imsClient, RequestToken(*identityToken2, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(delegationToken))));
    EXPECT_EQ(nullptr, provider->GetToken());
    EXPECT_EQ(delegationToken, provider->UpdateToken()->GetResult());
    EXPECT_EQ(delegationToken, provider->GetToken());
    
    // Sign-in again
    manager->SignOut();
    EXPECT_CALL(*imsClient, RequestToken(*identityToken3, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken3))));
    ASSERT_TRUE(manager->SignInWithToken(identityToken3)->GetResult().IsSuccess());

    EXPECT_CALL(*imsClient, RequestToken(*identityToken3, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(delegationToken))));
    EXPECT_EQ(nullptr, provider->GetToken());
    EXPECT_EQ(delegationToken, provider->UpdateToken()->GetResult());
    EXPECT_EQ(delegationToken, provider->GetToken());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetTokenProvider_SignedOutAndSignedInWithFinalizeAndTokensRequested_NewDelegationTokenReturned)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    SamlTokenPtr identityToken1 = StubSamlTokenWithUser("A");
    SamlTokenPtr identityToken2 = StubSamlTokenWithUser("B");
    SamlTokenPtr delegationToken1 = StubSamlToken();
    SamlTokenPtr delegationToken2 = StubSamlToken();

    // Sign-in
    EXPECT_CALL(*imsClient, RequestToken(*identityToken1, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken1))));
    ASSERT_TRUE(manager->SignInWithToken(identityToken1)->GetResult().IsSuccess());
    manager->FinalizeSignIn();

    // User provider
    auto provider = manager->GetTokenProvider("https://foo.com");
    EXPECT_CALL(*imsClient, RequestToken(*identityToken1, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(delegationToken1))));
    EXPECT_EQ(nullptr, provider->GetToken());
    EXPECT_EQ(delegationToken1, provider->UpdateToken()->GetResult());
    EXPECT_EQ(delegationToken1, provider->GetToken());

    // Sign-in again
    manager->SignOut();
    EXPECT_CALL(*imsClient, RequestToken(*identityToken2, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken2))));
    ASSERT_TRUE(manager->SignInWithToken(identityToken2)->GetResult().IsSuccess());
    manager->FinalizeSignIn();

    // Use same provider
    EXPECT_CALL(*imsClient, RequestToken(*identityToken2, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(delegationToken2))));
    EXPECT_EQ(nullptr, provider->GetToken());
    EXPECT_EQ(delegationToken2, provider->UpdateToken()->GetResult());
    EXPECT_EQ(delegationToken2, provider->GetToken());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetTokenProvider_SignedInWithTokenAndGetTokenCalledMultipleTimesAfterUpdateWithLongLivedToken_ReturnsSameToken)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    SamlTokenPtr signInToken = StubSamlToken();
    SamlTokenPtr identityToken = StubSamlToken();
    SamlTokenPtr delegationToken = StubSamlToken(24*60*60);

    EXPECT_CALL(*imsClient, RequestToken(*signInToken, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken))));
    ASSERT_TRUE(manager->SignInWithToken(signInToken)->GetResult().IsSuccess());

    auto provider = manager->GetTokenProvider("https://foo.com");

    EXPECT_CALL(*imsClient, RequestToken(*identityToken, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(delegationToken))));
    EXPECT_EQ(delegationToken, provider->UpdateToken()->GetResult());

    auto token1 = provider->GetToken();
    auto token2 = provider->GetToken();
    EXPECT_EQ(delegationToken, token1);
    EXPECT_EQ(token1, token2);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetTokenProvider_SignedInWithTokenAndGetTokenCalledMultipleTimesAfterUpdateWithShortLivedToken_DoesNotReturnExpiredToken)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    SamlTokenPtr signInToken = StubSamlToken();
    SamlTokenPtr identityToken = StubSamlToken();
    SamlTokenPtr delegationToken = StubSamlToken(0); // Short lifetime

    EXPECT_CALL(*imsClient, RequestToken(*signInToken, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(identityToken))));
    ASSERT_TRUE(manager->SignInWithToken(signInToken)->GetResult().IsSuccess());

    auto provider = manager->GetTokenProvider("https://foo.com");

    EXPECT_CALL(*imsClient, RequestToken(*identityToken, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(delegationToken))));
    EXPECT_EQ(delegationToken, provider->UpdateToken()->GetResult());

    EXPECT_EQ(nullptr, provider->GetToken());
    EXPECT_EQ(nullptr, provider->GetToken());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
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
    manager->SetTokenExpiredHandler([&] { count++; });

    NiceMock<MockConnectSignInManagerListener> listener;
    EXPECT_CALL(listener, _OnUserTokenExpired()).Times(1);
    manager->RegisterListener(&listener);

    auto provider = manager->GetTokenProvider("https://foo.com");

    HttpError error(ConnectionStatus::OK, HttpStatus::Unauthorized);
    EXPECT_CALL(*imsClient, RequestToken(*identityToken, _, _)).Times(2).WillRepeatedly(Return(CreateCompletedAsyncTask(SamlTokenResult::Error(error))));
    EXPECT_EQ(nullptr, provider->UpdateToken()->GetResult());
    EXPECT_EQ(1, count);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
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
    manager->SetTokenExpiredHandler([&] { count++; });

    NiceMock<MockConnectSignInManagerListener> listener;
    EXPECT_CALL(listener, _OnUserTokenExpired()).Times(0);
    manager->RegisterListener(&listener);

    auto provider = manager->GetTokenProvider("https://foo.com");

    HttpError error(ConnectionStatus::CouldNotConnect, HttpStatus::None);
    EXPECT_CALL(*imsClient, RequestToken(*identityToken, _, _)).Times(1).WillRepeatedly(Return(CreateCompletedAsyncTask(SamlTokenResult::Error(error))));
    EXPECT_EQ(nullptr, provider->UpdateToken()->GetResult());
    EXPECT_EQ(0, count);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
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
    manager->SetTokenExpiredHandler([&] { count++; });

    NiceMock<MockConnectSignInManagerListener> listener;
    EXPECT_CALL(listener, _OnUserTokenExpired()).Times(0);
    manager->RegisterListener(&listener);

    auto provider = manager->GetTokenProvider("https://foo.com");

    HttpError error(ConnectionStatus::OK, HttpStatus::Unauthorized);
    EXPECT_CALL(*imsClient, RequestToken(*identityToken, Not(Utf8String()), _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Error(error))));
    EXPECT_CALL(*imsClient, RequestToken(*identityToken, Utf8String(), _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(newIdentityToken))));
    EXPECT_CALL(*imsClient, RequestToken(*newIdentityToken, Not(Utf8String()), _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(delegationToken))));

    EXPECT_EQ(delegationToken, provider->UpdateToken()->GetResult());
    EXPECT_EQ(0, count);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, SignInWithToken_FirstTime_UserChangeHandlerNotCalled)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    int count = 0;
    manager->SetUserChangeHandler([&] { count++; });

    NiceMock<MockConnectSignInManagerListener> listener;
    EXPECT_CALL(listener, _OnUserChanged()).Times(0);
    manager->RegisterListener(&listener);

    EXPECT_CALL(*imsClient, RequestToken(An<SamlTokenCR>(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken({{"name", "TestUser"}})))));

    ASSERT_TRUE(manager->SignInWithToken(StubSamlToken())->GetResult().IsSuccess());

    EXPECT_EQ(0, count);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, SignInWithToken_SecondTimeWithSameUser_UserChangeHandlerNotCalled)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    int count = 0;
    manager->SetUserChangeHandler([&] { count++; });

    NiceMock<MockConnectSignInManagerListener> listener;
    EXPECT_CALL(listener, _OnUserChanged()).Times(0);
    manager->RegisterListener(&listener);

    EXPECT_CALL(*imsClient, RequestToken(An<SamlTokenCR>(), _, _))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken({{"name", "TestUser"}})))));

    ASSERT_TRUE(manager->SignInWithToken(StubSamlToken())->GetResult().IsSuccess());
    ASSERT_TRUE(manager->SignInWithToken(StubSamlToken())->GetResult().IsSuccess());

    EXPECT_EQ(0, count);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, SignInWithToken_SecondTimeAfterSignOutWithSameUser_UserChangeHandlerNotCalled)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    int count = 0;
    manager->SetUserChangeHandler([&] { count++; });

    NiceMock<MockConnectSignInManagerListener> listener;
    EXPECT_CALL(listener, _OnUserChanged()).Times(0);
    manager->RegisterListener(&listener);

    EXPECT_CALL(*imsClient, RequestToken(An<SamlTokenCR>(), _, _))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken({{"name", "TestUser"}})))));

    ASSERT_TRUE(manager->SignInWithToken(StubSamlToken())->GetResult().IsSuccess());
    manager->SignOut();
    ASSERT_TRUE(manager->SignInWithToken(StubSamlToken())->GetResult().IsSuccess());

    EXPECT_EQ(0, count);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, SignInWithToken_SecondTimeAfterSignOutWithDifferentUser_UserChangeHandlerCalled)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    int count = 0;
    manager->SetUserChangeHandler([&] { count++; });

    NiceMock<MockConnectSignInManagerListener> listener;
    EXPECT_CALL(listener, _OnUserChanged()).Times(1);
    manager->RegisterListener(&listener);

    InSequence seq;
    EXPECT_CALL(*imsClient, RequestToken(An<SamlTokenCR>(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken({{"name", "TestUserA"}})))));
    EXPECT_CALL(*imsClient, RequestToken(An<SamlTokenCR>(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken({{"name", "TestUserB"}})))));

    ASSERT_TRUE(manager->SignInWithToken(StubSamlToken())->GetResult().IsSuccess());
    manager->SignOut();
    ASSERT_TRUE(manager->SignInWithToken(StubSamlToken())->GetResult().IsSuccess());

    EXPECT_EQ(1, count);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, SignInWithCredentials_SecondTimeAfterSignOutWithDifferentUser_UserChangeHandlerCalled)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    int count = 0;
    manager->SetUserChangeHandler([&] { count++; });

    NiceMock<MockConnectSignInManagerListener> listener;
    EXPECT_CALL(listener, _OnUserChanged()).Times(1);
    manager->RegisterListener(&listener);

    InSequence seq;
    EXPECT_CALL(*imsClient, RequestToken(An<CredentialsCR>(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken({{"name", "TestUserA"}})))));
    EXPECT_CALL(*imsClient, RequestToken(An<CredentialsCR>(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken({{"name", "TestUserB"}})))));

    ASSERT_TRUE(manager->SignInWithCredentials({"Foo", "Boo"})->GetResult().IsSuccess());
    manager->SignOut();
    ASSERT_TRUE(manager->SignInWithCredentials({"Foo", "Boo"})->GetResult().IsSuccess());

    EXPECT_EQ(1, count);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, SignInWithToken_DifferentUserInOtherSignInManagerWithSharedSecureStore_UserDoesNotChangeForOtherManager)
    {
    // Mimics two apps signing-in independently with shared secure store

    auto imsClient1 = std::make_shared<MockImsClient>();
    auto imsClient2 = std::make_shared<MockImsClient>();
    StubLocalState localState1;
    StubLocalState localState2;

    EXPECT_CALL(*imsClient1, RequestToken(An<SamlTokenCR>(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken({{"name", "TestUserA"}})))));
    EXPECT_CALL(*imsClient2, RequestToken(An<SamlTokenCR>(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken({{"name", "TestUserB"}})))));

    // App 1
    auto manager1 = ConnectSignInManager::Create(imsClient1, &localState1, m_secureStore);
    ASSERT_FALSE(manager1->IsSignedIn());
    ASSERT_TRUE(manager1->SignInWithToken(StubSamlToken())->GetResult().IsSuccess());
    manager1->FinalizeSignIn();
    ASSERT_EQ("TestUserA", manager1->GetUserInfo().username);

    // App 2
    auto manager2 = ConnectSignInManager::Create(imsClient2, &localState2, m_secureStore);
    ASSERT_FALSE(manager2->IsSignedIn());
    ASSERT_TRUE(manager2->SignInWithToken(StubSamlToken())->GetResult().IsSuccess());
    manager2->FinalizeSignIn();
    EXPECT_EQ("TestUserB", manager2->GetUserInfo().username);

    // App 1 restarts - no change
    manager1 = ConnectSignInManager::Create(imsClient1, &localState1, m_secureStore);
    EXPECT_EQ("TestUserA", manager1->GetUserInfo().username);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, SignInWithCredentials_DifferentUserInOtherSignInManagerWithSharedSecureStore_UserChangesForOtherManager)
    {
    // Mimics two apps signing-in independently with shared secure store

    auto imsClient1 = std::make_shared<MockImsClient>();
    auto imsClient2 = std::make_shared<MockImsClient>();
    StubLocalState localState1;
    StubLocalState localState2;

    EXPECT_CALL(*imsClient1, RequestToken(An<CredentialsCR>(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken({{"name", "TestUserA"}})))));
    EXPECT_CALL(*imsClient2, RequestToken(An<CredentialsCR>(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken({{"name", "TestUserB"}})))));

    // App 1
    auto manager1 = ConnectSignInManager::Create(imsClient1, &localState1, m_secureStore);
    ASSERT_FALSE(manager1->IsSignedIn());
    ASSERT_TRUE(manager1->SignInWithCredentials({"A", "PA"})->GetResult().IsSuccess());
    manager1->FinalizeSignIn();
    ASSERT_EQ("TestUserA", manager1->GetUserInfo().username);

    // App 2
    auto manager2 = ConnectSignInManager::Create(imsClient2, &localState2, m_secureStore);
    ASSERT_FALSE(manager2->IsSignedIn());
    ASSERT_TRUE(manager2->SignInWithCredentials({"B", "PB"})->GetResult().IsSuccess());
    manager2->FinalizeSignIn();
    EXPECT_EQ("TestUserB", manager2->GetUserInfo().username);

    // App 1 restarts - different user
    manager1 = ConnectSignInManager::Create(imsClient1, &localState1, m_secureStore);
    EXPECT_EQ("TestUserB", manager1->GetUserInfo().username);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, SetUserChangeHandler_CreedentialsUserWasChangedInOtherSignInManagerWithSharedPersistence_HandlerCalledOnce)
    {
    // Mimics two apps signing-in independently with shared secure store

    auto imsClient1 = std::make_shared<MockImsClient>();
    auto imsClient2 = std::make_shared<MockImsClient>();
    StubLocalState localState1;
    StubLocalState localState2;

    EXPECT_CALL(*imsClient1, RequestToken(An<CredentialsCR>(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken({{"name", "TestUserA"}})))));
    EXPECT_CALL(*imsClient2, RequestToken(An<CredentialsCR>(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken({{"name", "TestUserB"}})))));

    // App 1
    auto manager1 = ConnectSignInManager::Create(imsClient1, &localState1, m_secureStore);
    ASSERT_TRUE(manager1->SignInWithCredentials({"A", "PA"})->GetResult().IsSuccess());
    manager1->FinalizeSignIn();
    ASSERT_EQ("TestUserA", manager1->GetUserInfo().username);

    // App 2
    auto manager2 = ConnectSignInManager::Create(imsClient2, &localState2, m_secureStore);
    ASSERT_FALSE(manager2->IsSignedIn());
    ASSERT_TRUE(manager2->SignInWithCredentials({"B", "PB"})->GetResult().IsSuccess());
    manager2->FinalizeSignIn();
    EXPECT_EQ("TestUserB", manager2->GetUserInfo().username);

    // App 1 restarts
    manager1 = ConnectSignInManager::Create(imsClient1, &localState1, m_secureStore);
    EXPECT_EQ("TestUserB", manager1->GetUserInfo().username);

    int count = 0;
    manager1->SetUserChangeHandler([&] { count++; });
    EXPECT_EQ(1, count);

    // App 1 restarts again
    manager1 = ConnectSignInManager::Create(imsClient1, &localState1, m_secureStore);
    EXPECT_EQ("TestUserB", manager1->GetUserInfo().username);

    count = 0;
    manager1->SetUserChangeHandler([&] { count++; });
    EXPECT_EQ(0, count);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, SetUserChangeHandler_CreedentialsUserWasChangedInOtherSignInManagerWithSharedPersistence_ListenerCalledOnce)
    {
    // Mimics two apps signing-in independently with shared secure store

    auto imsClient1 = std::make_shared<MockImsClient>();
    auto imsClient2 = std::make_shared<MockImsClient>();
    StubLocalState localState1;
    StubLocalState localState2;

    EXPECT_CALL(*imsClient1, RequestToken(An<CredentialsCR>(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken({{"name", "TestUserA"}})))));
    EXPECT_CALL(*imsClient2, RequestToken(An<CredentialsCR>(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken({{"name", "TestUserB"}})))));

    // App 1
    auto manager1 = ConnectSignInManager::Create(imsClient1, &localState1, m_secureStore);
    ASSERT_TRUE(manager1->SignInWithCredentials({"A", "PA"})->GetResult().IsSuccess());
    manager1->FinalizeSignIn();
    ASSERT_EQ("TestUserA", manager1->GetUserInfo().username);

    // App 2
    auto manager2 = ConnectSignInManager::Create(imsClient2, &localState2, m_secureStore);
    ASSERT_FALSE(manager2->IsSignedIn());
    ASSERT_TRUE(manager2->SignInWithCredentials({"B", "PB"})->GetResult().IsSuccess());
    manager2->FinalizeSignIn();
    EXPECT_EQ("TestUserB", manager2->GetUserInfo().username);

    // App 1 restarts
    manager1 = ConnectSignInManager::Create(imsClient1, &localState1, m_secureStore);
    EXPECT_EQ("TestUserB", manager1->GetUserInfo().username);

    NiceMock<MockConnectSignInManagerListener> listener;
    EXPECT_CALL(listener, _OnUserChanged()).Times(1);
    manager1->RegisterListener(&listener);
    Mock::VerifyAndClearExpectations(&listener);

    // App 1 restarts again
    manager1 = ConnectSignInManager::Create(imsClient1, &localState1, m_secureStore);
    EXPECT_EQ("TestUserB", manager1->GetUserInfo().username);

    EXPECT_CALL(listener, _OnUserChanged()).Times(0);
    manager1->RegisterListener(&listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, SignInWithToken_SignInHandlerSet_CallsHandlerAfterFinalize)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    int count = 0;
    manager->SetUserSignInHandler([&] { count++; });

    NiceMock<MockConnectSignInManagerListener> listener;
    EXPECT_CALL(listener, _OnUserSignedIn()).Times(1);
    manager->RegisterListener(&listener);

    InSequence seq;
    EXPECT_CALL(*imsClient, RequestToken(An<SamlTokenCR>(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken({{"name", "TestUser"}})))));

    ASSERT_TRUE(manager->SignInWithToken(StubSamlToken())->GetResult().IsSuccess());
    EXPECT_EQ(0, count);
    manager->FinalizeSignIn();
    EXPECT_EQ(1, count);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, UnregisterListener_ListenerWasSetAndSignIn_DoesNotCallSignInListenerAfterUnregister)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    EXPECT_CALL(*imsClient, RequestToken(An<SamlTokenCR>(), _, _))
        .WillRepeatedly(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken({{"name", "TestUser"}})))));

    NiceMock<MockConnectSignInManagerListener> listener1, listener2;
    manager->RegisterListener(&listener1);
    manager->RegisterListener(&listener2);

    EXPECT_CALL(listener1, _OnUserSignedIn()).Times(1);
    EXPECT_CALL(listener2, _OnUserSignedIn()).Times(1);

    ASSERT_TRUE(manager->SignInWithToken(StubSamlToken())->GetResult().IsSuccess());
    manager->FinalizeSignIn();
    Mock::VerifyAndClearExpectations(&listener1);
    Mock::VerifyAndClearExpectations(&listener2);

    EXPECT_CALL(listener1, _OnUserSignedIn()).Times(1);
    EXPECT_CALL(listener2, _OnUserSignedIn()).Times(0);
    manager->UnregisterListener(&listener2);

    ASSERT_TRUE(manager->SignInWithToken(StubSamlToken())->GetResult().IsSuccess());
    manager->FinalizeSignIn();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, SignOut_SignInHandlerSet_CallsHandler)
    {
    auto imsClient = std::make_shared<MockImsClient>();
    auto manager = ConnectSignInManager::Create(imsClient, &m_localState, m_secureStore);

    int count = 0;
    manager->SetUserSignOutHandler([&] { count++; });

    NiceMock<MockConnectSignInManagerListener> listener;
    EXPECT_CALL(listener, _OnUserSignedOut()).Times(1);
    manager->RegisterListener(&listener);

    InSequence seq;
    EXPECT_CALL(*imsClient, RequestToken(An<SamlTokenCR>(), _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(StubSamlToken({ { "name", "TestUser" } })))));

    ASSERT_TRUE(manager->SignInWithToken(StubSamlToken())->GetResult().IsSuccess());
    manager->FinalizeSignIn();

    EXPECT_EQ(0, count);
    manager->SignOut();
    EXPECT_EQ(1, count);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, IsConnectionClientInstalled_ReturnTrue)
    {
    auto connectionClient = std::make_shared<MockConnectionClientInterface>();
    auto manager = ConnectSignInManager::Create(m_imsClient, &m_localState, m_secureStore, connectionClient);

    EXPECT_CALL(*connectionClient, IsInstalled()).WillOnce(Return(true));
    auto result = manager->IsConnectionClientInstalled();
    ASSERT_TRUE(result);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetConnectionClientToken_ReturnSuccessWithToken)
    {
    auto connectionClient = std::make_shared<MockConnectionClientInterface>();
    auto manager = ConnectSignInManager::Create(m_imsClient, &m_localState, m_secureStore, connectionClient);

    Utf8StringCR uri = "";
    SamlTokenPtr token = StubSamlTokenWithUser("TestUser");
    ON_CALL(*connectionClient, GetSerializedDelegateSecurityToken(uri, _)).WillByDefault(Return(token));
    ON_CALL(*connectionClient, IsRunning()).WillByDefault(Return(true));

    EXPECT_CALL(*connectionClient, IsInstalled()).WillOnce(Return(true));
    EXPECT_CALL(*connectionClient, IsRunning()).WillOnce(Return(true));
    EXPECT_CALL(*connectionClient, StartClientApp()).Times(0);
    EXPECT_CALL(*connectionClient, IsLoggedIn()).WillOnce(Return(true));
    EXPECT_CALL(*connectionClient, GetSerializedDelegateSecurityToken(uri, _)).WillOnce(Return(token));
                                    
    auto result = manager->GetConnectionClientToken(uri);
    EXPECT_NE(nullptr, result);
    ASSERT_TRUE(result->GetResult().IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, GetConnectionClientToken_CantStartClient_ReturnError)
    {
    auto connectionClient = std::make_shared<MockConnectionClientInterface>();
    auto manager = ConnectSignInManager::Create(m_imsClient, &m_localState, m_secureStore, connectionClient);

    Utf8StringCR uri = "";
    SamlTokenPtr token = StubSamlTokenWithUser("TestUser");
    ON_CALL(*connectionClient, GetSerializedDelegateSecurityToken(uri, _)).WillByDefault(Return(token));
    ON_CALL(*connectionClient, IsRunning()).WillByDefault(Return(false));

    EXPECT_CALL(*connectionClient, IsInstalled()).WillOnce(Return(true));
    EXPECT_CALL(*connectionClient, IsRunning()).Times(2);
    EXPECT_CALL(*connectionClient, StartClientApp()).WillOnce(Return(BentleyStatus::SUCCESS));

    auto result = manager->GetConnectionClientToken(uri);
    EXPECT_NE(nullptr, result);
    ASSERT_FALSE(result->GetResult().IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Daumantas.Kojelis    7/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectSignInManagerTests, StartConnectionClientListener_StartsTwoTimes_OnlyOneListener)
    {
    auto connectionClient = std::make_shared<MockConnectionClientInterface>();
    auto manager = ConnectSignInManager::Create(m_imsClient, &m_localState, m_secureStore, connectionClient);

    ON_CALL(*connectionClient, IsInstalled()).WillByDefault(Return(true));
    ON_CALL(*connectionClient, IsRunning()).WillByDefault(Return(true)); 

    EXPECT_CALL(*connectionClient, AddClientEventListener(_)).Times(1);
    EXPECT_CALL(*connectionClient, IsInstalled()).Times(1);
    EXPECT_CALL(*connectionClient, IsRunning()).Times(1);

    manager->StartConnectionClientListener();
    manager->StartConnectionClientListener();
    }

#endif
