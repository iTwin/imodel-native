/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "WebTestsHelper.h"

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

struct HttpHeadersTests : public ::testing::Test{};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpHeadersTests, SetValue_EmptyString_RemovesValue)
    {
    HttpHeaders headers;

    headers.SetValue("TestField", "foo");
    EXPECT_EQ(1, headers.GetMap().size());

    headers.SetValue("TestField", "");

    EXPECT_EQ(nullptr, headers.GetValue("TestField"));
    EXPECT_EQ(0, headers.GetMap().size());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpHeadersTests, GetValue_SingleSetValue_ReturnsValue)
    {
    HttpHeaders headers;

    headers.SetValue("Content-Type", REQUESTHEADER_ContentType_TextHtml);

    EXPECT_STREQ(REQUESTHEADER_ContentType_TextHtml, headers.GetValue("Content-Type"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpHeadersTests, GetValue_SetValueAndAddValue_ReturnValuesCommaSeparated)
    {
    HttpHeaders headers;

    headers.SetValue("Server", "foo");
    headers.AddValue("Server", "bar");

    EXPECT_STREQ("foo, bar", headers.GetValue("Server"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpHeadersTests, GetValue_MultipleSetValue_ReturnsOnlyLastValue)
    {
    HttpHeaders headers;

    headers.SetValue("Server", "foo");
    headers.SetValue("Server", "bar");

    EXPECT_STREQ("bar", headers.GetValue("Server"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpHeadersTests, GetValue_ValueNotSet_ReturnsNullptr)
    {
    HttpHeaders headers;
    EXPECT_EQ(nullptr, headers.GetValue("Server"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpHeadersTests, GetMap_ValuesSet_ReturnsMap)
    {
    HttpHeaders headers;

    headers.SetValue("Server", "foo");
    headers.SetValue("Content-Type", REQUESTHEADER_ContentType_TextHtml);

    EXPECT_EQ(2, headers.GetMap().size());
    EXPECT_STREQ(REQUESTHEADER_ContentType_TextHtml, headers.GetMap().find("Content-Type")->second.c_str());
    EXPECT_STREQ("foo", headers.GetMap().find("Server")->second.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                       05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpHeadersTests, Clear_SetValueAndClear_ReturnsEmptyMap)
    {
    HttpHeaders headers;

    headers.SetValue("Server", "foo");
    headers.Clear();

    EXPECT_EQ(0, headers.GetMap().size());
    }
