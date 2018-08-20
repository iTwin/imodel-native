/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Tests/UnitTests/Published/ClientTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ClientTests.h"

#include <Licensing/Client.h>
#include "../../../Licensing/ClientImpl.h"
#include "../../../Licensing/UsageDb.h"
#include "../../../Licensing/PolicyToken.h"
#include "../../../Licensing/DummyJsonHelper.h"
#include "../../../Licensing/DateHelper.h"
#include "../../../PublicAPI/Licensing/Utils/SCVWritter.h"

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
        manager->GetUserInfo(),
        clientInfo,
        manager,
        dbPath,
        true,
        "",
        "",
        proxy);
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

//TEST_F(ClientTests, StartApplication_Called2ndTimeBeforeEndTime_ExtendsEndTime)
//    {
//    auto timeRetriever = TestTimeRetriever::Create();
//    auto delayedExecutor = TestDelayedExecutor::Create();
//    auto client = CreateTestClient(false, 10, timeRetriever, delayedExecutor);
//    EXPECT_NE(nullptr, client);
//
//    timeRetriever->time = 10;
//    EXPECT_SUCCESS(client->StartApplication());
//
//    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
//    EXPECT_EQ(10 + 10, client->GetUsageDb().GetLastRecordEndTime());
//
//    timeRetriever->time = 15;
//    EXPECT_SUCCESS(client->StartApplication());
//
//    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
//    EXPECT_EQ(15 + 10, client->GetUsageDb().GetLastRecordEndTime());
//    }

//TEST_F(ClientTests, StartApplication_Called2ndTimeAfterEndTime_CreatesNewRecord)
//    {
//    auto timeRetriever = TestTimeRetriever::Create();
//    auto delayedExecutor = TestDelayedExecutor::Create();
//    auto client = CreateTestClient(false, 10, timeRetriever, delayedExecutor);
//    EXPECT_NE(nullptr, client);
//
//    timeRetriever->time = 10;
//    EXPECT_SUCCESS(client->StartApplication());
//
//    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
//    EXPECT_EQ(10 + 10, client->GetUsageDb().GetLastRecordEndTime());
//
//    timeRetriever->time = 25;
//    EXPECT_SUCCESS(client->StartApplication());
//
//    EXPECT_EQ(2, client->GetUsageDb().GetRecordCount());
//    EXPECT_EQ(25 + 10, client->GetUsageDb().GetLastRecordEndTime());
//    }

//TEST_F(ClientTests, StartApplication_AfterStopApplication_CreatesNewRecord)
//    {
//    auto timeRetriever = TestTimeRetriever::Create();
//    auto delayedExecutor = TestDelayedExecutor::Create();
//    auto client = CreateTestClient(false, 10, timeRetriever, delayedExecutor);
//    EXPECT_NE(nullptr, client);
//
//    timeRetriever->time = 10;
//    EXPECT_SUCCESS(client->StartApplication());
//
//    timeRetriever->time = 13;
//    EXPECT_SUCCESS(client->StopApplication());
//
//    timeRetriever->time = 15;
//    EXPECT_SUCCESS(client->StartApplication());
//
//    EXPECT_EQ(2, client->GetUsageDb().GetRecordCount());
//    EXPECT_EQ(15 + 10, client->GetUsageDb().GetLastRecordEndTime());
//    }

//TEST_F(ClientTests, Heartbeat_AfterApplicationStart_UpdatesEndTime)
//    {
//    auto timeRetriever = TestTimeRetriever::Create();
//    auto delayedExecutor = TestDelayedExecutor::Create();
//    auto client = CreateTestClient(false, 10, timeRetriever, delayedExecutor);
//    EXPECT_NE(nullptr, client);
//
//    timeRetriever->time = 10;
//    EXPECT_SUCCESS(client->StartApplication());
//
//    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
//    EXPECT_EQ(10 + 10, client->GetUsageDb().GetLastRecordEndTime());
//
//    timeRetriever->time = 20;
//    delayedExecutor->Execute();
//
//    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
//    EXPECT_EQ(20 + 10, client->GetUsageDb().GetLastRecordEndTime());
//
//    timeRetriever->time = 30;
//    delayedExecutor->Execute();
//
//    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
//    EXPECT_EQ(30 + 10, client->GetUsageDb().GetLastRecordEndTime());
//    }

