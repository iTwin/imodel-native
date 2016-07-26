/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Examples/ExampleTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------+
|
| Example tests for WIKI page:
| http://bsw-wiki.bentley.com/bin/view.pl/Main/WSClient#WSClientWSG

+--------------------------------------------------------------------------------------*/

#include "ExampleTests.h"

#include <WebServices/Cache/CachingDataSource.h>
#include <WebServices/Cache/Transactions/CacheTransactionManager.h>
#include <WebServices/Connect/ConnectAuthenticationHandler.h>
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/ConnectTokenProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>
#include <WebServices/Connect/ImsClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <WebServices/Configuration/UrlProvider.h>

#include "../../UnitTests/Published/WebServices/Cache/CachingTestsHelper.h"
#include "../../UnitTests/Published/WebServices/Connect/StubLocalState.h"

void ExampleTests::SetUp()
    {
    WSClientBaseTest::SetUp();
    m_localState = StubLocalState();

    ConnectAuthenticationPersistence::CustomInitialize(&m_localState);
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &m_localState);

    CacheTransactionManager::SetAllowUnsafeAccess(true);
    }

TEST_F(ExampleTests, WSRepositoryClient_PWConnectionCreationAndQuery)
    {
    auto clientInfo = ClientInfo::Create("TestApp", {1, 0}, "TestAppGuid");
    Utf8String serverUrl = "https://viltest2-8.bentley.com/ws23";
    Utf8String repositoryId = "Bentley.PW--VILTEST2-5.bentley.com~3APW_Mobile_SS3";

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, clientInfo);
    client->SetCredentials(Credentials("admin", "admin"));

    WSQuery query("PW_WSG", "Document");
    query.SetFilter("FileName+like+'.png'");
    query.SetSelect("FileName,Name");
    query.SetTop(10);

    WSObjectsResult result = client->SendQueryRequest(query)->GetResult();
    if (!result.IsSuccess())
        {
        BeDebugLog(result.GetError().GetMessage().c_str());
        return;
        }

    for (auto instance : result.GetValue().GetInstances())
        {
        ObjectId objId = instance.GetObjectId();
        Utf8String name = instance.GetProperties()["FileName"].GetString();
        BeDebugLog(("Got: " + objId.ToString() + ": " + name).c_str());
        }
    }

TEST_F(ExampleTests, WSRepositoryClient_ConnectConnectionCreationAndQuery)
    {
    auto clientInfo = ClientInfo::Create("TestApp", {1, 0}, "TestAppGuid", "2545");
    Utf8String serverUrl = "https://qa-wsg20-eus.cloudapp.net/";
    Utf8String repositoryId = "BentleyCONNECT.Global--CONNECT.GLOBAL";
    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");

    auto manager = ConnectSignInManager::Create(clientInfo, nullptr, &m_localState);
    auto signInResult = manager->SignInWithCredentials(credentials)->GetResult();
    if (!signInResult.IsSuccess())
        {
        BeDebugLog(signInResult.GetError().GetMessage().c_str());
        return;
        };
    auto authHandler = manager->GetAuthenticationHandler(serverUrl);

    auto client = WSRepositoryClient::Create(serverUrl, repositoryId, clientInfo, nullptr, authHandler);

    WSQuery query("GlobalSchema", "Project");
    query.SetTop(10);

    WSObjectsResult result = client->SendQueryRequest(query)->GetResult();
    if (!result.IsSuccess())
        {
        BeDebugLog(result.GetError().GetMessage().c_str());
        return;
        }
    }

