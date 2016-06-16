/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ProxyHttpHandlerTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <BeHttp/ProxyHttpHandler.h>
#include "ProxyHttpHandlerTests.h"
#include "WebTestsHelper.h"
USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

TEST_F (ProxyHttpHandlerTests, PerformRequest_HandlerWithProxyUrl_RequestWithProxyUrl)
    {
    auto mockHandler = std::make_shared<MockHttpHandler> ();
    ProxyHttpHandler proxyHandler ("http://proxy.com", mockHandler);

    mockHandler->ExpectOneRequest ().ForAnyRequest ([=] (HttpRequestCR request)
        {
        EXPECT_EQ ("http://test.com", request.GetUrl ());
        EXPECT_EQ ("http://proxy.com", request.GetProxy ());
        return StubHttpResponse ();
        });

    auto request = StubHttpGetRequest ("http://test.com");
    proxyHandler.PerformRequest (request)->Wait ();
    }

TEST_F (ProxyHttpHandlerTests, PerformRequest_HandlerWithoutProxyUrl_RequestWithoutProxyUrl)
    {
    auto mockHandler = std::make_shared<MockHttpHandler> ();
    ProxyHttpHandler proxyHandler ("", mockHandler);

    mockHandler->ExpectOneRequest ().ForAnyRequest ([=] (HttpRequestCR request)
        {
        EXPECT_EQ ("http://test.com", request.GetUrl ());
        EXPECT_TRUE (request.GetProxy ().empty ());
        return StubHttpResponse ();
        });

    auto request = StubHttpGetRequest ("http://test.com");
    proxyHandler.PerformRequest (request)->Wait ();
    }

TEST_F (ProxyHttpHandlerTests, PerformRequest_HandlerWithProxyUrl_RequestWithProxyUrlAndCredentials)
    {
    Utf8String proxyUrl ("http://proxy.com");
    Utf8String proxyUsername ("proxyuser");
    Utf8String proxyPassword ("drowssap");

    auto mockHandler = std::make_shared<MockHttpHandler> ();
    mockHandler->ExpectOneRequest ().ForAnyRequest ([&] (HttpRequestCR request)
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
    proxy.PerformRequest (request)->Wait ();
    }

TEST_F (ProxyHttpHandlerTests, PerformRequest_ResponseWithNoneStatus_HandlerWithoutProxyUrl)
    {
    auto mockHandler = std::make_shared<MockHttpHandler> ();

    mockHandler->ExpectOneRequest ().ForAnyRequest ([=] (HttpRequestCR request)
        {
        return StubHttpResponse (ConnectionStatus::None);
        });

    auto proxy = ProxyHttpHandler::GetProxyIfReachable ("http://test.com", mockHandler);
    EXPECT_TRUE (proxy->GetProxyUrl ().empty ());
    }

TEST_F (ProxyHttpHandlerTests, PerformRequest_ResponseWithOKStatus_HandlerWithProxyUrl)
    {
    auto mockHandler = std::make_shared<MockHttpHandler> ();

    mockHandler->ExpectOneRequest ().ForAnyRequest ([=] (HttpRequestCR request)
        {
        return StubHttpResponse (ConnectionStatus::OK);
        });

    auto proxy = ProxyHttpHandler::GetProxyIfReachable ("http://test.com", mockHandler);
    EXPECT_EQ ("http://test.com", proxy->GetProxyUrl ());
    }
