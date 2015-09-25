/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Persistence/ChangeManagerTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ChangeManagerTests.h"

using namespace ::testing;
using std::shared_ptr;
USING_NAMESPACE_BENTLEY_WEBSERVICES

#ifdef USE_GTEST
TEST_F(ChangeManagerTests, IsSyncActive_SyncSetActiveToTrue_True)
    {
    auto cache = GetTestCache();
    cache->GetChangeManager().SetSyncActive(true);
    EXPECT_TRUE(cache->GetChangeManager().IsSyncActive());
    }

TEST_F(ChangeManagerTests, IsSyncActive_SyncNotActive_False)
    {
    auto cache = GetTestCache();
    EXPECT_FALSE(cache->GetChangeManager().IsSyncActive());
    }

TEST_F(ChangeManagerTests, SetSyncActive_SchemaUpdatedAndCacheStateReset_KeepsSyncActiveFlag)
    {
    // Arrange
    auto cache = GetTestCache();
    // Act
    cache->GetChangeManager().SetSyncActive(true);
    EXPECT_TRUE(cache->GetChangeManager().IsSyncActive());
    auto status = cache->UpdateSchemas({GetTestSchema()});
    // Assert
    EXPECT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->GetChangeManager().IsSyncActive());
    }

TEST_F(ChangeManagerTests, CreateObject_SyncSetToActive_Success)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    // Act
    cache->GetChangeManager().SetSyncActive(true);
    auto instance = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue);
    cache->GetChangeManager().SetSyncActive(false);
    // Assert
    EXPECT_TRUE(instance.IsValid());
    }

TEST_F(ChangeManagerTests, ModifyObject_SyncSetToActiveAndModifyingModifiedObject_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, Json::objectValue));
    // Act
    cache->GetChangeManager().SetSyncActive(true);
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().ModifyObject(instance, Json::objectValue);
    BeTest::SetFailOnAssert(true);
    cache->GetChangeManager().SetSyncActive(false);
    // Assert
    EXPECT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, DeleteObject_SyncSetToActiveAndDeletingCreatedObject_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue);
    // Act
    cache->GetChangeManager().SetSyncActive(true);
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().DeleteObject(instance);
    BeTest::SetFailOnAssert(true);
    cache->GetChangeManager().SetSyncActive(false);
    // Assert
    EXPECT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, CreateRelationship_SyncSetToActive_Success)
    {
    // Arrange
    auto cache = GetTestCache();
    auto source = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto target = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});
    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    // Act
    cache->GetChangeManager().SetSyncActive(true);
    auto relationship = cache->GetChangeManager().CreateRelationship(*testRelClass, source, target);
    cache->GetChangeManager().SetSyncActive(false);
    // Assert
    EXPECT_TRUE(relationship.IsValid());
    }

TEST_F(ChangeManagerTests, DeleteRelationship_SyncSetToActiveAndDeletingCreatedRelationship_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto source = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto target = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});
    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = cache->GetChangeManager().CreateRelationship(*testRelClass, source, target);
    // Act
    cache->GetChangeManager().SetSyncActive(true);
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().DeleteRelationship(relationship);
    BeTest::SetFailOnAssert(true);
    cache->GetChangeManager().SetSyncActive(false);
    // Assert
    EXPECT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, ModifyFile_SyncSetToActiveAndModifyingModifiedFile_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, FSTest::StubFile(), false));
    // Act
    cache->GetChangeManager().SetSyncActive(true);
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().ModifyFile(instance, FSTest::StubFile(), false);
    BeTest::SetFailOnAssert(true);
    cache->GetChangeManager().SetSyncActive(false);
    // Assert
    EXPECT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, GetLegacyParentRelationshipClass_Default_NonNull)
    {
    auto cache = GetTestCache();
    EXPECT_FALSE(nullptr == cache->GetChangeManager().GetLegacyParentRelationshipClass());
    }

TEST_F(ChangeManagerTests, LegacyCreateObject_RemoteIdIsEmpty_SetsRemoteId)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto parent = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Parent"});
    // Act
    Json::Value properties;
    properties["TestProperty"] = "TestValue";

    auto instance = cache->GetChangeManager().LegacyCreateObject(*testClass, properties, parent);
    // Assert
    ASSERT_NE("", cache->FindInstance(instance).remoteId);
    }

TEST_F(ChangeManagerTests, LegacyCreateObject_ParentPassed_CreatedInstanceAndRelatesWithLegacyRelationship)
    {
    // Arrange
    auto cache = GetTestCache();
    auto parent = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Parent"});
    // Act
    Json::Value properties;
    properties["TestProperty"] = "TestValue";

    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = cache->GetChangeManager().LegacyCreateObject(*testClass, properties, parent);
    // Assert
    ASSERT_TRUE(instance.IsValid());
    ChangeManager::Changes changes;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().GetChanges(changes));

    auto& objectChanges = changes.GetObjectChanges();
    ASSERT_EQ(1, objectChanges.size());
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, objectChanges.begin()->GetChangeStatus());
    EXPECT_EQ(instance, objectChanges.begin()->GetInstanceKey());

    Json::Value instanceJson;
    cache->GetAdapter().GetJsonInstance(instanceJson, instance);
    EXPECT_EQ("TestValue", instanceJson["TestProperty"].asString());

    auto& relChanges = changes.GetRelationshipChanges();
    ASSERT_EQ(1, relChanges.size());
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, relChanges.begin()->GetChangeStatus());
    EXPECT_TRUE(VerifyHasRelationship(cache, cache->GetChangeManager().GetLegacyParentRelationshipClass(), parent, instance));
    }

TEST_F(ChangeManagerTests, CreateObject_RemoteIdIsEmpty_SetsRemoteId)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    //Act
    auto instance = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue);
    // Assert
    ASSERT_NE("", cache->FindInstance(instance).remoteId);
    }

TEST_F(ChangeManagerTests, CreateObject_PropertiesPassed_InstanceSavedToCache)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    // Act
    Json::Value properties;
    properties["TestProperty"] = "TestValue";
    auto instance = cache->GetChangeManager().CreateObject(*testClass, properties);
    // Assert
    ASSERT_TRUE(instance.IsValid());
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    EXPECT_EQ(1, cache->GetChangeManager().GetObjectChange(instance).GetChangeNumber());
    Json::Value instanceJson;
    cache->GetAdapter().GetJsonInstance(instanceJson, instance);
    EXPECT_EQ("TestValue", instanceJson["TestProperty"].asString());
    }

