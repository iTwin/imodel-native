/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/HttpClientTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WebTestsHelper.h"

#include <BeHttp/HttpClient.h>

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

struct HttpClientTests : public ::testing::Test{};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HttpClientTests, CreateRequest_UrlAndMethodPassed_SetsUrlAndMethod)
    {
    HttpClient client;

    auto request = client.CreateRequest ("https://foo", "BOO");
    EXPECT_EQ ("https://foo", request.GetUrl ());
    EXPECT_EQ ("BOO", request.GetMethod ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HttpClientTests, CreateRequest_NoHeaderProvider_EmptyHeaders)
    {
    HttpClient client (nullptr, nullptr);

    auto request = client.CreateRequest ("https://foo", "BOO");
    EXPECT_EQ (0, request.GetHeaders ().GetMap ().size ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HttpClientTests, CreateRequest_DefaultHeaderProvider_EmptyHeaders)
    {
    HttpClient client (HttpHeaderProvider::Create (), nullptr);

    auto request = client.CreateRequest ("https://foo", "BOO");
    EXPECT_EQ (0, request.GetHeaders ().GetMap ().size ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HttpClientTests, CreateRequest_HeaderProviderWithHeadersSet_HeadersSetToRequest)
    {
    HttpRequestHeaders headers;
    headers.AddValue ("TestHeader", "TestValue");

    HttpClient client (HttpHeaderProvider::Create (headers), nullptr);

    auto request = client.CreateRequest ("https://foo", "BOO");
    EXPECT_EQ (1, request.GetHeaders ().GetMap ().size ());
    EXPECT_STREQ ("TestValue", request.GetHeaders ().GetValue ("TestHeader"));
    }
