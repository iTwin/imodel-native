/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/ConnectAuthenticationHandlerTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    auto authHandler = ConnectAuthenticationHandler::Create("Foo", provider, GetHandlerPtr());

    EXPECT_TRUE(authHandler->_ShouldRetryAuthentication(StubHttpResponse(HttpStatus::Unauthorized)));
    EXPECT_TRUE(authHandler->_ShouldRetryAuthentication(StubHttpResponse(HttpStatus::Forbidden)));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _ShouldRetryAuthentication_HttpNonAuthErrors_False)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    auto authHandler = ConnectAuthenticationHandler::Create("Foo", provider, GetHandlerPtr());

    EXPECT_FALSE(authHandler->_ShouldRetryAuthentication(StubHttpResponse(HttpStatus::OK)));
    EXPECT_FALSE(authHandler->_ShouldRetryAuthentication(StubHttpResponse(HttpStatus::NotFound)));
    EXPECT_FALSE(authHandler->_ShouldRetryAuthentication(StubHttpResponse(ConnectionStatus::CouldNotConnect)));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _ShouldRetryAuthentication_DatasourceNotFoundError_True)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    auto authHandler = ConnectAuthenticationHandler::Create("Foo", provider, GetHandlerPtr());

    auto response = StubHttpResponse(HttpStatus::NotFound, R"({"errorId":"DatasourceNotFound"})");
    EXPECT_TRUE(authHandler->_ShouldRetryAuthentication(response));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_DifferentBaseUrl_ReturnsErrorToStopAuthentication)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    auto authHandler = ConnectAuthenticationHandler::Create("http://other.com", provider, GetHandlerPtr());

    auto result = authHandler->_RetrieveAuthorization(StubAttempt("http://test.com/foo"))->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_SameBaseUrl_ReturnsExistingToken)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    auto authHandler = ConnectAuthenticationHandler::Create("http://test.com", provider, GetHandlerPtr(), "prefix");

    SamlTokenPtr token = StubSamlToken(100);
    EXPECT_CALL(*provider, GetToken()).WillRepeatedly(Return(token));

    auto result = authHandler->_RetrieveAuthorization(StubAttempt("http://test.com/foo"))->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ("prefix " + token->ToAuthorizationString(), result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_SameBaseUrlWithDifferentCase_ReturnsExistingToken)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    auto authHandler = ConnectAuthenticationHandler::Create("hTtp://TeST.com", provider, GetHandlerPtr(), "prefix");

    SamlTokenPtr token = StubSamlToken(100);
    EXPECT_CALL(*provider, GetToken()).WillRepeatedly(Return(token));

    auto result = authHandler->_RetrieveAuthorization(StubAttempt("HttP://tESt.cOM/foo"))->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ("prefix " + token->ToAuthorizationString(), result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_PersistedToken_ReturnsToken)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    auto authHandler = ConnectAuthenticationHandler::Create("http://test.com", provider, GetHandlerPtr(), "prefix");

    SamlTokenPtr token = StubSamlToken(100);
    EXPECT_CALL(*provider, GetToken()).WillRepeatedly(Return(token));

    auto result = authHandler->_RetrieveAuthorization(StubAttempt("http://test.com"))->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ("prefix " + token->ToAuthorizationString(), result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_NoCachedTokenAndUpdateTokenFails_ReturnsError)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    auto authHandler = ConnectAuthenticationHandler::Create("http://test.com", provider, GetHandlerPtr());

    EXPECT_CALL(*provider, GetToken()).WillOnce(Return(nullptr));
    EXPECT_CALL(*provider, UpdateToken()).WillOnce(Return(CreateCompletedAsyncTask<ISecurityTokenPtr>(SamlTokenPtr())));

    auto result = authHandler->_RetrieveAuthorization(StubAttempt("http://test.com"))->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_TokenIsNotPersisted_UpdatesAndReturnsToken)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    auto authHandler = ConnectAuthenticationHandler::Create("http://test.com", provider, GetHandlerPtr(), "prefix");

    SamlTokenPtr newToken = StubSamlToken(100);

    EXPECT_CALL(*provider, GetToken()).WillOnce(Return(nullptr));
    EXPECT_CALL(*provider, UpdateToken()).WillOnce(Return(CreateCompletedAsyncTask<ISecurityTokenPtr>(newToken)));

    auto result = authHandler->_RetrieveAuthorization(StubAttempt("http://test.com"))->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ("prefix " +newToken->ToAuthorizationString(), result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_AttemptedOnce_ReturnsErrorToStopAuthentication)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ON_CALL(*provider, GetToken()).WillByDefault(Return(StubSamlToken()));
    ON_CALL(*provider, UpdateToken()).WillByDefault(Return(CreateCompletedAsyncTask<ISecurityTokenPtr>(StubSamlToken())));

    bool legacyMode = false;
    auto authHandler = ConnectAuthenticationHandler::Create("http://test.com", provider, GetHandlerPtr(), "prefix");

    AuthenticationHandler::Attempt attempt("http://test.com", "prefix SomeTestToken", DateTime(), 1);
    auto result = authHandler->_RetrieveAuthorization(attempt)->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_AttemptedTwiceAndIsDifferentAuth_UsesExistingToken)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    auto authHandler = ConnectAuthenticationHandler::Create("http://test.com", provider, GetHandlerPtr(), "prefix");

    SamlTokenPtr token = StubSamlToken(100);
    EXPECT_CALL(*provider, GetToken()).WillRepeatedly(Return(token));

    AuthenticationHandler::Attempt attempt("http://test.com", "TestOtherAuth", DateTime::GetCurrentTimeUtc(), 1);
    auto result = authHandler->_RetrieveAuthorization(attempt)->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ("prefix " + token->ToAuthorizationString(), result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_PersistedTokenWithLegacyHandlerWithTokenAuth_ReturnsSamlTokenStr)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    auto authHandler = ConnectAuthenticationHandler::CreateLegacy("http://test.com", provider, GetHandlerPtr(), false);
    SamlTokenPtr token = StubSamlToken(100);
    EXPECT_CALL(*provider, GetToken()).WillRepeatedly(Return(token));

    auto result = authHandler->_RetrieveAuthorization(StubAttempt("http://test.com"))->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ("token " + token->ToAuthorizationString(), result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_PersistedTokenWithLegacyHandlerWithSamlAuth_ReturnsSamlTokenStr)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    auto authHandler = ConnectAuthenticationHandler::CreateLegacy("http://test.com", provider, GetHandlerPtr(), true);
    SamlTokenPtr token = StubSamlToken(100);
    EXPECT_CALL(*provider, GetToken()).WillRepeatedly(Return(token));

    auto result = authHandler->_RetrieveAuthorization(StubAttempt("http://test.com"))->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ("SAML " + token->ToAuthorizationString(), result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_LegacyHandlerWithNonLegacyModeAttemptedOnceWithTokenAuth_ReturnsErrorToStopAuthentication)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ON_CALL(*provider, GetToken()).WillByDefault(Return(StubSamlToken()));
    ON_CALL(*provider, UpdateToken()).WillByDefault(Return(CreateCompletedAsyncTask<ISecurityTokenPtr>(StubSamlToken())));

    bool legacyMode = false;
    auto authHandler = ConnectAuthenticationHandler::Create("http://test.com", provider, GetHandlerPtr(), "token");

    AuthenticationHandler::Attempt attempt("http://test.com", "token SomeTestToken", DateTime(), 1);
    auto result = authHandler->_RetrieveAuthorization(attempt)->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_LegacyHandlerWithLegacyModeAttemptedOnceWithTokenAuth_RetrievesNewTokenAndRetries)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    auto authHandler = ConnectAuthenticationHandler::CreateLegacy("http://test.com", provider, GetHandlerPtr(), false);

    SamlTokenPtr newToken = StubSamlToken(100);

    EXPECT_CALL(*provider, GetToken()).WillOnce(Return(nullptr));
    EXPECT_CALL(*provider, UpdateToken()).WillOnce(Return(CreateCompletedAsyncTask<ISecurityTokenPtr>(newToken)));

    AuthenticationHandler::Attempt attempt("http://test.com", "token SomeTestToken", DateTime(), 1);
    auto result = authHandler->_RetrieveAuthorization(attempt)->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ("token " + newToken->ToAuthorizationString(), result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_LegacyHandlerWithLegacyModeAttemptedTwiceWithTokenAuth_ReturnsErrorToStopAuthentication)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ON_CALL(*provider, GetToken()).WillByDefault(Return(StubSamlToken()));
    ON_CALL(*provider, UpdateToken()).WillByDefault(Return(CreateCompletedAsyncTask<ISecurityTokenPtr>(StubSamlToken())));

    auto authHandler = ConnectAuthenticationHandler::CreateLegacy("http://test.com", provider, GetHandlerPtr(), false);

    AuthenticationHandler::Attempt attempt("http://test.com", "token SomeTestToken", DateTime(), 2);
    auto result = authHandler->_RetrieveAuthorization(attempt)->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_LegacyHandlerWithNonLegacyModeAttemptedOnceWithSamlAuth_ReturnsErrorToStopAuthentication)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ON_CALL(*provider, GetToken()).WillByDefault(Return(StubSamlToken()));
    ON_CALL(*provider, UpdateToken()).WillByDefault(Return(CreateCompletedAsyncTask<ISecurityTokenPtr>(StubSamlToken())));

    bool legacyMode = false;
    auto authHandler = ConnectAuthenticationHandler::Create("http://test.com", provider, GetHandlerPtr(), "SAML");

    AuthenticationHandler::Attempt attempt("http://test.com", "SAML SomeTestToken", DateTime(), 1);
    auto result = authHandler->_RetrieveAuthorization(attempt)->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_LegacyHandlerWithLegacyModeAttemptedOnceWithSamlAuth_RetrievesNewTokenAndRetries)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    auto authHandler = ConnectAuthenticationHandler::CreateLegacy("http://test.com", provider, GetHandlerPtr(), true);

    SamlTokenPtr newToken = StubSamlToken(100);

    EXPECT_CALL(*provider, GetToken()).WillOnce(Return(nullptr));
    EXPECT_CALL(*provider, UpdateToken()).WillOnce(Return(CreateCompletedAsyncTask<ISecurityTokenPtr>(newToken)));

    AuthenticationHandler::Attempt attempt("http://test.com", "SAML SomeTestToken", DateTime(), 1);
    auto result = authHandler->_RetrieveAuthorization(attempt)->GetResult();

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ("SAML " + newToken->ToAuthorizationString(), result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationHandlerTests, _RetrieveAuthorization_LegacyHandlerWithLegacyModeAttemptedTwiceWithSamlAuth_ReturnsErrorToStopAuthentication)
    {
    auto provider = std::make_shared<MockConnectTokenProvider>();
    ON_CALL(*provider, GetToken()).WillByDefault(Return(StubSamlToken()));
    ON_CALL(*provider, UpdateToken()).WillByDefault(Return(CreateCompletedAsyncTask<ISecurityTokenPtr>(StubSamlToken())));

    auto authHandler = ConnectAuthenticationHandler::CreateLegacy("http://test.com", provider, GetHandlerPtr(), true);

    AuthenticationHandler::Attempt attempt("http://test.com", "SAML SomeTestToken", DateTime(), 2);
    auto result = authHandler->_RetrieveAuthorization(attempt)->GetResult();

    EXPECT_FALSE(result.IsSuccess());
    }
#endif
