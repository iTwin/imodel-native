/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Cache/CachingDataSourceTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CachingDataSourceTests.h"

#include <WebServices/Cache/CachingDataSource.h>
#include <WebServices/Cache/Transactions/CacheTransactionManager.h>
#include <WebServices/Connect/ConnectAuthenticationHandler.h>
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/ConnectTokenProvider.h>
#include <WebServices/Connect/Connect.h>
#include <DgnClientFx/Utils/Http/ProxyHttpHandler.h>
#include <WebServices/Configuration/UrlProvider.h>

#include "../../UnitTests/Published/WebServices/Cache/CachingTestsHelper.h"
#include "../../UnitTests/Published/WebServices/Connect/StubLocalState.h"

void CachingDataSourceTests::SetUpTestCase()
    {
    static StubLocalState localState;
    localState.GetStubMap().clear();

    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &localState);

    CacheTransactionManager::SetAllowUnsafeAccess(true);
    }

TEST_F(CachingDataSourceTests, OpenOrCreate_BentleyConnectGlobal_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    Connect::Initialize(StubClientInfo(), proxy);

    Utf8String serverUrl = "https://dev-wsg20-eus.cloudapp.net/";
    Utf8String dataSourceId = "BentleyCONNECT.Global--CONNECT.GLOBAL";
    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");
    BeFileName cachePath(L":memory:");

    StubLocalState localState;
    ConnectAuthenticationPersistence::CustomInitialize(&localState);
    auto persistence = ConnectAuthenticationPersistence::GetShared();
    auto provider = std::make_shared<ConnectTokenProvider>(persistence);
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(serverUrl, provider, proxy);

    persistence->SetCredentials(credentials);

    auto client = WSRepositoryClient::Create(serverUrl, dataSourceId, StubClientInfo(), nullptr, authHandler);

    auto ds = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_FALSE(nullptr == ds);
    }

TEST_F(CachingDataSourceTests, DISABLED_OpenOrCreate_BentleyConnectProject_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    Connect::Initialize(StubClientInfo(), proxy);

    Utf8String serverUrl = "https://dev-connectgateway-eus.cloudapp.net";
    Utf8String dataSourceId = "1171";
    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");
    BeFileName cachePath(L":memory:");

    StubLocalState localState;
    ConnectAuthenticationPersistence::CustomInitialize(&localState);
    auto persistence = ConnectAuthenticationPersistence::GetShared();
    auto provider = std::make_shared<ConnectTokenProvider>(persistence);
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(serverUrl, provider, proxy);

    persistence->SetCredentials(credentials);

    auto client = WSRepositoryClient::Create(serverUrl, dataSourceId, StubClientInfo(), nullptr, authHandler);

    auto ds = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_FALSE(nullptr == ds);

    ObjectId fileId("eBSchema.Document", "62172");
    auto txn = ds->StartCacheTransaction();
    txn.GetCache().LinkInstanceToRoot(nullptr, fileId);
    txn.Commit();

    auto fileResult = ds->GetFile(fileId, CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr, nullptr)->GetResult();
    ASSERT_TRUE(fileResult.IsSuccess());
    BeDebugLog(fileResult.GetValue().GetFilePath().GetNameUtf8().c_str());
    auto fileResult2 = ds->GetFile(fileId, CachingDataSource::DataOrigin::RemoteData, nullptr, nullptr)->GetResult();

    auto result = ds->GetNavigationChildren(ObjectId(), CachingDataSource::DataOrigin::CachedOrRemoteData, nullptr)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    BeDebugLog(result.GetValue().GetJson().toStyledString().c_str());
    auto result2 = ds->GetNavigationChildren(ObjectId(), CachingDataSource::DataOrigin::RemoteData, nullptr)->GetResult();
    }

TEST_F(CachingDataSourceTests, OpenOrCreate_WSG2eBPluginRepository_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://viltest2-7.bentley.com/ws22";
    Utf8String dataSourceId = "Bentley.eB--viltest2-5.bentley.com~2CeB_Mobile";
    Credentials creds("admin", "admin");
    BeFileName cachePath(L":memory:");

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, dataSourceId, StubClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto ds = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_FALSE(nullptr == ds);
    }

