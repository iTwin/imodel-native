/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/SyncLocalChangesTask.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include "Network/ResponseGuard.h"
#include <atomic>

#include "CachingTaskBase.h"
#include "ChangesGraph.h"
#include <WebServices/Cache/CachingDataSource.h>
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct SyncLocalChangesTask : public CachingTaskBase
    {
    private:
        std::shared_ptr<bset<ECInstanceKey>> m_objectsToSyncPtr;
        const CachingDataSource::SyncProgressCallback m_onProgressCallback;

        bvector<CacheChangeGroupPtr> m_changeGroups;
        size_t m_changeGroupIndexToSyncNext;

        std::atomic<uint64_t> m_totalBytesToUpload;
        std::atomic<uint64_t> m_totalBytesUploaded;

        bvector<ObjectId> m_objectsToRefreshAfterSync;

    private:
        virtual void _OnExecute();
        virtual void _OnError(CachingDataSource::ErrorCR error);
        void OnSyncDone();

        void SyncNextCacheChangeGroup();

        AsyncTaskPtr<void> SyncCacheChangeGroup(CacheChangeGroupPtr changeGroup);
        AsyncTaskPtr<void> SyncCreation(CacheChangeGroupPtr changeGroup);
        AsyncTaskPtr<void> SyncObjectModification(CacheChangeGroupPtr changeGroup);
        AsyncTaskPtr<void> SyncFileModification(CacheChangeGroupPtr changeGroup);
        AsyncTaskPtr<void> SyncObjectDeletion(CacheChangeGroupPtr changeGroup);

        void HandleCreationError(WSErrorCR error, CacheChangeGroupPtr changeGroup, Utf8StringCR objectLabel);

        void ReportProgress(double currentFileBytesUploaded, Utf8StringCR label) const;
        ResponseGuardPtr CreateResponseGuard(Utf8StringCR objectLabel, bool reportProgress) const;

        BentleyStatus BuildSyncJson(IDataSourceCache& cache, CacheChangeGroupCR changeGroup, JsonValueR syncJsonOut) const;
        BentleyStatus BuildSyncJsonForObjectCreation(IDataSourceCache& cache, CacheChangeGroupCR changeGroup, JsonValueR syncJsonOut) const;
        BentleyStatus BuildSyncJsonForRelationshipCreation(IDataSourceCache& cache, ChangeManager::RelationshipChangeCR relationshipChange, JsonValueR syncJsonOut) const;

        std::map<ECInstanceKey, Utf8String> ReadChangedRemoteIds(CacheChangeGroupCR changeGroup, WSCreateObjectResponseCR response) const;

        BentleyStatus ReadObjectProperties(IDataSourceCache& cache, ECInstanceKeyCR instanceKey, JsonValueR propertiesJsonOut) const;
        BentleyStatus ReadObjectPropertiesForUpdate(IDataSourceCache& cache, ECInstanceKeyCR instanceKey, JsonValueR propertiesJsonOut) const;
        BentleyStatus ReadObjectPropertiesForCreation(IDataSourceCache& cache, ECInstanceKeyCR instanceKey, JsonValueR propertiesJsonOut) const;

        void RemoveCacheSpecificProperties(JsonValueR propertiesJson) const;
        void RemoveReadOnlyProperties(JsonValueR propertiesJson, ECClassCR ecClass) const;
        void RemoveCalculatedProperties(JsonValueR propertiesJson, ECClassCR ecClass) const;

        void SetExistingInstanceInfoToJson(IDataSourceCache& cache, ECInstanceKeyCR instanceKey, JsonValueR json) const;
        void SetChangedInstanceClassInfoToJson(IDataSourceCache& cache, IChangeManager::ObjectChangeCR change, JsonValueR json) const;
        Utf8String GetChangeStateStr(IChangeManager::ChangeStatus changeStatus) const;

        void RegisterFailedSync(IDataSourceCache& cache, CacheChangeGroupCR changeGroup, CachingDataSource::ErrorCR error, Utf8StringCR objectLabel);
        void SetUpdatedInstanceKeyInCacheChangeGroups(ECInstanceKey oldKey, ECInstanceKey newKey);

        static void RemoveEmptyMembersRecursively(JsonValueR jsonObject);
        static void RemoveEmptyMembersRecursively(JsonValueR childJson, Utf8StringCR childMemberNameInParent, JsonValueR parentJson);

    public:
        SyncLocalChangesTask
            (
            CachingDataSourcePtr cachingDataSource,
            std::shared_ptr<bset<ECInstanceKey>> objectsToSync,
            CachingDataSource::SyncProgressCallback&& onProgress,
            ICancellationTokenPtr cancellationToken
            );
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
