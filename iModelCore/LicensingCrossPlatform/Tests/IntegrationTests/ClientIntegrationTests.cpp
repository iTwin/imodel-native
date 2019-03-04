/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/ClientIntegrationTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestsHelper.h"
#include "ClientIntegrationTests.h"

#include <Licensing/Client.h>
#include <Licensing/SaasClient.h>
#include <Licensing/Utils/DateHelper.h>
#include "../../Licensing/ClientImpl.h"
#include "../../Licensing/SaasClientImpl.h"
#include "../../Licensing/ClientWithKeyImpl.h"
#include "../../Licensing/UsageDb.h"
#include "../../PublicAPI/Licensing/Utils/SCVWritter.h"

#include <BeHttp/HttpClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/L10N.h>
#include <fstream>
#include <Licensing/Utils/InMemoryJsonLocalState.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

using ::testing::AtLeast;
using ::testing::Return;
using ::testing::ByMove;
using ::testing::A;
using ::testing::_;

#define TEST_PRODUCT_ID     "2545"

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

 BeFileName GetUsageDbPathIntegration()
     {
     BeFileName path;
     BeTest::GetHost().GetTempDir(path);
     path.AppendToPath(L"License.db");

     return path;
     }

 ClientPtr CreateTestClient(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId)
 {
	 InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
	 UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, localState);

	 auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

	 auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
	 auto manager = ConnectSignInManager::Create(clientInfo, proxy, localState);
	 if (signIn)
	 {
		 Credentials credentials("qa2_devuser2@mailinator.com", "bentley");
		 if (!manager->SignInWithCredentials(credentials)->GetResult().IsSuccess())
			 return nullptr;
	 }
	 BeFileName dbPath = GetUsageDbPathIntegration();

	 return std::shared_ptr<Client>(Client::Create(manager->GetUserInfo(),
		 clientInfo,
		 manager,
		 dbPath,
		 true,
		 "",
		 "",
		 proxy));
 }

 ClientPtr CreateTestClient(bool signIn)
     {
	 return CreateTestClient(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID);
     }

// SaasClientImplPtr CreateFreeTestClient(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId, IBuddiProviderPtr buddiProvider)
//     {
//     InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
//     UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, localState);
//     auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

//     return std::make_shared<SaasClientImpl>(
//         "",
//         proxy,
//         buddiProvider);
//     }

// ClientWithKeyImplPtr CreateWithKeyTestClient(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId, IBuddiProviderPtr buddiProvider, IUlasProviderPtr ulasProvider)
//     {
//     InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
//     UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, localState);

//     auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

//     auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();


//     auto manager = ConnectSignInManager::Create(clientInfo, proxy, localState);

//     BeFileName dbPath = GetUsageDbPath();

//     Utf8String accesskey = "somekey";

//     return std::make_shared<ClientWithKeyImpl>(
//         accesskey,
//         clientInfo,
//         dbPath,
//         true,
//         buddiProvider,
//         ulasProvider,
//         "",
//         "",
//         proxy);
//     }

// // Note: cannot use BuddiProvider mocks with clients created with the factory
// ClientPtr CreateTestClientFromFactory(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId)
//     {
//     InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
//     UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, localState);

//     auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

//     auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
//     auto manager = ConnectSignInManager::Create(clientInfo, proxy, localState);
//     if (signIn)
//     {
//         Credentials credentials("qa2_devuser2@mailinator.com", "bentley");
//         if (!manager->SignInWithCredentials(credentials)->GetResult().IsSuccess())
//             return nullptr;
//     }
//     BeFileName dbPath = GetUsageDbPath();

//     return Client::Create(
//         manager->GetUserInfo(),
//         clientInfo,
//         manager,
//         dbPath,
//         true,
//         "",
//         "",
//         proxy);
//     }

// SaasClientPtr CreateFreeTestClientFromFactory(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId)
//     {
//     InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
//     UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, localState);

//     auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

//     return SaasClient::Create(
//         "",
//         proxy);
//     }

 ClientPtr CreateWithKeyTestClient(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId)
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    //RuntimeJsonLocalState* localState = new RuntimeJsonLocalState();
    UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, localState);

    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    auto manager = ConnectSignInManager::Create(clientInfo, proxy, localState);

    BeFileName dbPath = GetUsageDbPathIntegration();

    //Utf8String accesskey = "somekey"; // need a real one?
    Utf8String accesskey = "3469AD8D095A53F3CBC9A905A8FF8926"; // this should work -> luke just made it

    return Client::CreateWithKey(
        accesskey,
        clientInfo,
        dbPath,
        true,
        "",
        "",
        proxy);
    }

