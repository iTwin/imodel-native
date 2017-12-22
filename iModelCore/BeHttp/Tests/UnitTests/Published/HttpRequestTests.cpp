/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/HttpRequestTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WebTestsHelper.h"

#include <BeHttp/HttpProxy.h>

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

struct HttpRequestTests : public ::testing::Test{};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Ctor_UrlWithArbitraryText_UrlIsSetToSameString)
    {
    Request request("test url");
    EXPECT_STREQ("test url", request.GetUrl().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Ctor_UrlWithUnsafeCharacters_CharactersAreEscaped)
    {
    Request request(TEST_URL_UNSAFE_CHARS);
    EXPECT_STREQ(TEST_URL_UNSAFE_CHARS_ESCAPED, request.GetUrl().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, SetProxy_ProxyUrlWithUnsafeCharacters_CharactersAreEscaped)
    {
    Request request("foo");
    request.SetProxy(TEST_URL_UNSAFE_CHARS);
    EXPECT_STREQ(TEST_URL_UNSAFE_CHARS_ESCAPED, request.GetProxy().c_str());
    }
