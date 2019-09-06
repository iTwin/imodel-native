/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

        const CachingDataSource::ProgressCallback m_onProgressCallback;

        bvector<CacheChangeGroupPtr> m_changeGroups;
        size_t m_changeGroupIndexToSyncNext;
        CacheChangeGroupPtr m_currentChangeGroup;

        bset<ECInstanceKey> m_instancesStillInSync;

        CachingDataSource::Progress::State m_uploadBytesProgress;

        bvector<ObjectId> m_objectsToRefreshAfterSync;

    private:
        virtual void _OnExecute();
        void OnSyncDone();

        BentleyStatus PrepareChangeGroups(IDataSourceCache& cache);
        AsyncTaskPtr<void> SyncNext();

        bool CanSyncChangeset(CacheChangeGroupCR changeGroup) const;
        AsyncTaskPtr<bool> ShouldSyncObjectAndFileCreationSeperately(CacheChangeGroupPtr changeGroup);

        AsyncTaskPtr<void> SyncNextChangeset();

        void SetUploadActiveForChangeGroup(CacheTransactionCR txn, CacheChangeGroupCR changeGroup, bool active);
        void SetUploadActiveForSingleInstance(CacheTransactionCR txn, ECInstanceKeyCR key, bool active);

        AsyncTaskPtr<void> SyncChangeGroup(CacheChangeGroupPtr changeGroup);
        AsyncTaskPtr<void> SyncNextChangeGroup();
        AsyncTaskPtr<void> SyncCreation(CacheChangeGroupPtr changeGroup);
        AsyncTaskPtr<void> SyncObjectWithFileCreation(CacheChangeGroupPtr changeGroup, bool includeFile);
        AsyncTaskPtr<void> SyncObjectModification(CacheChangeGroupPtr changeGroup);
        AsyncTaskPtr<void> SyncFileModification(CacheChangeGroupPtr changeGroup);
        AsyncTaskPtr<void> SyncObjectDeletion(CacheChangeGroupPtr changeGroup);

        void HandleSyncError(WSErrorCR error, CacheChangeGroupPtr changeGroup, Utf8StringCR objectLabel);

        void ReportProgress(double currentFileBytesUploaded, Utf8StringCPtr label, double currentFileTotalBytes = 0.0) const;
        void ReportFinalProgress() const;
        ResponseGuardPtr CreateResponseGuard(Utf8StringCR objectLabel, bool reportProgress, double currentFileTotalBytes = 0.0) const;

        WSChangesetPtr BuildChangeset
            (
            IDataSourceCache& cache,
            RevisionMap& revisionsOut,
            bset<CacheChangeGroup*>& changesetChangeGroupsOut
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

        void RegisterFailedSync(IDataSourceCache& cache, CacheChangeGroupCR changeGroup, CachingDataSource::ErrorCR error, Utf8StringCR objectLabel = nullptr);
        void SetUpdatedInstanceKeyInChangeGroups(ECInstanceKey oldKey, ECInstanceKey newKey);

        ICancellationTokenPtr GetFileCancellationToken() const;

    public:
        SyncLocalChangesTask
            (
            CachingDataSourcePtr cachingDataSource,
            std::shared_ptr<bset<ECInstanceKey>> objectsToSync,
            SyncOptions options,
            CachingDataSource::ProgressCallback&& onProgress,
            ICancellationTokenPtr ct
            );
        void PrepareObjectsForSync(CacheTransactionCR txn);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
