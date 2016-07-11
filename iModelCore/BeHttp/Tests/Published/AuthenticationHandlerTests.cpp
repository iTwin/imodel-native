/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/AuthenticationHandlerTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "AuthenticationHandlerTests.h"

#ifdef USE_GTEST
#include <gmock/gmock.h>
#endif

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS
#ifdef USE_GTEST

bmap<Utf8String, Utf8String> GetHttpHeaders(Utf8String key, Utf8String value)
    {
    bmap<Utf8String, Utf8String> httpHeaders;
    httpHeaders[key] = value;
    return httpHeaders;
    }

TEST_F (AuthenticationHandlerTests, PerformRequest_AuthorisationHeaderHasValue_PassesRequestToSuppliedHandler)
    {
    MockAuthenticationHandler authHandler(GetHandlerPtr());

    GetHandler().ForFirstRequest([] (Http::RequestCR request)
        {
        EXPECT_EQ ("http://test.com", request.GetUrl());
        return StubHttpResponse();
        });
    authHandler._PerformRequest(StubHttpGetRequest("http://test.com", GetHttpHeaders("Authorization", "Foo")))->Wait();

    EXPECT_EQ (1, GetHandler().GetRequestsPerformed());
    }

TEST_F (AuthenticationHandlerTests, PerformRequest_AuthorisationHeaderHasNoValue_RetrievesAuthorizationAndPerformsRequest)
    {
    MockAuthenticationHandler authHandler(GetHandlerPtr());
    EXPECT_CALL (authHandler, _RetrieveAuthorization(_))
        .WillOnce(Invoke([&] (AuthenticationHandler::AttemptCR attempt)
        {
        EXPECT_EQ ("http://test.com", attempt.GetRequestUrl());
        return CreateCompletedAsyncTask(AuthenticationHandler::AuthorizationResult::Success("TestAuth"));
        }));

    GetHandler().ForFirstRequest(StubHttpResponse());
    authHandler._PerformRequest(StubHttpGetRequest("http://test.com", GetHttpHeaders("Authorization", "")))->Wait();

    EXPECT_EQ (1, GetHandler().GetRequestsPerformed());
    }
    
TEST_F (AuthenticationHandlerTests, PerformRequest_AuthorizationHeaderHasNoValueAndRetrieveAuthorizationReturnsError_TriesRequestWithoutCredentials)
    {
    MockAuthenticationHandler authHandler(GetHandlerPtr());
    EXPECT_CALL (authHandler, _RetrieveAuthorization(_))
        .WillOnce(Return(CreateCompletedAsyncTask(AuthenticationHandler::AuthorizationResult::Error(AsyncError("")))));

    GetHandler().ForFirstRequest([] (Http::RequestCR request)
        {
        EXPECT_EQ ("http://test.com", request.GetUrl());
        EXPECT_TRUE (Utf8String::IsNullOrEmpty(request.GetHeaders().GetAuthorization()));
        return StubHttpResponse();
        });
    authHandler._PerformRequest(StubHttpGetRequest("http://test.com", GetHttpHeaders("Authorization", "")))->Wait();

    EXPECT_EQ (1, GetHandler().GetRequestsPerformed());
    }

TEST_F (AuthenticationHandlerTests, PerformRequest_FirstRequestHasAuthorizationButFailsAndRetrieveAuthorizationReturnsError_ReturnsResponse)
    {
    MockAuthenticationHandler authHandler(GetHandlerPtr());

    GetHandler().ForFirstRequest(StubHttpResponse(HttpStatus::Unauthorized, "TestBody"));

    EXPECT_CALL (authHandler, _RetrieveAuthorization(_))
        .WillOnce(Return(CreateCompletedAsyncTask(AuthenticationHandler::AuthorizationResult::Error(AsyncError("")))));

    auto response = authHandler._PerformRequest(StubHttpGetRequest("http://test.com", GetHttpHeaders("Authorization", "Foo")))->GetResult();

    EXPECT_EQ (HttpStatus::Unauthorized, response.GetHttpStatus());
    EXPECT_EQ ("TestBody", response.GetBody().AsString());
    EXPECT_EQ (1, GetHandler().GetRequestsPerformed());
    }

TEST_F (AuthenticationHandlerTests, PerformRequest_ResponseUnauthorized_RetrievesAuthorizationAndRetriesRequest)
    {
    MockAuthenticationHandler authHandler(GetHandlerPtr());
    EXPECT_CALL (authHandler, _RetrieveAuthorization(_))
        .WillOnce(Return(CreateCompletedAsyncTask(AuthenticationHandler::AuthorizationResult::Success("TestAuth"))));

    GetHandler().ForRequest(1, StubHttpResponse(HttpStatus::Unauthorized));
    GetHandler().ForRequest(2, [] (Http::RequestCR request)
        {
        EXPECT_EQ ("http://test.com", request.GetUrl());
        EXPECT_STREQ ("TestAuth", request.GetHeaders().GetAuthorization());
        return StubHttpResponse();
        });

    authHandler._PerformRequest(StubHttpGetRequest("http://test.com", GetHttpHeaders("Authorization", "Foo")))->Wait();

    EXPECT_EQ (2, GetHandler().GetRequestsPerformed());
    }

TEST_F (AuthenticationHandlerTests, PerformRequest_ResponseNonLoginError_ReturnsResponseDirectly)
    {
    MockAuthenticationHandler authHandler(GetHandlerPtr());

    GetHandler().ForFirstRequest(StubHttpResponse(HttpStatus::BadRequest));

    auto response = authHandler._PerformRequest(StubHttpGetRequest("http://test.com", GetHttpHeaders("Authorization", "Foo")))->GetResult();

    EXPECT_EQ (HttpStatus::BadRequest, response.GetHttpStatus());
    EXPECT_EQ (1, GetHandler().GetRequestsPerformed());
    }

#endif