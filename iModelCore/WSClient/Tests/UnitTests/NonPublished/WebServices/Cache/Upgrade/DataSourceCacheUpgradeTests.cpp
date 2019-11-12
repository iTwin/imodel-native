/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

//--------------------------------------------------------------------------------------+
// Adding DataSourceCacheUpgradeTests test case:
// 1. Write SetupVx test that creates version x cache test against. See SetupV7, SetupV8.
// 2. Zip files into:
//      Tests\Published\WebServices\Cache\Upgrade\UpgradeSeeds\x.zip
//    Zip should contain:
//      Named folders with cache.ecdb file and "persistent", "temporary" folders inside. See existing xx.zip files.
// 3. Add new x.zip delivery/extraction into TestAssetsDeliver.mke
//    You will need to update test assets in build output with this:
//    bb -r WSClient -f WSClient -p Tests re WSClientTestAssets
// 4. Write Open_Vx... test case to test upgrade. 
// 5. Use GetSeedPaths() to get paths to extracted files.
//--------------------------------------------------------------------------------------+

//--------------------------------------------------------------------------------------+
// KNOWN ISSUES:
// V9 and lower - Graphite05x ECDb upgrade is not yet supported or will not be.
// V20 - DgnDb0601 - Q4->BIM0200 ECDb upgrade not supported.
//--------------------------------------------------------------------------------------+

#include "../../../Utils/WebServicesTestsHelper.h"
#include "../BaseCachingDataSourceTest.h"
#include <WebServices/Cache/ICachingDataSource.h>
#include <Bentley/BeDebugLog.h>
#include <WebServices/Cache/Persistence/DataSourceCache.h>
#include <WebServices/Cache/CachingDataSource.h>
#include "../../Client/MockWSRepositoryClient.h"
#include "../CachingTestsHelper.h"

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

#ifdef USE_GTEST
struct DataSourceCacheUpgradeTests : BaseCachingDataSourceTest
    {
    void ValidateV5SeedData(ICachingDataSourcePtr ds, BeFileNameCR path, CacheEnvironmentCR environment);
    void ValidateV5SeedData(IDataSourceCache& cache, BeFileNameCR path, CacheEnvironmentCR environment, bool checkETags = true);
    virtual void SetUp();
    };

void DataSourceCacheUpgradeTests::SetUp()
    {
    BaseCachingDataSourceTest::SetUp();

    BeFileName assetsSeedPath(GetTestsAssetsDir() + L"WSClientTestAssets\\Cache\\UpgradeSeeds\\");
    BeFileName targetSeedPath(GetTestsTempDir() + L"DataSourceCacheUpgradeTests\\UpgradeSeeds\\");

    if (targetSeedPath.DoesPathExist())
        {
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::EmptyAndRemoveDirectory(targetSeedPath));
        }

    EXPECT_EQ(BeFileNameStatus::Success, BeFileName::CloneDirectory(assetsSeedPath, targetSeedPath, true));
    }

BeFileName GetSeedDir(int version, Utf8StringCR subdir, BeFileName basedir)
    {
    BeFileName path =
        basedir
        .AppendToPath(L"DataSourceCacheUpgradeTests")
        .AppendToPath(L"UpgradeSeeds")
        .AppendToPath(WPrintfString(L"%d", version).c_str())
        .AppendToPath(BeFileName(subdir));

    return path;
    }

bpair<BeFileName, CacheEnvironment> GetSeedPaths(int version, Utf8StringCR subdir, BeFileName basedir)
    {
    BeFileName path =
        GetSeedDir(version, subdir, basedir)
        .AppendToPath(L"cache.ecdb");


    BeFileName persistent = GetSeedDir(version, subdir, basedir).AppendToPath(L"persistent");
    BeFileName temporary = GetSeedDir(version, subdir, basedir).AppendToPath(L"temporary");

    CacheEnvironment environment(persistent, temporary);

    return {path, environment};
    }

bpair<BeFileName, CacheEnvironment> GetSeedPaths(int version, Utf8StringCR subdir)
    {
    auto paths = GetSeedPaths(version, subdir, GetTestsTempDir());

    if (!paths.first.DoesPathExist())
        {
        EXPECT_TRUE(false);
        }

    return paths;
    }

bpair<BeFileName, CacheEnvironment> GetNewSeedPaths(int version, Utf8StringCR subdir, BeFileName basedir = BeFileName(LR"(C:\temp-wsclient\)"))
    {
    BeFileName::EmptyAndRemoveDirectory(basedir);
    auto paths = GetSeedPaths(version, subdir, basedir);

    BeDebugLog("Got new seed paths:");
    BeDebugLog(Utf8String(paths.first).c_str());
    BeDebugLog(Utf8String(paths.second.persistentFileCacheDir).c_str());
    BeDebugLog(Utf8String(paths.second.temporaryFileCacheDir).c_str());
    BeDebugLog(Utf8String(paths.second.externalFileCacheDir).c_str());

    return paths;
    }

BeFileName GetSeedFilePath(BeFileName cachePath, Utf8StringCR fileName)
    {
    BeFileName path = cachePath.GetDirectoryName();
    return path.AppendToPath(BeFileName(fileName));
    }

