/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/SyncLocalChangesTask.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "SyncLocalChangesTask.h"

#include <Bentley/BeTimeUtilities.h>

#include <WebServices/Cache/Util/FileUtil.h>
#include <WebServices/Client/WSChangeset.h>
#include <MobileDgn/Utils/Http/HttpStatusHelper.h>

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
ICancellationTokenPtr cancellationToken
) :
CachingTaskBase(cachingDataSource, cancellationToken),
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

    m_serverInfo = m_ds->GetServerInfo(txn);

    if (!txn.GetCache().GetChangeManager().HasChanges())
        {
        OnSyncDone();
        return;
        }

    ChangeManager::Changes changesToSync;
    if (m_objectsToSyncPtr == nullptr)
        {
        // sync all
        //depricated.    SyncAll just has an array of a list of items we are going to sync.
        if (SUCCESS != txn.GetCache().GetChangeManager().GetChanges(changesToSync, true))
            {
            SetError(CachingDataSource::Status::InternalCacheError);
            return;
            }
        }
    else
        {
        // sync specific
        for (auto instanceKey : *m_objectsToSyncPtr)
            {
            txn.GetCache().GetChangeManager().GetChanges(instanceKey, changesToSync);
            }
        }

    m_changeGroups = ChangesGraph(changesToSync).BuildChangeGroups();
    if (m_changeGroups.empty())
        {
        return;
        }

    for (ChangeGroupPtr changeGroup : m_changeGroups)
        {
        if (IChangeManager::ChangeStatus::NoChange != changeGroup->GetFileChange().GetChangeStatus())
            {
            BeFileName filePath = txn.GetCache().ReadFilePath(changeGroup->GetFileChange().GetInstanceKey());
            m_totalBytesToUpload += FileUtil::GetFileSize(filePath);
            }
        }

    SyncNext();
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

    ChangeGroupPtr changeGroup = m_changeGroups[m_changeGroupIndexToSyncNext];

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

        auto changesetIdMap = std::make_shared<bmap<ObjectId, ECInstanceKey>>();
        auto changesetChangeGroups = std::make_shared<bvector<ChangeGroup*>>();
        auto changeset = BuildChangeset(txn.GetCache(), *changesetIdMap, *changesetChangeGroups);
        if (nullptr == changeset)
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

            std::map<ECInstanceKey, Utf8String> createdInstanceIds;
            auto handler = [&] (ObjectIdCR oldId, ObjectIdCR newId)
                {
                createdInstanceIds[(*changesetIdMap)[oldId]] = newId.remoteId;
                return SUCCESS;
                };

            if (SUCCESS != changeset->ExtractNewIdsFromResponse(changesetResponse, handler))
                {
                SetError();
                return;
                };

            auto txn = m_ds->StartCacheTransaction();
            for (auto& pair : createdInstanceIds)
                {
                if (SUCCESS != txn.GetCache().GetChangeManager().CommitCreationChange(pair.first, pair.second))
                    {
                    SetError();
                    return;
                    };
                }

            for (auto& pair : *changesetIdMap)
                {
                ECInstanceKeyCR changedInstanceKey = pair.second;
                if (createdInstanceIds.find(changedInstanceKey) != createdInstanceIds.end())
                    {
                    continue;
                    }
                if (SUCCESS != txn.GetCache().GetChangeManager().CommitObjectChange(changedInstanceKey))
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
bool SyncLocalChangesTask::CanSyncChangeset(ChangeGroupCR changeGroup) const
    {
    return
        m_options.GetUseChangesets() &&
        m_serverInfo.GetWebApiVersion() >= BeVersion(2, 1) &&
        changeGroup.GetFileChange().GetChangeStatus() == IChangeManager::ChangeStatus::NoChange;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> SyncLocalChangesTask::SyncChangeGroup(ChangeGroupPtr changeGroup)
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
        SetError(CachingDataSource::Status::InternalCacheError);

        return CreateCompletedAsyncTask();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> SyncLocalChangesTask::SyncCreation(ChangeGroupPtr changeGroup)
    {
    return m_ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        if (IsTaskCanceled()) return;
        auto txn = m_ds->StartCacheTransaction();

        Utf8String objectLabel = m_ds->GetObjectLabel(txn, changeGroup->GetObjectChange().GetInstanceKey());

        BeFileName filePath;
        uint64_t currentFileSize = 0;

        if (changeGroup->GetFileChange().GetChangeStatus() != IChangeManager::ChangeStatus::NoChange)
            {
            filePath = txn.GetCache().ReadFilePath(changeGroup->GetFileChange().GetInstanceKey());
            currentFileSize = FileUtil::GetFileSize(filePath);
            }

        if (!changeGroup->AreAllDependenciesSynced())
            {
            BeAssert(false && "One or more dependencies were not synced");
            RegisterFailedSync(txn.GetCache(), *changeGroup, CachingDataSource::Status::DependencyNotSynced, objectLabel);
            return;
            }

        auto changeset = BuildSingleInstanceChangeset(txn.GetCache(), *changeGroup);
        if (nullptr == changeset)
            {
            SetError(CachingDataSource::Status::InternalCacheError);
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
                HandleCreationError(objectsResult.GetError(), changeGroup, objectLabel);
                return;
                }

            m_totalBytesUploaded += currentFileSize;

            auto txn = m_ds->StartCacheTransaction();

            for (auto& pair : ReadChangedRemoteIds(*changeGroup, objectsResult.GetValue()))
                {
                if (SUCCESS != txn.GetCache().GetChangeManager().CommitCreationChange(pair.first, pair.second))
                    {
                    SetError(CachingDataSource::Status::InternalCacheError);
                    return;
                    }
                }

            if (changeGroup->GetFileChange().GetChangeStatus() != IChangeManager::ChangeStatus::NoChange)
                {
                if (SUCCESS != txn.GetCache().GetChangeManager().CommitFileChange(changeGroup->GetFileChange().GetInstanceKey()))
                    {
                    SetError(CachingDataSource::Status::InternalCacheError);
                    return;
                    }
                }

            changeGroup->SetSynced(true);

            if (changeGroup->GetObjectChange().GetChangeStatus() == IChangeManager::ChangeStatus::Created)
                {
                ObjectId newObjectId = txn.GetCache().FindInstance(changeGroup->GetObjectChange().GetInstanceKey());
                if (m_ds->GetServerInfo(txn).GetVersion() < BeVersion(2, 0))
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

                    m_ds->GetClient()->SendQueryRequest(query, nullptr, GetCancellationToken())
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
                            SetError(CachingDataSource::Status::InternalCacheError);
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
void SyncLocalChangesTask::HandleCreationError(WSErrorCR error, ChangeGroupPtr changeGroup, Utf8StringCR objectLabel)
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
AsyncTaskPtr<void> SyncLocalChangesTask::SyncObjectModification(ChangeGroupPtr changeGroup)
    {
    return m_ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        if (IsTaskCanceled()) return;
        auto txn = m_ds->StartCacheTransaction();

        ECInstanceKey instanceKey = changeGroup->GetObjectChange().GetInstanceKey();
        ObjectId objectId = txn.GetCache().FindInstance(instanceKey);

        Utf8String objectLabel = m_ds->GetObjectLabel(txn, objectId);
        Utf8String eTag = txn.GetCache().ReadInstanceCacheTag(objectId);

        Json::Value propertiesJson;
        if (SUCCESS != ReadObjectPropertiesForUpdate(txn.GetCache(), instanceKey, propertiesJson))
            {
            SetError(CachingDataSource::Status::InternalCacheError);
            return;
            }

        ResponseGuardPtr guard = CreateResponseGuard(objectLabel, false);
        m_ds->GetClient()->SendUpdateObjectRequest(objectId, propertiesJson, eTag, guard->GetProgressCallback(), guard)
            ->Then(m_ds->GetCacheAccessThread(), [=] (WSUpdateObjectResult& result)
            {
            if (!result.IsSuccess())
                {
                SetError(result.GetError());
                return;
                }

            auto txn = m_ds->StartCacheTransaction();
            if (SUCCESS != txn.GetCache().GetChangeManager().CommitObjectChange(instanceKey))
                {
                SetError(CachingDataSource::Status::InternalCacheError);
                return;
                }
            txn.Commit();
            });
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> SyncLocalChangesTask::SyncFileModification(ChangeGroupPtr changeGroup)
    {
    return m_ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        if (IsTaskCanceled()) return;
        auto txn = m_ds->StartCacheTransaction();

        ECInstanceKey instanceKey = changeGroup->GetFileChange().GetInstanceKey();
        ObjectId objectId = txn.GetCache().FindInstance(instanceKey);

        Utf8String objectLabel = m_ds->GetObjectLabel(txn, objectId);
        BeFileName filePath = txn.GetCache().ReadFilePath(objectId);
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
            if (SUCCESS != txn.GetCache().GetChangeManager().CommitFileChange(instanceKey))
                {
                SetError(CachingDataSource::Status::InternalCacheError);
                return;
                }
            txn.Commit();
            });
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> SyncLocalChangesTask::SyncObjectDeletion(ChangeGroupPtr changeGroup)
    {
    return m_ds->GetCacheAccessThread()->ExecuteAsync([=]
        {
        if (IsTaskCanceled()) return;

        auto txn = m_ds->StartCacheTransaction();

        ObjectId objectId;
        ECInstanceKey instanceKey;

        if (changeGroup->GetObjectChange().GetChangeStatus() == IChangeManager::ChangeStatus::Deleted)
            {
            instanceKey = changeGroup->GetObjectChange().GetInstanceKey();
            objectId = txn.GetCache().FindInstance(instanceKey);
            }
        else if (changeGroup->GetRelationshipChange().GetChangeStatus() == IChangeManager::ChangeStatus::Deleted)
            {
            instanceKey = changeGroup->GetRelationshipChange().GetInstanceKey();
            objectId = txn.GetCache().FindRelationship(instanceKey);
            }
        else
            {
            SetError(CachingDataSource::Status::InternalCacheError);
            BeAssert(false);
            return;
            }

        Utf8String objectLabel = m_ds->GetObjectLabel(txn, objectId);
        ResponseGuardPtr guard = CreateResponseGuard(objectLabel, false);

        m_ds->GetClient()->SendDeleteObjectRequest(objectId, guard)
            ->Then(m_ds->GetCacheAccessThread(), [=] (WSUpdateFileResult& result)
            {
            if (!result.IsSuccess())
                {
                SetError(result.GetError());
                return;
                }

            auto txn = m_ds->StartCacheTransaction();
            if (SUCCESS != txn.GetCache().GetChangeManager().CommitObjectChange(instanceKey))
                {
                SetError(CachingDataSource::Status::InternalCacheError);
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
bmap<ObjectId, ECInstanceKey>& changesetIdMapOut,
bvector<ChangeGroup*>& changesetChangeGroupsOut
)
    {
    auto changeset = std::make_shared<WSChangeset>();
    for (auto i = m_changeGroupIndexToSyncNext; i < m_changeGroups.size(); ++i)
        {
        ChangeGroup& changeGroup = *m_changeGroups[i];
        if (!CanSyncChangeset(changeGroup))
            {
            break;
            }

        bmap<ObjectId, ECInstanceKey> changeIds;
        WSChangeset::Instance* newInstance = AddChangeToChangeset(cache, *changeset, changeGroup, changeIds, false);
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
            break;
            }

        changesetIdMapOut.insert(changeIds.begin(), changeIds.end());
        changesetChangeGroupsOut.push_back(&changeGroup);
        m_changeGroupIndexToSyncNext += 1;
        }

    if (changesetChangeGroupsOut.empty())
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
ChangeGroupCR changeGroup,
bmap<ObjectId, ECInstanceKey>& changesetIdMapOut,
bool ensureChangedInstanceInRoot
)
    {
    if (IChangeManager::ChangeStatus::NoChange == changeGroup.GetRelationshipChange().GetChangeStatus())
        {
        auto& change = changeGroup.GetObjectChange();
        auto state = ToWSChangesetChangeState(change.GetChangeStatus());

        auto properties = ReadChangeProperties(cache, state, change.GetInstanceKey());
        auto objectId = cache.FindInstance(change.GetInstanceKey());

        changesetIdMapOut[objectId] = change.GetInstanceKey();
        return &changeset.AddInstance(objectId, state, properties);
        }

    auto& change = changeGroup.GetRelationshipChange();
    auto state = ToWSChangesetChangeState(change.GetChangeStatus());
    ObjectId relId = cache.FindRelationship(change.GetInstanceKey());
    changesetIdMapOut[relId] = change.GetInstanceKey();

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
            if (change.GetSourceKey() == changeGroup.GetObjectChange().GetInstanceKey())
                {
                sourceState = ToWSChangesetChangeState(changeGroup.GetObjectChange().GetChangeStatus());
                sourceProperties = ReadChangeProperties(cache, sourceState, change.GetSourceKey());
                changesetIdMapOut[sourceId] = change.GetSourceKey();
                if (ensureChangedInstanceInRoot && nullptr == source && nullptr == target)
                    {
                    source = &changeset.AddInstance(sourceId, sourceState, sourceProperties);
                    }
                }
            else
                {
                targetState = ToWSChangesetChangeState(changeGroup.GetObjectChange().GetChangeStatus());
                targetProperties = ReadChangeProperties(cache, targetState, change.GetTargetKey());
                changesetIdMapOut[targetId] = change.GetTargetKey();
                if (ensureChangedInstanceInRoot && nullptr == source && nullptr == target)
                    {
                    target = &changeset.AddInstance(targetId, targetState, targetProperties);
                    }
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
WSChangesetPtr SyncLocalChangesTask::BuildSingleInstanceChangeset(IDataSourceCache& cache, ChangeGroupCR changeGroup)
    {
    auto changeset = std::make_shared<WSChangeset>(WSChangeset::SingeInstance);

    bmap<ObjectId, ECInstanceKey> changeIds;
    if (nullptr == AddChangeToChangeset(cache, *changeset, changeGroup, changeIds, true))
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
JsonValuePtr SyncLocalChangesTask::ReadChangeProperties
(
IDataSourceCache& cache,
WSChangeset::ChangeState state,
ECInstanceKeyCR instance
) const
    {
    auto properties = std::make_shared<Json::Value>();
    if (WSChangeset::Created == state)
        {
        if (SUCCESS != ReadObjectPropertiesForCreation(cache, instance, *properties))
            {
            BeAssert(false);
            return nullptr;
            }
        }
    else if (WSChangeset::Modified == state)
        {
        if (SUCCESS != ReadObjectPropertiesForUpdate(cache, instance, *properties))
            {
            BeAssert(false);
            return nullptr;
            }
        }
    return properties;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncLocalChangesTask::ReadObjectPropertiesForCreation(IDataSourceCache& cache, ECInstanceKeyCR instanceKey, JsonValueR propertiesJsonOut) const
    {
    if (SUCCESS != ReadObjectProperties(cache, instanceKey, propertiesJsonOut))
        {
        return ERROR;
        }

    ECClassCP ecClass = cache.GetAdapter().GetECClass(instanceKey.GetECClassId());
    RemoveCalculatedProperties(propertiesJsonOut, *ecClass);

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncLocalChangesTask::ReadObjectPropertiesForUpdate(IDataSourceCache& cache, ECInstanceKeyCR instanceKey, JsonValueR propertiesJsonOut) const
    {
    if (SUCCESS != cache.GetChangeManager().ReadModifiedProperties(instanceKey, propertiesJsonOut))
        {
        return ERROR;
        }

    ECClassCP ecClass = cache.GetAdapter().GetECClass(instanceKey);
    RemoveReadOnlyProperties(propertiesJsonOut, *ecClass);

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncLocalChangesTask::ReadObjectProperties(IDataSourceCache& cache, ECInstanceKeyCR instanceKey, JsonValueR propertiesJsonOut) const
    {
    if (SUCCESS != cache.GetAdapter().GetJsonInstance(propertiesJsonOut, instanceKey))
        {
        return ERROR;
        }

    RemoveCacheSpecificProperties(propertiesJsonOut);

    // Graphite03 ECDb is incapable of storing NULLABLE structs and arrays so it returns structs with empty array properties.
    // This causes errors to be returned from the server.
    // TODO: verify if still needed
    RemoveEmptyMembersRecursively(propertiesJsonOut);

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncLocalChangesTask::RemoveCacheSpecificProperties(JsonValueR propertiesJson) const
    {
    for (Utf8StringCR member : propertiesJson.getMemberNames())
        {
        // Remove cache-specific properties

        // TODO: Remove "type_string" check when implemented sending only changed properties to WSG
        // "type_string" is not cache specific, it probably came as a workaround from PW_WSG 1.1 schema as it is readonly but not marked so
        if (member[0] == '$' || member == "type_string")
            {
            propertiesJson.removeMember(member);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncLocalChangesTask::RemoveReadOnlyProperties(JsonValueR propertiesJson, ECClassCR ecClass) const
    {
    for (Utf8StringCR member : propertiesJson.getMemberNames())
        {
        ECPropertyCP ecProperty = ecClass.GetPropertyP(member.c_str());
        if (nullptr != ecProperty && ecProperty->GetIsReadOnly())
            {
            propertiesJson.removeMember(member);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncLocalChangesTask::RemoveCalculatedProperties(JsonValueR propertiesJson, ECClassCR ecClass) const
    {
    for (Utf8StringCR member : propertiesJson.getMemberNames())
        {
        ECPropertyCP ecProperty = ecClass.GetPropertyP(member.c_str());
        if (nullptr != ecProperty->GetCalculatedPropertySpecification())
            {
            propertiesJson.removeMember(member);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
std::map<ECInstanceKey, Utf8String> SyncLocalChangesTask::ReadChangedRemoteIds(ChangeGroupCR changeGroup, WSCreateObjectResponseCR response) const
    {
    std::map<ECInstanceKey, Utf8String> remoteIdMap;

    // TODO: return WSChangedObjectsResponse from WSRepositoryClient
    JsonValueCR changedInstance = response.GetObject()["changedInstance"]["instanceAfterChange"];

    if (changeGroup.GetObjectChange().GetChangeStatus() == IChangeManager::ChangeStatus::Created)
        {
        remoteIdMap[changeGroup.GetObjectChange().GetInstanceKey()] = changedInstance["instanceId"].asString();
        }

    if (changeGroup.GetRelationshipChange().GetChangeStatus() == IChangeManager::ChangeStatus::Created)
        {
        remoteIdMap[changeGroup.GetRelationshipChange().GetInstanceKey()] = changedInstance["relationshipInstances"][0]["instanceId"].asString();
        }

    return remoteIdMap;
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

    HttpRequest::ProgressCallback onProgress;

    if (isFileBeingUploaded)
        {
        onProgress = std::bind(&SyncLocalChangesTask::ReportProgress, this, std::placeholders::_1, objectLabel);
        }

    return ResponseGuard::Create(GetCancellationToken(), onProgress);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncLocalChangesTask::RegisterFailedSync(IDataSourceCache& cache, ChangeGroupCR changeGroup, CachingDataSource::ErrorCR error, Utf8StringCR objectLabel)
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncLocalChangesTask::RemoveEmptyMembersRecursively(JsonValueR jsonObject)
    {
    for (Utf8StringCR memberName : jsonObject.getMemberNames())
        {
        RemoveEmptyMembersRecursively(jsonObject[memberName], memberName, jsonObject);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncLocalChangesTask::RemoveEmptyMembersRecursively(JsonValueR childJson, Utf8StringCR childMemberNameInParent, JsonValueR parentJson)
    {
    if (childJson.isObject())
        {
        for (Utf8StringCR memberName : childJson.getMemberNames())
            {
            RemoveEmptyMembersRecursively(childJson[memberName], memberName, childJson);
            }
        }
    if (childJson.empty())
        {
        parentJson.removeMember(childMemberNameInParent);
        return;
        }
    }
