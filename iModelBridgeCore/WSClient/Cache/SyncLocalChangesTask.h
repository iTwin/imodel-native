/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/SyncLocalChangesTask.h $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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

        bvector<ChangeGroupPtr> m_changeGroups;
        size_t m_changeGroupIndexToSyncNext;
        ChangeGroupPtr m_currentChangeGroup;

        bset<ECInstanceKey> m_instancesStillInSync;

        CachingDataSource::Progress::State m_uploadBytesProgress;

        bvector<ObjectId> m_objectsToRefreshAfterSync;

    private:
        virtual void _OnExecute();
        virtual void _OnError(CachingDataSource::ErrorCR error);
        void OnSyncDone();

        BentleyStatus PrepareChangeGroups(IDataSourceCache& cache);
        AsyncTaskPtr<void> SyncNext();

        bool CanSyncChangeset(ChangeGroupCR changeGroup) const;
        AsyncTaskPtr<bool> ShouldSyncObjectAndFileCreationSeperately(ChangeGroupPtr changeGroup);

        AsyncTaskPtr<void> SyncNextChangeset();

        void SetSyncActiveForChangeGroup(CacheTransactionCR txn, ChangeGroupCR changeGroup, bool active);

        AsyncTaskPtr<void> SyncChangeGroup(ChangeGroupPtr changeGroup);
        AsyncTaskPtr<void> SyncNextChangeGroup();
        AsyncTaskPtr<void> SyncCreation(ChangeGroupPtr changeGroup);
        AsyncTaskPtr<void> SyncObjectWithFileCreation(ChangeGroupPtr changeGroup, bool includeFile);
        AsyncTaskPtr<void> SyncObjectModification(ChangeGroupPtr changeGroup);
        AsyncTaskPtr<void> SyncFileModification(ChangeGroupPtr changeGroup);
        AsyncTaskPtr<void> SyncObjectDeletion(ChangeGroupPtr changeGroup);

        void HandleSyncError(WSErrorCR error, ChangeGroupPtr changeGroup, Utf8StringCR objectLabel);

        void ReportProgress(double currentFileBytesUploaded, Utf8StringCPtr label, double currentFileTotalBytes = 0.0) const;
        void ReportFinalProgress() const;
        ResponseGuardPtr CreateResponseGuard(Utf8StringCR objectLabel, bool reportProgress, double currentFileTotalBytes = 0.0) const;

        WSChangesetPtr BuildChangeset
            (
            IDataSourceCache& cache,
            RevisionMap& revisionsOut,
            bvector<ChangeGroup*>& changesetChangeGroupsOut
            );
        WSChangesetPtr BuildSingleInstanceChangeset
            (
            IDataSourceCache& cache,
            ChangeGroupCR changeGroup,
            RevisionMap& revisionsOut
            );
        WSChangeset::Instance* AddChangeToChangeset
            (
            IDataSourceCache& cache,
            WSChangeset& changeset,
            ChangeGroupCR changeGroup,
            RevisionMap& revisionsOut,
            bool ensureChangedInstanceInRoot
            );

        void SetExistingInstanceInfoToJson(IDataSourceCache& cache, ECInstanceKeyCR instanceKey, JsonValueR json) const;
        void SetChangedInstanceClassInfoToJson(IDataSourceCache& cache, IChangeManager::ObjectChangeCR change, JsonValueR json) const;
        Utf8String GetChangeStateStr(IChangeManager::ChangeStatus changeStatus) const;
        WSChangeset::ChangeState ToWSChangesetChangeState(IChangeManager::ChangeStatus status) const;

        void RegisterFailedSync(IDataSourceCache& cache, ChangeGroupCR changeGroup, CachingDataSource::ErrorCR error, Utf8StringCR objectLabel);
        void SetUpdatedInstanceKeyInChangeGroups(ECInstanceKey oldKey, ECInstanceKey newKey);

    public:
        SyncLocalChangesTask
            (
            CachingDataSourcePtr cachingDataSource,
            std::shared_ptr<bset<ECInstanceKey>> objectsToSync,
            SyncOptions options,
            CachingDataSource::ProgressCallback&& onProgress,
            ICancellationTokenPtr ct
            );
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
