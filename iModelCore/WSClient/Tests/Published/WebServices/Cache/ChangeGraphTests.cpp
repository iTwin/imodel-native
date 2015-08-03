/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/ChangeGraphTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ChangeGraphTests.h"

#include "../../../../Cache/ChangesGraph.h"
#include "CachingTestsHelper.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

TEST_F(ChangeGraphTests, BuildCacheChangeGroups_EmptyChanges_ReturnsEmpty)
    {
    ChangeManager::Changes changes;
    EXPECT_TRUE(ChangesGraph(changes).BuildCacheChangeGroups().empty());
    }

TEST_F(ChangeGraphTests, BuildCacheChangeGroups_CreatedObjectWithRelationship_ReturnsOneChangeGroup)
    {
    auto keyA = StubECInstanceKey(1, 2);
    auto keyB = StubECInstanceKey(1, 3);
    auto keyAB = StubECInstanceKey(1, 4);

    ChangeManager::Changes changes;
    changes.AddChange(ChangeManager::ObjectChange(keyA, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 1));
    changes.AddChange(ChangeManager::RelationshipChange(keyAB, keyA, keyB, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 2));

    auto groups = ChangesGraph(changes).BuildCacheChangeGroups();

    ASSERT_EQ(1, groups.size());
    EXPECT_EQ(keyA, groups[0]->GetObjectChange().GetInstanceKey());
    EXPECT_EQ(keyAB, groups[0]->GetRelationshipChange().GetInstanceKey());
    }

TEST_F(ChangeGraphTests, BuildCacheChangeGroups_CreatedObjectWithFile_ReturnsOneChangeGroup)
    {
    auto keyA = StubECInstanceKey(1, 2);

    ChangeManager::Changes changes;
    changes.AddChange(ChangeManager::ObjectChange(keyA, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 1));
    changes.AddChange(ChangeManager::FileChange(keyA, IChangeManager::ChangeStatus::Modified, ChangeManager::SyncStatus::Ready, 2));

    auto groups = ChangesGraph(changes).BuildCacheChangeGroups();

    ASSERT_EQ(1, groups.size());
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, groups[0]->GetObjectChange().GetChangeStatus());
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, groups[0]->GetFileChange().GetChangeStatus());
    EXPECT_EQ(keyA, groups[0]->GetObjectChange().GetInstanceKey());
    EXPECT_EQ(keyA, groups[0]->GetFileChange().GetInstanceKey());
    }

TEST_F(ChangeGraphTests, BuildCacheChangeGroups_ModifiedObjectWithFile_ReturnsSeperateChangeGroups)
    {
    auto keyA = StubECInstanceKey(1, 2);

    ChangeManager::Changes changes;
    changes.AddChange(ChangeManager::ObjectChange(keyA, IChangeManager::ChangeStatus::Modified, ChangeManager::SyncStatus::Ready, 1));
    changes.AddChange(ChangeManager::FileChange(keyA, IChangeManager::ChangeStatus::Modified, ChangeManager::SyncStatus::Ready, 2));

    auto groups = ChangesGraph(changes).BuildCacheChangeGroups();

    ASSERT_EQ(2, groups.size());
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, groups[0]->GetObjectChange().GetChangeStatus());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, groups[0]->GetFileChange().GetChangeStatus());

    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, groups[1]->GetObjectChange().GetChangeStatus());
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, groups[1]->GetFileChange().GetChangeStatus());
    }

TEST_F(ChangeGraphTests, BuildCacheChangeGroups_ObjectsCreatedFromRelatedExistingToNew_OrderedFromTopToBottom)
    {
    ChangeManager::Changes changes;

    auto keyA = StubECInstanceKey(1, 2);
    auto keyB = StubECInstanceKey(1, 3);
    auto keyE = StubECInstanceKey(1, 4);
    auto keyAE = StubECInstanceKey(1, 5);
    auto keyAB = StubECInstanceKey(1, 6);

    changes.AddChange(ChangeManager::ObjectChange(keyA, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 1));
    changes.AddChange(ChangeManager::ObjectChange(keyB, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 2));
    changes.AddChange(ChangeManager::RelationshipChange(keyAE, keyA, keyE, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 3));
    changes.AddChange(ChangeManager::RelationshipChange(keyAB, keyA, keyB, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 4));

    auto groups = ChangesGraph(changes).BuildCacheChangeGroups();

    ASSERT_EQ(2, groups.size());

    EXPECT_EQ(keyA, groups[0]->GetObjectChange().GetInstanceKey());
    EXPECT_EQ(keyAE, groups[0]->GetRelationshipChange().GetInstanceKey());

    EXPECT_EQ(keyB, groups[1]->GetObjectChange().GetInstanceKey());
    EXPECT_EQ(keyAB, groups[1]->GetRelationshipChange().GetInstanceKey());
    }

