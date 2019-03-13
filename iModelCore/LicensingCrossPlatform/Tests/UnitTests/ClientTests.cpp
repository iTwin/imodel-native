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

using ::testing::AtLeast;
using ::testing::Return;
using ::testing::ByMove;
using ::testing::A;
using ::testing::_;

#define TEST_PRODUCT_ID         "2545"
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

ClientImplPtr CreateTestClient(ConnectSignInManager::UserInfo userInfo, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId, IPolicyProviderPtr policyProvider, IUlasProviderPtr ulasProvider, ILicensingDbPtr licensingDb)
    {
    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    BeFileName dbPath = GetLicensingDbPath();

    return std::make_shared<ClientImpl>
        (
        userInfo,
        clientInfo,
        dbPath,
        true,
        policyProvider,
        ulasProvider,
        "",
        "",
        licensingDb
        );
    }

SaasClientImplPtr CreateTestSaasClient(uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, int productId, IUlasProviderPtr ulasProvider)
    {
    return std::make_shared<SaasClientImpl>(
        productId,
        "",
        ulasProvider);
    }

ClientWithKeyImplPtr CreateWithKeyTestClient(uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId, IPolicyProviderPtr policyProvider, IUlasProviderPtr ulasProvider, ILicensingDbPtr licensingDb)
    {
    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    BeFileName dbPath = GetLicensingDbPath();

    return std::make_shared<ClientWithKeyImpl>(
        TEST_ACCESSKEY,
        clientInfo,
        dbPath,
        true,
        policyProvider,
        ulasProvider,
        "",
        "",
        licensingDb);
    }

// Note: cannot use mocks with clients created with the factory
ClientPtr CreateTestClientFromFactory(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId)
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
    BeFileName dbPath = GetLicensingDbPath();

    return Client::Create(
        manager->GetUserInfo(),
        clientInfo,
        manager,
        dbPath,
        true,
        "",
        "",
        proxy);
    }

SaasClientPtr CreateTestSaasClientFromFactory(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, int productId)
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, localState);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    return SaasClient::Create(
        productId,
        "",
        proxy);
    }

ClientPtr CreateWithKeyTestClientFromFactory(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId)
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, localState);

    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();


    auto manager = ConnectSignInManager::Create(clientInfo, proxy, localState);

    BeFileName dbPath = GetLicensingDbPath();

    return Client::CreateWithKey(
        TEST_ACCESSKEY,
        clientInfo,
        dbPath,
        true,
        "",
        "",
        proxy);
    }

ClientImplPtr CreateTestClient(ConnectSignInManager::UserInfo userInfo, IPolicyProviderPtr policyProvider, IUlasProviderPtr ulasProvider, ILicensingDbPtr licensingDb)
    {
    return CreateTestClient(userInfo, TEST_HEARTBEAT_INTERVAL, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID, policyProvider, ulasProvider, licensingDb);
    }

ClientPtr CreateTestClientFromFactory(bool signIn)
    {
    return CreateTestClientFromFactory(signIn, TEST_HEARTBEAT_INTERVAL, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID);
    }

SaasClientImplPtr CreateTestSaasClient(IUlasProviderPtr ulasProvider)
    {
    return CreateTestSaasClient(TEST_HEARTBEAT_INTERVAL, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, std::atoi(TEST_PRODUCT_ID), ulasProvider);
    }

SaasClientPtr CreateTestSaasClientFromFactory(bool signIn)
    {
    return CreateTestSaasClientFromFactory(signIn, TEST_HEARTBEAT_INTERVAL, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, std::atoi(TEST_PRODUCT_ID));
    }

ClientWithKeyImplPtr CreateWithKeyTestClient(IPolicyProviderPtr policyProvider, IUlasProviderPtr ulasProvider, ILicensingDbPtr licensingDb)
    {
    return CreateWithKeyTestClient(TEST_HEARTBEAT_INTERVAL, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID, policyProvider, ulasProvider, licensingDb);
    }

