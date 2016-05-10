/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/Persistence/ChangeManagerTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ChangeManagerTests.h"

#include "../Util/MockECDbSchemaChangeListener.h"

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

TEST_F(ChangeManagerTests, ModifyObject_SyncSetToActiveAndModifyingModifiedObject_Success)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"}, {{"TestProperty", "OldValue"}});
    // Act
    Json::Value properties;
    properties["TestProperty"] = "NewValue";
    cache->GetChangeManager().SetSyncActive(true);
    auto status = cache->GetChangeManager().ModifyObject(instance, properties);
    cache->GetChangeManager().SetSyncActive(false);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    Json::Value instanceJson;
    ASSERT_EQ(SUCCESS, cache->GetAdapter().GetJsonInstance(instanceJson, instance));
    EXPECT_EQ("NewValue", instanceJson["TestProperty"].asString());
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
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile(), false));
    // Act
    cache->GetChangeManager().SetSyncActive(true);
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().ModifyFile(instance, StubFile(), false);
    BeTest::SetFailOnAssert(true);
    cache->GetChangeManager().SetSyncActive(false);
    // Assert
    EXPECT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, GetLegacyParentRelationshipClass_InvalidClassIds_ReturnsInvalid)
    {
    auto cache = GetTestCache();
    EXPECT_EQ(nullptr, cache->GetChangeManager().GetLegacyParentRelationshipClass(ECClassId(UINT64_C(99999)), ECClassId(UINT64_C(99999))));
    }

TEST_F(ChangeManagerTests, GetLegacyParentRelationshipClass_SameParameters_ReturnsSameClass)
    {
    auto cache = CreateTestCache();

    ECClassId classId = cache->GetAdapter().GetECClass("TestSchema.TestClass")->GetId();
    auto relClass1 = cache->GetChangeManager().GetLegacyParentRelationshipClass(classId, classId);
    auto relClass2 = cache->GetChangeManager().GetLegacyParentRelationshipClass(classId, classId);

    ASSERT_EQ(relClass1, relClass2);
    ASSERT_TRUE(nullptr != relClass1);
    ASSERT_TRUE(nullptr != cache->GetAdapter().GetECRelationshipClass(relClass1->GetId()));
    EXPECT_TRUE(relClass1->GetSource().SupportsClass(*cache->GetAdapter().GetECClass(classId)));
    EXPECT_TRUE(relClass1->GetTarget().SupportsClass(*cache->GetAdapter().GetECClass(classId)));
    }

TEST_F(ChangeManagerTests, GetLegacyParentRelationshipClass_SameParameters_CallsOnSchemaChangedListenerOnlyFirstTime)
    {
    auto cache = CreateTestCache();
    ECClassId classId = cache->GetAdapter().GetECClass("TestSchema.TestClass")->GetId();

    MockECDbSchemaChangeListener listener;
    cache->RegisterSchemaChangeListener(&listener);

    EXPECT_CALL(listener, OnSchemaChanged()).Times(2);
    auto relClass1 = cache->GetChangeManager().GetLegacyParentRelationshipClass(classId, classId);

    EXPECT_CALL(listener, OnSchemaChanged()).Times(0);
    auto relClass2 = cache->GetChangeManager().GetLegacyParentRelationshipClass(classId, classId);

    ASSERT_EQ(relClass1, relClass2);
    ASSERT_TRUE(nullptr != relClass1);
    ASSERT_TRUE(nullptr != cache->GetAdapter().GetECRelationshipClass(relClass1->GetId()));
    }

TEST_F(ChangeManagerTests, GetLegacyParentRelationshipClass_DifferentParameters_ReturnsDifferentClasses)
    {
    auto cache = CreateTestCache();

    ECClassId classId1 = cache->GetAdapter().GetECClass("TestSchema.TestClass")->GetId();
    ECClassId classId2 = cache->GetAdapter().GetECClass("TestSchema.TestClass2")->GetId();
    ECClassId classId3 = cache->GetAdapter().GetECClass("TestSchema2.TestClass")->GetId();
    ECRelationshipClassCP relClass = nullptr;

    relClass = cache->GetChangeManager().GetLegacyParentRelationshipClass(classId1, classId2);
    ASSERT_TRUE(nullptr != relClass);
    ECClassId relClassId1 = relClass->GetId();

    relClass = cache->GetChangeManager().GetLegacyParentRelationshipClass(classId1, classId3);
    ASSERT_TRUE(nullptr != relClass);
    ECClassId relClassId2 = relClass->GetId();

    EXPECT_NE(relClassId1, relClassId2);

    classId1 = cache->GetAdapter().GetECClass("TestSchema.TestClass")->GetId();
    classId2 = cache->GetAdapter().GetECClass("TestSchema.TestClass2")->GetId();
    classId3 = cache->GetAdapter().GetECClass("TestSchema2.TestClass")->GetId();

    auto relClass1 = cache->GetAdapter().GetECRelationshipClass(relClassId1);
    ASSERT_TRUE(nullptr != relClass1);
    EXPECT_TRUE(relClass1->GetSource().SupportsClass(*cache->GetAdapter().GetECClass(classId1)));
    EXPECT_TRUE(relClass1->GetTarget().SupportsClass(*cache->GetAdapter().GetECClass(classId2)));

    auto relClass2 = cache->GetAdapter().GetECRelationshipClass(relClassId2);
    ASSERT_TRUE(nullptr != relClass2);
    EXPECT_TRUE(relClass2->GetSource().SupportsClass(*cache->GetAdapter().GetECClass(classId1)));
    EXPECT_TRUE(relClass2->GetTarget().SupportsClass(*cache->GetAdapter().GetECClass(classId3)));
    }

TEST_F(ChangeManagerTests, GetLegacyParentRelationshipClass_DifferentSchemasWithSameNamedClasses_ReturnsDifferentClasses)
    {
    auto cache = CreateTestCache();

    ECClassId classId1 = cache->GetAdapter().GetECClass("TestSchema.TestClass")->GetId();
    ECClassId classId2 = cache->GetAdapter().GetECClass("TestSchema2.TestClass")->GetId();
    ECRelationshipClassCP relClass = nullptr;

    relClass = cache->GetChangeManager().GetLegacyParentRelationshipClass(classId1, classId1);
    ASSERT_TRUE(nullptr != relClass);
    ECClassId relClassId1 = relClass->GetId();

    relClass = cache->GetChangeManager().GetLegacyParentRelationshipClass(classId1, classId2);
    ASSERT_TRUE(nullptr != relClass);
    ECClassId relClassId2 = relClass->GetId();

    EXPECT_NE(relClassId1, relClassId2);
    EXPECT_TRUE(nullptr != cache->GetAdapter().GetECRelationshipClass(relClassId1));
    EXPECT_TRUE(nullptr != cache->GetAdapter().GetECRelationshipClass(relClassId2));
    }

TEST_F(ChangeManagerTests, GetLegacyParentRelationshipClass_ExistingClasses_ReturnsValidRelationshipForCreateRelationship)
    {
    auto cache = CreateTestCache();

    ECClassId classId = cache->GetAdapter().GetECClass("TestSchema.TestClass")->GetId();
    auto relClass = cache->GetChangeManager().GetLegacyParentRelationshipClass(classId, classId);
    ASSERT_TRUE(nullptr != relClass);

    auto parent = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Parent"});
    auto child = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Child"});

    auto relationship = cache->GetChangeManager().CreateRelationship(*relClass, parent, child);
    ASSERT_TRUE(relationship.IsValid());

    ChangeManager::Changes changes;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().GetChanges(changes));

    auto& relChanges = changes.GetRelationshipChanges();
    ASSERT_EQ(1, relChanges.size());
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, relChanges.begin()->GetChangeStatus());
    EXPECT_TRUE(VerifyHasRelationship(cache, relClass, parent, child));
    }

TEST_F(ChangeManagerTests, GetLegacyParentRelationshipClass_GenerateNewClassFalse_DoesNotGenerateNewClassAndReturnsNullForNotExisting)
    {
    auto cache = CreateTestCache();
    ECClassId classId = cache->GetAdapter().GetECClass("TestSchema.TestClass")->GetId();
    ECRelationshipClassCP relClass = nullptr;

    MockECDbSchemaChangeListener listener;
    cache->RegisterSchemaChangeListener(&listener);

    EXPECT_CALL(listener, OnSchemaChanged()).Times(0);
    relClass = cache->GetChangeManager().GetLegacyParentRelationshipClass(classId, classId, false);
    EXPECT_TRUE(nullptr == relClass);

    EXPECT_CALL(listener, OnSchemaChanged()).Times(2);
    relClass = cache->GetChangeManager().GetLegacyParentRelationshipClass(classId, classId);
    EXPECT_TRUE(nullptr != relClass);
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
    ASSERT_EQ(SUCCESS, cache->GetAdapter().GetJsonInstance(instanceJson, instance));
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
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"}, {{"TestProperty", "OldValue"}});
    // Act
    Json::Value properties;
    properties["TestProperty"] = "NewValue";
    auto status = cache->GetChangeManager().ModifyObject(instance, properties);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    Json::Value instanceJson;
    ASSERT_EQ(SUCCESS, cache->GetAdapter().GetJsonInstance(instanceJson, instance));
    EXPECT_EQ("NewValue", instanceJson["TestProperty"].asString());
    }

