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

ClientImplPtr CreateTestClient(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId)
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, localState);

    //auto productId = "2545";
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
    return CreateTestClient(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, "2545");
    }

ClientImplPtr CreateTestClient(bool signIn, UrlProvider::Environment env)
    {
    return CreateTestClient(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), env, "2545");
    }

ClientImplPtr CreateTestClient(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor)
    {
    return CreateTestClient(signIn, heartbeatInterval, timeRetriever, delayedExecutor, UrlProvider::Environment::Qa, "2545");
    }

ClientImplPtr CreateTestClient(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, Utf8StringCR productId)
    {
    return CreateTestClient(signIn, heartbeatInterval, timeRetriever, delayedExecutor, UrlProvider::Environment::Qa, productId);
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

TEST_F(ClientTests, StartApplication_Success)
    {
    auto client = CreateTestClient(true);
    EXPECT_NE((int) client->StartApplication(), (int) LicenseStatus::Error);
    client->StopApplication();
    }

TEST_F(ClientTests, GetCertificate_Success)
    {
    Utf8String cert;

    auto client = CreateTestClient(true);
        
    EXPECT_NO_THROW(cert = client->GetCertificate().get());
    EXPECT_NE(cert.empty(), true);
    }

TEST_F(ClientTests, StopApplication_Success)
    {
    auto client = CreateTestClient(true);
    using namespace std::chrono_literals;

    client->StartApplication();

    std::this_thread::sleep_for(10s);

    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientTests, GetPolicy_Success)
    {
    auto client = CreateTestClient(true);

    auto policyToken = client->GetPolicy().get();
    EXPECT_NE(policyToken, nullptr);
    }

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
	auto jsonPolicyOfflineNotAllowed = DummyJsonHelper::CreatePolicyOfflineNotAllowed(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9909, "", 1, false);

	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyValid));
	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyValidTrial));
	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyValidTrialExpired));
	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyExpired));
	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyNoSecurables));
	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyNoACLs));
	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyNoUserData));
	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyNoRequestData));
	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyIdBad));
	client->AddPolicyTokenToDb(PolicyToken::Create(jsonPolicyOfflineNotAllowed));
	
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
	ASSERT_EQ((int)client->GetProductStatus(9909), (int)LicenseStatus::Ok); // Grace Period NOT started; should return Ok
	
	auto timestamp = Utf8String(DateHelper::TimeToString(DateHelper::GetCurrentTime()).c_str());
	auto timestampPast = Utf8String(DateHelper::TimeToString(DateHelper::AddDaysToCurrentTime(-14)).c_str()); // Two weeks ago; default offline period allowed is only 1 week
	
	client->GetUsageDb().SetOfflineGracePeriodStart(timestamp);
	ASSERT_EQ((int)client->GetProductStatus(9900), (int)LicenseStatus::Offline); // Valid status should be Offline now
	ASSERT_EQ((int)client->GetProductStatus(9909), (int)LicenseStatus::DisabledByPolicy); // Grace Period started; should be disabled
	client->GetUsageDb().ResetOfflineGracePeriod();
	ASSERT_EQ((int)client->GetProductStatus(9900), (int)LicenseStatus::Ok); // Should be back to Ok
	ASSERT_EQ((int)client->GetProductStatus(9909), (int)LicenseStatus::Ok); // Same for this
	client->GetUsageDb().SetOfflineGracePeriodStart(timestampPast);
	ASSERT_EQ(client->GetUsageDb().GetOfflineGracePeriodStart(),timestampPast);
	ASSERT_EQ((int)client->GetProductStatus(9900), (int)LicenseStatus::Expired); // Valid status should be Expired now, since offline grace period has expired
	}

TEST_F(ClientTests, CleanUpPolicies_Success)
	{
	auto client = CreateTestClient(true);
	client->StartApplication();
	// create expired/invalid and valid policies and add them to database
	Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
	auto jsonPolicyExpired1 = DummyJsonHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(-1), DateHelper::AddDaysToCurrentTime(7), userId, 9903, "", 1, false);
	auto jsonPolicyExpired2 = DummyJsonHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(-1), DateHelper::AddDaysToCurrentTime(7), userId, 9913, "", 1, false);
	auto jsonPolicyValid = DummyJsonHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false);
	auto expiredPolicyToken1 = PolicyToken::Create(jsonPolicyExpired1);
	auto expiredPolicyToken2 = PolicyToken::Create(jsonPolicyExpired2);
	auto validPolicyToken = PolicyToken::Create(jsonPolicyValid);
	client->AddPolicyTokenToDb(expiredPolicyToken1);
	client->AddPolicyTokenToDb(expiredPolicyToken2);
	client->AddPolicyTokenToDb(validPolicyToken);
	// all 3 should be located
	ASSERT_NE(client->GetPolicyWithId(expiredPolicyToken1->GetPolicyId()),nullptr);
	ASSERT_NE(client->GetPolicyWithId(expiredPolicyToken2->GetPolicyId()), nullptr);
	ASSERT_NE(client->GetPolicyWithId(validPolicyToken->GetPolicyId()), nullptr);
	// clean up policies; expired policy should be removed
	client->CleanUpPolicies();
	// invalid policy should NOT be located and thus be a nullptr
	ASSERT_EQ(client->GetPolicyWithId(expiredPolicyToken1->GetPolicyId()), nullptr);
	ASSERT_EQ(client->GetPolicyWithId(expiredPolicyToken2->GetPolicyId()), nullptr);
	ASSERT_NE(client->GetPolicyWithId(validPolicyToken->GetPolicyId()), nullptr);
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

    EXPECT_NO_THROW(client->SendUsage(path, "1004175881").wait());
    }

