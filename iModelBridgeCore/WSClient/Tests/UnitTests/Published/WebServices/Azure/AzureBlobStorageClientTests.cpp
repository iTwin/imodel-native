/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "AzureBlobStorageClientTests.h"

#include <WebServices/Azure/AzureBlobStorageClient.h>
#include <BeHttp/HttpHeaders.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_TASKS

#define HEADER_XMsClientRequestId "x-ms-client-request-id"

const Utf8String s_errorResponse = "<?xml version=\"1.0\" encoding=\"utf-8\"?><Error><Code>TestCode</Code><Message>TestMessage</Message></Error>";

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AzureBlobStorageClientTests, SendGetFileRequest_ServerReturnsError_ReturnsErrorResponse)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForFirstRequest([=] (Http::RequestCR request)
        {
        return StubHttpResponse(HttpStatus::BadRequest, s_errorResponse, {{ "Content-Type" , REQUESTHEADER_ContentType_ApplicationXml }});
        });

    auto result = client->SendGetFileRequest("https://myaccount.blob.core.windows.net/sascontainer/sasblob.txt?SAS", StubFilePath())->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(HttpStatus::BadRequest, result.GetError().GetHttpStatus());
    EXPECT_EQ("TestMessage", result.GetError().GetMessage());
    EXPECT_EQ("TestCode", result.GetError().GetCode());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AzureBlobStorageClientTests, SendGetFileRequest_ResponseIsOK_ReturnsSuccess)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());

    BeFileName filePath = StubFilePath();

    GetHandler().ExpectRequests(1);
    GetHandler().ForFirstRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://myaccount.blob.core.windows.net/sascontainer/sasblob.txt?SAS", request.GetUrl().c_str());

        HttpFileBodyPtr httpFileBody = dynamic_cast<HttpFileBody*> (request.GetResponseBody().get());
        EXPECT_STREQ(filePath, httpFileBody->GetFilePath());

        return StubHttpResponse(HttpStatus::OK, "", {{"ETag", "FooBoo"}});
        });

    auto result = client->SendGetFileRequest("https://myaccount.blob.core.windows.net/sascontainer/sasblob.txt?SAS", filePath)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ("FooBoo", result.GetValue().GetETag());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Mantas.Smicius                        11/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AzureBlobStorageClientTests, SendGetFileRequest_ActivityOptionsEnabled_SendsRequestWithSpecifiedXMsClientRequestId)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForFirstRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("specifiedActivityId", request.GetHeaders().GetValue(HEADER_XMsClientRequestId));
        return StubHttpResponse(HttpStatus::OK);
        });

    auto options = std::make_shared<IAzureBlobStorageClient::RequestOptions>();
    options->GetActivityOptions()->SetActivityId("specifiedActivityId");

    EXPECT_TRUE(options->GetActivityOptions()->HasActivityId());
    EXPECT_STREQ("specifiedActivityId", options->GetActivityOptions()->GetActivityId().c_str());

    client->SendGetFileRequest("https://test/foo", StubFilePath(), nullptr, options)->GetResult();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AzureBlobStorageClientTests, SendUpdateFileRequest_ServerReturnsError_ReturnsErrorResponse)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (Http::RequestCR request)
        {
        return StubHttpResponse(HttpStatus::BadRequest, s_errorResponse, {{ "Content-Type" , REQUESTHEADER_ContentType_ApplicationXml }});
        });

    auto result = client->SendUpdateFileRequest("https://test/foo", StubFile())->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(HttpStatus::BadRequest, result.GetError().GetHttpStatus());
    EXPECT_EQ("TestMessage", result.GetError().GetMessage());
    EXPECT_EQ("TestCode", result.GetError().GetCode());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Andrius.Zonys                     01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AzureBlobStorageClientTests, SendUpdateFileRequest_ResponseIsOK_ReturnsSuccess)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());
    uint32_t chunkSize = 4 * 1024 * 1024; // 4MB


    GetHandler().ExpectRequests(4);
    GetHandler().ForRequest(1, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://test/foo&comp=block&blockid=MDAwMDA=", request.GetUrl().c_str());
        EXPECT_EQ(chunkSize, request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::Created);
        });
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://test/foo&comp=block&blockid=MDAwMDE=", request.GetUrl().c_str());
        EXPECT_EQ(chunkSize, request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::Created);
        });
    GetHandler().ForRequest(3, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://test/foo&comp=block&blockid=MDAwMDI=", request.GetUrl().c_str());
        EXPECT_EQ(chunkSize - 200, request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::Created);
        });
    GetHandler().ForRequest(4, [] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://test/foo&comp=blocklist", request.GetUrl().c_str());
        EXPECT_STREQ("<?xml version=\"1.0\" encoding=\"utf-8\"?><BlockList><Latest>MDAwMDA=</Latest><Latest>MDAwMDE=</Latest><Latest>MDAwMDI=</Latest></BlockList>", request.GetRequestBody()->AsString().c_str());
        return StubHttpResponse(HttpStatus::Created, "", {{"ETag", "FooBoo"}});
        });

    BeFileName filePath = StubFileWithSize(chunkSize * 3 - 200); // two 4MB chunks and one smaller.
    auto result = client->SendUpdateFileRequest("https://test/foo", filePath)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ("FooBoo", result.GetValue().GetETag());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Andrius.Zonys                     01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AzureBlobStorageClientTests, SendUpdateFileRequest_ProgressCallbackPassedForMultiChunkUpload_ProgressReported)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());
    uint32_t chunkSize = 4 * 1024 * 1024; // 4MB

    GetHandler().ExpectRequests(4);
    GetHandler().ForRequest(1, [=] (Http::RequestCR request)
        {
        request.GetUploadProgressCallback()(1010, (double)request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::Created);
        });
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        request.GetUploadProgressCallback()(1020, (double) request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::Created);
        });
    GetHandler().ForRequest(3, [=] (Http::RequestCR request)
        {
        request.GetUploadProgressCallback()(1030, (double) request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::Created);
        });
    GetHandler().ForRequest(4, [] (Http::RequestCR request)
        {
        // Final block list
        EXPECT_FALSE(request.GetUploadProgressCallback());
        EXPECT_FALSE(request.GetDownloadProgressCallback());
        return StubHttpResponse(HttpStatus::Created);
        });

    uint32_t fileSize = chunkSize * 3 - 200; // two 4MB chunks and one smaller.

    size_t i = 0;
    std::vector<uint32_t> expectedProgress {
        0,
        0 * chunkSize + 1010,
        1 * chunkSize + 1020,
        2 * chunkSize + 1030,
        fileSize
        };

    auto onProgress = [&] (double bytesTransfered, double bytesTotal)
        {
        EXPECT_EQ(fileSize, (uint32_t)bytesTotal);
        EXPECT_LT(i, expectedProgress.size());
        EXPECT_EQ(expectedProgress[i], (uint32_t)bytesTransfered);
        i++;
        };

    BeFileName filePath = StubFileWithSize(fileSize);
    auto result = client->SendUpdateFileRequest("https://test/foo", filePath, onProgress)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Andrius.Zonys                     01/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AzureBlobStorageClientTests, SendUpdateFileRequest_ResponseIsBadRequest_ReturnsError)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());
    uint32_t chunkSize = 4 * 1024 * 1024; // 4MB

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://test/foo&comp=block&blockid=MDAwMDA=", request.GetUrl().c_str());
        EXPECT_EQ(chunkSize, request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::OK);
        });
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://test/foo&comp=block&blockid=MDAwMDE=", request.GetUrl().c_str());
        EXPECT_EQ(chunkSize, request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::BadRequest);
        });

    BeFileName filePath = StubFileWithSize(chunkSize * 3 - 200); // two 4MB chunks and one smaller.
    auto result = client->SendUpdateFileRequest("https://test/foo", filePath)->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ("Http error: 400", result.GetError().GetDescription());
    }

