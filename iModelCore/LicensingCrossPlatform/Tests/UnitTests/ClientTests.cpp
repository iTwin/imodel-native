/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/ClientTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestsHelper.h"
#include "JsonHelper.h"
#include "ClientTests.h"
#include "Utils/MockHttpHandler.h"
#include "DummyPolicyHelper.h"
#include "DummyUserInfoHelper.h"

#include <Licensing/Client.h>
#include <Licensing/SaasClient.h>
#include <Licensing/Utils/DateHelper.h>
#include "../../Licensing/ClientImpl.h"
#include "../../Licensing/SaasClientImpl.h"
#include "../../Licensing/ClientWithKeyImpl.h"
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

#define TEST_HEARTBEAT_INTERVAL 10 // this is not passed to the actual client, so it does nothing
#define TEST_ACCESSKEY          "somekey"

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

BeFileName GetLicensingDbPath()
    {
    BeFileName path;
    BeTest::GetHost().GetTempDir(path);
    path.AppendToPath(L"License.db");

    return path;
    }



// Note: cannot use mocks with clients created with the factory
//ClientPtr CreateTestClientFromFactory(ConnectSignInManager::UserInfo userInfo, Utf8StringCR productId = TEST_PRODUCT_ID)
//    {
//    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);
//
//    BeFileName dbPath = GetLicensingDbPath();
//
//    return Client::Create(
//        userInfo,
//        clientInfo,
//        nullptr,
//        dbPath,
//        true,
//        "",
//        "",
//        nullptr);
//    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClientTests::ClientTests() :
    m_handler(std::make_shared<MockHttpHandler>()),
    m_policyProviderMock(std::make_shared<PolicyProviderMock>()),
    m_ulasProviderMock(std::make_shared<UlasProviderMock>()),
    m_licensingDbMock(std::make_shared<LicensingDbMock>())
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientTests::TearDown()
    {
    m_handler->ValidateAndClearExpectations();
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

    BeFileName path;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(path);
    path.AppendToPath(L"TestAssets/sqlang/DgnClientFx_en.sqlang.db3");

    EXPECT_EQ(SUCCESS, L10N::Initialize(BeSQLite::L10N::SqlangFiles(path)));
    }

ClientImplPtr ClientTests::CreateTestClient(ConnectSignInManager::UserInfo userInfo, Utf8StringCR productId) const
    {
    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    BeFileName dbPath = GetLicensingDbPath();

    return std::make_shared<ClientImpl> (
        userInfo,
        clientInfo,
        dbPath,
        true,
        GetPolicyProviderMockPtr(),
        GetUlasProviderMockPtr(),
        "",
        "",
        GetLicensingDbMockPtr()
        );
    }

TEST_F(ClientTests, DISABLED_JsonExample)
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

// TODO move to integration tests
//TEST_F(ClientTests, CreateClientFromFactory_Success)
//    {
//    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", "userId", "orgId");
//    auto client = CreateTestClientFromFactory(userInfo);
//
//    EXPECT_NE(nullptr, client);
//    }

TEST_F(ClientTests, StartApplication_Error)
    {
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", "userId", "orgId");
    auto client = CreateTestClient(userInfo);

    GetLicensingDbMock().MockOpenOrCreate(ERROR);

    EXPECT_EQ(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));
    EXPECT_EQ(1, GetLicensingDbMock().OpenOrCreateCount());

    EXPECT_SUCCESS(client->StopApplication());
    EXPECT_EQ(1, GetLicensingDbMock().CloseCount());
    }

TEST_F(ClientTests, StartApplicationNoHeartbeat_Success)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo);

    GetLicensingDbMock().MockOpenOrCreate(SUCCESS);

    auto jsonPolicyNoUserData = DummyPolicyHelper::CreatePolicyNoUserData(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    auto noUserDataPolicy = Policy::Create(jsonPolicyNoUserData);

    GetPolicyProviderMock().MockGetPolicy(noUserDataPolicy);

    GetLicensingDbMock().MockAddOrUpdatePolicyFile(SUCCESS);
    GetLicensingDbMock().MockDeleteAllOtherPolicyFilesByUser(SUCCESS);

    std::list<Json::Value> noUserDataPolicyList;
    noUserDataPolicyList.push_back(jsonPolicyNoUserData);

    GetLicensingDbMock().MockUserPolicyFiles(userId, noUserDataPolicyList);

    EXPECT_EQ(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::NotEntitled)); // not entitiled so skips the heartbeat calls
    EXPECT_EQ(1, GetPolicyProviderMock().GetPolicyCalls());
    EXPECT_EQ(1, GetLicensingDbMock().OpenOrCreateCount());
    EXPECT_EQ(1, GetLicensingDbMock().AddOrUpdatePolicyFileCount());
    EXPECT_EQ(1, GetLicensingDbMock().DeleteAllOtherPolicyFilesByUserCount());
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));

    EXPECT_SUCCESS(client->StopApplication());
    EXPECT_EQ(1, GetLicensingDbMock().CloseCount());
    }