TEST_F(ChangeManagerTests, ModifyObject_CreatedObject_SuccessAndLeavesStatusCreatedAndSameNumber)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
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
    EXPECT_EQ(0, CountClassInstances(*cache, "TestSchema.TestClass"));
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, DeleteObject_CreatedObject_RemovesInstanceAndMarksAsDeleted)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
    // Act
    auto status = cache->GetChangeManager().DeleteObject(instance);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(0, CountClassInstances(*cache, "TestSchema.TestClass"));
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
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
    EXPECT_EQ(0, CountClassInstances(*cache, "TestSchema.TestClass"));
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
    StubInstancesInCache(*cache, instances);
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
    auto relationship = StubRelationshipInCache(*cache, {"TestSchema.TestRelationshipClass", "AB"});
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteRelationship(relationship));
    // Assert
    EXPECT_EQ(0, CountClassInstances(*cache, "TestSchema.TestRelationshipClass"));
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    EXPECT_EQ(1, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeNumber());
    }

TEST_F(ChangeManagerTests, DeleteRelationship_CreatedRelationship_RemovesRelationshipAndMarksAsDeleted)
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
    EXPECT_EQ(0, CountClassInstances(*cache, "TestSchema.TestRelationshipClass"));
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, DeleteRelationship_DeletedRelationship_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto relationship = StubRelationshipInCache(*cache);
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
    auto status = cache->GetChangeManager().ModifyFile(nonExistingInstance, StubFile(), false);
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
    auto status = cache->GetChangeManager().ModifyFile(instance, StubFile("NewContent"), false);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetFileChange(instance).GetChangeStatus());
    EXPECT_TRUE(cache->ReadFilePath(instance).find(L"test_files_persistent") != BeFileName::npos);
    EXPECT_EQ(1, cache->GetChangeManager().GetFileChange(instance).GetChangeNumber());
    EXPECT_EQ("NewContent", SimpleReadFile(cache->ReadFilePath(instance)));
    }

TEST_F(ChangeManagerTests, ModifyFile_ExistingObjectAndCopyFileTrue_CopiesFileToPersistentLocationAndLeavesOriginal)
    {
    // Arrange
    auto cache = GetTestCache();
    ObjectId fileId {"TestSchema.TestClass", "Foo"};
    auto instance = StubInstanceInCache(*cache, fileId);
    // Act
    auto filePath = StubFile("NewContent");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, filePath, true));
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetFileChange(instance).GetChangeStatus());
    EXPECT_TRUE(cache->ReadFilePath(instance).find(L"test_files_persistent") != BeFileName::npos);
    EXPECT_EQ(1, cache->GetChangeManager().GetFileChange(instance).GetChangeNumber());
    EXPECT_EQ("NewContent", SimpleReadFile(cache->ReadFilePath(instance)));
    EXPECT_TRUE(filePath.DoesPathExist());
    }

TEST_F(ChangeManagerTests, ModifyFile_Twice_LeavesChangeNumberAndChangesContentAndRemovesOldFile)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile("A"), false));
    auto filePathA = cache->ReadFilePath(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile("B"), false));
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetFileChange(instance).GetChangeStatus());
    EXPECT_EQ(1, cache->GetChangeManager().GetFileChange(instance).GetChangeNumber());
    EXPECT_FALSE(filePathA.DoesPathExist());
    EXPECT_EQ("B", SimpleReadFile(cache->ReadFilePath(instance)));
    }

TEST_F(ChangeManagerTests, ModifyFile_ExistingFile_ReplacesFileContentAndSetsChangeStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->CacheFile({"TestSchema.TestClass", "Foo"}, StubWSFileResponse(StubFile("InitialContent")), FileCache::Persistent));
    // Act
    auto status = cache->GetChangeManager().ModifyFile(instance, StubFile("NewContent"), false);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetFileChange(instance).GetChangeStatus());
    EXPECT_EQ(1, cache->GetChangeManager().GetFileChange(instance).GetChangeNumber());
    EXPECT_EQ("NewContent", SimpleReadFile(cache->ReadFilePath(instance)));
    }

TEST_F(ChangeManagerTests, ModifyFileName_NotExistingInstance_Error)
    {
    auto cache = GetTestCache();

    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(ERROR, cache->GetChangeManager().ModifyFileName(StubECInstanceKey(), "Foo.txt"));
    BeTest::SetFailOnAssert(true);
    }

TEST_F(ChangeManagerTests, ModifyFileName_InstanceWithoutFile_Error)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});

    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(ERROR, cache->GetChangeManager().ModifyFileName(instance, "Foo.txt"));
    BeTest::SetFailOnAssert(true);
    }

TEST_F(ChangeManagerTests, ModifyFileName_InstanceWithNonModifiedFile_Error)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->CacheFile({"TestSchema.TestClass", "Foo"}, StubWSFileResponse(StubFile()), FileCache::Persistent));
    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(ERROR, cache->GetChangeManager().ModifyFileName(instance, "Foo.txt"));
    BeTest::SetFailOnAssert(true);
    }

TEST_F(ChangeManagerTests, ModifyFileName_InstanceWithModifiedFile_SuccessAndFileRenamed)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile("TestContent", "OldName.txt"), false));
    auto oldFilePath = cache->ReadFilePath(instance);
    ASSERT_TRUE(oldFilePath.DoesPathExist());
    // Act
    EXPECT_EQ(SUCCESS, cache->GetChangeManager().ModifyFileName(instance, "NewName.foo"));
    // Assert
    auto newFilePath = cache->ReadFilePath(instance);
    EXPECT_TRUE(newFilePath.DoesPathExist());
    EXPECT_FALSE(oldFilePath.DoesPathExist());
    EXPECT_EQ("TestContent", SimpleReadFile(newFilePath));
    EXPECT_EQ(L"NewName.foo", newFilePath.GetFileNameAndExtension());
    }

TEST_F(ChangeManagerTests, ModifyFileName_InstanceWithModifiedFileAndNameNotChanged_SuccessAndFilePathNotChanged)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile("TestContent", "OldName.txt"), false));
    auto oldFilePath = cache->ReadFilePath(instance);
    // Act
    EXPECT_EQ(SUCCESS, cache->GetChangeManager().ModifyFileName(instance, "OldName.txt"));
    // Assert
    auto newFilePath = cache->ReadFilePath(instance);
    EXPECT_EQ("TestContent", SimpleReadFile(newFilePath));
    EXPECT_EQ(oldFilePath, newFilePath);
    }

TEST_F(ChangeManagerTests, ModifyFileName_InstanceWithModifiedFileAndNewNameContainsInvalidSymbols_RenamesWithValidFileName)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile("TestContent", "OldName.txt"), false));
    // Act
    EXPECT_EQ(SUCCESS, cache->GetChangeManager().ModifyFileName(instance, R"(Foo\\/:#Boo.txt)"));
    // Assert
    auto newFilePath = cache->ReadFilePath(instance);
    EXPECT_EQ(L"Foo#Boo.txt", newFilePath.GetFileNameAndExtension());
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
    StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
    EXPECT_TRUE(cache->GetChangeManager().HasChanges());
    }

TEST_F(ChangeManagerTests, HasChanges_CreatedRelationship_True)
    {
    // Arrange
    auto cache = GetTestCache();
    StubCreatedRelationshipInCache(*cache);
    // Act & Assert
    EXPECT_TRUE(cache->GetChangeManager().HasChanges());
    }

TEST_F(ChangeManagerTests, HasChanges_ChangedFile_True)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile(), false));
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
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
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
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile(), false));
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
    auto instanceNotReady = StubCreatedObjectInCache(*cache, ChangeManager::SyncStatus::NotReady, "TestSchema.TestClass");
    auto instanceReady = StubCreatedObjectInCache(*cache, ChangeManager::SyncStatus::Ready, "TestSchema.TestClass");
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
    auto instanceA = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
    auto instanceB = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
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
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile(), false));
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
    auto relationship = StubCreatedRelationshipInCache(*cache);
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
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
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
    auto instance = StubCreatedObjectInCache(*cache, ChangeManager::SyncStatus::NotReady, "TestSchema.TestClass");
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
    auto relationship = StubRelationshipInCache(*cache, {"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"});
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
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile(), false));
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
    auto instance = StubCreatedObjectInCache(*cache, ChangeManager::SyncStatus::NotReady, "TestSchema.TestClass");
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
    auto relationship = StubCreatedRelationshipInCache(*cache);
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
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile(), false, ChangeManager::SyncStatus::NotReady));
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
    ASSERT_EQ(SUCCESS, cache->CacheInstanceAndLinkToRoot({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse(), "foo_root"));
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
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
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

TEST_F(ChangeManagerTests, DISABLED_GetObjectChangeStatus_ObjectDeleted_Deleted)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    // Act
    auto status = cache->GetChangeManager().GetObjectChangeStatus(instance);
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, status);
    }

