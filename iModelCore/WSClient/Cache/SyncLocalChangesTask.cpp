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
#include <MobileDgn/Utils/Http/HttpStatusHelper.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             William.Francis         01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SyncLocalChangesTask::SyncLocalChangesTask
(
CachingDataSourcePtr cachingDataSource,
std::shared_ptr<bset<ECInstanceKey>> objectsToSync,
CachingDataSource::SyncProgressCallback&& onProgress,
ICancellationTokenPtr cancellationToken
) :
CachingTaskBase(cachingDataSource, cancellationToken),
m_onProgressCallback(onProgress),
m_objectsToSyncPtr(objectsToSync),
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

    SyncNextChangeGroup();
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
void SyncLocalChangesTask::SyncNextChangeGroup()
    {
    if (IsTaskCanceled()) return;

    if (m_changeGroupIndexToSyncNext >= m_changeGroups.size())
        {
        ReportProgress(0, "");
        OnSyncDone();
        return;
        }

    ChangeGroupPtr changeGroup = m_changeGroups[m_changeGroupIndexToSyncNext];

    SyncChangeGroup(changeGroup)
        ->Then(m_ds->GetCacheAccessThread(), [=]
        {
        m_changeGroupIndexToSyncNext++;
        SyncNextChangeGroup();
        });
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
            BeAssert(false || "One or more dependencies were not synced");
            RegisterFailedSync(txn.GetCache(), *changeGroup, CachingDataSource::Status::DependencyNotSynced, objectLabel);
            return;
            }

        Json::Value creationJson;
        if (SUCCESS != BuildSyncJson(txn.GetCache(), *changeGroup, creationJson))
            {
            SetError(CachingDataSource::Status::InternalCacheError);
            return;
            }

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

            std::map<ECInstanceKey, Utf8String> changedRemoteIds = ReadChangedRemoteIds(*changeGroup, objectsResult.GetValue());
            if (SUCCESS != txn.GetCache().GetChangeManager().CommitCreationChanges(changedRemoteIds))
                {
                SetError(CachingDataSource::Status::InternalCacheError);
                return;
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
            if (SUCCESS != txn.GetCache().GetChangeManager().CommitObjectChanges(instanceKey))
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
            if (SUCCESS != txn.GetCache().GetChangeManager().CommitFileChanges(instanceKey))
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
            if (SUCCESS != txn.GetCache().GetChangeManager().CommitObjectChanges(instanceKey))
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
BentleyStatus SyncLocalChangesTask::BuildSyncJson(IDataSourceCache& cache, ChangeGroupCR changeGroup, JsonValueR syncJsonOut) const
    {
    if (changeGroup.GetObjectChange().GetChangeStatus() == IChangeManager::ChangeStatus::Created)
        {
        return BuildSyncJsonForObjectCreation(cache, changeGroup, syncJsonOut);
        }
    else if (changeGroup.GetRelationshipChange().GetChangeStatus() == IChangeManager::ChangeStatus::Created)
        {
        return BuildSyncJsonForRelationshipCreation(cache, changeGroup.GetRelationshipChange(), syncJsonOut);
        }
    BeAssert(false || "Not supported change group");
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncLocalChangesTask::BuildSyncJsonForObjectCreation(IDataSourceCache& cache, ChangeGroupCR changeGroup, JsonValueR syncJsonOut) const
    {
    JsonValueR instance = syncJsonOut["instance"];

    Json::Value propertiesJson;
    if (SUCCESS != ReadObjectPropertiesForCreation(cache, changeGroup.GetObjectChange().GetInstanceKey(), propertiesJson))
        {
        return ERROR;
        }

    SetChangedInstanceClassInfoToJson(cache, changeGroup.GetObjectChange(), instance);
    instance["properties"] = propertiesJson;

    if (changeGroup.GetRelationshipChange().GetChangeStatus() != IChangeManager::ChangeStatus::NoChange)
        {
        JsonValueR relationshipInstanceJson = instance["relationshipInstances"][0];

        SetChangedInstanceClassInfoToJson(cache, changeGroup.GetRelationshipChange(), relationshipInstanceJson);
        relationshipInstanceJson["properties"] = Json::objectValue;

        if (changeGroup.GetRelationshipChange().GetSourceKey() == changeGroup.GetObjectChange().GetInstanceKey())
            {
            relationshipInstanceJson["direction"] = "forward";
            SetExistingInstanceInfoToJson(cache, changeGroup.GetRelationshipChange().GetTargetKey(), relationshipInstanceJson["relatedInstance"]);
            }
        else if (changeGroup.GetRelationshipChange().GetTargetKey() == changeGroup.GetObjectChange().GetInstanceKey())
            {
            relationshipInstanceJson["direction"] = "backward";
            SetExistingInstanceInfoToJson(cache, changeGroup.GetRelationshipChange().GetSourceKey(), relationshipInstanceJson["relatedInstance"]);
            }
        else
            {
            BeAssert(false);
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SyncLocalChangesTask::BuildSyncJsonForRelationshipCreation
(
IDataSourceCache& cache,
ChangeManager::RelationshipChangeCR relationshipChange,
JsonValueR syncJsonOut
) const
    {
    JsonValueR instance = syncJsonOut["instance"];

    SetExistingInstanceInfoToJson(cache, relationshipChange.GetSourceKey(), instance);
    SetExistingInstanceInfoToJson(cache, relationshipChange.GetTargetKey(), instance["relationshipInstances"][0]["relatedInstance"]);

    SetChangedInstanceClassInfoToJson(cache, relationshipChange, instance["relationshipInstances"][0]);
    instance["relationshipInstances"][0]["properties"] = Json::objectValue;
    instance["relationshipInstances"][0]["direction"] = "forward";

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncLocalChangesTask::SetExistingInstanceInfoToJson(IDataSourceCache& cache, ECInstanceKeyCR instanceKey, JsonValueR json) const
    {
    ObjectId objectId = cache.FindInstance(instanceKey);
    json["schemaName"] = objectId.schemaName;
    json["className"] = objectId.className;
    json["instanceId"] = objectId.remoteId;
    json["changeState"] = GetChangeStateStr(IChangeManager::ChangeStatus::NoChange);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncLocalChangesTask::SetChangedInstanceClassInfoToJson(IDataSourceCache& cache, IChangeManager::ObjectChangeCR change, JsonValueR json) const
    {
    ECClassCP ecClass = cache.GetAdapter().GetECClass(change.GetInstanceKey());
    json["schemaName"] = Utf8String(ecClass->GetSchema().GetName());
    json["className"] = Utf8String(ecClass->GetName());
    json["changeState"] = GetChangeStateStr(change.GetChangeStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SyncLocalChangesTask::GetChangeStateStr(IChangeManager::ChangeStatus changeStatus) const
    {
    switch (changeStatus)
        {
        case IChangeManager::ChangeStatus::NoChange:
            return "existing";
        case IChangeManager::ChangeStatus::Created:
            return "new";
        case IChangeManager::ChangeStatus::Modified:
            return "modified";
        case IChangeManager::ChangeStatus::Deleted:
            return "deleted";
        default:
            BeAssert(false);
            break;
        }
    return nullptr;
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
    if (SUCCESS != ReadObjectProperties(cache, instanceKey, propertiesJsonOut))
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
        if (ecProperty->GetIsReadOnly())
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
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncLocalChangesTask::ReportProgress(double currentFileBytesUploaded, Utf8StringCR label) const
    {
    double synced = (double) m_changeGroupIndexToSyncNext / m_changeGroups.size();
    synced = trunc(synced * 100) / 100;

    if (m_onProgressCallback)
        {
        m_onProgressCallback(synced, label, (double) m_totalBytesUploaded + currentFileBytesUploaded, (double) m_totalBytesToUpload);
        }
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
