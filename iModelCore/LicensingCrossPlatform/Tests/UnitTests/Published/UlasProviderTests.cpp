/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/UlasProviderTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UlasProviderTests.h"
#include "DummyPolicyHelper.h"
#include "JsonHelper.h"

#include <BeHttp/HttpClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/L10N.h>

#define TEST_PRODUCT_ID     "2545"
#define TEST_ULT_ID         "1001389117"
#define TEST_DEVICE_ID      "TestDeviceId" 
#define TEST_DEVICE_SID     "IXravQ3f71wUupkp+tLBK+vGmCc="

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE

UlasProviderTests::UlasProviderTests() :
    m_handlerMock(std::make_shared<MockHttpHandler>()),
    m_buddiMock(std::make_shared<BuddiProviderMock>()),
    m_licensingDbMock(std::make_shared<LicensingDbMock>())
    {
    m_ulasProvider = std::make_shared<UlasProvider>(m_buddiMock, m_handlerMock);
    }

UlasProvider& UlasProviderTests::GetUlasProvider() const
    {
    return *m_ulasProvider;
    }

LicensingDbMock&  UlasProviderTests::GetLicensingDbMock() const
    {
    return *m_licensingDbMock;
    }

std::shared_ptr<LicensingDbMock> UlasProviderTests::GetLicensingDbMockPtr() const
    {
    return m_licensingDbMock;
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
    path.AppendToPath(L"TestAssets/sqlang/DgnClientFx_en.sqlang.db3");

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

TEST_F(UlasProviderTests, PostUsageLogs_Success)
    {
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(1, 0), "TestDeviceId", TEST_PRODUCT_ID);
    BeFileName dbPath("TestPath");
    auto userId = "UserId";
    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    auto validPolicy = Policy::Create(jsonPolicyValid);

    GetLicensingDbMock().MockWriteUsageToCSVFile(SUCCESS);
    GetLicensingDbMock().MockCleanUpUsages(SUCCESS);

    const auto mockUrl = MockUlasUrl();
    Utf8String expectedUrl = mockUrl + Utf8PrintfString("/usageLog?ultId=%s&prdId=%s&lng=%s", "1004175881", TEST_PRODUCT_ID, "en");

    const auto epUri = "https://locationmockurl.bentley.com/";
    const auto sharedAccessSignature = "MockSharedAccessSignature";
    Utf8String expectedUploadUrl = epUri + Utf8PrintfString(sharedAccessSignature);

    // does not call SendUsageLogs since there are no logs in the CSV
    GetUlasProvider().PostUsageLogs(appInfo, dbPath, GetLicensingDbMock(), validPolicy);

    EXPECT_EQ(1, GetLicensingDbMock().WriteUsageToCSVFileCount());
    EXPECT_EQ(1, GetLicensingDbMock().CleanUpUsagesCount());
    }

TEST_F(UlasProviderTests, SendUsageLogs_Success)
    {
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(1, 0), "TestDeviceId", TEST_PRODUCT_ID);
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

    GetUlasProvider().SendUsageLogs(appInfo, BeFileName("TestName"), Utf8String("1004175881")).get();
    }

TEST_F(UlasProviderTests, SendUsageLogs_Failure)
    {
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(1, 0), "TestDeviceId", TEST_PRODUCT_ID);
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
        return MockHttpHandler::StubHttpFailureResponse();
        });
    try
        {
        const auto result = GetUlasProvider().SendUsageLogs(appInfo, BeFileName("TestName"), Utf8String("1004175881")).get();
        FAIL() << "Expected an execption to be thrown";
        }
    catch (HttpError error)
        {
        // expect connection and http status to be the same as StubHttpFailureResponse
        EXPECT_EQ(error.GetConnectionStatus(), MockHttpHandler::StubHttpFailureResponse().GetConnectionStatus());
        EXPECT_EQ(error.GetHttpStatus(), MockHttpHandler::StubHttpFailureResponse().GetHttpStatus());
        }
    catch (...)
        {
        FAIL() << "Expected exception to be an HttpError";
        }
    }

