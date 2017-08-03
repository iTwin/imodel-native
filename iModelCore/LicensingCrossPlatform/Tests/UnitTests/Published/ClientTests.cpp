/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Tests/UnitTests/Published/ClientTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ClientTests.h"

#include <Licensing/Client.h>
#include "../../../Licensing/ClientImpl.h"
#include "../../../Licensing/UsageDb.h"
#include "../../../Licensing/PolicyToken.h"

#include <BeHttp/HttpClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeSQLite/BeSQLite.h>

#include <Licensing/Utils/InMemoryJsonLocalState.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS
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

BeFileName GetUsageDbPath()
    {
    BeFileName path;
    BeTest::GetHost().GetTempDir(path);
    path.AppendToPath(L"usage.db");

    return path;
    }

ClientImplPtr CreateTestClient(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env)
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, localState);

    auto productId = "2545";
    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    auto manager = ConnectSignInManager::Create(clientInfo, proxy, localState);

    if (signIn)
        {
        Credentials credentials("qa2_devuser2@mailinator.com", "bentley");
        if (!manager->SignInWithCredentials(credentials)->GetResult().IsSuccess())
            return nullptr;
        }

    BeFileName dbPath = GetUsageDbPath();

    return std::make_shared<ClientImpl>(
        dbPath, 
        manager,
        clientInfo,
        manager->GetUserInfo(),
        proxy,
        heartbeatInterval, 
        timeRetriever,
        delayedExecutor);
    }

ClientImplPtr CreateTestClient(bool signIn)
    {
    return CreateTestClient(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa);
    }

ClientImplPtr CreateTestClient(bool signIn, UrlProvider::Environment env)
    {
    return CreateTestClient(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), env);
    }

ClientImplPtr CreateTestClient(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor)
    {
    return CreateTestClient(signIn, heartbeatInterval, timeRetriever, delayedExecutor, UrlProvider::Environment::Qa);
    }

void ClientTests::SetUpTestCase()
    {
    // This is only an example of how to set logging severity and see info logs. Usually should be set more globally than in TestCase SetUp
    // NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_INFO);

    BeFileName asssetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(asssetsDir);
    HttpClient::Initialize(asssetsDir);

    BeFileName tmpDir;
    BeTest::GetHost().GetTempDir(tmpDir);
    BeSQLiteLib::Initialize(tmpDir);
    }

TEST_F(ClientTests, StartApplication_Called2ndTimeBeforeEndTime_ExtendsEndTime)
    {
    auto timeRetriever = TestTimeRetriever::Create();
    auto delayedExecutor = TestDelayedExecutor::Create();
    auto client = CreateTestClient(false, 10, timeRetriever, delayedExecutor);
    EXPECT_NE(nullptr, client);

    timeRetriever->time = 10;
    EXPECT_SUCCESS(client->StartApplication());

    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
    EXPECT_EQ(10 + 10, client->GetUsageDb().GetLastRecordEndTime());

    timeRetriever->time = 15;
    EXPECT_SUCCESS(client->StartApplication());

    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
    EXPECT_EQ(15 + 10, client->GetUsageDb().GetLastRecordEndTime());
    }

TEST_F(ClientTests, StartApplication_Called2ndTimeAfterEndTime_CreatesNewRecord)
    {
    auto timeRetriever = TestTimeRetriever::Create();
    auto delayedExecutor = TestDelayedExecutor::Create();
    auto client = CreateTestClient(false, 10, timeRetriever, delayedExecutor);
    EXPECT_NE(nullptr, client);

    timeRetriever->time = 10;
    EXPECT_SUCCESS(client->StartApplication());

    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
    EXPECT_EQ(10 + 10, client->GetUsageDb().GetLastRecordEndTime());

    timeRetriever->time = 25;
    EXPECT_SUCCESS(client->StartApplication());

    EXPECT_EQ(2, client->GetUsageDb().GetRecordCount());
    EXPECT_EQ(25 + 10, client->GetUsageDb().GetLastRecordEndTime());
    }

TEST_F(ClientTests, StartApplication_AfterStopApplication_CreatesNewRecord)
    {
    auto timeRetriever = TestTimeRetriever::Create();
    auto delayedExecutor = TestDelayedExecutor::Create();
    auto client = CreateTestClient(false, 10, timeRetriever, delayedExecutor);
    EXPECT_NE(nullptr, client);

    timeRetriever->time = 10;
    EXPECT_SUCCESS(client->StartApplication());

    timeRetriever->time = 13;
    EXPECT_SUCCESS(client->StopApplication());

    timeRetriever->time = 15;
    EXPECT_SUCCESS(client->StartApplication());

    EXPECT_EQ(2, client->GetUsageDb().GetRecordCount());
    EXPECT_EQ(15 + 10, client->GetUsageDb().GetLastRecordEndTime());
    }

TEST_F(ClientTests, Heartbeat_AfterApplicationStart_UpdatesEndTime)
    {
    auto timeRetriever = TestTimeRetriever::Create();
    auto delayedExecutor = TestDelayedExecutor::Create();
    auto client = CreateTestClient(false, 10, timeRetriever, delayedExecutor);
    EXPECT_NE(nullptr, client);

    timeRetriever->time = 10;
    EXPECT_SUCCESS(client->StartApplication());

    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
    EXPECT_EQ(10 + 10, client->GetUsageDb().GetLastRecordEndTime());

    timeRetriever->time = 20;
    delayedExecutor->Execute();

    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
    EXPECT_EQ(20 + 10, client->GetUsageDb().GetLastRecordEndTime());

    timeRetriever->time = 30;
    delayedExecutor->Execute();

    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
    EXPECT_EQ(30 + 10, client->GetUsageDb().GetLastRecordEndTime());
    }