TEST_F(ChangeGraphTests, BuildCacheChangeGroups_ObjectsCreatedFromNewToRelatedExisting_OrderedFromBottomToTop)
    {
    auto keyA = StubECInstanceKey(1, 2);
    auto keyB = StubECInstanceKey(1, 3);
    auto keyE = StubECInstanceKey(1, 4);
    auto keyBE = StubECInstanceKey(1, 5);
    auto keyAB = StubECInstanceKey(1, 6);

    ChangeManager::Changes changes;
    changes.AddChange(ChangeManager::ObjectChange(keyA, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 1));
    changes.AddChange(ChangeManager::ObjectChange(keyB, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 2));
    changes.AddChange(ChangeManager::RelationshipChange(keyAB, keyA, keyB, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 3));
    changes.AddChange(ChangeManager::RelationshipChange(keyBE, keyB, keyE, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 4));

    auto groups = ChangesGraph(changes).BuildCacheChangeGroups();

    ASSERT_EQ(3, groups.size());

    EXPECT_EQ(keyA, groups[0]->GetObjectChange().GetInstanceKey());

    EXPECT_EQ(keyB, groups[1]->GetObjectChange().GetInstanceKey());
    EXPECT_EQ(keyAB, groups[1]->GetRelationshipChange().GetInstanceKey());

    EXPECT_EQ(keyBE, groups[2]->GetRelationshipChange().GetInstanceKey());
    }

TEST_F(ChangeGraphTests, BuildCacheChangeGroups_TwoSeperateObjectTreesCreated_SeperateGroupsForEach)
    {
    auto keyA = StubECInstanceKey(1, 2);
    auto keyB = StubECInstanceKey(1, 3);
    auto keyD = StubECInstanceKey(1, 4);
    auto keyE = StubECInstanceKey(1, 5);
    auto keyAD = StubECInstanceKey(1, 6);
    auto keyBE = StubECInstanceKey(1, 7);

    ChangeManager::Changes changes;
    changes.AddChange(ChangeManager::ObjectChange(keyA, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 1));
    changes.AddChange(ChangeManager::ObjectChange(keyB, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 2));
    changes.AddChange(ChangeManager::RelationshipChange(keyAD, keyA, keyD, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 3));
    changes.AddChange(ChangeManager::RelationshipChange(keyBE, keyB, keyE, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 4));

    auto groups = ChangesGraph(changes).BuildCacheChangeGroups();

    ASSERT_EQ(2, groups.size());

    EXPECT_EQ(keyA, groups[0]->GetObjectChange().GetInstanceKey());
    EXPECT_EQ(keyAD, groups[0]->GetRelationshipChange().GetInstanceKey());

    EXPECT_EQ(keyB, groups[1]->GetObjectChange().GetInstanceKey());
    EXPECT_EQ(keyBE, groups[1]->GetRelationshipChange().GetInstanceKey());
    }

