/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/ClientTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestsHelper.h"
#include "ClientTests.h"
#include "Utils/MockHttpHandler.h"
#include "DummyPolicyHelper.h"

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
#define TEST_PRODUCT_ID_INT 2545

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
    path.AppendToPath(L"License.db");

    return path;
    }

ClientImplPtr CreateTestClient(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId, IBuddiProviderPtr buddiProvider, IPolicyProviderPtr policyProvider, IUlasProviderPtr ulasProvider, IUsageDbPtr usageDb)
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
    BeFileName dbPath = GetUsageDbPath();

    return std::make_shared<ClientImpl>
        (
        manager->GetUserInfo(),
        clientInfo,
        manager,
        dbPath,
        true,
        buddiProvider,
        policyProvider,
        ulasProvider,
        "",
        "",
        proxy,
        usageDb
        );
    }

SaasClientImplPtr CreateTestSaasClient(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, int productId, IBuddiProviderPtr buddiProvider)
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, localState);
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    return std::make_shared<SaasClientImpl>(
        productId,
        "",
        proxy,
        buddiProvider);
    }

ClientWithKeyImplPtr CreateWithKeyTestClient(bool signIn, uint64_t heartbeatInterval, ITimeRetrieverPtr timeRetriever, IDelayedExecutorPtr delayedExecutor, UrlProvider::Environment env, Utf8StringCR productId, IBuddiProviderPtr buddiProvider, IPolicyProviderPtr policyProvider, IUlasProviderPtr ulasProvider, IUsageDbPtr usageDb)
    {
    InMemoryJsonLocalState* localState = new InMemoryJsonLocalState();
    UrlProvider::Initialize(env, UrlProvider::DefaultTimeout, localState);

    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();


    auto manager = ConnectSignInManager::Create(clientInfo, proxy, localState);

    BeFileName dbPath = GetUsageDbPath();

    Utf8String accesskey = "somekey";

    return std::make_shared<ClientWithKeyImpl>(
        accesskey,
        clientInfo,
        dbPath,
        true,
        buddiProvider,
        policyProvider,
        ulasProvider,
        "",
        "",
        proxy,
        usageDb);
    }

// Note: cannot use BuddiProvider mocks with clients created with the factory
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
    BeFileName dbPath = GetUsageDbPath();

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

    BeFileName dbPath = GetUsageDbPath();

    Utf8String accesskey = "somekey";

    return Client::CreateWithKey(
        accesskey,
        clientInfo,
        dbPath,
        true,
        "",
        "",
        proxy);
    }

ClientImplPtr CreateTestClient(bool signIn, IBuddiProviderPtr buddiProvider, IPolicyProviderPtr policyProvider, IUlasProviderPtr ulasProvider, IUsageDbPtr usageDb)
    {
    return CreateTestClient(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID, buddiProvider, policyProvider, ulasProvider, usageDb);
    }

ClientPtr CreateTestClientFromFactory(bool signIn)
    {
    return CreateTestClientFromFactory(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID);
    }

SaasClientImplPtr CreateTestSaasClient(bool signIn, IBuddiProviderPtr buddiProvider)
    {
    return CreateTestSaasClient(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID_INT, buddiProvider);
    }

SaasClientPtr CreateTestSaasClientFromFactory(bool signIn)
    {
    return CreateTestSaasClientFromFactory(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID_INT);
    }

ClientWithKeyImplPtr CreateWithKeyTestClient(bool signIn, IBuddiProviderPtr buddiProvider, IPolicyProviderPtr policyProvider, IUlasProviderPtr ulasProvider, IUsageDbPtr usageDb)
    {
    return CreateWithKeyTestClient(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID, buddiProvider, policyProvider, ulasProvider, usageDb);
    }

