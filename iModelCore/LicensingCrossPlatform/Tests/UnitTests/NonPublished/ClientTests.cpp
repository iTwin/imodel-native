/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "TestsHelper.h"
#include "JsonHelper.h"
#include "ClientTests.h"
#include "Utils/MockHttpHandler.h"
#include "DummyPolicyHelper.h"
#include "DummyUserInfoHelper.h"

#include <Licensing/Client.h>
#include <Licensing/SaasClient.h>
#include <Licensing/Utils/DateHelper.h>
#include "../../../Licensing/LicensingDb.h"
#include "../../../PublicAPI/Licensing/Utils/SCVWritter.h"

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

    EXPECT_EQ(SUCCESS, L10N::Initialize(BeSQLite::L10N::SqlangFiles(path)));
    }

ClientImplPtr ClientTests::CreateTestClient(ConnectSignInManager::UserInfo userInfo, Utf8StringCR productId, ApplicationInfoPtr appInfo) const
    {
    if (appInfo == nullptr)
        {
        appInfo = std::make_shared<ApplicationInfo>(BeVersion(1, 0), "TestDeviceId", productId);
        }

    BeFileName dbPath = GetLicensingDbPath();

    return std::make_shared<ClientImpl>(
        userInfo,
        appInfo,
        dbPath,
        true,
        GetPolicyProviderMockPtr(),
        GetUlasProviderMockPtr(),
        "",
        "",
        GetLicensingDbMockPtr()
        );
    }

ClientImplPtr ClientTests::CreateTestClientWithBadParams(ConnectSignInManager::UserInfo userInfo, Utf8StringCR productId, bool BadAppInfo, bool BadDbPath) const
    {
    ApplicationInfoPtr appInfo = nullptr;
    if (!BadAppInfo)
        {
        appInfo = std::make_shared<ApplicationInfo>(BeVersion(1, 0), "TestDeviceId", productId);
        }
    BeFileName dbPath;
    if (!BadDbPath)
        {
        dbPath = GetLicensingDbPath();
        }
    return std::make_shared<ClientImpl>(
        userInfo,
        appInfo,
        dbPath,
        true,
        GetPolicyProviderMockPtr(),
        GetUlasProviderMockPtr(),
        "",
        "",
        GetLicensingDbMockPtr()
        );
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

    std::list<std::shared_ptr<Policy>> noUserDataPolicyList;
    noUserDataPolicyList.push_back(Policy::Create(jsonPolicyNoUserData));

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

    std::list<std::shared_ptr<Policy>> validPolicyList;
    validPolicyList.push_back(Policy::Create(jsonPolicyValid));

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

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_SUCCESS(client->StopApplication());
    EXPECT_LE(1, GetUlasProviderMock().PostUsageLogsCalls());
    EXPECT_LE(1, GetUlasProviderMock().PostFeatureLogsCalls());
    EXPECT_LE(1, GetLicensingDbMock().RecordUsageCount()); // called in usage heartbeat
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

TEST_F(ClientTests, GetLicenseStatusEmpty_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo);

    std::list<std::shared_ptr<Policy>> emptyPolicyList;

    GetLicensingDbMock().MockUserPolicyFiles(userId, emptyPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::NotEntitled));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetLicenseStatusValid_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<std::shared_ptr<Policy>> validPolicyList;
    validPolicyList.push_back(Policy::Create(jsonPolicyValid));

    GetLicensingDbMock().MockUserPolicyFiles(userId, validPolicyList);
    GetLicensingDbMock().MockGetOfflineGracePeriodStart("");

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Ok));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    EXPECT_EQ(1, GetLicensingDbMock().GetOfflineGracePeriodStartCount());
    }