TEST_F(ClientTests, Heartbeat_AfterApplicationEnd_DoesNothing)
    {
    auto timeRetriever = TestTimeRetriever::Create();
    auto delayedExecutor = TestDelayedExecutor::Create();
    auto client = CreateTestClient(false, 10, timeRetriever, delayedExecutor);
    EXPECT_NE(nullptr, client);

    timeRetriever->time = 10;
    EXPECT_SUCCESS(client->StartApplication());

    timeRetriever->time = 15;
    EXPECT_SUCCESS(client->StopApplication());
    
    timeRetriever->time = 20;
    delayedExecutor->Execute();

    // After stopping application db will be closed, so we open it again for inspection.
    EXPECT_SUCCESS(client->GetUsageDb().OpenOrCreate(GetUsageDbPath()));

    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
    EXPECT_EQ(15, client->GetUsageDb().GetLastRecordEndTime());
    }

TEST_F(ClientTests, Heartbeat_AfterStartApplicationCalledTwise_UpdatesAccordingToTheLast)
    {
    auto timeRetriever = TestTimeRetriever::Create();
    auto delayedExecutor = TestDelayedExecutor::Create();
    auto client = CreateTestClient(false, 10, timeRetriever, delayedExecutor);
    EXPECT_NE(nullptr, client);

    timeRetriever->time = 10;
    EXPECT_SUCCESS(client->StartApplication());

    timeRetriever->time = 15;
    EXPECT_SUCCESS(client->StartApplication());

    timeRetriever->time = 20;
    delayedExecutor->Execute();

    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
    EXPECT_EQ(15+10, client->GetUsageDb().GetLastRecordEndTime());

    timeRetriever->time = 25;
    delayedExecutor->Execute();

    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
    EXPECT_EQ(25+10, client->GetUsageDb().GetLastRecordEndTime());
    }

TEST_F(ClientTests, StopApplication_CalledBeforeStartApplication_DoesNothing)
    {
    auto client = CreateTestClient(false);
    EXPECT_NE(nullptr, client);

    EXPECT_SUCCESS(client->StopApplication());

    // After stopping application db will be closed, so we open it again for inspection.
    EXPECT_SUCCESS(client->GetUsageDb().OpenOrCreate(GetUsageDbPath()));

    EXPECT_EQ(0, client->GetUsageDb().GetRecordCount());
    }

TEST_F(ClientTests, StopApplication_CalledAfterStartApplication_UpdatesLastRecordEndTime)
    {
    auto timeRetriever = TestTimeRetriever::Create();
    auto delayedExecutor = TestDelayedExecutor::Create();
    auto client = CreateTestClient(false, 10, timeRetriever, delayedExecutor);
    EXPECT_NE(nullptr, client);

    timeRetriever->time = 10;
    EXPECT_SUCCESS(client->StartApplication());

    timeRetriever->time = 13;
    EXPECT_SUCCESS(client->StopApplication());

    // After stopping application db will be closed, so we open it again for inspection.
    EXPECT_SUCCESS(client->GetUsageDb().OpenOrCreate(GetUsageDbPath()));

    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
    EXPECT_EQ(13, client->GetUsageDb().GetLastRecordEndTime());
    }

TEST_F(ClientTests, CreateRealClient_StartAndStopApplication_Succeeds)
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, localState);

    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", "2545");
    auto manager = ConnectSignInManager::Create(clientInfo, nullptr, localState);
    BeFileName dbPath = GetUsageDbPath();

    auto client = Client::Create(
        dbPath,
        manager,
        clientInfo,
        manager->GetUserInfo());
    EXPECT_NE(nullptr, client);

    EXPECT_SUCCESS(client->StartApplication());
    EXPECT_SUCCESS(client->StopApplication());
    }

// TODO: Create separate Project for Integration tests.
TEST_F (ClientTests, GetPolicy_InegrationTest)
    {
    auto client = CreateTestClient(true);

    // Test code can be written like this:
    auto policyToken = client->GetPolicy().get(); // This will re throw exeptions occured and they will be catched by gtest.
    ASSERT_FALSE(policyToken->GetQualifier("HeartbeatInterval").isNull()); // Check if qualifier is in PolicyToken

    // If you want you can catch specific expection in tests simply like this:
    //try
    //    {
    //    auto policyToken = client->GetPolicy().get();
    //    ASSERT_FALSE(policyToken->GetQualifier("HeartbeatInterval").isNull()); // Check if qualifier is in PolicyToken
    //    }
    //catch (const HttpError& e)
    //    {
    //    FAIL();
    //    }

    // Production code should look like this:
    //client->GetPolicy().then([](std::shared_ptr<PolicyToken> token)
    //    {
    //    // use token
    //    }).onError([](const HttpError& e) 
    //    {
    //    // Error handling.
    //    });
    }

TEST_F(ClientTests, SendUsage_InegrationTest)
    {
    auto client = CreateTestClient(true, UrlProvider::Environment::Dev);

    EXPECT_SUCCESS(client->StartApplication());

    // TODO: SCV should be generated and send to server periodically inside the client logic
    BeFileName path;
    BeTest::GetHost().GetTempDir(path);
    path.AppendToPath(L"test.scv");
    EXPECT_SUCCESS(client->GetUsageDb().WriteUsageToSCVFile(path));

    client->SendUsage(path, "1004175881").get();
    // TODO: how to check if sending was successfull?
    }