TEST_F(CachingDataSourceTests, OpenOrCreate_WSG2PWPluginRepository_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://viltest2-8.bentley.com/ws23";
    Utf8String dataSourceId = "Bentley.PW--VILTEST2-5.bentley.com~3APW_Mobile_SS3";
    Credentials creds("admin", "admin");
    BeFileName cachePath(L":memory:");

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, dataSourceId, StubClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto ds = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_FALSE(nullptr == ds);
    }

TEST_F(CachingDataSourceTests, OpenOrCreate_WSG2PWPluginMapMobileRepository_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://geo-demo-ws.bentley.com/ws";
    Utf8String dataSourceId = "Bentley.PW--geo-demo.bentley.com~3AGeo-Demo";
    Credentials creds("twc", "admin");
    BeFileName cachePath(L":memory:");

    auto info = std::make_shared<ClientInfo>("Bentley-MapMobile", BeVersion(5,4), "77def89a-7e50-4f0e-a4c7-24fb6044dbfb", 
        "CLQIqB7y8eCUpdJe5uyRVVaaGbk=", "Windows 6.1", nullptr);

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, dataSourceId, StubClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto ds = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_FALSE(nullptr == ds);
    }

TEST_F(CachingDataSourceTests, OpenOrCreate_WSG2SPPluginRepository_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://viltest2-8.bentley.com/ws23";
    Utf8String dataSourceId = "Bentley.SP--http~3A~2F~2Fviltest2-10";
    Credentials creds(R"(.\administrator)", "Q!w2e3r4");
    BeFileName cachePath(L":memory:");

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, dataSourceId, StubClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto ds = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_FALSE(nullptr == ds);
    }

TEST_F(CachingDataSourceTests, DISABLED_SyncLocalChanges_PWPluginRepository_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://bsw-construct.bentley.com/ws";
    Utf8String dataSourceId = "pw--PW";
    Credentials creds("admin", "admin");
    BeFileName cachePath(L":memory:");

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, dataSourceId, StubClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto ds = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_FALSE(nullptr == ds);
    BentleyStatus status;

    ObjectId uploadsId {"PW_WSG", "project", "acd8be01-f097-461d-837a-63afeea9a8ea"};
    ObjectId uploadsNavNodeId {"Navigation", "NavNode", "ECObjects--" + uploadsId.schemaName + "-" + uploadsId.className + "-" + uploadsId.remoteId};

    {
    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, uploadsId));
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, uploadsNavNodeId));
    txn.Commit();
    }

    auto navigationResult = ds->GetNavigationChildren(uploadsNavNodeId, CachingDataSource::DataOrigin::RemoteData, nullptr)->GetResult();
    ASSERT_TRUE(navigationResult.IsSuccess());

    {
    auto txn = ds->StartCacheTransaction();
    auto& uploadedInstanceNavNodes = navigationResult.GetValue().GetJson();
    for (auto& uploadedInstanceNavNode : uploadedInstanceNavNodes)
        {
        if (uploadedInstanceNavNode.asBool())
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
        }
    txn.Commit();
    }

    Json::Value newDocument = ToJson(
        R"({
        "name": "TestCreation",
        "create_time" : "2014-06-03T08:59:58.673Z"
        })");

    {
    auto txn = ds->StartCacheTransaction();
    auto ecClass = txn.GetCache().GetAdapter().GetECClass("PW_WSG", "document");
    auto ecRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("PW_WSG", "DocumentParent");

    auto documentKey = txn.GetCache().GetChangeManager().CreateObject(*ecClass, newDocument);

    ASSERT_EQ(SUCCESS, txn.GetCache().GetChangeManager().ModifyFile(documentKey, StubFile("TestContent", "'Foo file'.txt"), false));
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot("NewDocument", txn.GetCache().FindInstance(documentKey)));

    auto folderKey = txn.GetCache().FindInstance(uploadsId);
    auto relationshipKey = txn.GetCache().GetChangeManager().CreateRelationship(*ecRelClass, documentKey, folderKey);
    ASSERT_TRUE(relationshipKey.IsValid());
    txn.Commit();
    }

    Json::Value newProject = ToJson(
        R"({
        "name": "TestCreatedFolder",
        "create_time" : "2014-06-03T08:59:58.673Z"
        })");

    {
    auto txn = ds->StartCacheTransaction();
    auto ecClass = txn.GetCache().GetAdapter().GetECClass("PW_WSG", "project");
    auto ecRelClass = txn.GetCache().GetAdapter().GetECRelationshipClass("PW_WSG", "ProjectParent");

    auto projectKey = txn.GetCache().GetChangeManager().CreateObject(*ecClass, newProject);
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot("NewProject", txn.GetCache().FindInstance(projectKey)));
    auto folderKey = txn.GetCache().FindInstance(uploadsId);
    auto relationshipKey = txn.GetCache().GetChangeManager().CreateRelationship(*ecRelClass, projectKey, folderKey);
    ASSERT_TRUE(relationshipKey.IsValid());
    txn.Commit();
    }

    auto syncResult = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_TRUE(syncResult.IsSuccess());

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