TEST_F(ClientTests, StartApplicationStopApplication_Success)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo);

    GetLicensingDbMock().MockOpenOrCreate(SUCCESS);

    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    auto validPolicy = Policy::Create(jsonPolicyValid);

    GetPolicyProviderMock().MockGetPolicy(validPolicy);
    GetLicensingDbMock().MockAddOrUpdatePolicyFile(SUCCESS);
    GetLicensingDbMock().MockDeleteAllOtherPolicyFilesByUser(SUCCESS);

    std::list<Json::Value> validPolicyList;
    validPolicyList.push_back(jsonPolicyValid);

    GetLicensingDbMock().MockUserPolicyFiles(userId, validPolicyList);

    // called in usage heartbeat
    GetLicensingDbMock().MockRecordUsage(SUCCESS);

    // LogPosting heartbeat
    GetLicensingDbMock().MockGetUsageRecordCount(1);
    GetLicensingDbMock().MockGetFeatureRecordCount(1);

    GetUlasProviderMock().MockPostUsageLogs(SUCCESS);
    GetUlasProviderMock().MockPostFeatureLogs(SUCCESS);

    // called in Policy heartbeat
    GetLicensingDbMock().MockGetOfflineGracePeriodStart("");

    // EQ for exact number of expected calls, LE for "At least" number of calls
    EXPECT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));
    EXPECT_EQ(1, GetPolicyProviderMock().GetPolicyCalls());
    EXPECT_EQ(1, GetLicensingDbMock().OpenOrCreateCount());
    EXPECT_LE(1, GetLicensingDbMock().AddOrUpdatePolicyFileCount());
    EXPECT_LE(1, GetLicensingDbMock().DeleteAllOtherPolicyFilesByUserCount());
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));

    EXPECT_SUCCESS(client->StopApplication());
    EXPECT_LE(1, GetUlasProviderMock().PostUsageLogsCalls());
    EXPECT_LE(1, GetUlasProviderMock().PostFeatureLogsCalls());
    EXPECT_EQ(1, GetLicensingDbMock().RecordUsageCount()); // called in usage heartbeat
    EXPECT_LE(1, GetLicensingDbMock().GetUsageRecordCountCount());
    EXPECT_LE(1, GetLicensingDbMock().GetFeatureRecordCountCount());
    EXPECT_LE(1, GetLicensingDbMock().GetOfflineGracePeriodStartCount());
    EXPECT_EQ(1, GetLicensingDbMock().CloseCount());
    }

// Tests for specific situations
TEST_F(ClientTests, GetPolicy_Success)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false);
    auto policy = Policy::Create(jsonPolicyValid);

    GetPolicyProviderMock().MockGetPolicy(policy);

    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", "userId", "orgId");
    auto client = CreateTestClient(userInfo);

    auto policyToken = client->GetPolicy().get();
    EXPECT_NE(policyToken, nullptr); //not testing the policy token here, just that GetPolicy is called
    EXPECT_EQ(1, GetPolicyProviderMock().GetPolicyCalls());
    }

TEST_F(ClientTests, GetCertificate_Success_HttpMock)
    {
    auto url = "https://qa-connect-ulastm.bentley.com/Bentley.Entitlement.PolicyService/PolicySvcWebApi/api";

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (Http::RequestCR request)
        {
        EXPECT_EQ(url, request.GetUrl());
        return MockHttpHandler::StubHttpResponse();
        });

    HttpClient client(nullptr, GetHandlerPtr());
    auto cert = client.CreateGetRequest(url).Perform().get();
    }

TEST_F(ClientTests, GetProductStatusEmpty_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo);

    std::list<Json::Value> emptyPolicyList;

    GetLicensingDbMock().MockUserPolicyFiles(userId, emptyPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetProductStatus()), static_cast<int>(LicenseStatus::NotEntitled));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetProductStatusValid_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<Json::Value> validPolicyList;
    validPolicyList.push_back(jsonPolicyValid);

    GetLicensingDbMock().MockUserPolicyFiles(userId, validPolicyList);
    GetLicensingDbMock().MockGetOfflineGracePeriodStart("");

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetProductStatus()), static_cast<int>(LicenseStatus::Ok));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    EXPECT_EQ(1, GetLicensingDbMock().GetOfflineGracePeriodStartCount());
    }

