/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Cache/CachingDataSourceTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CachingDataSourceTests.h"

#include <WebServices/Cache/CachingDataSource.h>
#include <WebServices/Cache/Transactions/CacheTransactionManager.h>
#include <WebServices/Connect/ConnectAuthenticationHandler.h>
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/ConnectTokenProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>
#include <WebServices/Connect/ImsClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Cache/Util/JsonUtil.h>

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

    UrlProvider::Initialize(UrlProvider::Dev, UrlProvider::DefaultTimeout, &m_localState);
    Utf8String serverUrl = "https://dev-wsg20-eus.cloudapp.net";
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

TEST_F(CachingDataSourceTests, OpenOrCreate_BentleyConnectProjectShare_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://qa-wsg20-eus.cloudapp.net";
    Utf8String repositoryId = "BentleyCONNECT.ProjectContent--c4d60664-1226-4d4f-8beb-7d0e372ccc06";
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

    Utf8String serverUrl = "https://viltest2-7.bentley.com/ws";
    Utf8String repositoryId = "eB.viltest2-5.bentley.com,eB_Mobile";
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

    Utf8String serverUrl = "https://viltest2-7.bentley.com/ws";
    Utf8String repositoryId = "pw.PW_Mobile_SS3";
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

    Utf8String serverUrl = "https://viltest2-7.bentley.com/ws";
    Utf8String repositoryId = "ec.Bentley.ECOM.SharePointProvider--http~3A~2F~2Fviltest2-10";
    Credentials creds("VILTEST2-10\\administrator", "Q!w2e3r4");
    BeFileName cachePath = GetTestCachePath();

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

// WIP06: fails due to schema compatibility issues to 06xx ECv3 ECDb
TEST_F(CachingDataSourceTests, OpenOrCreate_WSG22eBPluginRepository_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://viltest2-5.bentley.com/ws22";
    Utf8String repositoryId = "Bentley.eB--viltest2-5.bentley.com~2CeB_Mobile";
    Credentials creds("admin", "admin");
    BeFileName cachePath = GetTestCachePath();

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

TEST_F(CachingDataSourceTests, OpenOrCreate_WSG23ProjectWisePluginRepository_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://viltest2-7.bentley.com/ws23";
    Utf8String repositoryId = "Bentley.PW--VILTEST2-5.bentley.com~3APW_Mobile_SS3";
    Credentials creds("admin", "admin");
    BeFileName cachePath = GetTestCachePath();

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

TEST_F(CachingDataSourceTests, OpenOrCreate_WSG24ProjectWisePluginRepository_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://viltest2-8.bentley.com/ws24";
    Utf8String repositoryId = "Bentley.PW--VILTEST2-5.bentley.com~3APW_Mobile_SS3";
    Credentials creds("admin", "admin");
    BeFileName cachePath = GetTestCachePath();

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

TEST_F(CachingDataSourceTests, OpenOrCreate_WSG2xProjectWisePluginMapMobileRepository_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://geo-demo-ws.bentley.com/ws";
    Utf8String repositoryId = "Bentley.PW--geo-demo.bentley.com~3AGeo-Demo";
    Credentials creds("twc", "admin");
    BeFileName cachePath = GetTestCachePath();

    auto info = std::make_shared<ClientInfo>("Bentley-MapMobile", BeVersion(5, 4), "77def89a-7e50-4f0e-a4c7-24fb6044dbfb",
                                             "CLQIqB7y8eCUpdJe5uyRVVaaGbk=", "Windows 6.1", nullptr);

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto result = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult();
    ASSERT_FALSE(nullptr == result.GetValue());
    }

TEST_F(CachingDataSourceTests, SyncLocalChanges_WSG24ProjectWisePluginRepository_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://viltest2-8.bentley.com/ws24";
    Utf8String repositoryId = "Bentley.PW--VILTEST2-5.bentley.com~3APW_Mobile_SS3";
    Credentials creds("admin", "admin");
    BeFileName cachePath = GetTestCachePath();

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto ds = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_FALSE(nullptr == ds);
    BentleyStatus status;

    /* pw:\\VILTEST2-5.bentley.com:PW_Mobile_SS3\Documents\VRA\WSClientIntegrationTests\CachingDataSourceTests\Uploads\ */
    ObjectId uploadsFolderId {"PW_WSG", "Project", "30ff82ee-0a91-42ca-9dd6-c941e577be94"};
    ObjectId uploadsFolderNavNodeId {"Navigation", "NavNode", "ECObjects--" + uploadsFolderId.schemaName + "-" + uploadsFolderId.className + "-" + uploadsFolderId.remoteId};

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
        auto txn = ds->StartCacheTransaction();
        auto& uploadedInstanceNavNodes = navigationResult.GetValue().GetJson();
        for (auto& uploadedInstanceNavNode : uploadedInstanceNavNodes)
            {
            ObjectId uploadedInstanceId
                (
                uploadedInstanceNavNode["Key_SchemaName"].asString(),
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

TEST_F(CachingDataSourceTests, GetObjects_WSG24ProjectWiseSpatialQuery_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://viltest2-8.bentley.com/ws24";
    Utf8String repositoryId = "Bentley.PW--VILTEST2-5.bentley.com~3APW_Mobile_SS3";
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
                     "20.896001+53.065279,"
                     "20.896001+57.105292,"
                     "26.872565+57.105292,"
                     "26.872565+53.065279,"
                     "20.896001+53.065279))')");
    query->SetSkip(0);
    query->SetTop(100);

    auto txn = ds->StartCacheTransaction();
    CachedResponseKey resultsKey(txn.GetCache().FindOrCreateRoot("Spatial"), "SpatialData");
    txn.Commit();

    auto result = ds->GetObjects(resultsKey, *query, ICachingDataSource::DataOrigin::CachedOrRemoteData, nullptr, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    BeDebugLog(result.GetValue().GetJson().toStyledString().c_str());
    }

TEST_F(CachingDataSourceTests, GetObjects_PunchlistQueries_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://dev-punchlist-eus.cloudapp.net/"; // Only DEV has 06xx support now
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
