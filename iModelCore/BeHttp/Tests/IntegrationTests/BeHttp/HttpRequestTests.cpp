/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/BeHttp/HttpRequestTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "Tests.h"

#include <Bentley/BeDebugLog.h>
#include <Bentley/BeTimeUtilities.h>
#include <BeHttp/HttpBody.h>
#include <BeHttp/HttpProxy.h>
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

#include "../../../BeHttp/Backdoor.h"
#include "../../UnitTests/Published/FSTest.h"
#include "../Scripts/ScriptRunner.h"
#include "AsyncTestCheckpoint.h"

#define WAITTIMEOUT 30000
#define COMPRESSION_OPTIONS_URL "https://mobilevm2.bentley.com/ws250/v2.5/Repositories"

using namespace ::testing;
using namespace folly;
USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

struct HttpRequestTests : ::testing::Test
    {
    static void SetUpTestCase()
        {
        BeFileName path;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(path);
        HttpClient::Initialize(path);
        }
    void Reset()
        {
        Backdoor::InitStartBackgroundTask([] (Utf8CP name, std::function<void()> task, std::function<void()> onExpired) {});
        Backdoor::CallOnApplicationSentToForeground();
        NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_HTTP, NativeLogging::LOG_WARNING);
        HttpProxy::SetDefaultProxy(HttpProxy());
        putenv("http_proxy=");
        putenv("https_proxy=");
        }
    void SetUp()
        {
        Reset();
        // Enable full logging with LOG_TRACE if needed
        //NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_MOBILEDGN_UTILS_HTTP, NativeLogging::LOG_TRACE);
        }
    void TearDown()
        {
        Reset();
        }
    };

