/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/BeHttp/HttpRequestTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#define BEHTTP_ENABLE_DUPLICATING_SYMBOLS

#include "Tests.h"

#include <Bentley/BeDebugLog.h>
#include <Bentley/BeTimeUtilities.h>
#include <BeHttp/HttpBody.h>
#include <BeHttp/HttpProxy.h>
#include <BeHttp/HttpClient.h>
#include <BeHttp/HttpError.h>
#include <BeHttp/HttpStatus.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <Bentley/Bentley.h>
#include <Bentley/BeThread.h>
#include <Bentley/Bentley.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/Base64Utilities.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/AsyncTasksManager.h>
#include <Bentley/Tasks/TaskScheduler.h>
#include <Bentley/Tasks/WorkerThread.h>
#include <BeJsonCpp/BeJsonUtilities.h>

#include "../../../BeHttp/Backdoor.h"
#include "../../UnitTests/Published/FSTest.h"
#include "../../UnitTests/Published/WebTestsHelper.h"
#include "../Scripts/ScriptRunner.h"
#include "AsyncTestCheckpoint.h"

#define WAITTIMEOUT 30000

// Server need to have compression enabled - WSG 2.5+ has this by default
#define COMPRESSION_OPTIONS_URL "https://mobilevm2.bentley.com/ws/v2.5/Repositories"

#define GENERIC_URL "https://httpbin.org/uuid"

#define HTTPBIN_HOST "httpbin.bentley.com"
#define HTTPBIN_HTTP_URL "http://" HTTPBIN_HOST
#define HTTPBIN_HTTPS_URL "https://" HTTPBIN_HOST

using namespace ::testing;
using namespace folly;

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS
USING_NAMESPACE_BENTLEY_TASKS

struct HttpRequestTests : ::testing::Test
    {
    static IHttpHandlerPtr s_proxy;

    static void SetUpTestCase()
        {
        BeFileName path;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(path);
        HttpClient::Initialize(path);

        // Enable routing tests trough Fiddler for addtional tests. Note that addtional proxy might change behaviour.
        // s_proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
        }
    void Reset()
        {
        // Enable full logging with LOG_TRACE if needed
        NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_HTTP, NativeLogging::LOG_INFO);
        NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BEHTTP_TESTS, NativeLogging::SEVERITY::LOG_INFO);

        putenv("http_proxy=");
        putenv("https_proxy=");

        AsyncTasksManager::SetDefaultScheduler(nullptr);

        HttpClient::Reinitialize();

        Backdoor::InitStartBackgroundTask([] (Utf8CP name, std::function<void()> task, std::function<void()> onExpired) {});
        Backdoor::CallOnApplicationSentToForeground();

        HttpProxy::SetDefaultProxy(HttpProxy());
        // Enable routing tests trough Fiddler for addtional tests. Note that addtional proxy might change behaviour.
        // HttpProxy::SetDefaultProxy(HttpProxy("http://127.0.0.1:8888"));
        }

    void SetUp() { Reset(); }
    void TearDown() { Reset(); }
    };

