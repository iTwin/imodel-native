/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/BeHttp/HttpRequestTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "HttpRequestTests.h"

#include <Bentley/BeDebugLog.h>
#include <Bentley/BeTimeUtilities.h>
#include <BeHttp/HttpBody.h>
#include <BeHttp/HttpClient.h>
#include <BeHttp/HttpError.h>
#include <BeHttp/HttpStatus.h>
#include <Bentley/Bentley.h>
#include <Bentley/BeThread.h>
#include <Bentley/Bentley.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/Base64Utilities.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <BeJsonCpp/BeJsonUtilities.h>

USING_NAMESPACE_BENTLEY_TASKS

void HttpRequestTests::SetUpTestCase()
    {
    BeFileName path;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(path);
    HttpClient::Initialize(path);
    }

//TEST_F (HttpRequestTests, PerformAsync_FiddlerProxySet_ExecutesSuccessfully)
//    {
//    HttpRequest request ("http://httpbin.org/ip");
//    request.SetProxy ("http://127.0.0.1:8888"); // Fiddler proxy
//
//    HttpResponse response = request.PerformAsync ()->GetResult ();
//
//    EXPECT_EQ (HttpStatus::OK, response.GetHttpStatus ());
//    }

TEST_F(HttpRequestTests, PerformAsync_CertValidationSetAndSiteHasValidCert_Success)
    {
    Http::Request request("https://google.com/");
    request.SetValidateCertificate(true);

    Http::Response response = request.Perform();

    EXPECT_EQ(Http::ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ(Http::HttpStatus::OK, response.GetHttpStatus());
    }

TEST_F(HttpRequestTests, PerformAsync_CertValidationNotSetAndSiteHasValidCert_Success)
    {
    Http::Request request("https://google.com/");
    request.SetValidateCertificate(false);

    Http::Response response = request.Perform();

    EXPECT_EQ(Http::ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ(Http::HttpStatus::OK, response.GetHttpStatus());
    }

TEST_F(HttpRequestTests, PerformAsync_CertValidationSetAndSiteHasInvalidCert_Fails)
    {
    Http::Request request("https://viltest2-8.bentley.com/");
    request.SetValidateCertificate(true);

    Http::Response response = request.Perform();

    EXPECT_EQ(Http::ConnectionStatus::CertificateError, response.GetConnectionStatus());
    EXPECT_EQ(Http::HttpStatus::None, response.GetHttpStatus());
    }

TEST_F(HttpRequestTests, PerformAsync_CertValidationNotSetAndSiteHasInvalidCert_Succeeds)
    {
    Http::Request request("https://viltest2-8.bentley.com/");
    request.SetValidateCertificate(false);

    Http::Response response = request.Perform();

    EXPECT_EQ(Http::ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ(Http::HttpStatus::OK, response.GetHttpStatus());
    }

TEST_F(HttpRequestTests, Perform_OneRequest_ExecutesSuccessfully)
    {
    // Simple test for deprecated method

    Http::Request request("http://httpbin.org/ip");

    Http::Response response = request.Perform();

    EXPECT_EQ(Http::HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(Http::ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
    }

TEST_F(HttpRequestTests, PerformAsync_OneRequest_ExecutesSuccessfully)
    {
    Http::Request request("http://httpbin.org/ip");

    Http::Response response = request.PerformAsync()->GetResult();

    EXPECT_EQ(Http::HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(Http::ConnectionStatus::OK, response.GetConnectionStatus());
    //EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
    }

TEST_F(HttpRequestTests, PerformAsync_MovedRequest_ExecutesSuccessfully)
    {
    Http::Request request(std::move(Http::Request("http://httpbin.org/ip")));

    Http::Response response = request.PerformAsync()->GetResult();

    EXPECT_EQ(Http::HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(Http::ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
    }

TEST_F(HttpRequestTests, PerformAsync_UnsafeCharactersInUrl_EncodesUnsafeCharactersAndExecutesSuccessfully)
    {
    //2014. Folowing urls with special symbols tested on browsers (# and % not tested )
    //  query: http://httpbin.org/ip?<>"{}|\^~[]`
    //  url:   http://httpbin.org/ip<>"{}|\^~[]`
    //FireFox:
    //  query: http://httpbin.org/ip?%3C%3E%22{}|\^~[]%60 
    //  url:   http://httpbin.org/ip%3C%3E%22%7B%7D|%5C%5E~%5B%5D%60
    //InternetExplorer:
    //  query: http://httpbin.org/ip?<>"{}|\^~[]`
    //  url:   http://httpbin.org/ip%3C%3E%22%7B%7D%7C/%5E~[]%60
    //Chrome:
    //  query: http://httpbin.org/ip?%3C%3E%22{}|\^~[]` 
    //  url:   http://httpbin.org/ip%3C%3E%22%7B%7D%7C/%5E~[]%60
    //Casablanca :
    //  Allows only #, % and ~ from unsafe sumbols
    //CURL:
    //  Removes everything after #, does not encode anything. Some have special meaning - [], {} for using multiple urls in command line

    Http::Request request(R"(http://httpbin.org/ip?<>"#{}|\^[]`)");

    Http::Response response = request.PerformAsync()->GetResult();

    EXPECT_EQ(Http::HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(Http::ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_STREQ("http://httpbin.org/ip?%3C%3E%22%23%7B%7D%7C%5C%5E%5B%5D%60", response.GetEffectiveUrl().c_str());
    }

TEST_F(HttpRequestTests, GetUrl_UnsafeCharactersInUrl_ReturnsEncodedCharactersInUrl)
    {
    Http::Request request(R"(http://httpbin.org/ip?<>"#{}|\^[]`)");

    EXPECT_STREQ("http://httpbin.org/ip?%3C%3E%22%23%7B%7D%7C%5C%5E%5B%5D%60", request.GetUrl().c_str());
    }

TEST_F(HttpRequestTests, GetUrl_TextAsUrlPassed_ReturnsSameText)
    {
    Http::Request request("test url");

    EXPECT_STREQ("test url", request.GetUrl().c_str());
    }

TEST_F(HttpRequestTests, PerformAsync_MalformedUrl_Fails)
    {
    BeTest::SetFailOnAssert(false);
    Http::Request request("this is not url");

    Http::Response response = request.PerformAsync()->GetResult();

    EXPECT_EQ(Http::HttpStatus::None, response.GetHttpStatus());
    EXPECT_EQ(Http::ConnectionStatus::CouldNotConnect, response.GetConnectionStatus());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(HttpRequestTests, PerformAsync_NonexistingUrl_ExecutesWithError)
    {
    Http::Request request("http://aaaaaa.bentley.com");

    Http::Response response = request.PerformAsync()->GetResult();

    EXPECT_EQ(Http::HttpStatus::None, response.GetHttpStatus());
    EXPECT_EQ(Http::ConnectionStatus::CouldNotConnect, response.GetConnectionStatus());
    EXPECT_STREQ("", response.GetBody().AsString().c_str());
    }

TEST_F(HttpRequestTests, PerformAsync_BasicAuthorizationCorrectCredentials_Success)
    {
    Http::Request request("https://httpbin.org/basic-auth/user/pass");
    request.SetCredentials({"user", "pass"});

    Http::Response response = request.PerformAsync()->GetResult();

    EXPECT_EQ(Http::HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(Http::ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_TRUE(Json::Reader::DoParse(response.GetBody().AsString()).isObject());
    }

TEST_F(HttpRequestTests, PerformAsync_BasicAuthorizationNoCredentials_Fails)
    {
    Http::Request request("https://httpbin.org/basic-auth/user/pass");

    Http::Response response = request.PerformAsync()->GetResult();

    EXPECT_EQ(Http::HttpStatus::Unauthorized, response.GetHttpStatus());
    EXPECT_EQ(Http::ConnectionStatus::OK, response.GetConnectionStatus());
    }

TEST_F(HttpRequestTests, PerformAsync_BasicAuthorizationIncorrectCredentials_Fails)
    {
    Http::Request request("https://httpbin.org/basic-auth/user/pass");
    request.SetCredentials({"not", "correct"});

    Http::Response response = request.PerformAsync()->GetResult();

    EXPECT_EQ(Http::HttpStatus::Unauthorized, response.GetHttpStatus());
    EXPECT_EQ(Http::ConnectionStatus::OK, response.GetConnectionStatus());
    }

TEST_F(HttpRequestTests, PerformAsync_FollowRedirectsTrue_RedirectsSuccessfully)
    {
    Http::Request request("http://httpbin.org/redirect/1");
    request.SetFollowRedirects(true);

    Http::Response response = request.PerformAsync()->GetResult();

    EXPECT_EQ(Http::HttpStatus::OK, response.GetHttpStatus());
    }

TEST_F(HttpRequestTests, PerformAsync_FolowRedirectsFalse_ReturnsWithFound)
    {
    Http::Request request("http://httpbin.org/redirect/1");
    request.SetFollowRedirects(false);

    Http::Response response = request.PerformAsync()->GetResult();

    EXPECT_EQ(Http::HttpStatus::Found, response.GetHttpStatus());
    }

TEST_F(HttpRequestTests, PerformAsync_OneRequest_ExecutesSuccessfullyWithChainedTask)
    {
    Http::Response response;
    Http::Request request("http://httpbin.org/ip");

    auto task = request.PerformAsync()->Then(
        [&] (Http::Response& finishedResponse)
        {
        response = finishedResponse;
        });

    task->Wait();

    EXPECT_EQ(Http::HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(Http::ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
    }

TEST_F(HttpRequestTests, PerformAsync_ReusingSameResponseBodyWithData_ResetsResponseBodySoDataWouldNotBeMerged)
    {
    auto responseBody = Http::HttpStringBody::Create("SomeData");
    responseBody->SetPosition(3);

    Http::Request request("http://httpbin.org/bytes/2?seed=7");
    request.SetResponseBody(responseBody);

    Http::Response response = request.PerformAsync()->GetResult();
    EXPECT_EQ(responseBody.get(), &response.GetBody());
    EXPECT_EQ(2, responseBody->GetLength());
    EXPECT_EQ("R&", responseBody->AsString());

    request = Http::Request("http://httpbin.org/bytes/1?seed=13");
    request.SetResponseBody(responseBody);

    response = request.PerformAsync()->GetResult();
    EXPECT_EQ(responseBody.get(), &response.GetBody());
    EXPECT_EQ(1, responseBody->GetLength());
    EXPECT_EQ("B", responseBody->AsString());
    }

TEST_F(HttpRequestTests, PerformAsync_ReusingSameRequestBody_KeepsSameBody)
    {
    Http::Request request("http://httpbin.org/post", "POST");

    request.SetRequestBody(Http::HttpStringBody::Create("TestBody"));
    Http::Response response = request.PerformAsync()->GetResult();
    EXPECT_EQ(Json::Reader::DoParse(response.GetBody().AsString())["data"].asString(), "TestBody");

    response = request.PerformAsync()->GetResult();
    EXPECT_EQ(Json::Reader::DoParse(response.GetBody().AsString())["data"].asString(), "TestBody");
    }

TEST_F(HttpRequestTests, PerformAsync_ManyRequests_ExecutesSuccessfully)
    {
    int testRequestCount = 10;

    BeMutex resultCS;
    std::vector<Http::Response> responses;
    bset<std::shared_ptr<AsyncTask>> tasks;

    uint64_t start = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

    // Setup & Test
    for (int i = 0; i < testRequestCount; i++)
        {
        Http::Request request("http://httpbin.org/ip");
        auto task = request.PerformAsync()->Then([&, i] (Http::Response& finishedResponse)
            {
            BeMutexHolder holder(resultCS);
            responses.push_back(finishedResponse);
            BeDebugLog(Utf8PrintfString("Finished running: %d", i).c_str());
            });
        tasks.insert(task->Then([]
            {}));
        }

    // Wait
    BeDebugLog("Waiting for all requests to finish");

    uint64_t mid = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

    AsyncTask::WhenAll(tasks)->Wait();

    uint64_t end = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

    // Assert
    EXPECT_EQ(testRequestCount, responses.size());
    for (Http::Response& response : responses)
        {
        EXPECT_EQ(Http::HttpStatus::OK, response.GetHttpStatus());
        EXPECT_EQ(Http::ConnectionStatus::OK, response.GetConnectionStatus());
        EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
        }

    BeDebugLog(Utf8PrintfString("Setupping requests took: %4llu ms. Waiting took: %4llu ms", mid - start, end - mid).c_str());
    }

TEST_F(HttpRequestTests, PerformAsync_DeclareCompressedEncodingWithoutEnablingRequestCompression_BadRequestError)
    {
    Http::Request request("https://bsw-wsg.bentley.com/bistro/v2.4/Repositories/", "POST");
    request.SetRequestBody(Http::HttpStringBody::Create("TestBody"));

    request.GetHeaders().AddValue("Content-Encoding", "gzip");

    Http::Response response = request.PerformAsync()->GetResult();

    EXPECT_EQ(Http::HttpStatus::BadRequest, response.GetHttpStatus());
    }

TEST_F(HttpRequestTests, PerformAsync_EnableRequestCompression_Success)
    {
    Http::Request request("https://bsw-wsg.bentley.com/bistro/v2.4/Repositories/", "POST");
    request.SetRequestBody(Http::HttpStringBody::Create("TestBody"));
    CompressionOptions options;
    options.EnableRequestCompression(true, 0);
    request.SetCompressionOptions(options);

    Http::Response response = request.PerformAsync()->GetResult();

    //Request was sucesfull but POST method is not allowed
    EXPECT_EQ(Http::HttpStatus::MethodNotAllowed, response.GetHttpStatus());
    }

TEST_F(HttpRequestTests, PerformAsync_EnableRequestCompressionWithMinimalSizeLargerThanContentSize_Success)
    {
    Http::Request request("https://bsw-wsg.bentley.com/bistro/v2.4/Repositories/", "POST");
    request.SetRequestBody(Http::HttpStringBody::Create("TestBody"));
    CompressionOptions options;
    options.EnableRequestCompression(true, 10);
    request.SetCompressionOptions(options);

    Http::Response response = request.PerformAsync()->GetResult();
    //Request was sucesfull but POST method is not allowed
    EXPECT_EQ(Http::HttpStatus::MethodNotAllowed, response.GetHttpStatus());
    }

TEST_F(HttpRequestTests, PerformAsync_EnableRequestCompressionWithMinimalSizeLargerThanContentSizeWithCompressedContentEncodingEnabled_BadRequestError)
    {
    Http::Request request("https://bsw-wsg.bentley.com/bistro/v2.4/Repositories/", "POST");
    request.SetRequestBody(Http::HttpStringBody::Create("TestBody"));
    CompressionOptions options;
    options.EnableRequestCompression(true, 10);
    request.SetCompressionOptions(options);

    request.GetHeaders().AddValue("Content-Encoding", "gzip");

    Http::Response response = request.PerformAsync()->GetResult();
    EXPECT_EQ(Http::HttpStatus::BadRequest, response.GetHttpStatus());
    }