TEST_F(ChangeManagerTests, ModifyObject_NotExistingObject_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto nonExistingInstance = StubNonExistingInstanceKey(*cache);
    // Act
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().ModifyObject(nonExistingInstance, Json::objectValue);
    BeTest::SetFailOnAssert(true);
    // Assert
    ASSERT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, ModifyObject_ExistingObject_SavesNewValuesToCache)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    // Act
    Json::Value properties;
    properties["TestProperty"] = "TestValue";
    auto status = cache->GetChangeManager().ModifyObject(instance, properties);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    Json::Value instanceJson;
    cache->GetAdapter().GetJsonInstance(instanceJson, instance);
    EXPECT_EQ("TestValue", instanceJson["TestProperty"].asString());
    }

TEST_F(ChangeManagerTests, ModifyObject_CreatedObject_SuccessAndLeavesStatusCreatedAndSameNumber)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue);
    // Act
    auto status = cache->GetChangeManager().ModifyObject(instance, Json::objectValue);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    EXPECT_EQ(1, cache->GetChangeManager().GetObjectChange(instance).GetChangeNumber());
    }

TEST_F(ChangeManagerTests, ModifyObject_ModifiedObject_LeavesStatusModifiedAndSameNumber)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, Json::objectValue));
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, Json::objectValue));
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    EXPECT_EQ(1, cache->GetChangeManager().GetObjectChange(instance).GetChangeNumber());
    }

TEST_F(ChangeManagerTests, ModifyObject_DeletedObject_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    // Act
    BeTest::SetFailOnAssert(false);
    ASSERT_EQ(ERROR, cache->GetChangeManager().ModifyObject(instance, Json::objectValue));
    BeTest::SetFailOnAssert(true);
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    EXPECT_EQ(1, cache->GetChangeManager().GetObjectChange(instance).GetChangeNumber());
    }

TEST_F(ChangeManagerTests, DeleteObject_NotExistingObject_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto nonExistingInstance = StubNonExistingInstanceKey(*cache);
    // Act
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().DeleteObject(nonExistingInstance);
    BeTest::SetFailOnAssert(true);
    // Assert
    ASSERT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, DeleteObject_ExistingObject_RemovesObjectFromCacheAndKeepsChange)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    // Act
    auto status = cache->GetChangeManager().DeleteObject(instance);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(0, cache->GetAdapter().CountClassInstances(cache->GetAdapter().GetECClass("TestSchema.TestClass")));
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, DeleteObject_CreatedObject_RemovesObjectAndChangeFromCache)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue);
    ASSERT_TRUE(instance.IsValid());
    // Act
    auto status = cache->GetChangeManager().DeleteObject(instance);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(0, cache->GetAdapter().CountClassInstances(cache->GetAdapter().GetECClass("TestSchema.TestClass")));
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, DeleteObject_ModifiedObject_RemovesObjectFromCacheAndSetsStatusDeletedAndIncrementsChangeNumber)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, Json::objectValue));
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    // Assert
    EXPECT_EQ(0, cache->GetAdapter().CountClassInstances(cache->GetAdapter().GetECClass("TestSchema.TestClass")));
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    EXPECT_EQ(2, cache->GetChangeManager().GetObjectChange(instance).GetChangeNumber());
    }

TEST_F(ChangeManagerTests, DeleteObject_DeletedObject_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    // Act
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().DeleteObject(instance);
    BeTest::SetFailOnAssert(TRUE);
    // Assert
    ASSERT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, CreateRelationship_RemoteIdIsEmpty_SetsRemoteId)
    {
    // Arrange
    auto cache = GetTestCache();
    auto source = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto target = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});
    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    // Act
    auto relationship = cache->GetChangeManager().CreateRelationship(*testRelClass, source, target);
    // Assert
    EXPECT_NE("", cache->FindRelationship(relationship).remoteId);
    }

TEST_F(ChangeManagerTests, CreateRelationship_NotExistingSourceAndTarget_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto source = StubNonExistingInstanceKey(*cache, "TestSchema.TestClass", 99991);
    auto target = StubNonExistingInstanceKey(*cache, "TestSchema.TestClass", 29999);
    // Act
    auto relationship = cache->GetChangeManager().CreateRelationship(*testRelClass, source, target);
    // Assert
    ASSERT_FALSE(relationship.IsValid());
    }

TEST_F(ChangeManagerTests, CreateRelationship_SuchRelationshipBetweenInstancesAlreadyExists_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(SUCCESS, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));
    // Act
    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto source = cache->FindInstance({"TestSchema.TestClass", "A"});
    auto target = cache->FindInstance({"TestSchema.TestClass", "B"});

    BeTest::SetFailOnAssert(false);
    auto relationship = cache->GetChangeManager().CreateRelationship(*testRelClass, source, target);
    BeTest::SetFailOnAssert(true);
    // Assert
    ASSERT_FALSE(relationship.IsValid());
    }

TEST_F(ChangeManagerTests, CreateRelationship_ValidRelationshipEndsWithECInstanceKeys_RelationshipSavedInCache)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto source = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto target = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});
    // Act
    auto relationship = cache->GetChangeManager().CreateRelationship(*testRelClass, source, target);
    // Assert
    ASSERT_TRUE(relationship.IsValid());
    auto relationshipClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    EXPECT_TRUE(VerifyHasRelationship(cache, relationshipClass, source, target));
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    EXPECT_EQ(1, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeNumber());
    }

TEST_F(ChangeManagerTests, CreateRelationship_ValidRelationshipEndsWithObjectIds_RelationshipSavedInCache)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto source = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto target = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});
    // Act
    auto relationship = cache->GetChangeManager().CreateRelationship(*testRelClass, source, target);
    // Assert
    ASSERT_TRUE(relationship.IsValid());
    auto relationshipClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    EXPECT_TRUE(VerifyHasRelationship(cache, relationshipClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"}));
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    EXPECT_EQ(1, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeNumber());
    }

TEST_F(ChangeManagerTests, DeleteRelationship_ExistingRelationship_RemovesRelationshipFromCacheAndKeepsChange)
    {
    // Arrange
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(SUCCESS, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));

    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = cache->FindRelationship(*relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"});
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteRelationship(relationship));
    // Assert
    EXPECT_EQ(0, cache->GetAdapter().CountClassInstances(cache->GetAdapter().GetECClass("TestSchema.TestRelationshipClass")));
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    EXPECT_EQ(1, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeNumber());
    }

