/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <WebServices/Cache/Persistence/IChangeManager.h>

#ifdef USE_GTEST
BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct MockChangeManager> MockChangeManagerPtr;
struct MockChangeManager : public IChangeManager
    {
    public:
        static MockChangeManagerPtr Create ()
            {
            return std::make_shared<MockChangeManager> ();
            }

        MOCK_CONST_METHOD1 (IsUploadActive,
            bool(ECInstanceKeyCR));
        MOCK_METHOD2 (SetUploadActive,
            void(ECInstanceKeyCR, bool active));
        MOCK_CONST_METHOD0(GetUploadingInstances,
            const ECInstanceKeyMultiMap&());

        MOCK_METHOD3 (GetLegacyParentRelationshipClass,
            ECRelationshipClassCP(ECClassId, ECClassId, bool));
        MOCK_METHOD3 (CreateObject,
            ECInstanceKey (ECClassCR, JsonValueCR, SyncStatus));
        MOCK_METHOD3 (ModifyObject,
            BentleyStatus (ECInstanceKeyCR, JsonValueCR, SyncStatus));
        MOCK_METHOD1 (RevertModifiedObject,
            BentleyStatus(ECInstanceKeyCR));
        MOCK_METHOD2 (DeleteObject,
            BentleyStatus (ECInstanceKeyCR, SyncStatus));
        MOCK_METHOD4 (CreateRelationship,
            ECInstanceKey (ECRelationshipClassCR, ECInstanceKeyCR, ECInstanceKeyCR, SyncStatus));
        MOCK_METHOD2 (DeleteRelationship,
            BentleyStatus (ECInstanceKeyCR, SyncStatus));
        MOCK_METHOD4 (ModifyFile,
            BentleyStatus(ECInstanceKeyCR, BeFileNameCR, bool, SyncStatus));
        MOCK_METHOD2(ModifyFileName,
            BentleyStatus(ECInstanceKeyCR, Utf8StringCR));
        MOCK_METHOD2(DetectFileModification,
            BentleyStatus(ECInstanceKeyCR, bool&));
        MOCK_METHOD2 (SetSyncStatus,
            BentleyStatus (ECInstanceKeyCR, SyncStatus));
        MOCK_METHOD2 (AddCreatedInstanceToResponse, 
            BentleyStatus(CachedResponseKeyCR responseKey, ECInstanceKeyCR instanceKey));
        MOCK_METHOD2 (RemoveCreatedInstanceFromResponse, 
            BentleyStatus(CachedResponseKeyCR responseKey, ECInstanceKeyCR instanceKey));
        MOCK_METHOD0 (HasChanges,
            bool ());
        MOCK_METHOD2 (GetChanges,
            BentleyStatus (Changes&, bool));
        MOCK_METHOD2 (GetChanges,
            BentleyStatus (ECInstanceKeyCR, Changes&));
        MOCK_METHOD2 (GetCreatedRelationships,
            BentleyStatus (ECInstanceKeyCR, bvector<RelationshipChange>&));
        MOCK_METHOD1 (GetObjectChange,
            ObjectChange (ECInstanceKeyCR instanceKey));
        MOCK_METHOD1 (GetRelationshipChange,
            RelationshipChange (ECInstanceKeyCR relationshipId));
        MOCK_METHOD1 (GetFileChange,
            FileChange (ECInstanceKeyCR instanceKey));
        MOCK_METHOD1 (GetObjectChangeStatus,
            ChangeStatus (ECInstanceKeyCR instance));
        MOCK_METHOD1 (GetObjectSyncStatus,
            SyncStatus (ECInstanceKeyCR instance));
        MOCK_METHOD1 (ReadInstanceRevision,
            std::shared_ptr<InstanceRevision>(ECInstanceKeyCR));
        MOCK_METHOD1 (ReadFileRevision,
            std::shared_ptr<FileRevision>(ECInstanceKeyCR));
        MOCK_METHOD2 (ReadModifiedProperties,
            BentleyStatus (ECInstanceKeyCR instance, JsonValueR propertiesOut));
        MOCK_METHOD0(CommitLocalDeletions,
            BentleyStatus());
        MOCK_METHOD1 (CommitInstanceRevision,
            BentleyStatus (InstanceRevisionCR));
        MOCK_METHOD1 (CommitFileRevision,
            BentleyStatus (FileRevisionCR));
        MOCK_METHOD3 (UpdateCreatedInstance, 
            BentleyStatus (ObjectIdCR instanceId, WSObjectsResponseCR instanceResponse, bmap<ECInstanceKey, ECInstanceKey>&));
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
#endif
