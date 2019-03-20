/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/AccessKeyClientTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestsHelper.h"
#include "JsonHelper.h"
#include "AccessKeyClientTests.h"
#include "DummyPolicyHelper.h"
#include "DummyUserInfoHelper.h"

#include <Licensing/Client.h>
#include <Licensing/Utils/DateHelper.h>
#include "../../Licensing/AccessKeyClientImpl.h"
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

#define TEST_PRODUCT_ID         "2545"
#define TEST_HEARTBEAT_INTERVAL 10 // this is not passed to the actual client, so it does nothing
#define TEST_ACCESSKEY          "somekey"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE

BeFileName GetWithKeyLicensingDbPath()
    {
    BeFileName path;
    BeTest::GetHost().GetTempDir(path);
    path.AppendToPath(L"License.db");

    return path;
    }

AccessKeyClientImplPtr CreateWithKeyTestClient(IPolicyProviderPtr policyProvider, IUlasProviderPtr ulasProvider, ILicensingDbPtr licensingDb, Utf8StringCR productId = TEST_PRODUCT_ID)
    {
    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);

    BeFileName dbPath = GetWithKeyLicensingDbPath();

    return std::make_shared<AccessKeyClientImpl>
        (
        TEST_ACCESSKEY,
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

// TODO: move to Integration tests
//ClientPtr CreateWithKeyTestClientFromFactory(Utf8StringCR productId = TEST_PRODUCT_ID)
//    {
//    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", productId);
//
//    BeFileName dbPath = GetWithKeyLicensingDbPath();
//
//    return Client::CreateWithKey
//        (
//        TEST_ACCESSKEY,
//        clientInfo,
//        dbPath,
//        true,
//        "",
//        "",
//        nullptr
//        );
//    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AccessKeyClientTests::AccessKeyClientTests() :
    m_policyProviderMock(std::make_shared<PolicyProviderMock>()),
    m_ulasProviderMock(std::make_shared<UlasProviderMock>()),
    m_licensingDbMock(std::make_shared<LicensingDbMock>())
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PolicyProviderMock&  AccessKeyClientTests::GetPolicyProviderMock() const
    {
    return *m_policyProviderMock;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<PolicyProviderMock> AccessKeyClientTests::GetPolicyProviderMockPtr() const
    {
    return m_policyProviderMock;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UlasProviderMock&  AccessKeyClientTests::GetUlasProviderMock() const
    {
    return *m_ulasProviderMock;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<UlasProviderMock> AccessKeyClientTests::GetUlasProviderMockPtr() const
    {
    return m_ulasProviderMock;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LicensingDbMock&  AccessKeyClientTests::GetLicensingDbMock() const
    {
    return *m_licensingDbMock;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<LicensingDbMock> AccessKeyClientTests::GetLicensingDbMockPtr() const
    {
    return m_licensingDbMock;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AccessKeyClientTests::TearDown() { }

void AccessKeyClientTests::SetUpTestCase()
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

TEST_F(AccessKeyClientTests, DISABLED_JsonExample)
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

// TODO: move to Integration tests
//TEST_F(AccessKeyClientTests, CreateWithKeyClientFromFactory_Success)
//    {
//    auto client = CreateWithKeyTestClientFromFactory();
//
//    EXPECT_NE(nullptr, client);
//    }

TEST_F(AccessKeyClientTests, WithKeyStartApplication_Error)
    {
    auto client = CreateWithKeyTestClient(GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetLicensingDbMockPtr());

    GetLicensingDbMock().MockOpenOrCreate(ERROR);

    EXPECT_EQ(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));
    EXPECT_EQ(1, GetLicensingDbMock().OpenOrCreateCount());

    EXPECT_SUCCESS(client->StopApplication());
    EXPECT_EQ(1, GetLicensingDbMock().CloseCount());
    }

TEST_F(AccessKeyClientTests, WithKeyStartApplicationNullPolicy_Error)
    {
    auto client = CreateWithKeyTestClient(GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetLicensingDbMockPtr());

    GetLicensingDbMock().MockOpenOrCreate(SUCCESS);

    const std::shared_ptr<Policy> nullPolicy = nullptr;

    GetPolicyProviderMock().MockGetPolicyWithKey(nullPolicy);

    EXPECT_EQ(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));
    EXPECT_EQ(1, GetPolicyProviderMock().GetPolicyWithKeyCalls());
    EXPECT_EQ(1, GetLicensingDbMock().OpenOrCreateCount());

    EXPECT_SUCCESS(client->StopApplication());
    EXPECT_EQ(1, GetLicensingDbMock().CloseCount());
    }

TEST_F(AccessKeyClientTests, WithKeyStartApplicationInvalidKey_Success)
    {
    auto client = CreateWithKeyTestClient(GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetLicensingDbMockPtr());

    GetLicensingDbMock().MockOpenOrCreate(SUCCESS);

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFull(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false);
    const auto validPolicy = Policy::Create(jsonPolicyValid);

    std::list<Json::Value> validPolicyList;
    validPolicyList.push_back(jsonPolicyValid);

    GetPolicyProviderMock().MockGetPolicyWithKey(validPolicy);

    GetLicensingDbMock().MockAddOrUpdatePolicyFile(SUCCESS);
    GetLicensingDbMock().MockDeleteAllOtherPolicyFilesByKey(SUCCESS);

    Json::Value jsonAccessKeyResponse(Json::objectValue);
    jsonAccessKeyResponse["status"] = "Failure";
    jsonAccessKeyResponse["msg"] = "Record is Inactive or Expired";

    GetUlasProviderMock().MockGetAccessKeyInfo(jsonAccessKeyResponse);

    EXPECT_EQ(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::NotEntitled));
    EXPECT_EQ(1, GetLicensingDbMock().OpenOrCreateCount());
    EXPECT_LE(1, GetLicensingDbMock().AddOrUpdatePolicyFileCount());
    EXPECT_LE(1, GetLicensingDbMock().DeleteAllOtherPolicyFilesByKeyCount());
    EXPECT_EQ(1, GetPolicyProviderMock().GetPolicyWithKeyCalls());
    EXPECT_EQ(1, GetUlasProviderMock().GetAccessKeyInfoCalls());

    EXPECT_SUCCESS(client->StopApplication());
    EXPECT_EQ(1, GetLicensingDbMock().CloseCount());
    }

TEST_F(AccessKeyClientTests, WithKeyStartApplication_StopApplication_Success)
    {
    auto client = CreateWithKeyTestClient(GetPolicyProviderMockPtr(), GetUlasProviderMockPtr(), GetLicensingDbMockPtr());

    GetLicensingDbMock().MockOpenOrCreate(SUCCESS);

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";
    auto jsonPolicyValid = DummyPolicyHelper::CreatePolicyFullWithKey(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), DateHelper::AddDaysToCurrentTime(7), userId, std::atoi(TEST_PRODUCT_ID), "", 1, false, TEST_ACCESSKEY);
    auto validPolicy = Policy::Create(jsonPolicyValid);

    Utf8StringCR testAccessKey = TEST_ACCESSKEY;

    GetPolicyProviderMock().MockGetPolicyWithKey(validPolicy);

    GetLicensingDbMock().MockAddOrUpdatePolicyFile(SUCCESS);
    GetLicensingDbMock().MockDeleteAllOtherPolicyFilesByKey(SUCCESS);

    Json::Value jsonAccessKeyResponse(Json::objectValue);
    jsonAccessKeyResponse["status"] = "Success";

    GetUlasProviderMock().MockGetAccessKeyInfo(jsonAccessKeyResponse);

    std::list<Json::Value> validPolicyList;
    validPolicyList.push_back(jsonPolicyValid);

    GetLicensingDbMock().MockKeyPolicyFiles(testAccessKey, validPolicyList);
    GetLicensingDbMock().MockGetOfflineGracePeriodStart("");

    EXPECT_NE(static_cast<int>(client->StartApplication()), static_cast<int>(LicenseStatus::Error));
    EXPECT_EQ(1, GetLicensingDbMock().OpenOrCreateCount());
    EXPECT_LE(1, GetLicensingDbMock().AddOrUpdatePolicyFileCount());
    EXPECT_LE(1, GetLicensingDbMock().DeleteAllOtherPolicyFilesByKeyCount());
    EXPECT_EQ(1, GetLicensingDbMock().GetPolicyFilesByKeyCount(testAccessKey));
    EXPECT_EQ(1, GetLicensingDbMock().GetOfflineGracePeriodStartCount());
    EXPECT_EQ(1, GetPolicyProviderMock().GetPolicyWithKeyCalls());
    EXPECT_EQ(1, GetUlasProviderMock().GetAccessKeyInfoCalls());

    EXPECT_SUCCESS(client->StopApplication());
    EXPECT_EQ(1, GetLicensingDbMock().CloseCount());
    }