TEST_F(ChangeManagerTests, DeleteRelationship_CreatedRelationship_RemovesRelationshipAndChangeFromCache)
    {
    // Arrange
    auto cache = GetTestCache();
    auto source = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto target = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});
    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = cache->GetChangeManager().CreateRelationship(*testRelClass, source, target);
    ASSERT_TRUE(relationship.IsValid());
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteRelationship(relationship));
    // Assert
    EXPECT_EQ(0, cache->GetAdapter().CountClassInstances(cache->GetAdapter().GetECClass("TestSchema.TestRelationshipClass")));
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, DeleteRelationship_DeletedRelationship_Error)
    {
    // Arrange
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(SUCCESS, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));

    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = cache->FindRelationship(*relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"});

    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteRelationship(relationship));
    // Act
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().DeleteRelationship(relationship);
    BeTest::SetFailOnAssert(true);
    // Assert
    ASSERT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, ModifyFile_NonExistingObject_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto nonExistingInstance = StubNonExistingInstanceKey(*cache);
    // Act
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().ModifyFile(nonExistingInstance, FSTest::StubFile(), false);
    BeTest::SetFailOnAssert(true);
    // Assert
    ASSERT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, ModifyFile_ExistingObject_SetsChangeStatusAndCachesFileToPersistentLocation)
    {
    // Arrange
    auto cache = GetTestCache();
    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    auto instance = StubInstanceInCache(*cache, fileId);
    // Act
    auto status = cache->GetChangeManager().ModifyFile(instance, FSTest::StubFile("NewContent"), false);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetFileChange(instance).GetChangeStatus());
    EXPECT_TRUE(cache->ReadFilePath(instance).find(L"test_files_persistent") != BeFileName::npos);
    EXPECT_EQ(1, cache->GetChangeManager().GetFileChange(instance).GetChangeNumber());
    EXPECT_EQ("NewContent", FSTest::ReadFile(cache->ReadFilePath(instance)));
    }

TEST_F(ChangeManagerTests, ModifyFile_ExistingObjectAndCopyFileTrue_CopiesFileToPersistentLocationAndLeavesOriginal)
    {
    // Arrange
    auto cache = GetTestCache();
    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    auto instance = StubInstanceInCache(*cache, fileId);
    // Act
    auto filePath = FSTest::StubFile("NewContent");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, filePath, true));
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetFileChange(instance).GetChangeStatus());
    EXPECT_TRUE(cache->ReadFilePath(instance).find(L"test_files_persistent") != BeFileName::npos);
    EXPECT_EQ(1, cache->GetChangeManager().GetFileChange(instance).GetChangeNumber());
    EXPECT_EQ("NewContent", FSTest::ReadFile(cache->ReadFilePath(instance)));
    EXPECT_TRUE(filePath.DoesPathExist());
    }

TEST_F(ChangeManagerTests, ModifyFile_Twice_LeavesChangeNumberAndChangesContent)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, FSTest::StubFile("A"), false));
    auto status = cache->GetChangeManager().ModifyFile(instance, FSTest::StubFile("B"), false);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetFileChange(instance).GetChangeStatus());
    EXPECT_EQ(1, cache->GetChangeManager().GetFileChange(instance).GetChangeNumber());
    EXPECT_EQ("B", FSTest::ReadFile(cache->ReadFilePath(instance)));
    }

TEST_F(ChangeManagerTests, ModifyFile_ExistingFile_ReplacesFileContentAndSetsChangeStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    cache->CacheFile({"TestSchema.TestClass", "Foo"}, StubWSFileResponse(FSTest::StubFile("InitialContent")), FileCache::Persistent);
    // Act
    auto status = cache->GetChangeManager().ModifyFile(instance, FSTest::StubFile("NewContent"), false);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetFileChange(instance).GetChangeStatus());
    EXPECT_EQ(1, cache->GetChangeManager().GetFileChange(instance).GetChangeNumber());
    EXPECT_EQ("NewContent", FSTest::ReadFile(cache->ReadFilePath(instance)));
    }

TEST_F(ChangeManagerTests, ChangesIsEmpty_NoChanges_True)
    {
    ChangeManager::Changes changes;
    EXPECT_TRUE(changes.IsEmpty());
    }

TEST_F(ChangeManagerTests, ChangesIsEmpty_FileChangeAdded_False)
    {
    ChangeManager::Changes changes;
    changes.AddChange(ChangeManager::FileChange());
    EXPECT_FALSE(changes.IsEmpty());
    }

TEST_F(ChangeManagerTests, HasChanges_NoChanges_False)
    {
    auto cache = GetTestCache();
    EXPECT_FALSE(cache->GetChangeManager().HasChanges());
    }

TEST_F(ChangeManagerTests, HasChanges_CreatedObject_True)
    {
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue);
    EXPECT_TRUE(cache->GetChangeManager().HasChanges());
    }

TEST_F(ChangeManagerTests, HasChanges_CreatedRelationship_True)
    {
    // Arrange
    auto cache = GetTestCache();
    auto source = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto target = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});
    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    // Act
    auto relationship = cache->GetChangeManager().CreateRelationship(*testRelClass, source, target);
    // Assert
    EXPECT_TRUE(cache->GetChangeManager().HasChanges());
    }

TEST_F(ChangeManagerTests, HasChanges_ChangedFile_True)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, FSTest::StubFile(), false));
    // Assert
    EXPECT_TRUE(cache->GetChangeManager().HasChanges());
    }

TEST_F(ChangeManagerTests, GetChanges_NoChanges_Empty)
    {
    // Arrange
    auto cache = GetTestCache();
    // Act
    ChangeManager::Changes changes;
    auto status = cache->GetChangeManager().GetChanges(changes);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_TRUE(changes.IsEmpty());
    }

TEST_F(ChangeManagerTests, GetChanges_CreatedObject_ReturnsInstance)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue);
    // Act
    ChangeManager::Changes changes;
    auto status = cache->GetChangeManager().GetChanges(changes);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(1, changes.GetObjectChanges().size());
    auto change = ChangeManager::ObjectChange(instance, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 1);
    EXPECT_CONTAINS(changes.GetObjectChanges(), change);
    }

TEST_F(ChangeManagerTests, GetChanges_DeletedObject_ReturnsChanges)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    // Act
    ChangeManager::Changes changes;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().GetChanges(changes));
    // Assert
    ASSERT_EQ(1, changes.GetObjectChanges().size());
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, changes.GetObjectChanges().begin()->GetChangeStatus());
    EXPECT_EQ(instance, changes.GetObjectChanges().begin()->GetInstanceKey());
    EXPECT_EQ(1, changes.GetObjectChanges().begin()->GetChangeNumber());
    }

