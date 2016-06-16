/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/HttpClientTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "HttpClientTests.h"

#include <BeHttp/HttpClient.h>

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS
TEST_F (HttpClientTests, CreateRequest_UrlAndMethodPassed_SetsUrlAndMethod)
    {
    HttpClient client;

    auto request = client.CreateRequest ("https://foo", "BOO");
    EXPECT_EQ ("https://foo", request.GetUrl ());
    EXPECT_EQ ("BOO", request.GetMethod ());
    }

TEST_F (HttpClientTests, CreateRequest_NoHeaderProvider_EmptyHeaders)
    {
    HttpClient client (nullptr, nullptr);

    auto request = client.CreateRequest ("https://foo", "BOO");
    EXPECT_EQ (0, request.GetHeaders ().GetMap ().size ());
    }

TEST_F (HttpClientTests, CreateRequest_DefaultHeaderProvider_EmptyHeaders)
    {
    HttpClient client (HttpHeaderProvider::Create (), nullptr);

    auto request = client.CreateRequest ("https://foo", "BOO");
    EXPECT_EQ (0, request.GetHeaders ().GetMap ().size ());
    }

TEST_F (HttpClientTests, CreateRequest_HeaderProviderWithHeadersSet_HeadersSetToRequest)
    {
    HttpRequestHeaders headers;
    headers.AddValue ("TestHeader", "TestValue");

    HttpClient client (HttpHeaderProvider::Create (headers), nullptr);

    auto request = client.CreateRequest ("https://foo", "BOO");
    EXPECT_EQ (1, request.GetHeaders ().GetMap ().size ());
    EXPECT_STREQ ("TestValue", request.GetHeaders ().GetValue ("TestHeader"));
    }
