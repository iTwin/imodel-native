/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/ClientIntegrationTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ClientIntegrationTests.h"
#include "TestsHelper.h"
#include "../UnitTests/DummyPolicyHelper.h" // TODO: copy this to Integration tests project?

#include <Licensing/Utils/DateHelper.h>
#include "../../Licensing/LicensingDb.h"
#include "../../PublicAPI/Licensing/Utils/SCVWritter.h"

#include "../../Licensing/Providers/IAuthHandlerProvider.h"
#include "../../Licensing/Providers/IBuddiProvider.h"
#include "../../Licensing/Providers/IPolicyProvider.h"
#include "../../Licensing/Providers/IUlasProvider.h"

#include "../../Licensing/Providers/AuthHandlerProvider.h"
#include "../../Licensing/Providers/BuddiProvider.h"
#include "../../Licensing/Providers/PolicyProvider.h"
#include "../../Licensing/Providers/UlasProvider.h"

#include <BeHttp/HttpClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/L10N.h>
#include <fstream>
#include <Licensing/Utils/InMemoryJsonLocalState.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

#define TEST_PRODUCT_ID      "2545"
#define TEST_VALID_ACCESSKEY "3469AD8D095A53F3CBC9A905A8FF8926"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_INTEGRATION_TESTS
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE

struct TestTimeRetriever : ITimeRetriever
    {
public:
    int64_t time = 0;

public:
    static std::shared_ptr<TestTimeRetriever> Create()
        {
        return std::shared_ptr<TestTimeRetriever>(new TestTimeRetriever());
        }

    virtual int64_t GetCurrentTimeAsUnixMillis() override
        {
        return time;
        }
    };

struct TestDelayedExecutor : IDelayedExecutor
    {
private:
    std::queue<folly::Promise<folly::Unit>> m_promises;

public:
    static std::shared_ptr<TestDelayedExecutor> Create()
        {
        return std::shared_ptr<TestDelayedExecutor>(new TestDelayedExecutor());
        }

    virtual folly::Future<folly::Unit> Delayed(uint64_t ms) override
        {
        m_promises.emplace();
        return m_promises.back().getFuture();
        }

    void Execute()
        {
        m_promises.front().setValue();
        m_promises.pop();
        }
    };

BeFileName GetLicensingDbPathIntegration()
    {
    BeFileName path;
    BeTest::GetHost().GetTempDir(path);
    path.AppendToPath(L"License.db");

    return path;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClientIntegrationTests::ClientIntegrationTests() {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientIntegrationTests::TearDown() {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientIntegrationTests::SetUpTestCase()
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

ClientPtr ClientIntegrationTests::CreateTestClient(bool signIn, Utf8StringCR productId) const
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, localState);

    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    auto manager = ConnectSignInManager::Create(clientInfo, proxy, localState);
    if (signIn)
        {
        Credentials credentials("qa2_devuser2@mailinator.com", "bentley");
        if (!manager->SignInWithCredentials(credentials)->GetResult().IsSuccess())
            return nullptr;
        }
    BeFileName dbPath = GetLicensingDbPathIntegration();

    return Client::Create
        (
        manager->GetUserInfo(),
        clientInfo,
        manager,
        dbPath,
        true,
        "",
        "",
        proxy
        );
    }

AccessKeyClientPtr ClientIntegrationTests::CreateTestAccessKeyClient(Utf8StringCR productId, Utf8StringCR accessKey) const
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    //RuntimeJsonLocalState* localState = new RuntimeJsonLocalState();
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

SaasClientPtr ClientIntegrationTests::CreateTestSaasClient(int productId) const
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, localState);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    return SaasClient::Create
        (
        productId,
        "",
        proxy
        );
    }


ClientImplPtr ClientIntegrationTests::CreateTestClientImpl(bool signIn, Utf8StringCR productId) const
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, localState);

    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    auto manager = ConnectSignInManager::Create(clientInfo, proxy, localState);
    if (signIn)
        {
        Credentials credentials("qa2_devuser2@mailinator.com", "bentley");
        if (!manager->SignInWithCredentials(credentials)->GetResult().IsSuccess())
            return nullptr;
        }
    BeFileName dbPath = GetLicensingDbPathIntegration();

    AuthType authType = AuthType::SAML;

    IBuddiProviderPtr buddiProvider = std::make_shared<BuddiProvider>();
    IAuthHandlerProviderPtr authHandlerProvider = std::make_shared<AuthHandlerProvider>(manager, proxy);
    IPolicyProviderPtr policyProvider = std::make_shared<PolicyProvider>(buddiProvider, clientInfo, proxy, authType, authHandlerProvider);
    IUlasProviderPtr ulasProvider = std::make_shared<UlasProvider>(buddiProvider, proxy);

    return std::make_shared<ClientImpl>
        (
        manager->GetUserInfo(),
        clientInfo,
        dbPath,
        true,
        policyProvider,
        ulasProvider,
        "",
        "",
        nullptr
        );
    }

