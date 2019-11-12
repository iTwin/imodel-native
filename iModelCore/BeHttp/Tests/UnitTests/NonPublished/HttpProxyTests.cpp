/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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
    
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                 Robert.Lukasonok    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestHttpProxy : HttpProxy
    {
    TestHttpProxy(bvector<Utf8String> proxyBypassList) : HttpProxy("https://testproxyurl")
        {
        m_proxyBypassHosts = std::move(proxyBypassList);
        }
    };

/*--------------------------------------------------------------------------------------+
* @bsitest                                                  Robert.Lukasonok    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpProxyTests, GetProxiesForUrl_EmptyBypassList_ReturnsProxy)
    {
    auto httpProxy = TestHttpProxy(bvector<Utf8String>());
    bvector<HttpProxy> proxiesOut;
    ASSERT_EQ(SUCCESS, httpProxy.GetProxiesForUrl("foo://bar", proxiesOut));
    ASSERT_EQ(1, proxiesOut.size());
    EXPECT_STREQ("https://testproxyurl", proxiesOut[0].GetProxyUrl().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                  Robert.Lukasonok    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpProxyTests, GetProxiesForUrl_InvalidUri_ReturnsProxy)
    {
    auto httpProxy = TestHttpProxy({"foo"});
    bvector<HttpProxy> proxiesOut;
    ASSERT_EQ(SUCCESS, httpProxy.GetProxiesForUrl("//invalid:", proxiesOut));
    ASSERT_EQ(1, proxiesOut.size());
    EXPECT_STREQ("https://testproxyurl", proxiesOut[0].GetProxyUrl().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                  Robert.Lukasonok    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpProxyTests, GetProxiesForUrl_MultiplePatterns_FindsMatchAndBypassesProxy)
    {
    auto httpProxy = TestHttpProxy({"first", "second", "third"});
    bvector<HttpProxy> proxiesOut;
    ASSERT_EQ(SUCCESS, httpProxy.GetProxiesForUrl("foo://second", proxiesOut));
    EXPECT_EQ(0, proxiesOut.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                  Robert.Lukasonok    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpProxyTests, GetProxiesForUrl_MatchingUrisForValidProxyPatterns_ProxyBypassed)
    {
    auto testCaseList =
        {
        // Match host with any scheme
        std::make_tuple("a", "foo://a"),
        std::make_tuple("*", "foo://a"),
        std::make_tuple("**", "foo://a"),
        std::make_tuple("*b", "foo://b"),
        std::make_tuple("*b", "foo://aaabbb"),
        std::make_tuple("a*", "foo://a"),
        std::make_tuple("a*", "foo://aaab"),
        std::make_tuple("*b*", "foo://b"),
        std::make_tuple("*b*", "foo://aaabbbccc"),
        std::make_tuple("a*c", "foo://ac"),
        std::make_tuple("a**c", "foo://ac"),
        std::make_tuple("a*c", "foo://aaabbbccc"),
        std::make_tuple("a*c*e", "foo://ace"),
        std::make_tuple("a*c*e", "foo://abcde"),
        std::make_tuple("a**c**e*", "foo://abcde"),
        // Match host with specific scheme
        std::make_tuple("foo://a", "foo://a"),
        std::make_tuple("foo://*", "foo://a"),
        std::make_tuple("foo://**", "foo://a"),
        // Match host with port
        std::make_tuple("a:1234", "foo://a:1234"),
        std::make_tuple("a*:1234", "foo://a:1234"),
        std::make_tuple("a*:1234", "foo://aaabbb:1234"),
        std::make_tuple("*a:1234", "foo://a:1234"),
        std::make_tuple("*b:1234", "foo://aaabbb:1234"),
        // Match host with specific port adn scheme
        std::make_tuple("foo://a:1234", "foo://a:1234"),
        // Match authority with any scheme
        std::make_tuple("a@b", "foo://a@b"),
        std::make_tuple("*a@b", "foo://a@b"),
        std::make_tuple("*a@b", "foo://aaa@b"),
        std::make_tuple("*a**@b*", "foo://aaa@bbb"),
        // Match everything
        std::make_tuple("foo://**a**@**b***:1234", "foo://a@b:1234"),
        };

    for (const auto& testCase : testCaseList)
        {
        Utf8String proxyUrl, requestUrl;
        std::tie(proxyUrl, requestUrl) = testCase;

        auto httpProxy = TestHttpProxy({proxyUrl});
        bvector<HttpProxy> proxiesOut;
        ASSERT_EQ(SUCCESS, httpProxy.GetProxiesForUrl(requestUrl, proxiesOut));
        EXPECT_EQ(0, proxiesOut.size());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                  Robert.Lukasonok    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpProxyTests, GetProxiesForUrl_NonmatchingUrisForValidProxyPatterns_ReturnsProxy)
    {
    auto testCaseList =
        {
        std::make_tuple("a", "foo://b"),
        std::make_tuple("abc", "foo://ab"),
        std::make_tuple("abc", "foo://bc"),
        std::make_tuple("*b", "foo://bc"),
        std::make_tuple("a*", "foo://ba"),
        std::make_tuple("*b*", "foo://a"),
        std::make_tuple("a*c", "foo://a"),
        std::make_tuple("a*c", "foo://c"),
        std::make_tuple("a*c", "foo://abcd"),
        std::make_tuple("foo://a", "bar://a"),
        std::make_tuple("foo://*", "bar://a"),
        std::make_tuple("a:1234", "foo://a"),
        std::make_tuple("a:1234", "foo://a:6789"),
        std::make_tuple("a:1234", "foo://a:6789"),
        std::make_tuple("*://a", "foo://a"),
        std::make_tuple("http*://a", "https://a"),
        std::make_tuple("a@b", "foo://b"),
        std::make_tuple("*a@b", "foo://b@b"),
        std::make_tuple("*a@b", "foo://aaab@b"),
        };

    for (const auto& testCase : testCaseList)
        {
        Utf8String proxyUrl, requestUrl;
        std::tie(proxyUrl, requestUrl) = testCase;

        auto httpProxy = TestHttpProxy({proxyUrl});
        bvector<HttpProxy> proxiesOut;
        ASSERT_EQ(SUCCESS, httpProxy.GetProxiesForUrl(requestUrl, proxiesOut));
        ASSERT_EQ(1, proxiesOut.size());
        EXPECT_STREQ("https://testproxyurl", proxiesOut[0].GetProxyUrl().c_str());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                  Robert.Lukasonok    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpProxyTests, GetProxiesForUrl_WorstCasePatterns_CompletesWithoutStackOverflow)
    {
    // Proxy server exception list is limited to 2064 characters.
    // https://docs.microsoft.com/en-us/internet-explorer/ie11-ieak/proxy-settings-ieak11-wizard
    const size_t MAX_EXCEPTION_LIST_LENGTH = 2064;

    Utf8String requestUrl = "foo://" + Utf8String(MAX_EXCEPTION_LIST_LENGTH, 'a');

    Utf8String wildcardsOnly(MAX_EXCEPTION_LIST_LENGTH, '*');
    Utf8String alternatingWildcards;
    while (alternatingWildcards.length() < MAX_EXCEPTION_LIST_LENGTH)
        alternatingWildcards += "*a";

    bvector<Utf8String> testCaseList =
        {
        wildcardsOnly, // Optimised case
        alternatingWildcards // Absolute worst case
        };

    for (const auto& pattern : testCaseList)
        {
        auto httpProxy = TestHttpProxy({pattern});
        bvector<HttpProxy> proxiesOut;
        ASSERT_EQ(SUCCESS, httpProxy.GetProxiesForUrl(requestUrl, proxiesOut));
        EXPECT_EQ(0, proxiesOut.size());
        }
    }

