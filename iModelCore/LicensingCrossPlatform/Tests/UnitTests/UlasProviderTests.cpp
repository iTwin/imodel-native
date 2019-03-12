/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/UlasProviderTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UlasProviderTests.h"

#include <BeHttp/HttpClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/L10N.h>

#define TEST_PRODUCT_ID     "2545"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE

UlasProviderTests::UlasProviderTests() :
    m_handlerMock(std::make_shared<MockHttpHandler>()),
    m_buddiMock(std::make_shared<BuddiProviderMock>())
    {
    m_ulasProvider = std::make_shared<UlasProvider>(m_buddiMock, m_handlerMock);
    }

UlasProvider& UlasProviderTests::GetUlasProvider() const
    {
    return *m_ulasProvider;
    }

MockHttpHandler& UlasProviderTests::GetMockHttp() const
    {
    return *m_handlerMock;
    }

std::shared_ptr<MockHttpHandler> UlasProviderTests::GetHandlerPtr() const
    {
    return m_handlerMock;
    }


void UlasProviderTests::TearDown()
{
    m_handlerMock->ValidateAndClearExpectations();
}

void UlasProviderTests::SetUpTestCase()
{
    // This is only an example of how to set logging severity and see info logs. Usually should be set more globally than in TestCase SetUp
    // NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_INFO);

    BeFileName asssetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(asssetsDir);
    HttpClient::Initialize(asssetsDir);

    BeFileName tmpDir;
    BeTest::GetHost().GetTempDir(tmpDir);
    BeSQLiteLib::Initialize(tmpDir);

    BeFileName path;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(path);
    path.AppendToPath(L"sqlang/DgnClientFx_en.sqlang.db3");

    ASSERT_EQ(SUCCESS, L10N::Initialize(BeSQLite::L10N::SqlangFiles(path)));
}

BuddiProviderMock& UlasProviderTests::GetMockBuddi() const {
    return *m_buddiMock;
}

// this is used to use the mock url as input to the mock http client
Utf8String UlasProviderTests::MockUlasUrl()
    {
    Utf8String mockUrl("https://ulasmockurl.bentley.com");

    GetMockBuddi().MockUlasLocationBaseUrl(mockUrl);

    return mockUrl;
    }

// need PostUsageLogs and SendUsageLogs tests
// just post? (will call send...) -> wait and see

TEST_F(UlasProviderTests, SendUsageLogs_Success)
    {
    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", TEST_PRODUCT_ID);
    BeFileName dbPath("TestPath");

    const auto mockUrl = MockUlasUrl();
    Utf8String expectedUrl = mockUrl + Utf8PrintfString("/usageLog?ultId=%s&prdId=%s&lng=%s", "1004175881", TEST_PRODUCT_ID, "en");

    const auto epUri = "https://locationmockurl.bentley.com/";
    const auto sharedAccessSignature = "MockSharedAccessSignature";
    Utf8String expectedUploadUrl = epUri + Utf8PrintfString(sharedAccessSignature);

    GetMockHttp().ExpectRequests(2);
    // return a mock location response
    GetMockHttp().ForRequest(1, [=](Http::RequestCR request)
        {
        EXPECT_EQ(expectedUrl, request.GetUrl());
        Json::Value responseJson(Json::objectValue);
        responseJson["epUri"] = epUri;
        responseJson["epInfo"]["SharedAccessSignature"] = sharedAccessSignature;

        return Response(HttpStatus::OK, "", HttpResponseContent::Create(HttpStringBody::Create(Json::FastWriter().write(responseJson))));
        });
    // expected url is from the response of request 1
    GetMockHttp().ForRequest(2, [=](Http::RequestCR uploadRequest)
        {
        EXPECT_EQ(expectedUploadUrl, uploadRequest.GetUrl());
        return MockHttpHandler::StubHttpResponse();
        });

    GetUlasProvider().SendUsageLogs(clientInfo, BeFileName("TestName"), Utf8String("1004175881")).get();
    }

TEST_F(UlasProviderTests, SendFeatureLogs_Success)
    {
    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", TEST_PRODUCT_ID);
    BeFileName dbPath("TestPath");

    const auto mockUrl = MockUlasUrl();
    Utf8String expectedUrl = mockUrl + Utf8PrintfString("/featureLog?ultId=%s&prdId=%s&lng=%s", "1004175881", TEST_PRODUCT_ID, "en");

    const auto epUri = "https://locationmockurl.bentley.com/";
    const auto sharedAccessSignature = "MockSharedAccessSignature";
    Utf8String expectedUploadUrl = epUri + Utf8PrintfString(sharedAccessSignature);

    GetMockHttp().ExpectRequests(2);
    // return a mock location response
    GetMockHttp().ForRequest(1, [=](Http::RequestCR request)
        {
        EXPECT_EQ(expectedUrl, request.GetUrl());
        Json::Value responseJson(Json::objectValue);
        responseJson["epUri"] = epUri;
        responseJson["epInfo"]["SharedAccessSignature"] = sharedAccessSignature;

        return Response(HttpStatus::OK, "", HttpResponseContent::Create(HttpStringBody::Create(Json::FastWriter().write(responseJson))));
        });
    // expected url is from the response of request 1
    GetMockHttp().ForRequest(2, [=](Http::RequestCR uploadRequest)
        {
        EXPECT_EQ(expectedUploadUrl, uploadRequest.GetUrl());
        return MockHttpHandler::StubHttpResponse();
        });

    GetUlasProvider().SendFeatureLogs(clientInfo, BeFileName("TestName"), Utf8String("1004175881")).get();
    }
