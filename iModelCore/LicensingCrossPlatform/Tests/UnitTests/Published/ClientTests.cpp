/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/ClientTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ClientTests.h"
#include "DummyPolicyHelper.h"

#include <Licensing/Client.h>
#include <Licensing/Utils/DateHelper.h>
#include "../../../Licensing/ClientImpl.h"
#include "../../../Licensing/ClientFreeImpl.h"
#include "../../../Licensing/ClientWithKeyImpl.h"
#include "../../../Licensing/UsageDb.h"
#include "../../../PublicAPI/Licensing/Utils/SCVWritter.h"
#include "../../../PublicAPI/Licensing/Utils/UrlProvider.h"

#include <BeHttp/HttpClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeSQLite/BeSQLite.h>
#include <fstream>
#include <Licensing/Utils/InMemoryJsonLocalState.h>


#define TEST_PRODUCT_ID     "2545"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS
USING_NAMESPACE_BENTLEY_HTTP
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
    path.AppendToPath(L"License.db");

    return path;
    }

ClientImplPtr CreateTestClient(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId)
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, localState);

    auto applicationInfo = std::make_shared<ApplicationInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    auto manager = ConnectSignInManager::Create(applicationInfo, proxy, localState);
    if (signIn)
        {
        Credentials credentials("qa2_devuser2@mailinator.com", "bentley");
        if (!manager->SignInWithCredentials(credentials)->GetResult().IsSuccess())
            return nullptr;
        }
    BeFileName dbPath = GetUsageDbPath();
    
    return std::make_shared<ClientImpl>(
        manager->GetUserInfo(),
        applicationInfo,
        manager,
        dbPath,
        true,
        "",
        "",
        proxy);
    }

ClientFreeImplPtr CreateFreeTestClient(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId)
{
	InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
	UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, localState);
	auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

	return std::make_shared<ClientFreeImpl>(
		"",
		proxy);
}

ClientWithKeyImplPtr CreateWithKeyTestClient(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId)
{
	InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
	UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, localState);

	auto applicationInfo = std::make_shared<ApplicationInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

	auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();


	auto manager = ConnectSignInManager::Create(applicationInfo, proxy, localState);

	BeFileName dbPath = GetUsageDbPath();

	Utf8String accesskey = "somekey";

	return std::make_shared<ClientWithKeyImpl>(
		accesskey,
		applicationInfo,
		dbPath,
		true,
		"",
		"",
		proxy);
}

ClientPtr CreateTestClientFromFactory(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId)
{
	InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
	UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, localState);

	auto applicationInfo = std::make_shared<ApplicationInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

	auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
	auto manager = ConnectSignInManager::Create(applicationInfo, proxy, localState);
	if (signIn)
	{
		Credentials credentials("qa2_devuser2@mailinator.com", "bentley");
		if (!manager->SignInWithCredentials(credentials)->GetResult().IsSuccess())
			return nullptr;
	}
	BeFileName dbPath = GetUsageDbPath();

	return Client::Create(
		manager->GetUserInfo(),
		applicationInfo,
		manager,
		dbPath,
		true,
		"",
		"",
		proxy);
}

ClientPtr CreateFreeTestClientFromFactory(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId)
{
	InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
	UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, localState);

	auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

	return Client::CreateFree(
		"",
		proxy);
}

ClientPtr CreateWithKeyTestClientFromFactory(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId)
{
	InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
	UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, localState);

	auto applicationInfo = std::make_shared<ApplicationInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

	auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();


	auto manager = ConnectSignInManager::Create(ApplicationInfo, proxy, localState);

	BeFileName dbPath = GetUsageDbPath();

	Utf8String accesskey = "somekey";

	return Client::CreateWithKey(
		accesskey,
		applicationInfo,
		dbPath,
		true,
		"",
		"",
		proxy);
    }


ClientImplPtr CreateTestClient(bool signIn)
    {
    return CreateTestClient(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID);
    }

ClientPtr CreateTestClientFromFactory(bool signIn)
{
	return CreateTestClientFromFactory(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID);
}

ClientFreeImplPtr CreateFreeTestClient(bool signIn)
    {
	return CreateFreeTestClient(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID);
    }

ClientPtr CreateFreeTestClientFromFactory(bool signIn)
{
	return CreateFreeTestClientFromFactory(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID);
}

ClientWithKeyImplPtr CreateWithKeyTestClient(bool signIn)
{
	return CreateWithKeyTestClient(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID);
}

ClientPtr CreateWithKeyTestClientFromFactory(bool signIn)
{
	return CreateWithKeyTestClientFromFactory(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID);
}

ClientImplPtr CreateTestClient(bool signIn, UrlProvider::Environment env)
    {
    return CreateTestClient(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), env, TEST_PRODUCT_ID);
    }

