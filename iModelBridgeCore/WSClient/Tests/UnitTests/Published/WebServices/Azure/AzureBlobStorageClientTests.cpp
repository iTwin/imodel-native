/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Azure/AzureBlobStorageClientTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "AzureBlobStorageClientTests.h"

#include <WebServices/Azure/AzureBlobStorageClient.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

TEST_F(AzureBlobStorageClientTests, SendGetFileRequest_ServerReturnsError_ReturnsErrorResponse)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForFirstRequest([=] (HttpRequestCR request)
        {
        return StubHttpResponse(HttpStatus::BadRequest, "TestError");
        });

    auto result = client->SendGetFileRequest("https://myaccount.blob.core.windows.net/sascontainer/sasblob.txt?SAS", StubFilePath())->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ("TestError", result.GetError().GetBody().AsString());
    }

TEST_F(AzureBlobStorageClientTests, SendGetFileRequest_ResponseIsOK_ReturnsSuccess)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());

    BeFileName filePath = StubFilePath();

    GetHandler().ExpectRequests(1);
    GetHandler().ForFirstRequest([=] (HttpRequestCR request)
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

TEST_F(AzureBlobStorageClientTests, SendUpdateFileRequest_ServerReturnsError_ReturnsErrorResponse)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (HttpRequestCR request)
        {
        return StubHttpResponse(HttpStatus::BadRequest, "TestError");
        });

    auto result = client->SendUpdateFileRequest("SASUrl", StubFile())->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ("TestError", result.GetError().GetBody().AsString());
    }

TEST_F(AzureBlobStorageClientTests, SendUpdateFileRequest_ResponseIsOK_ReturnsSuccess)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());
    uint32_t chunkSize = 4 * 1024 * 1024; // 4MB


    GetHandler().ExpectRequests(4);
    GetHandler().ForRequest(1, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("SASUrl&comp=block&blockid=MDAwMDA=", request.GetUrl().c_str());
        EXPECT_EQ(chunkSize, request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::Created);
        });
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("SASUrl&comp=block&blockid=MDAwMDE=", request.GetUrl().c_str());
        EXPECT_EQ(chunkSize, request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::Created);
        });
    GetHandler().ForRequest(3, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("SASUrl&comp=block&blockid=MDAwMDI=", request.GetUrl().c_str());
        EXPECT_EQ(chunkSize - 200, request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::Created);
        });
    GetHandler().ForRequest(4, [] (HttpRequestCR request)
        {
        EXPECT_STREQ("SASUrl&comp=blocklist", request.GetUrl().c_str());
        EXPECT_STREQ("<?xml version=\"1.0\" encoding=\"utf-8\"?><BlockList><Latest>MDAwMDA=</Latest><Latest>MDAwMDE=</Latest><Latest>MDAwMDI=</Latest></BlockList>", request.GetRequestBody()->AsString().c_str());
        return StubHttpResponse(HttpStatus::Created, "", {{"ETag", "FooBoo"}});
        });

    BeFileName filePath = StubFileWithSize(chunkSize * 3 - 200); // two 4MB chunks and one smaller.
    auto result = client->SendUpdateFileRequest("SASUrl", filePath)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ("FooBoo", result.GetValue().GetETag());
    }

TEST_F(AzureBlobStorageClientTests, SendUpdateFileRequest_ProgressCallbackPassedForMultiChunkUpload_ProgressReported)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());
    uint32_t chunkSize = 4 * 1024 * 1024; // 4MB

    GetHandler().ExpectRequests(4);
    GetHandler().ForRequest(1, [=] (HttpRequestCR request)
        {
        request.GetUploadProgressCallback()(1010, (double)request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::Created);
        });
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        request.GetUploadProgressCallback()(1020, (double) request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::Created);
        });
    GetHandler().ForRequest(3, [=] (HttpRequestCR request)
        {
        request.GetUploadProgressCallback()(1030, (double) request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::Created);
        });
    GetHandler().ForRequest(4, [] (HttpRequestCR request)
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
    auto result = client->SendUpdateFileRequest("SASUrl", filePath, onProgress)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    }

TEST_F(AzureBlobStorageClientTests, SendUpdateFileRequest_ResponseIsBadRequest_ReturnsError)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());
    uint32_t chunkSize = 4 * 1024 * 1024; // 4MB

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("SASUrl&comp=block&blockid=MDAwMDA=", request.GetUrl().c_str());
        EXPECT_EQ(chunkSize, request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::OK);
        });
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("SASUrl&comp=block&blockid=MDAwMDE=", request.GetUrl().c_str());
        EXPECT_EQ(chunkSize, request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::BadRequest);
        });

    BeFileName filePath = StubFileWithSize(chunkSize * 3 - 200); // two 4MB chunks and one smaller.
    auto result = client->SendUpdateFileRequest("SASUrl", filePath)->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    }

TEST_F(AzureBlobStorageClientTests, SendUpdateFileRequest_FileSmallerThanChunk_UploadsWithOneRequest)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());
    uint32_t fileSize = 200;

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("TestUrl", request.GetUrl().c_str());
        EXPECT_EQ(fileSize, request.GetRequestBody()->GetLength());
        EXPECT_STREQ("BlockBlob", request.GetHeaders().GetValue("x-ms-blob-type"));
        return StubHttpResponse(HttpStatus::Created, "", {{"ETag", "FooBoo"}});
        });

    BeFileName filePath = StubFileWithSize(fileSize);
    auto result = client->SendUpdateFileRequest("TestUrl", filePath)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_EQ("FooBoo", result.GetValue().GetETag());
    }