ClientPtr CreateWithKeyTestClientFromFactory(bool signIn)
    {
    return CreateWithKeyTestClientFromFactory(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClientTests::ClientTests() :
    m_handler(std::make_shared<MockHttpHandler>()),
    m_buddiProviderMock(std::make_shared<BuddiProviderMock>()),
    m_policyProviderMock(std::make_shared<PolicyProviderMock>()),
    m_ulasProviderMock(std::make_shared<UlasProviderMock>()),
    m_usageDbMock(std::make_shared<UsageDbMock>())
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
BuddiProviderMock&  ClientTests::GetBuddiProviderMock() const
    {
    return *m_buddiProviderMock;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<BuddiProviderMock> ClientTests::GetBuddiProviderMockPtr() const
    {
    return m_buddiProviderMock;
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
UsageDbMock&  ClientTests::GetUsageDbMock() const
    {
    return *m_usageDbMock;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<UsageDbMock> ClientTests::GetUsageDbMockPtr() const
    {
    return m_usageDbMock;
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
    path.AppendToPath(L"sqlang/DgnClientFx_en.sqlang.db3");

    ASSERT_EQ(SUCCESS, L10N::Initialize(BeSQLite::L10N::SqlangFiles(path)));
    }

TEST_F(ClientTests, StartApplication_Error)
    {
    auto client = CreateTestClient(true, GetBuddiProviderMockPtr(), GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetUsageDbMockPtr());

    EXPECT_CALL(GetUsageDbMock(), OpenOrCreate(A<BeFileNameCR>()))
        .Times(1)
        .WillOnce(Return(BentleyStatus::ERROR));

    EXPECT_EQ((int)client->StartApplication(), (int)LicenseStatus::Error);
    }

TEST_F(ClientTests, StartApplicationNoHeartbeat_Success)
    {
    auto client = CreateTestClient(true, GetBuddiProviderMockPtr(), GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetUsageDbMockPtr());

    EXPECT_CALL(GetUsageDbMock(), OpenOrCreate(A<BeFileNameCR>()))
        .Times(1)
        .WillOnce(Return(BentleyStatus::SUCCESS));

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false);
    auto validPolicy = Policy::Create(jsonPolicyValid);

    std::list<Json::Value> validPolicyList;
    validPolicyList.push_back(jsonPolicyValid);

    EXPECT_CALL(GetPolicyProviderMock(), GetPolicy())
        .Times(1)
        .WillOnce(Return(ByMove(folly::makeFuture(validPolicy)))); // need ByMove since this calls the copy constructor for folly::Future, which is deleted

    EXPECT_CALL(GetUsageDbMock(), AddOrUpdatePolicyFile(A<Utf8StringCR>(), A<Utf8StringCR>(), A<Utf8StringCR>(), A<Utf8StringCR>(), A<Utf8StringCR>(), A<Json::Value>()))
        .Times(AtLeast(1));

    EXPECT_CALL(GetUsageDbMock(), DeleteAllOtherPolicyFilesByUser(A<Utf8StringCR>(), A<Utf8StringCR>()))
        .Times(AtLeast(1));

    EXPECT_CALL(GetUsageDbMock(), GetPolicyFilesByUser(A<Utf8StringCR>()))
        .Times(1)
        .WillOnce(Return(validPolicyList));

    // TO MAKE THIS RETURN OK -> use 2545 as productId in the policy!!
    EXPECT_NE((int)client->StartApplication(), (int)LicenseStatus::Error); // not entitiled so skips the heartbeat calls
    }

TEST_F(ClientTests, StartApplicationStopApplication_Success)
    {
    auto client = CreateTestClient(true, GetBuddiProviderMockPtr(), GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetUsageDbMockPtr());

    EXPECT_CALL(GetUsageDbMock(), OpenOrCreate(A<BeFileNameCR>()))
        .Times(1)
        .WillOnce(Return(BentleyStatus::SUCCESS));

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 2545, "", 1, false);
    auto validPolicy = Policy::Create(jsonPolicyValid);

    std::list<Json::Value> validPolicyList;
    validPolicyList.push_back(jsonPolicyValid);

    EXPECT_CALL(GetPolicyProviderMock(), GetPolicy())
        .WillRepeatedly(Return(ByMove(folly::makeFuture(validPolicy)))); // need ByMove since this calls the copy constructor for folly::Future, which is deleted

    EXPECT_CALL(GetUsageDbMock(), AddOrUpdatePolicyFile(A<Utf8StringCR>(), A<Utf8StringCR>(), A<Utf8StringCR>(), A<Utf8StringCR>(), A<Utf8StringCR>(), A<Json::Value>()))
        .Times(AtLeast(1));

    EXPECT_CALL(GetUsageDbMock(), DeleteAllOtherPolicyFilesByUser(A<Utf8StringCR>(), A<Utf8StringCR>()))
        .Times(AtLeast(1));

    EXPECT_CALL(GetUsageDbMock(), GetPolicyFilesByUser(A<Utf8StringCR>()))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(validPolicyList));

    // called in Usage heartbeat
    EXPECT_CALL(GetUsageDbMock(), IsDbOpen())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(GetUsageDbMock(), RecordUsageMock())
        .WillRepeatedly(Return(BentleyStatus::SUCCESS));

    // LogPosting heartbeat
    EXPECT_CALL(GetUsageDbMock(), GetUsageRecordCount())
        .WillRepeatedly(Return(1)); // will get PostUsageLogs() called
    EXPECT_CALL(GetUsageDbMock(), GetFeatureRecordCount())
        .WillRepeatedly(Return(1)); // will get PostFeatureLogs() called
    EXPECT_CALL(GetUlasProviderMock(), PostUsageLogs(_, _));
    EXPECT_CALL(GetUlasProviderMock(), PostFeatureLogs(_, _));

    // called in Policy heartbeat
    EXPECT_CALL(GetUsageDbMock(), GetOfflineGracePeriodStart())
        .WillRepeatedly(Return(""));

    EXPECT_CALL(GetUsageDbMock(), Close());

    EXPECT_NE((int)client->StartApplication(), (int)LicenseStatus::Error);
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientTests, DISABLED_TrackUsage_FreeApplication_Success)
    {
    EXPECT_CALL(GetBuddiProviderMock(), UlasRealtimeLoggingBaseUrl()) // called on TrackUsage()
        .Times(1)
        .WillRepeatedly(Return("https://qa-connect-ulastm.bentley.com/Bentley.ULAS.PostingService/PostingSvcWebApi/"));
    auto client = CreateTestSaasClient(true, GetBuddiProviderMockPtr());
    Utf8String tokenstring = "b683fe041bfd1ef554599e69253271f5f6775eb7106514fa56e512040d635d4a";
    auto version = BeVersion(1, 0);
    Utf8String projectId = "00000000-0000-0000-0000-000000000000";
    EXPECT_SUCCESS(client->TrackUsage(tokenstring,version,projectId).get());
    }

TEST_F(ClientTests, DISABLED_StartWithKeyApplication_StopApplication_Success)
    {
    EXPECT_CALL(GetBuddiProviderMock(), UlasRealtimeLoggingBaseUrl()) // called on SendUsageRealtimeWithKey() which is not currently called by StartApplication()
        //.Times(AtLeast(1))
        .WillRepeatedly(Return("https://qa-connect-ulastm.bentley.com/Bentley.ULAS.PostingService/PostingSvcWebApi/"));

    auto client = CreateWithKeyTestClient(true, GetBuddiProviderMockPtr(), GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetUsageDbMockPtr());
    EXPECT_NE((int)client->StartApplication(), (int)LicenseStatus::Error);
    EXPECT_SUCCESS(client->StopApplication());
    }

TEST_F(ClientTests, GetPolicy_Success)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false);
    auto policy = Policy::Create(jsonPolicyValid);

    EXPECT_CALL(GetPolicyProviderMock(), GetPolicy())
        .Times(1)
        .WillOnce(Return(ByMove(folly::makeFuture(policy)))); // need ByMove since this calls the copy constructor for Future, which is deleted

    auto client = CreateTestClient(true, GetBuddiProviderMockPtr(), GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetUsageDbMockPtr());

    auto policyToken = client->GetPolicy().get();
    EXPECT_NE(policyToken, nullptr); //not testing the policy token here, just that GetPolicy is called
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

TEST_F(ClientTests, GetProductStatusNoGracePeriod_Test)
    {
    auto client = CreateTestClient(true, GetBuddiProviderMockPtr(), GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetUsageDbMockPtr());

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    Utf8String userIdBad = "00000000-0000-0000-0000-000000000000";

    std::list<Json::Value> emptyPolicyList;

    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false);
    std::list<Json::Value> validPolicyList;
    validPolicyList.push_back(jsonPolicyValid);

    auto jsonPolicyValidTrial = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9901, "", 1, true);
    std::list<Json::Value> validTrialPolicyList;
    validTrialPolicyList.push_back(jsonPolicyValidTrial);

    auto jsonPolicyValidTrialExpired = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(-1), userId, 9902, "", 1, true);
    std::list<Json::Value> validTrialExpiredPolicyList;
    validTrialExpiredPolicyList.push_back(jsonPolicyValidTrialExpired);

    auto jsonPolicyExpired = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(-1), DateHelper::AddDaysToCurrentTime(7), userId, 9903, "", 1, false);
    std::list<Json::Value> expiredPolicyList;
    expiredPolicyList.push_back(jsonPolicyExpired);

    auto jsonPolicyNoSecurables = DummyPolicyHelper::CreatePolicyNoSecurables(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9904, "", 1, false);
    std::list<Json::Value> noSecurablesPolicyList;
    noSecurablesPolicyList.push_back(jsonPolicyNoSecurables);

    auto jsonPolicyNoACLs = DummyPolicyHelper::CreatePolicyNoACLs(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9905, "", 1, false);
    std::list<Json::Value> noACLsPolicyList;
    noACLsPolicyList.push_back(jsonPolicyNoACLs);

    auto jsonPolicyNoUserData = DummyPolicyHelper::CreatePolicyNoUserData(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9906, "", 1, false);
    std::list<Json::Value> noUserDataPolicyList;
    noUserDataPolicyList.push_back(jsonPolicyNoUserData);

    auto jsonPolicyNoRequestData = DummyPolicyHelper::CreatePolicyNoRequestData(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9907, "", 1, false);
    std::list<Json::Value> noRequestDataPolicyList;
    noRequestDataPolicyList.push_back(jsonPolicyNoRequestData);

    auto jsonPolicyIdBad = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userIdBad, 9908, "", 1, false);
    std::list<Json::Value> idBadPolicyList;
    idBadPolicyList.push_back(jsonPolicyIdBad);

    auto jsonPolicyOfflineNotAllowed = DummyPolicyHelper::CreatePolicyOfflineNotAllowed(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9909, "", 1, false);
    std::list<Json::Value> offlineNotAllowedPolicyList;
    offlineNotAllowedPolicyList.push_back(jsonPolicyOfflineNotAllowed);

    EXPECT_CALL(GetUsageDbMock(), GetPolicyFilesByUser(A<Utf8StringCR>()))
        .Times(12)
        .WillOnce(Return(emptyPolicyList))
        .WillOnce(Return(validPolicyList))
        .WillOnce(Return(validTrialPolicyList))
        .WillOnce(Return(validTrialExpiredPolicyList))
        .WillOnce(Return(expiredPolicyList))
        .WillOnce(Return(noSecurablesPolicyList))
        .WillOnce(Return(noACLsPolicyList))
        .WillOnce(Return(noUserDataPolicyList))
        .WillOnce(Return(noRequestDataPolicyList))
        .WillOnce(Return(idBadPolicyList))
        .WillOnce(Return(emptyPolicyList))
        .WillOnce(Return(offlineNotAllowedPolicyList));

    EXPECT_CALL(GetUsageDbMock(), GetOfflineGracePeriodStart())
        .WillRepeatedly(Return(""));

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    ASSERT_EQ((int)client->GetProductStatus(), (int)LicenseStatus::NotEntitled);
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
    }