TEST_F(ChangeManagerTests, DISABLED_GetObjectChangeStatus_CreatedObjectDeleted_Deleted)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    // Act
    auto status = cache->GetChangeManager().GetObjectChangeStatus(instance);
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, status);
    }

TEST_F(ChangeManagerTests, ReadInstanceRevision_NotExistingInstance_ReturnsInvalid)
    {
    // Arrange
    auto cache = GetTestCache();
    // Act
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto revision = cache->GetChangeManager().ReadInstanceRevision(StubECInstanceKey(testClass->GetId().GetValue(), 1));
    // Assert
    ASSERT_NE(nullptr, revision);
    EXPECT_FALSE(revision->IsValid());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, revision->GetChangeStatus());
    }

TEST_F(ChangeManagerTests, ReadInstanceRevision_NotChangedObject_ReturnsInvalidButFilled)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    // Assert
    ASSERT_NE(nullptr, revision);
    EXPECT_FALSE(revision->IsValid());
    EXPECT_EQ(instance, revision->GetInstanceKey());
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "Foo"), revision->GetObjectId());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, revision->GetChangeStatus());
    EXPECT_EQ(IChangeManager::SyncStatus::NotReady, revision->GetSyncStatus());
    EXPECT_EQ(nullptr, revision->GetChangedProperties());
    }

TEST_F(ChangeManagerTests, ReadInstanceRevision_CreatedObject_ReturnsValid)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    // Assert
    ASSERT_NE(nullptr, revision);
    EXPECT_TRUE(revision->IsValid());
    EXPECT_EQ(instance, revision->GetInstanceKey());
    EXPECT_TRUE(revision->GetObjectId().IsValid());
    EXPECT_EQ(cache->FindInstance(instance), revision->GetObjectId());
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, revision->GetChangeStatus());
    EXPECT_EQ(IChangeManager::SyncStatus::Ready, revision->GetSyncStatus());
    ASSERT_NE(nullptr, revision->GetChangedProperties());
    }

TEST_F(ChangeManagerTests, ReadInstanceRevision_CreatedObjectWithSyncStatusNotReady_ReturnsWithSameSyncStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache, IChangeManager::SyncStatus::NotReady, "TestSchema.TestClass");
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    // Assert
    ASSERT_NE(nullptr, revision);
    EXPECT_TRUE(revision->IsValid());
    EXPECT_EQ(instance, revision->GetInstanceKey());
    EXPECT_TRUE(revision->GetObjectId().IsValid());
    EXPECT_EQ(cache->FindInstance(instance), revision->GetObjectId());
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, revision->GetChangeStatus());
    EXPECT_EQ(IChangeManager::SyncStatus::NotReady, revision->GetSyncStatus());
    ASSERT_NE(nullptr, revision->GetChangedProperties());
    }

TEST_F(ChangeManagerTests, ReadInstanceRevision_CreatedObjectWithProperties_ReturnsAllNonNullProperties)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    Json::Value properties;
    properties["TestProperty2"] = "A";
    auto instance = cache->GetChangeManager().CreateObject(*testClass, Json::Value(properties));
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    // Assert
    ASSERT_NE(nullptr, revision);
    ASSERT_NE(nullptr, revision->GetChangedProperties());
    EXPECT_EQ(properties, *revision->GetChangedProperties());
    }

TEST_F(ChangeManagerTests, ReadInstanceRevision_CreatedObjectWithClassWithStructProperty_NullStructPropertyNotReturned_REGRESSION)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClassWithStruct");
    Json::Value properties;
    properties["TestProperty"] = "Foo";
    auto instance = cache->GetChangeManager().CreateObject(*testClass, Json::Value(properties));
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    // Assert
    ASSERT_NE(nullptr, revision);
    ASSERT_NE(nullptr, revision->GetChangedProperties());
    EXPECT_EQ(properties, *revision->GetChangedProperties());
    }

TEST_F(ChangeManagerTests, ReadInstanceRevision_CreatedObjectWithClassWithStructProperty_NonNullStructPropertyIsReturned_REGRESSION)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClassWithStruct");
    Json::Value properties;
    properties["TestProperty"] = "A";
    properties["TestStructProperty"]["TestStringProperty"] = "B";
    properties["TestStructProperty"]["TestArrayProperty"][0] = "C";
    properties["TestStructProperty"]["TestArrayProperty"][1] = "D";
    auto instance = cache->GetChangeManager().CreateObject(*testClass, Json::Value(properties));
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    // Assert
    ASSERT_NE(nullptr, revision);
    ASSERT_NE(nullptr, revision->GetChangedProperties());
    EXPECT_EQ(properties, *revision->GetChangedProperties());
    }

TEST_F(ChangeManagerTests, ReadInstanceRevision_ModifiedObject_ReturnsValid)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, Json::objectValue));
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    // Assert
    ASSERT_NE(nullptr, revision);
    EXPECT_TRUE(revision->IsValid());
    EXPECT_EQ(instance, revision->GetInstanceKey());
    EXPECT_TRUE(revision->GetObjectId().IsValid());
    EXPECT_EQ(cache->FindInstance(instance), revision->GetObjectId());
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, revision->GetChangeStatus());
    EXPECT_EQ(IChangeManager::SyncStatus::Ready, revision->GetSyncStatus());
    ASSERT_NE(nullptr, revision->GetChangedProperties());
    }

TEST_F(ChangeManagerTests, ReadInstanceRevision_ModifiedObjectWithSyncStatusNotReady_ReturnsWithSameStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, Json::objectValue, IChangeManager::SyncStatus::NotReady));
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    // Assert
    ASSERT_NE(nullptr, revision);
    EXPECT_TRUE(revision->IsValid());
    EXPECT_EQ(instance, revision->GetInstanceKey());
    EXPECT_TRUE(revision->GetObjectId().IsValid());
    EXPECT_EQ(cache->FindInstance(instance), revision->GetObjectId());
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, revision->GetChangeStatus());
    EXPECT_EQ(IChangeManager::SyncStatus::NotReady, revision->GetSyncStatus());
    ASSERT_NE(nullptr, revision->GetChangedProperties());
    }

TEST_F(ChangeManagerTests, ReadInstanceRevision_ModifiedObjectWithProperties_ReturnsModifiedPropertiesOnly)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"}, {{"TestProperty", "A"}, {"TestProperty2", "B"}});
    Json::Value properties;
    properties["TestProperty"] = "A";
    properties["TestProperty2"] = "NewValue";
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, Json::Value(properties)));
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    // Assert
    ASSERT_NE(nullptr, revision);
    ASSERT_NE(nullptr, revision->GetChangedProperties());
    Json::Value expected;
    expected["TestProperty2"] = "NewValue";
    EXPECT_EQ(expected, *revision->GetChangedProperties());
    }

TEST_F(ChangeManagerTests, ReadInstanceRevision_DeletedObject_ReturnsValid)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    // Assert
    ASSERT_NE(nullptr, revision);
    EXPECT_TRUE(revision->IsValid());
    EXPECT_EQ(instance, revision->GetInstanceKey());
    EXPECT_TRUE(revision->GetObjectId().IsValid());
    EXPECT_EQ(cache->FindInstance(instance), revision->GetObjectId());
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, revision->GetChangeStatus());
    EXPECT_EQ(IChangeManager::SyncStatus::Ready, revision->GetSyncStatus());
    ASSERT_EQ(nullptr, revision->GetChangedProperties());
    }

TEST_F(ChangeManagerTests, ReadInstanceRevision_DeletedObjectWithSyncStatusNotReady_ReturnsWithSameStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance, IChangeManager::SyncStatus::NotReady));
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    // Assert
    ASSERT_NE(nullptr, revision);
    EXPECT_TRUE(revision->IsValid());
    EXPECT_EQ(instance, revision->GetInstanceKey());
    EXPECT_TRUE(revision->GetObjectId().IsValid());
    EXPECT_EQ(cache->FindInstance(instance), revision->GetObjectId());
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, revision->GetChangeStatus());
    EXPECT_EQ(IChangeManager::SyncStatus::NotReady, revision->GetSyncStatus());
    ASSERT_EQ(nullptr, revision->GetChangedProperties());
    }