// ClientImplPtr CreateTestClient(bool signIn, IBuddiProviderPtr buddiProvider, IPolicyProviderPtr policyProvider, IUlasProviderPtr ulasProvider)
//     {
//     return CreateTestClient(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID, buddiProvider, policyProvider, ulasProvider);
//     }

// ClientPtr CreateTestClientFromFactory(bool signIn)
//     {
//     return CreateTestClientFromFactory(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID);
//     }

// SaasClientImplPtr CreateFreeTestClient(bool signIn, IBuddiProviderPtr buddiProvider)
//     {
//     return CreateFreeTestClient(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID, buddiProvider);
//     }

 //SaasClientPtr CreateFreeTestClientFromFactory(bool signIn)
 //    {
 //    return CreateFreeTestClientFromFactory(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID);
 //    }

// ClientWithKeyImplPtr CreateWithKeyTestClient(bool signIn, IBuddiProviderPtr buddiProvider, IUlasProviderPtr ulasProvider)
//     {
//     return CreateWithKeyTestClient(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID, buddiProvider, ulasProvider);
//     }

ClientPtr CreateWithKeyTestClient(bool signIn)
    {
    //return CreateWithKeyTestClient(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID);
    return CreateWithKeyTestClient(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, "1000");
    }

 ///*--------------------------------------------------------------------------------------+
 //* @bsimethod
 //+---------------+---------------+---------------+---------------+---------------+------*/
 //ClientIntegrationTests::ClientIntegrationTests() :
 //    m_handler(std::make_shared<IHttpHandler>()),
 //    m_buddiProvider(std::make_shared<IBuddiProvider>()),
 //    m_policyProvider(std::make_shared<IPolicyProvider>()),
 //    m_ulasProvider(std::make_shared<IUlasProvider>())
 //    {}
/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClientIntegrationTests::ClientIntegrationTests() {}

// /*--------------------------------------------------------------------------------------+
// * @bsimethod
// +---------------+---------------+---------------+---------------+---------------+------*/
// MockHttpHandler& ClientIntegrationTests::GetHandler() const
//     {
//     return *m_handler;
//     }

///*--------------------------------------------------------------------------------------+
//* @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------*/
//std::shared_ptr<IHttpHandler> ClientIntegrationTests::GetHandlerPtr() const
//    {
//    return m_handler;
//    }
//
///*--------------------------------------------------------------------------------------+
//* @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------*/
//IBuddiProvider&  ClientIntegrationTests::GetBuddiProvider() const
//    {
//    return *m_buddiProvider;
//    }
//
///*--------------------------------------------------------------------------------------+
//* @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------*/
//std::shared_ptr<IBuddiProvider> ClientIntegrationTests::GetBuddiProviderPtr() const
//    {
//    return m_buddiProvider;
//    }
//
///*--------------------------------------------------------------------------------------+
//* @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------*/
//IPolicyProvider&  ClientIntegrationTests::GetPolicyProvider() const
//    {
//    return *m_policyProvider;
//    }
//
///*--------------------------------------------------------------------------------------+
//* @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------*/
//std::shared_ptr<IPolicyProvider> ClientIntegrationTests::GetPolicyProviderPtr() const
//    {
//    return m_policyProvider;
//    }
//
///*--------------------------------------------------------------------------------------+
//* @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------*/
//IUlasProvider&  ClientIntegrationTests::GetUlasProvider() const
//    {
//    return *m_ulasProvider;
//    }
//
///*--------------------------------------------------------------------------------------+
//* @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------*/
//std::shared_ptr<IUlasProvider> ClientIntegrationTests::GetUlasProviderPtr() const
//    {
//    return m_ulasProvider;
//    }


/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientIntegrationTests::TearDown()
    {
    // m_handler->ValidateAndClearExpectations();
    }

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
     path.AppendToPath(L"sqlang/DgnClientFx_en.sqlang.db3");

     ASSERT_EQ(SUCCESS, L10N::Initialize(BeSQLite::L10N::SqlangFiles(path)));
    }

TEST_F(ClientIntegrationTests, StartApplication_StopApplication_Success)
{
	auto client = CreateTestClient(true);
	EXPECT_NE((int)client->StartApplication(), (int)LicenseStatus::Error);
	EXPECT_SUCCESS(client->StopApplication());
}

// Need to fix this to have mock return a valid policy (or reevaluate the logic here...)
TEST_F(ClientIntegrationTests, Equality_Test)
    {
    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    ASSERT_EQ((int)1, (int)1); // Mock policy should result in NotEntitled
    }

TEST_F(ClientIntegrationTests, ClientWithKeyStartApplicationStopApplication_Success)
    {
    auto client = CreateWithKeyTestClient(true);
    EXPECT_NE((int)client->StartApplication(), (int)LicenseStatus::Error);
    client->StopApplication();
    }


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