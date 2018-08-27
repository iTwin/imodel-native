/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/HttpProxyTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WebTestsHelper.h"

#include <BeHttp/HttpProxy.h>

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

struct HttpProxyTests : public ::testing::Test{};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HttpProxyTests, Ctor_Default_FieldsEmpty)
    {
    HttpProxy proxy;
    EXPECT_FALSE(proxy.IsValid());
    EXPECT_STREQ("", proxy.GetProxyUrl().c_str());
    EXPECT_FALSE(proxy.GetCredentials().IsValid());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpProxyTests, Ctor_UrlUsed_UrlSet)
    {
    HttpProxy proxy("foo://bar");
    EXPECT_TRUE(proxy.IsValid());
    EXPECT_STREQ("foo://bar", proxy.GetProxyUrl().c_str());
    EXPECT_FALSE(proxy.GetCredentials().IsValid());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpProxyTests, Ctor_UrlUsedAndCredentials_UrlAndCredentialsSet)
    {
    HttpProxy proxy("foo://bar", {"a", "b"});
    EXPECT_TRUE(proxy.IsValid());
    EXPECT_STREQ("foo://bar", proxy.GetProxyUrl().c_str());
    EXPECT_EQ(Credentials("a", "b"), proxy.GetCredentials());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpProxyTests, Ctor_UrlWithSpecialSymbols_SymbolsAreEncoded)
    {
    HttpProxy proxy("foo://bar", {"a", "b"});
    EXPECT_TRUE(proxy.IsValid());
    EXPECT_STREQ("foo://bar", proxy.GetProxyUrl().c_str());
    EXPECT_EQ(Credentials("a", "b"), proxy.GetCredentials());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpProxyTests, Ctor_UrlWithUnsafeCharacters_CharactersAreEscaped)
    {
    HttpProxy proxy(TEST_URL_UNSAFE_CHARS);
    EXPECT_STREQ(TEST_URL_UNSAFE_CHARS_ESCAPED, proxy.GetProxyUrl().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpProxyTests, SetProxyUrl_UrlWithUnsafeCharacters_CharactersAreEscaped)
    {
    HttpProxy proxy;
    proxy.SetProxyUrl(TEST_URL_UNSAFE_CHARS);
    EXPECT_STREQ(TEST_URL_UNSAFE_CHARS_ESCAPED, proxy.GetProxyUrl().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpProxyTests, SetProxyServer_EmptyHostnameAndPort_EmptyUrlSet)
    {
    HttpProxy proxy("foo");
    proxy.SetProxyServer("", 9999);
    EXPECT_STREQ("", proxy.GetProxyUrl().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpProxyTests, SetProxyServer_ValidHostnameAndPort_HttpUrlWithPortSet)
    {
    HttpProxy proxy;
    proxy.SetProxyServer("foo.bar.com", 9999);
    EXPECT_STREQ("http://foo.bar.com:9999", proxy.GetProxyUrl().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpProxyTests, SetProxyServer_HostWithUpperCaseCharactersAndPort_CharactersAreLowerCaseAndHttpUrlSet)
    {
    HttpProxy proxy;
    proxy.SetProxyServer("HOST", 9999);
    EXPECT_STREQ("http://host:9999", proxy.GetProxyUrl().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpProxyTests, SetProxyUrl_HostWithUnsafeCharactersAndPort_CharactersAreEscapedAndHttpUrlSet)
    {
    HttpProxy proxy;
    proxy.SetProxyUrl(TEST_URL_UNSAFE_CHARS);
    EXPECT_STREQ(TEST_URL_UNSAFE_CHARS_ESCAPED, proxy.GetProxyUrl().c_str());
    }
