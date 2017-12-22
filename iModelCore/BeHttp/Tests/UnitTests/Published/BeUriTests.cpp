/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/BeUriTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WebTestsHelper.h"
#include <BeHttp/BeUri.h>

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

struct BeUriTests : public ::testing::Test {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, Ctor_EmptyString_EmptySet)
    {
    EXPECT_STREQ("", BeUri().GetString().c_str());
    EXPECT_STREQ("", BeUri("").GetString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, Ctor_ValidUrl_UrlSet)
    {
    EXPECT_STREQ("http://test.com/foo/boo", BeUri("http://test.com/foo/boo").GetString().c_str());
    EXPECT_STREQ("test.com/foo/boo", BeUri("test.com/foo/boo").GetString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, Ctor_UrlWithUnsafeCharacters_CharactersAreEscaped)
    {
    BeUri uri(TEST_URL_UNSAFE_CHARS);
    EXPECT_STREQ(TEST_URL_UNSAFE_CHARS_ESCAPED, uri.GetString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, EscapeUnsafeCharactersInUrl_UrlWithUnsafeCharacters_CharactersAreEscaped)
    {
    Utf8String url = TEST_URL_UNSAFE_CHARS;
    EXPECT_STREQ(TEST_URL_UNSAFE_CHARS_ESCAPED, BeUri::EscapeUnsafeCharactersInUrl(url).c_str());
    EXPECT_STREQ(TEST_URL_UNSAFE_CHARS_ESCAPED, url.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, GetHost_Empty_EmptyReturned)
    {
    EXPECT_STREQ("", BeUri().GetHost().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, GetHost_UrlsWithHost_HostReturned)
    {
    EXPECT_STREQ("test.host.com", BeUri("http://test.site.com:80/foo").GetHost().c_str());
    EXPECT_STREQ("test.host.com", BeUri("https://test.site.com:80/foo").GetHost().c_str());
    EXPECT_STREQ("test.host.com", BeUri("foo://test.site.com:80/foo").GetHost().c_str());
    EXPECT_STREQ("test.host.com", BeUri("test.site.com:80/foo").GetHost().c_str());
    EXPECT_STREQ("test", BeUri("test").GetHost().c_str());
    EXPECT_STREQ("test.com", BeUri("test.com/foo").GetHost().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, GetScheme_Empty_EmptyReturned)
    {
    EXPECT_STREQ("", BeUri().GetScheme().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, GetScheme_UrlWithoutScheme_EmptyReturned)
    {
    EXPECT_STREQ("", BeUri("test.site.com:80/foo").GetScheme().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, GetScheme_UrlWithScheme_SchemeReturned)
    {
    EXPECT_STREQ("boo", BeUri("boo://test.site.com:80/foo").GetScheme().c_str());
    EXPECT_STREQ("http", BeUri("http://test").GetScheme().c_str());
    EXPECT_STREQ("file", BeUri("file://test.txt").GetScheme().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, EscapeString_Strings_EscapedReturnedWhereNeeded)
    {
    EXPECT_STREQ("", BeUri::EscapeString("").c_str());
    EXPECT_STREQ("Foo%20Boo", BeUri::EscapeString("Foo Boo").c_str());
    EXPECT_STREQ("%2F-%2B%2A", BeUri::EscapeString("/-+*").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeUriTests, UnescapeString_Strings_UnescapedReturned)
    {
    EXPECT_STREQ("", BeUri::UnescapeString("").c_str());
    EXPECT_STREQ("Foo Boo", BeUri::UnescapeString("Foo Boo").c_str());
    EXPECT_STREQ("Foo Boo", BeUri::UnescapeString("Foo%20Boo").c_str());
    EXPECT_STREQ("/-+*", BeUri::UnescapeString("/-+*").c_str());
    EXPECT_STREQ("/-+*", BeUri::UnescapeString("%2F-%2B%2A").c_str());
    }
