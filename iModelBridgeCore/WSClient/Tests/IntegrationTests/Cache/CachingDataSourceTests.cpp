/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Cache/CachingDataSourceTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CachingDataSourceTests.h"

#include <WebServices/Cache/CachingDataSource.h>
#include <WebServices/Cache/Util/JsonUtil.h>
#include <WebServices/Cache/Transactions/CacheTransactionManager.h>
#include <WebServices/Connect/ConnectAuthenticationHandler.h>
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/ConnectTokenProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>
#include <WebServices/Connect/ImsClient.h>
#include <MobileDgn/Utils/Http/ProxyHttpHandler.h>
#include <MobileDgn/Utils/Http/HttpConfigurationHandler.h>
#include <WebServices/Configuration/UrlProvider.h>

#include "../../UnitTests/Published/WebServices/Cache/CachingTestsHelper.h"
#include "../../UnitTests/Published/WebServices/Connect/StubLocalState.h"

void CachingDataSourceTests::SetUp()
    {
    WSClientBaseTest::SetUp();
    m_localState = StubLocalState();

    ConnectAuthenticationPersistence::CustomInitialize(&m_localState);
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &m_localState);

    CacheTransactionManager::SetAllowUnsafeAccess(true);

    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_WSCLIENT, NativeLogging::LOG_INFO);
    }

BeFileName GetTestCachePath()
    {
    BeFileName cachePath = StubFilePath("TestWSCache-" + BeGuid().ToString() + ".ecdb");
    CacheEnvironment env = StubCacheEnvironemnt();

    cachePath.BeDeleteFile();
    BeFileName::EmptyAndRemoveDirectory(env.persistentFileCacheDir);
    BeFileName::EmptyAndRemoveDirectory(env.temporaryFileCacheDir);

    EXPECT_FALSE(cachePath.DoesPathExist());
    EXPECT_FALSE(env.persistentFileCacheDir.DoesPathExist());
    EXPECT_FALSE(env.temporaryFileCacheDir.DoesPathExist());

    return cachePath;
    }

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

TEST_F(CachingDataSourceTests, OpenOrCreate_BentleyConnectSharedContent_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://qa-wsg20-eus.cloudapp.net/";
    Utf8String repositoryId = "BentleyCONNECT.SharedContent--CONNECT.SharedContent";
    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");
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

    auto objResult = ds->GetObjects(key, WSQuery("SharedContentSchema", "Project"),
        ICachingDataSource::DataOrigin::RemoteOrCachedData, nullptr, nullptr)->GetResult();
    ASSERT_TRUE(objResult.IsSuccess());
    }

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

TEST_F(CachingDataSourceTests, OpenOrCreate_BentleyConnectProjectSyncServiceShare_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    Utf8String serverUrl = "https://qa-connect-wsg20.bentley.com";
    Utf8String repositoryId = "BentleyCONNECT.ProjectContent--238f67b9-b6db-4b37-810e-bfdc0ab5e0b0"; // QA project "MOBILE-ATP-06-Shares"
    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");
    BeFileName cachePath = GetTestCachePath();

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(credentials)->GetResult().IsSuccess());
    auto authHandler = manager->GetAuthenticationHandler(serverUrl, proxy);

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

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

TEST_F(CachingDataSourceTests, OpenOrCreate_BentleyConnectPunchlist_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://qa-punchlist-eus.cloudapp.net";
    Utf8String repositoryId = "IssuePlugin--default";
    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");
    BeFileName cachePath = GetTestCachePath();

    cachePath = BeFileName(StubFilePath("punchlistcache.ecdb"));
    DataSourceCache::DeleteCacheFromDisk(cachePath, StubCacheEnvironemnt());

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(credentials)->GetResult().IsSuccess());
    auto authHandler = manager->GetAuthenticationHandler(serverUrl, proxy);

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, authHandler);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

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

TEST_F(CachingDataSourceTests, OpenOrCreate_WSG13eBPluginRepository_Succeeds)
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

TEST_F(CachingDataSourceTests, OpenOrCreate_WSG13ProjectWisePluginRepository_Succeeds)
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

TEST_F(CachingDataSourceTests, OpenOrCreate_WSG13SharePointPluginRepository_Succeeds)
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

TEST_F(CachingDataSourceTests, OpenOrCreate_WSG22eBPluginRepository_Succeeds)
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

TEST_F(CachingDataSourceTests, OpenOrCreate_WSG250xProjectWiseRepository_Succeeds)
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

TEST_F(CachingDataSourceTests, OpenOrCreate_WSG250xProjectWiseRepositoryWithImsUser_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    UrlProvider::Initialize(UrlProvider::Release, UrlProvider::DefaultTimeout, &m_localState);
    proxy = std::shared_ptr<HttpConfigurationHandler>(new HttpConfigurationHandler([] (HttpRequest& request)
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

TEST_F(CachingDataSourceTests, OpenOrCreate_WSG250xProjectWiseRepositoryWithInvalidImsUser_ErrorLoginFailed)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://mobilevm2.bentley.com/ws250";
    Utf8String repositoryId = "Bentley.PW--PWDIVM4.bentley.com~3APW10.00.03.30";
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

TEST_F(CachingDataSourceTests, SyncLocalChanges_WSG25ProjectWisePluginRepository_Succeeds)
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

TEST_F(CachingDataSourceTests, GetObjects_WSG250ProjectWiseSpatialQuery_Succeeds)
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

TEST_F(CachingDataSourceTests, GetObjects_PunchlistV1Queries_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://qa-punchlist-eus.cloudapp.net";
    Utf8String repositoryId = "IssuePlugin--default";
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
    CachedResponseKey key {rootKey, "A"};
    CachedResponseKey key2 {rootKey, "B"};
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
            properties["FileName"] = Utf8PrintfString("Test-%d-%s.txt", i, BeGuid().ToString().c_str());
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
