/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/HttpHeadersTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "HttpHeadersTests.h"

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS
TEST_F(HttpHeadersTests, SetValue_EmptyString_RemovesValue)
    {
    HttpHeaders headers;

    headers.SetValue("TestField", "foo");
    EXPECT_EQ(1, headers.GetMap().size());

    headers.SetValue("TestField", "");

    EXPECT_EQ(nullptr, headers.GetValue("TestField"));
    EXPECT_EQ(0, headers.GetMap().size());
    }

TEST_F(HttpHeadersTests, GetValue_SingleSetValue_ReturnsValue)
    {
    HttpHeaders headers;

    headers.SetValue("Content-Type", "text/html");

    EXPECT_STREQ("text/html", headers.GetValue("Content-Type"));
    }

TEST_F(HttpHeadersTests, GetValue_SetValueAndAddValue_ReturnValuesCommaSeparated)
    {
    HttpHeaders headers;

    headers.SetValue("Server", "foo");
    headers.AddValue("Server", "bar");

    EXPECT_STREQ("foo, bar", headers.GetValue("Server"));
    }

TEST_F(HttpHeadersTests, GetValue_MultipleSetValue_ReturnsOnlyLastValue)
    {
    HttpHeaders headers;

    headers.SetValue("Server", "foo");
    headers.SetValue("Server", "bar");

    EXPECT_STREQ("bar", headers.GetValue("Server"));
    }

TEST_F(HttpHeadersTests, GetValue_ValueNotSet_ReturnsNullptr)
    {
    HttpHeaders headers;
    EXPECT_EQ(nullptr, headers.GetValue("Server"));
    }

TEST_F(HttpHeadersTests, GetMap_ValuesSet_ReturnsMap)
    {
    HttpHeaders headers;

    headers.SetValue("Server", "foo");
    headers.SetValue("Content-Type", "text/html");

    EXPECT_EQ(2, headers.GetMap().size());
    EXPECT_STREQ("text/html", headers.GetMap().find("Content-Type")->second.c_str());
    EXPECT_STREQ("foo", headers.GetMap().find("Server")->second.c_str());
    }

TEST_F(HttpHeadersTests, Clear_SetValueAndClear_ReturnsEmptyMap)
    {
    HttpHeaders headers;

    headers.SetValue("Server", "foo");
    headers.Clear();

    EXPECT_EQ(0, headers.GetMap().size());
    }