TEST_F(ChangeGraphTests, BuildCacheChangeGroups_CreatedObjectsWithCyclicRelationship_FirstObjectSyncedSeperately)
    {
    auto keyA = StubECInstanceKey(1, 2);
    auto keyB = StubECInstanceKey(1, 3);
    auto keyC = StubECInstanceKey(1, 4);
    auto keyAB = StubECInstanceKey(1, 5);
    auto keyBC = StubECInstanceKey(1, 6);
    auto keyCA = StubECInstanceKey(1, 7);

    ChangeManager::Changes changes;
    changes.AddChange(ChangeManager::ObjectChange(keyA, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 1));
    changes.AddChange(ChangeManager::ObjectChange(keyB, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 2));
    changes.AddChange(ChangeManager::ObjectChange(keyC, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 3));
    changes.AddChange(ChangeManager::RelationshipChange(keyAB, keyA, keyB, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 4));
    changes.AddChange(ChangeManager::RelationshipChange(keyBC, keyB, keyC, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 5));
    changes.AddChange(ChangeManager::RelationshipChange(keyCA, keyC, keyA, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 6));

    auto groups = ChangesGraph(changes).BuildCacheChangeGroups();

    ASSERT_EQ(4, groups.size());

    EXPECT_EQ(keyA, groups[0]->GetObjectChange().GetInstanceKey());

    EXPECT_EQ(keyB, groups[1]->GetObjectChange().GetInstanceKey());
    EXPECT_EQ(keyAB, groups[1]->GetRelationshipChange().GetInstanceKey());

    EXPECT_EQ(keyC, groups[2]->GetObjectChange().GetInstanceKey());
    EXPECT_EQ(keyBC, groups[2]->GetRelationshipChange().GetInstanceKey());

    EXPECT_EQ(keyCA, groups[3]->GetRelationshipChange().GetInstanceKey());
    }

TEST_F(ChangeGraphTests, BuildCacheChangeGroups_CreatedObjectWithTwoRelationships_LastRelationshipInSeperateGroup)
    {
    auto keyA = StubECInstanceKey(1, 2);
    auto keyB = StubECInstanceKey(1, 3);
    auto keyC = StubECInstanceKey(1, 4);
    auto keyAB = StubECInstanceKey(1, 5);
    auto keyAC = StubECInstanceKey(1, 6);

    ChangeManager::Changes changes;
    changes.AddChange(ChangeManager::ObjectChange(keyA, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 1));
    changes.AddChange(ChangeManager::RelationshipChange(keyAB, keyA, keyB, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 2));
    changes.AddChange(ChangeManager::RelationshipChange(keyAC, keyA, keyC, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 3));

    auto groups = ChangesGraph(changes).BuildCacheChangeGroups();

    ASSERT_EQ(2, groups.size());

    EXPECT_EQ(keyA, groups[0]->GetObjectChange().GetInstanceKey());
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, groups[0]->GetObjectChange().GetChangeStatus());
    EXPECT_EQ(keyAB, groups[0]->GetRelationshipChange().GetInstanceKey());
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, groups[0]->GetRelationshipChange().GetChangeStatus());

    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, groups[1]->GetObjectChange().GetChangeStatus());
    EXPECT_EQ(keyAC, groups[1]->GetRelationshipChange().GetInstanceKey());
    EXPECT_EQ(IChangeManager::ChangeStatus::Created, groups[1]->GetRelationshipChange().GetChangeStatus());
    }

TEST_F(ChangeGraphTests, BuildCacheChangeGroups_ModifiedObjectWithCreatedRelationship_SeperateGroups)
    {
    auto keyA = StubECInstanceKey(1, 2);
    auto keyB = StubECInstanceKey(1, 3);
    auto keyAB = StubECInstanceKey(1, 5);

    ChangeManager::Changes changes;
    changes.AddChange(ChangeManager::ObjectChange(keyA, IChangeManager::ChangeStatus::Modified, ChangeManager::SyncStatus::Ready, 1));
    changes.AddChange(ChangeManager::RelationshipChange(keyAB, keyA, keyB, IChangeManager::ChangeStatus::Created, ChangeManager::SyncStatus::Ready, 2));

    auto groups = ChangesGraph(changes).BuildCacheChangeGroups();

    ASSERT_EQ(2, groups.size());

    EXPECT_EQ(keyA, groups[0]->GetObjectChange().GetInstanceKey());
    EXPECT_EQ(IChangeManager::ChangeStatus::Modified, groups[0]->GetObjectChange().GetChangeStatus());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, groups[0]->GetRelationshipChange().GetChangeStatus());

    EXPECT_EQ(keyAB, groups[1]->GetRelationshipChange().GetInstanceKey());
    EXPECT_EQ(IChangeManager::ChangeStatus::NoChange, groups[1]->GetObjectChange().GetChangeStatus());
    }