// AccessDenied "backup" ACL, valid Evaluation ACL
TEST_F(ClientTests, GetLicenseStatusEvalValid_Test)
    {
    Utf8String userId = "be7b9f4f-5b1e-4af8-bb05-6e060a6a48db";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(12, 0), "TestDeviceId", "1052");

    auto client = CreateTestClient(userInfo, "1052", appInfo);

    BeFileName testJson;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testJson);
    testJson.AppendToPath(L"TestAssets/EvalPolicy.json");

    Json::Value jsonPolicyEval = ReadJsonFile(testJson);
    Json::Value jsonPolicyEvalObject = Json::Reader::DoParse(jsonPolicyEval.asString()); // need to convert to Json::Value object type to use as a policy json
    std::list<std::shared_ptr<Policy>> validEvalPolicyList;
    validEvalPolicyList.push_back(Policy::Create(jsonPolicyEvalObject));

    GetLicensingDbMock().MockUserPolicyFiles(userId, validEvalPolicyList);
    GetLicensingDbMock().MockGetOfflineGracePeriodStart("");

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Ok));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    EXPECT_EQ(1, GetLicensingDbMock().GetOfflineGracePeriodStartCount());
    }

// AccessDenied "backup" ACL, expired Evaluation ACL
TEST_F(ClientTests, GetLicenseStatusEvalExpired_Test)
    {
    Utf8String userId = "be7b9f4f-5b1e-4af8-bb05-6e060a6a48db";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(12, 0), "TestDeviceId", "1052");

    auto client = CreateTestClient(userInfo, "1052", appInfo);

    BeFileName testJson;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testJson);
    testJson.AppendToPath(L"TestAssets/EvalPolicyExpired.json");

    Json::Value jsonPolicyEval = ReadJsonFile(testJson);
    Json::Value jsonPolicyEvalObject = Json::Reader::DoParse(jsonPolicyEval.asString()); // need to convert to Json::Value object type to use as a policy json
    std::list<std::shared_ptr<Policy>> expiredEvalPolicyList;
    expiredEvalPolicyList.push_back(Policy::Create(jsonPolicyEvalObject));

    GetLicensingDbMock().MockUserPolicyFiles(userId, expiredEvalPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::AccessDenied));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

// AccessDenied "backup" ACL, wrong version Evaluation ACL
TEST_F(ClientTests, GetLicenseStatusEvalBadVersion_Test)
    {
    Utf8String userId = "be7b9f4f-5b1e-4af8-bb05-6e060a6a48db";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(1, 0), "TestDeviceId", "1052");

    auto client = CreateTestClient(userInfo, "1052", appInfo);

    BeFileName testJson;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testJson);
    testJson.AppendToPath(L"TestAssets/EvalPolicy.json");

    Json::Value jsonPolicyEval = ReadJsonFile(testJson);
    Json::Value jsonPolicyEvalObject = Json::Reader::DoParse(jsonPolicyEval.asString()); // need to convert to Json::Value object type to use as a policy json
    std::list<std::shared_ptr<Policy>> expiredEvalPolicyList;
    expiredEvalPolicyList.push_back(Policy::Create(jsonPolicyEvalObject));

    GetLicensingDbMock().MockUserPolicyFiles(userId, expiredEvalPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::AccessDenied));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetLicenseStatusEvalGoodVersion_Test)
    {
    Utf8String userId = "be7b9f4f-5b1e-4af8-bb05-6e060a6a48db";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(12, 0), "TestDeviceId", "1052");

    auto client = CreateTestClient(userInfo, "1052", appInfo);

    BeFileName testJson;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testJson);
    testJson.AppendToPath(L"TestAssets/EvalPolicy.json");

    Json::Value jsonPolicyEval = ReadJsonFile(testJson);
    Json::Value jsonPolicyEvalObject = Json::Reader::DoParse(jsonPolicyEval.asString()); // need to convert to Json::Value object type to use as a policy json
    std::list<std::shared_ptr<Policy>> expiredEvalPolicyList;
    expiredEvalPolicyList.push_back(Policy::Create(jsonPolicyEvalObject));

    GetLicensingDbMock().MockUserPolicyFiles(userId, expiredEvalPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Ok));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetLicenseStatusEvalGoodFullVersion_Test)
    {
    Utf8String userId = "be7b9f4f-5b1e-4af8-bb05-6e060a6a48db";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(12, 3, 2, 1), "TestDeviceId", "1052");

    auto client = CreateTestClient(userInfo, "1052", appInfo);

    BeFileName testJson;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testJson);
    testJson.AppendToPath(L"TestAssets/FullVersionEvalPolicy.json");

    Json::Value jsonPolicyEval = ReadJsonFile(testJson);
    Json::Value jsonPolicyEvalObject = Json::Reader::DoParse(jsonPolicyEval.asString()); // need to convert to Json::Value object type to use as a policy json
    std::list<std::shared_ptr<Policy>> expiredEvalPolicyList;
    expiredEvalPolicyList.push_back(Policy::Create(jsonPolicyEvalObject));

    GetLicensingDbMock().MockUserPolicyFiles(userId, expiredEvalPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Ok));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetLicenseStatusEvalBadFullVersion_Test)
    {
    Utf8String userId = "be7b9f4f-5b1e-4af8-bb05-6e060a6a48db";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(12, 3, 6, 1), "TestDeviceId", "1052");

    auto client = CreateTestClient(userInfo, "1052", appInfo);

    BeFileName testJson;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testJson);
    testJson.AppendToPath(L"TestAssets/FullVersionEvalPolicy.json");

    Json::Value jsonPolicyEval = ReadJsonFile(testJson);
    Json::Value jsonPolicyEvalObject = Json::Reader::DoParse(jsonPolicyEval.asString()); // need to convert to Json::Value object type to use as a policy json
    std::list<std::shared_ptr<Policy>> expiredEvalPolicyList;
    expiredEvalPolicyList.push_back(Policy::Create(jsonPolicyEvalObject));

    GetLicensingDbMock().MockUserPolicyFiles(userId, expiredEvalPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::AccessDenied));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