// FIXME: currently WSG fails - "Error freeing document 'TestCreationModified name'."
//newInstance["name"] = "TestCreationModified name";
//status = txn.GetCache ().GetChangeManager ().ModifyObject (newInstanceId, newInstance);
//ASSERT_EQ (SUCCESS, status);

// FIXME: currently WSG fails - "Error modifying folder."
//newInstances.clear ();
//status = txn.GetCache ().ReadInstancesLinkedToRoot (newInstances, "NewProject");
//ASSERT_EQ (SUCCESS, status);
//ASSERT_EQ (1, newInstances.size ());
//newInstance = newInstances[0];

//newInstanceId = txn.GetCache ().ObjectIdFromJsonInstance (newInstance);
//newInstance["name"] = "TestModifiedName";

//status = txn.GetCache ().GetChangeManager ().ModifyObject (newInstanceId, newInstance);
//ASSERT_EQ (SUCCESS, status);

    syncResult = ds->SyncLocalChanges(nullptr, nullptr)->GetResult();
    ASSERT_TRUE(syncResult.IsSuccess());
    }

TEST_F(CachingDataSourceTests, DISABLED_SyncLocalChanges_WSG2PWPluginRepository_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://bsw-construct2.bentley.com/ws22";
    Utf8String dataSourceId = "Bentley.PW--BSW-CONSTRUCT.bentley.com~3APW";
    Credentials creds("admin", "admin");
    BeFileName cachePath(L":memory:");

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, dataSourceId, StubClientInfo(), nullptr, proxy);
    client->SetCredentials(creds);

    auto ds = CachingDataSource::OpenOrCreate(client, cachePath, StubCacheEnvironemnt())->GetResult().GetValue();
    ASSERT_FALSE(nullptr == ds);
    BentleyStatus status;

    ObjectId uploadsFolderId {"PW_WSG", "Project", "acd8be01-f097-461d-837a-63afeea9a8ea"};
    ObjectId uploadsFolderNavNodeId {"Navigation", "NavNode", "ECObjects--" + uploadsFolderId.schemaName + "-" + uploadsFolderId.className + "-" + uploadsFolderId.remoteId};

    {
    auto txn = ds->StartCacheTransaction();
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, uploadsFolderId));
    ASSERT_EQ(SUCCESS, txn.GetCache().LinkInstanceToRoot(nullptr, uploadsFolderNavNodeId));
    txn.Commit();
    }

    auto navigationResult = ds->GetNavigationChildren(uploadsFolderNavNodeId, CachingDataSource::DataOrigin::RemoteData, nullptr)->GetResult();
    ASSERT_TRUE(navigationResult.IsSuccess());

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

