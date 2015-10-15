/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Upgrade/DataSourceCacheUpgradeTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DataSourceCacheUpgradeTests.h"

#include <Bentley/BeDebugLog.h>
#include <WebServices/Cache/Persistence/DataSourceCache.h>
#include <WebServices/Cache/CachingDataSource.h>
#include "../../Client/MockWSRepositoryClient.h"
#include "../CachingTestsHelper.h"

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

#ifdef USE_GTEST

void DataSourceCacheUpgradeTests::SetUp()
    {
    BaseCachingDataSourceTest::SetUp();

    BeFileName assetsSeedPath(FSTest::GetAssetsDir() + L"WSClientTestAssets\\Cache\\UpgradeSeeds\\");
    BeFileName targetSeedPath(FSTest::GetTempDir() + L"DataSourceCacheUpgradeTests\\UpgradeSeeds\\");

    if (targetSeedPath.DoesPathExist())
        {
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::EmptyAndRemoveDirectory(targetSeedPath));
        }

    EXPECT_EQ(BeFileNameStatus::Success, BeFileName::CloneDirectory(assetsSeedPath, targetSeedPath, true));
    }

BeFileName GetSeedDir(int version, Utf8StringCR subdir)
    {
    BeFileName path =
        FSTest::GetTempDir()
        .AppendToPath(L"DataSourceCacheUpgradeTests")
        .AppendToPath(L"UpgradeSeeds")
        .AppendToPath(WPrintfString(L"%d", version))
        .AppendToPath(BeFileName(subdir));

    if (!path.DoesPathExist())
        {
        EXPECT_TRUE(path.DoesPathExist());
        }
    return path;
    }

bpair<BeFileName, CacheEnvironment> GetSeedPaths(int version, Utf8StringCR subdir)
    {
    BeFileName path =
        GetSeedDir(version, subdir)
        .AppendToPath(L"cache.ecdb");

    EXPECT_TRUE(path.DoesPathExist());

    BeFileName persistent = GetSeedDir(version, subdir).AppendToPath(L"persistent");
    BeFileName temporary = GetSeedDir(version, subdir).AppendToPath(L"temporary");

    CacheEnvironment environment(persistent, temporary);

    return {path, environment};
    }

TEST_F(DataSourceCacheUpgradeTests, Open_V5Empty_Success)
    {
    auto paths = GetSeedPaths(5, "empty");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));
    }

TEST_F(DataSourceCacheUpgradeTests, Open_V5Data_SuccessAndContainsOldData)
    {
    auto paths = GetSeedPaths(5, "data");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    ValidateV5SeedData(cache, paths.first, paths.second);
    EXPECT_FALSE(HasFailure());
    }

TEST_F(DataSourceCacheUpgradeTests, Open_V5DataUpgradeInterrupted1BeforeSettingNewCacheUpgradedFlag_SuccessAndContainsOldData)
    {
    // *-upgrade-new is finalized but has not set upgraded flag - should upgrade again
    auto paths = GetSeedPaths(5, "interrupted1");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    ValidateV5SeedData(cache, paths.first, paths.second);
    EXPECT_FALSE(HasFailure());
    }

TEST_F(DataSourceCacheUpgradeTests, Open_V5DataUpgradeInterrupted2AfterSettingNewCacheUpgradedFlag_SuccessAndContainsOldData)
    {
    // *-upgrade-new is finalized and has set upgraded flag - should copy file and return
    auto paths = GetSeedPaths(5, "interrupted2");

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Open(paths.first, paths.second));

    ValidateV5SeedData(cache, paths.first, paths.second);
    EXPECT_FALSE(HasFailure());
    }

TEST_F(DataSourceCacheUpgradeTests, CachingDataSource_OpenOrCreate_V5Data_ServerInfoSetDoDefaultAndSchemasAreValid)
    {
    auto paths = GetSeedPaths(5, "data");
    auto client = MockWSRepositoryClient::Create();

    auto result = CachingDataSource::OpenOrCreate(client, paths.first, paths.second)->GetResult();
    ASSERT_THAT(result.IsSuccess(), true);
    auto ds = result.GetValue();

    ValidateV5SeedData(ds, paths.first, paths.second);
    EXPECT_FALSE(HasFailure());
    }

TEST_F(DataSourceCacheUpgradeTests, CachingDataSource_OpenOrCreate_V5UpgradeInterrupted1_SuccessAndContainsOldData)
    {
    auto paths = GetSeedPaths(5, "interrupted1");
    auto client = MockWSRepositoryClient::Create();

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
    EXPECT_THAT(schemas[0]->GetName(), Eq(L"TestSchema"));

    // DataSourceCache
    ValidateV5SeedData(txn.GetCache(), path, environment);
    }