ECSchemaPtr GetLegacySchema()
    {
    Utf8String schemaXml =
        R"xml(<ECSchema schemaName="LEGACY_SCHEMA" nameSpacePrefix="LS" version="1.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *ECSchemaReadContext::CreateContext());
    return schema;
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_V5Empty_Success_KnownIssue_UpgradeNotSupported)
    {
    auto paths = GetSeedPaths(5, "empty");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_V5Data_SuccessAndContainsOldData_KnownIssue_UpgradeNotSupported)
    {
    auto paths = GetSeedPaths(5, "data");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    ValidateV5SeedData(cache, paths.first, paths.second);
    EXPECT_FALSE(HasFailure());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_V5DataUpgradeInterrupted1BeforeSettingNewCacheUpgradedFlag_SuccessAndContainsOldData_KnownIssue_UpgradeNotSupported)
    {
    // *-upgrade-new is finalized but has not set upgraded flag - should upgrade again
    auto paths = GetSeedPaths(5, "interrupted1");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    ValidateV5SeedData(cache, paths.first, paths.second);
    EXPECT_FALSE(HasFailure());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_V5DataUpgradeInterrupted2AfterSettingNewCacheUpgradedFlag_SuccessAndContainsOldData_KnownIssue_UpgradeNotSupported)
    {
    // *-upgrade-new is finalized and has set upgraded flag - should copy file and return
    auto paths = GetSeedPaths(5, "interrupted2");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    ValidateV5SeedData(cache, paths.first, paths.second, false); // false, because V12->13 removes tags
    EXPECT_FALSE(HasFailure());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, CachingDataSource_OpenOrCreate_V5Data_ServerInfoSetDoDefaultAndSchemasAreValid_KnownIssue_UpgradeNotSupported)
    {
    auto paths = GetSeedPaths(5, "data");
    auto client = MockWSRepositoryClient::Create();

    StubInstances schemaDefs;
    schemaDefs.Add({"MetaSchema.ECSchemaDef", "LEGACY_SCHEMA"}, {{"Name", "LEGACY_SCHEMA"}, {"VersionMajor", 1}, {"VersionMinor", 0}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi(BeVersion(1, 1))))));

    EXPECT_CALL(*client, GetInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSRepositoryResult::Success(StubWSRepository()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemaDefs.ToWSObjectsResponseNotModified()))));

    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "LEGACY_SCHEMA"), An<BeFileNameCR>(), _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        GetLegacySchema()->WriteToXmlFile(filePath);
        return CreateCompletedAsyncTask(StubWSFileResult(filePath));
        }));

    auto result = CachingDataSource::OpenOrCreate(client, paths.first, paths.second)->GetResult();
    ASSERT_THAT(result.IsSuccess(), true);
    auto ds = result.GetValue();

    ValidateV5SeedData(ds, paths.first, paths.second);
    EXPECT_FALSE(HasFailure());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, CachingDataSource_OpenOrCreate_V5UpgradeInterrupted1_SuccessAndContainsOldData_KnownIssue_UpgradeNotSupported)
    {
    auto paths = GetSeedPaths(5, "interrupted1");
    auto client = MockWSRepositoryClient::Create();

    StubInstances schemaDefs;
    schemaDefs.Add({"MetaSchema.ECSchemaDef", "LEGACY_SCHEMA"}, {{"Name", "LEGACY_SCHEMA"}, {"VersionMajor", 1}, {"VersionMinor", 0}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi(BeVersion(1, 1))))));

    EXPECT_CALL(*client, GetInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSRepositoryResult::Success(StubWSRepository()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemaDefs.ToWSObjectsResponseNotModified()))));

    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "LEGACY_SCHEMA"), An<BeFileNameCR>(), _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        GetLegacySchema()->WriteToXmlFile(filePath);
        return CreateCompletedAsyncTask(StubWSFileResult(filePath));
        }));

    auto result = CachingDataSource::OpenOrCreate(client, paths.first, paths.second)->GetResult();
    ASSERT_THAT(result.IsSuccess(), true);

    ValidateV5SeedData(result.GetValue(), paths.first, paths.second);
    EXPECT_FALSE(HasFailure());
    }

void DataSourceCacheUpgradeTests::ValidateV5SeedData(ICachingDataSourcePtr ds, BeFileNameCR path, CacheEnvironmentCR environment)
    {
    // CachingDataSource
    auto txn = ds->StartCacheTransaction();
    auto info = ds->GetServerInfo(txn);

    EXPECT_THAT(info.IsValid(), true);
    EXPECT_THAT(info.GetVersion(), BeVersion(1, 0));
    EXPECT_THAT(info.GetWebApiVersion(), BeVersion(1, 1));
    EXPECT_THAT(info.GetType(), WSInfo::Type::BentleyWSG);

    auto schemas = ds->GetRepositorySchemas(txn);

    ASSERT_THAT(schemas, SizeIs(1));
    EXPECT_TRUE(0 == strcmp(schemas[0]->GetName().c_str(), "TestSchema"));

    // DataSourceCache
    ValidateV5SeedData(txn.GetCache(), path, environment);
    }