TEST_F(CachingDataSourceTests, ECDbPrepareStatement_ChangesMadeInBetweenReuses_FindsChanges)
    {
    auto schemaPath = GetTestsAssetsDir();
    schemaPath.AppendToPath(LR"(\ECSchemas\WSClient\Cache\DSCacheSchema.01.05.ecschema.xml)");

    // Setup ECDb
    ECDb db;
    ASSERT_EQ(BE_SQLITE_OK, db.CreateNewDb(":memory:"));

    // Setup Schema
    auto context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlFile(schema, schemaPath, *context));
    ASSERT_EQ(SUCCESS, db.Schemas().ImportECSchemas(context->GetCache()));

    ECClassCP rootClass = db.GetClassLocater().LocateClass("DSCacheSchema", "Root");
    ASSERT_NE(nullptr, rootClass);

    // Names
    Utf8String rootName = "Foo";

    // Test quety for same instance
    Utf8String ecsql = "SELECT ECInstanceId FROM [DSC].[Root] WHERE [Name] = ? LIMIT 1 ";
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(db, ecsql.c_str()));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, rootName.c_str(), IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(BE_SQLITE_DONE, statement.Step());

    // Insert one instnace
    Json::Value rootInstance;
    rootInstance["Name"] = rootName;
    rootInstance["Persistence"] = 0;

    JsonInserter inserter(db, *rootClass);
    ASSERT_EQ(SUCCESS, inserter.Insert(rootInstance));

    // Try again
    statement.Reset();
    statement.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, statement.BindText(1, rootName.c_str(), IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
    EXPECT_EQ(ECInstanceId(1), statement.GetValueId <ECInstanceId>(0));
    }

TEST_F(CachingDataSourceTests, GetObjects_WSG2PWSpatialQuery_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Utf8String serverUrl = "https://viltest2-8.bentley.com/ws23";
    Utf8String dataSourceId = "Bentley.PW--VILTEST2-5.bentley.com~3APW_Mobile_SS3";
    Credentials creds("admin", "admin");
    BeFileName cachePath(L":memory:");

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, dataSourceId, StubClientInfo(), nullptr, proxy);
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

