/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/BeHttp/HttpRequestTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS
using namespace folly;

void HttpRequestTests::SetUpTestCase()
    {
    BeFileName path;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(path);
    HttpClient::Initialize(path);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, PerformAsync_BentleyTasksApiAndOneRequest_ExecutesSuccessfully)
    {
    Request request("http://httpbin.org/ip");

    Response response = request.PerformAsync()->GetResult();

    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, PerformAsync_OneRequestWithThen_ExecutesChainedTaskSuccessfully)
    {
    Response response;
    Request request("http://httpbin.org/ip");

    auto task = request.PerformAsync()->Then([&] (Response& finishedResponse)
        {
        response = finishedResponse;
        });

    task->Wait();

    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_OneRequestWithThen_ExecutesChainedTaskSuccessfully)
    {
    Response response;
    Request request("http://httpbin.org/ip");

    auto future = request.Perform().then([&] (Response finishedResponse)
        {
        response = finishedResponse;
        });

    future.wait();

    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
//TEST_F(HttpRequestTests, Perform_FiddlerProxySet_ExecutesSuccessfully)
//    {
//    HttpRequest request("http://httpbin.org/ip");
//    request.SetProxy("http://127.0.0.1:8888"); // Fiddler proxy
//
//    HttpResponse response = request.Perform().get();
//
//    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_CertValidationSetAndSiteHasValidCert_Success)
    {
    Request request("https://google.com/");
    request.SetValidateCertificate(true);

    Response response = request.Perform().get();

    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_CertValidationNotSetAndSiteHasValidCert_Success)
    {
    Request request("https://google.com/");
    request.SetValidateCertificate(false);

    Response response = request.Perform().get();

    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_CertValidationSetAndSiteHasInvalidCert_Fails)
    {
    Request request("https://qa-connect.bentley.com/");
    request.SetValidateCertificate(true);

    Response response = request.Perform().get();

    EXPECT_EQ(ConnectionStatus::CertificateError, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus::None, response.GetHttpStatus());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_CertValidationNotSetAndSiteHasInvalidCert_Succeeds)
    {
    Request request("https://qa-connect.bentley.com/");
    request.SetValidateCertificate(false);

    Response response = request.Perform().get();

    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_OneRequest_ExecutesSuccessfully)
    {
    // Simple test for deprecated method

    Request request("http://httpbin.org/ip");

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, PerformAsync_OneRequest_ExecutesSuccessfully)
    {
    Request request("http://httpbin.org/ip");

    Response response = request.PerformAsync()->GetResult();

    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_MovedRequest_ExecutesSuccessfully)
    {
    Request request(std::move(Request("http://httpbin.org/ip")));

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_UnsafeCharactersInUrl_EncodesUnsafeCharactersAndExecutesSuccessfully)
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

    Request request(R"(http://httpbin.org/ip?<>"#{}|\^[]`)");

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_STREQ("http://httpbin.org/ip?%3C%3E%22%23%7B%7D%7C%5C%5E%5B%5D%60", response.GetEffectiveUrl().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, GetUrl_UnsafeCharactersInUrl_ReturnsEncodedCharactersInUrl)
    {
    Request request(R"(http://httpbin.org/ip?<>"#{}|\^[]`)");

    EXPECT_STREQ("http://httpbin.org/ip?%3C%3E%22%23%7B%7D%7C%5C%5E%5B%5D%60", request.GetUrl().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, GetUrl_TextAsUrlPassed_ReturnsSameText)
    {
    Request request("test url");

    EXPECT_STREQ("test url", request.GetUrl().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_MalformedUrl_Fails)
    {
    BeTest::SetFailOnAssert(false);
    Request request("this is not url");

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::None, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::CouldNotConnect, response.GetConnectionStatus());
    BeTest::SetFailOnAssert(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_NonexistingUrl_ExecutesWithError)
    {
    Request request("http://aaaaaa.bentley.com");

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::None, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::CouldNotConnect, response.GetConnectionStatus());
    EXPECT_STREQ("", response.GetBody().AsString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_BasicAuthorizationCorrectCredentials_Success)
    {
    Request request("https://httpbin.org/basic-auth/user/pass");
    request.SetCredentials({"user", "pass"});

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_TRUE(Json::Reader::DoParse(response.GetBody().AsString()).isObject());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_BasicAuthorizationNoCredentials_Fails)
    {
    Request request("https://httpbin.org/basic-auth/user/pass");

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::Unauthorized, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_BasicAuthorizationIncorrectCredentials_Fails)
    {
    Request request("https://httpbin.org/basic-auth/user/pass");
    request.SetCredentials({"not", "correct"});

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::Unauthorized, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_FollowRedirectsTrue_RedirectsSuccessfully)
    {
    Request request("http://httpbin.org/redirect/1");
    request.SetFollowRedirects(true);

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_FolowRedirectsFalse_ReturnsWithFound)
    {
    Request request("http://httpbin.org/redirect/1");
    request.SetFollowRedirects(false);

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::Found, response.GetHttpStatus());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, PerformAsync_OneRequest_ExecutesSuccessfullyWithChainedTask)
    {
    Response response;
    Request request("http://httpbin.org/ip");

    auto task = request.PerformAsync()->Then(
        [&] (Http::Response& finishedResponse)
        {
        response = finishedResponse;
        });

    task->Wait();

    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, PerformAsync_ReusingSameResponseBodyWithData_ResetsResponseBodySoDataWouldNotBeMerged)
    {
    auto responseBody = HttpStringBody::Create("SomeData");
    responseBody->SetPosition(3);

    Request request("http://httpbin.org/bytes/2?seed=7");
    request.SetResponseBody(responseBody);

    Response response = request.Perform().get();
    EXPECT_EQ(responseBody.get(), &response.GetBody());
    EXPECT_EQ(2, responseBody->GetLength());
    EXPECT_EQ("R&", responseBody->AsString());

    request = Request("http://httpbin.org/bytes/1?seed=13");
    request.SetResponseBody(responseBody);

    response = request.Perform().get();
    EXPECT_EQ(responseBody.get(), &response.GetBody());
    EXPECT_EQ(1, responseBody->GetLength());
    EXPECT_EQ("B", responseBody->AsString());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_ReusingSameRequestBody_KeepsSameBody)
    {
    Request request("http://httpbin.org/post", "POST");

    request.SetRequestBody(HttpStringBody::Create("TestBody"));
    Response response = request.Perform().get();
    EXPECT_EQ(Json::Reader::DoParse(response.GetBody().AsString())["data"].asString(), "TestBody");

    response = request.Perform().get();
    EXPECT_EQ(Json::Reader::DoParse(response.GetBody().AsString())["data"].asString(), "TestBody");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_ManyRequests_ExecutesSuccessfully)
    {
    int testRequestCount = 10;

    std::vector<Future<Response>> futures;
    uint64_t start = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

    // Setup & Test
    for (int i = 0; i < testRequestCount; i++)
        {
        Request request("http://httpbin.org/ip");
        Future<Response> future = request.Perform().then([i] (Response response)
            {
            BeDebugLog(Utf8PrintfString("Finished running: %d", i).c_str());
            return response;
            });
        futures.push_back(std::move(future));
        }

    // Wait
    BeDebugLog("Waiting for all requests to finish");
    uint64_t mid = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
    // Could not get collectAll() compiling
    for (auto& future : futures)
        future.wait();
    uint64_t end = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

    // Assert
    for (auto& future : futures)
        {
        Response response = future.get();
        EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
        EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
        EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
        }

    BeDebugLog(Utf8PrintfString("Setupping requests took: %4llu ms. Waiting took: %4llu ms", mid - start, end - mid).c_str());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, PerformAsync_ManyRequests_ExecutesSuccessfully)
    {
    int testRequestCount = 10;

    BeMutex resultCS;
    std::vector<Response> responses;
    bset<std::shared_ptr<AsyncTask>> tasks;

    uint64_t start = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

    // Setup & Test
    for (int i = 0; i < testRequestCount; i++)
        {
        Request request("http://httpbin.org/ip");
        auto task = request.PerformAsync()->Then([&, i] (Response& response)
            {
            BeMutexHolder holder(resultCS);
            responses.push_back(response);
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
    for (Response response : responses)
        {
        EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
        EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
        EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
        }

    BeDebugLog(Utf8PrintfString("Setupping requests took: %4llu ms. Waiting took: %4llu ms", mid - start, end - mid).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_DeclareCompressedEncodingWithoutEnablingRequestCompression_BadRequestError)
    {
    Request request("https://bsw-wsg.bentley.com/bistro/v2.4/Repositories/", "POST");
    request.SetRequestBody(HttpStringBody::Create("TestBody"));

    request.GetHeaders().AddValue("Content-Encoding", "gzip");

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::BadRequest, response.GetHttpStatus());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_EnableRequestCompression_Success)
    {
    Request request("https://bsw-wsg.bentley.com/bistro/v2.4/Repositories/", "POST");
    request.SetRequestBody(HttpStringBody::Create("TestBody"));
    CompressionOptions options;
    options.EnableRequestCompression(true, 0);

    request.SetCompressionOptions(options);

    // Content-encoding should be added automaticly. 
    // But by forcing it we check whether request is trully compressed by api
    request.GetHeaders().AddValue("Content-Encoding", "gzip");

    Response response = request.Perform().get();

    //Request was sucesfull but POST method is not allowed
    EXPECT_EQ(HttpStatus::MethodNotAllowed, response.GetHttpStatus());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Vincas.Razma                           02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_EnableRequestCompressionForRequestWithType_Success)
    {
    Request request("https://bsw-wsg.bentley.com/bistro/v2.4/Repositories/", "POST");
    request.SetRequestBody(HttpStringBody::Create("TestBody"));
    CompressionOptions options;
    options.EnableRequestCompression(true, 0);

    request.GetHeaders().SetContentType(REQUESTHEADER_ContentType_ApplicationJson);
    request.SetCompressionOptions(options);

    // Content-encoding should be added automaticly. 
    // But by forcing it we check whether request is trully compressed by api
    request.GetHeaders().AddValue("Content-Encoding", "gzip");

    Response response = request.Perform().get();

    //Request was sucesfull but POST method is not allowed
    EXPECT_EQ(HttpStatus::MethodNotAllowed, response.GetHttpStatus());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Vincas.Razma                           02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_EnableRequestCompressionWithCompressionTypesSet_Success)
    {
    Request request("https://bsw-wsg.bentley.com/bistro/v2.4/Repositories/", "POST");
    request.SetRequestBody(HttpStringBody::Create("TestBody"));
    CompressionOptions options;
    options.EnableRequestCompression(true, 0);

    //Add supported type matches type of the request
    options.AddSupportedType(REQUESTHEADER_ContentType_ApplicationJson);
    request.GetHeaders().SetContentType(REQUESTHEADER_ContentType_ApplicationJson);
    request.SetCompressionOptions(options);

    // Content-encoding should be added automaticly. 
    // But by forcing it we check whether request is trully compressed by api
    request.GetHeaders().AddValue("Content-Encoding", "gzip");

    Response response = request.Perform().get();

    //Request was sucesfull but POST method is not allowed
    EXPECT_EQ(HttpStatus::MethodNotAllowed, response.GetHttpStatus());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Vincas.Razma                           02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_EnableRequestCompression_ContentEncodingHeaderAdded)
    {
    Request request("https://bsw-wsg.bentley.com/bistro/v2.4/Repositories/", "POST");
    request.SetRequestBody(HttpStringBody::Create("TestBody"));
    CompressionOptions options;
    options.EnableRequestCompression(true, 0);

    //Do not add Content-encoding header. It should be added automaticly.
    request.SetCompressionOptions(options);

    Response response = request.Perform().get();

    //Request was sucesfull but POST method is not allowed
    EXPECT_EQ(HttpStatus::MethodNotAllowed, response.GetHttpStatus());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Vincas.Razma                           02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_EnableRequestCompressionRequestNotCompressibleType_BadRequestError)
    {
    Request request("https://bsw-wsg.bentley.com/bistro/v2.4/Repositories/", "POST");
    request.SetRequestBody(HttpStringBody::Create("TestBody"));
    CompressionOptions options;
    options.EnableRequestCompression(true, 0);
    options.AddSupportedType(REQUESTHEADER_ContentType_ApplicationJson);
    request.GetHeaders().SetContentType(REQUESTHEADER_ContentType_ApplicationXml);
    request.SetCompressionOptions(options);

    //Force compression, thus checking if request is compressed by api
    request.GetHeaders().AddValue("Content-Encoding", "gzip");
    Response response = request.Perform().get();

    //Request was sucesfull but POST method is not allowed
    EXPECT_EQ(HttpStatus::BadRequest, response.GetHttpStatus());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_EnableRequestCompressionWithMinimalSizeLargerThanContentSize_Success)
    {
    Request request("https://bsw-wsg.bentley.com/bistro/v2.4/Repositories/", "POST");
    request.SetRequestBody(HttpStringBody::Create("TestBody"));
    CompressionOptions options;
    options.EnableRequestCompression(true, 7);

    //Add supported type matches type of the request
    options.AddSupportedType(REQUESTHEADER_ContentType_ApplicationJson);
    request.GetHeaders().SetContentType(REQUESTHEADER_ContentType_ApplicationJson);

    request.SetCompressionOptions(options);

    //Force compression, thus checking if request is compressed by api
    request.GetHeaders().AddValue("Content-Encoding", "gzip");
    Response response = request.Perform().get();
    //Request was sucesfull but POST method is not allowed
    EXPECT_EQ(HttpStatus::MethodNotAllowed, response.GetHttpStatus());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Julius.Cepukenas                       12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_EnableRequestCompressionWithMinimalSizeLargerThanContentSizeWithCompressedContentEncodingEnabled_BadRequestError)
    {
    Request request("https://bsw-wsg.bentley.com/bistro/v2.4/Repositories/", "POST");
    request.SetRequestBody(HttpStringBody::Create("TestBody"));
    CompressionOptions options;

    //Add supported type matches type of the request
    options.AddSupportedType(REQUESTHEADER_ContentType_ApplicationJson);
    request.GetHeaders().SetContentType(REQUESTHEADER_ContentType_ApplicationJson);
    options.EnableRequestCompression(true, 10);
    request.SetCompressionOptions(options);

    //Force compression, thus checking if request is compressed by api
    request.GetHeaders().AddValue("Content-Encoding", "gzip");

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::BadRequest, response.GetHttpStatus());
    }