ClientPtr CreateWithKeyTestClientFromFactory(bool signIn)
    {
    return CreateWithKeyTestClientFromFactory(signIn, TEST_HEARTBEAT_INTERVAL, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID);
    }

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
MockHttpHandler& ClientTests::GetHandler() const
    {
    return *m_handler;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<MockHttpHandler> ClientTests::GetHandlerPtr() const
    {
    return m_handler;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PolicyProviderMock&  ClientTests::GetPolicyProviderMock() const
    {
    return *m_policyProviderMock;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<PolicyProviderMock> ClientTests::GetPolicyProviderMockPtr() const
    {
    return m_policyProviderMock;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UlasProviderMock&  ClientTests::GetUlasProviderMock() const
    {
    return *m_ulasProviderMock;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<UlasProviderMock> ClientTests::GetUlasProviderMockPtr() const
    {
    return m_ulasProviderMock;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LicensingDbMock&  ClientTests::GetLicensingDbMock() const
    {
    return *m_licensingDbMock;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<LicensingDbMock> ClientTests::GetLicensingDbMockPtr() const
    {
    return m_licensingDbMock;
    }

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

    ASSERT_EQ(SUCCESS, L10N::Initialize(BeSQLite::L10N::SqlangFiles(path)));
    }

TEST_F(ClientTests, JsonExample)
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

	ASSERT_EQ(a, 0);
	ASSERT_NE(b, 0);
	ASSERT_NE(c, 0);
}

TEST_F(ClientTests, StartApplication_Error)
    {
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", "userId", "orgId");
    auto client = CreateTestClient(userInfo, GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetLicensingDbMockPtr());

    GetLicensingDbMock().MockOpenOrCreate(ERROR);

    EXPECT_EQ((int)client->StartApplication(), (int)LicenseStatus::Error);
    EXPECT_EQ(1, GetLicensingDbMock().OpenOrCreateCount());
    }

TEST_F(ClientTests, StartApplicationNoHeartbeat_Success)
    {
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", "userId", "orgId");
    auto client = CreateTestClient(userInfo, GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetLicensingDbMockPtr());

    GetLicensingDbMock().MockOpenOrCreate(SUCCESS);

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false);
    auto validPolicy = Policy::Create(jsonPolicyValid);

    std::list<Json::Value> validPolicyList;
    validPolicyList.push_back(jsonPolicyValid);

    GetPolicyProviderMock().MockGetPolicy(validPolicy);

    EXPECT_CALL(GetLicensingDbMock(), AddOrUpdatePolicyFile(A<Utf8StringCR>(), A<Utf8StringCR>(), A<Utf8StringCR>(), A<Utf8StringCR>(), A<Utf8StringCR>(), A<Json::Value>()))
        .Times(AtLeast(1));

    EXPECT_CALL(GetLicensingDbMock(), DeleteAllOtherPolicyFilesByUser(A<Utf8StringCR>(), A<Utf8StringCR>()))
        .Times(AtLeast(1));

    GetLicensingDbMock().MockUserPolicyFiles(userId, validPolicyList);

    EXPECT_NE((int)client->StartApplication(), (int)LicenseStatus::Error); // not entitiled so skips the heartbeat calls
    EXPECT_EQ(1, GetPolicyProviderMock().GetPolicyCalls());
    EXPECT_EQ(1, GetLicensingDbMock().OpenOrCreateCount());
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, StartApplicationStopApplication_Success)
    {
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa", "orgId");
    auto client = CreateTestClient(userInfo, GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetLicensingDbMockPtr());

    GetLicensingDbMock().MockOpenOrCreate(SUCCESS);

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    auto validPolicy = Policy::Create(jsonPolicyValid);

    std::list<Json::Value> validPolicyList;
    validPolicyList.push_back(jsonPolicyValid);

    GetPolicyProviderMock().MockGetPolicy(validPolicy);

    EXPECT_CALL(GetLicensingDbMock(), AddOrUpdatePolicyFile(A<Utf8StringCR>(), A<Utf8StringCR>(), A<Utf8StringCR>(), A<Utf8StringCR>(), A<Utf8StringCR>(), A<Json::Value>()))
        .Times(AtLeast(1));

    EXPECT_CALL(GetLicensingDbMock(), DeleteAllOtherPolicyFilesByUser(A<Utf8StringCR>(), A<Utf8StringCR>()))
        .Times(AtLeast(1));

    GetLicensingDbMock().MockUserPolicyFiles(userId, validPolicyList);

    // LogPosting heartbeat
    EXPECT_CALL(GetLicensingDbMock(), GetUsageRecordCount())
        .WillRepeatedly(Return(1)); // will get PostUsageLogs() called
    EXPECT_CALL(GetLicensingDbMock(), GetFeatureRecordCount())
        .WillRepeatedly(Return(1)); // will get PostFeatureLogs() called

    GetUlasProviderMock().MockPostUsageLogs(BentleyStatus::SUCCESS);
    GetUlasProviderMock().MockPostFeatureLogs(BentleyStatus::SUCCESS);

    // called in Policy heartbeat
    EXPECT_CALL(GetLicensingDbMock(), GetOfflineGracePeriodStart())
        .WillRepeatedly(Return(""));

    EXPECT_NE((int)client->StartApplication(), (int)LicenseStatus::Error);
    EXPECT_EQ(1, GetLicensingDbMock().OpenOrCreateCount());
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    EXPECT_SUCCESS(client->StopApplication());
    EXPECT_EQ(1, GetLicensingDbMock().RecordUsageCount()); // called in usage heartbeat
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
    auto client = CreateTestClient(userInfo, GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetLicensingDbMockPtr());

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


// TODO these should be 12 different unit tests
//TEST_F(ClientTests, GetProductStatusNoGracePeriod_Test)
//    {
//    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa", "orgId");
//    auto client = CreateTestClient(userInfo, GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetLicensingDbMockPtr());
//
//    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
//    Utf8String userIdBad = "00000000-0000-0000-0000-000000000000";
//
//    std::list<Json::Value> emptyPolicyList;
//
//    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false);
//    std::list<Json::Value> validPolicyList;
//    validPolicyList.push_back(jsonPolicyValid);
//
//    auto jsonPolicyValidTrial = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9901, "", 1, true);
//    std::list<Json::Value> validTrialPolicyList;
//    validTrialPolicyList.push_back(jsonPolicyValidTrial);
//
//    auto jsonPolicyValidTrialExpired = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(-1), userId, 9902, "", 1, true);
//    std::list<Json::Value> validTrialExpiredPolicyList;
//    validTrialExpiredPolicyList.push_back(jsonPolicyValidTrialExpired);
//
//    auto jsonPolicyExpired = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(-1), DateHelper::AddDaysToCurrentTime(7), userId, 9903, "", 1, false);
//    std::list<Json::Value> expiredPolicyList;
//    expiredPolicyList.push_back(jsonPolicyExpired);
//
//    auto jsonPolicyNoSecurables = DummyPolicyHelper::CreatePolicyNoSecurables(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9904, "", 1, false);
//    std::list<Json::Value> noSecurablesPolicyList;
//    noSecurablesPolicyList.push_back(jsonPolicyNoSecurables);
//
//    auto jsonPolicyNoACLs = DummyPolicyHelper::CreatePolicyNoACLs(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9905, "", 1, false);
//    std::list<Json::Value> noACLsPolicyList;
//    noACLsPolicyList.push_back(jsonPolicyNoACLs);
//
//    auto jsonPolicyNoUserData = DummyPolicyHelper::CreatePolicyNoUserData(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9906, "", 1, false);
//    std::list<Json::Value> noUserDataPolicyList;
//    noUserDataPolicyList.push_back(jsonPolicyNoUserData);
//
//    auto jsonPolicyNoRequestData = DummyPolicyHelper::CreatePolicyNoRequestData(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9907, "", 1, false);
//    std::list<Json::Value> noRequestDataPolicyList;
//    noRequestDataPolicyList.push_back(jsonPolicyNoRequestData);
//
//    auto jsonPolicyIdBad = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userIdBad, 9908, "", 1, false);
//    std::list<Json::Value> idBadPolicyList;
//    idBadPolicyList.push_back(jsonPolicyIdBad);
//
//    auto jsonPolicyOfflineNotAllowed = DummyPolicyHelper::CreatePolicyOfflineNotAllowed(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9909, "", 1, false);
//    std::list<Json::Value> offlineNotAllowedPolicyList;
//    offlineNotAllowedPolicyList.push_back(jsonPolicyOfflineNotAllowed);
//
//    EXPECT_CALL(GetLicensingDbMock(), GetPolicyFilesByUser(A<Utf8StringCR>()))
//        .Times(12)
//        .WillOnce(Return(emptyPolicyList))
//        .WillOnce(Return(validPolicyList))
//        .WillOnce(Return(validTrialPolicyList))
//        .WillOnce(Return(validTrialExpiredPolicyList))
//        .WillOnce(Return(expiredPolicyList))
//        .WillOnce(Return(noSecurablesPolicyList))
//        .WillOnce(Return(noACLsPolicyList))
//        .WillOnce(Return(noUserDataPolicyList))
//        .WillOnce(Return(noRequestDataPolicyList))
//        .WillOnce(Return(idBadPolicyList))
//        .WillOnce(Return(emptyPolicyList))
//        .WillOnce(Return(offlineNotAllowedPolicyList));
//
//    EXPECT_CALL(GetLicensingDbMock(), GetOfflineGracePeriodStart())
//        .WillRepeatedly(Return(""));
//
//    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
//    ASSERT_EQ((int)client->GetProductStatus(), (int)LicenseStatus::NotEntitled);
//    ASSERT_EQ((int)client->GetProductStatus(9900), (int)LicenseStatus::Ok);
//    ASSERT_EQ((int)client->GetProductStatus(9901), (int)LicenseStatus::Trial);
//    ASSERT_EQ((int)client->GetProductStatus(9902), (int)LicenseStatus::Expired);
//    ASSERT_EQ((int)client->GetProductStatus(9903), (int)LicenseStatus::NotEntitled); // Policy is not valid due to expiration, therefore no entitlement
//    ASSERT_EQ((int)client->GetProductStatus(9904), (int)LicenseStatus::NotEntitled);
//    ASSERT_EQ((int)client->GetProductStatus(9905), (int)LicenseStatus::NotEntitled);
//    ASSERT_EQ((int)client->GetProductStatus(9906), (int)LicenseStatus::NotEntitled);
//    ASSERT_EQ((int)client->GetProductStatus(9907), (int)LicenseStatus::NotEntitled);
//    ASSERT_EQ((int)client->GetProductStatus(9908), (int)LicenseStatus::NotEntitled);
//    ASSERT_EQ((int)client->GetProductStatus(9999), (int)LicenseStatus::NotEntitled); // Policy with productId does not exist
//    ASSERT_EQ((int)client->GetProductStatus(9909), (int)LicenseStatus::Ok); // Grace Period NOT started; should return Ok
//    }

// TODO two different tests
//TEST_F(ClientTests, GetProductStatusStartedGracePeriod_Test)
//    {
//    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa", "orgId");
//    auto client = CreateTestClient(userInfo, GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetLicensingDbMockPtr());
//
//    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
//
//    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false);
//    std::list<Json::Value> validPolicyList;
//    validPolicyList.push_back(jsonPolicyValid);
//
//    auto jsonPolicyOfflineNotAllowed = DummyPolicyHelper::CreatePolicyOfflineNotAllowed(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9909, "", 1, false);
//    std::list<Json::Value> offlineNotAllowedPolicyList;
//    offlineNotAllowedPolicyList.push_back(jsonPolicyOfflineNotAllowed);
//
//    auto timestamp = DateHelper::GetCurrentTime();
//
//    EXPECT_CALL(GetLicensingDbMock(), GetPolicyFilesByUser(A<Utf8StringCR>()))
//        .Times(2)
//        .WillOnce(Return(validPolicyList))
//        .WillOnce(Return(offlineNotAllowedPolicyList));
//
//    EXPECT_CALL(GetLicensingDbMock(), GetOfflineGracePeriodStart())
//        .WillRepeatedly(Return(timestamp));
//
//    ASSERT_EQ((int)client->GetProductStatus(9900), (int)LicenseStatus::Offline); // Valid status should be Offline now
//    ASSERT_EQ((int)client->GetProductStatus(9909), (int)LicenseStatus::DisabledByPolicy); // Grace Period started; should be disabled
//    }

TEST_F(ClientTests, GetProductStatusExpiredGracePeriod_Test)
    {
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa", "orgId");
    auto client = CreateTestClient(userInfo, GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetLicensingDbMockPtr());

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";

    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false);
    std::list<Json::Value> validPolicyList;
    validPolicyList.push_back(jsonPolicyValid);

    auto timestampPast = DateHelper::AddDaysToCurrentTime(-14); // Two weeks ago; default offline period allowed is only 1 week

    GetLicensingDbMock().MockUserPolicyFiles(userId, validPolicyList);

    EXPECT_CALL(GetLicensingDbMock(), GetOfflineGracePeriodStart())
        .WillRepeatedly(Return(timestampPast));

    ASSERT_EQ((int)client->GetProductStatus(9900), (int)LicenseStatus::Expired); // Valid status should be Expired now, since offline grace period has expired
    ASSERT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, CleanUpPolicies_Success)
    {
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", "userId", "orgId");
    auto client = CreateTestClient(userInfo, GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetLicensingDbMockPtr());

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";

    auto jsonPolicyExpired = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(-1), DateHelper::AddDaysToCurrentTime(7), userId, 9903, "", 1, false);
    std::list<Json::Value> expiredPolicyList;
    expiredPolicyList.push_back(jsonPolicyExpired);

    EXPECT_CALL(GetLicensingDbMock(), GetPolicyFiles())
        .WillRepeatedly(Return(expiredPolicyList));

    EXPECT_CALL(GetLicensingDbMock(), DeletePolicyFile(A<Utf8StringCR>()))
        .Times(AtLeast(1));

    client->CleanUpPolicies();
    }

TEST_F(ClientTests, DeleteAllOtherPoliciesByUser_Success)
    {
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", "userId", "orgId");
    auto client = CreateTestClient(userInfo, GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetLicensingDbMockPtr());

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";

    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false);
    auto validPolicy = Policy::Create(jsonPolicyValid);

    EXPECT_CALL(GetLicensingDbMock(), DeleteAllOtherPolicyFilesByUser(A<Utf8StringCR>(), A<Utf8StringCR>()))
        .Times(AtLeast(1));

    client->DeleteAllOtherPoliciesByUser(validPolicy);
    }

// SaasClient tests
TEST_F(ClientTests, SaasClientTrackUsage_Success)
    {
    auto client = CreateTestSaasClient(GetUlasProviderMockPtr());

    Utf8String accessToken = "TestToken";
    const auto version = BeVersion(1, 0);
    FeatureEvent featureEvent = FeatureEvent("TestFeatureId", version);

    BentleyStatus status = BentleyStatus::SUCCESS;

    GetUlasProviderMock().MockRealtimeTrackUsage(BentleyStatus::SUCCESS);

    Utf8String projectId = "00000000-0000-0000-0000-000000000000";
    EXPECT_SUCCESS(client->TrackUsage(accessToken, version, projectId).get());
    EXPECT_EQ(1, GetUlasProviderMock().RealtimeTrackUsageCalls());
    }

TEST_F(ClientTests, SaasClientMarkFeatureNoDataNoProject_Success)
    {
    auto client = CreateTestSaasClient(GetUlasProviderMockPtr());

    Utf8String accessToken = "TestToken";
    const auto version = BeVersion(1, 0);
    FeatureEvent featureEvent = FeatureEvent("TestFeatureId", version);

    BentleyStatus status = BentleyStatus::SUCCESS;

    GetUlasProviderMock().MockRealtimeMarkFeature(BentleyStatus::SUCCESS);

    EXPECT_SUCCESS(client->MarkFeature(accessToken, featureEvent).get());
    EXPECT_EQ(1, GetUlasProviderMock().RealtimeMarkFeatureCalls());
    }

// ClientWithKey tests - move to new test file?
TEST_F(ClientTests, WithKeyStartApplication_Error)
    {
    auto client = CreateWithKeyTestClient(GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetLicensingDbMockPtr());

    GetLicensingDbMock().MockOpenOrCreate(ERROR);

    EXPECT_EQ((int)client->StartApplication(), (int)LicenseStatus::Error);
    EXPECT_EQ(1, GetLicensingDbMock().OpenOrCreateCount());
    }

TEST_F(ClientTests, WithKeyStartApplicationNullPolicy_Error)
    {
    auto client = CreateWithKeyTestClient(GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetLicensingDbMockPtr());

    GetLicensingDbMock().MockOpenOrCreate(SUCCESS);

    const std::shared_ptr<Policy> nullPolicy = nullptr;

    GetPolicyProviderMock().MockGetPolicyWithKey(nullPolicy);

    EXPECT_EQ((int)client->StartApplication(), static_cast<int>(LicenseStatus::Error));
    EXPECT_EQ(1, GetPolicyProviderMock().GetPolicyWithKeyCalls());
    EXPECT_EQ(1, GetLicensingDbMock().OpenOrCreateCount());
    }

TEST_F(ClientTests, WithKeyStartApplicationInvalidKey_Success)
    {
    auto client = CreateWithKeyTestClient(GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetLicensingDbMockPtr());

    GetLicensingDbMock().MockOpenOrCreate(SUCCESS);

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false);
    const auto validPolicy = Policy::Create(jsonPolicyValid);

    std::list<Json::Value> validPolicyList;
    validPolicyList.push_back(jsonPolicyValid);

    GetPolicyProviderMock().MockGetPolicyWithKey(validPolicy);

    EXPECT_CALL(GetLicensingDbMock(), AddOrUpdatePolicyFile(A<Utf8StringCR>(), A<Utf8StringCR>(), A<Utf8StringCR>(), A<Utf8StringCR>(), A<Utf8StringCR>(), A<Json::Value>()))
        .Times(AtLeast(1));

    EXPECT_CALL(GetLicensingDbMock(), DeleteAllOtherPolicyFilesByKey(A<Utf8StringCR>(), A<Utf8StringCR>()))
        .Times(AtLeast(1));

    Json::Value jsonAccessKeyResponse(Json::objectValue);
    jsonAccessKeyResponse["status"] = "Failure";
    jsonAccessKeyResponse["msg"] = "Record is Inactive or Expired";

    GetUlasProviderMock().MockGetAccessKeyInfo(jsonAccessKeyResponse);

    EXPECT_EQ((int)client->StartApplication(), (int)LicenseStatus::NotEntitled);
    EXPECT_EQ(1, GetLicensingDbMock().OpenOrCreateCount());
    EXPECT_EQ(1, GetPolicyProviderMock().GetPolicyWithKeyCalls());
    EXPECT_EQ(1, GetUlasProviderMock().GetAccessKeyInfoCalls());
    }

TEST_F(ClientTests, WithKeyStartApplication_StopApplication_Success)
    {
    auto client = CreateWithKeyTestClient(GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetLicensingDbMockPtr());

    GetLicensingDbMock().MockOpenOrCreate(SUCCESS);

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFullWithKey(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false, TEST_ACCESSKEY);
    auto validPolicy = Policy::Create(jsonPolicyValid);

    Utf8StringCR testAccessKey = TEST_ACCESSKEY;

    GetPolicyProviderMock().MockGetPolicyWithKey(validPolicy);

    EXPECT_CALL(GetLicensingDbMock(), AddOrUpdatePolicyFile(A<Utf8StringCR>(), A<Utf8StringCR>(), A<Utf8StringCR>(), A<Utf8StringCR>(), A<Utf8StringCR>(), A<Json::Value>()))
        .Times(AtLeast(1));

    EXPECT_CALL(GetLicensingDbMock(), DeleteAllOtherPolicyFilesByKey(A<Utf8StringCR>(), A<Utf8StringCR>()))
        .Times(AtLeast(1));

    Json::Value jsonAccessKeyResponse(Json::objectValue);
    jsonAccessKeyResponse["status"] = "Success";

    GetUlasProviderMock().MockGetAccessKeyInfo(jsonAccessKeyResponse);

    std::list<Json::Value> validPolicyList;
    validPolicyList.push_back(jsonPolicyValid);

    EXPECT_CALL(GetLicensingDbMock(), GetPolicyFilesByKey(testAccessKey))
        .Times(1)
        .WillOnce(Return(validPolicyList));

    EXPECT_CALL(GetLicensingDbMock(), GetOfflineGracePeriodStart())
        .Times(1)
        .WillOnce(Return(""));

    EXPECT_NE((int)client->StartApplication(), (int)LicenseStatus::Error);
    EXPECT_EQ(1, GetLicensingDbMock().OpenOrCreateCount());
    EXPECT_EQ(1, GetPolicyProviderMock().GetPolicyWithKeyCalls());
	EXPECT_EQ(1, GetUlasProviderMock().GetAccessKeyInfoCalls());
    EXPECT_SUCCESS(client->StopApplication());
    EXPECT_EQ(1, GetLicensingDbMock().CloseCount());
    }