TEST_F(ClientTests, GetProductStatusStartedGracePeriod_Test)
    {
    auto client = CreateTestClient(true, GetBuddiProviderMockPtr(), GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetUsageDbMockPtr());

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";

    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false);
    std::list<Json::Value> validPolicyList;
    validPolicyList.push_back(jsonPolicyValid);

    auto jsonPolicyOfflineNotAllowed = DummyPolicyHelper::CreatePolicyOfflineNotAllowed(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9909, "", 1, false);
    std::list<Json::Value> offlineNotAllowedPolicyList;
    offlineNotAllowedPolicyList.push_back(jsonPolicyOfflineNotAllowed);

    auto timestamp = DateHelper::GetCurrentTime();

    EXPECT_CALL(GetUsageDbMock(), GetPolicyFilesByUser(A<Utf8StringCR>()))
        .Times(2)
        .WillOnce(Return(validPolicyList))
        .WillOnce(Return(offlineNotAllowedPolicyList));

    EXPECT_CALL(GetUsageDbMock(), GetOfflineGracePeriodStart())
        .WillRepeatedly(Return(timestamp));

    ASSERT_EQ((int)client->GetProductStatus(9900), (int)LicenseStatus::Offline); // Valid status should be Offline now
    ASSERT_EQ((int)client->GetProductStatus(9909), (int)LicenseStatus::DisabledByPolicy); // Grace Period started; should be disabled
    }