TEST_F(ChangeManagerTests, ReadInstanceRevision_CreatedRelationship_ReturnsValid)
    {
    // Arrange
    auto cache = GetTestCache();
    auto relationship = StubCreatedRelationshipInCache(*cache, IChangeManager::SyncStatus::Ready);
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(relationship);
    // Assert
    ASSERT_NE(nullptr, revision);
    EXPECT_TRUE(revision->IsValid());
    EXPECT_EQ(relationship, revision->GetInstanceKey());
    EXPECT_TRUE(revision->GetObjectId().IsValid());
    EXPECT_EQ(cache->FindRelationship(relationship), revision->GetObjectId());
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, revision->GetChangeStatus());
    EXPECT_EQ(IChangeManager::SyncStatus::Ready, revision->GetSyncStatus());
    ASSERT_EQ(nullptr, revision->GetChangedProperties());
    }

TEST_F(ChangeManagerTests, ReadInstanceRevision_CreatedRelationshipWithSyncStatusNotReady_ReturnsWithSameStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto relationship = StubCreatedRelationshipInCache(*cache, IChangeManager::SyncStatus::NotReady);
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(relationship);
    // Assert
    ASSERT_NE(nullptr, revision);
    EXPECT_TRUE(revision->IsValid());
    EXPECT_EQ(relationship, revision->GetInstanceKey());
    EXPECT_TRUE(revision->GetObjectId().IsValid());
    EXPECT_EQ(cache->FindRelationship(relationship), revision->GetObjectId());
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, revision->GetChangeStatus());
    EXPECT_EQ(IChangeManager::SyncStatus::NotReady, revision->GetSyncStatus());
    ASSERT_EQ(nullptr, revision->GetChangedProperties());
    }

TEST_F(ChangeManagerTests, ReadInstanceRevision_DeletedRelationship_ReturnsValid)
    {
    // Arrange
    auto cache = GetTestCache();
    auto relationship = StubRelationshipInCache(*cache);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteRelationship(relationship));
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(relationship);
    // Assert
    ASSERT_NE(nullptr, revision);
    EXPECT_TRUE(revision->IsValid());
    EXPECT_EQ(relationship, revision->GetInstanceKey());
    EXPECT_TRUE(revision->GetObjectId().IsValid());
    EXPECT_EQ(cache->FindRelationship(relationship), revision->GetObjectId());
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, revision->GetChangeStatus());
    EXPECT_EQ(IChangeManager::SyncStatus::Ready, revision->GetSyncStatus());
    ASSERT_EQ(nullptr, revision->GetChangedProperties());
    }

TEST_F(ChangeManagerTests, ReadInstanceRevision_DeletedRelationshipWithSyncStatusNotReady_ReturnsWithSameStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto relationship = StubRelationshipInCache(*cache);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteRelationship(relationship, ChangeManager::SyncStatus::NotReady));
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(relationship);
    // Assert
    ASSERT_NE(nullptr, revision);
    EXPECT_TRUE(revision->IsValid());
    EXPECT_EQ(relationship, revision->GetInstanceKey());
    EXPECT_TRUE(revision->GetObjectId().IsValid());
    EXPECT_EQ(cache->FindRelationship(relationship), revision->GetObjectId());
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, revision->GetChangeStatus());
    EXPECT_EQ(IChangeManager::SyncStatus::NotReady, revision->GetSyncStatus());
    ASSERT_EQ(nullptr, revision->GetChangedProperties());
    }

TEST_F(ChangeManagerTests, ReadInstanceRevision_DeletedCreatedRelationship_ReturnsValid)
    {
    // Arrange
    auto cache = GetTestCache();
    auto relationship = StubCreatedRelationshipInCache(*cache, IChangeManager::SyncStatus::NotReady);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteRelationship(relationship));
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(relationship);
    // Assert
    ASSERT_NE(nullptr, revision);
    EXPECT_TRUE(revision->IsValid());
    EXPECT_EQ(relationship, revision->GetInstanceKey());
    EXPECT_TRUE(revision->GetObjectId().IsValid());
    EXPECT_EQ(cache->FindRelationship(relationship), revision->GetObjectId());
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, revision->GetChangeStatus());
    EXPECT_EQ(IChangeManager::SyncStatus::Ready, revision->GetSyncStatus());
    ASSERT_EQ(nullptr, revision->GetChangedProperties());
    }

TEST_F(ChangeManagerTests, ReadFileRevision_NotChanged_ReturnsInvalid)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    // Act
    BeTest::SetFailOnAssert(false);
    auto revision = cache->GetChangeManager().ReadFileRevision(instance);
    BeTest::SetFailOnAssert(true);
    // Assert
    ASSERT_NE(nullptr, revision);
    EXPECT_FALSE(revision->IsValid());
    EXPECT_EQ(instance, revision->GetInstanceKey());
    EXPECT_TRUE(revision->GetObjectId().IsValid());
    EXPECT_EQ(cache->FindInstance(instance), revision->GetObjectId());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, revision->GetChangeStatus());
    EXPECT_EQ(IChangeManager::SyncStatus::NotReady, revision->GetSyncStatus());
    EXPECT_EQ(L"", revision->GetFilePath());
    }

TEST_F(ChangeManagerTests, ReadFileRevision_ModifiedFile_ReturnsValidWithFilePath)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile(), false));
    // Act
    auto revision = cache->GetChangeManager().ReadFileRevision(instance);
    // Assert
    ASSERT_NE(nullptr, revision);
    EXPECT_TRUE(revision->IsValid());
    EXPECT_EQ(instance, revision->GetInstanceKey());
    EXPECT_TRUE(revision->GetObjectId().IsValid());
    EXPECT_EQ(cache->FindInstance(instance), revision->GetObjectId());
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, revision->GetChangeStatus());
    EXPECT_EQ(IChangeManager::SyncStatus::Ready, revision->GetSyncStatus());
    EXPECT_NE(L"", revision->GetFilePath());
    EXPECT_EQ(cache->ReadFilePath(instance), revision->GetFilePath());
    }

TEST_F(ChangeManagerTests, ReadFileRevision_ModifiedFileWithSyncStatusNotReady_ReturnsWithSameStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile(), false, ChangeManager::SyncStatus::NotReady));
    // Act
    auto revision = cache->GetChangeManager().ReadFileRevision(instance);
    // Assert
    ASSERT_NE(nullptr, revision);
    EXPECT_TRUE(revision->IsValid());
    EXPECT_EQ(instance, revision->GetInstanceKey());
    EXPECT_TRUE(revision->GetObjectId().IsValid());
    EXPECT_EQ(cache->FindInstance(instance), revision->GetObjectId());
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, revision->GetChangeStatus());
    EXPECT_EQ(IChangeManager::SyncStatus::NotReady, revision->GetSyncStatus());
    EXPECT_NE(L"", revision->GetFilePath());
    EXPECT_EQ(cache->ReadFilePath(instance), revision->GetFilePath());
    }

TEST_F(ChangeManagerTests, CommitLocalDeletions_NoChanges_Success)
    {
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitLocalDeletions());
    }