ClientImplPtr CreateTestClient(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor)
    {
    return CreateTestClient(signIn, heartbeatInterval, timeRetriever, delayedExecutor, UrlProvider::Environment::Qa, TEST_PRODUCT_ID);
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

TEST_F(ClientTests, StartApplication_StopApplication_Success)
    {
    auto client = CreateTestClient(true);
    EXPECT_NE((int) client->StartApplication(), (int) LicenseStatus::Error);    
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientTests, StartApplication_StopApplication_Repeat_Success)
	{
	auto client = CreateTestClient(true);
	EXPECT_NE((int)client->StartApplication(), (int)LicenseStatus::Error);
	EXPECT_SUCCESS(client->StopApplication());
	EXPECT_NE((int)client->StartApplication(), (int)LicenseStatus::Error);
	EXPECT_SUCCESS(client->StopApplication());
	}

TEST_F(ClientTests, DISABLED_TrackUsage_FreeApplication_Success)
	{
	auto client = CreateFreeTestClient(true);
	Utf8String tokenstring = "b683fe041bfd1ef554599e69253271f5f6775eb7106514fa56e512040d635d4a";
	auto version = BeVersion(1, 0);
	Utf8String projectId = "00000000-0000-0000-0000-000000000000";
	EXPECT_SUCCESS(client->TrackUsage(tokenstring,version,projectId).get());
    }

TEST_F(ClientTests, StartWithKeyApplication_StopApplication_Success)
{
	auto client = CreateWithKeyTestClient(true);
	EXPECT_NE((int)client->StartApplication(), (int)LicenseStatus::Error);
	EXPECT_SUCCESS(client->StopApplication());
}

TEST_F(ClientTests, StartApplicationFromFactory_Success)
{
	auto client = CreateTestClientFromFactory(true);
	EXPECT_NE((int)client->StartApplication(), (int)LicenseStatus::Error);
	client->StopApplication();
}

TEST_F(ClientTests, DISABLED_TrackUsage_FreeApplicationFromFactory_Success)
{
	auto client = CreateFreeTestClientFromFactory(true);
	Utf8String tokenstring = "0cd2c3d2b0b813ce8c8e1b297f173f9e42f775ca32a2ee32a27b0a55daff1db9";
	auto version = BeVersion(1, 0);
	Utf8String projectId = "00000000-0000-0000-0000-000000000000";
	EXPECT_SUCCESS(client->TrackUsage(tokenstring, version, projectId).get());
}

TEST_F(ClientTests, StartWithKeyApplicationFromFactory_Success)
{
	auto client = CreateWithKeyTestClientFromFactory(true);
	EXPECT_NE((int)client->StartApplication(), (int)LicenseStatus::Error);
	client->StopApplication();
}

TEST_F(ClientTests, GetCertificate_Success)
    {
    Utf8String cert;

    auto client = CreateTestClient(true);
        
    EXPECT_NO_THROW(cert = client->GetCertificate().get());
    EXPECT_NE(cert.empty(), true);
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
    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false);
    auto jsonPolicyValidTrial = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9901, "", 1, true);
    auto jsonPolicyValidTrialExpired = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(-1), userId, 9902, "", 1, true);
    auto jsonPolicyExpired = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(-1), DateHelper::AddDaysToCurrentTime(7), userId, 9903, "", 1, false);
    auto jsonPolicyNoSecurables = DummyPolicyHelper::CreatePolicyNoSecurables(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9904, "", 1, false);
    auto jsonPolicyNoACLs = DummyPolicyHelper::CreatePolicyNoACLs(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9905, "", 1, false);
    auto jsonPolicyNoUserData = DummyPolicyHelper::CreatePolicyNoUserData(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9906, "", 1, false);
    auto jsonPolicyNoRequestData = DummyPolicyHelper::CreatePolicyNoRequestData(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9907, "", 1, false);
    auto jsonPolicyIdBad = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userIdBad, 9908, "", 1, false);
    auto jsonPolicyOfflineNotAllowed = DummyPolicyHelper::CreatePolicyOfflineNotAllowed(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9909, "", 1, false);

	// Add empty policy to make sure does not cause issues for policy searching
	client->GetUsageDb().AddOrUpdatePolicyFile("11111111-1111-1111-1111-111111111111", "", "", "", "");

	client->AddPolicyToDb(Policy::Create(jsonPolicyValid));
	client->AddPolicyToDb(Policy::Create(jsonPolicyValidTrial));
	client->AddPolicyToDb(Policy::Create(jsonPolicyValidTrialExpired));
	client->AddPolicyToDb(Policy::Create(jsonPolicyExpired));
	client->AddPolicyToDb(Policy::Create(jsonPolicyNoSecurables));
	client->AddPolicyToDb(Policy::Create(jsonPolicyNoACLs));
	client->AddPolicyToDb(Policy::Create(jsonPolicyNoUserData));
	client->AddPolicyToDb(Policy::Create(jsonPolicyNoRequestData));
	client->AddPolicyToDb(Policy::Create(jsonPolicyIdBad));
	client->AddPolicyToDb(Policy::Create(jsonPolicyOfflineNotAllowed));
	
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
	
	auto timestamp = DateHelper::GetCurrentTime();
	auto timestampPast = DateHelper::AddDaysToCurrentTime(-14); // Two weeks ago; default offline period allowed is only 1 week
	
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
	Utf8String userIdOther = "00000000-0000-0000-0000-000000000000";
    auto jsonPolicyExpired1 = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(-1), DateHelper::AddDaysToCurrentTime(7), userId, 9903, "", 1, false);
    auto jsonPolicyExpired2 = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(-1), DateHelper::AddDaysToCurrentTime(7), userId, 9913, "", 1, false);
    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9904, "", 1, false);
    auto jsonPolicyValid2 = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9905, "", 1, false);
    auto jsonPolicyValid3 = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userIdOther, 9906, "", 1, false);
    auto expiredPolicy1 = Policy::Create(jsonPolicyExpired1);
	auto expiredPolicy2 = Policy::Create(jsonPolicyExpired2);
	auto validPolicy = Policy::Create(jsonPolicyValid);
	auto validPolicy2 = Policy::Create(jsonPolicyValid2);
	auto validPolicy3 = Policy::Create(jsonPolicyValid3);
	client->AddPolicyToDb(expiredPolicy1);
	client->AddPolicyToDb(expiredPolicy2);
	client->AddPolicyToDb(validPolicy);
	client->AddPolicyToDb(validPolicy2);
	client->AddPolicyToDb(validPolicy3);
	// all 3 should be located
	ASSERT_NE(client->GetPolicyWithId(expiredPolicy1->GetPolicyId()),nullptr);
	ASSERT_NE(client->GetPolicyWithId(expiredPolicy2->GetPolicyId()), nullptr);
	ASSERT_NE(client->GetPolicyWithId(validPolicy->GetPolicyId()), nullptr);
	// clean up policies; expired policy should be removed
	client->CleanUpPolicies();
	// invalid policy should NOT be located and thus be a nullptr
	ASSERT_EQ(client->GetPolicyWithId(expiredPolicy1->GetPolicyId()), nullptr);
	ASSERT_EQ(client->GetPolicyWithId(expiredPolicy2->GetPolicyId()), nullptr);
	ASSERT_NE(client->GetPolicyWithId(validPolicy->GetPolicyId()), nullptr);
	// delete other policies for user; policies that don't match policyId for that user should be removed
	client->DeleteAllOtherUserPolicies(validPolicy);
	// provided policy and other user's policy should remain, while the other user policy should be gone
	ASSERT_NE(client->GetPolicyWithId(validPolicy->GetPolicyId()), nullptr);
	ASSERT_EQ(client->GetPolicyWithId(validPolicy2->GetPolicyId()), nullptr);
	ASSERT_NE(client->GetPolicyWithId(validPolicy3->GetPolicyId()), nullptr);
	}

