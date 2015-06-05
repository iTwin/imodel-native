/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Connect/ConnectAuthenticationHandlerTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConnectAuthenticationHandlerTests.h"
#include <WebServices/Connect/ConnectAuthenticationHandler.h>
#include <WebServices/Connect/Connect.h>
#include <Bentley/Base64Utilities.h>
#include "MockConnectTokenProvider.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace ::testing;

#ifdef USE_GTEST
AuthenticationHandler::Attempt StubAttempt (Utf8StringCR url)
    {
    return AuthenticationHandler::Attempt (url, "", DateTime::GetCurrentTimeUtc (), 0);
    }

TEST_F (ConnectAuthenticationHandlerTests, _ShouldRetryAuthentication_HttpAuthErrors_True)
    {
    auto provider = std::make_shared<MockConnectTokenProvider> ();
    ConnectAuthenticationHandler authHandler ("Foo", provider, GetHandlerPtr ());

    EXPECT_TRUE (authHandler._ShouldRetryAuthentication (StubHttpResponse (HttpStatus::Unauthorized)));
    EXPECT_TRUE (authHandler._ShouldRetryAuthentication (StubHttpResponse (HttpStatus::Forbidden)));
    }

TEST_F (ConnectAuthenticationHandlerTests, _ShouldRetryAuthentication_HttpNonAuthErrors_False)
    {
    auto provider = std::make_shared<MockConnectTokenProvider> ();
    ConnectAuthenticationHandler authHandler ("Foo", provider, GetHandlerPtr ());

    EXPECT_FALSE (authHandler._ShouldRetryAuthentication (StubHttpResponse (HttpStatus::OK)));
    EXPECT_FALSE (authHandler._ShouldRetryAuthentication (StubHttpResponse (HttpStatus::NotFound)));
    EXPECT_FALSE (authHandler._ShouldRetryAuthentication (StubHttpResponse (ConnectionStatus::CouldNotConnect)));
    }

TEST_F (ConnectAuthenticationHandlerTests, _ShouldRetryAuthentication_DatasourceNotFoundError_True)
    {
    auto provider = std::make_shared<MockConnectTokenProvider> ();
    ConnectAuthenticationHandler authHandler ("Foo", provider, GetHandlerPtr ());

    auto response = StubHttpResponse (HttpStatus::NotFound, R"({"errorId":"DatasourceNotFound"})");
    EXPECT_TRUE (authHandler._ShouldRetryAuthentication (response));
    }

TEST_F (ConnectAuthenticationHandlerTests, _RetrieveAuthorization_DifferentBaseUrl_ReturnsEmpty)
    {
    auto provider = std::make_shared<MockConnectTokenProvider> ();
    ConnectAuthenticationHandler authHandler ("http://other.com", provider, GetHandlerPtr ());

    auto result = authHandler._RetrieveAuthorization (StubAttempt ("http://test.com/foo"))->GetResult ();

    EXPECT_EQ ("", result.GetValue ());
    }

TEST_F (ConnectAuthenticationHandlerTests, _RetrieveAuthorization_SameBaseUrl_ReturnsExistingToken)
    {
    auto provider = std::make_shared<MockConnectTokenProvider> ();
    ConnectAuthenticationHandler authHandler ("http://test.com", provider, GetHandlerPtr ());

    SamlTokenPtr token = StubSamlToken (100);
    EXPECT_CALL (*provider, GetToken ()).WillRepeatedly (Return (token));

    auto result = authHandler._RetrieveAuthorization (StubAttempt ("http://test.com/foo"))->GetResult ();

    EXPECT_EQ (token->ToAuthorizationString (), result.GetValue ());
    }

TEST_F (ConnectAuthenticationHandlerTests, _RetrieveAuthorization_PersistedToken_ReturnsToken)
    {
    auto provider = std::make_shared<MockConnectTokenProvider> ();
    ConnectAuthenticationHandler authHandler ("http://test.com", provider, GetHandlerPtr ());

    SamlTokenPtr token = StubSamlToken (100);
    EXPECT_CALL (*provider, GetToken ()).WillRepeatedly (Return (token));

    auto result = authHandler._RetrieveAuthorization (StubAttempt ("http://test.com"))->GetResult ();

    EXPECT_EQ (token->ToAuthorizationString (), result.GetValue ());
    }