TEST_F(ChangeManagerTests, CommitLocalDeletions_CreatedDeletedObject_ChangeRemoved)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitLocalDeletions());
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, CommitLocalDeletions_CreatedObjectDeletedAndCommitedWithCreationRevision_DeletionChangeNotRemoved)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    revision->SetRemoteId("Foo");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitLocalDeletions());
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, CommitLocalDeletions_ExistingDeletedObject_ChangeNotRemoved)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitLocalDeletions());
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, CommitLocalDeletions_CreatedDeletedRelationship_ChangeRemoved)
    {
    // Arrange
    auto cache = GetTestCache();
    auto relationship = StubCreatedRelationshipInCache(*cache);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteRelationship(relationship));
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitLocalDeletions());
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, CommitLocalDeletions_CreatedRelationshipDeletedAndCommitedWithCreationRevision_DeletionChangeNotRemoved)
    {
    // Arrange
    auto cache = GetTestCache();
    auto relationship = StubCreatedRelationshipInCache(*cache);
    auto revision = cache->GetChangeManager().ReadInstanceRevision(relationship);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteRelationship(relationship));
    revision->SetRemoteId("Foo");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitLocalDeletions());
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, CommitLocalDeletions_ExistingDeletedRelationship_ChangeNotRemoved)
    {
    // Arrange
    auto cache = GetTestCache();
    auto relationship = StubRelationshipInCache(*cache);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteRelationship(relationship));
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitLocalDeletions());
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, CommitLocalDeletions_CreatedObjectComitedAndDeleted_DoesNotTreatItAsLocal)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    revision->SetRemoteId("NewId");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitLocalDeletions());
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_InvalidRevision_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    IChangeManager::InstanceRevision revision;
    // Act
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().CommitInstanceRevision(revision);
    BeTest::SetFailOnAssert(true);
    // Assert
    EXPECT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_NonExistingObject_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto revision = cache->GetChangeManager().ReadInstanceRevision(StubECInstanceKey(testClass->GetId().GetValue(), 1));
    // Act
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().CommitInstanceRevision(*revision);
    BeTest::SetFailOnAssert(true);
    // Assert
    ASSERT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_NotChangedObject_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    // Act
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().CommitInstanceRevision(*revision);
    BeTest::SetFailOnAssert(true);
    // Assert
    ASSERT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_CreatedObjectIdIsNotChanged_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    // Act
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().CommitInstanceRevision(*revision);
    BeTest::SetFailOnAssert(true);
    // Assert
    ASSERT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_CreatedObjectIdIsEmpty_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    // Act
    revision->SetRemoteId("");
    BeTest::SetFailOnAssert(false);
    auto status = cache->GetChangeManager().CommitInstanceRevision(*revision);
    BeTest::SetFailOnAssert(true);
    // Assert
    ASSERT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_CreatedObject_RemovesChangeStatusAndChangesRemoteId)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    // Act
    revision->SetRemoteId("NewId");
    auto status = cache->GetChangeManager().CommitInstanceRevision(*revision);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_TRUE(cache->FindInstance({"TestSchema.TestClass", "NewId"}).IsValid());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetCachedObjectInfo(instance).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_CreatedObjectWithFile_DoesNotCommitFileRevision)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile(), false));
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    // Act
    revision->SetRemoteId("NewId");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetCachedObjectInfo(instance).GetChangeStatus());
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetFileChange(instance).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_CreatedObjectThatWasAddedToResponses_RemovesItFromResponses)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache);
    auto responseKey1 = StubCachedResponseKey(*cache);
    auto responseKey2 = StubCachedResponseKey(*cache);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().AddCreatedInstanceToResponse(responseKey1, instance));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().AddCreatedInstanceToResponse(responseKey2, instance));

    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    // Act
    revision->SetRemoteId("NewId");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetCachedObjectInfo(instance).GetChangeStatus());

    ECInstanceKeyMultiMap instances;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(responseKey1, instances));
    EXPECT_NCONTAIN(instances, ECDbHelper::ToPair(instance));
    ASSERT_TRUE(cache->IsResponseCached(responseKey1));

    instances.clear();
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(responseKey2, instances));
    EXPECT_NCONTAIN(instances, ECDbHelper::ToPair(instance));
    ASSERT_TRUE(cache->IsResponseCached(responseKey2));
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_NewRemoteIdExistsInCache_RemovesOldInstanceAndMovesRootRelationships)
    {
    // Arrange
    auto cache = GetTestCache();
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass", "Foo"}));
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    // Act
    revision->SetRemoteId("Foo");
    auto status = cache->GetChangeManager().CommitInstanceRevision(*revision);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(instance, cache->FindInstance({"TestSchema.TestClass", "Foo"}));
    EXPECT_EQ(1, CountClassInstances(*cache, "TestSchema.TestClass"));
    EXPECT_TRUE(cache->IsInstanceInRoot("TestRoot", cache->FindInstance({"TestSchema.TestClass", "Foo"})));
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_NewRemoteIdExistsInCacheAndInCachedResponse_InvalidatesCachedResponse)
    {
    // Arrange
    auto cache = GetTestCache();

    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"});
    instances.Add({"TestSchema.TestClass", "Other"});

    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey, instances.ToWSObjectsResponse("TestTag")));
    EXPECT_THAT(cache->ReadResponseCacheTag(responseKey), Eq("TestTag"));

    auto newInstance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
    auto revision = cache->GetChangeManager().ReadInstanceRevision(newInstance);

    // Act
    revision->SetRemoteId("Foo");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));

    // Assert
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "Foo"}), newInstance);
    EXPECT_THAT(cache->IsResponseCached(responseKey), true);
    EXPECT_THAT(cache->ReadResponseCacheTag(responseKey), IsEmpty());

    bset<ObjectId> responseObjectIds;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseObjectIds(responseKey, responseObjectIds));

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
    auto relationship = cache->GetChangeManager().CreateRelationship(*testRelClass, source, target);
    auto revision = cache->GetChangeManager().ReadInstanceRevision(relationship);
    // Act
    revision->SetRemoteId("NewId");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Assert
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
    auto relationship = cache->GetChangeManager().CreateRelationship(*testRelClass, source, target);
    auto revision = cache->GetChangeManager().ReadInstanceRevision(relationship);
    // Act
    revision->SetRemoteId("");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Assert
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
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance(objectId, jsonInstance));
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
    ASSERT_EQ(CacheStatus::OK, cache->ReadInstance({"TestSchema.TestClass2", "Foo"}, jsonInstance));
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
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass", "Foo"}));
    auto oldInstanceKey = cache->FindInstance({"TestSchema.TestClass", "Foo"});

    StubInstances instances;
    instances.Add({"TestSchema.TestClass2", "Foo"});
    bmap<ECInstanceKey, ECInstanceKey> changedKeys;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().UpdateCreatedInstance({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse(), changedKeys));

    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass", "Foo"}).IsValid(), false);
    EXPECT_THAT(cache->FindInstance({"TestSchema.TestClass2", "Foo"}).IsValid(), true);
    EXPECT_EQ(0, CountClassInstances(*cache, "TestSchema.TestClass"));
    EXPECT_EQ(1, CountClassInstances(*cache, "TestSchema.TestClass2"));
    EXPECT_TRUE(cache->IsInstanceInRoot("TestRoot", cache->FindInstance({"TestSchema.TestClass2", "Foo"})));

    EXPECT_THAT(changedKeys, SizeIs(1));
    EXPECT_THAT(changedKeys, Contains(StubBPair(oldInstanceKey, cache->FindInstance({"TestSchema.TestClass2", "Foo"}))));
    }

TEST_F(ChangeManagerTests, UpdateCreatedInstance_OldInstanceExistsInCachedResponseWithSameClassAndId_InvalidatesCachedResponse)
    {
    auto cache = GetTestCache();

    auto responseKey = StubCachedResponseKey(*cache);
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass", "Foo"}));
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
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseObjectIds(responseKey, responseObjectIds));

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
    ASSERT_EQ(SUCCESS, cache->LinkInstanceToRoot("TestRoot", {"TestSchema.TestClass", "Foo"}));
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
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseObjectIds(responseKey, responseObjectIds));

    EXPECT_THAT(responseObjectIds, SizeIs(1));
    EXPECT_THAT(responseObjectIds, Contains(ObjectId {"TestSchema.TestClass", "Other"}));
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_ModifiedObject_RemovesChangeStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, Json::objectValue));
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    auto status = cache->GetChangeManager().CommitInstanceRevision(*revision);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_MultipleModifiedObjects_CommitsOnlySpecified)
    {
    // Arrange
    auto cache = GetTestCache();
    auto a = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"}, {{"TestProperty", "A"}});
    auto b = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"}, {{"TestProperty", "B"}});
    auto c = StubInstanceInCache(*cache, {"TestSchema.TestClass", "C"}, {{"TestProperty", "C"}});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(a, ToJson(R"({"TestProperty":"A"})")));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(b, ToJson(R"({"TestProperty":"B"})")));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(c, ToJson(R"({"TestProperty":"C"})")));

    EXPECT_EQ(Json::Value::null, ReadModifiedProperties(*cache, a)["TestProperty"]);
    EXPECT_EQ(Json::Value::null, ReadModifiedProperties(*cache, b)["TestProperty"]);
    EXPECT_EQ(Json::Value::null, ReadModifiedProperties(*cache, c)["TestProperty"]);
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(b);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Assert
    EXPECT_EQ(Json::Value::null, ReadModifiedProperties(*cache, a)["TestProperty"]);
    EXPECT_EQ(Json::Value::null, ReadModifiedProperties(*cache, c)["TestProperty"]);
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_DeletedObject_RemovesChangeStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    auto status = cache->GetChangeManager().CommitInstanceRevision(*revision);
    // Assert
    ASSERT_EQ(SUCCESS, status);
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_DeletedRelationship_RemovesChangeStatus)
    {
    // Arrange
    auto cache = GetTestCache();
    auto relationship = StubRelationshipInCache(*cache);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteRelationship(relationship));
    // Act
    auto revision = cache->GetChangeManager().ReadInstanceRevision(relationship);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    }

TEST_F(ChangeManagerTests, CommitFileRevision_NotChanged_Error)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    // Act
    BeTest::SetFailOnAssert(false);
    auto revision = cache->GetChangeManager().ReadFileRevision(instance);
    auto status = cache->GetChangeManager().CommitFileRevision(*revision);
    BeTest::SetFailOnAssert(true);
    // Assert
    ASSERT_EQ(ERROR, status);
    }