TEST_F(ClientTests, GetProductStatusExpiredGracePeriod_Test)
    {
    auto client = CreateTestClient(true, GetBuddiProviderMockPtr(), GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetUsageDbMockPtr());

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";

    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false);
    std::list<Json::Value> validPolicyList;
    validPolicyList.push_back(jsonPolicyValid);

    auto timestampPast = DateHelper::AddDaysToCurrentTime(-14); // Two weeks ago; default offline period allowed is only 1 week

    EXPECT_CALL(GetUsageDbMock(), GetPolicyFilesByUser(A<Utf8StringCR>()))
        .Times(1)
        .WillOnce(Return(validPolicyList));

    EXPECT_CALL(GetUsageDbMock(), GetOfflineGracePeriodStart())
        .WillRepeatedly(Return(timestampPast));

    ASSERT_EQ((int)client->GetProductStatus(9900), (int)LicenseStatus::Expired); // Valid status should be Expired now, since offline grace period has expired
    }

TEST_F(ClientTests, CleanUpPolicies_Success)
    {
    auto client = CreateTestClient(true, GetBuddiProviderMockPtr(), GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetUsageDbMockPtr());

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";

    auto jsonPolicyExpired = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(-1), DateHelper::AddDaysToCurrentTime(7), userId, 9903, "", 1, false);
    std::list<Json::Value> expiredPolicyList;
    expiredPolicyList.push_back(jsonPolicyExpired);

    EXPECT_CALL(GetUsageDbMock(), GetPolicyFiles())
        .WillRepeatedly(Return(expiredPolicyList));

    EXPECT_CALL(GetUsageDbMock(), DeletePolicyFile(A<Utf8StringCR>()))
        .Times(AtLeast(1));

    client->CleanUpPolicies();
    }

TEST_F(ClientTests, DeleteAllOtherPoliciesByUser_Success)
    {
    auto client = CreateTestClient(true, GetBuddiProviderMockPtr(), GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetUsageDbMockPtr());

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";

    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false);
    auto validPolicy = Policy::Create(jsonPolicyValid);

    EXPECT_CALL(GetUsageDbMock(), DeleteAllOtherPolicyFilesByUser(A<Utf8StringCR>(), A<Utf8StringCR>()))
        .Times(AtLeast(1));

    client->DeleteAllOtherPoliciesByUser(validPolicy);
    }
