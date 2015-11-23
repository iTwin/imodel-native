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
#include <WebServices/Client/WSChangeset.h>
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

typedef std::shared_ptr<WSChangeset> WSChangesetPtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct SyncLocalChangesTask : public CachingTaskBase
    {
    private:
        typedef bmap<ObjectId, IChangeManager::InstanceRevisionPtr> RevisionMap;

    private:
        WSInfo m_serverInfo;
        SyncOptions m_options;

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

        BentleyStatus PrepareChangeGroups(IDataSourceCache& cache);
        void SyncNext();

        bool CanSyncChangeset(CacheChangeGroupCR changeGroup) const;
        AsyncTaskPtr<void> SyncNextChangeset();

        AsyncTaskPtr<void> SyncChangeGroup(CacheChangeGroupPtr changeGroup);
        AsyncTaskPtr<void> SyncCreation(CacheChangeGroupPtr changeGroup);
        AsyncTaskPtr<void> SyncObjectModification(CacheChangeGroupPtr changeGroup);
        AsyncTaskPtr<void> SyncFileModification(CacheChangeGroupPtr changeGroup);
        AsyncTaskPtr<void> SyncObjectDeletion(CacheChangeGroupPtr changeGroup);

        void HandleCreationError(WSErrorCR error, CacheChangeGroupPtr changeGroup, Utf8StringCR objectLabel);

        void ReportProgress(double currentFileBytesUploaded, Utf8StringCR label) const;
        void ReportFinalProgress() const;
        ResponseGuardPtr CreateResponseGuard(Utf8StringCR objectLabel, bool reportProgress) const;

        WSChangesetPtr BuildChangeset
            (
            IDataSourceCache& cache,
            RevisionMap& revisionsOut,
            bvector<CacheChangeGroup*>& changesetChangeGroupsOut
            );
        WSChangesetPtr BuildSingleInstanceChangeset
            (
            IDataSourceCache& cache,
            CacheChangeGroupCR changeGroup,
            RevisionMap& revisionsOut
            );
        WSChangeset::Instance* AddChangeToChangeset
            (
            IDataSourceCache& cache,
            WSChangeset& changeset,
            CacheChangeGroupCR changeGroup,
            RevisionMap& revisionsOut,
            bool ensureChangedInstanceInRoot
            );

        void SetExistingInstanceInfoToJson(IDataSourceCache& cache, ECInstanceKeyCR instanceKey, JsonValueR json) const;
        void SetChangedInstanceClassInfoToJson(IDataSourceCache& cache, IChangeManager::ObjectChangeCR change, JsonValueR json) const;
        Utf8String GetChangeStateStr(IChangeManager::ChangeStatus changeStatus) const;
        WSChangeset::ChangeState ToWSChangesetChangeState(IChangeManager::ChangeStatus status) const;

        void RegisterFailedSync(IDataSourceCache& cache, CacheChangeGroupCR changeGroup, CachingDataSource::ErrorCR error, Utf8StringCR objectLabel);
        void SetUpdatedInstanceKeyInChangeGroups(ECInstanceKey oldKey, ECInstanceKey newKey);

    public:
        SyncLocalChangesTask
            (
            CachingDataSourcePtr cachingDataSource,
            std::shared_ptr<bset<ECInstanceKey>> objectsToSync,
            SyncOptions options,
            CachingDataSource::SyncProgressCallback&& onProgress,
            ICancellationTokenPtr cancellationToken
            );
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