TEST_F(ClientTests, SendUsageLogs_Success)
    {
    auto client = CreateTestClient(true, UrlProvider::Environment::Qa);
    ASSERT_NE(client, nullptr);

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
    path.AppendToPath(L"Usages.csv");

    EXPECT_SUCCESS(writter.WriteToFile(path));

    EXPECT_NO_THROW(client->SendUsageLogs(path, "1004175881").wait());
    }

TEST_F(ClientTests, SendFeatureLogs_Success)
    {
    auto client = CreateTestClient(true, UrlProvider::Environment::Qa);
    ASSERT_NE(client, nullptr);

    SCVWritter writter;

    Utf8String currentTime = DateTime::GetCurrentTimeUtc().ToString();

    writter.AddRow(1004175881, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                   "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                   "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                   "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", currentTime,
                   1.0, "RealTime", "US", "Production", "6270e4fc-9b5b-4d3a-a4b2-81ef9c6ca440", currentTime, currentTime, "");

    writter.AddRow(1004175881, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                   "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                   "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                   "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", currentTime,
                   1.0, "RealTime", "US", "Production", "0b5c9fc1-f80f-4af9-a65d-bcf2d2cc0612", currentTime, currentTime, "Feature^DrawLine#");

    writter.AddRow(1004175881, "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "dfdc08b5-2077-4b73-8fc1-c60cb47abc63", "TestDeviceId",
                   "IXravQ3f71wUupkp+tLBK+vGmCc=", "qa2_devuser2@mailinator.com", "3Q746c3/YJfSzlDyMbrq6oMUbMQ=",
                   "7a265495-71a8-4557-bbaf-de57f31b26b8", "4d701223-37ca-4ffb-b91c-f650a937d6fd", 2545, "", 1000000000000,
                   "00000000-0000-0000-0000-000000000000", "c0d1ed44-3b6c-4316-9f3e-e856c85b4995", currentTime,
                   1.0, "RealTime", "US", "Production", "dce293da-e914-4a90-bb21-de27d48d51ea", currentTime, currentTime,
                   "Feature^DrawCircle#Manufacturer^Bentley#Comment^This is, in fact, a test.#");

    BeFileName path;
    BeTest::GetHost().GetTempDir(path);
    path.AppendToPath(L"Features.csv");

    EXPECT_SUCCESS(writter.WriteToFile(path));

    EXPECT_NO_THROW(client->SendFeatureLogs(path, "1004175881").wait());
    }
