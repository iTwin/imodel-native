/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
        EXPECT_NE("", Response::ToStatusString((ConnectionStatus) i, HttpStatus::None));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        09/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpResponseTests, Ctor_Default_ExpectedValues)
    {
    Response response;
    EXPECT_STREQ("", response.GetBody().AsString().c_str());
    EXPECT_STREQ("", response.GetEffectiveUrl().c_str());
    EXPECT_EQ(ConnectionStatus::None, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus::None, response.GetHttpStatus());
    EXPECT_TRUE(response.GetHeaders().GetMap().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        09/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpResponseTests, Ctor_UsingHttpStatus_ExpectedValues)
    {
    auto content = HttpResponseContent::Create(HttpStringBody::Create("TestBody"));
    content->GetHeaders().SetValue("A", "B");
    Response response(HttpStatus::BadGateway, "TestUrl", content);
    EXPECT_STREQ("TestBody", response.GetBody().AsString().c_str());
    EXPECT_STREQ("TestUrl", response.GetEffectiveUrl().c_str());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus::BadGateway, response.GetHttpStatus());
    EXPECT_EQ(1, response.GetHeaders().GetMap().size());
    EXPECT_STREQ(response.GetHeaders().GetValue("A"), "B");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        09/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpResponseTests, Ctor_UsingHttpStatusAndNullContent_ExpectedValues)
    {
    Response response(HttpStatus::BadGateway, "TestUrl", nullptr);
    EXPECT_STREQ("", response.GetBody().AsString().c_str());
    EXPECT_STREQ("TestUrl", response.GetEffectiveUrl().c_str());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus::BadGateway, response.GetHttpStatus());
    EXPECT_TRUE(response.GetHeaders().GetMap().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        09/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpResponseTests, Ctor_UsingConnectionStatus_ExpectedValues)
    {
    Response response(ConnectionStatus::CertificateError);
    EXPECT_STREQ("", response.GetBody().AsString().c_str());
    EXPECT_STREQ("", response.GetEffectiveUrl().c_str());
    EXPECT_EQ(ConnectionStatus::CertificateError, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus::None, response.GetHttpStatus());
    EXPECT_TRUE(response.GetHeaders().GetMap().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpResponseTests, Ctor_FromStringsWithCorrectFormatNoHeaders_CorrectResponse)
    {
    Response response(HttpStatus::OK, "good/url", "", "good-content");
    EXPECT_STREQ("good-content", response.GetBody().AsString().c_str());
    EXPECT_STREQ("good/url", response.GetEffectiveUrl().c_str());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus(200), response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpResponseTests, Ctor_FromStringsWithCorrectOneHeader_ResponseHasCorrectHeader)
    {
    Response response(HttpStatus::OK, "", "header1:a", "");
    EXPECT_EQ(1, response.GetHeaders().GetMap().size());
    EXPECT_STREQ("a", response.GetHeaders().GetValue("header1"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpResponseTests, Ctor_FromStringsWithHeaderWithoutHeaderSeparator_HeaderValueIsEmpty)
    {
    Response response(HttpStatus::OK, "", "header1", "");
    EXPECT_EQ(1, response.GetHeaders().GetMap().size());
    EXPECT_STREQ(" ", response.GetHeaders().GetValue("header1"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpResponseTests, Ctor_FromStringsWithHeaderWithoutHeaderValue_HeaderValueIsEmpty)
    {
    Response response(HttpStatus::OK, "", "header1:", "");
    EXPECT_EQ(1, response.GetHeaders().GetMap().size());
    EXPECT_STREQ(" ", response.GetHeaders().GetValue("header1"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpResponseTests, Ctor_FromStringsWithHeaderWithoutHeaderName_NoHeaderCreated)
    {
    Response response(HttpStatus::OK, "", ":a", "");
    EXPECT_EQ(0, response.GetHeaders().GetMap().size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpResponseTests, Ctor_FromStringsWithCorrectOneHeaderWithSpaces_ResponseHasCorrectHeader)
    {
    Response response(HttpStatus::OK, "", " header1  : a  ", "");
    EXPECT_EQ(1, response.GetHeaders().GetMap().size());
    EXPECT_STREQ("a", response.GetHeaders().GetValue("header1"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpResponseTests, Ctor_FromStringsWithMultipleHeadersSeparatedByNewLine_ResponseHasCorrectHeaders)
    {
    Response response(HttpStatus::OK, "", "header:a\nheader2:b", "");
    EXPECT_EQ(2, response.GetHeaders().GetMap().size());
    EXPECT_STREQ("a", response.GetHeaders().GetValue("header"));
    EXPECT_STREQ("b", response.GetHeaders().GetValue("header2"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpResponseTests, Ctor_FromStringsWithMultipleHeadersSeparatedByNewLineWithSpaces_ResponseHasCorrectHeaders)
    {
    Response response(HttpStatus::OK, "", "header:a \n  header2:b", "");
    EXPECT_EQ(2, response.GetHeaders().GetMap().size());
    EXPECT_STREQ("a", response.GetHeaders().GetValue("header"));
    EXPECT_STREQ("b", response.GetHeaders().GetValue("header2"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpResponseTests, Ctor_FromStringsWithDublicatedHeaderNames_ResponseHeadersHasLastHeaderValue)
    {
    Response response(HttpStatus::OK, "", "header:a\nheader:b", "");
    EXPECT_EQ(1, response.GetHeaders().GetMap().size());
    EXPECT_STREQ("b", response.GetHeaders().GetValue("header"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpResponseTests, Ctor_FromStringsWithMultipleDoubleLineSepareted_ResponseHasCorrectHeaders)
    {
    Response response(HttpStatus::OK, "", "header:a\n\nheader3:c", "");
    EXPECT_EQ(2, response.GetHeaders().GetMap().size());
    EXPECT_STREQ("a", response.GetHeaders().GetValue("header"));
    EXPECT_STREQ(nullptr, response.GetHeaders().GetValue("header2"));
    EXPECT_STREQ("c", response.GetHeaders().GetValue("header3"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpResponseTests, Ctor_FromStringsWithMultipleHeaderSeparator_HeaderValueWithHeaderSeparator)
    {
    Response response(HttpStatus::OK, "", "header:aheader2:b", "");
    EXPECT_EQ(1, response.GetHeaders().GetMap().size());
    EXPECT_STREQ("aheader2:b", response.GetHeaders().GetValue("header"));
    }