TEST_F(UlasProviderTests, PostFeatureLogs_Success)
    {
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(1, 0), "TestDeviceId", TEST_PRODUCT_ID);
    BeFileName dbPath("TestPath");
    auto userId = "UserId";
    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    auto validPolicy = Policy::Create(jsonPolicyValid);

    GetLicensingDbMock().MockWriteFeatureToCSVFile(SUCCESS);
    GetLicensingDbMock().MockCleanUpFeatures(SUCCESS);

    const auto mockUrl = MockUlasUrl();
    Utf8String expectedUrl = mockUrl + Utf8PrintfString("/featureLog?ultId=%s&prdId=%s&lng=%s", "1004175881", TEST_PRODUCT_ID, "en");

    const auto epUri = "https://locationmockurl.bentley.com/";
    const auto sharedAccessSignature = "MockSharedAccessSignature";
    Utf8String expectedUploadUrl = epUri + Utf8PrintfString(sharedAccessSignature);

    // does not call SendFeatureLogs since there are no logs in the CSV
    GetUlasProvider().PostFeatureLogs(appInfo, dbPath, GetLicensingDbMock(), validPolicy);

    EXPECT_EQ(1, GetLicensingDbMock().WriteFeatureToCSVFileCount());
    EXPECT_EQ(1, GetLicensingDbMock().CleanUpFeaturesCount());
    }

TEST_F(UlasProviderTests, SendFeatureLogs_Success)
    {
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(1, 0), "TestDeviceId", TEST_PRODUCT_ID);
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

    GetUlasProvider().SendFeatureLogs(appInfo, BeFileName("TestName"), Utf8String("1004175881")).get();
    }

TEST_F(UlasProviderTests, SendFeatureLogs_Failure)
    {
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(1, 0), "TestDeviceId", TEST_PRODUCT_ID);
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
        return MockHttpHandler::StubHttpFailureResponse();
        });

    try
        {
        const auto result = GetUlasProvider().SendFeatureLogs(appInfo, BeFileName("TestName"), Utf8String("1004175881")).get();
        FAIL() << "Expected an execption to be thrown";
        }
    catch (HttpError error)
        {
        // expect connection and http status to be the same as StubHttpFailureResponse
        EXPECT_EQ(error.GetConnectionStatus(), MockHttpHandler::StubHttpFailureResponse().GetConnectionStatus());
        EXPECT_EQ(error.GetHttpStatus(), MockHttpHandler::StubHttpFailureResponse().GetHttpStatus());
        }
    catch (...)
        {
        FAIL() << "Expected exception to be an HttpError";
        }
    }

TEST_F(UlasProviderTests, RealtimeTrackUsage_Success)
    {
    Utf8String mockUrl("https://ulasmockurl.bentley.com");

    GetMockBuddi().MockUlasRealtimeLoggingBaseUrl(mockUrl);

    GetMockHttp().ExpectRequests(1);
    // return a mock location response
    GetMockHttp().ForRequest(1, [=](Http::RequestCR request)
        {
        EXPECT_EQ(mockUrl, request.GetUrl());

        return MockHttpHandler::StubHttpResponse();
        });

    EXPECT_SUCCESS(GetUlasProvider().RealtimeTrackUsage("AccessToken", std::atoi(TEST_PRODUCT_ID), "", "DeviceId", BeVersion(1, 0), "ProjectId", UsageType::Production, "", AuthType::OIDC).get());
    }

TEST_F(UlasProviderTests, RealtimeTrackUsage_Failure)
    {
    Utf8String mockUrl("https://ulasmockurl.bentley.com");

    GetMockBuddi().MockUlasRealtimeLoggingBaseUrl(mockUrl);

    GetMockHttp().ExpectRequests(1);
    // return a mock location response
    GetMockHttp().ForRequest(1, [=](Http::RequestCR request)
        {
        EXPECT_EQ(mockUrl, request.GetUrl());

        return MockHttpHandler::StubHttpFailureResponse();
        });

    EXPECT_ERROR(GetUlasProvider().RealtimeTrackUsage("AccessToken", std::atoi(TEST_PRODUCT_ID), "", "DeviceId", BeVersion(1, 0), "ProjectId", UsageType::Production, "", AuthType::OIDC).get());
    }

