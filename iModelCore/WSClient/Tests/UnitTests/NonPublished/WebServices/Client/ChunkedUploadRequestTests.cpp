/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../../Utils/WebServicesTestsHelper.h"
#include "../../../../../Client/ChunkedUploadRequest.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

struct ChunkedUploadRequestTests : BaseMockHttpHandlerTest {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChunkedUploadRequestTests, PerformAsync_MethodSpecified_HandShakeWithSameMethod)
    {
    ChunkedUploadRequest request("BOO", "http://foo.com", GetClient());

    GetHandler().ExpectOneRequest().ForAnyRequest([] (Http::RequestCR request)
        {
        EXPECT_STREQ("BOO", request.GetMethod().c_str());
        return StubHttpResponse(ConnectionStatus::Canceled);
        });

    request.PerformAsync()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChunkedUploadRequestTests, PerformAsync_RequestBodySpecified_SendsRequiredHandshakeHeaders)
    {
    ChunkedUploadRequest request("PUT", "http://foo.com", GetClient());
    request.SetRequestBody(HttpStringBody::Create("abcd"), "Test.txt");
    request.SetETag("foo");

    GetHandler().ExpectOneRequest().ForAnyRequest([] (Http::RequestCR request)
        {
        EXPECT_STREQ("foo", request.GetHeaders().GetIfMatch());
        EXPECT_STREQ("bytes */4", request.GetHeaders().GetContentRange());

        EXPECT_STREQ(R"(attachment; filename="Test.txt")", request.GetHeaders().GetContentDisposition());

        return StubHttpResponse(ConnectionStatus::Canceled);
        });

    EXPECT_EQ(ConnectionStatus::Canceled, request.PerformAsync()->GetResult().GetConnectionStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChunkedUploadRequestTests, PerformAsync_RequestBodySpecifiedWithFileName_EncodesFileName)
    {
    ChunkedUploadRequest request("PUT", "http://foo.com", GetClient());
    request.SetRequestBody(HttpStringBody::Create("Foo"), "'A B'.txt");

    GetHandler().ExpectOneRequest().ForAnyRequest([] (Http::RequestCR request)
        {
        EXPECT_STREQ(R"(attachment; filename="'A B'.txt")", request.GetHeaders().GetContentDisposition());

        return StubHttpResponse();
        });

    request.PerformAsync()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChunkedUploadRequestTests, PerformAsync_HandshakeBodySpecified_SendsRequiredHandshakeHeaders)
    {
    ChunkedUploadRequest request("PUT", "http://foo.com", GetClient());
    request.SetHandshakeRequestBody(HttpStringBody::Create("abcd"), REQUESTHEADER_ContentType_ApplicationXml);

    GetHandler().ExpectOneRequest().ForAnyRequest([] (Http::RequestCR request)
        {
        EXPECT_STREQ(REQUESTHEADER_ContentType_ApplicationXml, request.GetHeaders().GetContentType());
        return StubHttpResponse(ConnectionStatus::Canceled);
        });

    EXPECT_EQ(ConnectionStatus::Canceled, request.PerformAsync()->GetResult().GetConnectionStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChunkedUploadRequestTests, PerformAsync_FirtResponseNotResumeIncomplete_StopsRequest)
    {
    ChunkedUploadRequest request("PUT", "http://foo.com", GetClient());
    request.SetHandshakeRequestBody(HttpStringBody::Create("abcd"), REQUESTHEADER_ContentType_ApplicationXml);

    GetHandler().ExpectOneRequest().ForAnyRequest(StubHttpResponse(HttpStatus::NotFound));

    EXPECT_EQ(HttpStatus::NotFound, request.PerformAsync()->GetResult().GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChunkedUploadRequestTests, PerformAsync_FirstResponseResumeIncomplete_SendsSecondRequestWithCorrectContent)
    {
    ChunkedUploadRequest request("PUT", "http://foo.com", GetClient());

    request.SetRequestBody(HttpStringBody::Create("abcd"), nullptr);

    GetHandler()
        .ExpectRequests(2)
        .ForFirstRequest(StubHttpResponse(HttpStatus::ResumeIncomplete, "", {{"ETag", "uploadTag"}}))
        .ForAnyRequest([] (Http::RequestCR request)
        {
        EXPECT_STREQ("uploadTag", request.GetHeaders().GetIfMatch());
        EXPECT_STREQ("bytes 0-3/4", request.GetHeaders().GetContentRange());
        EXPECT_STREQ("abcd", request.GetRequestBody()->AsString().c_str());
        return StubHttpResponse(HttpStatus::OK);
        });

    EXPECT_EQ(HttpStatus::OK, request.PerformAsync()->GetResult().GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChunkedUploadRequestTests, PerformAsync_FirstResponseResumeIncompleteWithRange_SendsSecondRequestWithCorrectContent)
    {
    ChunkedUploadRequest request("PUT", "http://foo.com", GetClient());

    request.SetRequestBody(HttpStringBody::Create("abcd"), nullptr);

    GetHandler()
        .ExpectRequests(2)
        .ForFirstRequest(StubHttpResponse(HttpStatus::ResumeIncomplete, "", {{"ETag", "uploadTag"}, {"Range", "bytes=0-2"}}))
        .ForAnyRequest([] (Http::RequestCR request)
        {
        EXPECT_STREQ("uploadTag", request.GetHeaders().GetIfMatch());
        EXPECT_STREQ("bytes 3-3/4", request.GetHeaders().GetContentRange());
        EXPECT_STREQ("d", request.GetRequestBody()->AsString().c_str());
        return StubHttpResponse(HttpStatus::OK);
        });

    EXPECT_EQ(HttpStatus::OK, request.PerformAsync()->GetResult().GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChunkedUploadRequestTests, PerformAsync_ResponseToChunkedUploadIsResumeIncomplete_SendsChunkAgain)
    {
    ChunkedUploadRequest request("PUT", "http://foo.com", GetClient());

    request.SetRequestBody(HttpStringBody::Create("abcd"), nullptr);

    GetHandler()
        .ExpectRequests(3)
        .ForRequest(1, StubHttpResponse(HttpStatus::ResumeIncomplete, "", {{"ETag", "uploadTag"}, {"Range", "bytes=0-2"}}))
        .ForRequest(2, StubHttpResponse(HttpStatus::ResumeIncomplete, "", {{"ETag", "uploadTag"}, {"Range", "bytes=0-1"}}))
        .ForRequest(3, [] (Http::RequestCR request)
        {
        EXPECT_STREQ("uploadTag", request.GetHeaders().GetIfMatch());
        EXPECT_STREQ("bytes 2-3/4", request.GetHeaders().GetContentRange());
        EXPECT_STREQ("cd", request.GetRequestBody()->AsString().c_str());
        return StubHttpResponse(HttpStatus::OK);
        });

    EXPECT_EQ(HttpStatus::OK, request.PerformAsync()->GetResult().GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChunkedUploadRequestTests, PerformAsync_SecondResponseWithPreconditionFailed_SendsHandshakeAgain)
    {
    ChunkedUploadRequest request("PUT", "http://foo.com", GetClient());

    request.SetRequestBody(HttpStringBody::Create("abcd"), nullptr);

    GetHandler()
        .ExpectRequests(3)
        .ForRequest(1, StubHttpResponse(HttpStatus::ResumeIncomplete, "", {{"ETag", "uploadTag"}}))
        .ForRequest(2, StubHttpResponse(HttpStatus::PreconditionFailed))
        .ForRequest(3, [] (Http::RequestCR request)
        {
        EXPECT_STREQ("bytes */4", request.GetHeaders().GetContentRange());
        return StubHttpResponse(ConnectionStatus::Canceled);
        });

    EXPECT_EQ(ConnectionStatus::Canceled, request.PerformAsync()->GetResult().GetConnectionStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChunkedUploadRequestTests, PerformAsync_ChunkedRequestAndRequestsHeadersAdded_HeadersCorrect)
    {
    ChunkedUploadRequest request("PUT", "http://foo.com", GetClient());
    request.SetHandshakeRequestBody(HttpStringBody::Create("abcd"), nullptr);
    request.SetRequestBody(HttpStringBody::Create("abcd"), nullptr);
    request.GetRequestsHeaders().AddValue("A", "1");

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, [] (Http::RequestCR request)
        {
        EXPECT_STREQ("1", request.GetHeaders().GetValue("A"));
        return StubHttpResponse(HttpStatus::ResumeIncomplete);
        });
    GetHandler().ForRequest(2, [] (Http::RequestCR request)
        {
        EXPECT_STREQ("1", request.GetHeaders().GetValue("A"));
        return StubHttpResponse(HttpStatus::OK);
        });

    EXPECT_EQ(HttpStatus::OK, request.PerformAsync()->GetResult().GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                    02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChunkedUploadRequestTests, PerformAsync_LastRequestHeadersAdded_HeadersCorrect)
    {
    ChunkedUploadRequest request("PUT", "http://foo.com", GetClient());
    request.GetLastRequestHeaders().AddValue("A", "1");

    GetHandler().ExpectOneRequest().ForAnyRequest([] (Http::RequestCR request)
        {
        EXPECT_STREQ("1", request.GetHeaders().GetValue("A"));
        return StubHttpResponse(ConnectionStatus::OK);
        });

    request.PerformAsync()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                    02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChunkedUploadRequestTests, PerformAsync_RequestBodySpecifiedLastRequestHeadersAdded_HeadersCorrect)
    {
    ChunkedUploadRequest request("PUT", "http://foo.com", GetClient());

    request.SetRequestBody(HttpStringBody::Create("abcd"), nullptr);
    request.GetLastRequestHeaders().AddValue("A", "1");

    GetHandler().ExpectOneRequest().ForAnyRequest([] (Http::RequestCR request)
        {
        EXPECT_STREQ("1", request.GetHeaders().GetValue("A"));
        return StubHttpResponse(ConnectionStatus::OK);
        });

    request.PerformAsync()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                    02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChunkedUploadRequestTests, PerformAsync_HandshakeRequestBodySpecifiedLastRequestHeadersAdded_HeadersCorrect)
    {
    ChunkedUploadRequest request("PUT", "http://foo.com", GetClient());

    request.SetHandshakeRequestBody(HttpStringBody::Create("abcd"), nullptr);
    request.GetLastRequestHeaders().AddValue("A", "1");

    GetHandler().ExpectOneRequest().ForAnyRequest([] (Http::RequestCR request)
        {
        EXPECT_STREQ("1", request.GetHeaders().GetValue("A"));
        return StubHttpResponse(ConnectionStatus::OK);
        });

    request.PerformAsync()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                    02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChunkedUploadRequestTests, PerformAsync_ResponseToChunkedLastRequestHeadersAdded_HeadersCorrect)
    {
    ChunkedUploadRequest request("PUT", "http://foo.com", GetClient());
    request.SetHandshakeRequestBody(HttpStringBody::Create("abcd"), nullptr);
    request.SetRequestBody(HttpStringBody::Create("abcd"), nullptr);
    request.GetLastRequestHeaders().AddValue("A", "1");

    GetHandler()
        .ExpectRequests(2)
        .ForRequest(1, [] (Http::RequestCR request)
            {
            EXPECT_STREQ(nullptr, request.GetHeaders().GetValue("A"));
            return StubHttpResponse(HttpStatus::ResumeIncomplete, "", {{"ETag", "uploadTag"}, {"Range", "bytes=0-2"}});
            })
        .ForRequest(2, [] (Http::RequestCR request)
            {
            EXPECT_STREQ("1", request.GetHeaders().GetValue("A"));
            return StubHttpResponse(HttpStatus::OK);
            });

    EXPECT_EQ(HttpStatus::OK, request.PerformAsync()->GetResult().GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChunkedUploadRequestTests, PerformAsync_ChunkedRequestAndRequestsHeadersAndLastRequestHeadersHaveSameKey_HeadersCorrect)
    {
    ChunkedUploadRequest request("PUT", "http://foo.com", GetClient());
    request.SetHandshakeRequestBody(HttpStringBody::Create("abcd"), nullptr);
    request.SetRequestBody(HttpStringBody::Create("abcd"), nullptr);
    request.GetRequestsHeaders().AddValue("A", "1");
    request.GetLastRequestHeaders().AddValue("A", "2");

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, [] (Http::RequestCR request)
        {
        EXPECT_STREQ("1", request.GetHeaders().GetValue("A"));
        return StubHttpResponse(HttpStatus::ResumeIncomplete);
        });
    GetHandler().ForRequest(2, [] (Http::RequestCR request)
        {
        EXPECT_STREQ("1, 2", request.GetHeaders().GetValue("A"));
        return StubHttpResponse(HttpStatus::OK);
        });

    EXPECT_EQ(HttpStatus::OK, request.PerformAsync()->GetResult().GetHttpStatus());
    }