TEST_F(ClientTests, GetProductStatusValidTrial_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyValidTrial = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, true);
    std::list<Json::Value> validTrialPolicyList;
    validTrialPolicyList.push_back(jsonPolicyValidTrial);

    GetLicensingDbMock().MockUserPolicyFiles(userId, validTrialPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetProductStatus()), static_cast<int>(LicenseStatus::Trial));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetProductStatusExpiredTrial_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyExpiredTrial = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(-1), userId, std::atoi(TEST_PRODUCT_ID), "", 1, true);
    std::list<Json::Value> expiredTrialPolicyList;
    expiredTrialPolicyList.push_back(jsonPolicyExpiredTrial);

    GetLicensingDbMock().MockUserPolicyFiles(userId, expiredTrialPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetProductStatus()), static_cast<int>(LicenseStatus::Expired));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetProductStatusExpired_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyExpired = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(-1), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<Json::Value> expiredPolicyList;
    expiredPolicyList.push_back(jsonPolicyExpired);

    GetLicensingDbMock().MockUserPolicyFiles(userId, expiredPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetProductStatus()), static_cast<int>(LicenseStatus::NotEntitled)); // not valid, so SearchForPolicy returns nullptr
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetProductStatusNoSecurables_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyNoSecurables = DummyPolicyHelper::CreatePolicyNoSecurables(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<Json::Value> noSecurablesPolicyList;
    noSecurablesPolicyList.push_back(jsonPolicyNoSecurables);

    GetLicensingDbMock().MockUserPolicyFiles(userId, noSecurablesPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetProductStatus()), static_cast<int>(LicenseStatus::NotEntitled));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetProductStatusNoACLs_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyNoACLs = DummyPolicyHelper::CreatePolicyNoACLs(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<Json::Value> noACLsPolicyList;
    noACLsPolicyList.push_back(jsonPolicyNoACLs);

    GetLicensingDbMock().MockUserPolicyFiles(userId, noACLsPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetProductStatus()), static_cast<int>(LicenseStatus::NotEntitled));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetProductStatusNoUserData_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyNoUserData = DummyPolicyHelper::CreatePolicyNoUserData(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<Json::Value> noUserDataPolicyList;
    noUserDataPolicyList.push_back(jsonPolicyNoUserData);

    GetLicensingDbMock().MockUserPolicyFiles(userId, noUserDataPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetProductStatus()), static_cast<int>(LicenseStatus::NotEntitled));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetProductStatusNoRequestData_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyNoRequestData = DummyPolicyHelper::CreatePolicyNoRequestData(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<Json::Value> noRequestDataPolicyList;
    noRequestDataPolicyList.push_back(jsonPolicyNoRequestData);

    GetLicensingDbMock().MockUserPolicyFiles(userId, noRequestDataPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetProductStatus()), static_cast<int>(LicenseStatus::NotEntitled));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetProductStatusIdBad_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    Utf8String userIdBad = "00000000-0000-0000-0000-000000000000";

    auto jsonPolicyIdBad = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userIdBad, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<Json::Value> idBadPolicyList;
    idBadPolicyList.push_back(jsonPolicyIdBad);

    GetLicensingDbMock().MockUserPolicyFiles(userId, idBadPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetProductStatus()), static_cast<int>(LicenseStatus::NotEntitled));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetProductStatusOfflineNotAllowed_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyOfflineNotAllowed = DummyPolicyHelper::CreatePolicyOfflineNotAllowed(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<Json::Value> offlineNotAllowedPolicyList;
    offlineNotAllowedPolicyList.push_back(jsonPolicyOfflineNotAllowed);

    GetLicensingDbMock().MockUserPolicyFiles(userId, offlineNotAllowedPolicyList);
    GetLicensingDbMock().MockGetOfflineGracePeriodStart("");

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetProductStatus()), static_cast<int>(LicenseStatus::Ok));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    EXPECT_EQ(1, GetLicensingDbMock().GetOfflineGracePeriodStartCount());
    }