TEST_F(UlasProviderTests, RealtimeTrackUsageNoFeatureUserData_Success)
    {
    const auto version = BeVersion(1, 0);
    FeatureEvent featureEvent = FeatureEvent("TestFeatureId", version);

    Utf8String mockUrl("https://ulasmockurl.bentley.com/feature");

    GetMockBuddi().MockUlasRealtimeFeatureUrl(mockUrl);

    GetMockHttp().ExpectRequests(1);
    // return a mock location response
    GetMockHttp().ForRequest(1, [=](Http::RequestCR request)
        {
        EXPECT_EQ(mockUrl, request.GetUrl());

        return MockHttpHandler::StubHttpResponse();
        });

    EXPECT_SUCCESS(GetUlasProvider().RealtimeMarkFeature("AccessToken", featureEvent, std::atoi(TEST_PRODUCT_ID), "", "DeviceId", UsageType::Production, "", AuthType::OIDC).get());
    }

TEST_F(UlasProviderTests, RealtimeTrackUsageNoFeatureUserData_Failure)
    {
    const auto version = BeVersion(1, 0);
    FeatureEvent featureEvent = FeatureEvent("TestFeatureId", version);

    Utf8String mockUrl("https://ulasmockurl.bentley.com/feature");

    GetMockBuddi().MockUlasRealtimeFeatureUrl(mockUrl);

    GetMockHttp().ExpectRequests(1);
    // return a mock location response
    GetMockHttp().ForRequest(1, [=](Http::RequestCR request)
        {
        EXPECT_EQ(mockUrl, request.GetUrl());

        return MockHttpHandler::StubHttpFailureResponse();
        });

    EXPECT_ERROR(GetUlasProvider().RealtimeMarkFeature("AccessToken", featureEvent, std::atoi(TEST_PRODUCT_ID), "", "DeviceId", UsageType::Production, "", AuthType::OIDC).get());
    }

TEST_F(UlasProviderTests, RealtimeTrackUsageWithUserData_Success)
    {
    const auto version = BeVersion(1, 0);

    FeatureUserDataMapPtr featureAttribute = std::make_shared<FeatureUserDataMap>();
    featureAttribute->AddAttribute("Manufacturer", "Bentley Systems, Inc.");
    featureAttribute->AddAttribute("Website", "https://www.w3schools.com");
    featureAttribute->AddAttribute("Title", "Mobile App");
    FeatureEvent featureEvent = FeatureEvent(Utf8String("TestFeatureId"), version, featureAttribute);

    Utf8String mockUrl("https://ulasmockurl.bentley.com/feature");

    GetMockBuddi().MockUlasRealtimeFeatureUrl(mockUrl);

    GetMockHttp().ExpectRequests(1);
    // return a mock location response
    GetMockHttp().ForRequest(1, [=](Http::RequestCR request)
        {
        EXPECT_EQ(mockUrl, request.GetUrl());

        return MockHttpHandler::StubHttpResponse();
        });

    EXPECT_SUCCESS(GetUlasProvider().RealtimeMarkFeature("AccessToken", featureEvent, std::atoi(TEST_PRODUCT_ID), "", "DeviceId", UsageType::Production, "", AuthType::OIDC).get());
    }