void DataSourceCacheUpgradeTests::ValidateV5SeedData(IDataSourceCache& cache, BeFileNameCR path, CacheEnvironmentCR environment, bool checkETags)
    {
    // FILE
    EXPECT_THAT(BeFileName(cache.GetAdapter().GetECDb().GetDbFileName()), Eq(path));

    // SCHEMA
    CachedResponseKey schemasResponseKey {cache.FindOrCreateRoot(nullptr), "CachingDataSource.Schemas"};
    EXPECT_THAT(cache.IsResponseCached(schemasResponseKey), true);
    EXPECT_THAT(cache.ReadResponseCacheTag(schemasResponseKey), Eq(""));

    Json::Value schemaDefinitions;
    EXPECT_EQ(CacheStatus::OK, cache.ReadResponse(schemasResponseKey, schemaDefinitions));

    EXPECT_THAT(schemaDefinitions.size(), 1);
    EXPECT_THAT(schemaDefinitions[0]["Name"], Eq("TestSchema"));
    EXPECT_THAT(schemaDefinitions[0]["NameSpacePrefix"], Eq("TS"));
    EXPECT_THAT(schemaDefinitions[0]["VersionMajor"], Eq(1));
    EXPECT_THAT(schemaDefinitions[0]["VersionMinor"], Eq(0));

    ECClassCP testClass = cache.GetAdapter().GetECClass("TestSchema.TestClass");
    ASSERT_THAT(testClass, NotNull());
    EXPECT_THAT(testClass->GetPropertyCount(), 1);
    EXPECT_THAT(testClass->GetPropertyP("TestProperty"), NotNull());

    // ROOTS
    EXPECT_THAT(cache.IsInstanceInRoot("TemporaryRoot", cache.FindInstance(ObjectId())), true);
    EXPECT_THAT(cache.IsInstanceInRoot("TemporaryRoot", cache.FindInstance({"TestSchema.TestClass", "InstanceInTemporaryRoot"})), true);
    EXPECT_THAT(cache.IsInstanceInRoot("PersistedRoot", cache.FindInstance({"TestSchema.TestClass", "InstanceInFullyPersistedRoot"})), true);

    EXPECT_THAT(cache.IsInstanceFullyPersisted(cache.FindInstance(ObjectId())), false);
    EXPECT_THAT(cache.IsInstanceFullyPersisted(cache.FindInstance({"TestSchema.TestClass", "InstanceInTemporaryRoot"})), false);
    EXPECT_THAT(cache.IsInstanceFullyPersisted(cache.FindInstance({"TestSchema.TestClass", "InstanceInFullyPersistedRoot"})), true);

    // INSTANCES
    EXPECT_THAT(cache.FindInstance(ObjectId()).IsValid(), true);

    Json::Value instanceJson;
    cache.ReadInstance({"TestSchema.TestClass", "InstanceWithProperties"}, instanceJson);
    EXPECT_THAT(instanceJson["TestProperty"], Eq("TestValue"));
    EXPECT_THAT(cache.ReadInstanceCacheTag({"TestSchema.TestClass", "InstanceWithProperties"}), Eq("InstanceTag"));

    // HIERARCHY
    CachedResponseKey responseKey1 {cache.FindInstance(ObjectId()), "CachingDataSource.Navigation"};
    CachedResponseKey responseKey2 {cache.FindInstance({"TestSchema.TestClass", "RootInstance1"}), "CachingDataSource.Navigation"};
    CachedResponseKey responseKey3 {cache.FindInstance({"TestSchema.TestClass", "RootInstance2"}), "CachingDataSource.Navigation"};
    CachedResponseKey responseKey4 {cache.FindInstance({"TestSchema.TestClass", "RootInstance3"}), "CachingDataSource.Navigation"};
    EXPECT_THAT(cache.IsResponseCached(responseKey1), true);
    EXPECT_THAT(cache.IsResponseCached(responseKey2), true);
    EXPECT_THAT(cache.IsResponseCached(responseKey3), true);
    EXPECT_THAT(cache.IsResponseCached(responseKey4), false);
    if (checkETags)
        {
        EXPECT_THAT(cache.ReadResponseCacheTag(responseKey1), Eq("Tag1"));
        EXPECT_THAT(cache.ReadResponseCacheTag(responseKey2), Eq("Tag2"));
        EXPECT_THAT(cache.ReadResponseCacheTag(responseKey3), Eq("Tag3"));
        EXPECT_THAT(cache.ReadResponseCacheTag(responseKey4), Eq(""));
        }

    bset<ObjectId> ids;
    ASSERT_EQ(CacheStatus::OK, cache.ReadResponseObjectIds(responseKey1, ids));

    EXPECT_THAT(ids, SizeIs(3));
    EXPECT_THAT(ids, Contains(ObjectId("TestSchema.TestClass", "RootInstance1")));
    EXPECT_THAT(ids, Contains(ObjectId("TestSchema.TestClass", "RootInstance2")));
    EXPECT_THAT(ids, Contains(ObjectId("TestSchema.TestClass", "RootInstance3")));

    ids.clear();
    ASSERT_EQ(CacheStatus::OK, cache.ReadResponseObjectIds(responseKey2, ids));

    EXPECT_THAT(ids, SizeIs(2));
    EXPECT_THAT(ids, Contains(ObjectId("TestSchema.TestClass", "Child1")));
    EXPECT_THAT(ids, Contains(ObjectId("TestSchema.TestClass", "Child2")));

    ids.clear();
    ASSERT_EQ(CacheStatus::OK, cache.ReadResponseObjectIds(responseKey3, ids));

    EXPECT_THAT(ids, SizeIs(1));
    EXPECT_THAT(ids, Contains(ObjectId("TestSchema.TestClass", "Parent")));

    // FILES
    auto filePath1 = cache.ReadFilePath({"TestSchema.TestClass", "Child1"});
    auto filePath2 = cache.ReadFilePath({"TestSchema.TestClass", "Child2"});
    EXPECT_THAT(filePath1, Not(IsEmpty()));
    EXPECT_THAT(filePath2, Not(IsEmpty()));
    EXPECT_THAT(filePath1.GetFileNameAndExtension(), Eq(L"TestFile1.txt"));
    EXPECT_THAT(filePath2.GetFileNameAndExtension(), Eq(L"TestFile2.txt"));
    EXPECT_THAT(SimpleReadFile(filePath1), Eq("CachedFile1"));
    EXPECT_THAT(SimpleReadFile(filePath2), Eq("CachedFile2"));

    filePath1.PopDir();
    filePath1.PopDir();
    filePath1.PopDir();

    filePath2.PopDir();
    filePath2.PopDir();
    filePath2.PopDir();

    EXPECT_THAT(filePath1.c_str(), EndsWith(L"persistent"));
    EXPECT_THAT(filePath2.c_str(), EndsWith(L"temporary"));
    EXPECT_THAT(cache.GetFileCacheLocation({"TestSchema.TestClass", "Child1"}), FileCache::Persistent);
    EXPECT_THAT(cache.GetFileCacheLocation({"TestSchema.TestClass", "Child2"}), FileCache::Temporary);
    EXPECT_THAT(cache.ReadFileCacheTag({"TestSchema.TestClass", "Child1"}), Eq("FileTag1"));
    EXPECT_THAT(cache.ReadFileCacheTag({"TestSchema.TestClass", "Child2"}), Eq("FileTag2"));

    // CRUD
    auto findInstanceByPropertyValue = [&] (Utf8StringCR testPropertyValue)
        {
        ECSqlStatement statement;
        Utf8String ecsql = "SELECT ECClassId, ECInstanceId FROM TS.TestClass WHERE TestProperty=?";
        EXPECT_EQ(SUCCESS, cache.GetAdapter().PrepareStatement(statement, ecsql));
        EXPECT_EQ(ECSqlStatus::Success, statement.BindText(1, testPropertyValue.c_str(), IECSqlBinder::MakeCopy::No));
        EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
        return ECInstanceKey(statement.GetValueId<ECClassId>(0), statement.GetValueId<ECInstanceId>(1));
        };

    auto newInstanceKey1 = findInstanceByPropertyValue("NewValue1");
    auto newInstanceKey2 = findInstanceByPropertyValue("NewValue2");
    auto newInstanceKey3 = findInstanceByPropertyValue("NewValue3");
    auto newInstanceKey4 = findInstanceByPropertyValue("NewValue4");

    EXPECT_THAT(newInstanceKey1.IsValid(), true);
    EXPECT_THAT(newInstanceKey2.IsValid(), true);
    EXPECT_THAT(newInstanceKey3.IsValid(), true);
    EXPECT_THAT(newInstanceKey4.IsValid(), true);

    EXPECT_THAT(cache.GetChangeManager().GetObjectChangeStatus(newInstanceKey1), IChangeManager::ChangeStatus::Created);
    EXPECT_THAT(cache.GetChangeManager().GetObjectChangeStatus(newInstanceKey2), IChangeManager::ChangeStatus::Created);
    EXPECT_THAT(cache.GetChangeManager().GetObjectChangeStatus(newInstanceKey3), IChangeManager::ChangeStatus::Created);
    EXPECT_THAT(cache.GetChangeManager().GetObjectChangeStatus(newInstanceKey4), IChangeManager::ChangeStatus::Created);
    EXPECT_THAT(cache.GetChangeManager().GetObjectSyncStatus(newInstanceKey1), IChangeManager::SyncStatus::Ready);
    EXPECT_THAT(cache.GetChangeManager().GetObjectSyncStatus(newInstanceKey2), IChangeManager::SyncStatus::Ready);
    EXPECT_THAT(cache.GetChangeManager().GetObjectSyncStatus(newInstanceKey3), IChangeManager::SyncStatus::Ready);
    EXPECT_THAT(cache.GetChangeManager().GetObjectSyncStatus(newInstanceKey4), IChangeManager::SyncStatus::NotReady);
    EXPECT_THAT(cache.GetChangeManager().GetFileChange(newInstanceKey1).GetChangeStatus(), IChangeManager::ChangeStatus::NoChange);
    EXPECT_THAT(cache.GetChangeManager().GetFileChange(newInstanceKey2).GetChangeStatus(), IChangeManager::ChangeStatus::Modified);
    EXPECT_THAT(cache.GetChangeManager().GetFileChange(newInstanceKey3).GetChangeStatus(), IChangeManager::ChangeStatus::NoChange);
    EXPECT_THAT(cache.GetChangeManager().GetFileChange(newInstanceKey4).GetChangeStatus(), IChangeManager::ChangeStatus::NoChange);

    auto newFilePath = cache.ReadFilePath(cache.FindInstance(newInstanceKey2));
    EXPECT_THAT(newFilePath, Not(IsEmpty()));
    EXPECT_THAT(SimpleReadFile(newFilePath), Eq("NewFile1"));
    EXPECT_THAT(newFilePath.GetFileNameAndExtension(), Eq(L"TestFile.txt"));

    EXPECT_THAT(cache.IsInstanceInRoot("NewInstanceRoot", newInstanceKey3), false);

    bvector<IChangeManager::RelationshipChange> changes;
    EXPECT_EQ(SUCCESS, cache.GetChangeManager().GetCreatedRelationships(newInstanceKey1, changes));
    ASSERT_THAT(changes.size(), 2);

    EXPECT_THAT(changes[1].GetSourceKey(), cache.FindInstance({"TestSchema.TestClass", "Parent"}));
    EXPECT_THAT(changes[1].GetTargetKey(), newInstanceKey1);

    EXPECT_THAT(changes[0].GetSourceKey(), newInstanceKey1);
    EXPECT_THAT(changes[0].GetTargetKey(), newInstanceKey2);

    EXPECT_TRUE(0 == strcmp(cache.GetAdapter().GetECClass(changes[0].GetInstanceKey())->GetName().c_str(), "LegacyParentRelationship"));
    EXPECT_TRUE(0 == strcmp(cache.GetAdapter().GetECClass(changes[1].GetInstanceKey())->GetName().c_str(), "LegacyParentRelationship"));

    changes.clear();
    EXPECT_EQ(SUCCESS, cache.GetChangeManager().GetCreatedRelationships(newInstanceKey2, changes));
    ASSERT_THAT(changes.size(), 1);
    EXPECT_THAT(changes[0].GetSourceKey(), newInstanceKey1);
    EXPECT_THAT(changes[0].GetTargetKey(), newInstanceKey2);
    EXPECT_TRUE(0 == strcmp(cache.GetAdapter().GetECClass(changes[0].GetInstanceKey())->GetName().c_str(), "LegacyParentRelationship"));

    changes.clear();
    EXPECT_EQ(SUCCESS, cache.GetChangeManager().GetCreatedRelationships(newInstanceKey3, changes));
    EXPECT_THAT(changes, IsEmpty());

    changes.clear();
    EXPECT_EQ(SUCCESS, cache.GetChangeManager().GetCreatedRelationships(newInstanceKey4, changes));
    EXPECT_THAT(changes, IsEmpty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_CurrentVersionDb_Success)
    {
    BeFileName path = StubFilePath();

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Create(path, StubCacheEnvironemnt()));
    ASSERT_EQ(BE_SQLITE_OK, cache.GetAdapter().GetECDb().SaveChanges());
    ASSERT_EQ(SUCCESS, cache.Close());

    EXPECT_EQ(SUCCESS, DataSourceCache().Open(path, CacheEnvironment()));
    }