TEST_F(ChangeManagerTests, GetChanges_ModifiedFile_ReturnsChange)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, FSTest::StubFile(), false));
    // Act
    ChangeManager::Changes changes;
    auto status = cache->GetChangeManager().GetChanges(changes, true);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(1, changes.GetFileChanges().size());
    auto change = ChangeManager::FileChange(instance, IChangeManager::ChangeStatus::Modified, ChangeManager::SyncStatus::Ready, 1);
    EXPECT_CONTAINS(changes.GetFileChanges(), change);
    }

TEST_F(ChangeManagerTests, GetChanges_GetOnlySyncReady_ReturnOnlyReadyInstances)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto instanceNotReady = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue, ChangeManager::SyncStatus::NotReady);
    auto instanceReady = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue, ChangeManager::SyncStatus::Ready);
    ASSERT_TRUE(instanceNotReady.IsValid());
    ASSERT_TRUE(instanceReady.IsValid());
    // Act
    ChangeManager::Changes changes;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().GetChanges(changes, true));
    // Assert
    EXPECT_EQ(1, changes.GetObjectChanges().size());
    auto change = ChangeManager::ObjectChange(instanceReady, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 2);
    EXPECT_CONTAINS(changes.GetObjectChanges(), change);
    }

TEST_F(ChangeManagerTests, GetChanges_SpecificInstanceAndCreatedObject_ReturnsInstance)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto instanceA = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue);
    auto instanceB = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue);
    // Act
    ChangeManager::Changes changes;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().GetChanges(instanceB, changes));
    // Assert
    ASSERT_EQ(1, changes.GetObjectChanges().size());
    EXPECT_EQ(0, changes.GetRelationshipChanges().size());
    EXPECT_EQ(0, changes.GetFileChanges().size());
    EXPECT_EQ(instanceB, changes.GetObjectChanges().begin()->GetInstanceKey());
    }

TEST_F(ChangeManagerTests, GetChanges_SpecificInstanceAndModifiedFile_ReturnsChange)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, FSTest::StubFile(), false));
    // Act
    ChangeManager::Changes changes;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().GetChanges(instance, changes));
    // Assert
    EXPECT_EQ(0, changes.GetObjectChanges().size());
    EXPECT_EQ(0, changes.GetRelationshipChanges().size());
    ASSERT_EQ(1, changes.GetFileChanges().size());
    EXPECT_EQ(instance, changes.GetFileChanges().begin()->GetInstanceKey());
    }

TEST_F(ChangeManagerTests, GetChanges_SpecificInstanceAndCreatedRelationship_ReturnsChange)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto source = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto target = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});
    auto relationship = cache->GetChangeManager().CreateRelationship(*testRelClass, source, target);
    // Act
    ChangeManager::Changes changes;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().GetChanges(relationship, changes));
    // Assert
    EXPECT_EQ(0, changes.GetObjectChanges().size());
    ASSERT_EQ(1, changes.GetRelationshipChanges().size());
    EXPECT_EQ(0, changes.GetFileChanges().size());
    EXPECT_EQ(relationship, changes.GetRelationshipChanges().begin()->GetInstanceKey());
    }

TEST_F(ChangeManagerTests, GetObjectChange_NotExistingObject_NoChangesStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto nonExistingInstance = StubNonExistingInstanceKey(*cache);
    // Act
    auto change = cache->GetChangeManager().GetObjectChange(nonExistingInstance);
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, change.GetChangeStatus());
    EXPECT_EQ(ChangeManager::SyncStatus::NotReady, change.GetSyncStatus());
    }

TEST_F(ChangeManagerTests, GetObjectChange_NoChanges_ReturnsNoChange)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "NoChanges"});
    // Act
    auto change = cache->GetChangeManager().GetObjectChange(instance);
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, change.GetChangeStatus());
    }

TEST_F(ChangeManagerTests, GetObjectChange_CreatedInstance_ReturnsObjectChange)
    {
    // Arrange
    auto cache = GetTestCache();
    Json::Value properties(Json::objectValue);
    properties["TestProperty"] = "TestValue";
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = cache->GetChangeManager().CreateObject(*testClass, properties, ChangeManager::SyncStatus::Ready);
    // Act
    ChangeManager::ObjectChange change = cache->GetChangeManager().GetObjectChange(instance);
    // Assert
    EXPECT_EQ(instance, change.GetInstanceKey());
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, change.GetChangeStatus());
    EXPECT_EQ(ChangeManager::SyncStatus::Ready, change.GetSyncStatus());
    }

TEST_F(ChangeManagerTests, GetObjectChange_ModifiedObject_ModifiedStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, Json::objectValue));
    // Act
    auto change = cache->GetChangeManager().GetObjectChange(instance);
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, change.GetChangeStatus());
    }

TEST_F(ChangeManagerTests, GetObjectChange_DeletedObject_DeletedStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    // Act
    auto change = cache->GetChangeManager().GetObjectChange(instance);
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, change.GetChangeStatus());
    }

TEST_F(ChangeManagerTests, GetObjectChange_CreatedObjectWithSyncStatusNotReady_SyncStatusNotReady)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue, ChangeManager::SyncStatus::NotReady);
    // Act
    auto change = cache->GetChangeManager().GetObjectChange(instance);
    // Assert
    EXPECT_EQ(ChangeManager::SyncStatus::NotReady, change.GetSyncStatus());
    }

TEST_F(ChangeManagerTests, GetObjectChange_ModificationsOneAfterOther_ChangeNumbersInSequence)
    {
    // Arrange
    auto cache = GetTestCache();
    // Mix initial order
    auto instanceA = StubInstanceInCache(*cache, {"TestSchema.TestClass", "C"});
    auto instanceB = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto instanceC = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});
    // Modify in specific order
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instanceA, Json::objectValue));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instanceB, Json::objectValue));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instanceC, Json::objectValue));
    // Act
    auto changeA = cache->GetChangeManager().GetObjectChange(instanceA);
    auto changeB = cache->GetChangeManager().GetObjectChange(instanceB);
    auto changeC = cache->GetChangeManager().GetObjectChange(instanceC);
    // Assert
    EXPECT_LT(changeA.GetChangeNumber(), changeB.GetChangeNumber());
    EXPECT_LT(changeB.GetChangeNumber(), changeC.GetChangeNumber());
    }

TEST_F(ChangeManagerTests, GetRelationshipChange_NonExistingRelationship_ReturnsNoChange)
    {
    // Arrange
    auto cache = GetTestCache();
    auto nonExistingInstance = StubNonExistingInstanceKey(*cache, "TestSchema.TestRelationshipClass");
    // Act
    auto change = cache->GetChangeManager().GetRelationshipChange(nonExistingInstance);
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, change.GetChangeStatus());
    EXPECT_FALSE(change.GetSourceKey().IsValid());
    EXPECT_FALSE(change.GetTargetKey().IsValid());
    }