AccessKeyClientImplPtr ClientIntegrationTests::CreateTestAccessKeyClientImpl(Utf8StringCR productId, Utf8StringCR accessKey) const
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, localState);

    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    BeFileName dbPath = GetLicensingDbPathIntegration();

    IBuddiProviderPtr buddiProvider = std::make_shared<BuddiProvider>();
    IPolicyProviderPtr policyProvider = std::make_shared<PolicyProvider>(buddiProvider, clientInfo, proxy, AuthType::None);
    IUlasProviderPtr ulasProvider = std::make_shared<UlasProvider>(buddiProvider, proxy);

    return std::make_shared<AccessKeyClientImpl>
        (
        accessKey,
        clientInfo,
        dbPath,
        true,
        policyProvider,
        ulasProvider,
        "",
        "",
        nullptr
        );
    }

SaasClientImplPtr ClientIntegrationTests::CreateTestSaasClientImpl(int productId) const
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, localState);
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    IBuddiProviderPtr buddiProvider = std::make_shared<BuddiProvider>();
    IUlasProviderPtr ulasProvider = std::make_shared<UlasProvider>(buddiProvider, proxy);

    return std::make_shared<SaasClientImpl>
        (
        productId,
        "",
        ulasProvider
        );
    }

TEST_F(ClientIntegrationTests, DISABLED_Equality_Test)
    {
    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    ASSERT_EQ((int)1, (int)1); // Mock policy should result in NotEntitled
    }

// Tests using the Clients' Create methods
TEST_F(ClientIntegrationTests, FactoryStartStopApplication_Success)
    {
    auto client = CreateTestClient(true, TEST_PRODUCT_ID);
    ASSERT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, AccessKeyFactoryStartStopApplication_Success)
    {
    auto client = CreateTestAccessKeyClient(TEST_PRODUCT_ID, TEST_VALID_ACCESSKEY);
    ASSERT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, SaasFactoryTrackUsage_Success)
    {
    auto client = CreateTestSaasClient(std::atoi(TEST_PRODUCT_ID));

    //Utf8String tokenstring = "5e1518a2d48671ce2c75d76b35b6324329ad61b0c6570dfa8781a016177e7ec4";
    Utf8String tokenString = "0cd2c3d2b0b813ce8c8e1b297f173f9e42f775ca32a2ee32a27b0a55daff1db9";
    auto version = BeVersion(1, 0);
    Utf8String projectId = "00000000-0000-0000-0000-000000000000";

    EXPECT_NE(static_cast<int>(client->TrackUsage(tokenString, version, projectId).get()), static_cast<int>(LicenseStatus::Error));
    }

// Tests using the Clients' implementation
TEST_F(ClientIntegrationTests, StartStopApplication_Success)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);
    ASSERT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, MarkFeature_Success)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);

    // need to add valid policy to the DB for MarkFeature to succeed
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);

    FeatureUserDataMapPtr featureData = std::make_shared<FeatureUserDataMap>();

    featureData->AddAttribute("Manufacturer", "Bentley Systems, Inc.");
    featureData->AddAttribute("Website", "https://www.w3schools.com");
    featureData->AddAttribute("Title", "Mobile App");

    ASSERT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));

    client->AddPolicyToDb(Policy::Create(jsonPolicyValid));

    EXPECT_EQ(static_cast<int>(client->MarkFeature("TestFeatureId", featureData)), static_cast<int>(BentleyStatus::SUCCESS));
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, GetLicenseStatusValidTrialPolicy_Test)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);

    // need to add valid policy to the DB for MarkFeature to succeed
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValidTrial = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, true);

    ASSERT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));

    client->AddPolicyToDb(Policy::Create(jsonPolicyValidTrial));

    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Trial));
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, GetLicenseStatusValidExpiredTrialPolicy_Test)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);

    // need to add valid policy to the DB for MarkFeature to succeed
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValidTrialExpired = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(-1), userId, std::atoi(TEST_PRODUCT_ID), "", 1, true);

    ASSERT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));

    client->AddPolicyToDb(Policy::Create(jsonPolicyValidTrialExpired));

    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Expired));
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, GetLicenseStatusValidPolicyWithGracePeriod_Test)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);

    // need to add valid policy to the DB for MarkFeature to succeed
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);

    auto timestamp = DateHelper::GetCurrentTime();
    auto timestampPast = DateHelper::AddDaysToCurrentTime(-14); // Two weeks ago; default offline period allowed is only 1 week

    ASSERT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));

    client->AddPolicyToDb(Policy::Create(jsonPolicyValid));

    client->GetLicensingDb().SetOfflineGracePeriodStart(timestamp);
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Offline)); // Valid status should be Offline now
    client->GetLicensingDb().ResetOfflineGracePeriod();
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Ok)); // Should be back to Ok
    client->GetLicensingDb().SetOfflineGracePeriodStart(timestampPast);
    EXPECT_EQ(client->GetLicensingDb().GetOfflineGracePeriodStart(), timestampPast);
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Expired)); // Valid status should be Expired now, since offline grace period has expired
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, GetLicenseStatusOfflineNotAllowedPolicy_Test)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);

    // need to add valid policy to the DB for MarkFeature to succeed
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyOfflineNotAllowed = DummyPolicyHelper::CreatePolicyOfflineNotAllowed(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);

    auto timestamp = DateHelper::GetCurrentTime();

    ASSERT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));

    client->AddPolicyToDb(Policy::Create(jsonPolicyOfflineNotAllowed));

    client->GetLicensingDb().SetOfflineGracePeriodStart(timestamp);
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::DisabledByPolicy)); // Grace Period started; should be disabled
    client->GetLicensingDb().ResetOfflineGracePeriod();
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Ok)); // Should be back to Ok
    EXPECT_SUCCESS(client->StopApplication());
    }