//TEST_F(ClientTests, Heartbeat_AfterApplicationEnd_DoesNothing)
//    {
//    auto timeRetriever = TestTimeRetriever::Create();
//    auto delayedExecutor = TestDelayedExecutor::Create();
//    auto client = CreateTestClient(false, 10, timeRetriever, delayedExecutor);
//    EXPECT_NE(nullptr, client);
//
//    timeRetriever->time = 10;
//    EXPECT_SUCCESS(client->StartApplication());
//
//    timeRetriever->time = 15;
//    EXPECT_SUCCESS(client->StopApplication());
//    
//    timeRetriever->time = 20;
//    delayedExecutor->Execute();
//
//    // After stopping application db will be closed, so we open it again for inspection.
//    EXPECT_SUCCESS(client->GetUsageDb().OpenOrCreate(GetUsageDbPath()));
//
//    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
//    EXPECT_EQ(15, client->GetUsageDb().GetLastRecordEndTime());
//    }

//TEST_F(ClientTests, Heartbeat_AfterStartApplicationCalledTwise_UpdatesAccordingToTheLast)
//    {
//    auto timeRetriever = TestTimeRetriever::Create();
//    auto delayedExecutor = TestDelayedExecutor::Create();
//    auto client = CreateTestClient(false, 10, timeRetriever, delayedExecutor);
//    EXPECT_NE(nullptr, client);
//
//    timeRetriever->time = 10;
//    EXPECT_SUCCESS(client->StartApplication());
//
//    timeRetriever->time = 15;
//    EXPECT_SUCCESS(client->StartApplication());
//
//    timeRetriever->time = 20;
//    delayedExecutor->Execute();
//
//    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
//    EXPECT_EQ(15+10, client->GetUsageDb().GetLastRecordEndTime());
//
//    timeRetriever->time = 25;
//    delayedExecutor->Execute();
//
//    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
//    EXPECT_EQ(25+10, client->GetUsageDb().GetLastRecordEndTime());
//    }

//TEST_F(ClientTests, StopApplication_CalledBeforeStartApplication_DoesNothing)
//    {
//    auto client = CreateTestClient(false);
//    EXPECT_NE(nullptr, client);
//
//    EXPECT_SUCCESS(client->StopApplication());
//
//    // After stopping application db will be closed, so we open it again for inspection.
//    EXPECT_SUCCESS(client->GetUsageDb().OpenOrCreate(GetUsageDbPath()));
//
//    EXPECT_EQ(0, client->GetUsageDb().GetRecordCount());
//    }

//TEST_F(ClientTests, StopApplication_CalledAfterStartApplication_UpdatesLastRecordEndTime)
//    {
//    auto timeRetriever = TestTimeRetriever::Create();
//    auto delayedExecutor = TestDelayedExecutor::Create();
//    auto client = CreateTestClient(false, 10, timeRetriever, delayedExecutor);
//    EXPECT_NE(nullptr, client);
//
//    timeRetriever->time = 10;
//    EXPECT_SUCCESS(client->StartApplication());
//
//    timeRetriever->time = 13;
//    EXPECT_SUCCESS(client->StopApplication());
//
//    // After stopping application db will be closed, so we open it again for inspection.
//    EXPECT_SUCCESS(client->GetUsageDb().OpenOrCreate(GetUsageDbPath()));
//
//    EXPECT_EQ(1, client->GetUsageDb().GetRecordCount());
//    EXPECT_EQ(13, client->GetUsageDb().GetLastRecordEndTime());
//    }

//TEST_F(ClientTests, CreateRealClient_StartAndStopApplication_Succeeds)
//    {
//    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
//    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, localState);
//
//    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", "2545");
//    auto manager = ConnectSignInManager::Create(clientInfo, nullptr, localState);
//    BeFileName dbPath = GetUsageDbPath();
//
//    auto client = Client::Create(
//        dbPath,
//        manager,
//        clientInfo,
//        manager->GetUserInfo());
//    EXPECT_NE(nullptr, client);
//
//    EXPECT_SUCCESS(client->StartApplication());
//    EXPECT_SUCCESS(client->StopApplication());
//    }

// TODO: Create separate Project for Integration tests.
// TEST_F (ClientTests, GetPolicy_IntegrationTest)
//    {
//    auto client = CreateTestClient(true);
//
//    // Test code can be written like this:
//    auto policyToken = client->GetPolicy().get(); // This will re throw exeptions occured and they will be catched by gtest.
//    ASSERT_FALSE(policyToken->GetDefaultQualifier("HeartbeatInterval").isNull()); // Check if qualifier is in PolicyToken
//
//    JsonValueCR hbi = policyToken->GetDefaultQualifier("HeartbeatInterval");
//    auto nameText = hbi["Name"].asString();
//    auto valueInt = hbi["Value"].asInt();
//    
//    JsonValueCR ut = policyToken->GetDefaultQualifier("UsageType");
//    nameText = ut["Name"].asString();
//    auto valueText = ut["Value"].asString();

    /*JsonValueCR polid = policyToken->GetPolicyId();
    auto policyId = polid.ToString();
    policyId.ReplaceAll("\"", "");*/

    //auto ut = policyToken->GetQualifier("UsageType").asCString();

    //printf("\nHeartbeatInterval = %s\n", hbi.c_str());

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
    //}

