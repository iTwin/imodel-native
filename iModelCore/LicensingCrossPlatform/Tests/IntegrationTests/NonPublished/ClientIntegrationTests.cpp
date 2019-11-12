/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ClientIntegrationTests.h"
#include "TestsHelper.h"
#include "DummyPolicyHelper.h"

#include <Licensing/Utils/DateHelper.h>
#include <Licensing/Utils/SCVWritter.h>
#include "../../../Licensing/LicensingDb.h"

//#include "../../../Licensing/Providers/IAuthHandlerProvider.h"
//#include "../../../Licensing/Providers/IBuddiProvider.h"
//#include "../../../Licensing/Providers/IPolicyProvider.h"
//#include "../../../Licensing/Providers/IUlasProvider.h"

#include "../../../Licensing/Providers/AuthHandlerProvider.h"
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


#define TEST_PRODUCT_ID      "2545"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_INTEGRATION_TESTS
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE

BeFileName ClientIntegrationTests::GetLicensingDbPathIntegration() const
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
    //NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_INFO);
    //NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_DEBUG);
    //NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_TRACE);

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
    //InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    RuntimeJsonLocalState* localState = new RuntimeJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, localState);

    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    // NOTE (7/30/19): all of these tests fail because this policy is expired... see if we can renew this
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    auto manager = ConnectSignInManager::Create(clientInfo, proxy, localState);
    if (signIn)
        {
        Credentials credentials("qa2_devuser2@mailinator.com", "bentley");
        auto result = manager->SignInWithCredentials(credentials)->GetResult();
        if (!result.IsSuccess())
            {
            LOG.infov("error: %s, %s", result.GetError().GetMessage().c_str(), result.GetError().GetDescription().c_str());
            return nullptr;
            }
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

ClientImplPtr ClientIntegrationTests::CreateTestClientImpl(bool signIn, Utf8StringCR productId) const
    {
    //InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    RuntimeJsonLocalState* localState = new RuntimeJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, localState);

    //NOTE (5/28/19): the policy for this test user and product ID seems to be trial now so some integration tests fail. Will take a look at this...
    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    auto manager = ConnectSignInManager::Create(clientInfo, proxy, localState);
    if (signIn)
        {
        Credentials credentials("qa2_devuser2@mailinator.com", "bentley");
        auto result = manager->SignInWithCredentials(credentials)->GetResult();
        if (!result.IsSuccess())
            {
            LOG.infov("error: %s, %s", result.GetError().GetMessage().c_str(), result.GetError().GetDescription().c_str());
            return nullptr;
            }
        }
    BeFileName dbPath = GetLicensingDbPathIntegration();

    AuthType authType = AuthType::SAML;

    ApplicationInfoPtr applicationInfo = std::make_shared<ApplicationInfo>(clientInfo->GetApplicationVersion(), clientInfo->GetDeviceId(), clientInfo->GetApplicationProductId());
    IBuddiProviderPtr buddiProvider = std::make_shared<BuddiProvider>();
    IAuthHandlerProviderPtr authHandlerProvider = std::make_shared<AuthHandlerProvider>(manager, proxy);
    IPolicyProviderPtr policyProvider = std::make_shared<PolicyProvider>(buddiProvider, applicationInfo, proxy, authType, authHandlerProvider);
    IUlasProviderPtr ulasProvider = std::make_shared<UlasProvider>(buddiProvider, proxy);

    return std::make_shared<ClientImpl>
        (
        manager->GetUserInfo(),
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

TEST_F(ClientIntegrationTests, DISABLED_Equality_Test)
    {
    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    ASSERT_EQ((int)1, (int)1); // Mock policy should result in NotEntitled
    }

// Tests using the Client's Create method
TEST_F(ClientIntegrationTests, FactoryStartStopApplication_Success)
    {
    auto client = CreateTestClient(true, TEST_PRODUCT_ID);
    if (nullptr == client)
        {
        FAIL() << "client is null";
        }
    auto startStatus = client->StartApplication();
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    EXPECT_SUCCESS(client->StopApplication());
    }

// Tests using the Clients' implementation
TEST_F(ClientIntegrationTests, StartStopApplication_Success)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);
    auto startStatus = client->StartApplication();
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));

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

    auto startStatus = client->StartApplication();
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));

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

    client->AddPolicyToDb(Policy::Create(jsonPolicyValidTrial));

    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Trial));

	auto startStatus = client->StartApplication();
	ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
	ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, GetLicenseStatusValidExpiredTrialPolicy_Test)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);

    // need to add valid policy to the DB for MarkFeature to succeed
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValidTrialExpired = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(-1), DateHelper::AddDaysToCurrentTime(-1), userId, std::atoi(TEST_PRODUCT_ID), "", 1, true);

   

    client->AddPolicyToDb(Policy::Create(jsonPolicyValidTrialExpired));

    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Expired));

	auto startStatus = client->StartApplication();
	ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
	ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));

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

    auto startStatus = client->StartApplication();
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
    ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));

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

    client->AddPolicyToDb(Policy::Create(jsonPolicyOfflineNotAllowed));

    client->GetLicensingDb().SetOfflineGracePeriodStart(timestamp);
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::DisabledByPolicy)); // Grace Period started; should be disabled
    client->GetLicensingDb().ResetOfflineGracePeriod();
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Ok)); // Should be back to Ok
    
	auto startStatus = client->StartApplication();
	ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::Error));
	ASSERT_NE(static_cast<int>(startStatus), static_cast<int>(LicenseStatus::NotEntitled));
	
	EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientIntegrationTests, GetTrialDaysRemainingValidTrialPolicy_Test)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);

    // need to add valid policy to the DB for MarkFeature to succeed
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValidTrial = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, true);

    // NOTE: we don't need to start application here since we are not testing the heartbeat, and are manually adding a policy

    client->AddPolicyToDb(Policy::Create(jsonPolicyValidTrial));

    EXPECT_EQ(client->GetTrialDaysRemaining(), 6);
    }

TEST_F(ClientIntegrationTests, GetTrialDaysRemainingValidExpiredTrialPolicy_Test)
    {
    auto client = CreateTestClientImpl(true, TEST_PRODUCT_ID);

    // need to add valid policy to the DB for MarkFeature to succeed
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValidTrialExpired = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(-1), userId, std::atoi(TEST_PRODUCT_ID), "", 1, true);

    // NOTE: we don't need to start application here since we are not testing the heartbeat, and are manually adding a policy

    client->AddPolicyToDb(Policy::Create(jsonPolicyValidTrialExpired));

    EXPECT_EQ(client->GetTrialDaysRemaining(), 0);
    }

// TODO: heartbeat tests, different LicenseStatus situations