// TODO: heartbeat tests, different LicenseStatus situations

TEST_F(ClientIntegrationTests, AccessKeyStartStopApplication_Success)
    {
    auto client = CreateTestAccessKeyClientImpl(TEST_PRODUCT_ID, TEST_VALID_ACCESSKEY);
    ASSERT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, AccessKeyMarkFeature_Success)
    {
    auto client = CreateTestAccessKeyClientImpl("1000"); // TEST_VALID_ACCESSKEY is entitled to microstation (1000)

    FeatureUserDataMapPtr featureData = std::make_shared<FeatureUserDataMap>();

    featureData->AddAttribute("Manufacturer", "Bentley Systems, Inc.");
    featureData->AddAttribute("Website", "https://www.w3schools.com");
    featureData->AddAttribute("Title", "Mobile App");

    ASSERT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));
    EXPECT_EQ(static_cast<int>(client->MarkFeature("TestFeatureId", featureData)), static_cast<int>(BentleyStatus::SUCCESS));
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, AccessKeyGetLicenseStatusValidPolicy_Success)
    {
    auto client = CreateTestAccessKeyClientImpl("1000"); // TEST_VALID_ACCESSKEY is entitled to microstation (1000)

    ASSERT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));

    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Ok));
    EXPECT_SUCCESS(client->StopApplication());
    }

// TODO: heartbeat tests, different LicenseStatus situations

TEST_F(ClientIntegrationTests, SaasTrackUsage_Success)
    {
    auto client = CreateTestSaasClientImpl(std::atoi(TEST_PRODUCT_ID));

    Utf8String tokenString = "0cd2c3d2b0b813ce8c8e1b297f173f9e42f775ca32a2ee32a27b0a55daff1db9";
    auto version = BeVersion(1, 0);
    Utf8String projectId = "00000000-0000-0000-0000-000000000000";

    EXPECT_NE(static_cast<int>(client->TrackUsage(tokenString, version, projectId).get()), static_cast<int>(LicenseStatus::Error));
    }

//TEST_F(ClientIntegrationTests, AccessKeyClientTestPolicyHeartbeat_Test)
//    {
//    // I am using this test to manually debug/test the heartbeat to make sure of the following:
//    // - policy heartbeat does in fact run as expected (heartbeat every 1 second, refresh policy by PolicyInterval)
//    // - policy heartbeat cleans up as expected (after StopApplication is called, it doens't try to access disposed resources)
//    // - maybe: policy heartbeat handles going offline well (keeps using old policy and access key until policy expires)
//    auto client = CreateWithKeyTestClient(true);
//    EXPECT_NE((int)client->StartApplication(), (int)LicenseStatus::Error);
//
//    // use std::chrono::seconds(2); to wait in tests
//    // PolicyRefresh: make a custom policy with a fast refresh! -> right now it is set for 60 days!
//    // Heartbeat: check that function is entered multiple times [GOOD]
//    // Cleanup: stop application and wait and check if anything is accessed [double check!]
//
//    std::this_thread::sleep_for(std::chrono::seconds(10));
//
//    EXPECT_SUCCESS(client->StopApplication());
//
//    std::this_thread::sleep_for(std::chrono::seconds(10));
//
//    EXPECT_EQ(1, 0);
//    }
