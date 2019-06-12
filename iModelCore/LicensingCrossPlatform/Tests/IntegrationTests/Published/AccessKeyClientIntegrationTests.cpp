/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Published/AccessKeyClientIntegrationTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "AccessKeyClientIntegrationTests.h"
#include "TestsHelper.h"
#include "DummyPolicyHelper.h"

#include <Licensing/Utils/DateHelper.h>
#include "../../../Licensing/LicensingDb.h"
#include "../../../PublicAPI/Licensing/Utils/SCVWritter.h"

//#include "../../../Licensing/Providers/IBuddiProvider.h"
//#include "../../../Licensing/Providers/IPolicyProvider.h"
//#include "../../../Licensing/Providers/IUlasProvider.h"

#include "../../../Licensing/Providers/BuddiProvider.h"
#include "../../../Licensing/Providers/PolicyProvider.h"
#include "../../../Licensing/Providers/UlasProvider.h"

#include <BeHttp/HttpClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/L10N.h>
#include <fstream>
#include <Licensing/Utils/InMemoryJsonLocalState.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

#define TEST_ACCESSKEY_PRODUCT_ID      "1000"
#define TEST_VALID_ACCESSKEY "3469AD8D095A53F3CBC9A905A8FF8926"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_INTEGRATION_TESTS
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE

BeFileName AccessKeyClientIntegrationTests::GetLicensingDbPathIntegration() const
    {
    BeFileName path;
    BeTest::GetHost().GetTempDir(path);
    path.AppendToPath(L"License.db");

    BeFileName hardCodedPath("c:\\dev\\License.db");

    return hardCodedPath;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AccessKeyClientIntegrationTests::AccessKeyClientIntegrationTests() {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AccessKeyClientIntegrationTests::TearDown() {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AccessKeyClientIntegrationTests::SetUpTestCase()
    {
    // This is only an example of how to set logging severity and see info logs. Usually should be set more globally than in TestCase SetUp
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_INFO);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_DEBUG);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_TRACE);

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

AccessKeyClientPtr AccessKeyClientIntegrationTests::CreateTestClient(Utf8StringCR productId, Utf8StringCR accessKey) const
    {
    //InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    RuntimeJsonLocalState* localState = new RuntimeJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, localState);
 
    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    BeFileName dbPath = GetLicensingDbPathIntegration();

    return AccessKeyClient::Create
        (
        accessKey,
        clientInfo,
        dbPath,
        true,
        "",
        "",
        proxy
        );
    }

AccessKeyClientImplPtr AccessKeyClientIntegrationTests::CreateTestClientImpl(Utf8StringCR productId, Utf8StringCR accessKey) const
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, localState);

    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    BeFileName dbPath = GetLicensingDbPathIntegration();

    ApplicationInfoPtr applicationInfo = std::make_shared<ApplicationInfo>(clientInfo->GetApplicationVersion(), clientInfo->GetDeviceId(), clientInfo->GetApplicationProductId());
    IBuddiProviderPtr buddiProvider = std::make_shared<BuddiProvider>();
    IPolicyProviderPtr policyProvider = std::make_shared<PolicyProvider>(buddiProvider, applicationInfo, proxy, AuthType::None);
    IUlasProviderPtr ulasProvider = std::make_shared<UlasProvider>(buddiProvider, proxy);

    return std::make_shared<AccessKeyClientImpl>
        (
        accessKey,
        applicationInfo,
        dbPath,
        true,
        policyProvider,
        ulasProvider,
        "",
        "",
        nullptr
        );
    }

