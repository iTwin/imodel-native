/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/HttpResponseTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WebTestsHelper.h"
#include <BeHttp/HttpResponse.h>

USING_NAMESPACE_BENTLEY_HTTP

struct HttpResponseTests : public ::testing::Test {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpResponseTests, ToStatusString_AllConnectionStatusValues_NotEmpty)
    {
    for (int i = (int) ConnectionStatus::None; i <= (int) ConnectionStatus::UnknownError; i++)
        {
        EXPECT_NE("", Response::ToStatusString((ConnectionStatus) i, HttpStatus::None));
        }
    }

TEST_F(HttpResponseTests, HttpResponseFromStrings_CorrectFormatNoHeaders_CorrectResponse)
    {
    Response response(HttpStatus::OK, "good/url", "", "good-content");
    EXPECT_STREQ("good-content", response.GetBody().AsString().c_str());
    EXPECT_STREQ("good/url", response.GetEffectiveUrl().c_str());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus(200), response.GetHttpStatus());
    }

TEST_F(HttpResponseTests, HttpResponseFromStrings_CorrectOneHeader_ResponseHasCorrectHeader)
    {
    Response response(HttpStatus::OK, "", "header1:a", "");
    EXPECT_EQ(1, response.GetHeaders().GetMap().size());
    EXPECT_STREQ("a", response.GetHeaders().GetValue("header1"));
    }

TEST_F(HttpResponseTests, HttpResponseFromStrings_HeaderWithoutHeaderSeparator_HeaderValueIsEmpty)
    {
    Response response(HttpStatus::OK, "", "header1", "");
    EXPECT_EQ(1, response.GetHeaders().GetMap().size());
    EXPECT_STREQ(" ", response.GetHeaders().GetValue("header1"));
    }

TEST_F(HttpResponseTests, HttpResponseFromStrings_HeaderWithoutHeaderValue_HeaderValueIsEmpty)
    {
    Response response(HttpStatus::OK, "", "header1:", "");
    EXPECT_EQ(1, response.GetHeaders().GetMap().size());
    EXPECT_STREQ(" ", response.GetHeaders().GetValue("header1"));
    }

TEST_F(HttpResponseTests, HttpResponseFromStrings_HeaderWithoutHeaderName_NoHeaderCreated)
    {
    Response response(HttpStatus::OK, "", ":a", "");
    EXPECT_EQ(0, response.GetHeaders().GetMap().size());
    }

TEST_F(HttpResponseTests, HttpResponseFromStrings_CorrectOneHeaderWithSpaces_ResponseHasCorrectHeader)
    {
    Response response(HttpStatus::OK, "", " header1  : a  ", "");
    EXPECT_EQ(1, response.GetHeaders().GetMap().size());
    EXPECT_STREQ("a", response.GetHeaders().GetValue("header1"));
    }

TEST_F(HttpResponseTests, HttpResponseFromStrings_MultipleHeadersSeparatedByNewLine_ResponseHasCorrectHeaders)
    {
    Response response(HttpStatus::OK, "", "header:a\nheader2:b", "");
    EXPECT_EQ(2, response.GetHeaders().GetMap().size());
    EXPECT_STREQ("a", response.GetHeaders().GetValue("header"));
    EXPECT_STREQ("b", response.GetHeaders().GetValue("header2"));
    }

TEST_F(HttpResponseTests, HttpResponseFromStrings_MultipleHeadersSeparatedByNewLineWithSpaces_ResponseHasCorrectHeaders)
    {
    Response response(HttpStatus::OK, "", "header:a \n  header2:b", "");
    EXPECT_EQ(2, response.GetHeaders().GetMap().size());
    EXPECT_STREQ("a", response.GetHeaders().GetValue("header"));
    EXPECT_STREQ("b", response.GetHeaders().GetValue("header2"));
    }

TEST_F(HttpResponseTests, HttpResponseFromStrings_DublicatedHeaderNames_ResponseHeadersHasLastHeaderValue)
    {
    Response response(HttpStatus::OK, "", "header:a\nheader:b", "");
    EXPECT_EQ(1, response.GetHeaders().GetMap().size());
    EXPECT_STREQ("b", response.GetHeaders().GetValue("header"));
    }

TEST_F(HttpResponseTests, HttpResponseFromStrings_MultipleDoubleLineSepareted_ResponseHasCorrectHeaders)
    {
    Response response(HttpStatus::OK, "", "header:a\n\nheader3:c", "");
    EXPECT_EQ(2, response.GetHeaders().GetMap().size());
    EXPECT_STREQ("a", response.GetHeaders().GetValue("header"));
    EXPECT_STREQ(nullptr, response.GetHeaders().GetValue("header2"));
    EXPECT_STREQ("c", response.GetHeaders().GetValue("header3"));
    }

TEST_F(HttpResponseTests, HttpResponseFromStrings_MultipleHeaderSeparator_HeaderValueWithHeaderSeparator)
    {
    Response response(HttpStatus::OK, "", "header:aheader2:b", "");
    EXPECT_EQ(1, response.GetHeaders().GetMap().size());
    EXPECT_STREQ("aheader2:b", response.GetHeaders().GetValue("header"));
    }