// Left for referance
//TEST_F(DataSourceCacheUpgradeTests, DISABLED_SetupV7)
//    {
//    DataSourceCache cache;
//    ASSERT_EQ(SUCCESS, cache.Create(BeFileName("C:/t/data/cache.ecdb"), {BeFileName("C:/t/data/persistent"), BeFileName("C:/t/data/temporary")}));
//    ASSERT_EQ(SUCCESS, cache.UpdateSchemas(std::vector<ECSchemaPtr> {GetTestSchema()}));
//
//    StubInstances instances;
//    instances.Add({"TestSchema.TestClass", "Modified"}, {{"TestProperty", "OldValueA"}, {"TestProperty2", "OldValueB"}});
//    ASSERT_EQ(SUCCESS, cache.CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Modified"}, instances.ToWSObjectsResponse(), nullptr));
//
//    auto instance = cache.FindInstance({"TestSchema.TestClass", "Modified"});
//
//    Json::Value instanceJson;
//    instanceJson["TestProperty"] = "NewValueA";
//    instanceJson["TestProperty2"] = "OldValueB";
//
//    ASSERT_EQ(SUCCESS, cache.GetChangeManager().ModifyObject(instance, instanceJson));
//
//    cache.GetECDb().SaveChanges();
//    cache.Close();
//    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_V7ModifiedInstance_ReadModifiedPropertiesTreatsAllInstancePropertiesAsModified_KnownIssue_UpgradeNotSupported)
    {
    auto paths = GetSeedPaths(7, "data");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    Json::Value instanceJson;
    ASSERT_EQ(CacheStatus::OK, cache.ReadInstance({"TestSchema.TestClass", "Modified"}, instanceJson));
    EXPECT_EQ("NewValueA", instanceJson["TestProperty"].asString());
    EXPECT_EQ("OldValueB", instanceJson["TestProperty2"].asString());

    auto instance = cache.FindInstance({"TestSchema.TestClass", "Modified"});

    Json::Value changesJson;
    ASSERT_EQ(SUCCESS, cache.GetChangeManager().ReadModifiedProperties(instance, changesJson));

    Json::Value expected;
    expected["TestProperty"] = "NewValueA";
    expected["TestProperty2"] = "OldValueB";
    EXPECT_EQ(expected, changesJson);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_V7ModifiedInstanceModifiedAgain_ReadModifiedPropertiesTreatsAllInstancePropertiesAsModified_KnownIssue_UpgradeNotSupported)
    {
    auto paths = GetSeedPaths(7, "data");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    auto instance = cache.FindInstance({"TestSchema.TestClass", "Modified"});

    Json::Value instanceJson;
    instanceJson["TestProperty"] = "NewValueA";
    instanceJson["TestProperty2"] = "LatestValueB";
    ASSERT_EQ(SUCCESS, cache.GetChangeManager().ModifyObject(instance, instanceJson));

    Json::Value changesJson;
    ASSERT_EQ(SUCCESS, cache.GetChangeManager().ReadModifiedProperties(instance, changesJson));

    Json::Value expected;
    expected["TestProperty"] = "NewValueA";
    expected["TestProperty2"] = "LatestValueB";
    EXPECT_EQ(expected, changesJson);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_V7ModifiedPropertyModifiedAgain_ReadModifiedPropertiesTreatsAllInstancePropertiesAsModified_KnownIssue_UpgradeNotSupported)
    {
    auto paths = GetSeedPaths(7, "data");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    auto instance = cache.FindInstance({"TestSchema.TestClass", "Modified"});

    Json::Value instanceJson;
    instanceJson["TestProperty"] = "LatestValueA";
    instanceJson["TestProperty2"] = "OldValueB";
    ASSERT_EQ(SUCCESS, cache.GetChangeManager().ModifyObject(instance, instanceJson));

    Json::Value changesJson;
    ASSERT_EQ(SUCCESS, cache.GetChangeManager().ReadModifiedProperties(instance, changesJson));

    Json::Value expected;
    expected["TestProperty"] = "LatestValueA";
    expected["TestProperty2"] = "OldValueB";
    EXPECT_EQ(expected, changesJson);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
// Left for referance
//TEST_F(DataSourceCacheUpgradeTests, DISABLED_SetupV8)
//    {
//    DataSourceCache cache;
//    BeFileName::EmptyAndRemoveDirectory(L"C:/t/");
//    ASSERT_EQ(SUCCESS, cache.Create(BeFileName("C:/t/data/cache.ecdb"), {BeFileName("C:/t/data/persistent"), BeFileName("C:/t/data/temporary")}));
//    ASSERT_EQ(SUCCESS, cache.UpdateSchemas(std::vector<ECSchemaPtr> {GetTestSchema()}));
//
//    // Setup test data
//    StubInstances instances;
//    instances.Add({"TestSchema.TestClass", "A"});
//    instances.Add({"TestSchema.TestClass", "B"});
//    CachedResponseKey key(cache.FindOrCreateRoot(nullptr), nullptr);
//    ASSERT_EQ(SUCCESS, cache.CacheResponse(key, instances.ToWSObjectsResponse()));
//
//    // Create instance
//    auto ecClass = cache.GetAdapter().GetECClass("TestSchema.TestClass");
//    Json::Value properties;
//    properties["TestProperty"] = "NewValueA";
//    ECInstanceKey instance = cache.GetChangeManager().CreateObject(*ecClass, properties);
//    ASSERT_TRUE(instance.IsValid());
//
//    // Created relationship
//    auto ecRelClass = cache.GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
//    auto a = cache.FindInstance({"TestSchema.TestClass", "A"});
//    auto b = cache.FindInstance({"TestSchema.TestClass", "B"});
//    ECInstanceKey relationship = cache.GetChangeManager().CreateRelationship(*ecRelClass, a, b);
//    ASSERT_TRUE(relationship.IsValid());
//
//    // Save
//    cache.GetECDb().SaveChanges();
//    cache.Close();
//    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_V8CreatedObjectsAreDeleted_CommitLocalDeletionsRemovesThem_KnownIssue_UpgradeNotSupported)
    {
    // Arrange
    auto paths = GetSeedPaths(8, "data");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    IChangeManager::Changes changes;
    ASSERT_EQ(SUCCESS, cache.GetChangeManager().GetChanges(changes));

    ASSERT_EQ(1, changes.GetObjectChanges().size());
    ASSERT_EQ(1, changes.GetRelationshipChanges().size());
    ASSERT_EQ(0, changes.GetFileChanges().size());
    ASSERT_EQ(IChangeManager::ChangeStatus::Created, changes.GetObjectChanges().begin()->GetChangeStatus());
    ASSERT_EQ(IChangeManager::ChangeStatus::Created, changes.GetRelationshipChanges().begin()->GetChangeStatus());

    auto instance = changes.GetObjectChanges().begin()->GetInstanceKey();
    auto relationship = changes.GetRelationshipChanges().begin()->GetInstanceKey();

    ASSERT_EQ(SUCCESS, cache.GetChangeManager().DeleteObject(instance));
    ASSERT_EQ(SUCCESS, cache.GetChangeManager().DeleteRelationship(relationship));

    // Act
    ASSERT_EQ(SUCCESS, cache.GetChangeManager().CommitLocalDeletions());

    // Assert
    EXPECT_FALSE(cache.GetChangeManager().HasChanges());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache.GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache.GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    }

//// Left for referance
/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
//TEST_F(DataSourceCacheUpgradeTests, DISABLED_SetupV9)
//    {
//    DataSourceCache cache;
//    auto paths = GetNewSeedPaths(9, "data");
//    ASSERT_EQ(SUCCESS, cache.Create(paths.first, paths.second));
//    ASSERT_EQ(SUCCESS, cache.UpdateSchemas(std::vector<ECSchemaPtr> {GetTestSchema()}));
//
//    // Setup test data
//    StubInstances instances;
//    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
//    CachedResponseKey key1(cache.FindOrCreateRoot(nullptr), "ResponseA");
//    ASSERT_EQ(SUCCESS, cache.CacheResponse(key1, instances.ToWSObjectsResponse("ETagA")));
//    SimpleWriteToFile(cache.ReadResponseCachedDate(key1).ToString(), GetNewFilePath(paths.first, "CacheDateResponseA"));
//
//    instances.Clear();
//    instances.Add({"TestSchema.TestClass", "C"});
//    CachedResponseKey key2(cache.FindOrCreateRoot("Parent"), "ResponseB", cache.FindOrCreateRoot("Holder"));
//    ASSERT_EQ(SUCCESS, cache.CacheResponse(key2, instances.ToWSObjectsResponse()));
//
//    // Save
//    cache.GetECDb().SaveChanges();
//    cache.Close();
//    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_V9_ResponsesAreStillCached_KnownIssue_UpgradeNotSupported)
    {
    // Arrange
    auto paths = GetSeedPaths(9, "data");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    CachedResponseKey key1(cache.FindOrCreateRoot(nullptr), "ResponseA");
    CachedResponseKey key2(cache.FindOrCreateRoot("Parent"), "ResponseB", cache.FindOrCreateRoot("Holder"));

    // Check if cached
    EXPECT_TRUE(cache.IsResponseCached(key1));
    EXPECT_TRUE(cache.IsResponseCached(key2));

    // Check date
    Utf8String dateStr = SimpleReadFile(GetSeedFilePath(paths.first, "CacheDateResponseA"));
    EXPECT_FALSE(dateStr.empty());
    DateTime cachedDate;
    DateTime::FromString(cachedDate, dateStr.c_str());
    EXPECT_EQ(cachedDate, cache.ReadResponseCachedDate(key1));

    // Check data
    ECInstanceKeyMultiMap keys1, keys2;
    EXPECT_EQ(CacheStatus::OK, cache.ReadResponseInstanceKeys(key1, keys1));
    EXPECT_EQ(CacheStatus::OK, cache.ReadResponseInstanceKeys(key2, keys2));

    EXPECT_EQ(2, keys1.size());
    EXPECT_EQ(1, keys2.size());

    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache.FindInstance({"TestSchema.TestClass", "A"}), keys1));
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache.FindInstance({"TestSchema.TestClass", "B"}), keys1));
    EXPECT_TRUE(ECDbHelper::IsInstanceInMultiMap(cache.FindInstance({"TestSchema.TestClass", "C"}), keys2));

    auto relClass = cache.GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = cache.FindRelationship(*relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"});
    EXPECT_EQ(ObjectId("TestSchema.TestRelationshipClass", "AB"), cache.FindRelationship(relationship));

    // Deprecated data is removed
    EXPECT_EQ(0, CountClassInstances(cache, "DSCacheSchema.CachedResponseInfoToCachedRelationshipInfo"));
    EXPECT_EQ(0, CountClassInstances(cache, "DSCacheSchema.CachedResponseInfoToResultRelationship"));
    EXPECT_EQ(0, CountClassInstances(cache, "DSCacheSchema.CachedResponseInfoToResultWeakRelationship"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_V9RemovingResponses_InstancesAreRemoved_KnownIssue_UpgradeNotSupported)
    {
    // Arrange
    auto paths = GetSeedPaths(9, "data");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    CachedResponseKey key1(cache.FindOrCreateRoot(nullptr), "ResponseA");
    CachedResponseKey key2(cache.FindOrCreateRoot("Parent"), "ResponseB", cache.FindOrCreateRoot("Holder"));

    // Check if cached
    ASSERT_EQ(SUCCESS, cache.RemoveResponse(key1));
    ASSERT_EQ(SUCCESS, cache.RemoveResponse(key2));

    // Check tags
    EXPECT_EQ("", cache.ReadResponseCacheTag(key1));
    EXPECT_EQ("", cache.ReadResponseCacheTag(key2));

    // Check date
    EXPECT_FALSE(cache.ReadResponseCachedDate(key1).IsValid());
    EXPECT_FALSE(cache.ReadResponseCachedDate(key2).IsValid());

    // Check data
    ECInstanceKeyMultiMap keys;
    EXPECT_EQ(CacheStatus::DataNotCached, cache.ReadResponseInstanceKeys(key1, keys));
    EXPECT_EQ(0, keys.size());
    EXPECT_EQ(CacheStatus::DataNotCached, cache.ReadResponseInstanceKeys(key2, keys));
    EXPECT_EQ(0, keys.size());

    auto relClass = cache.GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    EXPECT_FALSE(cache.FindInstance({"TestSchema.TestClass", "A"}).IsValid());
    EXPECT_FALSE(cache.FindInstance({"TestSchema.TestClass", "B"}).IsValid());
    EXPECT_FALSE(cache.FindInstance({"TestSchema.TestClass", "C"}).IsValid());
    EXPECT_FALSE(cache.FindRelationship(*relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"}).IsValid());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_V9CachingNewPagedData_WorksFine_KnownIssue_UpgradeNotSupported)
    {
    // Arrange
    auto paths = GetSeedPaths(9, "data");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    CachedResponseKey key1(cache.FindOrCreateRoot(nullptr), "ResponseA");

    // Check if caches
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(CacheStatus::OK, cache.CacheResponse(key1, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 0));
    EXPECT_FALSE(cache.IsResponseCached(key1));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "C"});
    ASSERT_EQ(CacheStatus::OK, cache.CacheResponse(key1, instances.ToWSObjectsResponse("", "NotFinal"), nullptr, nullptr, 1));
    EXPECT_FALSE(cache.IsResponseCached(key1));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "D"});
    ASSERT_EQ(CacheStatus::OK, cache.CacheResponse(key1, instances.ToWSObjectsResponse("", ""), nullptr, nullptr, 2));
    EXPECT_TRUE(cache.IsResponseCached(key1));

    auto relClass = cache.GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    EXPECT_TRUE(cache.FindInstance({"TestSchema.TestClass", "A"}).IsValid());
    EXPECT_TRUE(cache.FindInstance({"TestSchema.TestClass", "B"}).IsValid());
    EXPECT_TRUE(cache.FindInstance({"TestSchema.TestClass", "C"}).IsValid());
    EXPECT_TRUE(cache.FindInstance({"TestSchema.TestClass", "D"}).IsValid());
    EXPECT_TRUE(cache.FindRelationship(*relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"}).IsValid());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
//// Left for referance
//TEST_F(DataSourceCacheUpgradeTests, SetupV10)
//    {
//    DataSourceCache cache;
//    auto paths = GetNewSeedPaths(10, "data");
//    ASSERT_EQ(SUCCESS, cache.Create(paths.first, paths.second));
//    ASSERT_EQ(SUCCESS, cache.UpdateSchemas(std::vector<ECSchemaPtr> {GetTestSchema()}));
//
//    // Setup test data
//    StubInstances instances;
//    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
//    instances.Add({"TestSchema.TestClass", "C"});
//    CachedResponseKey key(cache.FindOrCreateRoot(nullptr), "ResponseA");
//    ASSERT_EQ(SUCCESS, cache.CacheResponse(key, instances.ToWSObjectsResponse("ETagA")));
//
//    // Save
//    cache.GetECDb().SaveChanges();
//    cache.Close();
//    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_V10AddingAdditionalInstanceToExistingResponse_WorksFine_KnownIssue_UpgradeNotSupported)
    {
    // Arrange
    auto paths = GetSeedPaths(10, "data");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    // New instace
    auto ecClass = cache.GetAdapter().GetECClass("TestSchema", "TestClass");
    ASSERT_NE(nullptr, ecClass);
    auto instance = cache.GetChangeManager().CreateObject(*ecClass, Json::objectValue);
    ASSERT_TRUE(instance.IsValid());

    // Check
    CachedResponseKey responseKey(cache.FindOrCreateRoot(nullptr), "ResponseA");
    ASSERT_EQ(SUCCESS, cache.GetChangeManager().AddCreatedInstanceToResponse(responseKey, instance));

    ECInstanceKeyMultiMap instances;
    ASSERT_EQ(CacheStatus::OK, cache.ReadResponseInstanceKeys(responseKey, instances));
    EXPECT_EQ(4, instances.size());
    EXPECT_CONTAINS(instances, ECDbHelper::ToPair(instance));
    EXPECT_CONTAINS(instances, ECDbHelper::ToPair(cache.FindInstance({"TestSchema.TestClass", "A"})));
    EXPECT_CONTAINS(instances, ECDbHelper::ToPair(cache.FindInstance({"TestSchema.TestClass", "B"})));
    EXPECT_CONTAINS(instances, ECDbHelper::ToPair(cache.FindInstance({"TestSchema.TestClass", "C"})));
    }

//// Left for referance
/*--------------------------------------------------------------------------------------+
* @bsitest                              Benediktas.Lipnickas                     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
//TEST_F(DataSourceCacheUpgradeTests, SetupV11)
//    {
//    DataSourceCache cache;
//    auto paths = GetNewSeedPaths(11, "data");
//    ASSERT_EQ(SUCCESS, cache.Create(paths.first, paths.second));
//    ASSERT_EQ(SUCCESS, cache.UpdateSchemas(std::vector<ECSchemaPtr> {GetTestSchema()}));
//
//    // Setup test data
//    ObjectId fileId {"TestSchema.TestClass", "Foo"};
//    ASSERT_EQ(SUCCESS, cache.LinkInstanceToRoot("Root", fileId));
//
//    BeFileName fileToCachePath = StubFile();
//    EXPECT_TRUE(fileToCachePath.DoesPathExist());
//
//    EXPECT_EQ(SUCCESS, cache.CacheFile(fileId, WSFileResponse(fileToCachePath, HttpStatus::OK, "TestTag"), FileCache::Persistent));
//
//    // Save
//    cache.GetECDb().SaveChanges();
//    cache.Close();
//    }

/*--------------------------------------------------------------------------------------+
* @bsitest                              Benediktas.Lipnickas                     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_V11DetectFileModification_DetectsChanges_KnownIssue_UpgradeNotSupported)
    {
    // Arrange
    auto paths = GetSeedPaths(11, "data");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    // Check
    auto instanceKey = cache.FindInstance({"TestSchema.TestClass", "Foo"});
    ASSERT_TRUE(instanceKey.IsValid());

    SimpleWriteToFile("NewTestContent", cache.ReadFilePath(instanceKey));

    bool isFileModified = false;
    EXPECT_EQ(SUCCESS, cache.GetChangeManager().DetectFileModification(instanceKey, isFileModified));

    EXPECT_TRUE(isFileModified);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
// Left for referance
//TEST_F(DataSourceCacheUpgradeTests, SetupV12)
//    {
//    DataSourceCache cache;
//    auto paths = GetNewSeedPaths(12, "data");
//    ASSERT_EQ(SUCCESS, cache.Create(paths.first, paths.second));
//    ASSERT_EQ(SUCCESS, cache.UpdateSchemas(std::vector<ECSchemaPtr> {GetTestSchema()}));
//
//    // Setup test data
//    ASSERT_EQ(SUCCESS, cache.SetupRoot(nullptr, CacheRootPersistence::Temporary));
//
//    // Arrange
//    CachedResponseKey partialResponseKey(cache.FindOrCreateRoot(nullptr), "Partial");
//    CachedResponseKey fullResponseKey(cache.FindOrCreateRoot(nullptr), "Full");
//
//    WSQuery partialQuery("TestSchema", "TestClass");
//    partialQuery.SetSelect("TestProperty");
//
//    WSQuery fullQuery("TestSchema", "TestClass");
//    fullQuery.SetSelect("*");
//
//    StubInstances instances;
//    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "Full"}});
//
//    bset<ObjectId> rejected;
//    ASSERT_EQ(SUCCESS, cache.CacheResponse(partialResponseKey, instances.ToWSObjectsResponse("TestETag"), &rejected, &partialQuery));
//    ASSERT_THAT(rejected, IsEmpty());
//    ASSERT_EQ(SUCCESS, cache.CacheResponse(fullResponseKey, instances.ToWSObjectsResponse("TestETag"), &rejected, &fullQuery));
//    ASSERT_THAT(rejected, IsEmpty());
//
//    ASSERT_TRUE(cache.IsResponseCached(fullResponseKey));
//    ASSERT_TRUE(cache.GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
//
//    // Save
//    cache.GetECDb().SaveChanges();
//    cache.Close();
//    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_V12WithCachedResponses_ResponseTagsRemovedSoSyncWouldBeForced_KnownIssue_UpgradeNotSupported)
    {
    // Arrange
    auto paths = GetSeedPaths(12, "data");
    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    // Check
    CachedResponseKey partialResponseKey(cache.FindOrCreateRoot(nullptr), "Partial");
    CachedResponseKey fullResponseKey(cache.FindOrCreateRoot(nullptr), "Full");

    EXPECT_TRUE(cache.IsResponseCached(partialResponseKey));
    EXPECT_TRUE(cache.IsResponseCached(fullResponseKey));

    EXPECT_EQ("", cache.ReadResponseCacheTag(partialResponseKey));
    EXPECT_EQ("", cache.ReadResponseCacheTag(fullResponseKey));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_V12CacheTemporaryResponsesWithFullAndPartialInstance_InvalidatesFullResponsesWhenOverridenWithPartialData_KnownIssue_UpgradeNotSupported)
    {
    // Arrange
    auto paths = GetSeedPaths(12, "data");
    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    // Check
    CachedResponseKey partialResponseKey(cache.FindOrCreateRoot(nullptr), "Partial");
    CachedResponseKey fullResponseKey(cache.FindOrCreateRoot(nullptr), "Full");
    CachedResponseKey newResponseKey(cache.FindOrCreateRoot(nullptr), "New");

    WSQuery partialQuery("TestSchema", "TestClass");
    partialQuery.SetSelect("TestProperty");

    WSQuery fullQuery("TestSchema", "TestClass");
    fullQuery.SetSelect("*");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "Full"}});

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache.CacheResponse(partialResponseKey, instances.ToWSObjectsResponse("TagA"), &rejected, &partialQuery));
    ASSERT_THAT(rejected, IsEmpty());
    ASSERT_EQ(CacheStatus::OK, cache.CacheResponse(fullResponseKey, instances.ToWSObjectsResponse("TagB"), &rejected, &fullQuery));
    ASSERT_THAT(rejected, IsEmpty());

    ASSERT_TRUE(cache.IsResponseCached(fullResponseKey));
    ASSERT_TRUE(cache.GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "Partial"}});

    ASSERT_EQ(CacheStatus::OK, cache.CacheResponse(newResponseKey, instances.ToWSObjectsResponse("TagC"), &rejected, &partialQuery));
    ASSERT_THAT(rejected, IsEmpty());
    ASSERT_EQ("Partial", ReadInstance(cache, {"TestSchema.TestClass", "A"})["TestProperty"].asString());
    ASSERT_FALSE(cache.GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());

    // Assert
    EXPECT_EQ("TagC", cache.ReadResponseCacheTag(newResponseKey));
    EXPECT_EQ("TagA", cache.ReadResponseCacheTag(partialResponseKey));
    EXPECT_EQ("", cache.ReadResponseCacheTag(fullResponseKey));
    EXPECT_TRUE(cache.IsResponseCached(fullResponseKey));
    }

///*--------------------------------------------------------------------------------------+
//* @bsitest                                    Vincas.Razma                     10/15
//+---------------+---------------+---------------+---------------+---------------+------*/
//// Left for referance
//TEST_F(DataSourceCacheUpgradeTests, SetupV13_CachingDataSource)
//    {
//    auto paths = GetNewSeedPaths(13, "data");
//
//    auto client = MockWSRepositoryClient::Create();
//
//    StubInstances schemaDefs;
//    schemaDefs.Add({"MetaSchema.ECSchemaDef", "TestSchema"}, {{"Name", "TestSchema"}, {"VersionMajor", 1}, {"VersionMinor", 0}});
//
//    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
//        .WillOnce(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));
//
//    EXPECT_CALL(*client, SendGetSchemasRequest(_, _))
//        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemaDefs.ToWSObjectsResponse()))));
//
//    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "TestSchema"), An<BeFileNameCR>(), _, _, _))
//        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
//        {
//        GetTestSchema()->WriteToXmlFile(filePath);
//        return CreateCompletedAsyncTask(StubWSFileResult(filePath));
//        }));
//
//    auto result = CachingDataSource::OpenOrCreate(client, paths.first, paths.second)->GetResult();
//
//    ASSERT_TRUE(result.IsSuccess());
//
//    // Setup test data
//    auto txn = result.GetValue()->StartCacheTransaction();
//    StubInstances instances;
//    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
//    instances.Add({"TestSchema.TestClass", "C"});
//    CachedResponseKey key(txn.GetCache().FindOrCreateRoot(nullptr), "ResponseA");
//    ASSERT_EQ(CacheStatus::OK, txn.GetCache().CacheResponse(key, instances.ToWSObjectsResponse("ETagA")));
//
//    // Save
//    txn.GetCache().GetECDb().SaveChanges();
//    txn.GetCache().Close();
//    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_V13RepositoryInfo_LoadsCorrectly)
    {
    // Arrange
    auto paths = GetSeedPaths(13, "data");

    auto client = MockWSRepositoryClient::Create();

    StubInstances schemaDefs;
    schemaDefs.Add({"MetaSchema.ECSchemaDef", "TestSchema"}, {{"Name", "TestSchema"}, {"VersionMajor", 1}, {"VersionMinor", 0}});

    EXPECT_CALL(client->GetMockWSClient(), GetServerInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSInfoResult::Success(StubWSInfoWebApi()))));

    EXPECT_CALL(*client, GetInfo(_))
        .WillOnce(Return(CreateCompletedAsyncTask(WSRepositoryResult::Success(StubWSRepository()))));

    EXPECT_CALL(*client, SendGetSchemasRequest(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(WSObjectsResult::Success(schemaDefs.ToWSObjectsResponse()))));

    EXPECT_CALL(*client, SendGetFileRequest(ObjectId("MetaSchema.ECSchemaDef", "TestSchema"), An<BeFileNameCR>(), _, _, _))
        .WillOnce(Invoke([&] (ObjectIdCR, BeFileNameCR filePath, Utf8StringCR, Http::Request::ProgressCallbackCR, ICancellationTokenPtr)
        {
        GetTestSchema()->WriteToXmlFile(filePath);
        return CreateCompletedAsyncTask(StubWSFileResult(filePath));
        }));

    auto result = CachingDataSource::OpenOrCreate(client, paths.first, paths.second)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
// Left for referance
//TEST_F(DataSourceCacheUpgradeTests, SetupV20)
//    {
//    DataSourceCache cache;
//    auto paths = GetNewSeedPaths(20, "data");
//    ASSERT_EQ(SUCCESS, cache.Create(paths.first, paths.second));
//    ASSERT_EQ(SUCCESS, cache.UpdateSchemas(std::vector<ECSchemaPtr> {GetTestSchema()}));
//
//    // Setup test data
//    ASSERT_EQ(SUCCESS, cache.SetupRoot(nullptr, CacheRootPersistence::Temporary));
//
//    // Arrange
//    CachedResponseKey partialResponseKey(cache.FindOrCreateRoot(nullptr), "Partial");
//    CachedResponseKey fullResponseKey(cache.FindOrCreateRoot(nullptr), "Full");
//
//    WSQuery partialQuery("TestSchema", "TestClass");
//    partialQuery.SetSelect("TestProperty");
//
//    WSQuery fullQuery("TestSchema", "TestClass");
//    fullQuery.SetSelect("*");
//
//    StubInstances instances;
//    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "Full"}});
//
//    bset<ObjectId> rejected;
//    ASSERT_EQ(SUCCESS, cache.CacheResponse(partialResponseKey, instances.ToWSObjectsResponse("TestETag"), &rejected, &partialQuery));
//    ASSERT_THAT(rejected, IsEmpty());
//    ASSERT_EQ(SUCCESS, cache.CacheResponse(fullResponseKey, instances.ToWSObjectsResponse("TestETag"), &rejected, &fullQuery));
//    ASSERT_THAT(rejected, IsEmpty());
//
//    ASSERT_TRUE(cache.IsResponseCached(fullResponseKey));
//    ASSERT_TRUE(cache.GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());
//
//    // Save
//    cache.GetECDb().SaveChanges();
//    cache.Close();
//    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_V20WithCachedResponses_ResponseTagsRemovedSoSyncWouldBeForced_KnownIssue_UpgradeNotSupported)
    {
    // Arrange
    auto paths = GetSeedPaths(20, "data");
    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    // Check
    CachedResponseKey partialResponseKey(cache.FindOrCreateRoot(nullptr), "Partial");
    CachedResponseKey fullResponseKey(cache.FindOrCreateRoot(nullptr), "Full");

    EXPECT_TRUE(cache.IsResponseCached(partialResponseKey));
    EXPECT_TRUE(cache.IsResponseCached(fullResponseKey));

    EXPECT_EQ("", cache.ReadResponseCacheTag(partialResponseKey));
    EXPECT_EQ("", cache.ReadResponseCacheTag(fullResponseKey));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DataSourceCacheUpgradeTests, Open_V20CacheTemporaryResponsesWithFullAndPartialInstance_InvalidatesFullResponsesWhenOverridenWithPartialData_KnownIssue_UpgradeNotSupported)
    {
    // Arrange
    auto paths = GetSeedPaths(20, "data");
    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    // Check
    CachedResponseKey partialResponseKey(cache.FindOrCreateRoot(nullptr), "Partial");
    CachedResponseKey fullResponseKey(cache.FindOrCreateRoot(nullptr), "Full");
    CachedResponseKey newResponseKey(cache.FindOrCreateRoot(nullptr), "New");

    WSQuery partialQuery("TestSchema", "TestClass");
    partialQuery.SetSelect("TestProperty");

    WSQuery fullQuery("TestSchema", "TestClass");
    fullQuery.SetSelect("*");

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "Full"}});

    bset<ObjectId> rejected;
    ASSERT_EQ(CacheStatus::OK, cache.CacheResponse(partialResponseKey, instances.ToWSObjectsResponse("TagA"), &rejected, &partialQuery));
    ASSERT_THAT(rejected, IsEmpty());
    ASSERT_EQ(CacheStatus::OK, cache.CacheResponse(fullResponseKey, instances.ToWSObjectsResponse("TagB"), &rejected, &fullQuery));
    ASSERT_THAT(rejected, IsEmpty());

    ASSERT_TRUE(cache.IsResponseCached(fullResponseKey));
    ASSERT_TRUE(cache.GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "A"}, {{"TestProperty", "Partial"}});

    ASSERT_EQ(CacheStatus::OK, cache.CacheResponse(newResponseKey, instances.ToWSObjectsResponse("TagC"), &rejected, &partialQuery));
    ASSERT_THAT(rejected, IsEmpty());
    ASSERT_EQ("Partial", ReadInstance(cache, {"TestSchema.TestClass", "A"})["TestProperty"].asString());
    ASSERT_FALSE(cache.GetCachedObjectInfo({"TestSchema.TestClass", "A"}).IsFullyCached());

    // Assert
    EXPECT_EQ("TagC", cache.ReadResponseCacheTag(newResponseKey));
    EXPECT_EQ("TagA", cache.ReadResponseCacheTag(partialResponseKey));
    EXPECT_EQ("", cache.ReadResponseCacheTag(fullResponseKey));
    EXPECT_TRUE(cache.IsResponseCached(fullResponseKey));
    }
#endif
