/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "WebTestsHelper.h"

#include <BeHttp/ProxyHttpHandler.h>

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

struct ProxyHttpHandlerTests : public ::testing::Test{};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProxyHttpHandlerTests, PerformRequest_HandlerWithProxyUrl_RequestWithProxyUrl)
    {
    auto mockHandler = std::make_shared<MockHttpHandler> ();
    ProxyHttpHandler proxyHandler ("http://proxy.com", mockHandler);

    mockHandler->ExpectOneRequest ().ForAnyRequest ([=] (Http::RequestCR request)
        {
        EXPECT_EQ ("http://test.com", request.GetUrl ());
        EXPECT_EQ ("http://proxy.com", request.GetProxy ());
        return StubHttpResponse ();
        });

    auto request = StubHttpGetRequest ("http://test.com");
    proxyHandler._PerformRequest (request)->Wait ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProxyHttpHandlerTests, PerformRequest_HandlerWithoutProxyUrl_RequestWithoutProxyUrl)
    {
    auto mockHandler = std::make_shared<MockHttpHandler> ();
    ProxyHttpHandler proxyHandler ("", mockHandler);

    mockHandler->ExpectOneRequest ().ForAnyRequest ([=] (Http::RequestCR request)
        {
        EXPECT_EQ ("http://test.com", request.GetUrl ());
        EXPECT_TRUE (request.GetProxy ().empty ());
        return StubHttpResponse ();
        });

    auto request = StubHttpGetRequest ("http://test.com");
    proxyHandler._PerformRequest (request)->Wait ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProxyHttpHandlerTests, PerformRequest_HandlerWithProxyUrl_RequestWithProxyUrlAndCredentials)
    {
    Utf8String proxyUrl ("http://proxy.com");
    Utf8String proxyUsername ("proxyuser");
    Utf8String proxyPassword ("drowssap");

    auto mockHandler = std::make_shared<MockHttpHandler> ();
    mockHandler->ExpectOneRequest ().ForAnyRequest ([&] (Http::RequestCR request)
        {
        // Verify that the request targets the proxy's URL, including its credentials.
        EXPECT_EQ (proxyUrl, request.GetProxy ());
        EXPECT_EQ (proxyUsername, request.GetProxyCredentials ().GetUsername ());
        EXPECT_EQ (proxyPassword, request.GetProxyCredentials ().GetPassword ());
        return StubHttpResponse (ConnectionStatus::OK);
        });

    ProxyHttpHandler proxy (proxyUrl, mockHandler);
    proxy.SetProxyCredentials (Credentials (proxyUsername, proxyPassword));

    EXPECT_EQ (proxyUrl, proxy.GetProxyUrl ());
    EXPECT_EQ (proxyUsername, proxy.GetProxyCredentials ().GetUsername ());
    EXPECT_EQ (proxyPassword, proxy.GetProxyCredentials ().GetPassword ());

    auto request = StubHttpGetRequest ("http://test.com");
    proxy._PerformRequest (request)->Wait ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ProxyHttpHandlerTests, PerformRequest_ResponseWithNoneStatus_ReturnsHandlerThatDoesNotSetProxy)
    {
    auto mockHandler = std::make_shared<MockHttpHandler> ();

    mockHandler->ExpectRequests(2).ForRequest(1, [=] (Http::RequestCR request)
        {
        return StubHttpResponse (ConnectionStatus::None);
        });
    auto proxy = ProxyHttpHandler::GetProxyIfReachable ("http://test.com", mockHandler);

    mockHandler->ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ("", request.GetProxy());
        return StubHttpResponse();
        });
    proxy->_PerformRequest(Http::Request("http://foo"))->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProxyHttpHandlerTests, PerformRequest_ResponseWithOKStatus_ReturnsHandlerThatDoesNotSetProxy)
    {
    auto mockHandler = std::make_shared<MockHttpHandler> ();

    mockHandler->ExpectRequests(2).ForRequest (1, [=] (Http::RequestCR request)
        {
        return StubHttpResponse (ConnectionStatus::OK);
        });
    auto proxy = ProxyHttpHandler::GetProxyIfReachable ("http://test.com", mockHandler);

    mockHandler->ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ("http://test.com", request.GetProxy());
        return StubHttpResponse();
        });
    proxy->_PerformRequest(Http::Request("http://foo"))->Wait();
    }