TEST_F(ExampleTests, CachingDataSource_PWConnectionCreationAndQuery)
    {
    BeFileName rootPath = GetTestsTempDir();

    Utf8String serverUrl = "https://viltest2-8.bentley.com/ws23";
    Utf8String repositoryId = "Bentley.PW--VILTEST2-5.bentley.com~3APW_Mobile_SS3";
    Credentials creds("admin", "admin");

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo());
    client->SetCredentials(creds);

    BeFileName cachePath(rootPath + L"/cache.ecdb");
    CacheEnvironment env;
    env.persistentFileCacheDir = BeFileName(rootPath + L"/Persitent");
    env.temporaryFileCacheDir = BeFileName(rootPath + L"/Temp");

    auto openResult = CachingDataSource::OpenOrCreate(client, cachePath, env)->GetResult();
    if (!openResult.IsSuccess())
        {
        BeDebugLog(openResult.GetError().GetMessage().c_str());
        return;
        }

    ICachingDataSourcePtr ds = openResult.GetValue();

    auto txn = ds->StartCacheTransaction();
    ECInstanceKey rootKey = txn.GetCache().FindOrCreateRoot("Searches"); // Create root node to bind results to
    txn.Commit();

    CachedResponseKey responseKey(rootKey, "DocumentSearch"); // Create unique response key under given parent to save results to

    WSQuery query("PW_WSG", "Document");
    query.SetFilter("FileName+like+'.png'");
    query.SetSelect("FileName,Name");
    query.SetTop(10);

    auto result = ds->GetObjects(responseKey, query, ICachingDataSource::DataOrigin::RemoteData, nullptr, nullptr)->GetResult();
    for (auto& jsonInstance : result.GetValue().GetJson())
        {
        Utf8String name = jsonInstance["FileName"].asString();
        BeDebugLog(("Got: " + name).c_str());
        }
    }

TEST_F(ExampleTests, CachingDataSource_PWDocumentRenameAndSync)
    {
    BeFileName rootPath = GetTestsTempDir();

    Utf8String serverUrl = "https://viltest2-8.bentley.com/ws23";
    Utf8String repositoryId = "Bentley.PW--VILTEST2-5.bentley.com~3APW_Mobile_SS3";
    Credentials creds("admin", "admin");

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, repositoryId, StubValidClientInfo());
    client->SetCredentials(creds);

    BeFileName cachePath(rootPath + L"/cache.ecdb");
    CacheEnvironment env;
    env.persistentFileCacheDir = BeFileName(rootPath + L"/Persitent");
    env.temporaryFileCacheDir = BeFileName(rootPath + L"/Temp");

    auto openResult = CachingDataSource::OpenOrCreate(client, cachePath, env)->GetResult();
    if (!openResult.IsSuccess())
        {
        BeDebugLog(openResult.GetError().GetMessage().c_str());
        return;
        }

    ICachingDataSourcePtr ds = openResult.GetValue();
    ObjectId documentId("PW_WSG.Document", "19ddddb1-8652-4141-874b-5de930435a3b");

    auto txn1 = ds->StartCacheTransaction();
    txn1.GetCache().LinkInstanceToRoot(nullptr, documentId);
    txn1.Commit();

    auto result = ds->GetObject(documentId, ICachingDataSource::DataOrigin::RemoteData, IDataSourceCache::JsonFormat::Raw)->GetResult();
    if (!result.IsSuccess())
        {
        BeDebugLog(result.GetError().GetMessage().c_str());
        return;
        }

    auto txn = ds->StartCacheTransaction();
    auto documentKey = txn.GetCache().FindInstance(documentId);

    Json::Value properties;
    txn.GetCache().GetAdapter().GetJsonInstance(properties, documentKey);

    properties["Name"] = ("Renamed " + DateTime::GetCurrentTime().ToUtf8String()).c_str();
    txn.GetCache().GetChangeManager().ModifyObject(documentKey, properties);
    txn.Commit();

    auto syncResult = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    if (!syncResult.IsSuccess())
        {
        BeDebugLog(syncResult.GetError().GetMessage().c_str());
        return;
        }
    if (!syncResult.GetValue().empty())
        {
        for (auto failure : syncResult.GetValue())
            BeDebugLog(failure.GetError().GetMessage().c_str());
        return;
        }
    }