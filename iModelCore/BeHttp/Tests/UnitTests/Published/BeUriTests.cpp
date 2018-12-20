/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/BeUriTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WebTestsHelper.h"
#include <BeHttp/BeUri.h>

#include <tuple>

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

struct BeUriTests : public ::testing::Test {};

/*--------------------------------------------------------------------------------------+
* @bsitest                                      Vincas.Razma                    12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, Ctor_UrlWithUnsafeCharacters_CharactersAreEscaped)
    {
    BeUri uri(TEST_URL_UNSAFE_CHARS);
    EXPECT_STREQ(TEST_URL_UNSAFE_CHARS_ESCAPED, uri.ToString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                      Robert.Lukasonok                07/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, Ctor_ValidUriWithoutUnsafeSymbols_ExpectedComponentsAndOriginalString)
    {
    using TestCase = std::tuple<CharCP, CharCP, CharCP, CharCP, CharCP, int32_t, CharCP, CharCP, CharCP>;
    bvector<TestCase> testCases =
        {
        std::make_tuple("scheme://user:info@host:1234/path?query#fragment", "scheme", "user:info@host:1234", "user:info", "host", 1234, "/path", "query", "fragment"),
        std::make_tuple("scheme://www.host.com", "scheme", "www.host.com", "", "www.host.com", -1, "/", "", ""),
        std::make_tuple("c:/Windows/System32", "c", "", "", "", -1, "/Windows/System32", "", ""),
        std::make_tuple("s.c+e-m0e:", "s.c+e-m0e", "", "", "", -1, "/", "", ""),
        std::make_tuple("scheme:path", "scheme", "", "", "", -1, "path", "", ""),
        std::make_tuple("scheme:path/", "scheme", "", "", "", -1, "path/", "", ""),
        std::make_tuple("scheme:/path", "scheme", "", "", "", -1, "/path", "", ""),
        std::make_tuple("scheme:/path/", "scheme", "", "", "", -1, "/path/", "", ""),
        std::make_tuple("scheme:/", "scheme", "", "", "", -1, "/", "", ""), // No authority, path is separator
        std::make_tuple("scheme://", "scheme", "", "", "", -1, "/", "", ""), // Empty authority
        std::make_tuple("scheme:///", "scheme", "", "", "", -1, "/", "", ""), // Empty authority, path is separator
        std::make_tuple("scheme:////", "scheme", "", "", "", -1, "//", "", ""), // Empty authority, path is two seperators
        std::make_tuple("scheme:path:to:something", "scheme", "", "", "", -1, "path:to:something", "", ""),
        std::make_tuple("scheme://1.2.3.4:1234", "scheme", "1.2.3.4:1234", "", "1.2.3.4", 1234, "/", "", ""), // IPv4
        std::make_tuple("scheme://[::ffff:0.0.0.0]:1234", "scheme", "[::ffff:0.0.0.0]:1234", "", "[::ffff:0.0.0.0]", 1234, "/", "", ""), // IPv6
        std::make_tuple("scheme:pat?h://host", "scheme", "", "", "", -1, "pat", "h://host", ""),
        std::make_tuple("scheme:pat#h://host", "scheme", "", "", "", -1, "pat", "", "h://host"),
        std::make_tuple("scheme::", "scheme", "", "", "", -1, ":", "", ""),
        std::make_tuple("scheme:@", "scheme", "", "", "", -1, "@", "", ""),
        };

    for (const auto& testCase : testCases)
        {
        CharCP uriString, scheme, authority, userInfo, host, path, query, fragment;
        int32_t port;
        std::tie(uriString, scheme, authority, userInfo, host, port, path, query, fragment) = testCase;
        BeUri uri(uriString);

        EXPECT_STREQ(scheme, uri.GetScheme().c_str());
        EXPECT_STREQ(authority, uri.GetAuthority().c_str());
        EXPECT_STREQ(userInfo, uri.GetUserInfo().c_str());
        EXPECT_STREQ(host, uri.GetHost().c_str());
        EXPECT_EQ(port, uri.GetPort());
        EXPECT_STREQ(path, uri.GetPath().c_str());
        EXPECT_STREQ(query, uri.GetQuery().c_str());
        EXPECT_STREQ(fragment, uri.GetFragment().c_str());
        EXPECT_STREQ(uriString, uri.ToString().c_str());
        EXPECT_TRUE(uri.IsValid());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                      Robert.Lukasonok                07/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, Ctor_ValidUriWithUpperCaseSymbols_ExpectedComponentsAreLowerCase)
    {
    BeUri uri("sChEmE://uSeR:iNfO@hOsT:1234/pAtH?qUeRy#FrAgMeNt");

    EXPECT_STREQ("scheme", uri.GetScheme().c_str());
    EXPECT_STREQ("uSeR:iNfO@host:1234", uri.GetAuthority().c_str());
    EXPECT_STREQ("uSeR:iNfO", uri.GetUserInfo().c_str());
    EXPECT_STREQ("host", uri.GetHost().c_str());
    EXPECT_EQ(1234, uri.GetPort());
    EXPECT_STREQ("/pAtH", uri.GetPath().c_str());
    EXPECT_STREQ("qUeRy", uri.GetQuery().c_str());
    EXPECT_STREQ("FrAgMeNt", uri.GetFragment().c_str());
    EXPECT_STREQ("scheme://uSeR:iNfO@host:1234/pAtH?qUeRy#FrAgMeNt", uri.ToString().c_str());
    EXPECT_TRUE(uri.IsValid());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                      Robert.Lukasonok                07/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, Ctor_ValidUriWithUnsafeSymbols_ExpectedComponentsAndOriginalString)
    {
    using TestCase = std::tuple<CharCP, CharCP, CharCP, CharCP, CharCP, CharCP, int32_t, CharCP, CharCP, CharCP>;
    bvector<TestCase> testCases =
        {
        std::make_tuple("scheme://hos|t:42/pat|h", "scheme://hos|t:42/pat%7Ch", "scheme", "hos|t:42", "", "hos|t", 42, "/pat%7Ch", "", ""),
        std::make_tuple("scheme:[]", "scheme:%5B%5D", "scheme", "", "", "", -1, "%5B%5D", "", ""), // [] in path component
        std::make_tuple("scheme:[::1/64]", "scheme:%5B::1/64%5D", "scheme", "", "", "", -1, "%5B::1/64%5D", "", "") // [] in path component
        };

    for (const auto& testCase : testCases)
        {
        CharCP uriString, expectedUriString, scheme, authority, userInfo, host, path, query, fragment;
        int32_t port;
        std::tie(uriString, expectedUriString, scheme, authority, userInfo, host, port, path, query, fragment) = testCase;
        BeUri uri(uriString);

        EXPECT_STREQ(scheme, uri.GetScheme().c_str());
        EXPECT_STREQ(authority, uri.GetAuthority().c_str());
        EXPECT_STREQ(userInfo, uri.GetUserInfo().c_str());
        EXPECT_STREQ(host, uri.GetHost().c_str());
        EXPECT_EQ(port, uri.GetPort());
        EXPECT_STREQ(path, uri.GetPath().c_str());
        EXPECT_STREQ(query, uri.GetQuery().c_str());
        EXPECT_STREQ(fragment, uri.GetFragment().c_str());
        EXPECT_STREQ(expectedUriString, uri.ToString().c_str());
        EXPECT_TRUE(uri.IsValid());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                      Robert.Lukasonok                07/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, Ctor_InvalidUri_EmptyComponents)
    {
    // If a URI contains an authority component, then the path component
    // must either be empty or begin with a slash("/") character. [RFC 3986] section 3.2.

    bvector<BeUri> testCases =
        {
        BeUri(),
        BeUri(""),
        BeUri("path"),
        BeUri("/path"),
        BeUri(":path"),
        BeUri("?query"),
        BeUri("#fragment"),
        BeUri("//host"),
        BeUri("0scheme:/"),
        BeUri("scheme://host:"),
        BeUri("scheme://host:path"),
        BeUri("scheme://[]"),
        BeUri("scheme://[a][b]"),
        BeUri("scheme://[::1/64]")
        };


    for (const auto& uri : testCases)
        {
        EXPECT_STREQ("", uri.GetScheme().c_str());
        EXPECT_STREQ("", uri.GetAuthority().c_str());
        EXPECT_STREQ("", uri.GetUserInfo().c_str());
        EXPECT_STREQ("", uri.GetHost().c_str());
        EXPECT_EQ(-1, uri.GetPort());
        EXPECT_STREQ("", uri.GetPath().c_str());
        EXPECT_STREQ("", uri.GetQuery().c_str());
        EXPECT_STREQ("", uri.GetFragment().c_str());
        EXPECT_STREQ("", uri.ToString().c_str());
        EXPECT_FALSE(uri.IsValid());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                      Robert.Lukasonok                08/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, Ctor_LongUriComponents_SucceedsWithoutStackOverflow)
    {
    const size_t SCHEME_LENGTH = 128;
    const size_t USER_INFO_LENGTH = 256;
    const size_t HOST_LENGTH = 512;
    const size_t PATH_LENGTH = 2048;
    const size_t QUERY_LENGTH = 2048;
    const size_t FRAGMENT_LENGTH = 2048;

    Utf8String scheme(SCHEME_LENGTH, 's');
    Utf8String userInfo(USER_INFO_LENGTH, 'u');
    Utf8String host(HOST_LENGTH, 'h');
    Utf8String path(PATH_LENGTH, 'p');
    Utf8String query(QUERY_LENGTH, 'q');
    Utf8String fragment(FRAGMENT_LENGTH, 'f');

    Utf8String uriString = scheme + "://" + userInfo + "@" + host + ":12345/" + path + "?" + query + "#" + fragment;
    BeUri uri(uriString);
    EXPECT_TRUE(uri.IsValid());
    EXPECT_STREQ(uriString.c_str(), uri.ToString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                      Vincas.Razma                    12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, EscapeUnsafeCharactersInUrl_UrlWithUnsafeCharacters_CharactersAreEscaped)
    {
    Utf8String url = TEST_URL_UNSAFE_CHARS;
    EXPECT_STREQ(TEST_URL_UNSAFE_CHARS_ESCAPED, BeUri::EscapeUnsafeCharactersInUrl(url).c_str());
    EXPECT_STREQ(TEST_URL_UNSAFE_CHARS_ESCAPED, url.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                      Vincas.Razma                    12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, EscapeString_Strings_EscapedReturnedWhereNeeded)
    {
    EXPECT_STREQ("", BeUri::EscapeString("").c_str());
    EXPECT_STREQ("Foo%20Boo", BeUri::EscapeString("Foo Boo").c_str());
    EXPECT_STREQ("%2F-%2B%2A", BeUri::EscapeString("/-+*").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                      Vincas.Razma                    12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, UnescapeString_Strings_UnescapedReturned)
    {
    EXPECT_STREQ("", BeUri::UnescapeString("").c_str());
    EXPECT_STREQ("Foo Boo", BeUri::UnescapeString("Foo Boo").c_str());
    EXPECT_STREQ("Foo Boo", BeUri::UnescapeString("Foo%20Boo").c_str());
    EXPECT_STREQ("/-+*", BeUri::UnescapeString("/-+*").c_str());
    EXPECT_STREQ("/-+*", BeUri::UnescapeString("%2F-%2B%2A").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                      Robert.Lukasonok                08/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, Ctor_ValidPortNumberSpecified_ValidUri)
    {
#define EXPECT_VALID_PORT(port)              \
do {                                         \
BeUri uri(Utf8String("foo://bar:") + #port); \
EXPECT_TRUE(uri.IsValid());                  \
EXPECT_EQ(port, uri.GetPort());              \
} while(false)

    EXPECT_VALID_PORT(0);
    EXPECT_VALID_PORT(123);
    EXPECT_VALID_PORT(65535); // 2^16 - 1
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                      Robert.Lukasonok                08/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, Ctor_InvalidPortNumberSpecified_InValidUri)
    {
#define EXPECT_INVALID_PORT(port)           \
do {                                        \
BeUri uri(Utf8String("foo://bar:") + port); \
EXPECT_FALSE(uri.IsValid());                \
EXPECT_EQ(-1, uri.GetPort());               \
} while(false)

    BeTest::SetFailOnAssert(false);

    EXPECT_INVALID_PORT("-1");
    EXPECT_INVALID_PORT("65536"); // 2^16
    EXPECT_INVALID_PORT("4294967295"); // 2^32 - 1
    EXPECT_INVALID_PORT("4294967296"); // 2^32
    EXPECT_INVALID_PORT("18446744073709551615"); // 2^64 - 1
    EXPECT_INVALID_PORT("18446744073709551616"); // 2^64

    BeTest::SetFailOnAssert(true);
    }
