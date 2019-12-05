/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "TestsHelper.h"
#include "EntitlementProviderTests.h"
#include "Utils/MockHttpHandler.h"
#include "DummyPolicyHelper.h"

#include <Licensing/Client.h>
#include <Licensing/SaasClient.h>
#include <Licensing/Utils/DateHelper.h>
#include <Licensing/AuthType.h>
#include "../../../Licensing/Providers/EntitlementProvider.h"

#include <BeHttp/HttpClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/L10N.h>
#include <fstream>
#include <vector>
#include <Licensing/Utils/InMemoryJsonLocalState.h>
#include <Licensing/Utils/SCVWritter.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

#include "Mocks/BuddiProviderMock.h"

#define TEST_PRODUCT_ID     "2545"
#define TEST_ULT_ID         "1001389117"
#define TEST_DEVICE_ID      "TestDeviceId" 
#define TEST_DEVICE_SID     "IXravQ3f71wUupkp+tLBK+vGmCc="

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE

EntitlementProviderTests::EntitlementProviderTests() :
    m_httpHandlerMock(std::make_shared<MockHttpHandler>()),
    m_buddiMock(std::make_shared<BuddiProviderMock>()),
    m_authProviderMock(std::make_shared<AuthHandlerProviderMock>())
    {
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(1, 0), "TestDeviceId", "2545");

    GetAuthHandlerProviderMock().MockGetAuthHandler(m_httpHandlerMock);

    m_entitlementProvider = std::make_shared<EntitlementProvider>(m_buddiMock, m_httpHandlerMock);
    }

EntitlementProvider& EntitlementProviderTests::GetEntitlementProvider() const
    {
    return *m_entitlementProvider;
    }

MockHttpHandler& EntitlementProviderTests::GetMockHttpHandler() const
    {
    return *m_httpHandlerMock;
    }

std::shared_ptr<MockHttpHandler> EntitlementProviderTests::GetMockHttpHandlerPtr() const
    {
    return m_httpHandlerMock;
    }

AuthHandlerProviderMock& EntitlementProviderTests::GetAuthHandlerProviderMock() const
    {
    return *m_authProviderMock;
    }

std::shared_ptr<AuthHandlerProviderMock> EntitlementProviderTests::GetAuthHandlerProviderMockPtr() const
    {
    return m_authProviderMock;
    }

void EntitlementProviderTests::TearDown()
    {
    m_httpHandlerMock->ValidateAndClearExpectations();
    }

void EntitlementProviderTests::SetUpTestCase()
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

BuddiProviderMock& EntitlementProviderTests::GetMockBuddi() const
    {
    return *m_buddiMock;
    } 

Utf8String EntitlementProviderTests::MockEntitlementUrl()
    {
    Utf8String mockUrl("https://entitlementmockurl.bentley.com/");

    GetMockBuddi().MockEntitlementPolicyBaseUrl(mockUrl);

    return mockUrl;
    }

Utf8String GetMockWebEntitlementBody(std::vector<int> productIds, Utf8StringCR mockPrincipalId)
	{
	Json::Value responseBody(Json::objectValue);
	Json::Value result(Json::objectValue);
	Json::Value entitlements(Json::arrayValue);

	for (auto id : productIds)
		{
		Json::Value entitlement(Json::objectValue);
		entitlement["ProductId"] = id;
		entitlement["FeatureString"] = "";
		entitlement["LicenseStatus"] = "Allowed";
		entitlement["PrincipalId"] = Utf8String(mockPrincipalId.c_str());
		entitlements.append(entitlement);
		}
	result["Entitlements"] = entitlements;
	responseBody["result"] = result;
	return responseBody.ToString();
	}

TEST_F(EntitlementProviderTests, FetchWebEntitlementV4_Success)
    {
	auto mockUrl = MockEntitlementUrl();
	mockUrl.ReplaceAll("/api", "");
	Utf8String expectedUrl = mockUrl + "/v4.0/api/WebEntitlement";
	std::vector<int> mockProductIds{ 1000, 1001, 1002 };
	auto mockPrincipalId = Utf8String("9853E17E-94AE-46A7-825C-155AF41C589F");

    GetMockHttpHandler().ExpectRequests(1);
    GetMockHttpHandler().ForRequest(1, [=] (Http::RequestCR request)
        {
		// assert our outgoing request matches expectation
        EXPECT_EQ(expectedUrl, request.GetUrl());
        return MockHttpHandler::StubHttpResponse(GetMockWebEntitlementBody(mockProductIds, mockPrincipalId));
        });

    // assert that we return the result of the call on success
    const auto result = GetEntitlementProvider().FetchWebEntitlementV4(mockProductIds, BeVersion(1, 0), "device-id", "00000000-0000-0000-0000-000000000000", "mocked-token").get();
    EXPECT_EQ(LicenseStatus::Ok, result.Status);
	EXPECT_STREQ(mockPrincipalId.c_str(), result.PrincipalId.c_str());
    }

// TODO more tests for making sure it picks the correct productId and handles other statuses

// TODO we should catch and return another reponse

TEST_F(EntitlementProviderTests, FetchWebEntitlementV4_Failure)
    {
    // assert that we we fall into catch on non-success status code
    auto mockUrl = MockEntitlementUrl();
	mockUrl.ReplaceAll("/api", "");
    Utf8String expectedUrl = mockUrl + "/v4.0/api/WebEntitlement";
	std::vector<int> mockProductIds{ 1000, 1001, 1002 };

    GetMockHttpHandler().ExpectRequests(1);
    GetMockHttpHandler().ForRequest(1, [=](Http::RequestCR request)
        {
        EXPECT_EQ(expectedUrl, request.GetUrl());
        return MockHttpHandler::StubHttpFailureResponse();
        });

    try
        {
		const auto result = GetEntitlementProvider().FetchWebEntitlementV4(mockProductIds, BeVersion(1, 0), "device-id", "00000000-0000-0000-0000-000000000000", "mocked-token").get();
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