// valid "backup" ACL, wrong version Evaluation ACL
TEST_F(ClientTests, GetLicenseStatusEvalBackupAcl_Test)
    {
    Utf8String userId = "be7b9f4f-5b1e-4af8-bb05-6e060a6a48db";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(1, 0), "TestDeviceId", "1052");

    auto client = CreateTestClient(userInfo, "1052", appInfo);

    BeFileName testJson;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testJson);
    testJson.AppendToPath(L"TestAssets/EvalPolicyWithBackup.json");

    Json::Value jsonPolicyEval = ReadJsonFile(testJson);
    Json::Value jsonPolicyEvalObject = Json::Reader::DoParse(jsonPolicyEval.asString()); // need to convert to Json::Value object type to use as a policy json
    std::list<std::shared_ptr<Policy>> expiredEvalPolicyList;
    expiredEvalPolicyList.push_back(Policy::Create(jsonPolicyEvalObject));

    GetLicensingDbMock().MockUserPolicyFiles(userId, expiredEvalPolicyList);
    GetLicensingDbMock().MockGetOfflineGracePeriodStart("");

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Ok));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    EXPECT_EQ(1, GetLicensingDbMock().GetOfflineGracePeriodStartCount());
    }

TEST_F(ClientTests, GetLicenseStatusValidTrial_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyValidTrial = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, true);
    std::list<std::shared_ptr<Policy>> validTrialPolicyList;
    validTrialPolicyList.push_back(Policy::Create(jsonPolicyValidTrial));

    GetLicensingDbMock().MockUserPolicyFiles(userId, validTrialPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Trial));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetLicenseStatusExpiredTrial_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyExpiredTrial = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(-1), userId, std::atoi(TEST_PRODUCT_ID), "", 1, true);
    std::list<std::shared_ptr<Policy>> expiredTrialPolicyList;
    expiredTrialPolicyList.push_back(Policy::Create(jsonPolicyExpiredTrial));

    GetLicensingDbMock().MockUserPolicyFiles(userId, expiredTrialPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Expired));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetLicenseStatusExpired_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyExpired = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(-1), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<std::shared_ptr<Policy>> expiredPolicyList;
    expiredPolicyList.push_back(Policy::Create(jsonPolicyExpired));

    GetLicensingDbMock().MockUserPolicyFiles(userId, expiredPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Expired));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetLicenseStatusNoSecurables_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyNoSecurables = DummyPolicyHelper::CreatePolicyNoSecurables(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<std::shared_ptr<Policy>> noSecurablesPolicyList;
    noSecurablesPolicyList.push_back(Policy::Create(jsonPolicyNoSecurables));

    GetLicensingDbMock().MockUserPolicyFiles(userId, noSecurablesPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::NotEntitled));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetLicenseStatusNoACLs_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyNoACLs = DummyPolicyHelper::CreatePolicyNoACLs(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<std::shared_ptr<Policy>> noACLsPolicyList;
    noACLsPolicyList.push_back(Policy::Create(jsonPolicyNoACLs));

    GetLicensingDbMock().MockUserPolicyFiles(userId, noACLsPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::NotEntitled));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetLicenseStatusNoUserData_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyNoUserData = DummyPolicyHelper::CreatePolicyNoUserData(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<std::shared_ptr<Policy>> noUserDataPolicyList;
    noUserDataPolicyList.push_back(Policy::Create(jsonPolicyNoUserData));

    GetLicensingDbMock().MockUserPolicyFiles(userId, noUserDataPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::NotEntitled));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetLicenseStatusNoRequestData_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyNoRequestData = DummyPolicyHelper::CreatePolicyNoRequestData(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<std::shared_ptr<Policy>> noRequestDataPolicyList;
    noRequestDataPolicyList.push_back(Policy::Create(jsonPolicyNoRequestData));

    GetLicensingDbMock().MockUserPolicyFiles(userId, noRequestDataPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::NotEntitled));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetLicenseStatusIdBad_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    Utf8String userIdBad = "00000000-0000-0000-0000-000000000000";

    auto jsonPolicyIdBad = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userIdBad, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<std::shared_ptr<Policy>> idBadPolicyList;
    idBadPolicyList.push_back(Policy::Create(jsonPolicyIdBad));

    GetLicensingDbMock().MockUserPolicyFiles(userId, idBadPolicyList);

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::NotEntitled));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, GetLicenseStatusOfflineNotAllowed_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyOfflineNotAllowed = DummyPolicyHelper::CreatePolicyOfflineNotAllowed(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<std::shared_ptr<Policy>> offlineNotAllowedPolicyList;
    offlineNotAllowedPolicyList.push_back(Policy::Create(jsonPolicyOfflineNotAllowed));

    GetLicensingDbMock().MockUserPolicyFiles(userId, offlineNotAllowedPolicyList);
    GetLicensingDbMock().MockGetOfflineGracePeriodStart("");

    // NOTE: statuses are cast to int so that if test fails, logs will show human-readable values (rather than byte representation of enumeration value)
    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Ok));
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    EXPECT_EQ(1, GetLicensingDbMock().GetOfflineGracePeriodStartCount());
    }

