/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "TestsHelper.h"
#include "BuddiProviderTests.h"
#include "Utils/MockHttpHandler.h"
#include "DummyPolicyHelper.h"
#include "../../Licensing/Providers/BuddiProvider.h"

#include <Licensing/Client.h>
#include <Licensing/SaasClient.h>
#include <Licensing/Utils/DateHelper.h>
#include <Licensing/AuthType.h>
#include "../../PublicAPI/Licensing/Utils/SCVWritter.h"

#include <BeHttp/HttpClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/L10N.h>
#include <fstream>
#include <Licensing/Utils/InMemoryJsonLocalState.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

#define TEST_PRODUCT_ID     "2545"
#define MOCK_URL            "https://mockurl.bentley.com/"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE


BuddiProviderTests::BuddiProviderTests() :
    m_httpHandlerMock(std::make_shared<MockHttpHandler>()),
    m_authProviderMock(std::make_shared<AuthHandlerProviderMock>()),
    m_buddiProvider (std::make_shared<BuddiProvider>())
    {}

BuddiProvider& BuddiProviderTests::GetBuddiProvider() const
    {
    return *m_buddiProvider;
    }

MockHttpHandler& BuddiProviderTests::GetMockHttpHandler() const
    {
    return *m_httpHandlerMock;
    }

std::shared_ptr<MockHttpHandler> BuddiProviderTests::GetMockHttpHandlerPtr() const
    {
    return m_httpHandlerMock;
    }

//AuthHandlerProviderMock& PolicyProviderTests::GetAuthHandlerProviderMock() const
//    {
//    return *m_authProviderMock;
//    }
//
//std::shared_ptr<AuthHandlerProviderMock> PolicyProviderTests::GetAuthHandlerProviderMockPtr() const
//    {
//    return m_authProviderMock;
//    }

void BuddiProviderTests::TearDown()
    {
    m_httpHandlerMock->ValidateAndClearExpectations();
    }

void BuddiProviderTests::SetUpTestCase()
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
    path.AppendToPath(L"TestAssets/sqlang/DgnClientFx_en.sqlang.db3");

    ASSERT_EQ(SUCCESS, L10N::Initialize(BeSQLite::L10N::SqlangFiles(path)));
    }


TEST_F(BuddiProviderTests, UlasLocationBaseUrl_Success)
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    //RuntimeJsonLocalState* localState = new RuntimeJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, localState, nullptr, GetMockHttpHandlerPtr());

    Utf8PrintfString body(R"xml(<?xml version="1.0" encoding="utf-8"?>
                                <soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                                               xmlns:xsd="http://www.w3.org/2001/XMLSchema"
                                               xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
                                    <soap:Body>
                                        <GetUrlResponse xmlns="http://tempuri.org/">
                                          <GetUrlResult>%s</GetUrlResult>
                                        </GetUrlResponse>
                                    </soap:Body>
                                </soap:Envelope>)xml", MOCK_URL);

    GetMockHttpHandler().ExpectRequests(1);
    GetMockHttpHandler().ForRequest(1, [=](Http::RequestCR request)
        {
        //EXPECT_EQ(Utf8String(MOCK_URL), request.GetUrl());
        return MockHttpHandler::StubHttpResponse(body);
        });

    // assert that we return the result of the call on success
    const auto result = GetBuddiProvider().UlasLocationBaseUrl();
    EXPECT_EQ(Utf8String(MOCK_URL), result);
    }

TEST_F(BuddiProviderTests, EntitlementPolicyBaseUrl_Success)
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    //RuntimeJsonLocalState* localState = new RuntimeJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, localState, nullptr, GetMockHttpHandlerPtr());

    Utf8PrintfString body(R"xml(<?xml version="1.0" encoding="utf-8"?>
                                <soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                                               xmlns:xsd="http://www.w3.org/2001/XMLSchema"
                                               xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
                                    <soap:Body>
                                        <GetUrlResponse xmlns="http://tempuri.org/">
                                          <GetUrlResult>%s</GetUrlResult>
                                        </GetUrlResponse>
                                    </soap:Body>
                                </soap:Envelope>)xml", MOCK_URL);

    GetMockHttpHandler().ExpectRequests(1);
    GetMockHttpHandler().ForRequest(1, [=](Http::RequestCR request)
        {
        //EXPECT_EQ(Utf8String(MOCK_URL), request.GetUrl());
        return MockHttpHandler::StubHttpResponse(body);
        });

    // assert that we return the result of the call on success
    const auto result = GetBuddiProvider().EntitlementPolicyBaseUrl();
    EXPECT_EQ(Utf8String(MOCK_URL), result);
    }