/*--------------------------------------------------------------------------------------+
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

/*--------------------------------------------------------------------------------------+
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

/*--------------------------------------------------------------------------------------+
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_CertValidationSetAndSiteHasValidCert_Success)
    {
    Request request("https://httpbin.org/ip");
    request.SetValidateCertificate(true);

    Response response = request.Perform().get();

    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_CertValidationNotSetAndSiteHasValidCert_Success)
    {
    Request request("https://httpbin.org/ip");
    request.SetValidateCertificate(false);

    Response response = request.Perform().get();

    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_CertValidationSetAndSiteHasSelfSignedCert_Fails)
    {
    Request request("https://self-signed.badssl.com/");
    request.SetValidateCertificate(true);

    Response response = request.Perform().get();

    EXPECT_EQ(ConnectionStatus::CertificateError, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus::None, response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_CertValidationNotSetAndSiteHasSelfSignedCert_Succeeds)
    {
    Request request("https://self-signed.badssl.com/");
    request.SetValidateCertificate(false);

    Response response = request.Perform().get();

    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_CertValidationSetAndSiteHasCertIssuedToDifferentSite_Fails)
    {
    Request request("https://wrong.host.badssl.com/");
    request.SetValidateCertificate(true);

    Response response = request.Perform().get();

    EXPECT_EQ(ConnectionStatus::CertificateError, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus::None, response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_CertValidationNotSetAndSiteHasCertIssuedToDifferentSite_Succeeds)
    {
    Request request("https://wrong.host.badssl.com/");
    request.SetValidateCertificate(false);

    Response response = request.Perform().get();

    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
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


/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_MovedRequest_ExecutesSuccessfully)
    {
    Request request(std::move(Request("http://httpbin.org/ip")));

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, GetUrl_UnsafeCharactersInUrl_ReturnsEncodedCharactersInUrl)
    {
    Request request(R"(http://httpbin.org/ip?<>"#{}|\^[]`)");

    EXPECT_STREQ("http://httpbin.org/ip?%3C%3E%22%23%7B%7D%7C%5C%5E%5B%5D%60", request.GetUrl().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, GetUrl_TextAsUrlPassed_ReturnsSameText)
    {
    Request request("test url");

    EXPECT_STREQ("test url", request.GetUrl().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_NonexistingUrl_ExecutesWithError)
    {
    Request request("http://aaaaaa.bentley.com");

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::None, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::CouldNotConnect, response.GetConnectionStatus());
    EXPECT_STREQ("", response.GetBody().AsString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_BasicAuthorizationNoCredentials_Fails)
    {
    Request request("https://httpbin.org/basic-auth/user/pass");

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::Unauthorized, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_BasicAuthorizationIncorrectCredentials_Fails)
    {
    Request request("https://httpbin.org/basic-auth/user/pass");
    request.SetCredentials({"not", "correct"});

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::Unauthorized, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_FollowRedirectsTrue_RedirectsSuccessfully)
    {
    Request request("http://httpbin.org/redirect/1");
    request.SetFollowRedirects(true);

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_FolowRedirectsFalse_ReturnsWithFound)
    {
    Request request("http://httpbin.org/redirect/1");
    request.SetFollowRedirects(false);

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::Found, response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_DefaultHttpRequestAndReceivedBody_SucceedsWithBody)
    {
    Request request("http://httpbin.org/bytes/2?seed=7");

    Response response = request.Perform().get();
    ASSERT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_TRUE(response.GetContent()->GetBody().IsValid());
    EXPECT_EQ("R&", response.GetContent()->GetBody()->AsString());
    EXPECT_EQ(2, response.GetContent()->GetBody()->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_HttpRequestWithNullResponseBodyAndReceivedBody_SucceedsWithEmptyBody)
    {
    Request request("http://httpbin.org/bytes/2?seed=7");
    request.SetResponseBody(nullptr);

    Response response = request.Perform().get();
    ASSERT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_TRUE(response.GetContent()->GetBody().IsValid());
    EXPECT_EQ("", response.GetContent()->GetBody()->AsString());
    EXPECT_EQ(0, response.GetContent()->GetBody()->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_ReusingSameResponseBodyWithData_ResetsResponseBodySoDataWouldNotBeMerged)
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_ZeroByteBodyAndHttpFileBody_SavesZeroSizedFile)
    {
    auto path = FSTest::StubFilePath();
    auto responseBody = HttpFileBody::Create(path);

    Request request("http://httpbin.org/bytes/0");
    request.SetResponseBody(responseBody);

    EXPECT_FALSE(path.DoesPathExist());

    Response response = request.Perform().get();
    ASSERT_EQ(HttpStatus::OK, response.GetHttpStatus());

    ASSERT_TRUE(path.DoesPathExist());

    BeFile file;
    uint64_t size = 0;
    ASSERT_EQ(BeFileStatus::Success, file.Open(path.GetNameUtf8(), BeFileAccess::Read));
    ASSERT_EQ(BeFileStatus::Success, file.GetSize(size));
    EXPECT_EQ(0, size);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
// Disabled until httpbin.org fix is pushed, more info: TFS#633418
TEST_F(HttpRequestTests, DISABLED_Perform_SlowConnectionWithResumableDownload_DownloadsDataInMultipleRequests)
    {
    auto path = FSTest::StubFilePath();
    auto responseBody = HttpFileBody::Create(path);

    Request request("http://httpbin.org/range/10?duration=20&chunk_size=5");
    request.SetResponseBody(responseBody);
    request.SetTransferTimeoutSeconds(2);
    request.SetRetryOptions(Request::RetryOption::ResumeTransfer, 100);

    Response response = request.Perform().get();
    ASSERT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());

    EXPECT_EQ(10, response.GetContent()->GetBody()->GetLength());
    EXPECT_EQ("abcdefghij", response.GetContent()->GetBody()->AsString());
    EXPECT_EQ("abcdefghij", FSTest::ReadFile(path));
    }

/*--------------------------------------------------------------------------------------+
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
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
        auto task = request.PerformAsync()->Then([&, i] (Response& finishedResponse)
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
    for (Response& response : responses)
        {
        EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
        EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
        EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
        }

    BeDebugLog(Utf8PrintfString("Setupping requests took: %4llu ms. Waiting took: %4llu ms", mid - start, end - mid).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_DeclareCompressedEncodingWithoutEnablingRequestCompression_BadRequestError)
    {
    Request request(COMPRESSION_OPTIONS_URL, "POST");
    request.SetRequestBody(HttpStringBody::Create("TestBody"));

    request.GetHeaders().AddValue("Content-Encoding", "gzip");

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::BadRequest, response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_EnableRequestCompression_Success)
    {
    Request request(COMPRESSION_OPTIONS_URL, "POST");
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_EnableRequestCompressionForRequestWithType_Success)
    {
    Request request(COMPRESSION_OPTIONS_URL, "POST");
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_EnableRequestCompressionWithCompressionTypesSet_Success)
    {
    Request request(COMPRESSION_OPTIONS_URL, "POST");
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_EnableRequestCompression_ContentEncodingHeaderAdded)
    {
    Request request(COMPRESSION_OPTIONS_URL, "POST");
    request.SetRequestBody(HttpStringBody::Create("TestBody"));
    CompressionOptions options;
    options.EnableRequestCompression(true, 0);

    //Do not add Content-encoding header. It should be added automaticly.
    request.SetCompressionOptions(options);

    Response response = request.Perform().get();

    //Request was sucesfull but POST method is not allowed
    EXPECT_EQ(HttpStatus::MethodNotAllowed, response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_EnableRequestCompressionRequestNotCompressibleType_BadRequestError)
    {
    Request request(COMPRESSION_OPTIONS_URL, "POST");
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_EnableRequestCompressionWithMinimalSizeLargerThanContentSize_Success)
    {
    Request request(COMPRESSION_OPTIONS_URL, "POST");
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_EnableRequestCompressionWithMinimalSizeLargerThanContentSizeWithCompressedContentEncodingEnabled_BadRequestError)
    {
    Request request(COMPRESSION_OPTIONS_URL, "POST");
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

struct MethodParam
    {
    Utf8String url;
    Utf8String method;
    Utf8String requestBody;
    Request CreateRequest() const
        {
        Request request(url, method);
        if (!requestBody.empty())
            request.SetRequestBody(HttpStringBody::Create(requestBody));
        return request;
        }
    };

std::ostream& operator<<(std::ostream& os, const MethodParam& value)
    {
    os << value.url << "," << value.method << "," << Utf8PrintfString("%.8s(%d)", value.requestBody.c_str(), value.requestBody.size());
    return os;
    }

// GET vs POST/PUT/etc has different implementations
struct HttpRequestTestsMethods : HttpRequestTests, WithParamInterface<MethodParam> {};
INSTANTIATE_TEST_CASE_P(DifferentMethods, HttpRequestTestsMethods, Values(
    MethodParam {"http://httpbin.org/ip", "GET", ""},
    MethodParam {"http://httpbin.org/post", "POST", Utf8String(32 * 1024, 'x')},
    MethodParam {"http://httpbin.org/put", "PUT", Utf8String(32 * 1024, 'x')},
    MethodParam {"http://httpbin.org/delete", "DELETE", Utf8String(32 * 1024, 'x')}
));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HttpRequestTestsMethods, Perform_BeforeMovedToBackground_RequestFinishesInBackground)
    {
    Request request = GetParam().CreateRequest();

    AsyncTestCheckpoint checkpoint;
    request.SetDownloadProgressCallback([&] (double, double)
        {
        checkpoint.CheckinAndWait(WAITTIMEOUT);
        });

    auto task = request.PerformAsync();
    checkpoint.WaitUntilReached(WAITTIMEOUT);

    Backdoor::CallOnApplicationSentToBackground();
    EXPECT_FALSE(task->IsCompleted());
    checkpoint.Continue();

    Response response = task->GetResult();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HttpRequestTestsMethods, Perform_WhenMovedToBackground_RequestDoesNothingUntilInForeground)
    {
    Request request = GetParam().CreateRequest();

    auto body = HttpStringBody::Create();
    request.SetResponseBody(body);

    AsyncTestCheckpoint checkpoint;
    request.SetDownloadProgressCallback([&] (double, double)
        {
        checkpoint.Checkin();
        });

    Backdoor::CallOnApplicationSentToBackground();

    auto task = request.PerformAsync();
    BeThreadUtilities::BeSleep(1000);
    EXPECT_FALSE(checkpoint.WasReached());
    EXPECT_FALSE(task->IsCompleted());
    EXPECT_EQ(0, body->GetLength());

    Backdoor::CallOnApplicationSentToForeground();

    Response response = task->GetResult();
    EXPECT_TRUE(checkpoint.WasReached());
    EXPECT_TRUE(task->IsCompleted());
    EXPECT_NE(0, body->GetLength());
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HttpRequestTestsMethods, Perform_AfterMovedFromBackgroundToForeground_ExecutesRequestAsUsual)
    {
    Request request = GetParam().CreateRequest();

    AsyncTestCheckpoint checkpoint;
    request.SetDownloadProgressCallback([&] (double, double)
        {
        checkpoint.CheckinAndWait(WAITTIMEOUT);
        });

    auto task = request.PerformAsync();
    checkpoint.WaitUntilReached(WAITTIMEOUT);
    Backdoor::CallOnApplicationSentToBackground();
    checkpoint.Continue();
    EXPECT_EQ(HttpStatus::OK, task->GetResult().GetHttpStatus());
    Backdoor::CallOnApplicationSentToForeground();

    EXPECT_EQ(HttpStatus::OK, request.Perform().get().GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HttpRequestTestsMethods, Perform_ExecutingInBackgroundWithDownloadAndUploadProgressCallbacks_IntermediateProgressIsNotReported)
    {
    Request request = GetParam().CreateRequest();

    AsyncTestCheckpoint checkpoint;
    request.SetUploadProgressCallback([&] (double current, double total)
        {
        if (checkpoint.CanContinue())
            EXPECT_TRUE(current == total) << current << "/" << total;
        checkpoint.CheckinAndWait(WAITTIMEOUT);
        });
    request.SetDownloadProgressCallback([&] (double current, double total)
        {
        if (checkpoint.CanContinue())
            EXPECT_TRUE(current == total) << current << "/" << total;
        checkpoint.CheckinAndWait(WAITTIMEOUT);
        });

    auto task = request.PerformAsync();
    checkpoint.WaitUntilReached(WAITTIMEOUT);

    Backdoor::CallOnApplicationSentToBackground();
    EXPECT_FALSE(task->IsCompleted());
    checkpoint.Continue();

    Response response = task->GetResult();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HttpRequestTestsMethods, Perform_NonRetryaleRequestAndBackgroundTimeExpiredWhenRunning_RequestReturnsError)
    {
    std::function<void()> onExpired;
    Backdoor::InitStartBackgroundTask([&] (Utf8CP, std::function<void()>, std::function<void()> _onExpired)
        {
        EXPECT_TRUE(nullptr == onExpired);
        EXPECT_TRUE(nullptr != _onExpired);
        onExpired = _onExpired;
        });

    Request request = GetParam().CreateRequest();

    auto body = HttpStringBody::Create();
    request.SetResponseBody(body);
    request.SetRetryOptions(Request::RetryOption::DontRetry); // Default

    AsyncTestCheckpoint checkpoint;
    request.SetDownloadProgressCallback([&] (double, double)
        {
        checkpoint.CheckinAndWait(WAITTIMEOUT);
        });

    auto task = request.PerformAsync();
    checkpoint.WaitUntilReached(WAITTIMEOUT);

    Backdoor::CallOnApplicationSentToBackground();
    EXPECT_FALSE(task->IsCompleted());
    onExpired();
    BeThreadUtilities::BeSleep(1000);
    checkpoint.Continue();

    Response response = task->GetResult();
    EXPECT_EQ(ConnectionStatus::ConnectionLost, response.GetConnectionStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HttpRequestTestsMethods, Perform_RetryaleRequestAndBackgroundTimeExpiredWhenRunning_RequestRestartedWhenInForeground)
    {
    std::function<void()> onExpired;
    Backdoor::InitStartBackgroundTask([&] (Utf8CP, std::function<void()>, std::function<void()> _onExpired)
        {
        EXPECT_TRUE(nullptr == onExpired);
        EXPECT_TRUE(nullptr != _onExpired);
        onExpired = _onExpired;
        });

    Request request = GetParam().CreateRequest();

    auto body = HttpStringBody::Create();
    request.SetResponseBody(body);
    request.SetRetryOptions(Request::RetryOption::ResetTransfer);

    AsyncTestCheckpoint checkpoint;
    request.SetDownloadProgressCallback([&] (double, double)
        {
        checkpoint.CheckinAndWait(WAITTIMEOUT);
        });

    auto task = request.PerformAsync();
    checkpoint.WaitUntilReached(WAITTIMEOUT);

    Backdoor::CallOnApplicationSentToBackground();
    EXPECT_FALSE(task->IsCompleted());
    onExpired();
    BeThreadUtilities::BeSleep(1000);
    checkpoint.Continue();
    BeThreadUtilities::BeSleep(1000);
    EXPECT_FALSE(task->IsCompleted());

    Backdoor::CallOnApplicationSentToForeground();

    Response response = task->GetResult();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }

// Proof of concept tests using local server.
// Keeping HttpRequestTestsProxy isolated and communicating trough python file server.
// TODO: proxy with credentials tests (proxy2.py does not support that, so needs to be extended)
// TOOD: HttpRequestTestsProxy would need to be improved in future - run on iOS/Android in addition to Windows.
// TODO: Somehow test HttpProxy::GetSystemProxy(), possibly modifying settings with scripts
// TODO: Do not restart proxy or file servers and instead download latest log files and extract changes.
// TODO: Use different proxy servers to test what proxies are picked up
// TODO: PAC scripts are cached on windows, need some way to clear cache or disable it

#define LOCAL_SERVER_PORT       "9990"
#define LOCAL_SERVER_URL        "http://localhost:" LOCAL_SERVER_PORT   // Hosted by file-server.bat
#define LOCAL_PROXY_PORT        "9991"
#define LOCAL_PROXY_URL         "http://localhost:" LOCAL_PROXY_PORT    // Hosted by proxy-server.bat
#define NONEXISTING_PROXY_URL   "http://fake.bentley.com"

struct HttpRequestTestsProxy : HttpRequestTests
    {
    void SetUp() override
        {
        HttpRequestTests::SetUp();

        ASSERT_EQ(ConnectionStatus::CouldNotConnect, Request(LOCAL_SERVER_URL).Perform().get().GetConnectionStatus())
            << "Different server process is still running due to previous tests existing unexpectidely. Close existing processes";

        ScriptRunner::RunScriptAsync("file-server.bat");
        ScriptRunner::RunScriptAsync("proxy-server.bat");

        WaitUntilProxyServerIsRunning();
        }
    void TearDown() override
        {
        ScriptRunner::StopAllPythonScripts();
        HttpRequestTests::TearDown();
        }
    static void WaitUntilProxyServerIsRunning()
        {
        while (HttpStatus::OK != Request(LOCAL_SERVER_URL "/Logs/proxy-server.log").Perform().get().GetHttpStatus())
            {
            BeThreadUtilities::BeSleep(100);
            }
        }
    static Utf8String GetLocalProxyLog()
        {
        Request request(LOCAL_SERVER_URL "/Logs/proxy-server.log");
        request.SetProxy(LOCAL_PROXY_URL); // Override proxy that could be picked up from test body
        Response response = request.Perform().get();
        EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
        return response.GetBody().AsString();
        }
    static void VerifyProxyLogEmpty()
        {
        EXPECT_EQ("Serving HTTP Proxy on ::1 port " LOCAL_PROXY_PORT " ...\n", GetLocalProxyLog());
        }
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_LocalServerFileDownload_Downloads)
    {
    Request request(LOCAL_SERVER_URL "/Data/Test.txt");

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ("TestData", response.GetBody().AsString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_ProxyUrlSetToNotExisting_CouldNotResolveProxy)
    {
    VerifyProxyLogEmpty();

    Request request("http://httpbin.org/ip");
    request.SetProxy(NONEXISTING_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(ConnectionStatus::CouldNotResolveProxy, response.GetConnectionStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), Not(HasSubstr("GET http://httpbin.org/ip")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_ProxyUrlSet_ExecutesViaProxy)
    {
    VerifyProxyLogEmpty();

    Request request("http://httpbin.org/ip");
    request.SetProxy(LOCAL_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), HasSubstr("GET http://httpbin.org/ip"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_DefaultProxyUrlSet_ExecutesViaProxy)
    {
    VerifyProxyLogEmpty();

    Request request("http://httpbin.org/ip");
    HttpProxy::SetDefaultProxy(HttpProxy(LOCAL_PROXY_URL));

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), HasSubstr("GET http://httpbin.org/ip"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_DefaultProxyPacUrlSet_ExecutesViaProxy)
    {
    VerifyProxyLogEmpty();

    Request request("http://httpbin.org/ip");
    HttpProxy proxy;
    proxy.SetPacUrl(LOCAL_SERVER_URL "/Data/pac1.js");
    HttpProxy::SetDefaultProxy(proxy);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), HasSubstr("GET http://httpbin.org/ip"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_DefaultProxyPacUrlSetWithDirect_ExecutesDirectly)
    {
    VerifyProxyLogEmpty();

    Request request("http://httpbin.org/ip");
    HttpProxy proxy;
    proxy.SetPacUrl(LOCAL_SERVER_URL "/Data/pac2.js");
    HttpProxy::SetDefaultProxy(proxy);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), Not(HasSubstr("GET http://httpbin.org/ip")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_DefaultProxyPacUrlSetWithSecondProxyReachable_ExecutesViaProxy)
    {
    VerifyProxyLogEmpty();

    Request request("http://httpbin.org/ip");
    HttpProxy proxy;
    proxy.SetPacUrl(LOCAL_SERVER_URL "/Data/pac3.js");
    HttpProxy::SetDefaultProxy(proxy);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), HasSubstr("GET http://httpbin.org/ip"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_EnvVarProxyNotExisting_CouldNotResolveProxy)
    {
    VerifyProxyLogEmpty();

    Request request("http://httpbin.org/ip");
    putenv("http_proxy=" NONEXISTING_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(ConnectionStatus::CouldNotResolveProxy, response.GetConnectionStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), Not(HasSubstr("GET http://httpbin.org/ip")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_EnvVarProxyNotExistingForHttps_CouldNotResolveProxy)
    {
    VerifyProxyLogEmpty();

    Request request("https://httpbin.org/ip");
    putenv("https_proxy=" NONEXISTING_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(ConnectionStatus::CouldNotResolveProxy, response.GetConnectionStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), Not(HasSubstr("GET https://httpbin.org/ip")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_EnvVarProxy_ExecutesViaProxy)
    {
    VerifyProxyLogEmpty();

    Request request("http://httpbin.org/ip");
    putenv("http_proxy=" LOCAL_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), HasSubstr("GET http://httpbin.org/ip"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_EnvVarProxyForHttps_ExecutesViaProxy)
    {
    VerifyProxyLogEmpty();

    Request request("https://httpbin.org/ip");
    putenv("https_proxy=" LOCAL_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), HasSubstr("CONNECT httpbin.org:443"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_EnvVarProxyForHttpsButRequestHttp_ExecutesDirectly)
    {
    VerifyProxyLogEmpty();

    Request request("http://httpbin.org/ip");
    putenv("https_proxy=" LOCAL_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), Not(HasSubstr("GET http://httpbin.org/ip")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_EnvVarProxyForHttpButRequestHttps_ExecutesDirectly)
    {
    VerifyProxyLogEmpty();

    Request request("https://httpbin.org/ip");
    putenv("http_proxy=" LOCAL_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), Not(HasSubstr("GET https://httpbin.org/ip")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_EnvVarProxyButRequestOverrides_ExecutesViaRequestProxy)
    {
    VerifyProxyLogEmpty();

    Request request("http://httpbin.org/ip");
    request.SetProxy(LOCAL_PROXY_URL);
    putenv("http_proxy=" NONEXISTING_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), HasSubstr("GET http://httpbin.org/ip"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_EnvVarProxyButDefaultOverrides_ExecutesViaDefaultProxy)
    {
    VerifyProxyLogEmpty();

    Request request("http://httpbin.org/ip");
    HttpProxy::SetDefaultProxy(HttpProxy(LOCAL_PROXY_URL));
    putenv("http_proxy=" NONEXISTING_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), HasSubstr("GET http://httpbin.org/ip"));
    }