/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/SyncLocalChangesTask.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "SyncLocalChangesTask.h"

#include <Bentley/BeTimeUtilities.h>
#include <WebServices/Cache/Util/FileUtil.h>
#include <WebServices/Client/WSChangeset.h>
#include <BeHttp/HttpStatusHelper.h>

#include "SessionInfo.h"
#include <WebServices/Cache/Util/JsonUtil.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             William.Francis         01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SyncLocalChangesTask::SyncLocalChangesTask
(
CachingDataSourcePtr cachingDataSource,
std::shared_ptr<bset<ECInstanceKey>> objectsToSync,
SyncOptions options,
CachingDataSource::SyncProgressCallback&& onProgress,
ICancellationTokenPtr ct
) :
CachingTaskBase(cachingDataSource, ct),
m_objectsToSyncPtr(objectsToSync),
m_options(options),
m_onProgressCallback(onProgress),
m_totalBytesToUpload(0),
m_totalBytesUploaded(0),
m_changeGroupIndexToSyncNext(0)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncLocalChangesTask::_OnExecute()
    {
    auto txn = m_ds->StartCacheTransaction();

    if (SUCCESS != txn.GetCache().GetChangeManager().CommitLocalDeletions() ||
        SUCCESS != PrepareChangeGroups(txn.GetCache()))
        {
        SetError();
        return;
        }

    m_serverInfo = m_ds->GetServerInfo(txn);
    txn.Commit();

    SyncNext();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncLocalChangesTask::PrepareChangeGroups(IDataSourceCache& cache)
    {
    if (!cache.GetChangeManager().HasChanges())
        {
        return SUCCESS;
        }

    ChangeManager::Changes changesToSync;
    if (m_objectsToSyncPtr == nullptr)
        {
        // Sync all
        if (SUCCESS != cache.GetChangeManager().GetChanges(changesToSync, true))
            {
            return ERROR;
            }
        }
    else
        {
        // Sync specific
        for (auto instanceKey : *m_objectsToSyncPtr)
            {
            if (SUCCESS != cache.GetChangeManager().GetChanges(instanceKey, changesToSync))
                {
                return ERROR;
                }
            }
        }

    m_changeGroups = ChangesGraph(changesToSync).BuildCacheChangeGroups();

    for (CacheChangeGroupPtr changeGroup : m_changeGroups)
        {
        if (IChangeManager::ChangeStatus::NoChange != changeGroup->GetFileChange().GetChangeStatus())
            {
            BeFileName filePath = cache.ReadFilePath(changeGroup->GetFileChange().GetInstanceKey());
            m_totalBytesToUpload += FileUtil::GetFileSize(filePath);
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncLocalChangesTask::_OnError(CachingDataSource::ErrorCR error)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncLocalChangesTask::OnSyncDone()
    {
    if (m_objectsToRefreshAfterSync.empty())
        {
        return;
        }

    m_ds->DownloadAndCacheChildren(m_objectsToRefreshAfterSync, GetCancellationToken())
        ->Then([=] (CachingDataSource::Result& result)
        {
        if (!result.IsSuccess())
            {
            SetError(result.GetError());
            }
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncLocalChangesTask::SyncNext()
    {
    if (IsTaskCanceled()) return;

    if (m_changeGroupIndexToSyncNext >= m_changeGroups.size())
        {
        ReportFinalProgress();
        OnSyncDone();
        return;
        }

    CacheChangeGroupPtr changeGroup = m_changeGroups[m_changeGroupIndexToSyncNext];

    AsyncTaskPtr<void> task;
    if (CanSyncChangeset(*changeGroup))
        {
        task = SyncNextChangeset();
        }
    else
        {
        m_changeGroupIndexToSyncNext++;
        task = SyncChangeGroup(changeGroup);
        }

    task->Then(m_ds->GetCacheAccessThread(), [=]
        {
        SyncNext();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> SyncLocalChangesTask::SyncNextChangeset()
    {
    return m_ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        if (IsTaskCanceled()) return;
        auto txn = m_ds->StartCacheTransaction();

        auto revisions = std::make_shared<RevisionMap>();
        auto changesetChangeGroups = std::make_shared<bvector<CacheChangeGroup*>>();
        auto changeset = BuildChangeset(txn.GetCache(), *revisions, *changesetChangeGroups);
        if (nullptr == changeset || changeset->IsEmpty())
            {
            SetError();
            return;
            }

        auto changesetBody = HttpStringBody::Create(std::make_shared<Utf8String>(changeset->ToRequestString()));
        ResponseGuardPtr guard = CreateResponseGuard(nullptr, false); // TODO: label

        m_ds->GetClient()->SendChangesetRequest(changesetBody, guard->GetProgressCallback(), guard)
            ->Then(m_ds->GetCacheAccessThread(), [=] (WSChangesetResult result)
            {
            if (!result.IsSuccess())
                {
                SetError(result.GetError());
                return;
                }

            rapidjson::Document changesetResponse;
            result.GetValue()->AsRapidJson(changesetResponse);

            auto handler = [&] (ObjectIdCR oldId, ObjectIdCR newId)
                {
                revisions->find(oldId)->second->SetRemoteId(newId.remoteId);
                return SUCCESS;
                };

            if (SUCCESS != changeset->ExtractNewIdsFromResponse(changesetResponse, handler))
                {
                SetError();
                return;
                };

            auto txn = m_ds->StartCacheTransaction();
            for (auto& pair : *revisions)
                {
                if (SUCCESS != txn.GetCache().GetChangeManager().CommitInstanceRevision(*pair.second))
                    {
                    SetError();
                    return;
                    };
                }

            for (auto changeGroup : *changesetChangeGroups)
                {
                changeGroup->SetSynced(true);
                }
            txn.Commit();
            });
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SyncLocalChangesTask::CanSyncChangeset(CacheChangeGroupCR changeGroup) const
    {
    return
        m_options.GetUseChangesets() &&
        m_serverInfo.GetWebApiVersion() >= BeVersion(2, 1) &&
        changeGroup.GetFileChange().GetChangeStatus() == IChangeManager::ChangeStatus::NoChange;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<bool> SyncLocalChangesTask::ShouldSyncObjectAndFileCreationSeperately(CacheChangeGroupPtr changeGroup)
    {
    ECInstanceKey fileKey = changeGroup->GetFileChange().GetInstanceKey();
    if (!fileKey.IsValid())
        return CreateCompletedAsyncTask(false);

    if (m_serverInfo.GetWebApiVersion() < BeVersion(2, 4))
        return CreateCompletedAsyncTask(false);

    if (nullptr != m_ds->m_sessionInfo->repositorySupportsFileAccessUrl)
        return CreateCompletedAsyncTask(*m_ds->m_sessionInfo->repositorySupportsFileAccessUrl);

    // TODO: 500 server issue "N~3APersonalPublishing.01.00~3APublishedFile"
    //auto txn = m_ds->StartCacheTransaction();
    //ECClassCP ecClass = txn.GetCache().GetAdapter().GetECClass(fileKey);
    //if (nullptr == ecClass)
    //    return CreateCompletedAsyncTask(false);
    //Utf8String remoteClassId = "N~3A" + Utf8String(ecClass->GetSchema().GetFullSchemaName()) + "~3A" + Utf8String(ecClass->GetName());
    //query.SetFilter(
    //    "PolicyAppliesTo-forward-MetaSchema.ECClassDef.$id+eq+'" + remoteClassId + "'+and+"
    //    "Name+eq+'SupportsFileAccessUrl'+and+"
    //    "Supported+eq+true");

    WSQuery query("Policies", "PolicyAssertion");
    query.SetSelect("$id");
    query.SetFilter("Name+eq+'SupportsFileAccessUrl'+and+Supported+eq+true");

    return m_ds->GetClient()->SendQueryRequest(query, nullptr, nullptr, GetCancellationToken())
        ->Then<bool>(m_ds->GetCacheAccessThread(), [=] (WSObjectsResult result)
        {
        if (!result.IsSuccess())
            {
            SetError(result.GetError());
            return false;
            }

        bool supportsFileAccessUrl = !result.GetValue().GetInstances().IsEmpty();
        m_ds->m_sessionInfo->repositorySupportsFileAccessUrl.reset(new bool(supportsFileAccessUrl));
        return supportsFileAccessUrl;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> SyncLocalChangesTask::SyncChangeGroup(CacheChangeGroupPtr changeGroup)
    {
    if (changeGroup->GetObjectChange().GetChangeStatus() == IChangeManager::ChangeStatus::Created ||
        changeGroup->GetRelationshipChange().GetChangeStatus() == IChangeManager::ChangeStatus::Created)
        {
        return SyncCreation(changeGroup);
        }
    else if (changeGroup->GetObjectChange().GetChangeStatus() == IChangeManager::ChangeStatus::Modified)
        {
        return SyncObjectModification(changeGroup);
        }
    else if (changeGroup->GetFileChange().GetChangeStatus() == IChangeManager::ChangeStatus::Modified)
        {
        return SyncFileModification(changeGroup);
        }
    else if (changeGroup->GetObjectChange().GetChangeStatus() == IChangeManager::ChangeStatus::Deleted ||
             changeGroup->GetRelationshipChange().GetChangeStatus() == IChangeManager::ChangeStatus::Deleted)
        {
        return SyncObjectDeletion(changeGroup);
        }
    else
        {
        BeAssert(false && "Sync not supported for this change");
        SetError();

        return CreateCompletedAsyncTask();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> SyncLocalChangesTask::SyncCreation(CacheChangeGroupPtr changeGroup)
    {
    if (changeGroup->GetFileChange().GetChangeStatus() == IChangeManager::ChangeStatus::NoChange)
        {
        return SyncObjectWithFileCreation(changeGroup, false);
        }

    return ShouldSyncObjectAndFileCreationSeperately(changeGroup)->Then([=] (bool syncSeperately)
        {
        if (IsTaskCanceled()) return;
        if (!syncSeperately)
            {
            SyncObjectWithFileCreation(changeGroup, true);
            return;
            }

        SyncObjectWithFileCreation(changeGroup, false)->Then([=]
            {
            if (IsTaskCanceled()) return;
            if (!changeGroup->IsSynced()) return;

            SyncFileModification(changeGroup)->Then(m_ds->GetCacheAccessThread(), [=]
                {
                if (IsTaskCanceled()) return;

                auto txn = m_ds->StartCacheTransaction();
                auto fileId = txn.GetCache().FindInstance(changeGroup->GetFileChange().GetInstanceKey());
                m_ds->CacheObject(fileId, GetCancellationToken())
                    ->Then(m_ds->GetCacheAccessThread(), [=] (CachingDataSource::Result result)
                    {
                    if (!result.IsSuccess())
                        SetError(result.GetError());
                    });
                });
            });
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> SyncLocalChangesTask::SyncObjectWithFileCreation(CacheChangeGroupPtr changeGroup, bool includeFile)
    {
    return m_ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        if (IsTaskCanceled()) return;
        auto txn = m_ds->StartCacheTransaction();

        Utf8String objectLabel = m_ds->GetObjectLabel(txn, changeGroup->GetObjectChange().GetInstanceKey());

        IChangeManager::FileRevisionPtr fileRevision;

        BeFileName filePath;
        uint64_t currentFileSize = 0;

        if (includeFile && changeGroup->GetFileChange().GetChangeStatus() != IChangeManager::ChangeStatus::NoChange)
            {
            fileRevision = txn.GetCache().GetChangeManager().ReadFileRevision(changeGroup->GetFileChange().GetInstanceKey());
            filePath = fileRevision->GetFilePath();
            currentFileSize = FileUtil::GetFileSize(filePath);
            }

        if (!changeGroup->AreAllDependenciesSynced())
            {
            RegisterFailedSync(txn.GetCache(), *changeGroup, CachingDataSource::Status::DependencyNotSynced, objectLabel);
            return;
            }

        auto revisions = std::make_shared<RevisionMap>();
        auto changeset = BuildSingleInstanceChangeset(txn.GetCache(), *changeGroup, *revisions);
        if (nullptr == changeset)
            {
            SetError();
            return;
            }

        Json::Value creationJson;
        changeset->ToRequestJson(creationJson);

        ResponseGuardPtr guard = CreateResponseGuard(objectLabel, 0 != currentFileSize);

        m_ds->GetClient()->SendCreateObjectRequest(creationJson, filePath, guard->GetProgressCallback(), guard)
            ->Then(m_ds->GetCacheAccessThread(), [=] (WSCreateObjectResult& objectsResult)
            {
            if (!objectsResult.IsSuccess())
                {
                m_totalBytesToUpload -= currentFileSize;
                HandleSyncError(objectsResult.GetError(), changeGroup, objectLabel);
                return;
                }

            m_totalBytesUploaded += currentFileSize;

            auto txn = m_ds->StartCacheTransaction();

            auto handler = [&] (ObjectIdCR oldId, ObjectIdCR newId)
                {
                revisions->find(oldId)->second->SetRemoteId(newId.remoteId);
                return SUCCESS;
                };

            //! TODO: return HttpBody from WSCreateObjectResult
            rapidjson::Document rapidJsonBody;
            JsonUtil::ToRapidJson(objectsResult.GetValue().GetObject(), rapidJsonBody);

            if (SUCCESS != changeset->ExtractNewIdsFromResponse(rapidJsonBody, handler))
                {
                SetError();
                return;
                };

            for (auto& pair : *revisions)
                {
                if (SUCCESS != txn.GetCache().GetChangeManager().CommitInstanceRevision(*pair.second))
                    {
                    SetError();
                    return;
                    };
                }

            if (nullptr != fileRevision)
                {
                if (SUCCESS != txn.GetCache().GetChangeManager().CommitFileRevision(*fileRevision))
                    {
                    SetError();
                    return;
                    }
                }

            changeGroup->SetSynced(true);

            if (changeGroup->GetObjectChange().GetChangeStatus() == IChangeManager::ChangeStatus::Created)
                {
                ObjectId newObjectId = txn.GetCache().FindInstance(changeGroup->GetObjectChange().GetInstanceKey());
                if (!newObjectId.IsValid())
                    {
                    SetError();
                    return;
                    }
                if (m_serverInfo.GetVersion() < BeVersion(2, 0))
                    {
                    m_ds->CacheObject(newObjectId, GetCancellationToken())
                        ->Then(m_ds->GetCacheAccessThread(), [=] (CachingDataSource::Result result)
                        {
                        if (!result.IsSuccess())
                            {
                            SetError(result.GetError());
                            return;
                            }
                        });
                    }
                else
                    {
                    // TFS#198198: Workaround for WSG 2.2 eBECPlugin Document import where created instance is of derived class
                    ECClassCP newInstanceClass = txn.GetCache().GetAdapter().GetECClass(newObjectId);
                    WSQuery query(*newInstanceClass, true);
                    query.SetFilter("$id+eq+'" + newObjectId.remoteId + "'");

                    m_ds->GetClient()->SendQueryRequest(query, nullptr, nullptr, GetCancellationToken())
                        ->Then(m_ds->GetCacheAccessThread(), [=] (WSObjectsResult result)
                        {
                        if (!result.IsSuccess())
                            {
                            SetError(result.GetError());
                            return;
                            }

                        auto txn = m_ds->StartCacheTransaction();

                        bmap<ECInstanceKey, ECInstanceKey> changedKeys;
                        if (SUCCESS != txn.GetCache().GetChangeManager().UpdateCreatedInstance(newObjectId, result.GetValue(), changedKeys))
                            {
                            SetError();
                            return;
                            }

                        for (auto& pair : changedKeys)
                            {
                            SetUpdatedInstanceKeyInChangeGroups(pair.first, pair.second);
                            }

                        txn.Commit();
                        });
                    }
                }

            txn.Commit();
            });
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncLocalChangesTask::HandleSyncError(WSErrorCR error, CacheChangeGroupPtr changeGroup, Utf8StringCR objectLabel)
    {
    if (WSError::Status::ReceivedError == error.GetStatus())
        {
        auto txn = m_ds->StartCacheTransaction();
        RegisterFailedSync(txn.GetCache(), *changeGroup, error, objectLabel);
        }
    else
        {
        SetError(error);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> SyncLocalChangesTask::SyncObjectModification(CacheChangeGroupPtr changeGroup)
    {
    return m_ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        if (IsTaskCanceled()) return;
        auto txn = m_ds->StartCacheTransaction();

        ECInstanceKey instanceKey = changeGroup->GetObjectChange().GetInstanceKey();
        auto revision = txn.GetCache().GetChangeManager().ReadInstanceRevision(instanceKey);

        Utf8String objectLabel = m_ds->GetObjectLabel(txn, revision->GetObjectId());
        Utf8String eTag = txn.GetCache().ReadInstanceCacheTag(revision->GetObjectId());
        auto properties = revision->GetChangedProperties();

        ResponseGuardPtr guard = CreateResponseGuard(objectLabel, false);
        m_ds->GetClient()->SendUpdateObjectRequest(revision->GetObjectId(), *properties, eTag, guard->GetProgressCallback(), guard)
            ->Then(m_ds->GetCacheAccessThread(), [=] (WSUpdateObjectResult& result)
            {
            if (!result.IsSuccess())
                {
                HandleSyncError(result.GetError(), changeGroup, objectLabel);
                return;
                }

            auto txn = m_ds->StartCacheTransaction();
            if (SUCCESS != txn.GetCache().GetChangeManager().CommitInstanceRevision(*revision))
                {
                SetError();
                return;
                }
            txn.Commit();
            });
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> SyncLocalChangesTask::SyncFileModification(CacheChangeGroupPtr changeGroup)
    {
    return m_ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        if (IsTaskCanceled()) return;
        auto txn = m_ds->StartCacheTransaction();

        auto instanceKey = changeGroup->GetFileChange().GetInstanceKey();
        auto revision = txn.GetCache().GetChangeManager().ReadFileRevision(instanceKey);

        ObjectId objectId = revision->GetObjectId();
        Utf8String objectLabel = m_ds->GetObjectLabel(txn, revision->GetObjectId());
        BeFileName filePath = revision->GetFilePath();
        uint64_t currentFileSize = FileUtil::GetFileSize(filePath);

        ResponseGuardPtr guard = CreateResponseGuard(objectLabel, true);

        m_ds->GetClient()->SendUpdateFileRequest(objectId, filePath, guard->GetProgressCallback(), guard)
            ->Then(m_ds->GetCacheAccessThread(), [=] (WSUpdateFileResult& result)
            {
            if (!result.IsSuccess())
                {
                m_totalBytesToUpload -= currentFileSize;
                SetError(result.GetError());
                return;
                }

            auto txn = m_ds->StartCacheTransaction();
            m_totalBytesUploaded += currentFileSize;
            if (SUCCESS != txn.GetCache().GetChangeManager().CommitFileRevision(*revision))
                {
                SetError();
                return;
                }
            txn.Commit();
            });
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> SyncLocalChangesTask::SyncObjectDeletion(CacheChangeGroupPtr changeGroup)
    {
    return m_ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        if (IsTaskCanceled()) return;

        auto txn = m_ds->StartCacheTransaction();

        ECInstanceKey instanceKey;
        if (changeGroup->GetObjectChange().GetChangeStatus() == IChangeManager::ChangeStatus::Deleted)
            {
            instanceKey = changeGroup->GetObjectChange().GetInstanceKey();
            }
        else if (changeGroup->GetRelationshipChange().GetChangeStatus() == IChangeManager::ChangeStatus::Deleted)
            {
            instanceKey = changeGroup->GetRelationshipChange().GetInstanceKey();
            }
        else
            {
            SetError();
            BeAssert(false);
            return;
            }

        auto revision = txn.GetCache().GetChangeManager().ReadInstanceRevision(instanceKey);
        Utf8String objectLabel = m_ds->GetObjectLabel(txn, revision->GetObjectId());
        ResponseGuardPtr guard = CreateResponseGuard(objectLabel, false);

        m_ds->GetClient()->SendDeleteObjectRequest(revision->GetObjectId(), guard)
            ->Then(m_ds->GetCacheAccessThread(), [=] (WSUpdateFileResult& result)
            {
            if (!result.IsSuccess())
                {
                SetError(result.GetError());
                return;
                }

            auto txn = m_ds->StartCacheTransaction();
            if (SUCCESS != txn.GetCache().GetChangeManager().CommitInstanceRevision(*revision))
                {
                SetError();
                return;
                }
            txn.Commit();
            });
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WSChangesetPtr SyncLocalChangesTask::BuildChangeset
(
IDataSourceCache& cache,
RevisionMap& revisionsOut,
bvector<CacheChangeGroup*>& changesetChangeGroupsOut
)
    {
    auto changeset = std::make_shared<WSChangeset>();

    bool changesetClipped = false;
    for (auto i = m_changeGroupIndexToSyncNext; i < m_changeGroups.size(); ++i)
        {
        CacheChangeGroup& changeGroup = *m_changeGroups[i];
        if (!CanSyncChangeset(changeGroup))
            {
            break;
            }

        RevisionMap revisions;
        WSChangeset::Instance* newInstance = AddChangeToChangeset(cache, *changeset, changeGroup, revisions, false);
        if (nullptr == newInstance)
            {
            return nullptr;
            }

        if (m_options.GetMaxChangesetSize() != 0 &&
            m_options.GetMaxChangesetSize() < changeset->CalculateSize() ||
            m_options.GetMaxChangesetInstanceCount() != 0 &&
            m_options.GetMaxChangesetInstanceCount() < changeset->GetInstanceCount())
            {
            changeset->RemoveInstance(*newInstance);
            changesetClipped = true;
            break;
            }

        revisionsOut.insert(revisions.begin(), revisions.end());
        changesetChangeGroupsOut.push_back(&changeGroup);
        m_changeGroupIndexToSyncNext += 1;
        }

    if (changeset->IsEmpty() && changesetClipped)
        {
        BeAssert(false && "Could not fit any changes into changeset. Check SyncOptions limitations");
        return nullptr;
        }

    return changeset;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
WSChangeset::Instance* SyncLocalChangesTask::AddChangeToChangeset
(
IDataSourceCache& cache,
WSChangeset& changeset,
CacheChangeGroupCR changeGroup,
RevisionMap& revisionsOut,
bool ensureChangedInstanceInRoot
)
    {
    if (IChangeManager::ChangeStatus::NoChange == changeGroup.GetRelationshipChange().GetChangeStatus())
        {
        auto& change = changeGroup.GetObjectChange();
        auto revision = cache.GetChangeManager().ReadInstanceRevision(change.GetInstanceKey());
        revisionsOut[revision->GetObjectId()] = revision;

        auto state = ToWSChangesetChangeState(revision->GetChangeStatus());
        return &changeset.AddInstance(revision->GetObjectId(), state, revision->GetChangedProperties());
        }

    auto& change = changeGroup.GetRelationshipChange();
    auto revision = cache.GetChangeManager().ReadInstanceRevision(change.GetInstanceKey());
    revisionsOut[revision->GetObjectId()] = revision;

    ObjectId relId = revision->GetObjectId();
    auto state = ToWSChangesetChangeState(revision->GetChangeStatus());

    if (WSChangeset::Deleted == state)
        {
        return &changeset.AddInstance(relId, state, nullptr);
        }
    else if (WSChangeset::Created == state)
        {
        ObjectId sourceId = cache.FindInstance(change.GetSourceKey());
        ObjectId targetId = cache.FindInstance(change.GetTargetKey());

        WSChangeset::Instance* source = changeset.FindInstance(sourceId);
        WSChangeset::Instance* target = changeset.FindInstance(targetId);

        WSChangeset::ChangeState sourceState = WSChangeset::Existing;
        WSChangeset::ChangeState targetState = WSChangeset::Existing;

        JsonValuePtr sourceProperties;
        JsonValuePtr targetProperties;

        if (IChangeManager::ChangeStatus::NoChange != changeGroup.GetObjectChange().GetChangeStatus())
            {
            auto endKey = changeGroup.GetObjectChange().GetInstanceKey();
            auto revision = cache.GetChangeManager().ReadInstanceRevision(endKey);
            revisionsOut[revision->GetObjectId()] = revision;

            if (change.GetSourceKey() == endKey)
                {
                sourceId = revision->GetObjectId();
                sourceState = ToWSChangesetChangeState(revision->GetChangeStatus());
                sourceProperties = revision->GetChangedProperties();

                if (ensureChangedInstanceInRoot && nullptr == source && nullptr == target)
                    {
                    source = &changeset.AddInstance(sourceId, sourceState, sourceProperties);
                    }
                }
            else if (change.GetTargetKey() == endKey)
                {
                targetId = revision->GetObjectId();
                targetState = ToWSChangesetChangeState(revision->GetChangeStatus());
                targetProperties = revision->GetChangedProperties();

                if (ensureChangedInstanceInRoot && nullptr == source && nullptr == target)
                    {
                    target = &changeset.AddInstance(targetId, targetState, targetProperties);
                    }
                }
            else
                {
                BeAssert(false);
                return nullptr;
                }
            }

        if (nullptr == source && nullptr == target)
            {
            source = &changeset.AddInstance(sourceId, sourceState, sourceProperties);
            target = &source->AddRelatedInstance(relId, state, ECRelatedInstanceDirection::Forward,
                                                 targetId, targetState, targetProperties);
            return source;
            }
        else if (nullptr == source)
            {
            source = &target->AddRelatedInstance(relId, state, ECRelatedInstanceDirection::Backward,
                                                 sourceId, sourceState, sourceProperties);
            return source;
            }
        else if (nullptr == target)
            {
            target = &source->AddRelatedInstance(relId, state, ECRelatedInstanceDirection::Forward,
                                                 targetId, targetState, targetProperties);
            return target;
            }
        else
            {
            BeAssert(false && "Both relationship ends are already in changeset");
            return nullptr;
            }
        }

    BeAssert(false && "Change state not supported");
    return nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WSChangesetPtr SyncLocalChangesTask::BuildSingleInstanceChangeset
(
IDataSourceCache& cache,
CacheChangeGroupCR changeGroup,
RevisionMap& revisionsOut
)
    {
    auto changeset = std::make_shared<WSChangeset>(WSChangeset::SingeInstance);

    if (nullptr == AddChangeToChangeset(cache, *changeset, changeGroup, revisionsOut, true))
        {
        return nullptr;
        }

    return changeset;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WSChangeset::ChangeState SyncLocalChangesTask::ToWSChangesetChangeState(IChangeManager::ChangeStatus status) const
    {
    // TODO: USE WSChangeset::ChangeState in IChangeManager?
    switch (status)
        {
        case IChangeManager::ChangeStatus::NoChange:
            return WSChangeset::ChangeState::Existing;
        case IChangeManager::ChangeStatus::Created:
            return WSChangeset::ChangeState::Created;
        case IChangeManager::ChangeStatus::Modified:
            return WSChangeset::ChangeState::Modified;
        case IChangeManager::ChangeStatus::Deleted:
            return WSChangeset::ChangeState::Deleted;
        default:
            break;
        }
    BeAssert(false);
    return WSChangeset::ChangeState::Existing;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncLocalChangesTask::ReportProgress(double currentFileBytesUploaded, Utf8StringCR label) const
    {
    if (!m_onProgressCallback)
        {
        return;
        }

    double synced = (double) (m_changeGroupIndexToSyncNext - 1) / m_changeGroups.size();
    synced = trunc(synced * 100) / 100;

    m_onProgressCallback(synced, label, (double) m_totalBytesUploaded + currentFileBytesUploaded, (double) m_totalBytesToUpload);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncLocalChangesTask::ReportFinalProgress() const
    {
    if (!m_onProgressCallback)
        {
        return;
        }

    m_onProgressCallback(1, "", (double) m_totalBytesUploaded, (double) m_totalBytesToUpload);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ResponseGuardPtr SyncLocalChangesTask::CreateResponseGuard(Utf8StringCR objectLabel, bool isFileBeingUploaded) const
    {
    ReportProgress(0, objectLabel);

    Http::Request::ProgressCallback onProgress;

    if (isFileBeingUploaded)
        {
        onProgress = std::bind(&SyncLocalChangesTask::ReportProgress, this, std::placeholders::_1, objectLabel);
        }

    return ResponseGuard::Create(GetCancellationToken(), onProgress);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncLocalChangesTask::RegisterFailedSync(IDataSourceCache& cache, CacheChangeGroupCR changeGroup, CachingDataSource::ErrorCR error, Utf8StringCR objectLabel)
    {
    if (changeGroup.GetObjectChange().GetChangeStatus() != IChangeManager::ChangeStatus::NoChange ||
        changeGroup.GetFileChange().GetChangeStatus() != IChangeManager::ChangeStatus::NoChange)
        {
        ObjectId objectId = cache.FindInstance(changeGroup.GetObjectChange().GetInstanceKey());
        GetFailedObjects().push_back(CachingDataSource::FailedObject(objectId, objectLabel, error));
        }
    if (changeGroup.GetRelationshipChange().GetChangeStatus() != IChangeManager::ChangeStatus::NoChange)
        {
        ObjectId objectId = cache.FindRelationship(changeGroup.GetRelationshipChange().GetInstanceKey());
        GetFailedObjects().push_back(CachingDataSource::FailedObject(objectId, objectLabel, error));
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncLocalChangesTask::SetUpdatedInstanceKeyInChangeGroups(ECInstanceKey oldKey, ECInstanceKey newKey)
    {
    if (!oldKey.IsValid() || !newKey.IsValid())
        {
        BeAssert(false);
        return;
        }

    for (auto& group : m_changeGroups)
        {
        if (group->GetFileChange().GetInstanceKey() == oldKey)
            {
            group->GetFileChange().SetInstanceKey(newKey);
            }
        if (group->GetObjectChange().GetInstanceKey() == oldKey)
            {
            group->GetObjectChange().SetInstanceKey(newKey);
            }
        if (group->GetRelationshipChange().GetInstanceKey() == oldKey)
            {
            group->GetRelationshipChange().SetInstanceKey(newKey);
            }
        if (group->GetRelationshipChange().GetSourceKey() == oldKey)
            {
            group->GetRelationshipChange().SetSourceKey(newKey);
            }
        if (group->GetRelationshipChange().GetTargetKey() == oldKey)
            {
            group->GetRelationshipChange().SetTargetKey(newKey);
            }
        }
    }