TEST_F(ClientTests, GetProductStatusGracePeriodStartedOfflineNotAllowed_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyOfflineNotAllowed = DummyPolicyHelper::CreatePolicyOfflineNotAllowed(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<Json::Value> offlineNotAllowedPolicyList;
    offlineNotAllowedPolicyList.push_back(jsonPolicyOfflineNotAllowed);

    auto timestamp = DateHelper::GetCurrentTime();

    GetLicensingDbMock().MockUserPolicyFiles(userId, offlineNotAllowedPolicyList);
    GetLicensingDbMock().MockGetOfflineGracePeriodStart(timestamp);

    EXPECT_EQ(static_cast<int>(client->GetProductStatus()), static_cast<int>(LicenseStatus::DisabledByPolicy)); // Grace Period started; should be disabled
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    EXPECT_EQ(1, GetLicensingDbMock().GetOfflineGracePeriodStartCount());
    }

TEST_F(ClientTests, GetProductStatusGracePeriodStarted_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<Json::Value> validPolicyList;
    validPolicyList.push_back(jsonPolicyValid);

    auto timestamp = DateHelper::GetCurrentTime();

    GetLicensingDbMock().MockUserPolicyFiles(userId, validPolicyList);
    GetLicensingDbMock().MockGetOfflineGracePeriodStart(timestamp);

    EXPECT_EQ(static_cast<int>(client->GetProductStatus()), static_cast<int>(LicenseStatus::Offline)); // Valid status should be Offline now
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    EXPECT_EQ(2, GetLicensingDbMock().GetOfflineGracePeriodStartCount());
    }

TEST_F(ClientTests, GetProductStatusExpiredGracePeriod_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<Json::Value> validPolicyList;
    validPolicyList.push_back(jsonPolicyValid);

    auto timestampPast = DateHelper::AddDaysToCurrentTime(-14); // Two weeks ago; default offline period allowed is only 1 week

    GetLicensingDbMock().MockUserPolicyFiles(userId, validPolicyList);
    GetLicensingDbMock().MockGetOfflineGracePeriodStart(timestampPast);

    EXPECT_EQ(static_cast<int>(client->GetProductStatus()), static_cast<int>(LicenseStatus::Expired)); // Valid status should be Expired now, since offline grace period has expired
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    EXPECT_LE(1, GetLicensingDbMock().GetOfflineGracePeriodStartCount());
    }

TEST_F(ClientTests, CleanUpPolicies_Success)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo);

    auto jsonPolicyExpired = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(-1), DateHelper::AddDaysToCurrentTime(7), userId, 9903, "", 1, false);
    std::list<Json::Value> expiredPolicyList;
    expiredPolicyList.push_back(jsonPolicyExpired);

    GetLicensingDbMock().MockPolicyFiles(expiredPolicyList);
    GetLicensingDbMock().MockDeletePolicyFile(SUCCESS);

    client->CleanUpPolicies();
    EXPECT_LE(1, GetLicensingDbMock().GetPolicyFilesCount());
    EXPECT_LE(1, GetLicensingDbMock().DeletePolicyFileCount());
    }

TEST_F(ClientTests, DeleteAllOtherPoliciesByUser_Success)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo);

    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false);
    auto validPolicy = Policy::Create(jsonPolicyValid);

    GetLicensingDbMock().MockDeleteAllOtherPolicyFilesByUser(SUCCESS);

    client->DeleteAllOtherPoliciesByUser(validPolicy);
    EXPECT_LE(1, GetLicensingDbMock().DeleteAllOtherPolicyFilesByUserCount());
    }

TEST_F(ClientTests, MarkFeature_Success)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo);

    GetLicensingDbMock().MockOpenOrCreate(SUCCESS);

    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    auto validPolicy = Policy::Create(jsonPolicyValid);

    GetPolicyProviderMock().MockGetPolicy(validPolicy); 

    GetLicensingDbMock().MockAddOrUpdatePolicyFile(SUCCESS);
    GetLicensingDbMock().MockDeleteAllOtherPolicyFilesByUser(SUCCESS);

    std::list<Json::Value> validPolicyList;
    validPolicyList.push_back(jsonPolicyValid);

    GetLicensingDbMock().MockUserPolicyFiles(userId, validPolicyList);
    GetLicensingDbMock().MockGetOfflineGracePeriodStart("");
    GetLicensingDbMock().MockRecordFeature(SUCCESS);

    FeatureUserDataMapPtr featureAttribute = std::make_shared<FeatureUserDataMap>();

    featureAttribute->AddAttribute("Manufacturer", "Bentley Systems, Inc.");
    featureAttribute->AddAttribute("Website", "https://www.w3schools.com");
    featureAttribute->AddAttribute("Title", "Mobile App");

    EXPECT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));
    EXPECT_SUCCESS(client->MarkFeature("FeatureId", featureAttribute));
    EXPECT_EQ(1, GetPolicyProviderMock().GetPolicyCalls());
    EXPECT_EQ(1, GetLicensingDbMock().AddOrUpdatePolicyFileCount());
    EXPECT_EQ(1, GetLicensingDbMock().DeleteAllOtherPolicyFilesByUserCount());
    EXPECT_EQ(2, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    EXPECT_EQ(1, GetLicensingDbMock().OpenOrCreateCount());
    EXPECT_LE(1, GetLicensingDbMock().GetOfflineGracePeriodStartCount());

    EXPECT_SUCCESS(client->StopApplication());
    EXPECT_EQ(1, GetLicensingDbMock().CloseCount());
    }