TEST_F(ChangeManagerTests, GetRelationshipChange_NotChangedRelationship_ReturnsNoChange)
    {
    // Arrange
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(SUCCESS, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));

    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = cache->FindRelationship(*relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"});
    // Act
    auto change = cache->GetChangeManager().GetRelationshipChange(relationship);
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, change.GetChangeStatus());
    EXPECT_EQ(relationship, change.GetInstanceKey());
    EXPECT_EQ(cache->FindInstance({"TestSchema.TestClass", "A"}), change.GetSourceKey());
    EXPECT_EQ(cache->FindInstance({"TestSchema.TestClass", "B"}), change.GetTargetKey());
    }

TEST_F(ChangeManagerTests, GetRelationshipChange_CreatedRelationship_ReturnsRelationshipChange)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto source = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto target = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});
    auto relationship = cache->GetChangeManager().CreateRelationship(*testRelClass, source, target);
    // Act
    auto change = cache->GetChangeManager().GetRelationshipChange(relationship);
    // Assert
    EXPECT_EQ(relationship, change.GetInstanceKey());
    EXPECT_EQ(source, change.GetSourceKey());
    EXPECT_EQ(target, change.GetTargetKey());
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, change.GetChangeStatus());
    EXPECT_EQ(ChangeManager::SyncStatus::Ready, change.GetSyncStatus());
    }

TEST_F(ChangeManagerTests, GetFileChange_NoChanges_ReturnsNoChange)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "NoChanges"});
    // Act
    auto change = cache->GetChangeManager().GetFileChange(instance);
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, change.GetChangeStatus());
    }

TEST_F(ChangeManagerTests, GetFileChange_ModifiedFile_ReturnsFileChange)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, FSTest::StubFile(), false));
    // Act
    auto change = cache->GetChangeManager().GetFileChange(instance);
    // Assert
    EXPECT_EQ(instance, change.GetInstanceKey());
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, change.GetChangeStatus());
    EXPECT_EQ(ChangeManager::SyncStatus::Ready, change.GetSyncStatus());
    }

TEST_F(ChangeManagerTests, SetSyncStatus_NonExistingObject_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto nonExistingInstance = StubNonExistingInstanceKey(*cache);
    // Act
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().SetSyncStatus(nonExistingInstance, ChangeManager::SyncStatus::Ready);
    BeTest::SetFailOnAssert(true);
    // Assert
    ASSERT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, SetSyncStatus_NotChangedObject_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    // Act
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().SetSyncStatus(instance, ChangeManager::SyncStatus::Ready);
    BeTest::SetFailOnAssert(true);
    // Assert
    ASSERT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, SetSyncStatus_CreatedObject_SuccessAndChangesStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue, ChangeManager::SyncStatus::NotReady);
    // Act
    auto status = cache->GetChangeManager().SetSyncStatus(instance, ChangeManager::SyncStatus::Ready);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(ChangeManager::SyncStatus::Ready, cache->GetChangeManager().GetObjectChange(instance).GetSyncStatus());
    }

TEST_F(ChangeManagerTests, SetSyncStatus_CreatedRelationship_SuccessAndChangesStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto source = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto target = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});
    auto relationship = cache->GetChangeManager().CreateRelationship(*testRelClass, source, target, ChangeManager::SyncStatus::NotReady);
    // Act
    auto status = cache->GetChangeManager().SetSyncStatus(relationship, ChangeManager::SyncStatus::Ready);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(ChangeManager::SyncStatus::Ready, cache->GetChangeManager().GetRelationshipChange(relationship).GetSyncStatus());
    }

TEST_F(ChangeManagerTests, SetSyncStatus_ModfiedFile_SuccessAndChangesStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, FSTest::StubFile(), false, ChangeManager::SyncStatus::NotReady));
    // Act
    auto status = cache->GetChangeManager().SetSyncStatus(instance, ChangeManager::SyncStatus::Ready);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(ChangeManager::SyncStatus::Ready, cache->GetChangeManager().GetFileChange(instance).GetSyncStatus());
    }

TEST_F(ChangeManagerTests, GetObjectChangeStatus_ObjectNotInCache_NoChange)
    {
    // Arrange
    auto cache = GetTestCache();
    auto nonExistingInstance = StubNonExistingInstanceKey(*cache);
    // Act
    auto status = cache->GetChangeManager().GetObjectChangeStatus(nonExistingInstance);
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, status);
    }

TEST_F(ChangeManagerTests, GetObjectChangeStatus_ObjectCached_NoChange)
    {
    // Arrange
    auto cache = GetTestCache();
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse(), "foo_root");
    auto instance = cache->FindInstance({"TestSchema.TestClass", "Foo"});
    // Act
    auto status = cache->GetChangeManager().GetObjectChangeStatus(instance);
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, status);
    }

TEST_F(ChangeManagerTests, GetObjectChangeStatus_ObjectCreated_Created)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue);
    // Act
    auto status = cache->GetChangeManager().GetObjectChangeStatus(instance);
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, status);
    }

TEST_F(ChangeManagerTests, GetObjectChangeStatus_ObjectModified_Modified)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, Json::objectValue));
    // Act
    auto status = cache->GetChangeManager().GetObjectChangeStatus(instance);
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, status);
    }

TEST_F(ChangeManagerTests, CommitCreationChanges_NotExistingObject_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto nonExistingInstance = StubNonExistingInstanceKey(*cache);
    // Act
    auto status = cache->GetChangeManager().CommitCreationChanges({{nonExistingInstance, "newId"}});
    // Assert
    EXPECT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, CommitCreationChanges_NotCreatedObject_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, Json::objectValue));
    // Act
    auto status = cache->GetChangeManager().CommitCreationChanges({{instance, "NewId"}});
    // Assert
    EXPECT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, CommitCreationChanges_NewObjectIdIsEmpty_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue);
    // Act
    auto status = cache->GetChangeManager().CommitCreationChanges({{instance, ""}});
    // Assert
    ASSERT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, CommitCreationChanges_CreatedObject_RemovesChangeStatusAndChangesRemoteId)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue);
    // Act
    auto status = cache->GetChangeManager().CommitCreationChanges({{instance, "NewId"}});
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "NewId"}).IsValid());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetCachedObjectInfo(instance).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, CommitCreationChanges_CreatedObjectWithFile_RemovesFileChangeStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, FSTest::StubFile(), false));
    // Act
    auto status = cache->GetChangeManager().CommitCreationChanges({{instance, "NewId"}});
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetFileChange(instance).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, CommitCreationChanges_NewRemoteIdExistsInCache_RemovesOldInstanceAndMovesRootRelationships)
    {
    // Arrange
    auto cache = GetTestCache();
    cache->LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass", "Foo"});

    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto newInstance = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue);
    // Act
    auto status = cache->GetChangeManager().CommitCreationChanges({{newInstance, "Foo"}});
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(newInstance, cache->FindInstance({"TestSchema.TestClass", "Foo"}));
    EXPECT_EQ(1, cache->GetAdapter().CountClassInstances(cache->GetAdapter().GetECClass("TestSchema.TestClass")));
    EXPECT_TRUE(cache->IsInstanceInRoot("TestRoot", cache->FindInstance({"TestSchema.TestClass", "Foo"})));
    }