void DataSourceCacheUpgradeTests::ValidateV5SeedData(IDataSourceCache& cache, BeFileNameCR path, CacheEnvironmentCR environment)
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
    EXPECT_THAT(cache.ReadResponseCacheTag(responseKey1), Eq("Tag1"));
    EXPECT_THAT(cache.ReadResponseCacheTag(responseKey2), Eq("Tag2"));
    EXPECT_THAT(cache.ReadResponseCacheTag(responseKey3), Eq("Tag3"));
    EXPECT_THAT(cache.ReadResponseCacheTag(responseKey4), Eq(""));

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
    EXPECT_THAT(FSTest::ReadFile(filePath1), Eq("CachedFile1"));
    EXPECT_THAT(FSTest::ReadFile(filePath2), Eq("CachedFile2"));

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
        Utf8String ecsql = "SELECT GetECClassId(), ECInstanceId FROM [TS].[TestClass] WHERE [TestProperty] = ? ";
        EXPECT_EQ(SUCCESS, cache.GetAdapter().PrepareStatement(statement, ecsql));
        EXPECT_EQ(ECSqlStatus::Success, statement.BindText(1, testPropertyValue.c_str(), IECSqlBinder::MakeCopy::No));
        EXPECT_EQ(ECSqlStepStatus::HasRow, statement.Step());
        return ECInstanceKey(statement.GetValueInt64(0), statement.GetValueId<ECInstanceId>(1));
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
    EXPECT_THAT(FSTest::ReadFile(newFilePath), Eq("NewFile1"));
    EXPECT_THAT(newFilePath.GetFileNameAndExtension(), Eq(L"TestFile.txt"));

    EXPECT_THAT(cache.IsInstanceInRoot("NewInstanceRoot", newInstanceKey3), false);

    bvector<IChangeManager::RelationshipChange> changes;
    EXPECT_EQ(SUCCESS, cache.GetChangeManager().GetCreatedRelationships(newInstanceKey1, changes));
    ASSERT_THAT(changes.size(), 2);

    EXPECT_THAT(changes[1].GetSourceKey(), cache.FindInstance({"TestSchema.TestClass", "Parent"}));
    EXPECT_THAT(changes[1].GetTargetKey(), newInstanceKey1);

    EXPECT_THAT(changes[0].GetSourceKey(), newInstanceKey1);
    EXPECT_THAT(changes[0].GetTargetKey(), newInstanceKey2);

    EXPECT_THAT(cache.GetAdapter().GetECClass(changes[0].GetInstanceKey())->GetName(), Eq(L"LegacyParentRelationship"));
    EXPECT_THAT(cache.GetAdapter().GetECClass(changes[1].GetInstanceKey())->GetName(), Eq(L"LegacyParentRelationship"));

    changes.clear();
    EXPECT_EQ(SUCCESS, cache.GetChangeManager().GetCreatedRelationships(newInstanceKey2, changes));
    ASSERT_THAT(changes.size(), 1);
    EXPECT_THAT(changes[0].GetSourceKey(), newInstanceKey1);
    EXPECT_THAT(changes[0].GetTargetKey(), newInstanceKey2);
    EXPECT_THAT(cache.GetAdapter().GetECClass(changes[0].GetInstanceKey())->GetName(), Eq(L"LegacyParentRelationship"));

    changes.clear();
    EXPECT_EQ(SUCCESS, cache.GetChangeManager().GetCreatedRelationships(newInstanceKey3, changes));
    EXPECT_THAT(changes, IsEmpty());

    changes.clear();
    EXPECT_EQ(SUCCESS, cache.GetChangeManager().GetCreatedRelationships(newInstanceKey4, changes));
    EXPECT_THAT(changes, IsEmpty());
    }

TEST_F(DataSourceCacheUpgradeTests, Open_CurrentVersionDb_Success)
    {
    BeFileName path = FSTest::StubFilePath();

    DataSourceCache cache;
    ASSERT_EQ(SUCCESS, cache.Create(path, StubCacheEnvironemnt()));
    ASSERT_EQ(BE_SQLITE_OK, cache.GetAdapter().GetECDb().SaveChanges());
    ASSERT_EQ(SUCCESS, cache.Close());

    EXPECT_EQ(SUCCESS, DataSourceCache().Open(path, CacheEnvironment()));
    }

// Left for referance
//TEST_F(DataSourceCacheUpgradeTests, SetupV7)
//    {
//    DataSourceCache cache;
//
//    ASSERT_EQ(SUCCESS, cache.Create(BeFileName("C:/t/data/cache.ecdb"), {BeFileName("C:/t/data/persistent"), BeFileName("C:/t/data/temporary")}));
//
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

TEST_F(DataSourceCacheUpgradeTests, Open_V7ModifiedInstance_ReadModifiedPropertiesTreatsAllInstancePropertiesAsModified)
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

TEST_F(DataSourceCacheUpgradeTests, Open_V7ModifiedInstanceModifiedAgain_ReadModifiedPropertiesTreatsAllInstancePropertiesAsModified)
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

TEST_F(DataSourceCacheUpgradeTests, Open_V7ModifiedPropertyModifiedAgain_ReadModifiedPropertiesTreatsAllInstancePropertiesAsModified)
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

#endif
