/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/ConnectAuthenticationHandlerTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConnectAuthenticationHandlerTests.h"
#include <WebServices/Connect/ConnectAuthenticationHandler.h>
#include <Bentley/Base64Utilities.h>
#include "MockConnectTokenProvider.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace ::testing;

#ifdef USE_GTEST
AuthenticationHandler::Attempt StubAttempt(Utf8StringCR url)
    {
    return AuthenticationHandler::Attempt(url, "", DateTime::GetCurrentTimeUtc(), 0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _ShouldRetryAuthentication_HttpAuthErrors_True)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ConnectAuthenticationHandler authHandler("Foo", provider, GetHandlerPtr());

    EXPECT_TRUE(authHandler._ShouldRetryAuthentication(StubHttpResponse(HttpStatus::Unauthorized)));
    EXPECT_TRUE(authHandler._ShouldRetryAuthentication(StubHttpResponse(HttpStatus::Forbidden)));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _ShouldRetryAuthentication_HttpNonAuthErrors_False)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ConnectAuthenticationHandler authHandler("Foo", provider, GetHandlerPtr());

    EXPECT_FALSE(authHandler._ShouldRetryAuthentication(StubHttpResponse(HttpStatus::OK)));
    EXPECT_FALSE(authHandler._ShouldRetryAuthentication(StubHttpResponse(HttpStatus::NotFound)));
    EXPECT_FALSE(authHandler._ShouldRetryAuthentication(StubHttpResponse(ConnectionStatus::CouldNotConnect)));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _ShouldRetryAuthentication_DatasourceNotFoundError_True)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ConnectAuthenticationHandler authHandler("Foo", provider, GetHandlerPtr());

    auto response = StubHttpResponse(HttpStatus::NotFound, R"({"errorId":"DatasourceNotFound"})");
    EXPECT_TRUE(authHandler._ShouldRetryAuthentication(response));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_DifferentBaseUrl_ReturnsErrorToStopAuthentication)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ConnectAuthenticationHandler authHandler("http://other.com", provider, GetHandlerPtr());

    auto result = authHandler._RetrieveAuthorization(StubAttempt("http://test.com/foo"))->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_SameBaseUrl_ReturnsExistingToken)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ConnectAuthenticationHandler authHandler("http://test.com", provider, GetHandlerPtr());

    SamlTokenPtr token = StubSamlToken(100);
    EXPECT_CALL(*provider, GetToken()).WillRepeatedly(Return(token));

    auto result = authHandler._RetrieveAuthorization(StubAttempt("http://test.com/foo"))->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(token->ToAuthorizationString(), result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_PersistedToken_ReturnsToken)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ConnectAuthenticationHandler authHandler("http://test.com", provider, GetHandlerPtr());

    SamlTokenPtr token = StubSamlToken(100);
    EXPECT_CALL(*provider, GetToken()).WillRepeatedly(Return(token));

    auto result = authHandler._RetrieveAuthorization(StubAttempt("http://test.com"))->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(token->ToAuthorizationString(), result.GetValue());
    EXPECT_NE(token->ToSAMLAuthorizationString(), result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_PersistedTokenWithSamlAuth_ReturnsSamlTokenStr)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ConnectAuthenticationHandler authHandler("http://test.com", provider, GetHandlerPtr(), true);

    SamlTokenPtr token = StubSamlToken(100);
    EXPECT_CALL(*provider, GetToken()).WillRepeatedly(Return(token));

    auto result = authHandler._RetrieveAuthorization(StubAttempt("http://test.com"))->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(token->ToSAMLAuthorizationString(), result.GetValue());
    EXPECT_NE(token->ToAuthorizationString(), result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_NoCachedTokenAndUpdateTokenFails_ReturnsError)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ConnectAuthenticationHandler authHandler("http://test.com", provider, GetHandlerPtr());

    EXPECT_CALL(*provider, GetToken()).WillOnce(Return(nullptr));
    EXPECT_CALL(*provider, UpdateToken()).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenPtr())));

    auto result = authHandler._RetrieveAuthorization(StubAttempt("http://test.com"))->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_TokenIsNotPersisted_UpdatesAndReturnsToken)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ConnectAuthenticationHandler authHandler("http://test.com", provider, GetHandlerPtr());

    SamlTokenPtr newToken = StubSamlToken(100);

    EXPECT_CALL(*provider, GetToken()).WillOnce(Return(nullptr));
    EXPECT_CALL(*provider, UpdateToken()).WillOnce(Return(CreateCompletedAsyncTask(newToken)));

    auto result = authHandler._RetrieveAuthorization(StubAttempt("http://test.com"))->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(newToken->ToAuthorizationString(), result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_NonLegacyModeAttemptedOnceWithTokenAuth_ReturnsErrorToStopAuthentication)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ON_CALL(*provider, GetToken()).WillByDefault(Return(StubSamlToken()));
    ON_CALL(*provider, UpdateToken()).WillByDefault(Return(CreateCompletedAsyncTask(StubSamlToken())));

    bool legacyMode = false;
    ConnectAuthenticationHandler authHandler("http://test.com", provider, GetHandlerPtr(), false, legacyMode);

    AuthenticationHandler::Attempt attempt("http://test.com", "token SomeTestToken", DateTime(), 1);
    auto result = authHandler._RetrieveAuthorization(attempt)->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_LegacyModeAttemptedOnceWithTokenAuth_RetrievesNewTokenAndRetries)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ConnectAuthenticationHandler authHandler("http://test.com", provider, GetHandlerPtr(), false); // default parameter legacyMode = true

    SamlTokenPtr newToken = StubSamlToken(100);

    EXPECT_CALL(*provider, GetToken()).WillOnce(Return(nullptr));
    EXPECT_CALL(*provider, UpdateToken()).WillOnce(Return(CreateCompletedAsyncTask(newToken)));

    AuthenticationHandler::Attempt attempt("http://test.com", "token SomeTestToken", DateTime(), 1);
    auto result = authHandler._RetrieveAuthorization(attempt)->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(newToken->ToAuthorizationString(), result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_LegacyModeAttemptedTwiceWithTokenAuth_ReturnsErrorToStopAuthentication)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ON_CALL(*provider, GetToken()).WillByDefault(Return(StubSamlToken()));
    ON_CALL(*provider, UpdateToken()).WillByDefault(Return(CreateCompletedAsyncTask(StubSamlToken())));

    ConnectAuthenticationHandler authHandler("http://test.com", provider, GetHandlerPtr(), false); // default parameter legacyMode = true

    AuthenticationHandler::Attempt attempt("http://test.com", "token SomeTestToken", DateTime(), 2);
    auto result = authHandler._RetrieveAuthorization(attempt)->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_NonLegacyModeAttemptedOnceWithSamlAuth_ReturnsErrorToStopAuthentication)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ON_CALL(*provider, GetToken()).WillByDefault(Return(StubSamlToken()));
    ON_CALL(*provider, UpdateToken()).WillByDefault(Return(CreateCompletedAsyncTask(StubSamlToken())));

    bool legacyMode = false;
    ConnectAuthenticationHandler authHandler("http://test.com", provider, GetHandlerPtr(), true, legacyMode);

    AuthenticationHandler::Attempt attempt("http://test.com", "SAML SomeTestToken", DateTime(), 1);
    auto result = authHandler._RetrieveAuthorization(attempt)->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_LegacyModeAttemptedOnceWithSamlAuth_RetrievesNewTokenAndRetries)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ConnectAuthenticationHandler authHandler("http://test.com", provider, GetHandlerPtr(), true); // default parameter legacyMode = true

    SamlTokenPtr newToken = StubSamlToken(100);

    EXPECT_CALL(*provider, GetToken()).WillOnce(Return(nullptr));
    EXPECT_CALL(*provider, UpdateToken()).WillOnce(Return(CreateCompletedAsyncTask(newToken)));

    AuthenticationHandler::Attempt attempt("http://test.com", "SAML SomeTestToken", DateTime(), 1);
    auto result = authHandler._RetrieveAuthorization(attempt)->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(newToken->ToSAMLAuthorizationString(), result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_LegacyModeAttemptedTwiceWithSamlAuth_ReturnsErrorToStopAuthentication)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ON_CALL(*provider, GetToken()).WillByDefault(Return(StubSamlToken()));
    ON_CALL(*provider, UpdateToken()).WillByDefault(Return(CreateCompletedAsyncTask(StubSamlToken())));

    ConnectAuthenticationHandler authHandler("http://test.com", provider, GetHandlerPtr(), true); // default parameter legacyMode = true

    AuthenticationHandler::Attempt attempt("http://test.com", "SAML SomeTestToken", DateTime(), 2);
    auto result = authHandler._RetrieveAuthorization(attempt)->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_AttemptedTwiceAndIsNotTokenAuth_UsesExistingToken)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ConnectAuthenticationHandler authHandler("http://test.com", provider, GetHandlerPtr());

    SamlTokenPtr token = StubSamlToken(100);
    EXPECT_CALL(*provider, GetToken()).WillRepeatedly(Return(token));

    AuthenticationHandler::Attempt attempt("http://test.com", "TestOtherAuth", DateTime::GetCurrentTimeUtc(), 1);
    auto result = authHandler._RetrieveAuthorization(attempt)->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(token->ToAuthorizationString(), result.GetValue());
    }
#endif