TEST_F(ChangeManagerTests, CommitCreationChanges_NewRemoteIdExistsInCacheAndInCachedResponse_InvalidatesCachedResponse)
    {
    // Arrange
    auto cache = GetTestCache();

    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    instances.Add({"TestSchema.TestClass", "Other"});

    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse("TestTag")));
    EXPECT_THAT(cache->ReadResponseCacheTag(responseKey), Eq("TestTag"));

    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto newInstance = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue);

    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitCreationChanges({{newInstance, "Foo"}}));

    // Assert
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "Foo"}), newInstance);
    EXPECT_THAT(cache->IsResponseCached(responseKey), true);
    EXPECT_THAT(cache->ReadResponseCacheTag(responseKey), IsEmpty());

    bset<ObjectId> responseObjectIds;
    cache->ReadResponseObjectIds(responseKey, responseObjectIds);

    EXPECT_THAT(responseObjectIds, SizeIs(1));
    EXPECT_THAT(responseObjectIds, Contains(ObjectId {"TestSchema.TestClass", "Other"}));
    }

TEST_F(ChangeManagerTests, UpdateCreatedInstance_CreatedRelationship_RemovesChangeStatusAndChangesRemoteId)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto source = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto target = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});
    auto relationship = cache->GetChangeManager().CreateRelationship(*testRelClass, source, target, ChangeManager::SyncStatus::Ready);
    // Act
    auto status = cache->GetChangeManager().CommitCreationChanges({{relationship, "NewId"}});
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->FindRelationship(relationship).IsValid());
    EXPECT_EQ(ObjectId("TestSchema.TestRelationshipClass", "NewId"), cache->FindRelationship(cache->FindRelationship(*testRelClass, source, target)));
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, UpdateCreatedInstance_NewRelationshipIdIsEmpty_RemovesRelationshipFromCache)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto source = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto target = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});
    auto relationship = cache->GetChangeManager().CreateRelationship(*testRelClass, source, target, ChangeManager::SyncStatus::Ready);
    // Act
    auto status = cache->GetChangeManager().CommitCreationChanges({{relationship, ""}});
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_FALSE(cache->FindRelationship(relationship).IsValid());
    EXPECT_FALSE(cache->FindRelationship(*testRelClass, source, target).IsValid());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, UpdateCreatedInstance_NoInstance_ReturnError)
    {
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});

    bmap<ECInstanceKey, ECInstanceKey> changedKeys;
    ASSERT_EQ(ERROR, cache->GetChangeManager().UpdateCreatedInstance({"TestSchema.TestClass", "NotExisting"}, instances.ToWSObjectsResponse(), changedKeys));
    }

TEST_F(ChangeManagerTests, UpdateCreatedInstance_MoreThanOneInstance_ReturnError)
    {
    auto cache = GetTestCache();
    StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    instances.Add({"TestSchema.TestClass", "B"});

    bmap<ECInstanceKey, ECInstanceKey> changedKeys;
    ASSERT_EQ(ERROR, cache->GetChangeManager().UpdateCreatedInstance({"TestSchema.TestClass", "A"}, instances.ToWSObjectsResponse(), changedKeys));
    }

TEST_F(ChangeManagerTests, UpdateCreatedInstance_InstanceWithSameClassAndId_UpdatesPropertiesWithoutChangingInstanceKey)
    {
    auto cache = GetTestCache();
    ObjectId objectId("TestSchema.TestClass", "Foo");
    auto oldInstanceKey = StubInstanceInCache(*cache, objectId);

    StubInstances instances;
    instances.Add(objectId, {{"TestProperty", "TestValue"}});
    bmap<ECInstanceKey, ECInstanceKey> changedKeys;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().UpdateCreatedInstance(objectId, instances.ToWSObjectsResponse(), changedKeys));

    EXPECT_THAT(cache->GetCachedObjectInfo(objectId).IsInCache(), true);

    Json::Value jsonInstance;
    cache->ReadInstance(objectId, jsonInstance);
    EXPECT_THAT(jsonInstance["TestProperty"], Eq("TestValue"));
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "Foo"}), Eq(oldInstanceKey));

    EXPECT_THAT(changedKeys, IsEmpty());
    }

TEST_F(ChangeManagerTests, UpdateCreatedInstance_InstanceWithDifferentId_Error)
    {
    auto cache = GetTestCache();
    auto oldInstanceKey = StubInstanceInCache(*cache, {"TestSchema.TestClass", "OldId"});

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "NewId"}, {{"TestProperty", "TestValue"}});
    bmap<ECInstanceKey, ECInstanceKey> changedKeys;
    ASSERT_EQ(ERROR, cache->GetChangeManager().UpdateCreatedInstance({"TestSchema.TestClass", "OldId"}, instances.ToWSObjectsResponse(), changedKeys));
    }

TEST_F(ChangeManagerTests, UpdateCreatedInstance_InstanceWithDifferentClassAndSameId_UpdatesClassAndPropertiesAndChangesInstanceKey)
    {
    auto cache = GetTestCache();
    auto oldInstanceKey = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});

    StubInstances instances;
    instances.Add({"TestSchema.TestClass2", "Foo"}, {{"TestProperty", "TestValue"}});
    bmap<ECInstanceKey, ECInstanceKey> changedKeys;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().UpdateCreatedInstance({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse(), changedKeys));

    EXPECT_THAT(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsInCache(), false);
    EXPECT_THAT(cache->GetCachedObjectInfo({"TestSchema.TestClass2", "Foo"}).IsInCache(), true);

    Json::Value jsonInstance;
    cache->ReadInstance({"TestSchema.TestClass2", "Foo"}, jsonInstance);
    EXPECT_THAT(jsonInstance["TestProperty"], Eq("TestValue"));
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "NewId"}), Not(Eq(oldInstanceKey)));

    EXPECT_THAT(changedKeys, SizeIs(1));
    EXPECT_THAT(changedKeys, Contains(StubBPair(oldInstanceKey, cache->FindInstance({"TestSchema.TestClass2", "Foo"}))));
    }