TEST_F(ChangeManagerTests, CommitFileRevision_ModifiedFile_RemovesChangeStatusAndMovesToTemporaryLocation)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile(), false));
    // Act
    auto revision = cache->GetChangeManager().ReadFileRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitFileRevision(*revision));
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetFileChange(instance).GetChangeStatus());
    EXPECT_TRUE(cache->ReadFilePath(instance).find(L"test_files_temporary") != BeFileName::npos);
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_ModifiedObjectModifiedAfterRevisionWasRead_PreservesNewChangesAndLeavesAsModified)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCacheJson(*cache, {"TestSchema.TestClass", "Foo"}, ToJson(R"({"TestProperty":"A1", "TestProperty2":"B1", "TestProperty3":"C1"})"));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, ToJson(R"({"TestProperty":"A1", "TestProperty2":"B2", "TestProperty3":"C1"})")));
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, ToJson(R"({"TestProperty":"A1", "TestProperty2":"B2", "TestProperty3":"C2"})")));
    ASSERT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Assert
    ASSERT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    Json::Value properties;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ReadModifiedProperties(instance, properties));
    EXPECT_EQ(ToJson(R"({"TestProperty3":"C2"})"), properties);
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_ModifiedObjectModifiedAfterRevisionWasRead_PreservesNewChangesThatChangedSameProperties)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, ToJson(R"({"TestProperty":"A1", "TestProperty2":"B1"})")));
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, ToJson(R"({"TestProperty":"A2", "TestProperty2":"B2"})")));
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Assert
    ASSERT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    Json::Value properties;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ReadModifiedProperties(instance, properties));
    EXPECT_EQ(ToJson(R"({"TestProperty":"A2", "TestProperty2":"B2"})"), properties);
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_ModifiedObjectModifiedAfterRevisionWasReadAndCommittedSecondTime_LeavesAsNoChange)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, ToJson(R"({"TestProperty":"A"})")));
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, ToJson(R"({"TestProperty":"B"})")));
    ASSERT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    ASSERT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    // Act
    revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Assert
    ASSERT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    Json::Value properties;
    ASSERT_EQ(SUCCESS, cache->GetAdapter().GetJsonInstance(properties, instance));
    EXPECT_EQ("B", properties["TestProperty"].asString());
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_ModifiedObjectDeletedAfterRevisionWasRead_LeavesAsDeleted)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, Json::objectValue));
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    ASSERT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Assert
    ASSERT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    EXPECT_EQ(0, CountClassInstances(*cache, "TestSchema.TestClass"));
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_ModifiedObjectDeletedAfterRevisionWasReadAndCommittedSecondTime_LeavesAsNoChange)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, Json::objectValue));
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    ASSERT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Act
    revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Assert
    ASSERT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    EXPECT_EQ(0, CountClassInstances(*cache, "TestSchema.TestClass"));
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_CreatedObjectModifiedAfterRevisionWasRead_PreservesNewChangesAndLeavesAsModified)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = cache->GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"A1", "TestProperty2":"B1"})"));
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, ToJson(R"({"TestProperty":"A1", "TestProperty2":"B2"})")));
    ASSERT_EQ(IChangeManager::ChangeStatus::Created, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    // Act
    revision->SetRemoteId("NewId");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Assert
    ASSERT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    Json::Value properties;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ReadModifiedProperties(instance, properties));
    EXPECT_EQ(ToJson(R"({"TestProperty2":"B2"})"), properties);
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_CreatedObjectModifiedAfterRevisionWasReadAndCommitedSecondTime_LeavesAsNoChange)
    {
    // Arrange
    auto cache = GetTestCache();
    auto testClass = cache->GetAdapter().GetECClass("TestSchema.TestClass");
    auto instance = cache->GetChangeManager().CreateObject(*testClass, ToJson(R"({"TestProperty":"A1", "TestProperty2":"B1"})"));
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, ToJson(R"({"TestProperty":"A1", "TestProperty2":"B2"})")));
    ASSERT_EQ(IChangeManager::ChangeStatus::Created, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    revision->SetRemoteId("NewId");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    ASSERT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    // Act
    revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Assert
    ASSERT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "NewId"}).IsInCache());
    Json::Value properties;
    ASSERT_EQ(SUCCESS, cache->GetAdapter().GetJsonInstance(properties, instance));
    EXPECT_EQ("A1", properties["TestProperty"].asString());
    EXPECT_EQ("B2", properties["TestProperty2"].asString());
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_CreatedObjectDeletedAfterRevisionWasRead_LeavesAsDeleted)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    // Act
    revision->SetRemoteId("NewId");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Assert
    ASSERT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    EXPECT_EQ(0, CountClassInstances(*cache, "TestSchema.TestClass"));
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_CreatedObjectDeletedAfterRevisionWasReadAndCommitedSecondTime_LeavesAsNoChange)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");
    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    revision->SetRemoteId("NewId");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Act
    revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Assert
    ASSERT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    EXPECT_EQ(0, CountClassInstances(*cache, "TestSchema.TestClass"));
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_CreatedRelationshipDeletedAfterRevisionWasRead_LeavesAsDeleted)
    {
    // Arrange
    auto cache = GetTestCache();
    auto source = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto target = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});
    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = cache->GetChangeManager().CreateRelationship(*testRelClass, source, target);
    auto revision = cache->GetChangeManager().ReadInstanceRevision(relationship);
    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass, source, target));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteRelationship(relationship));
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    // Act
    revision->SetRemoteId("NewId");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Assert
    ASSERT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    EXPECT_FALSE(VerifyHasRelationship(cache, testRelClass, source, target));
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_CreatedRelationshipDeletedAfterRevisionWasReadAndCommitedSecondTime_LeavesAsNoChange)
    {
    // Arrange
    auto cache = GetTestCache();
    auto source = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"});
    auto target = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"});
    auto testRelClass = cache->GetAdapter().GetECRelationshipClass("TestSchema.TestRelationshipClass");
    auto relationship = cache->GetChangeManager().CreateRelationship(*testRelClass, source, target);
    auto revision = cache->GetChangeManager().ReadInstanceRevision(relationship);
    EXPECT_TRUE(VerifyHasRelationship(cache, testRelClass, source, target));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteRelationship(relationship));
    EXPECT_EQ(IChangeManager::ChangeStatus::Deleted, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    revision->SetRemoteId("NewId");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Act
    revision = cache->GetChangeManager().ReadInstanceRevision(relationship);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    // Assert
    ASSERT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetRelationshipChange(relationship).GetChangeStatus());
    EXPECT_FALSE(VerifyHasRelationship(cache, testRelClass, source, target));
    }

TEST_F(ChangeManagerTests, CommitFileRevision_ModifiedFileModifiedAfterRevisionWasRead_PreservesNewFileAndLeavesAsModified)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile("A"), false));
    auto filePathA = cache->ReadFilePath(instance);
    ASSERT_EQ("A", SimpleReadFile(filePathA));
    auto revision = cache->GetChangeManager().ReadFileRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile("B"), false));
    ASSERT_FALSE(filePathA.DoesPathExist());
    ASSERT_EQ("B", SimpleReadFile(cache->ReadFilePath(instance)));
    ASSERT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetFileChange(instance).GetChangeStatus());
    // Act 
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitFileRevision(*revision));
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetFileChange(instance).GetChangeStatus());
    EXPECT_EQ("B", SimpleReadFile(cache->ReadFilePath(instance)));
    }

TEST_F(ChangeManagerTests, CommitFileRevision_ModifiedFileModifiedAfterRevisionWasReadAndCommitedSecondTime_LeavesAsNoChange)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile("A"), false));
    auto revision = cache->GetChangeManager().ReadFileRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyFile(instance, StubFile("B"), false));
    ASSERT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetFileChange(instance).GetChangeStatus());
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitFileRevision(*revision));
    ASSERT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetFileChange(instance).GetChangeStatus());
    // Act 
    revision = cache->GetChangeManager().ReadFileRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitFileRevision(*revision));
    // Assert
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetFileChange(instance).GetChangeStatus());
    EXPECT_EQ("B", SimpleReadFile(cache->ReadFilePath(instance)));
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
    auto relationship = StubRelationshipInCache(*cache, {"TestSchema.TestRelationshipClass", "AB"}, {"TestSchema.TestClass", "A"}, {"TestSchema.TestClass", "B"});
    auto source = cache->FindInstance({"TestSchema.TestClass", "A"});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteRelationship(relationship));
    ASSERT_TRUE(source.IsValid());
    // Act
    bvector<IChangeManager::RelationshipChange> changes;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().GetCreatedRelationships(source, changes));
    // Assert
    ASSERT_EQ(0, changes.size());
    }

TEST_F(ChangeManagerTests, ReadModifiedProperties_ModifiedPropertyInstance_ReturnsChangedPropertiesOnly)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"}, {{"TestProperty", "A"}, {"TestProperty2", "B"}});
    Json::Value properties;
    properties["TestProperty"] = "A";
    properties["TestProperty2"] = "ModifiedValue";
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, properties));

    Json::Value modifiedProperties;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ReadModifiedProperties(instance, modifiedProperties));

    Json::Value expected;
    expected["TestProperty2"] = "ModifiedValue";
    EXPECT_EQ(expected, modifiedProperties);
    }