TEST_F(ClientTests, GetLicenseStatusGracePeriodStartedOfflineNotAllowed_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyOfflineNotAllowed = DummyPolicyHelper::CreatePolicyOfflineNotAllowed(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<std::shared_ptr<Policy>> offlineNotAllowedPolicyList;
    offlineNotAllowedPolicyList.push_back(Policy::Create(jsonPolicyOfflineNotAllowed));

    auto timestamp = DateHelper::GetCurrentTime();

    GetLicensingDbMock().MockUserPolicyFiles(userId, offlineNotAllowedPolicyList);
    GetLicensingDbMock().MockGetOfflineGracePeriodStart(timestamp);

    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::DisabledByPolicy)); // Grace Period started; should be disabled
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    EXPECT_EQ(1, GetLicensingDbMock().GetOfflineGracePeriodStartCount());
    }

TEST_F(ClientTests, GetLicenseStatusGracePeriodStarted_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<std::shared_ptr<Policy>> validPolicyList;
    validPolicyList.push_back(Policy::Create(jsonPolicyValid));

    auto timestamp = DateHelper::GetCurrentTime();

    GetLicensingDbMock().MockUserPolicyFiles(userId, validPolicyList);
    GetLicensingDbMock().MockGetOfflineGracePeriodStart(timestamp);

    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Offline)); // Valid status should be Offline now
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    EXPECT_EQ(2, GetLicensingDbMock().GetOfflineGracePeriodStartCount());
    }