TEST_F(CachingDataSourceTests, DISABLED_OpenOrCreate_WSG2eBPluginProductionRepositoryAndECDbAdapterFiledRelationshipClassMethods_MeasurePerformance)
    {
    // Test can be copied to app to test performance on device.

    auto m_clientInfo = StubClientInfo();
    BeFileName tempDir = DgnClientFxCommon::GetApplicationPaths().GetTemporaryDirectory();
    BeFileName cachePath(L"C:\\temp\\temp.ecdb");
    //BeFileName cachePath(tempDir + L"/temp.ecdb");

    Utf8String serverUrl = "https://crossrail-dev.bentley.com/ws";
    Utf8String dataSourceId = "Bentley.eB--wazdevcrapp01~2CCrossRailDev";
    Credentials creds("bentleyadmin", "bentleyadmin");

    IWSRepositoryClientPtr client = WSRepositoryClient::Create(serverUrl, dataSourceId, m_clientInfo);
    client->SetCredentials(creds);

    CacheEnvironment env;
    env.persistentFileCacheDir = tempDir;
    env.temporaryFileCacheDir = tempDir;

    auto ds = CachingDataSource::OpenOrCreate(client, cachePath, env)->GetResult().GetValue();
    BeAssert(nullptr != ds);

    ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        auto txn = ds->StartCacheTransaction();

        uint64_t a, b;
        bvector<ECRelationshipClassCP> rels;

        ECClassCP aClass, bClass = nullptr;

        auto setSchemas = [&]
            {
            aClass = txn.GetCache().GetAdapter().GetECClass("eB_Dynamic.Global_Projects_CON");
            bClass = txn.GetCache().GetAdapter().GetECClass("eB_Dynamic.Global_Events_OBS");
            };

        auto callMultipleTimes = [&] (std::function<void()> f)
            {
            setSchemas();
            f();
            f();
            };

        a = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        ECSchemaList eschemas;
        txn.GetCache().GetECDb().Schemas().GetECSchemas(eschemas);
        b = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        BeDebugLog(Utf8PrintfString("GetECSchemas  took:%lld ms rels:%d", b - a));

        //txn.GetCache().GetECDb().ClearCache();

        callMultipleTimes([&]
            {
            a = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
            rels = txn.GetCache().GetAdapter().FindRelationshipClasses(aClass->GetId(), bClass->GetId());
            b = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
            BeDebugLog(Utf8PrintfString("ECDbAdapter:FindRelationshipClasses  took:%lld ms rels:%d", b - a, rels.size()));
            });

        //txn.GetCache().GetECDb().ClearCache();

        callMultipleTimes([&]
            {
            a = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
            rels = txn.GetCache().GetAdapter().FindRelationshipClassesInSchema(aClass->GetId(), bClass->GetId(), "eB_Dynamic");
            b = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
            BeDebugLog(Utf8PrintfString("ECDbAdapter:FindRelationshipClassesInSchema  took:%lld ms rels:%d", b - a, rels.size()));
            });

        //txn.GetCache().GetECDb().ClearCache();

        callMultipleTimes([&]
            {
            a = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
            rels = txn.GetCache().GetAdapter().FindRelationshipClassesWithSource(aClass->GetId(), "eB_Dynamic");
            b = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
            BeDebugLog(Utf8PrintfString("ECDbAdapter:FindRelationshipClassesWithSource  took:%lld ms rels:%d", b - a, rels.size()));
            });

        for (auto rel : rels)
            {
            BeDebugLog(Utf8PrintfString("ECDbAdapter:FindRelationshipClassesWithSource %s:%s",
                Utf8String(rel->GetSchema().GetName()).c_str(),
                Utf8String(rel->GetName()).c_str()));
            }

        //txn.GetCache().GetECDb().ClearCache();

        callMultipleTimes([&]
            {
            a = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
            txn.GetCache().GetAdapter().FindRelationshipClassWithSource(aClass->GetId(), bClass->GetId());
            b = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
            BeDebugLog(Utf8PrintfString("ECDbAdapter:FindRelationshipClassWithSource  took:%lld ms", b - a));
            });

        //txn.GetCache().GetECDb().ClearCache();

        callMultipleTimes([&]
            {
            a = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
            txn.GetCache().GetAdapter().FindRelationshipClassWithTarget(aClass->GetId(), bClass->GetId());
            b = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
            BeDebugLog(Utf8PrintfString("ECDbAdapter:FindRelationshipClassWithTarget  took:%lld ms", b - a));
            });
        })->Wait();
    }

TEST_F(CachingDataSourceTests, GetObjects_PunchlistQueries_Succeeds)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    Connect::Initialize(StubClientInfo(), proxy);

    Utf8String serverUrl = "https://qa-punchlist-eus.cloudapp.net";
    Utf8String dataSourceId = "IssuePlugin--default";
    Credentials credentials("bcc_user4@mailinator.com", "member#4");
    BeFileName cachePath(L":memory:");

    cachePath = BeFileName(StubFilePath("punchlistcache.ecdb"));
    DataSourceCache::DeleteCacheFromDisk(cachePath, StubCacheEnvironemnt());

    StubLocalState localState;
    ConnectAuthenticationPersistence::CustomInitialize(&localState);
    auto persistence = ConnectAuthenticationPersistence::GetShared();
    auto provider = std::make_shared<ConnectTokenProvider>(persistence);
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(serverUrl, provider, proxy);
    persistence->SetCredentials(credentials);

    auto client = WSRepositoryClient::Create(serverUrl, dataSourceId, StubClientInfo(), nullptr, authHandler);

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
            properties["FileName"] = Utf8PrintfString("Test-%d-%s.txt", i, BeGuid().ToString().c_str()) ;
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