IHttpHandlerPtr HttpRequestTests::s_proxy;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, PerformAsync_RandomlyGeneratedDataApi_PrintsUrlsAndDataToFixFailingTests)
    {
    for (int i = 0; i < 1; i++)
        {
        Utf8PrintfString url(HTTPBIN_HTTP_URL "/bytes/2?seed=%d", i);
        Request request(url.c_str());
        Response response = request.PerformAsync()->GetResult();;
        TESTLOG.infov("URL: %s out:'%s'", url.c_str(), response.GetBody().AsString().c_str());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, PerformAsync_EmptyUrl_ErrorCouldNotConnect)
    {
    BeTest::SetFailOnAssert(false);
    Request request("");
    BeTest::SetFailOnAssert(true);
    Response response = request.PerformAsync()->GetResult();
    EXPECT_EQ(ConnectionStatus::CouldNotConnect, response.GetConnectionStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, PerformAsync_BentleyTasksApiAndOneRequest_ExecutesSuccessfully)
    {
    Request request(HTTPBIN_HTTPS_URL "/ip");

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
    Request request(HTTPBIN_HTTPS_URL "/ip");

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
    Request request(HTTPBIN_HTTP_URL "/ip");

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
TEST_F(HttpRequestTests, Perform_EmptyUrl_ErrorCouldNotConnect)
    {
    BeTest::SetFailOnAssert(false);
    Request request("");
    BeTest::SetFailOnAssert(true);
    Response response = request.Perform().get();
    EXPECT_EQ(ConnectionStatus::CouldNotConnect, response.GetConnectionStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_CertValidationSetAndSiteHasValidCert_Success)
    {
    Request request(HTTPBIN_HTTPS_URL "/ip");
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
    Request request(HTTPBIN_HTTPS_URL "/ip");
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
TEST_F(HttpRequestTests, Perform_CertValidationSetAndSiteHasBentleyInternalCertificate_SucceedsAsUsesNativeSystemSslEngineAndSystemTrustsCertificate)
    {
    Request request("https://mobilevm6.bentley.com/ws");
    request.SetValidateCertificate(true);

    Response response = request.Perform().get();

    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_OneRequest_ExecutesSuccessfully)
    {
    // Simple test for deprecated method

    Request request(HTTPBIN_HTTP_URL "/ip");

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, PerformAsync_OneRequest_ExecutesSuccessfully)
    {
    Request request(HTTPBIN_HTTP_URL "/ip");

    Response response = request.PerformAsync()->GetResult();

    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_MovedRequest_ExecutesSuccessfully)
    {
    Request request(std::move(Request(HTTPBIN_HTTP_URL "/ip")));

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_FALSE(Json::Reader::DoParse(response.GetBody().AsString()).isNull());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_RequestSuccessfull_SetsEffectiveUrl)
    {
    Request request(HTTPBIN_HTTP_URL "/ip");
    Response response = request.Perform().get();

    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ(HTTPBIN_HTTP_URL "/ip", response.GetEffectiveUrl());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_RequestWithRedirect_SetsEffectiveUrlFromRedirect)
    {
    Request request(HTTPBIN_HTTP_URL "/redirect-to?url=http%3A%2F%2Fexample.com%2F");
    Response response = request.Perform().get();

    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ("http://example.com/", response.GetEffectiveUrl());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_RequestCouldNotConnect_SetsEffectiveUrlToRequestUrl)
    {
    Request request("http://foo.boo.test.not.existing");
    Response response = request.Perform().get();

    EXPECT_EQ(ConnectionStatus::CouldNotConnect, response.GetConnectionStatus());
    EXPECT_EQ("http://foo.boo.test.not.existing/", response.GetEffectiveUrl());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_UnsafeCharactersInUrl_EscapesUnsafeCharactersAndExecutesSuccessfully)
    {
    //2014. Folowing urls with special symbols tested on browsers (# and % not tested )
    //  query: http://httpbin.bentley.org/ip?<>"{}|\^~[]`
    //  url:   http://httpbin.bentley.org/ip<>"{}|\^~[]`
    //FireFox:
    //  query: http://httpbin.bentley.org/ip?%3C%3E%22{}|\^~[]%60
    //  url:   http://httpbin.bentley.org/ip%3C%3E%22%7B%7D|%5C%5E~%5B%5D%60
    //InternetExplorer:
    //  query: http://httpbin.bentley.org/ip?<>"{}|\^~[]`
    //  url:   http://httpbin.bentley.org/ip%3C%3E%22%7B%7D%7C/%5E~[]%60
    //Chrome:
    //  query: http://httpbin.bentley.org/ip?%3C%3E%22{}|\^~[]`
    //  url:   http://httpbin.bentley.org/ip%3C%3E%22%7B%7D%7C/%5E~[]%60
    //Casablanca :
    //  Allows only #, % and ~ from unsafe sumbols
    //CURL:
    //  Removes everything after #, does not encode anything. Some have special meaning - [], {} for using multiple urls in command line

    Request request(TEST_URL_UNSAFE_CHARS_NO_FRAGMENT);

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_STREQ(TEST_URL_UNSAFE_CHARS_NO_FRAGMENT_ESCAPED, response.GetEffectiveUrl().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, GetUrl_UnsafeCharactersInUrl_ReturnsEscapedCharactersInUrl)
    {
    Request request(TEST_URL_UNSAFE_CHARS);
    EXPECT_STREQ(TEST_URL_UNSAFE_CHARS_ESCAPED, request.GetUrl().c_str());
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
    Request request(HTTPBIN_HTTPS_URL "/basic-auth/user/pass");
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
    Request request(HTTPBIN_HTTPS_URL "/basic-auth/user/pass");

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::Unauthorized, response.GetHttpStatus());
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_BasicAuthorizationIncorrectCredentials_Fails)
    {
    Request request(HTTPBIN_HTTPS_URL "/basic-auth/user/pass");
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
    Request request(HTTPBIN_HTTP_URL "/redirect/1");
    request.SetFollowRedirects(true);

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_FolowRedirectsFalse_ReturnsWithFound)
    {
    Request request(HTTPBIN_HTTP_URL "/redirect/1");
    request.SetFollowRedirects(false);

    Response response = request.Perform().get();

    EXPECT_EQ(HttpStatus::Found, response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, PerformAsync_OneRequest_ExecutesSuccessfullyWithChainedTask)
    {
    Response response;
    Request request(HTTPBIN_HTTP_URL "/ip");

    auto task = request.PerformAsync()->Then(
        [&] (Response& finishedResponse)
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
TEST_F(HttpRequestTests, Perform_DefaultHttpRequestAndReceivedBody_SucceedsWithBody)
    {
    Request request(HTTPBIN_HTTP_URL "/bytes/2?seed=7");

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
    Request request(HTTPBIN_HTTP_URL "/bytes/2?seed=7");
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

    Request request(HTTPBIN_HTTP_URL "/bytes/2?seed=7");
    request.SetResponseBody(responseBody);

    Response response = request.Perform().get();
    EXPECT_EQ(responseBody.get(), &response.GetBody());
    EXPECT_EQ(2, responseBody->GetLength());
    EXPECT_EQ("R&", responseBody->AsString());

    request = Request(HTTPBIN_HTTP_URL "/bytes/1?seed=1");
    request.SetResponseBody(responseBody);

    response = request.Perform().get();
    EXPECT_EQ(responseBody.get(), &response.GetBody());
    EXPECT_EQ(1, responseBody->GetLength());
    EXPECT_EQ("\"", responseBody->AsString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Perform_ReusingSameRequestBody_KeepsSameBody)
    {
    Request request(HTTPBIN_HTTP_URL "/post", "POST");

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

    Request request(HTTPBIN_HTTP_URL "/bytes/0");
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

    Request request(HTTPBIN_HTTP_URL "/range/10?duration=20&chunk_size=5");
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
        Request request(HTTPBIN_HTTP_URL "/ip");
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
        Request request(HTTPBIN_HTTP_URL "/ip");
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(HttpRequestTests, NoBody)
    {
    Request request(HTTPBIN_HTTP_URL "/ip", "HEAD");

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    Utf8String body = response.GetBody().AsString();
    EXPECT_STREQ("", body.c_str());
    auto headers = response.GetHeaders();
    EXPECT_TRUE(Utf8String::IsNullOrEmpty(headers.GetETag()));
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
    os << value.url << "," << value.method << "," << Utf8PrintfString("Body: \"%.8s..\" Size:(%d)", value.requestBody.c_str(), value.requestBody.size());
    return os;
    }

// GET vs POST/PUT/etc has different implementations
#define BODY_SIZE_DifferentMethods 1000
struct HttpRequestTestsMethods : HttpRequestTests, WithParamInterface<MethodParam> {};
INSTANTIATE_TEST_CASE_P(DifferentMethods, HttpRequestTestsMethods, Values(
    MethodParam {HTTPBIN_HTTP_URL "/ip",       "GET",      ""},
    MethodParam {HTTPBIN_HTTP_URL "/put",      "PUT",      Utf8String(BODY_SIZE_DifferentMethods, 'x')},
    MethodParam {HTTPBIN_HTTP_URL "/delete",   "DELETE",   Utf8String(BODY_SIZE_DifferentMethods, 'x')},
    MethodParam {HTTPBIN_HTTP_URL "/post",     "POST",     Utf8String(BODY_SIZE_DifferentMethods, 'x')}
));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HttpRequestTestsMethods, PerformAsync_BeforeMovedToBackground_RequestFinishesInBackground)
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
TEST_P(HttpRequestTestsMethods, PerformAsync_WhenMovedToBackground_RequestDoesNothingUntilInForeground)
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
TEST_P(HttpRequestTestsMethods, PerformAsync_AfterMovedFromBackgroundToForeground_ExecutesRequestAsUsual)
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
TEST_P(HttpRequestTestsMethods, PerformAsync_ExecutingInBackgroundWithDownloadAndUploadProgressCallbacks_IntermediateProgressIsNotReported)
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
TEST_P(HttpRequestTestsMethods, PerformAsync_NonRetryaleRequestAndBackgroundTimeExpiredWhenRunning_RequestReturnsError)
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
TEST_P(HttpRequestTestsMethods, PerformAsync_RetryaleRequestAndBackgroundTimeExpiredWhenRunning_RequestRestartedWhenInForeground_FLAKYTEST)
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
* @bsimethod                                Robert.Lukasonok                       07/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_SwitchingBetweenTwoServers_SamePerformanceBetweenPickers)
    {
    const bvector<Utf8String> urlList =
        {
        GENERIC_URL,
        HTTPBIN_HTTP_URL "/uuid"
        };
    const size_t numUrls = urlList.size();
    const size_t requestsPerUrl = 5;
    const auto urlPickerList = {
        std::function<size_t(size_t)>([numUrls] (size_t i) { return i % numUrls; }),
        std::function<size_t(size_t)>([requestsPerUrl] (size_t i) { return i / requestsPerUrl; })
        };

    // Ignore first-time loads
    for (const auto& url : urlList)
        EXPECT_EQ(HttpStatus::OK, Request(url).Perform().get().GetHttpStatus());

    bvector<bvector<uint64_t>> pickerDurations;
    for (const auto& urlPicker : urlPickerList)
        {
        bvector<uint64_t> urlDurations(numUrls, 0);
        for (size_t i = 0; i < numUrls * requestsPerUrl; ++i)
            {
            size_t urlIndex = urlPicker(i);
            Utf8String url = urlList[urlIndex];
            Request request(url);
            uint64_t before = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
            Response response = request.Perform().get();
            EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
            uint64_t after = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
            urlDurations[urlIndex] += after - before;
            }

        pickerDurations.push_back(std::move(urlDurations));
        }

    for (size_t i = 0; i < pickerDurations.size(); ++i)
        {
        TESTLOG.infov("Switching method #%d average wait time:", i + 1);
        for (size_t j = 0; j < urlList.size(); ++j)
            TESTLOG.infov("\t%31s - %5dms", urlList[j], pickerDurations[i][j] / requestsPerUrl);
        }
    }

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

    Request request(HTTPBIN_HTTP_URL "/ip");
    request.SetProxy(NONEXISTING_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(ConnectionStatus::CouldNotResolveProxy, response.GetConnectionStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), Not(HasSubstr("GET " HTTPBIN_HTTP_URL "/ip")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_ProxyUrlSet_ExecutesViaProxy)
    {
    VerifyProxyLogEmpty();

    Request request(HTTPBIN_HTTP_URL "/ip");
    request.SetProxy(LOCAL_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), HasSubstr("GET " HTTPBIN_HTTP_URL "/ip"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_DefaultProxyUrlSet_ExecutesViaProxy)
    {
    VerifyProxyLogEmpty();

    Request request(HTTPBIN_HTTP_URL "/ip");
    HttpProxy::SetDefaultProxy(HttpProxy(LOCAL_PROXY_URL));

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), HasSubstr("GET " HTTPBIN_HTTP_URL "/ip"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_DefaultProxyPacUrlSetToInvalidUrl_CouldNotResolveProxy)
    {
    VerifyProxyLogEmpty();

    Request request(HTTPBIN_HTTP_URL "/ip");
    HttpProxy proxy;
    proxy.SetPacUrl("boofoo.js");
    HttpProxy::SetDefaultProxy(proxy);

    Response response = request.Perform().get();
    EXPECT_EQ(ConnectionStatus::CouldNotResolveProxy, response.GetConnectionStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), Not(HasSubstr("GET " HTTPBIN_HTTP_URL "/ip")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_DefaultProxyPacUrlSetToInvalidUrlAndPOSTRequest_CouldNotResolveProxy)
    {
    VerifyProxyLogEmpty();

    Request request(HTTPBIN_HTTP_URL "/ip", "POST");
    HttpProxy proxy;
    proxy.SetPacUrl("boofoo.js");
    HttpProxy::SetDefaultProxy(proxy);

    Response response = request.Perform().get();
    EXPECT_EQ(ConnectionStatus::CouldNotResolveProxy, response.GetConnectionStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), Not(HasSubstr("GET " HTTPBIN_HTTP_URL "/ip")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_DefaultProxyPacUrlSetToNotExisting_CouldNotResolveProxy)
    {
    VerifyProxyLogEmpty();

    Request request(HTTPBIN_HTTP_URL "/ip");
    HttpProxy proxy;
    proxy.SetPacUrl(NONEXISTING_PROXY_URL "/foo.js");
    HttpProxy::SetDefaultProxy(proxy);

    Response response = request.Perform().get();
    EXPECT_EQ(ConnectionStatus::CouldNotResolveProxy, response.GetConnectionStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), Not(HasSubstr("GET " HTTPBIN_HTTP_URL "/ip")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_DefaultProxyPacUrlSetToInvalidUrl_SetsEffectiveUrl)
    {
    Request request(HTTPBIN_HTTP_URL "/ip");
    HttpProxy proxy;
    proxy.SetPacUrl("boofoo.js");
    HttpProxy::SetDefaultProxy(proxy);

    Response response = request.Perform().get();
    EXPECT_EQ(ConnectionStatus::CouldNotResolveProxy, response.GetConnectionStatus());
    EXPECT_EQ(HTTPBIN_HTTP_URL "/ip", response.GetEffectiveUrl());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_DefaultProxyPacUrlSetWithDirect_ExecutesDirectly)
    {
    VerifyProxyLogEmpty();

    Request request(HTTPBIN_HTTP_URL "/ip");
    HttpProxy proxy;
    proxy.SetPacUrl(LOCAL_SERVER_URL "/Data/pac2.js"); // pac2.js PROXY DIRECT
    HttpProxy::SetDefaultProxy(proxy);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), Not(HasSubstr("GET " HTTPBIN_HTTP_URL "/ip")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_DefaultProxyPacUrlSetWithSecondProxyReachable_ExecutesViaProxy)
    {
    VerifyProxyLogEmpty();

    Request request(HTTPBIN_HTTP_URL "/ip");
    HttpProxy proxy;
    proxy.SetPacUrl(LOCAL_SERVER_URL "/Data/pac3.js"); // pac3.js PROXY fake.bentley.com; PROXY localhost:9991
    HttpProxy::SetDefaultProxy(proxy);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), HasSubstr("GET " HTTPBIN_HTTP_URL "/ip"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_EnvVarProxyNotExisting_CouldNotResolveProxy)
    {
    VerifyProxyLogEmpty();

    Request request(HTTPBIN_HTTP_URL "/ip");
    putenv("http_proxy=" NONEXISTING_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(ConnectionStatus::CouldNotResolveProxy, response.GetConnectionStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), Not(HasSubstr("GET " HTTPBIN_HTTP_URL "/ip")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_EnvVarProxyNotExistingForHttps_CouldNotResolveProxy)
    {
    VerifyProxyLogEmpty();

    Request request(HTTPBIN_HTTPS_URL "/ip");
    putenv("https_proxy=" NONEXISTING_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(ConnectionStatus::CouldNotResolveProxy, response.GetConnectionStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), Not(HasSubstr("GET " HTTPBIN_HTTPS_URL "/ip")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_EnvVarProxy_ExecutesViaProxy)
    {
    VerifyProxyLogEmpty();

    Request request(HTTPBIN_HTTP_URL "/ip");
    putenv("http_proxy=" LOCAL_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), HasSubstr("GET " HTTPBIN_HTTP_URL "/ip"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_EnvVarProxyForHttps_ExecutesViaProxy)
    {
    VerifyProxyLogEmpty();

    Request request(HTTPBIN_HTTPS_URL "/ip");
    putenv("https_proxy=" LOCAL_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), HasSubstr("CONNECT " HTTPBIN_HOST ":443"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_EnvVarProxyForHttpsButRequestHttp_ExecutesDirectly)
    {
    VerifyProxyLogEmpty();

    Request request(HTTPBIN_HTTP_URL "/ip");
    putenv("https_proxy=" LOCAL_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), Not(HasSubstr("GET " HTTPBIN_HTTP_URL "/ip")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_EnvVarProxyForHttpButRequestHttps_ExecutesDirectly)
    {
    VerifyProxyLogEmpty();

    Request request(HTTPBIN_HTTPS_URL "/ip");
    putenv("http_proxy=" LOCAL_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), Not(HasSubstr("GET " HTTPBIN_HTTPS_URL "/ip")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_EnvVarProxyButRequestOverrides_ExecutesViaRequestProxy)
    {
    VerifyProxyLogEmpty();

    Request request(HTTPBIN_HTTP_URL "/ip");
    request.SetProxy(LOCAL_PROXY_URL);
    putenv("http_proxy=" NONEXISTING_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), HasSubstr("GET " HTTPBIN_HTTP_URL "/ip"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTestsProxy, Perform_EnvVarProxyButDefaultOverrides_ExecutesViaDefaultProxy)
    {
    VerifyProxyLogEmpty();

    Request request(HTTPBIN_HTTP_URL "/ip");
    HttpProxy::SetDefaultProxy(HttpProxy(LOCAL_PROXY_URL));
    putenv("http_proxy=" NONEXISTING_PROXY_URL);

    Response response = request.Perform().get();
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());

    EXPECT_THAT(GetLocalProxyLog().c_str(), HasSubstr("GET " HTTPBIN_HTTP_URL "/ip"));
    }
    
enum class TlsVersion
    {
    v1_0,
    v1_1,
    v1_2
    };

static const bvector<TlsVersion> AllTlsVersions
    {
    TlsVersion::v1_0, 
    TlsVersion::v1_1, 
    TlsVersion::v1_2
    };

struct TlsRequestSettings
    {
    TlsVersion tlsVersion;
    ConnectionStatus expectedConnectionStatus;
    };

struct HttpRequestTestsTls : HttpRequestTests, WithParamInterface<TlsRequestSettings>
    {
    static bmap<TlsVersion, unsigned short> s_portMap;
    static bmap<TlsVersion, unsigned short> CreatePortMap()
        {
        bmap<TlsVersion, unsigned short> portMap;
        portMap[TlsVersion::v1_0] = 4410;
        portMap[TlsVersion::v1_1] = 4411;
        portMap[TlsVersion::v1_2] = 4412;
        return portMap;
        }
    static const Utf8String MakeServerUrl(TlsVersion serverTlsVersion)
        {
        static const Utf8String host = "localhost";
        unsigned short port = s_portMap[serverTlsVersion];
        return "https://" + host + ":" + std::to_string(port).c_str();
        }
    static void AssertServerIsStopped (TlsVersion serverTlsVersion) 
        {
        auto serverUrl = MakeServerUrl(serverTlsVersion);
        Request request = Request(serverUrl);
        Response response = request.PerformAsync()->GetResult();
        ASSERT_EQ(ConnectionStatus::CouldNotConnect, response.GetConnectionStatus()) << "Different server process is still running due to previous tests existing unexpectidely. Close existing processes";
        }
    static void AssertServersAreStopped ()
        {
        // The servers are checked in async to shorten the waiting time for checking server availability
        bset<std::shared_ptr<AsyncTask>> serversAreStoppedTasks;
        for (TlsVersion tlsVersion : AllTlsVersions)
            {
            auto serverIsStoppedTask = Tasks::WorkerThread::Create()->ExecuteAsync([=]
                {
                AssertServerIsStopped(tlsVersion);
                });
            serversAreStoppedTasks.insert(serverIsStoppedTask);
            }

        AsyncTask::WhenAll(serversAreStoppedTasks)->Wait();
        }
    static void StartServers () 
        {
        static const Utf8String tlsServersBatFile = "tls-server.bat";
        ScriptRunner::RunScriptAsync(tlsServersBatFile);
        WaitUntilServersAreRunning();
        }
    static void StopServers ()
        {
        ScriptRunner::StopAllPythonScripts();
        }
    static void SetUpTestCase()
        {
        AssertServersAreStopped();
        StartServers();
        }
    static void TearDownTestCase()
        {
        StopServers();
        }
    static void WaitUntilServerIsRunning(TlsVersion serverTlsVersion)
        {
        uint64_t startMs = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        auto serverUrl = MakeServerUrl(serverTlsVersion);
        // waiting until either the request is good or there is a certificate error, which we assume indicates incompatible tls requirements
        while (true)
            {
            Response response = Request(serverUrl).Perform().get();

            if (HttpStatus::OK == response.GetHttpStatus() ||
                ConnectionStatus::CertificateError == response.GetConnectionStatus())
                break;
            
            uint64_t nowMs = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
            if (nowMs - startMs > WAITTIMEOUT)
                {
                FAIL() << "Timed out waiting for TLS test server to start up.";
                return;
                }
            BeThreadUtilities::BeSleep(100);
            }
        }
    static void WaitUntilServersAreRunning()
        {
        for (TlsVersion tlsVersion : AllTlsVersions)
            WaitUntilServerIsRunning(tlsVersion);
        }
    };

bmap<TlsVersion, unsigned short> HttpRequestTestsTls::s_portMap = HttpRequestTestsTls::CreatePortMap();

TEST_P(HttpRequestTestsTls, PerformAsync_TlsServerConnectionRequest_GetsExpectedConnectionResult)
    {
    TlsRequestSettings tslRequestSettings = GetParam();
    Request request(MakeServerUrl(tslRequestSettings.tlsVersion));
    Response response = request.Perform().get();
    EXPECT_EQ(tslRequestSettings.expectedConnectionStatus, response.GetConnectionStatus());
    }

// Disabling until a curl fix is applied
INSTANTIATE_TEST_CASE_P(, HttpRequestTestsTls, Values(
     //TlsRequestSettings {TlsVersion::v1_0, ConnectionStatus::CertificateError},
     TlsRequestSettings {TlsVersion::v1_1, ConnectionStatus::OK},
     TlsRequestSettings {TlsVersion::v1_2, ConnectionStatus::OK}
));

struct StubTaskScheduler : ITaskScheduler
    {
    void Push(std::shared_ptr<AsyncTask> task, AsyncTask::Priority priority)
        {
        ADD_FAILURE();
        task->Execute();
        };
    void Push(std::shared_ptr<AsyncTask> task, std::shared_ptr<AsyncTask> parentTask, AsyncTask::Priority priority)
        {
        ADD_FAILURE();
        if (parentTask)
            parentTask->AddSubTask(task);
        task->Execute();
        };
    std::shared_ptr<AsyncTask> WaitAndPop() { ADD_FAILURE(); return nullptr; };
    std::shared_ptr<AsyncTask> TryPop() { ADD_FAILURE(); return nullptr; };
    int GetQueueTaskCount() const { ADD_FAILURE(); return 0; };
    bool HasRunningTasks() const { ADD_FAILURE(); return false; };
    AsyncTaskPtr<void> OnEmpty() const { ADD_FAILURE(); return CreateCompletedAsyncTask(); };
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HttpRequestTestsMethods, Perform_WithCustomDefaultSheduler_DoesNotUseDefaultShedulerToAvoidDeadlocks)
    {
    auto sheduler = std::make_shared<StubTaskScheduler>();
    AsyncTasksManager::SetDefaultScheduler(sheduler);

    Request request = GetParam().CreateRequest();
    Response response = request.Perform().get();
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HttpRequestTestsMethods, PerformAsync_WithCustomDefaultSheduler_DoesNotUseDefaultShedulerToAvoidDeadlocks)
    {
    auto sheduler = std::make_shared<StubTaskScheduler>();
    AsyncTasksManager::SetDefaultScheduler(sheduler);

    Request request = GetParam().CreateRequest();
    Response response = request.PerformAsync()->GetResult();
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }

typedef RefCountedPtr<struct StubHttpBody> StubHttpBodyPtr;
struct StubHttpBody : public HttpBody
    {
    static StubHttpBodyPtr Create() { return new StubHttpBody(); }
    std::function<void()> onData;

    virtual void Open() override {};
    virtual void Close() override {};
    virtual BentleyStatus SetPosition(uint64_t position) override { return SUCCESS; };
    virtual BentleyStatus GetPosition(uint64_t& position) override { return SUCCESS; };
    virtual BentleyStatus Reset() override { return SUCCESS; };

    virtual size_t Write(const char* buffer, size_t bufferSize) override { onData(); return bufferSize; };
    virtual size_t Read(char* bufferOut, size_t bufferSize) override { onData(); return bufferSize; };

    virtual uint64_t GetLength() override { return 100; };

    virtual Utf8String AsString() const override { return nullptr; };
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HttpRequestTestsMethods, Uninitialize_BeforeRequest_EachRequestIsCancelled)
    {
    HttpClient::Uninitialize();

    Request request = GetParam().CreateRequest();
    Response response = request.Perform().get();
    EXPECT_EQ(ConnectionStatus::Canceled, response.GetConnectionStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HttpRequestTestsMethods, Uninitialize_WhileDownloading_ReturnsCancelled)
    {
    Request request = GetParam().CreateRequest();

    AsyncTestCheckpoint cp;
    auto body = StubHttpBody::Create();
    body->onData = [&]
        {
        cp.CheckinAndWait();
        };
    request.SetResponseBody(body);

    auto task = request.PerformAsync();
    cp.WaitUntilReached();
    Backdoor::UninitializeCancelAllRequests();
    cp.Continue();
    HttpClient::Uninitialize();
    Response response = task->GetResult();
    EXPECT_EQ(ConnectionStatus::Canceled, response.GetConnectionStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HttpRequestTestsMethods, Uninitialize_WhilePerformingRequest_ReturnsCancelled_FLAKYTEST)
    {
    Request request = GetParam().CreateRequest();

    AsyncTestCheckpoint cp;
    request.SetDownloadProgressCallback([&] (double, double)
        {
        cp.CheckinAndWait();
        });

    auto task = request.PerformAsync();
    cp.WaitUntilReached();
    Backdoor::UninitializeCancelAllRequests();
    cp.Continue();
    HttpClient::Uninitialize();
    Response response = task->GetResult();
    EXPECT_EQ(ConnectionStatus::Canceled, response.GetConnectionStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HttpRequestTestsMethods, Reinitialize_AfterUninitialize_RequestsPass)
    {
    HttpClient::Uninitialize();
    HttpClient::Reinitialize();

    Request request = GetParam().CreateRequest();
    Response response = request.Perform().get();
    EXPECT_EQ(ConnectionStatus::OK, response.GetConnectionStatus());
    EXPECT_EQ(HttpStatus::OK, response.GetHttpStatus());
    }

