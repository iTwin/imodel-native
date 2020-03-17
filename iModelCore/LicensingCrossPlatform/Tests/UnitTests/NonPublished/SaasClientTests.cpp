/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "TestsHelper.h"
#include "JsonHelper.h"
#include "SaasClientTests.h"
#include "DummyPolicyHelper.h"
#include "DummyUserInfoHelper.h"

#include <Licensing/SaasClient.h>
#include <Licensing/Utils/DateHelper.h>
#include "../../../Licensing/SaasClientImpl.h"
#include "../../../Licensing/LicensingDb.h"

#include <BeHttp/HttpClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/L10N.h>
#include <fstream>
#include <Licensing/Utils/InMemoryJsonLocalState.h>
#include <Licensing/Utils/SCVWritter.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

#define TEST_PRODUCT_ID         "2545"
#define TEST_DEVICE_ID          "ATPDeviceID"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE


SaasClientImplPtr CreateTestSaasClient(IUlasProviderPtr ulasProvider, IEntitlementProviderPtr entitlementProvider, int productId = std::atoi(TEST_PRODUCT_ID))
    {
    return std::make_shared<SaasClientImpl>
        (
        productId,
        "",
        ulasProvider,
        entitlementProvider
        );
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SaasClientTests::SaasClientTests() :
    m_ulasProviderMock(std::make_shared<UlasProviderMock>()),
    m_entitlementProviderMock(std::make_shared<EntitlementProviderMock>())
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UlasProviderMock&  SaasClientTests::GetUlasProviderMock() const
    {
    return *m_ulasProviderMock;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<UlasProviderMock> SaasClientTests::GetUlasProviderMockPtr() const
    {
    return m_ulasProviderMock;
    }

EntitlementProviderMock& SaasClientTests::GetEntitlementProviderMock() const
{
    return *m_entitlementProviderMock;
}

std::shared_ptr<EntitlementProviderMock> SaasClientTests::GetEntitlementProviderMockPtr() const
{
    return m_entitlementProviderMock;
}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SaasClientTests::TearDown() { }

void SaasClientTests::SetUpTestCase()
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

    EXPECT_EQ(SUCCESS, L10N::Initialize(BeSQLite::L10N::SqlangFiles(path)));
    }

TEST_F(SaasClientTests, JsonExample)
    {
    BeFileName testJson;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testJson);
    testJson.AppendToPath(L"TestAssets/test.json");

    BeFileName test3Json;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(test3Json);
    test3Json.AppendToPath(L"TestAssets/test3.json");

    Json::Value test = ReadJsonFile(testJson);
    Json::Value test2 = ReadJsonFile(testJson);
    Json::Value test3 = ReadJsonFile(test3Json);

    int a = test.compare(test2);
    int b = test.compare(test3);
    int c = test3.compare(test2);

    EXPECT_EQ(a, 0);
    EXPECT_NE(b, 0);
    EXPECT_NE(c, 0);
    }

TEST_F(SaasClientTests, SaasClientTrackUsage_Success)
    {
    auto client = CreateTestSaasClient(GetUlasProviderMockPtr(), GetEntitlementProviderMockPtr());

    Utf8String accessToken = "TestToken";
    const auto version = BeVersion(1, 0);
    FeatureEvent featureEvent = FeatureEvent("TestFeatureId", version);

    GetUlasProviderMock().MockRealtimeTrackUsage(folly::Unit());
    WebEntitlementResult mocked{ std::atoi(TEST_PRODUCT_ID), LicenseStatus::Ok, "00000000-0000-0000-0000-000000000000" };
    GetEntitlementProviderMock().MockV4Result(mocked);

    Utf8String projectId = "00000000-0000-0000-0000-000000000000";
    std::vector<int> pList;
    pList.push_back((std::atoi(TEST_PRODUCT_ID)));
    EXPECT_LICENSE_SUCCESS(client->TrackUsage(accessToken, version, projectId, AuthType::OIDC, pList, TEST_DEVICE_ID, "").get());
    EXPECT_EQ(1, GetUlasProviderMock().RealtimeTrackUsageCalls());
    }

TEST_F(SaasClientTests, SaasClientTrackUsage_Success_Original)
{
    auto client = CreateTestSaasClient(GetUlasProviderMockPtr(), GetEntitlementProviderMockPtr());

    Utf8String accessToken = "TestToken";
    const auto version = BeVersion(1, 0);
    FeatureEvent featureEvent = FeatureEvent("TestFeatureId", version);

    GetUlasProviderMock().MockRealtimeTrackUsage(folly::Unit());
    //GetEntitlementProviderMock().MockV4Result(LicenseStatus::Ok);

    Utf8String projectId = "00000000-0000-0000-0000-000000000000";
 
    client->PostUserUsage(accessToken, version, projectId, AuthType::OIDC, std::atoi(TEST_PRODUCT_ID), "", UsageType::Production, "", "").get();
    EXPECT_EQ(1, GetUlasProviderMock().RealtimeTrackUsageCalls());
}

TEST_F(SaasClientTests, SaasClientMarkFeatureNoDataNoProject_Success)
    {
    auto client = CreateTestSaasClient(GetUlasProviderMockPtr(), GetEntitlementProviderMockPtr());

    Utf8String accessToken = "TestToken";
    const auto version = BeVersion(1, 0);
    FeatureEvent featureEvent = FeatureEvent("TestFeatureId", version);

    GetUlasProviderMock().MockRealtimeMarkFeature(folly::Unit());

    client->PostFeatureUsage(accessToken, featureEvent, AuthType::OIDC, std::atoi(TEST_PRODUCT_ID), "", UsageType::Production, "", "").get();
    EXPECT_EQ(1, GetUlasProviderMock().RealtimeMarkFeatureCalls());
    }