TEST_F(AzureBlobStorageClientTests, SendUpdateFileRequest_FileSmallerThanChunk_UploadsWithOneRequest)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());
    uint32_t fileSize = 200;

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://test/foo", request.GetUrl().c_str());
        EXPECT_EQ(fileSize, request.GetRequestBody()->GetLength());
        EXPECT_STREQ("BlockBlob", request.GetHeaders().GetValue("x-ms-blob-type"));
        return StubHttpResponse(HttpStatus::Created, "", {{"ETag", "FooBoo"}});
        });

    BeFileName filePath = StubFileWithSize(fileSize);
    auto result = client->SendUpdateFileRequest("https://test/foo", filePath)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_EQ("FooBoo", result.GetValue().GetETag());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Mantas.Smicius                        11/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AzureBlobStorageClientTests, SendUpdateFileRequest_ActivityOptionsNotSpecified_SendsRequestWithoutXMsClientRequestId)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (Http::RequestCR request)
        {
        EXPECT_EQ(nullptr, request.GetHeaders().GetValue(HEADER_XMsClientRequestId));
        return StubHttpResponse(HttpStatus::Created);
        });

    client->SendUpdateFileRequest("https://test/foo", StubFile())->GetResult();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Mantas.Smicius                        11/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AzureBlobStorageClientTests, SendUpdateFileRequest_DefaultActivityOptions_SendsRequestWithoutXMsClientRequestId)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (Http::RequestCR request)
        {
        EXPECT_EQ(nullptr, request.GetHeaders().GetValue(HEADER_XMsClientRequestId));
        return StubHttpResponse(HttpStatus::Created);
        });

    auto options = std::make_shared<IAzureBlobStorageClient::RequestOptions>();

    EXPECT_FALSE(options->GetActivityOptions()->HasActivityId());
    EXPECT_TRUE(options->GetActivityOptions()->GetActivityId().empty());

    client->SendUpdateFileRequest("https://test/foo", StubFile(), nullptr, options)->GetResult();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Mantas.Smicius                        11/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AzureBlobStorageClientTests, SendUpdateFileRequest_EmptyActivityIdSpecified_SendsRequestWithoutXMsClientRequestId)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (Http::RequestCR request)
        {
        EXPECT_EQ(nullptr, request.GetHeaders().GetValue(HEADER_XMsClientRequestId));
        return StubHttpResponse(HttpStatus::Created);
        });

    auto options = std::make_shared<IAzureBlobStorageClient::RequestOptions>();
    options->GetActivityOptions()->SetActivityId(Utf8String());

    EXPECT_FALSE(options->GetActivityOptions()->HasActivityId());
    EXPECT_TRUE(options->GetActivityOptions()->GetActivityId().empty());

    client->SendUpdateFileRequest("https://test/foo", StubFile(), nullptr, options)->GetResult();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Mantas.Smicius                        11/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AzureBlobStorageClientTests, SendUpdateFileRequest_ActivityOptionsEnabledForMultiChunkUpload_SendsAllRequestsWithSpecifiedXMsClientRequestId)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());
    uint32_t chunkSize = 4 * 1024 * 1024; // 4MB

    GetHandler().ExpectRequests(4);
    GetHandler().ForAnyRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("specifiedActivityId", request.GetHeaders().GetValue(HEADER_XMsClientRequestId));
        return StubHttpResponse(HttpStatus::Created);
        });

    auto options = std::make_shared<IAzureBlobStorageClient::RequestOptions>();
    options->GetActivityOptions()->SetActivityId("specifiedActivityId");

    EXPECT_TRUE(options->GetActivityOptions()->HasActivityId());
    EXPECT_STREQ("specifiedActivityId", options->GetActivityOptions()->GetActivityId().c_str());

    uint32_t fileSize = chunkSize * 3 - 200; // 3 chunks
    BeFileName filePath = StubFileWithSize(fileSize);
    client->SendUpdateFileRequest("https://test/foo", filePath, nullptr, options)->GetResult();
    }