TEST_F(ChangeManagerTests, UpdateCreatedInstance_InstanceWithDifferentClassAndId_Error)
    {
    auto cache = GetTestCache();
    StubInstanceInCache(*cache, {"TestSchema.TestClass", "OldId"});

    StubInstances instances;
    instances.Add({"TestSchema.TestClass2", "NewId"}, {{"TestProperty", "TestValue"}});
    bmap<ECInstanceKey, ECInstanceKey> changedKeys;
    ASSERT_EQ(ERROR, cache->GetChangeManager().UpdateCreatedInstance({"TestSchema.TestClass", "OldId"}, instances.ToWSObjectsResponse(), changedKeys));
    }

TEST_F(ChangeManagerTests, UpdateCreatedInstancee_InstanceWithDifferentClass_RemovesOldInstanceAndMovesRootRelationships)
    {
    auto cache = GetTestCache();
    cache->LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass", "Foo"});
    auto oldInstanceKey = cache->FindInstance({"TestSchema.TestClass", "Foo"});

    StubInstances instances;
    instances.Add({"TestSchema.TestClass2", "Foo"});
    bmap<ECInstanceKey, ECInstanceKey> changedKeys;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().UpdateCreatedInstance({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse(), changedKeys));

    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid(), false);
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass2", "Foo"}).IsValid(), true);
    EXPECT_EQ(0, cache->GetAdapter().CountClassInstances(cache->GetAdapter().GetECClass("TestSchema.TestClass")));
    EXPECT_EQ(1, cache->GetAdapter().CountClassInstances(cache->GetAdapter().GetECClass("TestSchema.TestClass2")));
    EXPECT_TRUE(cache->IsInstanceInRoot("TestRoot", cache->FindInstance({"TestSchema.TestClass2", "Foo"})));

    EXPECT_THAT(changedKeys, SizeIs(1));
    EXPECT_THAT(changedKeys, Contains(StubBPair(oldInstanceKey, cache->FindInstance({"TestSchema.TestClass2", "Foo"}))));
    }

TEST_F(ChangeManagerTests, UpdateCreatedInstance_OldInstanceExistsInCachedResponseWithSameClassAndId_InvalidatesCachedResponse)
    {
    auto cache = GetTestCache();

    auto responseKey = StubCachedResponseKey(*cache);
    cache->LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass", "Foo"});
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    instances.Add({"TestSchema.TestClass", "Other"});
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse("TestTag")));
    EXPECT_THAT(cache->ReadResponseCacheTag(responseKey), Eq("TestTag"));

    instances.Clear();
    instances.Add({"TestSchema.TestClass", "Foo"});
    bmap<ECInstanceKey, ECInstanceKey> changedKeys;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().UpdateCreatedInstance({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse(), changedKeys));

    EXPECT_THAT(changedKeys, IsEmpty());
    EXPECT_THAT(cache->IsResponseCached(responseKey), true);
    EXPECT_THAT(cache->ReadResponseCacheTag(responseKey), IsEmpty());

    bset<ObjectId> responseObjectIds;
    cache->ReadResponseObjectIds(responseKey, responseObjectIds);

    EXPECT_THAT(responseObjectIds, SizeIs(2));
    EXPECT_THAT(responseObjectIds, Contains(ObjectId {"TestSchema.TestClass", "Foo"}));
    EXPECT_THAT(responseObjectIds, Contains(ObjectId {"TestSchema.TestClass", "Other"}));
    }

TEST_F(ChangeManagerTests, UpdateCreatedInstance_ChangedClassAndOldInstanceHasCreatedSourceRelationship_PreservesRelationshipWithSameChangeNumber)
    {
    auto cache = GetTestCache();

    auto responseKey = StubCachedResponseKey(*cache);
    auto oldInstanceKey = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    auto target = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Target"});

    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relKey = cache->GetChangeManager().CreateRelationship(*relClass, oldInstanceKey, target);
    auto relChange = cache->GetChangeManager().GetRelationshipChange(relKey);

    StubInstances instances;
    instances.Add({"TestSchema.TestDerivedClass", "Foo"});
    bmap<ECInstanceKey, ECInstanceKey> changedKeys;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().UpdateCreatedInstance({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse(), changedKeys));

    auto newInstanceKey = cache->FindInstance({"TestSchema.TestDerivedClass", "Foo"});

    bvector<IChangeManager::RelationshipChange> changes;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().GetCreatedRelationships(newInstanceKey, changes));
    ASSERT_THAT(changes, SizeIs(1));
    EXPECT_THAT(changes[0].GetChangeNumber(), relChange.GetChangeNumber());
    EXPECT_THAT(changes[0].GetSourceKey(), newInstanceKey);
    EXPECT_THAT(changes[0].GetTargetKey(), relChange.GetTargetKey());
    EXPECT_THAT(changes[0].GetSyncStatus(), relChange.GetSyncStatus());
    EXPECT_THAT(changes[0].GetChangeStatus(), relChange.GetChangeStatus());

    EXPECT_THAT(changedKeys, SizeIs(2));
    EXPECT_THAT(changedKeys, Contains(StubBPair(oldInstanceKey, newInstanceKey)));
    EXPECT_THAT(changedKeys, Contains(StubBPair(relChange.GetInstanceKey(), changes[0].GetInstanceKey())));
    }

TEST_F(ChangeManagerTests, UpdateCreatedInstance_ChangedClassAndOldInstanceHasCreatedTargetRelationship_PreservesRelationshipWithSameChangeNumber)
    {
    auto cache = GetTestCache();

    auto responseKey = StubCachedResponseKey(*cache);
    auto oldInstanceKey = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    auto source = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Target"});

    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relKey = cache->GetChangeManager().CreateRelationship(*relClass, source, oldInstanceKey);
    auto relChange = cache->GetChangeManager().GetRelationshipChange(relKey);

    StubInstances instances;
    instances.Add({"TestSchema.TestDerivedClass", "Foo"});
    bmap<ECInstanceKey, ECInstanceKey> changedKeys;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().UpdateCreatedInstance({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse(), changedKeys));

    auto newInstanceKey = cache->FindInstance({"TestSchema.TestDerivedClass", "Foo"});

    bvector<IChangeManager::RelationshipChange> changes;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().GetCreatedRelationships(newInstanceKey, changes));
    ASSERT_THAT(changes, SizeIs(1));
    EXPECT_THAT(changes[0].GetChangeNumber(), relChange.GetChangeNumber());
    EXPECT_THAT(changes[0].GetSourceKey(), relChange.GetSourceKey());
    EXPECT_THAT(changes[0].GetTargetKey(), newInstanceKey);
    EXPECT_THAT(changes[0].GetSyncStatus(), relChange.GetSyncStatus());
    EXPECT_THAT(changes[0].GetChangeStatus(), relChange.GetChangeStatus());

    EXPECT_THAT(changedKeys, SizeIs(2));
    EXPECT_THAT(changedKeys, Contains(StubBPair(oldInstanceKey, newInstanceKey)));
    EXPECT_THAT(changedKeys, Contains(StubBPair(relChange.GetInstanceKey(), changes[0].GetInstanceKey())));
    }