TEST_F(UlasProviderTests, GetAccessKeyInfo_Success)
    {
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(1, 0), "TestDeviceId", TEST_PRODUCT_ID);
    Utf8String accessKey = "TestAccessKey";
    Utf8String mockUrl("https://ulasaccesskeymockurl.bentley.com");

    BeFileName testJsonFile;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testJsonFile);
    testJsonFile.AppendToPath(L"TestAssets/test.json");

    Json::Value testJson = ReadJsonFile(testJsonFile);
    Utf8String testJsonString = testJson.asString();

    GetMockBuddi().MockUlasAccessKeyBaseUrl(mockUrl);
    Utf8String expectedUrl = mockUrl + "/info";

    GetMockHttp().ExpectRequests(1);
    // return a mock location response
    GetMockHttp().ForRequest(1, [=](Http::RequestCR request)
        {
        EXPECT_EQ(expectedUrl, request.GetUrl());
		auto requeststr = request.GetRequestBody()->AsString();
		Json::Value requestJSON = Json::Reader::DoParse(requeststr);
		EXPECT_STREQ(TEST_DEVICE_SID, requestJSON["cSID"].asCString());
        return MockHttpHandler::StubHttpResponse(testJsonString);
        });

    EXPECT_NE(Json::Value::GetNull(), GetUlasProvider().GetAccessKeyInfo(appInfo, accessKey, "").get());
    }

TEST_F(UlasProviderTests, GetAccessKeyInfo_SuccessAgnostic)
{
	auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(1, 0), "TestDeviceId", TEST_PRODUCT_ID);
	Utf8String accessKey = "TestAccessKey";
	Utf8String mockUrl("https://ulasaccesskeymockurl.bentley.com");

	BeFileName testJsonFile;
	BeTest::GetHost().GetDgnPlatformAssetsDirectory(testJsonFile);
	testJsonFile.AppendToPath(L"TestAssets/test.json");

	Json::Value testJson = ReadJsonFile(testJsonFile);
	Utf8String testJsonString = testJson.asString();

	GetMockBuddi().MockUlasAccessKeyBaseUrl(mockUrl);
	Utf8String expectedUrl = mockUrl + "/info";

	GetMockHttp().ExpectRequests(1);
	// return a mock location response
	GetMockHttp().ForRequest(1, [=](Http::RequestCR request)
	{
		EXPECT_EQ(expectedUrl, request.GetUrl());
		auto requeststr = request.GetRequestBody()->AsString();
		Json::Value requestJSON = Json::Reader::DoParse(requeststr);		
		EXPECT_STREQ(TEST_ULT_ID, requestJSON["cSID"].asCString());
		return MockHttpHandler::StubHttpResponse(testJsonString);
	});

	EXPECT_NE(Json::Value::GetNull(), GetUlasProvider().GetAccessKeyInfo(appInfo, accessKey, TEST_ULT_ID).get());
}

// TODO: add a GetAccessKeyInfo success test with ultimate ID

TEST_F(UlasProviderTests, GetAccessKeyInfo_Failure)
    {
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(1, 0), "TestDeviceId", TEST_PRODUCT_ID);
    Utf8String accessKey = "TestAccessKey";
    Utf8String mockUrl("https://ulasaccesskeymockurl.bentley.com");

    //BeFileName testJsonFile;
    //BeTest::GetHost().GetDgnPlatformAssetsDirectory(testJsonFile);
    //testJsonFile.AppendToPath(L"TestAssets/test.json");

    //Json::Value testJson = ReadJsonFile(testJsonFile);
    //Utf8String testJsonString = testJson.asString();

    GetMockBuddi().MockUlasAccessKeyBaseUrl(mockUrl);
    Utf8String expectedUrl = mockUrl + "/info";

    GetMockHttp().ExpectRequests(1);
    // return a mock location response
    GetMockHttp().ForRequest(1, [=](Http::RequestCR request)
        {
        EXPECT_EQ(expectedUrl, request.GetUrl());

        return MockHttpHandler::StubHttpFailureResponse();
        });

    EXPECT_EQ(Json::Value::GetNull(), GetUlasProvider().GetAccessKeyInfo(appInfo, accessKey, "").get());
    }

// TODO: add a GetAccessKeyInfo failure test with invalid ultimate ID?