TEST_F(AccessKeyClientIntegrationTests, FactoryStartStopApplication_Success)
    {
    auto client = CreateTestClient(TEST_ACCESSKEY_PRODUCT_ID, TEST_VALID_ACCESSKEY);
    ASSERT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(AccessKeyClientIntegrationTests, StartStopApplication_Success)
    {
    auto client = CreateTestClientImpl(TEST_ACCESSKEY_PRODUCT_ID, TEST_VALID_ACCESSKEY);
    ASSERT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(AccessKeyClientIntegrationTests, StartApplicationInvalidKey_Success)
    {
    // note: expired access keys are cleaned occasionaly by ULAS, so this is a non-expired key where "active" = false
    auto client = CreateTestClientImpl(TEST_ACCESSKEY_PRODUCT_ID, "F1F5BAC030C7CF3F472655412617CF5D");

    // inactive key will give a nullptr policy, so StartApplication will fail
    ASSERT_EQ(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));
    }

TEST_F(AccessKeyClientIntegrationTests, MarkFeature_Success)
    {
    auto client = CreateTestClientImpl(TEST_ACCESSKEY_PRODUCT_ID); // TEST_VALID_ACCESSKEY is entitled to microstation (1000)

    FeatureUserDataMapPtr featureData = std::make_shared<FeatureUserDataMap>();

    featureData->AddAttribute("Manufacturer", "Bentley Systems, Inc.");
    featureData->AddAttribute("Website", "https://www.w3schools.com");
    featureData->AddAttribute("Title", "Mobile App");

    ASSERT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));
    EXPECT_EQ(static_cast<int>(client->MarkFeature("TestFeatureId", featureData)), static_cast<int>(BentleyStatus::SUCCESS));
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(AccessKeyClientIntegrationTests, GetLicenseStatusValidPolicy_Success)
    {
    auto client = CreateTestClientImpl(TEST_ACCESSKEY_PRODUCT_ID); // TEST_VALID_ACCESSKEY is entitled to microstation (1000)

    ASSERT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));

    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Ok));
    EXPECT_SUCCESS(client->StopApplication());
    }

// The following are tests we use to verify behavior, not meant to be run on a regular basis

TEST_F(AccessKeyClientIntegrationTests, AccessKeyClientTestPolicyHeartbeat_Test)
    {
    // I am using this test to manually debug/test the heartbeat to make sure of the following:
    // - policy heartbeat does in fact run as expected (heartbeat every 1 second, refresh policy by PolicyInterval)
    // - policy heartbeat cleans up as expected (after StopApplication is called, it doens't try to access disposed resources)
    // - maybe: policy heartbeat handles going offline well (keeps using old policy and access key until policy expires)
    auto client = CreateTestClient();
    ASSERT_NE((int)client->StartApplication(), (int)LicenseStatus::Error);

    // use std::chrono::seconds(2); to wait in tests
    // PolicyRefresh: make a custom policy with a fast refresh! -> right now it is set for 60 days!
    // Heartbeat: check that function is entered multiple times [GOOD]
    // Cleanup: stop application and wait and check if anything is accessed [double check!]

    std::this_thread::sleep_for(std::chrono::seconds(10));

    EXPECT_SUCCESS(client->StopApplication());

    std::this_thread::sleep_for(std::chrono::seconds(10));

    EXPECT_EQ(1, 0);
    }

//TEST_F(AccessKeyClientIntegrationTests, OfflinePolicyHeartbeat_Test)
//    {
//    // I am using this test to manually debug/test the heartbeat to make sure of the following:
//    // - policy heartbeat does in fact run as expected (heartbeat every 1 second, refresh policy by PolicyInterval)
//    // - policy heartbeat cleans up as expected (after StopApplication is called, it doens't try to access disposed resources)
//
//    auto client = CreateTestClientImpl(TEST_ACCESSKEY_PRODUCT_ID, TEST_VALID_ACCESSKEY);
//
//    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa"; // shouldn't need this
//    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFullWithKey(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_ACCESSKEY_PRODUCT_ID), "", 1, false, TEST_VALID_ACCESSKEY);
//    auto validPolicy = Policy::Create(jsonPolicyValid);
//    client->AddPolicyToDb(validPolicy);
//
//    ASSERT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));
//
//    std::this_thread::sleep_for(std::chrono::seconds(10));
//
//    EXPECT_SUCCESS(client->StopApplication());
//
//    std::this_thread::sleep_for(std::chrono::seconds(10));
//
//    }