TEST_F(ClientTests, GetProductStatus_Test)
	{
	auto client = CreateTestClient(true);
	client->StartApplication();
	// Add policies with unique ProductIds for testing multiple cases
	Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
	Utf8String userIdBad = "00000000-0000-0000-0000-000000000000";
	auto jsonPolicyValid = DummyJsonHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false);
	auto jsonPolicyValidTrial = DummyJsonHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9901, "", 1, true);
	auto jsonPolicyValidTrialExpired = DummyJsonHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(-1), userId, 9902, "", 1, true);
	auto jsonPolicyExpired = DummyJsonHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(-1), DateHelper::AddDaysToCurrentTime(7), userId, 9903, "", 1, false);
	auto jsonPolicyNoSecurables = DummyJsonHelper::CreatePolicyNoSecurables(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9904, "", 1, false);
	auto jsonPolicyNoACLs = DummyJsonHelper::CreatePolicyNoACLs(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9905, "", 1, false);
	auto jsonPolicyNoUserData = DummyJsonHelper::CreatePolicyNoUserData(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9906, "", 1, false);
	auto jsonPolicyNoRequestData = DummyJsonHelper::CreatePolicyNoRequestData(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9907, "", 1, false);
	auto jsonPolicyIdBad = DummyJsonHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userIdBad, 9908, "", 1, false);

	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyValid));
	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyValidTrial));
	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyValidTrialExpired));
	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyExpired));
	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyNoSecurables));
	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyNoACLs));
	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyNoUserData));
	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyNoRequestData));
	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyIdBad));
	
	// NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
	ASSERT_EQ((int)client->GetProductStatus(), (int)LicenseStatus::AccessDenied); // Obtained test policy should result in AccessDenied
	ASSERT_EQ((int)client->GetProductStatus(9900), (int)LicenseStatus::Ok);
	ASSERT_EQ((int)client->GetProductStatus(9901), (int)LicenseStatus::Trial);
	ASSERT_EQ((int)client->GetProductStatus(9902), (int)LicenseStatus::Expired);
	ASSERT_EQ((int)client->GetProductStatus(9903), (int)LicenseStatus::NotEntitled); // Policy is not valid due to expiration, therefore no entitlement
	ASSERT_EQ((int)client->GetProductStatus(9904), (int)LicenseStatus::NotEntitled);
	ASSERT_EQ((int)client->GetProductStatus(9905), (int)LicenseStatus::NotEntitled);
	ASSERT_EQ((int)client->GetProductStatus(9906), (int)LicenseStatus::NotEntitled);
	ASSERT_EQ((int)client->GetProductStatus(9907), (int)LicenseStatus::NotEntitled);
	ASSERT_EQ((int)client->GetProductStatus(9908), (int)LicenseStatus::NotEntitled);
	ASSERT_EQ((int)client->GetProductStatus(9999), (int)LicenseStatus::NotEntitled); // Policy with productId does not exist
	}

TEST_F(ClientTests, SendUsage_Success)
    {
    auto client = CreateTestClient(true, UrlProvider::Environment::Qa);

    SCVWritter writter;

    writter.AddRow(1004175881, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                   "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                   "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                   "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", DateTime::GetCurrentTimeUtc().ToString(),
                   1.0, "RealTime", "US", "Production");

    writter.AddRow(1004175881, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                   "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                   "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                   "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", DateTime::GetCurrentTimeUtc().ToString(),
                   1.0, "RealTime", "US", "Production");

    writter.AddRow(1004175881, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                   "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                   "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                   "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", DateTime::GetCurrentTimeUtc().ToString(),
                   1.0, "RealTime", "US", "Production");

    BeFileName path;
    BeTest::GetHost().GetTempDir(path);
    path.AppendToPath(L"test.csv");

    EXPECT_SUCCESS(writter.WriteToFile(path));

    try
        {
        client->SendUsage(path, "1004175881").get();
        EXPECT_SUCCESS(SUCCESS);
        }
    catch (HttpError err)
        {
        GTEST_FATAL_FAILURE_(err.GetMessage().c_str());
        }
    }

TEST_F(ClientTests, DISABLED_CreateDemoClient_StartAndStopApplication_Succeeds)
    {
    auto client = CreateTestClient(true);
    using namespace std::chrono_literals;

    EXPECT_EQ((int)client->StartApplication(), (int)LicenseStatus::AccessDenied);

    std::this_thread::sleep_for(5min);

    EXPECT_SUCCESS(client->StopApplication());
    }