TEST_F(BuddiProviderTests, UlasRealtimeLoggingBaseUrl_Success)
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    //RuntimeJsonLocalState* localState = new RuntimeJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, localState, nullptr, GetMockHttpHandlerPtr());

    Utf8PrintfString body(R"xml(<?xml version="1.0" encoding="utf-8"?>
                                <soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                                               xmlns:xsd="http://www.w3.org/2001/XMLSchema"
                                               xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
                                    <soap:Body>
                                        <GetUrlResponse xmlns="http://tempuri.org/">
                                          <GetUrlResult>%s</GetUrlResult>
                                        </GetUrlResponse>
                                    </soap:Body>
                                </soap:Envelope>)xml", MOCK_URL);

    GetMockHttpHandler().ExpectRequests(1);
    GetMockHttpHandler().ForRequest(1, [=](Http::RequestCR request)
        {
        //EXPECT_EQ(Utf8String(MOCK_URL), request.GetUrl());
        return MockHttpHandler::StubHttpResponse(body);
        });

    // assert that we return the result of the call on success
    const auto result = GetBuddiProvider().UlasRealtimeLoggingBaseUrl();
    EXPECT_EQ(Utf8String(MOCK_URL), result);
    }

TEST_F(BuddiProviderTests, UlasRealtimeFeatureUrl_Success)
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    //RuntimeJsonLocalState* localState = new RuntimeJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, localState, nullptr, GetMockHttpHandlerPtr());

    Utf8PrintfString body(R"xml(<?xml version="1.0" encoding="utf-8"?>
                                <soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                                               xmlns:xsd="http://www.w3.org/2001/XMLSchema"
                                               xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
                                    <soap:Body>
                                        <GetUrlResponse xmlns="http://tempuri.org/">
                                          <GetUrlResult>%s</GetUrlResult>
                                        </GetUrlResponse>
                                    </soap:Body>
                                </soap:Envelope>)xml", MOCK_URL);

    GetMockHttpHandler().ExpectRequests(1);
    GetMockHttpHandler().ForRequest(1, [=](Http::RequestCR request)
        {
        //EXPECT_EQ(Utf8String(MOCK_URL), request.GetUrl());
        return MockHttpHandler::StubHttpResponse(body);
        });

    // assert that we return the result of the call on success
    const auto result = GetBuddiProvider().UlasRealtimeFeatureUrl();
    EXPECT_EQ(Utf8String(MOCK_URL) + "/featureLog", result);
    }

TEST_F(BuddiProviderTests, UlasAccessKeyBaseUrl_Success)
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    //RuntimeJsonLocalState* localState = new RuntimeJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, localState, nullptr, GetMockHttpHandlerPtr());

    Utf8PrintfString body(R"xml(<?xml version="1.0" encoding="utf-8"?>
                                <soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                                               xmlns:xsd="http://www.w3.org/2001/XMLSchema"
                                               xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
                                    <soap:Body>
                                        <GetUrlResponse xmlns="http://tempuri.org/">
                                          <GetUrlResult>%s</GetUrlResult>
                                        </GetUrlResponse>
                                    </soap:Body>
                                </soap:Envelope>)xml", MOCK_URL);

    GetMockHttpHandler().ExpectRequests(1);
    GetMockHttpHandler().ForRequest(1, [=](Http::RequestCR request)
        {
        //EXPECT_EQ(Utf8String(MOCK_URL), request.GetUrl());
        return MockHttpHandler::StubHttpResponse(body);
        });

    // assert that we return the result of the call on success
    const auto result = GetBuddiProvider().UlasAccessKeyBaseUrl();
    EXPECT_EQ(Utf8String(MOCK_URL), result);
    }