TEST_F(ChangeManagerTests, UpdateCreatedInstance_OldInstanceExistsInCachedResponseWhenClassChanges_InvalidatesCachedResponse)
    {
    auto cache = GetTestCache();

    auto responseKey = StubCachedResponseKey(*cache);
    cache->LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass", "Foo"});
    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    instances.Add({"TestSchema.TestClass", "Other"});
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse("TestTag")));
    EXPECT_THAT(cache->ReadResponseCacheTag(responseKey), Eq("TestTag"));

    instances.Clear();
    instances.Add({"TestSchema.TestClass2", "Foo"});
    bmap<ECInstanceKey, ECInstanceKey> changedKeys;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().UpdateCreatedInstance({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse(), changedKeys));

    EXPECT_THAT(changedKeys, Not(IsEmpty()));
    EXPECT_THAT(cache->IsResponseCached(responseKey), true);
    EXPECT_THAT(cache->ReadResponseCacheTag(responseKey), IsEmpty());

    bset<ObjectId> responseObjectIds;
    cache->ReadResponseObjectIds(responseKey, responseObjectIds);

    EXPECT_THAT(responseObjectIds, SizeIs(1));
    EXPECT_THAT(responseObjectIds, Contains(ObjectId {"TestSchema.TestClass", "Other"}));
    }

TEST_F(ChangeManagerTests, CommitObjectChange_NonExistingObject_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto nonExistingInstance = StubNonExistingInstanceKey(*cache);
    // Act
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().CommitObjectChanges(nonExistingInstance);
    BeTest::SetFailOnAssert(true);
    // Assert
    ASSERT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, CommitObjectChange_NotChangedObject_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    // Act
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().CommitObjectChanges(instance);
    BeTest::SetFailOnAssert(true);
    // Assert
    ASSERT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, CommitObjectChange_CreatedObject_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = cache->GetChangeManager().CreateObject(*testClass, Json::objectValue);
    // Act
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().CommitObjectChanges(instance);
    BeTest::SetFailOnAssert(true);
    // Assert
    ASSERT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, CommitObjectChange_ModifiedObject_RemovesChangeStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, Json::objectValue));
    // Act
    auto status = cache->GetChangeManager().CommitObjectChanges(instance);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, CommitObjectChange_DeletedObject_RemovesChangeStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    // Act
    auto status = cache->GetChangeManager().CommitObjectChanges(instance);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, CommitObjectChange_DeletedRelationship_RemovesChangeStatus)
    {
    // Arrange
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    CachedResponseKey resultsKey(cache->FindOrCreateRoot(nullptr), "Test");
    ASSERT_EQ(SUCCESS, cache->CacheResponse(resultsKey, instances.ToWSObjectsResponse()));

    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = cache->FindRelationship(*relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"});

    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteRelationship(relationship));
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitObjectChanges(relationship));
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, CommitFileChange_NotChanged_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    // Act
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().CommitFileChanges(instance);
    BeTest::SetFailOnAssert(true);
    // Assert
    ASSERT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, CommitFileChange_ModifiedFile_RemovesChangeStatusAndMovesToTemporaryLocation)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, FSTest::StubFile(), false));
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitFileChanges(instance));
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetFileChange(instance).GetChangeStatus());
    EXPECT_TRUE(cache->ReadFilePath(instance).find(L"test_files_temporary") != BeFileName::npos);
    }

TEST_F(ChangeManagerTests, GetCreatedRelationships_NoChangedRelationshipsForThatInstance_ReturnsNone)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto source = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto target = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});
    ASSERT_TRUE(cache->GetChangeManager().CreateRelationship(*testRelClass, source, target, ChangeManager::SyncStatus::Ready).IsValid());
    auto otherInstance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    // Act
    bvector<IChangeManager::RelationshipChange> changes;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().GetCreatedRelationships(otherInstance, changes));
    // Assert
    ASSERT_TRUE(changes.empty());
    }

TEST_F(ChangeManagerTests, GetCreatedRelationships_CreatedRelationships_ReturnsRelationships)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto instanceA = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto instanceB = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});
    auto instanceC = StubInstanceInCache(*cache, {"TestSchema.TestClass", "C"});
    auto relationshipAB = cache->GetChangeManager().CreateRelationship(*testRelClass, instanceA, instanceB, ChangeManager::SyncStatus::Ready);
    auto relationshipAC = cache->GetChangeManager().CreateRelationship(*testRelClass, instanceA, instanceC, ChangeManager::SyncStatus::Ready);
    ASSERT_TRUE(relationshipAB.IsValid());
    ASSERT_TRUE(relationshipAC.IsValid());
    // Act
    bvector<IChangeManager::RelationshipChange> changes;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().GetCreatedRelationships(instanceA, changes));
    // Assert
    ASSERT_EQ(2, changes.size());
    EXPECT_NE(changes.end(), std::find_if(changes.begin(), changes.end(), [&] (IChangeManager::RelationshipChange change)
        {
        return change.GetInstanceKey() == relationshipAB;
        }));
    EXPECT_NE(changes.end(), std::find_if(changes.begin(), changes.end(), [&] (IChangeManager::RelationshipChange change)
        {
        return change.GetInstanceKey() == relationshipAC;
        }));
    }

TEST_F(ChangeManagerTests, GetCreatedRelationships_DeletedRelationship_DoesNotReturnDeletedRelationship)
    {
    // Arrange
    auto cache = GetTestCache();

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"}).AddRelated({"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "B"});
    ASSERT_EQ(SUCCESS, cache->CacheResponse(StubCachedResponseKey(*cache), instances.ToWSObjectsResponse()));

    auto relClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = cache->FindRelationship(*relClass, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"});
    auto source = cache->FindInstance({"TestSchema.TestClass", "A"});

    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteRelationship(relationship));
    ASSERT_TRUE(source.IsValid());
    // Act
    bvector<IChangeManager::RelationshipChange> changes;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().GetCreatedRelationships(source, changes));
    // Assert
    ASSERT_EQ(0, changes.size());
    }
#endif