TEST_F(ChangeManagerTests, ReadModifiedProperties_MultipleModifiedInstancesWithSameValues_ReturnsNoChanges)
    {
    // Arrange
    auto cache = GetTestCache();
    auto a = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"}, {{"TestProperty", "A"}});
    auto b = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"}, {{"TestProperty", "B"}});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(a, ToJson(R"({"TestProperty":"A"})")));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(b, ToJson(R"({"TestProperty":"B"})")));
    // Assert
    Json::Value ca, cb;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ReadModifiedProperties(a, ca));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ReadModifiedProperties(b, cb));
    EXPECT_EQ(Json::Value::null, ca["TestProperty"]);
    EXPECT_EQ(Json::Value::null, cb["TestProperty"]);
    }

TEST_F(ChangeManagerTests, ReadModifiedProperties_ModifiedProperty2Instance_ReturnsChangedPropertiesOnly)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"}, {{"TestProperty", "A"}, {"TestProperty2", "B"}});
    Json::Value properties;
    properties["TestProperty"] = "ModifiedValue";
    properties["TestProperty2"] = "B";
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, properties));

    Json::Value modifiedProperties;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ReadModifiedProperties(instance, modifiedProperties));

    Json::Value expected;
    expected["TestProperty"] = "ModifiedValue";
    EXPECT_EQ(expected, modifiedProperties);
    }

TEST_F(ChangeManagerTests, ReadModifiedProperties_ModifiedTwiceToOriginalVersion_ReturnsNoModifiedProperties)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"}, {{"TestProperty", "OriginalValue"}});

    Json::Value properties;
    properties["TestProperty"] = "SomeOtherValue";
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, properties));

    properties["TestProperty"] = "OriginalValue";
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, properties));

    Json::Value modifiedProperties;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ReadModifiedProperties(instance, modifiedProperties));

    Json::Value expected;
    expected["TestProperty"] = "OriginalValue";
    EXPECT_EQ(expected, properties);
    }

TEST_F(ChangeManagerTests, ReadModifiedProperties_ModifiedInstanceLabel_ReturnsChangedPropertiesWithoutECJsonProperties)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestLabeledClass", "Foo"}, {{"Name", "Old"}});
    Json::Value properties;
    properties["Name"] = "New";
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, properties));

    Json::Value modifiedProperties;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ReadModifiedProperties(instance, modifiedProperties));

    Json::Value expected;
    expected["Name"] = "New";
    EXPECT_EQ(expected, modifiedProperties);
    }

TEST_F(ChangeManagerTests, ReadModifiedProperties_CachedNewInstanceAfterModification_ReturnsChangesBetweenLatestAndLocalVersions)
    {
    // Arrange
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"}, {{"TestProperty", "OldA"}, {"TestProperty2", "OldB"}, {"TestProperty3", "OldC"}});
    auto properties = ToJson(R"({"TestProperty":"OldA", "TestProperty2":"NewB", "TestProperty3":"OtherC"})");
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, properties));

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "Foo"}, {{"TestProperty", "NewA"}, {"TestProperty2", "NewB"}, {"TestProperty3", "NewC"}});
    ASSERT_EQ(SUCCESS, cache->UpdateInstance({"TestSchema.TestClass", "Foo"}, instances.ToWSObjectsResponse()));
    // Act
    Json::Value modifiedProperties;
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ReadModifiedProperties(instance, modifiedProperties));
    // Assert
    auto expected = ToJson(R"({"TestProperty3":"OtherC"})");
    EXPECT_EQ(expected, modifiedProperties);
    }

TEST_F(ChangeManagerTests, ReadModifiedProperties_NotExistingInstance_ReturnsError)
    {
    auto cache = GetTestCache();
    auto instance = StubNonExistingInstanceKey(*cache);

    Json::Value properties;
    ASSERT_EQ(ERROR, cache->GetChangeManager().ReadModifiedProperties(instance, properties));
    EXPECT_EQ(Json::Value::null, properties);
    }

TEST_F(ChangeManagerTests, ReadModifiedProperties_NotChangedInstance_ReturnsError)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});

    Json::Value properties;
    ASSERT_EQ(ERROR, cache->GetChangeManager().ReadModifiedProperties(instance, properties));
    EXPECT_EQ(Json::Value::null, properties);
    }

TEST_F(ChangeManagerTests, ReadModifiedProperties_CreatedInstance_ReturnsError)
    {
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");

    Json::Value properties;
    ASSERT_EQ(ERROR, cache->GetChangeManager().ReadModifiedProperties(instance, properties));
    EXPECT_EQ(Json::Value::null, properties);
    }

TEST_F(ChangeManagerTests, ReadModifiedProperties_ModifiedAfterInstanceWasCreated_ReturnsErrorForCreatedObject)
    {
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache, "TestSchema.TestClass");

    Json::Value properties;
    properties["TestProperty"] = "SomeOtherValue";
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, properties));

    Json::Value modifiedProperties;
    ASSERT_EQ(ERROR, cache->GetChangeManager().ReadModifiedProperties(instance, modifiedProperties));
    }

TEST_F(ChangeManagerTests, ReadModifiedProperties_DeletedInstance_ReturnsError)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});

    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));

    Json::Value properties;
    ASSERT_EQ(ERROR, cache->GetChangeManager().ReadModifiedProperties(instance, properties));
    EXPECT_EQ(Json::Value::null, properties);
    }

TEST_F(ChangeManagerTests, ModifyObject_InstanceIsRemoved_BackupInstanceIsRemoved)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});

    ASSERT_EQ(0, CountClassInstances(*cache, "WSCache.InstanceBackup"));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, Json::objectValue));
    EXPECT_EQ(1, CountClassInstances(*cache, "WSCache.InstanceBackup"));

    ASSERT_EQ(CacheStatus::OK, cache->RemoveInstance({"TestSchema.TestClass", "Foo"}));
    EXPECT_EQ(0, CountClassInstances(*cache, "WSCache.InstanceBackup"));
    EXPECT_FALSE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsInCache());
    }

TEST_F(ChangeManagerTests, CommitInstanceRevision_ModifiedInstance_BackupInstanceIsRemoved)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"});

    ASSERT_EQ(0, CountClassInstances(*cache, "WSCache.InstanceBackup"));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, Json::objectValue));
    EXPECT_EQ(1, CountClassInstances(*cache, "WSCache.InstanceBackup"));

    auto revision = cache->GetChangeManager().ReadInstanceRevision(instance);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().CommitInstanceRevision(*revision));
    EXPECT_EQ(0, CountClassInstances(*cache, "WSCache.InstanceBackup"));

    EXPECT_TRUE(cache->GetCachedObjectInfo({"TestSchema.TestClass", "Foo"}).IsInCache());
    }

TEST_F(ChangeManagerTests, RevertModifiedObject_NonExistingInstance_Error)
    {
    auto cache = GetTestCache();
    BeTest::SetFailOnAssert(false);
    ASSERT_EQ(ERROR, cache->GetChangeManager().RevertModifiedObject(StubECInstanceKey()));
    BeTest::SetFailOnAssert(true);
    }

TEST_F(ChangeManagerTests, RevertModifiedObject_NonModifiedInstance_Error)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache);
    BeTest::SetFailOnAssert(false);
    ASSERT_EQ(ERROR, cache->GetChangeManager().RevertModifiedObject(instance));
    BeTest::SetFailOnAssert(true);
    }

TEST_F(ChangeManagerTests, RevertModifiedObject_CreatedInstance_Error)
    {
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache);
    BeTest::SetFailOnAssert(false);
    ASSERT_EQ(ERROR, cache->GetChangeManager().RevertModifiedObject(instance));
    BeTest::SetFailOnAssert(true);
    }

TEST_F(ChangeManagerTests, RevertModifiedObject_DeletedInstance_Error)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    BeTest::SetFailOnAssert(false);
    ASSERT_EQ(ERROR, cache->GetChangeManager().RevertModifiedObject(instance));
    BeTest::SetFailOnAssert(true);
    }

TEST_F(ChangeManagerTests, RevertModifiedObject_ModifiedInstance_RevertsToCachedState)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"}, {{"TestProperty", "A"}, {"TestProperty2", "B"}});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, ToJson(R"({"TestProperty":"A1", "TestProperty2":"B1"})")));

    ASSERT_EQ(SUCCESS, cache->GetChangeManager().RevertModifiedObject(instance));

    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    Json::Value properties;
    ASSERT_EQ(SUCCESS, cache->GetAdapter().GetJsonInstance(properties, instance));
    EXPECT_EQ("A", properties["TestProperty"].asString());
    EXPECT_EQ("B", properties["TestProperty2"].asString());
    }