TEST_F(ClientTests, GetLicenseStatusExpiredGracePeriod_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false);
    std::list<std::shared_ptr<Policy>> validPolicyList;
    validPolicyList.push_back(Policy::Create(jsonPolicyValid));

    auto timestampPast = DateHelper::AddDaysToCurrentTime(-14); // Two weeks ago; default offline period allowed is only 1 week

    GetLicensingDbMock().MockUserPolicyFiles(userId, validPolicyList);
    GetLicensingDbMock().MockGetOfflineGracePeriodStart(timestampPast);

    EXPECT_EQ(static_cast<int>(client->GetLicenseStatus()), static_cast<int>(LicenseStatus::Expired)); // Valid status should be Expired now, since offline grace period has expired
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    EXPECT_LE(1, GetLicensingDbMock().GetOfflineGracePeriodStartCount());
    }

// GetTrialDaysRemaining tests

// AccessDenied "backup" ACL, valid Evaluation ACL
TEST_F(ClientTests, GetTrialDaysRemainingEvalValid_Test)
    {
    Utf8String userId = "be7b9f4f-5b1e-4af8-bb05-6e060a6a48db";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(12, 0), "TestDeviceId", "1052");

    auto client = CreateTestClient(userInfo, "1052", appInfo);

    BeFileName testJson;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testJson);
    testJson.AppendToPath(L"TestAssets/EvalPolicy.json");

    Json::Value jsonPolicyEval = ReadJsonFile(testJson);
    Json::Value jsonPolicyEvalObject = Json::Reader::DoParse(jsonPolicyEval.asString()); // need to convert to Json::Value object type to use as a policy json
    std::list<std::shared_ptr<Policy>> validEvalPolicyList;
    validEvalPolicyList.push_back(Policy::Create(jsonPolicyEvalObject));

    GetLicensingDbMock().MockUserPolicyFiles(userId, validEvalPolicyList);
    // NOTE: the date for this comes from Assets/EvalPolicy.json
    auto daysLeft = DateHelper::diffdatedays("2022-04-26T00:00:00", DateHelper::GetCurrentTime());

    EXPECT_EQ(client->GetTrialDaysRemaining(), daysLeft);
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

// AccessDenied "backup" ACL, expired Evaluation ACL
TEST_F(ClientTests, GetTrialDaysRemainingEvalExpired_Test)
    {
    Utf8String userId = "be7b9f4f-5b1e-4af8-bb05-6e060a6a48db";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(12, 0), "TestDeviceId", "1052");

    auto client = CreateTestClient(userInfo, "1052", appInfo);

    BeFileName testJson;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testJson);
    testJson.AppendToPath(L"TestAssets/EvalPolicyExpired.json");

    Json::Value jsonPolicyEval = ReadJsonFile(testJson);
    Json::Value jsonPolicyEvalObject = Json::Reader::DoParse(jsonPolicyEval.asString()); // need to convert to Json::Value object type to use as a policy json
    std::list<std::shared_ptr<Policy>> expiredEvalPolicyList;
    expiredEvalPolicyList.push_back(Policy::Create(jsonPolicyEvalObject));

    GetLicensingDbMock().MockUserPolicyFiles(userId, expiredEvalPolicyList);

    EXPECT_EQ(static_cast<int>(client->GetTrialDaysRemaining()), 0);
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

// not trial or eval - valid "backup" ACL, wrong version Evaluation ACL
TEST_F(ClientTests, GetTrialDaysRemainingEvalBackupAcl_Test)
    {
    Utf8String userId = "be7b9f4f-5b1e-4af8-bb05-6e060a6a48db";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto appInfo = std::make_shared<ApplicationInfo>(BeVersion(1, 0), "TestDeviceId", "1052");

    auto client = CreateTestClient(userInfo, "1052", appInfo);

    BeFileName testJson;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testJson);
    testJson.AppendToPath(L"TestAssets/EvalPolicyWithBackup.json");

    Json::Value jsonPolicyEval = ReadJsonFile(testJson);
    Json::Value jsonPolicyEvalObject = Json::Reader::DoParse(jsonPolicyEval.asString()); // need to convert to Json::Value object type to use as a policy json
    std::list<std::shared_ptr<Policy>> expiredEvalPolicyList;
    expiredEvalPolicyList.push_back(Policy::Create(jsonPolicyEvalObject));

    GetLicensingDbMock().MockUserPolicyFiles(userId, expiredEvalPolicyList);

    EXPECT_EQ(static_cast<int>(client->GetTrialDaysRemaining()), -1);
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

