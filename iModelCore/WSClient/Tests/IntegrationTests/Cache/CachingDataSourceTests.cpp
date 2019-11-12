/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <WebServicesTestsHelper.h>
#include <WebServices/Cache/CachingDataSource.h>
#include <WebServices/Cache/Util/JsonUtil.h>
#include <WebServices/Cache/Transactions/CacheTransactionManager.h>
#include <WebServices/Connect/ConnectAuthenticationHandler.h>
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/ConnectTokenProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>
#include <WebServices/Connect/ImsClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeHttp/HttpConfigurationHandler.h>
#include <WebServices/Configuration/UrlProvider.h>

#include "../../UnitTests/NonPublished/WebServices/Cache/CachingTestsHelper.h"

struct CachingDataSourceTests : WSClientBaseTest
    {
    RuntimeJsonLocalState m_localState;
    void SetUp();
    };

void CachingDataSourceTests::SetUp()
    {
    WSClientBaseTest::SetUp();
    m_localState = RuntimeJsonLocalState();

    ConnectAuthenticationPersistence::CustomInitialize(&m_localState);
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &m_localState);

    CacheTransactionManager::SetAllowUnsafeAccess(true);

    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_WSCLIENT, NativeLogging::LOG_INFO);
    }

BeFileName GetTestCachePath()
    {
    BeFileName cachePath = StubFilePath("TestWSCache-" + BeGuid(true).ToString() + ".ecdb");
    CacheEnvironment env = StubCacheEnvironemnt();

    cachePath.BeDeleteFile();
    BeFileName::EmptyAndRemoveDirectory(env.persistentFileCacheDir);
    BeFileName::EmptyAndRemoveDirectory(env.temporaryFileCacheDir);

    EXPECT_FALSE(cachePath.DoesPathExist());
    EXPECT_FALSE(env.persistentFileCacheDir.DoesPathExist());
    EXPECT_FALSE(env.temporaryFileCacheDir.DoesPathExist());

    return cachePath;
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_BentleyConnectGlobal_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://qa-wsg20-eus.cloudapp.net/";
    Utf8String repositoryId = "BentleyCONNECT.Global--CONNECT.GLOBAL";
    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");
    BeFileName cachePath = GetTestCachePath();

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(credentials)->GetResult().IsSuccess());
    auto authHandler = manager->GetAuthenticationHandler(serverUrl, proxy);

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    auto ds = result.GetValue();

    auto txn = ds->StartCacheTransaction();
    CachedResponseKey key(txn.GetCache().FindOrCreateRoot(nullptr), "Foo");
    txn.Commit();

    WSQuery query("GlobalSchema", "Project");
    query.SetTop(100);
    auto objResult = ds->GetObjects(key, query,
        ICachingDataSource::DataOrigin::RemoteOrCachedData, nullptr, nullptr)->GetResult();
    ASSERT_TRUE(objResult.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_BentleyConnectGlobalDev_Succeeds)
    {
    UrlProvider::Initialize(UrlProvider::Dev, UrlProvider::DefaultTimeout, &m_localState);
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://dev-wsg20-eus.cloudapp.net/";
    Utf8String repositoryId = "BentleyCONNECT.Global--CONNECT.GLOBAL";
    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");
    BeFileName cachePath = GetTestCachePath();

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(credentials)->GetResult().IsSuccess());
    auto authHandler = manager->GetAuthenticationHandler(serverUrl, proxy);

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_BentleyConnectPersonalShare_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://qa-wsg20-eus.cloudapp.net";
    Utf8String repositoryId = "BentleyCONNECT.PersonalPublishing--CONNECT.PersonalPublishing";
    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");
    BeFileName cachePath = GetTestCachePath();

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(credentials)->GetResult().IsSuccess());
    auto authHandler = manager->GetAuthenticationHandler(serverUrl, proxy);

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_BentleyConnectPersonalShareNewFile_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &m_localState);
    Utf8String serverUrl = "https://qa-wsg20-eus.cloudapp.net";
    Utf8String repositoryId = "BentleyCONNECT.PersonalPublishing--CONNECT.PersonalPublishing";
    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");
    BeFileName cachePath = GetTestCachePath();

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(credentials)->GetResult().IsSuccess());
    auto authHandler = manager->GetAuthenticationHandler(serverUrl, proxy);

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);

    auto ds = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_FALSE(nullptr == ds);

    Utf8String fileName = "CachingDataSourceTests-Foo.txt";
    Utf8String fileContent = "Test data";

    // List all files and cleanup previous one
    auto filesResult = ds->GetClient()->SendQueryRequest(WSQuery("PersonalPublishing", "PublishedFile"))->GetResult();
    ASSERT_TRUE(filesResult.IsSuccess());
    for (auto file : filesResult.GetValue().GetInstances())
        {
        BeDebugLog(JsonUtil::ToStyledString(file.GetProperties()).c_str());
        if (file.GetProperties()["Name"].GetString() == fileName)
            ASSERT_TRUE(ds->GetClient()->SendDeleteObjectRequest(file.GetObjectId())->GetResult().IsSuccess());
        }

    // Create new file
    auto txn = ds->StartCacheTransaction();
    auto fileClass = txn.GetCache().GetAdapter().GetECClass("PersonalPublishing.PublishedFile");
    ASSERT_FALSE(nullptr == fileClass);

    Json::Value properties;
    properties["Name"] = fileName;
    auto fileKey = txn.GetCache().GetChangeManager().CreateObject(*fileClass, properties);
    ASSERT_TRUE(fileKey.IsValid());
    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(fileKey, StubFile(fileContent, fileName), true));
    txn.Commit();

    // Sync new file
    auto result = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.GetValue().empty());

    // Download file
    ObjectId fileId = ds->StartCacheTransaction().GetCache().FindInstance(fileKey);
    auto fileResult = ds->GetFile(fileId, ICachingDataSource::DataOrigin::RemoteData, nullptr, nullptr)->GetResult();
    ASSERT_TRUE(fileResult.IsSuccess());
    auto path = fileResult.GetValue().GetFilePath();
    EXPECT_EQ(fileContent, SimpleReadFile(path));
    EXPECT_EQ(fileName, Utf8String(path.GetFileNameAndExtension()));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_BentleyConnectProjectShare_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    Utf8String serverUrl = "https://qa-connect-projectsharestorage.bentley.com";
    Utf8String repositoryId = "BentleyCONNECT.ProjectShareV2--be34a3c3-e1df-4540-85a9-e39a2095225a"; // QA project "MOBILE-ATP-06-Shares"
    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");
    BeFileName cachePath = GetTestCachePath();

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(credentials)->GetResult().IsSuccess());
    auto authHandler = manager->GetAuthenticationHandler(serverUrl, proxy);

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
// FAIL: 
// BIM02: Invalid property in ECClass 'Forms_EC_Mapping:FormStyle': The property 'Id' has a name of an ECSQL system property which is not allowed.
// SOLUTION: "IssuePlugin--default" is only used in Graphite generation, no fix
TEST_F(CachingDataSourceTests, OpenOrCreate_BentleyConnectPunchlist_Succeeds_KnowIssue_NeedsServerFix)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://qa-punchlist-eus.cloudapp.net";
    Utf8String repositoryId = "IssuePlugin--default";
    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");
    BeFileName cachePath = GetTestCachePath();

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(credentials)->GetResult().IsSuccess());
    auto authHandler = manager->GetAuthenticationHandler(serverUrl, proxy);

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
// FAIL:
// BIM02: Invalid property in ECClass 'Forms_EC_Mapping:FormStyle': The property 'Id' has a name of an ECSQL system property which is not allowed.
// BIM02: ECClass 'ClashDetection:ResultBase' has invalid base class : An abstract class must not have a non - abstract base class.
// SOLUTION: Need IssuePluginV1.2 with changes
TEST_F(CachingDataSourceTests, OpenOrCreate_BentleyPunchlistV11_Succeeds_KnownIssue_NeedsServerFix)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://qa-punchlist-eus.cloudapp.net";
    Utf8String repositoryId = "IssuePluginV1.1--default";
    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");
    BeFileName cachePath = GetTestCachePath();

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(credentials)->GetResult().IsSuccess());
    auto authHandler = manager->GetAuthenticationHandler(serverUrl, proxy);

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_BentleyMarkupV2_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://qa-connect-punchlist.bentley.com";
    Utf8String repositoryId = "MarkupServiceV2.0--default";
    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");
    BeFileName cachePath = GetTestCachePath();

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(credentials)->GetResult().IsSuccess());
    auto authHandler = manager->GetAuthenticationHandler(serverUrl, proxy);

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_BentleyConnectFeatureTracking_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://QA-SELECTserver.bentley.com/LicensingProxy";
    Utf8String repositoryId = "BentleyCONNECT--Main";
    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");
    BeFileName cachePath = GetTestCachePath();

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(credentials)->GetResult().IsSuccess());
    auto authHandler = manager->GetAuthenticationHandler(serverUrl, proxy);

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_BentleyConnectBIMReviewShare_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = UrlProvider::Urls::BIMReviewShare.Get();
    Utf8String repositoryId = "ContentPlugin--default";
    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");
    BeFileName cachePath = GetTestCachePath();

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(credentials)->GetResult().IsSuccess());
    auto authHandler = manager->GetAuthenticationHandler(serverUrl, proxy);

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
// ERROR    ECDb                 JsonInserter failure. The JSON member '$id' does not match with a property in ECClass 'WSCacheMetaSchema:ECSchemaDef'
TEST_F(CachingDataSourceTests, OpenOrCreate_WSG13eBPluginRepository_Succeeds_KnownIssue_MaybeClientFixPossible)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://mobilevm1.bentley.com/ws";
    Utf8String repositoryId = "eB.MobileVM2.bentley.com,eB_15.4_CR";
    Credentials creds("admin", "admin");
    BeFileName cachePath = GetTestCachePath();

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
// ERROR    ECDb                 JsonInserter failure. The JSON member '$id' does not match with a property in ECClass 'WSCacheMetaSchema:ECSchemaDef'.
TEST_F(CachingDataSourceTests, OpenOrCreate_WSG13ProjectWisePluginRepository_Succeeds_KnownIssue_MaybeClientFixPossible)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://mobilevm5.bentley.com/ws";
    Utf8String repositoryId = "pw.Mobile_ATP_2";
    Credentials creds("admin", "admin");
    BeFileName cachePath = GetTestCachePath();

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
// FAIL:
// BIM02: Invalid property in ECClass 'Bentley_SP:Field': The property 'ID' has a name of an ECSQL system property which is not allowed.
// ERROR    ECDb                 JsonInserter failure. The JSON member '$id' does not match with a property in ECClass 'WSCacheMetaSchema:ECSchemaDef'.
// SOLUTION: bim02 added this constraing, share point plugin is deprecated, no fix
TEST_F(CachingDataSourceTests, OpenOrCreate_WSG13SharePointPluginRepository_Succeeds_KnowIssue_NeedsServerFix)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://mobilevm1.bentley.com/ws";
    Utf8String repositoryId = "ec.Bentley.ECOM.SharePointProvider--http~3A~2F~2Fpw_edge_sp_2013.bentley.com";
    Credentials creds("ps_edge_sp_2013.bentley.com\\Administrator", "Q!w2e3r4");
    BeFileName cachePath = GetTestCachePath();

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
// FAIL: eB schema incomaptible to BIM02
// SOLUTION: redesign eB schema in new WSG 2.6+ release, no fix for ProjectContent?
TEST_F(CachingDataSourceTests, OpenOrCreate_WSG22eBPluginRepository_Succeeds_KnownIssue_NeedsServerFix)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://mobilevm3.bentley.com/ws22";
    Utf8String repositoryId = "Bentley.eB--MobileVM2.bentley.com~2CeB_15.4_CR";
    Credentials creds("admin", "admin");
    BeFileName cachePath = GetTestCachePath();

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
// FAIL: 
// ERROR    ECDb                 ECClass 'PW_WSG:Set' has invalid base class: An abstract class must not have a non-abstract base class.
TEST_F(CachingDataSourceTests, OpenOrCreate_WSG250xProjectWiseRepository_Succeeds_KnownIssue_NeedsServerFix)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://mobilevm2.bentley.com/ws250";
    Utf8String repositoryId = "Bentley.PW--MobileVM1.bentley.com~3AMobile_ATP_2";
    Credentials creds("itest", "itest");
    BeFileName cachePath = GetTestCachePath();

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
// FAIL:
// ERROR    ECDb                 ECClass 'PW_WSG:Set' has invalid base class: An abstract class must not have a non-abstract base class.
TEST_F(CachingDataSourceTests, OpenOrCreate_WSG250xProjectWiseRepositoryWithImsUser_Succeeds_KnownIssue_NeedsServerFix)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    UrlProvider::Initialize(UrlProvider::Release, UrlProvider::DefaultTimeout, &m_localState);
    proxy = std::shared_ptr<HttpConfigurationHandler>(new HttpConfigurationHandler([] (Http::Request& request)
        {
        request.SetValidateCertificate(false);
        }, proxy));

    Utf8String serverUrl = "https://mobilevm2.bentley.com/ws250";
    Utf8String repositoryId = "Bentley.PW--MobileVM1.bentley.com~3APW_Mobile_10.00.02.265";
    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");
    BeFileName cachePath = GetTestCachePath();

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(credentials)->GetResult().IsSuccess());
    auto authHandler = manager->GetAuthenticationHandler(serverUrl, proxy);

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_WSG250xProjectWiseRepositoryWithInvalidImsUser_ErrorLoginFailed)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://mobilevm2.bentley.com/ws250";
    Utf8String repositoryId = "Bentley.PW--MobileVM1.bentley.com~3AMobile_ATP_2";
    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");
    BeFileName cachePath = GetTestCachePath();

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(credentials)->GetResult().IsSuccess());
    auto authHandler = manager->GetAuthenticationHandler(serverUrl, proxy);

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Id::LoginFailed, result.GetError().GetWSError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, OpenOrCreate_WSG262xProjectWise100003xxRepository_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://mobilevm6.bentley.com/ws";
    Utf8String repositoryId = "Bentley.PW--MobileVM1.bentley.com~3AMobile_ATP_2";
    Credentials creds("itest", "itest");
    BeFileName cachePath = GetTestCachePath();

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CachingDataSourceTests, SyncLocalChanges_WSG25ProjectWisePluginRepository_Succeeds_KnownIssue_NeedsServerFix)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://mobilevm2.bentley.com/ws250";
    Utf8String repositoryId = "Bentley.PW--MobileVM1.bentley.com~3AMobile_ATP_2";
    Credentials creds("itest", "itest");
    BeFileName cachePath = GetTestCachePath();

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto ds = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_FALSE(nullptr == ds);
    BentleyStatus status;

    /* pw:\\MobileVM1.bentley.com:Mobile_ATP_2\Documents\WSClientIntegrationTests_DO_NOT_DELETE\CachingDataSourceTests\Uploads\ */
    ObjectId uploadsFolderId{"PW_WSG", "Project", "a934340b-f7f8-4c78-a9fa-cc13cd80679e"};
    ObjectId uploadsFolderNavNodeId{"Navigation", "NavNode", "StandardDisplayItemTypes--PW_WSG.02.03-Project-a934340b--f7f8--4c78--a9fa--cc13cd80679e"};

    if (true)
        {
        auto txn = ds->StartCacheTransaction();
        ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, uploadsFolderId));
        ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, uploadsFolderNavNodeId));
        txn.Commit();
        }

    auto navigationResult = ds->GetNavigationChildren(uploadsFolderNavNodeId, CachingDataSource::DataOrigin::RemoteData, nullptr)->GetResult();
    ASSERT_TRUE(navigationResult.IsSuccess());

    if (true)
        {
        // Cleanup folder content
        auto txn = ds->StartCacheTransaction();
        auto& uploadedInstanceNavNodes = navigationResult.GetValue().GetJson();
        for (auto& uploadedInstanceNavNode : uploadedInstanceNavNodes)
            {
            auto schemaName = uploadedInstanceNavNode["Key_SchemaName"].asString();

            // Trim away version
            auto pos = schemaName.find('.');
            if (pos != Utf8String::npos)
                schemaName = schemaName.substr(0, pos);

            ObjectId uploadedInstanceId
                (
                schemaName,
                uploadedInstanceNavNode["Key_ClassName"].asString(),
                uploadedInstanceNavNode["Key_InstanceId"].asString()
                );

            ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot("DeletedInstances", uploadedInstanceId));
            ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().DeleteObject(txn.GetCache().FindInstance(uploadedInstanceId)));
            }
        txn.Commit();
        }

    Json::Value newDocument = ToJson(
        R"({
        "Name": "TestCreation",
        "CreateTime" : "2014-06-03T08:59:58.673Z"
        })");

    if (true)
        {
        auto txn = ds->StartCacheTransaction();
        auto ecClass = txn.GetCache().GetAdapter().GetECClass("PW_WSG", "Document");
        auto ecRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("PW_WSG", "DocumentParent");

        auto documentKey = txn.GetCache().GetChangeManager().CreateObject(*ecClass, newDocument);

        ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(documentKey, StubFile("TestContent", "'Foo file'.txt"), false));
        ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot("NewDocument", txn.GetCache().FindInstance(documentKey)));

        auto folderKey = txn.GetCache().FindInstance(uploadsFolderId);
        auto relationshipKey = txn.GetCache().GetChangeManager().CreateRelationship(*ecRelClass, documentKey, folderKey);
        ASSERT_TRUE(relationshipKey.IsValid());
        txn.Commit();
        }

    Json::Value newProject = ToJson(
        R"({
        "Name": "TestCreatedFolder",
        "CreateTime" : "2014-06-03T08:59:58.673Z"
        })");

    if (true)
        {
        auto txn = ds->StartCacheTransaction();
        auto ecClass = txn.GetCache().GetAdapter().GetECClass("PW_WSG", "Project");
        auto ecRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("PW_WSG", "ProjectParent");

        auto projectKey = txn.GetCache().GetChangeManager().CreateObject(*ecClass, newProject);
        ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot("NewProject", txn.GetCache().FindInstance(projectKey)));
        auto folderKey = txn.GetCache().FindInstance(uploadsFolderId);
        auto relationshipKey = txn.GetCache().GetChangeManager().CreateRelationship(*ecRelClass, projectKey, folderKey);
        ASSERT_TRUE(relationshipKey.IsValid());
        txn.Commit();
        }

    auto syncResult = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_TRUE(syncResult.IsSuccess());

    if (true)
        {
        auto txn = ds->StartCacheTransaction();
        Json::Value newInstances;
        status = txn.GetCache().ReadInstancesLinkedToRoot("NewDocument", newInstances);
        ASSERT_EQ(SUCCESS, status);
        ASSERT_EQ(1, newInstances.size());
        auto newInstance = newInstances[0];

        auto newKey = txn.GetCache().GetAdapter().GetInstanceKeyFromJsonInstance(newInstance);

        status = txn.GetCache().GetChangeManager().ModifyFile(newKey, StubFile("TestContentModified"), false);
        ASSERT_EQ(SUCCESS, status);
        txn.Commit();
        }

    syncResult = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_TRUE(syncResult.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
// WIP06: Fails on DgnDb61-16Q4 as PW_WSG:SpatialObjectLocation cannot be mapped to ECDb
TEST_F(CachingDataSourceTests, GetObjects_WSG250ProjectWiseSpatialQuery_Succeeds_KnownIssue_NeedsServerFix)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://mobilevm2.bentley.com/ws250";
    Utf8String repositoryId = "Bentley.PW--MobileVM1.bentley.com~3APW_Mobile_10.00.02.265";
    Credentials creds("admin", "admin");
    BeFileName cachePath = GetTestCachePath();

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto ds = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_FALSE(nullptr == ds);

    WSQueryPtr query = std::make_shared<WSQuery>("PW_WSG", "Project");

    query->SetSelect("*,SpatialLocation.*");
    query->SetFilter("geo.intersects("
                     "SpatialLocation.Location,geometry'Polygon(("
                     "-90+10,"
                     "20+90,"
                     "80+90,"
                     "80+10,"
                     "-90+10))')");
    query->SetSkip(0);
    query->SetTop(100);

    auto txn = ds->StartCacheTransaction();
    CachedResponseKey resultsKey(txn.GetCache().FindOrCreateRoot("Spatial"), "SpatialData");
    txn.Commit();

    auto result = ds->GetObjects(resultsKey, *query, ICachingDataSource::DataOrigin::CachedOrRemoteData, nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    BeDebugLog(result.GetValue().GetJson().toStyledString().c_str());
    ASSERT_NE(0, result.GetValue().GetJson().size());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
// BIM0200 does not support ECDbMap.01.00 schema, but it is required here for DgnDb0601 version, need IssuePluginV1.2--default
TEST_F(CachingDataSourceTests, GetObjects_PunchlistV11Queries_Succeeds_KnownIssue_NeedsServerFix)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://qa-punchlist-eus.cloudapp.net/"; // Only DEV has 06xx support now
    Utf8String repositoryId = "IssuePluginV1.1--default"; // New plugin version for 06xx compatibility
    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");
    BeFileName cachePath = GetTestCachePath();

    cachePath = BeFileName(StubFilePath("punchlistcache.ecdb"));
    DataSourceCache::DeleteCacheFromDisk(cachePath, StubCacheEnvironemnt());

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(credentials)->GetResult().IsSuccess());
    auto authHandler = manager->GetAuthenticationHandler(serverUrl, proxy);

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);

    auto ds = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_FALSE(nullptr == ds);

    auto txn = ds->StartCacheTransaction();
    auto rootKey = txn.GetCache().FindOrCreateRoot(nullptr);
    CachedResponseKey key{rootKey, "A"};
    CachedResponseKey key2{rootKey, "B"};
    txn.Commit();

    ICachingDataSource::ObjectsResult result;
    Utf8String jsonStr;

    Utf8String projectId = "0c9c2cf3-6031-47c8-b9ea-7da2f5cc57bf"; // Qa
    //Utf8String projectId = "41bd30f0-7b36-4253-aa01-5326b1a0ea24"; // Dev
    Utf8String oDataQuery = "ConnectProjectId+eq+'" + projectId + "'";

    WSQuery jobsQuery("ClashDetection", "Job");
    jobsQuery.SetSelect("*,*,ClashDetection.JobHasReport-forward-ClashDetection.Report.*,ClashDetection.JobHasRules-forward-ClashDetection.Rule.*");
    jobsQuery.SetFilter(oDataQuery);

    WSQuery resultQuery("ClashDetection", "Result");
    resultQuery.SetSelect("*,ClashDetection.ReportHasResults-backward-ClashDetection.Report.$id,Issue.IssueCommentRel-forward-Issue.Comment.*,Issue.IssueAttachmentRel-forward-Issue.Attachment.*");
    resultQuery.SetFilter(oDataQuery);

    result = ds->GetObjects(key2, jobsQuery, ICachingDataSource::DataOrigin::RemoteData, nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    jsonStr = result.GetValue().GetJson().toStyledString();
    BeDebugLog(jsonStr.c_str());

    result = ds->GetObjects(key, resultQuery, ICachingDataSource::DataOrigin::RemoteData, nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    jsonStr = result.GetValue().GetJson().toStyledString();
    BeDebugLog(jsonStr.c_str());

    bool createAttachment = false;
    int repeat = 1;

    for (int i = 1; i <= repeat; i++)
        {
        if (createAttachment)
            {
            auto txn = ds->StartCacheTransaction();
            auto attachmentClass = txn.GetCache().GetAdapter().GetECClass("Issue.Attachment");
            auto resultClass = txn.GetCache().GetAdapter().GetECClass("ClashDetection.Result");
            auto relClass = txn.GetCache().GetAdapter().GetECRelationshipClass("Issue.IssueAttachmentRel");
            ECInstanceKey result(resultClass->GetId(), txn.GetCache().GetAdapter().FindInstance(resultClass));
            Json::Value properties;
            properties["FileName"] = Utf8PrintfString("Test-%d-%s.txt", i, BeGuid(true).ToString().c_str());
            auto attachment = txn.GetCache().GetChangeManager().CreateObject(*attachmentClass, properties);
            auto relationship = txn.GetCache().GetChangeManager().CreateRelationship(*relClass, result, attachment);
            ASSERT_TRUE(relationship.IsValid());
            ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(attachment, StubFile("Content", "Test.txt"), true));
            txn.Commit();
            }

        auto syncResult = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
        ASSERT_TRUE(syncResult.IsSuccess());
        ASSERT_TRUE(syncResult.GetValue().empty());

        result = ds->GetObjects(key, resultQuery, ICachingDataSource::DataOrigin::RemoteData, nullptr, nullptr)->GetResult();
        ASSERT_TRUE(result.IsSuccess());
        jsonStr = result.GetValue().GetJson().toStyledString();
        BeDebugLog(jsonStr.c_str());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*
TEST_F(CachingDataSourceTests, ECDbPrepareStatement_ChangesMadeInBetweenReuses_FindsChanges)
    {
    // Setup ECDb
    ECDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    // Setup Schema
    auto schema = ParseSchema(R"xml(
        <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" > 
                <ECProperty propertyName="Name" typeName="string"/>
                <ECProperty propertyName="Persistence" typeName="int"/>
            </ECClass>
        </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, db.Schemas().ImportECSchemas({schema.get()}));

    ECClassCP rootClass = db.GetClassLocater().LocateClass("TestSchema", "TestClass");
    ASSERT_NE(nullptr, rootClass);

    // Names
    Utf8String rootName = "Foo";

    // Test quety for same instance
    Utf8String ecsql = "SELECT ECInstanceId FROM [TS].[TestClass] WHERE [Name] = ? LIMIT 1 ";
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(db, ecsql.c_str()));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, rootName.c_str(), IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(BE_SQLITE_DONE, statement.Step());

    // Insert one instnace
    Json::Value rootInstance;
    rootInstance["Name"] = rootName;
    rootInstance["Persistence"] = 0;

    JsonInserter inserter(db, *rootClass, nullptr);
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(rootInstance));

    // Try again
    statement.Reset();
    statement.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, rootName.c_str(), IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
    EXPECT_EQ(ECInstanceId(1ull), statement.GetValueId <ECInstanceId>(0));
    } */