TEST_F (ConnectAuthenticationHandlerTests, _RetrieveAuthorization_NoCachedTokenAndUpdateTokenFails_ReturnsError)
    {
    auto provider = std::make_shared<MockConnectTokenProvider> ();
    ConnectAuthenticationHandler authHandler ("http://test.com", provider, GetHandlerPtr ());

    EXPECT_CALL (*provider, GetToken ()).WillOnce (Return (nullptr));
    EXPECT_CALL (*provider, UpdateToken ()).WillOnce (Return (nullptr));

    auto result = authHandler._RetrieveAuthorization (StubAttempt ("http://test.com"))->GetResult ();

    EXPECT_FALSE (result.IsSuccess ());
    }

TEST_F (ConnectAuthenticationHandlerTests, _RetrieveAuthorization_TokenIsNotPersisted_UpdatesAndReturnsToken)
    {
    auto provider = std::make_shared<MockConnectTokenProvider> ();
    ConnectAuthenticationHandler authHandler ("http://test.com", provider, GetHandlerPtr ());

    SamlTokenPtr newToken = StubSamlToken (100);

    EXPECT_CALL (*provider, GetToken ()).WillOnce (Return (nullptr));
    EXPECT_CALL (*provider, UpdateToken ()).WillOnce (Return (newToken));

    auto result = authHandler._RetrieveAuthorization (StubAttempt ("http://test.com"))->GetResult ();

    EXPECT_EQ (newToken->ToAuthorizationString (), result.GetValue ());
    }

TEST_F (ConnectAuthenticationHandlerTests, _RetrieveAuthorization_AttemptedOnceWithTokenAuth_RetrievesnewTokenAndRetries)
    {
    auto provider = std::make_shared<MockConnectTokenProvider> ();
    ConnectAuthenticationHandler authHandler ("http://test.com", provider, GetHandlerPtr ());

    SamlTokenPtr newToken = StubSamlToken (100);

    EXPECT_CALL (*provider, GetToken ()).WillOnce (Return (nullptr));
    EXPECT_CALL (*provider, UpdateToken ()).WillOnce (Return (newToken));

    AuthenticationHandler::Attempt attempt ("http://test.com", "token SomeTestToken", DateTime (), 1);
    auto result = authHandler._RetrieveAuthorization (attempt)->GetResult ();

    EXPECT_TRUE (result.IsSuccess ());
    EXPECT_EQ (newToken->ToAuthorizationString (), result.GetValue ());
    }

TEST_F (ConnectAuthenticationHandlerTests, _RetrieveAuthorization_AttemptedTwiceWithTokenAuth_ReturnsError)
    {
    auto provider = std::make_shared<MockConnectTokenProvider> ();
    ConnectAuthenticationHandler authHandler ("http://test.com", provider, GetHandlerPtr ());

    AuthenticationHandler::Attempt attempt ("http://test.com", "token SomeTestToken", DateTime (), 2);
    auto result = authHandler._RetrieveAuthorization (attempt)->GetResult ();

    EXPECT_FALSE (result.IsSuccess ());
    }

TEST_F (ConnectAuthenticationHandlerTests, _RetrieveAuthorization_AttemptedTwiceAndIsNotTokenAuth_UsesExistingToken)
    {
    auto provider = std::make_shared<MockConnectTokenProvider> ();
    ConnectAuthenticationHandler authHandler ("http://test.com", provider, GetHandlerPtr ());

    SamlTokenPtr token = StubSamlToken (100);
    EXPECT_CALL (*provider, GetToken ()).WillRepeatedly (Return (token));

    AuthenticationHandler::Attempt attempt ("http://test.com", "TestOtherAuth", DateTime::GetCurrentTimeUtc (), 1);
    auto result = authHandler._RetrieveAuthorization (attempt)->GetResult ();

    EXPECT_TRUE (result.IsSuccess ());
    EXPECT_EQ (token->ToAuthorizationString (), result.GetValue ());
    }
#endif