TEST_F(ChangeManagerTests, RevertModifiedObject_MultipleModifiedObjects_RemovesChangeForSpecifiedObjectOnly)
    {
    // Arrange
    auto cache = GetTestCache();
    auto a = StubInstanceInCache(*cache, {"TestSchema.TestClass", "A"}, {{"TestProperty", "OldA"}});
    auto b = StubInstanceInCache(*cache, {"TestSchema.TestClass", "B"}, {{"TestProperty", "OldB"}});
    auto c = StubInstanceInCache(*cache, {"TestSchema.TestClass", "C"}, {{"TestProperty", "OldC"}});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(a, ToJson(R"({"TestProperty":"NewA"})")));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(b, ToJson(R"({"TestProperty":"NewB"})")));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(c, ToJson(R"({"TestProperty":"NewC"})")));
    // Act
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().RevertModifiedObject(b));
    // Assert
    EXPECT_EQ("NewA", ReadInstance(*cache, a)["TestProperty"].asString());
    EXPECT_EQ("OldB", ReadInstance(*cache, b)["TestProperty"].asString());
    EXPECT_EQ("NewC", ReadInstance(*cache, c)["TestProperty"].asString());
    }

TEST_F(ChangeManagerTests, RevertModifiedObject_ModifiedInstanceAndSyncActive_Error)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache, {"TestSchema.TestClass", "Foo"}, {{"TestProperty", "A"}, {"TestProperty2", "B"}});
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, ToJson(R"({"TestProperty":"A1", "TestProperty2":"B1"})")));

    cache->GetChangeManager().SetSyncActive(true);
    BeTest::SetFailOnAssert(false);
    ASSERT_EQ(ERROR, cache->GetChangeManager().RevertModifiedObject(instance));
    BeTest::SetFailOnAssert(true);
    cache->GetChangeManager().SetSyncActive(false);

    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, cache->GetChangeManager().GetObjectChange(instance).GetChangeStatus());
    Json::Value properties;
    ASSERT_EQ(SUCCESS, cache->GetAdapter().GetJsonInstance(properties, instance));
    EXPECT_EQ("A1", properties["TestProperty"].asString());
    EXPECT_EQ("B1", properties["TestProperty2"].asString());
    }

TEST_F(ChangeManagerTests, AddCreatedInstanceToResponse_NotExistingInstance_Error)
    {
    auto cache = GetTestCache();
    auto nonExistingInstance = StubNonExistingInstanceKey(*cache);
    auto responseKey = StubCachedResponseKey(*cache);
    ASSERT_EQ(ERROR, cache->GetChangeManager().AddCreatedInstanceToResponse(responseKey, nonExistingInstance));
    }

TEST_F(ChangeManagerTests, AddCreatedInstanceToResponse_NotChangedInstance_Error)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache);
    auto responseKey = StubCachedResponseKey(*cache);
    ASSERT_EQ(ERROR, cache->GetChangeManager().AddCreatedInstanceToResponse(responseKey, instance));
    }

TEST_F(ChangeManagerTests, AddCreatedInstanceToResponse_DeletedInstance_Error)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().DeleteObject(instance));
    auto responseKey = StubCachedResponseKey(*cache);
    ASSERT_EQ(ERROR, cache->GetChangeManager().AddCreatedInstanceToResponse(responseKey, instance));
    }

TEST_F(ChangeManagerTests, AddCreatedInstanceToResponse_ModifiedInstance_Error)
    {
    auto cache = GetTestCache();
    auto instance = StubInstanceInCache(*cache);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().ModifyObject(instance, Json::objectValue));
    auto responseKey = StubCachedResponseKey(*cache);
    ASSERT_EQ(ERROR, cache->GetChangeManager().AddCreatedInstanceToResponse(responseKey, instance));
    }

TEST_F(ChangeManagerTests, AddCreatedInstanceToResponse_CreatedRelationship_Error)
    {
    auto cache = GetTestCache();
    auto relationship = StubCreatedRelationshipInCache(*cache);
    auto responseKey = StubCachedResponseKey(*cache);
    ASSERT_EQ(ERROR, cache->GetChangeManager().AddCreatedInstanceToResponse(responseKey, relationship));
    }

TEST_F(ChangeManagerTests, AddCreatedInstanceToResponse_CreatedObjectAndNotExistingResponse_CreatesResponseAndAddsObjectToIt)
    {
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache);
    auto responseKey = StubCachedResponseKey(*cache);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().AddCreatedInstanceToResponse(responseKey, instance));

    EXPECT_TRUE(cache->IsResponseCached(responseKey));

    ECInstanceKeyMultiMap instances;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(responseKey, instances));
    EXPECT_EQ(1, instances.size());
    EXPECT_CONTAINS(instances, ECDbHelper::ToPair(instance));

    bset<ObjectId> objectIds;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseObjectIds(responseKey, objectIds));
    EXPECT_EQ(1, objectIds.size());
    EXPECT_CONTAINS(objectIds, cache->FindInstance(instance));
    }

TEST_F(ChangeManagerTests, AddCreatedInstanceToResponse_CreatedObjectAndExistingResponse_AddsObjectToExistingInstanceList)
    {
    auto cache = GetTestCache();
    auto responseKey = StubCachedResponseKey(*cache);

    StubInstances stubInstances;
    stubInstances.Add({"TestSchema.TestClass", "A"});
    stubInstances.Add({"TestSchema.TestClass", "B"});
    ASSERT_EQ(SUCCESS, cache->CacheResponse(responseKey, stubInstances.ToWSObjectsResponse()));
    
    auto instance = StubCreatedObjectInCache(*cache);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().AddCreatedInstanceToResponse(responseKey, instance));

    ECInstanceKeyMultiMap instances;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(responseKey, instances));
    EXPECT_EQ(3, instances.size());
    EXPECT_CONTAINS(instances, ECDbHelper::ToPair(instance));
    EXPECT_CONTAINS(instances, ECDbHelper::ToPair(cache->FindInstance({"TestSchema.TestClass", "A"})));
    EXPECT_CONTAINS(instances, ECDbHelper::ToPair(cache->FindInstance({"TestSchema.TestClass", "B"})));
    }

TEST_F(ChangeManagerTests, AddCreatedInstanceToResponse_MultipleCreatedObjects_AddsObjectsToResponse)
    {
    auto cache = GetTestCache();
    auto instance1 = StubCreatedObjectInCache(*cache);
    auto instance2 = StubCreatedObjectInCache(*cache);
    auto responseKey = StubCachedResponseKey(*cache);

    ASSERT_EQ(SUCCESS, cache->GetChangeManager().AddCreatedInstanceToResponse(responseKey, instance1));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().AddCreatedInstanceToResponse(responseKey, instance2));

    ECInstanceKeyMultiMap instances;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(responseKey, instances));
    EXPECT_EQ(2, instances.size());
    EXPECT_CONTAINS(instances, ECDbHelper::ToPair(instance1));
    EXPECT_CONTAINS(instances, ECDbHelper::ToPair(instance2));
    }

TEST_F(ChangeManagerTests, AddCreatedInstanceToResponse_MultipleResponses_AddsObjectsToResponses)
    {
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache);
    auto responseKey1 = StubCachedResponseKey(*cache);
    auto responseKey2 = StubCachedResponseKey(*cache);

    ASSERT_EQ(SUCCESS, cache->GetChangeManager().AddCreatedInstanceToResponse(responseKey1, instance));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().AddCreatedInstanceToResponse(responseKey2, instance));

    ECInstanceKeyMultiMap instances;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(responseKey1, instances));
    EXPECT_EQ(1, instances.size());
    EXPECT_CONTAINS(instances, ECDbHelper::ToPair(instance));

    instances.clear();
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(responseKey2, instances));
    EXPECT_EQ(1, instances.size());
    EXPECT_CONTAINS(instances, ECDbHelper::ToPair(instance));
    }

TEST_F(ChangeManagerTests, RemoveCreatedInstanceFromResponse_NotExistingInstance_Error)
    {
    auto cache = GetTestCache();
    auto nonExistingInstance = StubNonExistingInstanceKey(*cache);
    auto responseKey = StubCachedResponseKey(*cache);
    ASSERT_EQ(ERROR, cache->GetChangeManager().RemoveCreatedInstanceFromResponse(responseKey, nonExistingInstance));
    }

TEST_F(ChangeManagerTests, RemoveCreatedInstanceFromResponse_CreatedInstanceWasNotAdded_Error)
    {
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache);
    auto responseKey = StubCachedResponseKey(*cache);
    ASSERT_EQ(ERROR, cache->GetChangeManager().RemoveCreatedInstanceFromResponse(responseKey, instance));
    }

TEST_F(ChangeManagerTests, RemoveCreatedInstanceFromResponse_CreatedInstanceWasAdded_RemovesInstanceFromResponse)
    {
    auto cache = GetTestCache();
    auto instance = StubCreatedObjectInCache(*cache);

    auto responseKey = StubCachedResponseKey(*cache);
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().AddCreatedInstanceToResponse(responseKey, instance));
    ASSERT_EQ(SUCCESS, cache->GetChangeManager().RemoveCreatedInstanceFromResponse(responseKey, instance));

    ECInstanceKeyMultiMap instances;
    ASSERT_EQ(CacheStatus::OK, cache->ReadResponseInstanceKeys(responseKey, instances));
    EXPECT_TRUE(instances.empty());
    }

#endif