// valid trial
TEST_F(ClientTests, GetTrialDaysRemainingValidTrial_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyValidTrial = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, true);
    std::list<std::shared_ptr<Policy>> validTrialPolicyList;
    validTrialPolicyList.push_back(Policy::Create(jsonPolicyValidTrial));

    GetLicensingDbMock().MockUserPolicyFiles(userId, validTrialPolicyList);

    EXPECT_EQ(static_cast<int>(client->GetTrialDaysRemaining()), 6);
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

// expired trial
TEST_F(ClientTests, GetTrialDaysRemainingExpiredTrial_Test)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, TEST_PRODUCT_ID);

    auto jsonPolicyExpiredTrial = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(-1), userId, std::atoi(TEST_PRODUCT_ID), "", 1, true);
    std::list<std::shared_ptr<Policy>> expiredTrialPolicyList;
    expiredTrialPolicyList.push_back(Policy::Create(jsonPolicyExpiredTrial));

    GetLicensingDbMock().MockUserPolicyFiles(userId, expiredTrialPolicyList);

    EXPECT_EQ(static_cast<int>(client->GetTrialDaysRemaining()), 0);
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    }

TEST_F(ClientTests, CleanUpPolicies_Success)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo);

    auto jsonPolicyExpired = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(-22), DateHelper::AddDaysToCurrentTime(7), userId, 9903, "", 1, false);
    std::list<std::shared_ptr<Policy>> expiredPolicyList;
    expiredPolicyList.push_back(Policy::Create(jsonPolicyExpired));

    GetLicensingDbMock().MockPolicyFiles(expiredPolicyList);
    GetLicensingDbMock().MockDeletePolicyFile(SUCCESS);

    client->CleanUpPolicies();
    EXPECT_LE(1, GetLicensingDbMock().GetPolicyFilesCount());
    EXPECT_LE(1, GetLicensingDbMock().DeletePolicyFileCount());
    }

TEST_F(ClientTests, DeleteLocalCheckout_ValidParam)
    {
    GetLicensingDbMock().MockOpenOrCreate(SUCCESS);
    GetLicensingDbMock().MockDeleteLocalCheckout(SUCCESS);
    
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, "2545");
    EXPECT_SUCCESS(client->DeleteLocalCheckout("2545"));
    }

TEST_F(ClientTests, DeleteLocalCheckout_InvalidParam_TooShort)
    {
    GetLicensingDbMock().MockOpenOrCreate(SUCCESS);
    GetLicensingDbMock().MockDeleteLocalCheckout(ERROR);
    //needs fixed and asserts
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, "123");
    EXPECT_ERROR(client->DeleteLocalCheckout("123"));
    }

TEST_F(ClientTests, DeleteLocalCheckout_InvalidParam_AlphaChar)
    {
    GetLicensingDbMock().MockOpenOrCreate(SUCCESS);
    GetLicensingDbMock().MockDeleteLocalCheckout(ERROR);
    //needs fixed and asserts
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, "1234");
    EXPECT_ERROR(client->DeleteLocalCheckout("1a23"));
    }

TEST_F(ClientTests, DeleteLocalCheckout_InvalidParam_Decimal)
    {
    GetLicensingDbMock().MockOpenOrCreate(SUCCESS);
    GetLicensingDbMock().MockDeleteLocalCheckout(ERROR);
    //needs fixed and asserts
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, "1234");
    EXPECT_ERROR(client->DeleteLocalCheckout("12.12"));
    }

