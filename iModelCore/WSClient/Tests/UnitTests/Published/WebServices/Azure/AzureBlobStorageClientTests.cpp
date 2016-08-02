/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Azure/AzureBlobStorageClientTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "AzureBlobStorageClientTests.h"

#include <WebServices/Azure/AzureBlobStorageClient.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

TEST_F(AzureBlobStorageClientTests, SendGetFileRequest_ResponseIsOK_ReturnsSuccess)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());

    BeFileName fileName = StubFilePath();

    GetHandler().ExpectRequests(1);
    GetHandler().ForFirstRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://myaccount.blob.core.windows.net/sascontainer/sasblob.txt?SAS", request.GetUrl().c_str());

        HttpFileBodyPtr httpFileBody = dynamic_cast<HttpFileBody*> (request.GetResponseBody().get());
        ASSERT_TRUE(httpFileBody.IsValid());
        EXPECT_STREQ(fileName, httpFileBody->GetFilePath());

        return StubHttpResponse(HttpStatus::OK);
        });

    auto response = client->SendGetFileRequest("https://myaccount.blob.core.windows.net/sascontainer/sasblob.txt?SAS", fileName)->GetResult();
    EXPECT_TRUE(response.IsSuccess());
    }

TEST_F(AzureBlobStorageClientTests, SendUpdateFileRequest_ResponseIsOK_ReturnsSuccess)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());
    uint32_t chunkSize = 4 * 1024 * 1024; // 4MB

    GetHandler()
        .ExpectRequests(4)
        .ForRequest(1, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("SASUrl&comp=block&blockid=MDAwMDA=", request.GetUrl().c_str());
        EXPECT_EQ(chunkSize, request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::OK);
        })
        .ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("SASUrl&comp=block&blockid=MDAwMDE=", request.GetUrl().c_str());
        EXPECT_EQ(chunkSize, request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::OK);
        })
        .ForRequest(3, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("SASUrl&comp=block&blockid=MDAwMDI=", request.GetUrl().c_str());
        EXPECT_EQ(chunkSize - 200, request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::OK);
        })
        .ForRequest(4, [] (Http::RequestCR request)
        {
        EXPECT_STREQ("SASUrl&comp=blocklist", request.GetUrl().c_str());
        EXPECT_STREQ("<?xml version=\"1.0\" encoding=\"utf-8\"?><BlockList><Latest>MDAwMDA=</Latest><Latest>MDAwMDE=</Latest><Latest>MDAwMDI=</Latest></BlockList>", request.GetRequestBody()->AsString().c_str());
        return StubHttpResponse(HttpStatus::OK);
        });

    BeFileName fileName = StubFileWithSize(chunkSize * 3 - 200); // two 4MB chunks and one smaller.

    EXPECT_TRUE(client->SendUpdateFileRequest("SASUrl", fileName)->GetResult().IsSuccess());
    }

TEST_F(AzureBlobStorageClientTests, SendUpdateFileRequest_ResponseIsBadRequest_ReturnsError)
    {
    auto client = AzureBlobStorageClient::Create(GetHandlerPtr());
    uint32_t chunkSize = 4 * 1024 * 1024; // 4MB

    GetHandler()
        .ExpectRequests(2)
        .ForRequest(1, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("SASUrl&comp=block&blockid=MDAwMDA=", request.GetUrl().c_str());
        EXPECT_EQ(chunkSize, request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::OK);
        })
        .ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("SASUrl&comp=block&blockid=MDAwMDE=", request.GetUrl().c_str());
        EXPECT_EQ(chunkSize, request.GetRequestBody()->GetLength());
        return StubHttpResponse(HttpStatus::BadRequest);
        });

    BeFileName fileName = StubFileWithSize(chunkSize * 3 - 200); // two 4MB chunks and one smaller.

    EXPECT_FALSE(client->SendUpdateFileRequest("SASUrl", fileName)->GetResult().IsSuccess());
    }
