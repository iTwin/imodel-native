/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/ClientIntegrationTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestsHelper.h"
#include "ClientIntegrationTests.h"

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

TEST_F(ClientIntegrationTests, StartApplication_StopApplication_Success)
    {
    auto client = CreateTestClient(true, TEST_PRODUCT_ID);
    EXPECT_NE((int)client->StartApplication(), (int)LicenseStatus::Error);
    EXPECT_SUCCESS(client->StopApplication());
    }

// Need to fix this to have mock return a valid policy (or reevaluate the logic here...)
TEST_F(ClientIntegrationTests, DISABLED_Equality_Test)
    {
    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    ASSERT_EQ((int)1, (int)1); // Mock policy should result in NotEntitled
    }

TEST_F(ClientIntegrationTests, AccessKeyClientStartApplicationStopApplication_Success)
    {
    auto client = CreateTestAccessKeyClient(TEST_PRODUCT_ID, TEST_VALID_ACCESSKEY);
    EXPECT_NE((int)client->StartApplication(), (int)LicenseStatus::Error);
    EXPECT_SUCCESS(client->StopApplication());
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


// Below are "start from factory tests", some might be redundant since the client is already made from the factory here

//TEST_F(ClientTests, StartApplicationFromFactory_Success)
//    {
//    // Note: cannot use mocks with the factory-created clients
//    auto client = CreateTestClientFromFactory(true);
//    EXPECT_NE((int)client->StartApplication(), (int)LicenseStatus::Error);
//    client->StopApplication();
//    }
//
//TEST_F(ClientTests, DISABLED_TrackUsage_FreeApplicationFromFactory_Success)
//    {
//    // Note: cannot use mocks with the factory-created clients
//    auto client = CreateFreeTestClientFromFactory(true);
//    Utf8String tokenstring = "0cd2c3d2b0b813ce8c8e1b297f173f9e42f775ca32a2ee32a27b0a55daff1db9";
//    auto version = BeVersion(1, 0);
//    Utf8String projectId = "00000000-0000-0000-0000-000000000000";
//    EXPECT_SUCCESS(client->TrackUsage(tokenstring, version, projectId).get());
//    }
//
//TEST_F(ClientTests, StartWithKeyApplicationFromFactory_Success)
//    {
//    // Note: cannot use mocks with the factory-created clients
//    auto client = CreateWithKeyTestClientFromFactory(true);
//    EXPECT_NE((int)client->StartApplication(), (int)LicenseStatus::Error);
//    client->StopApplication();
//    }
