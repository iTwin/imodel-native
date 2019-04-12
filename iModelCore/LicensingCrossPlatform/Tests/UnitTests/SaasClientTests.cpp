/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/SaasClientTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestsHelper.h"
#include "JsonHelper.h"
#include "SaasClientTests.h"
#include "DummyPolicyHelper.h"
#include "DummyUserInfoHelper.h"

#include <Licensing/SaasClient.h>
#include <Licensing/Utils/DateHelper.h>
#include "../../Licensing/SaasClientImpl.h"
#include "../../Licensing/LicensingDb.h"
#include "../../PublicAPI/Licensing/Utils/SCVWritter.h"

#include <BeHttp/HttpClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/L10N.h>
#include <fstream>
#include <Licensing/Utils/InMemoryJsonLocalState.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

#define TEST_PRODUCT_ID         "2545"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE


SaasClientImplPtr CreateTestSaasClient(IUlasProviderPtr ulasProvider, int productId = std::atoi(TEST_PRODUCT_ID))
    {
    return std::make_shared<SaasClientImpl>
        (
        productId,
        "",
        ulasProvider
        );
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SaasClientTests::SaasClientTests() :
    m_ulasProviderMock(std::make_shared<UlasProviderMock>())
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
    auto client = CreateTestSaasClient(GetUlasProviderMockPtr());

    Utf8String accessToken = "TestToken";
    const auto version = BeVersion(1, 0);
    FeatureEvent featureEvent = FeatureEvent("TestFeatureId", version);

    BentleyStatus status = BentleyStatus::SUCCESS;

    GetUlasProviderMock().MockRealtimeTrackUsage(BentleyStatus::SUCCESS);

    Utf8String projectId = "00000000-0000-0000-0000-000000000000";
    EXPECT_SUCCESS(client->TrackUsage(accessToken, version, projectId).get());
    EXPECT_EQ(1, GetUlasProviderMock().RealtimeTrackUsageCalls());
    }

TEST_F(SaasClientTests, SaasClientMarkFeatureNoDataNoProject_Success)
    {
    auto client = CreateTestSaasClient(GetUlasProviderMockPtr());

    Utf8String accessToken = "TestToken";
    const auto version = BeVersion(1, 0);
    FeatureEvent featureEvent = FeatureEvent("TestFeatureId", version);

    BentleyStatus status = BentleyStatus::SUCCESS;

    GetUlasProviderMock().MockRealtimeMarkFeature(BentleyStatus::SUCCESS);

    EXPECT_SUCCESS(client->MarkFeature(accessToken, featureEvent).get());
    EXPECT_EQ(1, GetUlasProviderMock().RealtimeMarkFeatureCalls());
    }