TEST_F(ClientTests, DeleteLocalCheckout_InvalidParam_DollarSymbol)
    {
    GetLicensingDbMock().MockOpenOrCreate(SUCCESS);
    GetLicensingDbMock().MockDeleteLocalCheckout(ERROR);
    //needs fixed and asserts
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, "1234");
    EXPECT_ERROR(client->DeleteLocalCheckout("1$12"));
    }

TEST_F(ClientTests, DeleteLocalCheckout_InvalidParam_PercentSymbol)
    {
    GetLicensingDbMock().MockOpenOrCreate(SUCCESS);
    GetLicensingDbMock().MockDeleteLocalCheckout(ERROR);
    //needs fixed and asserts
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClient(userInfo, "1234");
    EXPECT_ERROR(client->DeleteLocalCheckout("11%2"));
    }

TEST_F(ClientTests, ClientImplBadParams_StartApplication_BadAppInfo)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClientWithBadParams(userInfo, "2545", true);//badapp info, null ptr
    auto result = client->StartApplication();
    EXPECT_EQ(result, LicenseStatus::Error);
    }

TEST_F(ClientTests, ClientImplBadParams_StartApplication_BadDBPath)
    {
    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto userInfo = DummyUserInfoHelper::CreateUserInfo("username", "firstName", "lastName", userId, "orgId");
    auto client = CreateTestClientWithBadParams(userInfo, "2545", false, true);//empty DB path
    auto result = client->StartApplication();
    EXPECT_EQ(result, LicenseStatus::Error);
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

    std::list<std::shared_ptr<Policy>> validPolicyList;
    validPolicyList.push_back(Policy::Create(jsonPolicyValid));

    GetLicensingDbMock().MockUserPolicyFiles(userId, validPolicyList);
    GetLicensingDbMock().MockGetOfflineGracePeriodStart("");
    GetLicensingDbMock().MockRecordFeature(SUCCESS);

    // called in usage heartbeat
    GetLicensingDbMock().MockRecordUsage(SUCCESS);

    // LogPosting heartbeat
    GetLicensingDbMock().MockGetUsageRecordCount(1);
    GetLicensingDbMock().MockGetFeatureRecordCount(1);

    GetUlasProviderMock().MockPostUsageLogs(SUCCESS);
    GetUlasProviderMock().MockPostFeatureLogs(SUCCESS);

    // called in Policy heartbeat
    GetLicensingDbMock().MockGetOfflineGracePeriodStart("");

    FeatureUserDataMapPtr featureAttribute = std::make_shared<FeatureUserDataMap>();

    featureAttribute->AddAttribute("Manufacturer", "Bentley Systems, Inc.");
    featureAttribute->AddAttribute("Website", "https://www.w3schools.com");
    featureAttribute->AddAttribute("Title", "Mobile App");

    ASSERT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));

    EXPECT_SUCCESS(client->MarkFeature("FeatureId", featureAttribute));
    EXPECT_EQ(1, GetPolicyProviderMock().GetPolicyCalls());
    EXPECT_EQ(1, GetLicensingDbMock().AddOrUpdatePolicyFileCount());
    EXPECT_EQ(1, GetLicensingDbMock().DeleteAllOtherPolicyFilesByUserCount());
    EXPECT_EQ(2, GetLicensingDbMock().GetPolicyFilesByUserCount(userId));
    EXPECT_EQ(1, GetLicensingDbMock().OpenOrCreateCount());
    EXPECT_EQ(1, GetLicensingDbMock().RecordFeatureCount());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_SUCCESS(client->StopApplication());
    EXPECT_LE(1, GetUlasProviderMock().PostUsageLogsCalls());
    EXPECT_LE(1, GetUlasProviderMock().PostFeatureLogsCalls());
    EXPECT_LE(1, GetLicensingDbMock().RecordUsageCount()); // called in usage heartbeat
    EXPECT_LE(1, GetLicensingDbMock().GetUsageRecordCountCount());
    EXPECT_LE(1, GetLicensingDbMock().GetFeatureRecordCountCount());
    EXPECT_LE(1, GetLicensingDbMock().GetOfflineGracePeriodStartCount());
    EXPECT_EQ(1, GetLicensingDbMock().CloseCount());
    }
