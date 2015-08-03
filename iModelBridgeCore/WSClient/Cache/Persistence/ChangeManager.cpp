/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/ChangeManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Persistence/ChangeManager.h>

#include <WebServices/Cache/Util/ECDbHelper.h>

#include "Changes/ChangeInfoManager.h"
#include "Core/CacheSchema.h"
#include "Core/SchemaManager.h"
#include "Files/FileCacheManager.h"
#include "Files/FileInfoManager.h"
#include "Hierarchy/HierarchyManager.h"
#include "Hierarchy/RootManager.h"
#include "Instances/InstanceCacheHelper.h"
#include "Instances/ObjectInfoManager.h"
#include "Instances/RelationshipInfoManager.h"
#include "Responses/CachedResponseManager.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

Utf8CP ChangeManager::NewObjectIdPrefix = "NewObject";

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ChangeManager::ChangeManager
(
ECDbAdapter& dbAdapter,
InstanceCacheHelper& instanceHelper,
HierarchyManager& hierarchyManager,
CachedResponseManager& responseManager,
ObjectInfoManager& objectInfoManager,
RelationshipInfoManager& relationshipInfoManager,
FileInfoManager& fileInfoManager,
ChangeInfoManager& changeInfoManager,
FileCacheManager& fileManager,
RootManager& rootManager
) :
m_dbAdapter(&dbAdapter),
m_instanceHelper(&instanceHelper),
m_hierarchyManager(&hierarchyManager),
m_responseManager(&responseManager),
m_objectInfoManager(&objectInfoManager),
m_relationshipInfoManager(&relationshipInfoManager),
m_fileInfoManager(&fileInfoManager),
m_changeInfoManager(&changeInfoManager),
m_fileManager(&fileManager),
m_rootManager(&rootManager),
m_isSyncActive(false)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Vytenis.Navalinskas  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ChangeManager::CreateRemoteId()
    {
    return NewObjectIdPrefix + BeGuid().ToString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool ChangeManager::IsSyncActive() const
    {
    return m_isSyncActive;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ChangeManager::SetSyncActive(bool active)
    {
    m_isSyncActive = active;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey ChangeManager::LegacyCreateObject(ECClassCR ecClass, JsonValueCR properties, ECInstanceKeyCR parentKey, SyncStatus syncStatus)
    {
    ECInstanceKey newInstanceKey = CreateObject(ecClass, properties, syncStatus);
    if (!newInstanceKey.IsValid())
        {
        return ECInstanceKey();
        }

    if (!CreateRelationship(*GetLegacyParentRelationshipClass(), parentKey, newInstanceKey).IsValid())
        {
        return ECInstanceKey();
        }

    return newInstanceKey;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP ChangeManager::GetLegacyParentRelationshipClass()
    {
    return m_dbAdapter->GetECRelationshipClass(SCHEMA_CacheLegacySupportSchema, SCHEMA_CacheLegacySupportSchema_LegacyParentRelationship);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECInstanceKey ChangeManager::CreateObject(ECClassCR ecClass, JsonValueCR properties, SyncStatus syncStatus)
    {
    ObjectId newObjectId(ecClass, CreateRemoteId());
    ObjectInfo info = m_objectInfoManager->ReadInfo(newObjectId);
    if (info.IsInCache())
        {
        BeAssert(false && "Object with same ObjectId already exists");
        return ECInstanceKey();
        }

    if (SUCCESS != m_changeInfoManager->SetupChangeNumber(info))
        {
        return ECInstanceKey();
        }

    info.SetObjectCacheDate(DateTime::GetCurrentTimeUtc());
    info.SetRemoteId(newObjectId.remoteId);
    info.SetChangeStatus(ChangeStatus::Created);
    info.SetSyncStatus(syncStatus);

    rapidjson::Document propertiesRapidJson;
    if (propertiesRapidJson.Parse<0>(properties.toStyledString().c_str()).HasParseError())
        {
        BeAssert(false && "Invalid JSON properties");
        return ECInstanceKey();
        }

    if (SUCCESS != m_instanceHelper->CacheNewInstance(info, propertiesRapidJson))
        {
        return ECInstanceKey();
        }

    return info.GetCachedInstanceKey();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeManager::ModifyObject(ECInstanceKeyCR instanceKey, JsonValueCR properties, SyncStatus syncStatus)
    {
    ObjectInfo info = m_objectInfoManager->ReadInfo(instanceKey);
    if (!info.IsInCache())
        {
        BeAssert(false && "Object is not it cache");
        return ERROR;
        }

    if (info.GetChangeStatus() != ChangeStatus::NoChange && IsSyncActive())
        {
        BeAssert(false && "Cannot modify object while syncing");
        return ERROR;
        }

    if (info.GetChangeStatus() == ChangeStatus::Deleted)
        {
        BeAssert(false && "Cannot modify object that is deleted");
        return ERROR;
        }

    if (SUCCESS != m_changeInfoManager->SetupChangeNumber(info))
        {
        return ERROR;
        }

    if (info.GetChangeStatus() != ChangeStatus::Created)
        {
        info.SetChangeStatus(ChangeStatus::Modified);
        }

    info.SetObjectCacheDate(DateTime::GetCurrentTimeUtc());
    info.SetSyncStatus(syncStatus);

    rapidjson::Document propertiesRapidJson;
    if (propertiesRapidJson.Parse<0>(properties.toStyledString().c_str()).HasParseError())
        {
        BeAssert(false && "Invalid JSON properties");
        return ERROR;
        }

    if (SUCCESS != m_instanceHelper->CacheExistingInstance(info, propertiesRapidJson))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeManager::DeleteObject(ECInstanceKeyCR instanceKey, SyncStatus syncStatus)
    {
    ObjectInfo info = m_objectInfoManager->ReadInfo(instanceKey);
    if (!info.IsInCache())
        {
        BeAssert(false && "Object is not it cache");
        return ERROR;
        }

    if (info.GetChangeStatus() != ChangeStatus::NoChange && IsSyncActive())
        {
        BeAssert(false && "Cannot delete changed object while syncing");
        return ERROR;
        }

    if (info.GetChangeStatus() == ChangeStatus::Deleted)
        {
        BeAssert(false && "Object is already deleted");
        return ERROR;
        }

    if (info.GetChangeStatus() == ChangeStatus::Created)
        {
        if (SUCCESS != m_hierarchyManager->DeleteInstance(info.GetCachedInstanceKey()))
            {
            return ERROR;
            }
        }
    else
        {
        // Reset change number
        info.ClearChangeInfo();

        if (SUCCESS != m_changeInfoManager->SetupChangeNumber(info))
            {
            return ERROR;
            }

        info.SetChangeStatus(ChangeStatus::Deleted);
        info.SetSyncStatus(syncStatus);

        if (SUCCESS != m_objectInfoManager->DeleteInstanceLeavingInfo(info))
            {
            return ERROR;
            }

        if (SUCCESS != m_objectInfoManager->UpdateInfo(info))
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeManager::ModifyFile(ECInstanceKeyCR instanceKey, BeFileNameCR filePath, bool copyFile, SyncStatus syncStatus)
    {
    if (!m_objectInfoManager->ReadInfo(instanceKey).IsInCache())
        {
        return ERROR;
        }

    FileInfo info = m_fileInfoManager->ReadInfo(instanceKey);
    if (info.GetChangeStatus() != ChangeStatus::NoChange && IsSyncActive())
        {
        BeAssert(false && "Cannot change modified file while syncing");
        return ERROR;
        }

    if (SUCCESS != m_changeInfoManager->SetupChangeNumber(info))
        {
        return ERROR;
        }

    info.SetChangeStatus(ChangeStatus::Modified);
    info.SetSyncStatus(syncStatus);

    if (SUCCESS != m_fileInfoManager->SaveInfo(info))
        {
        BeAssert(false);
        return ERROR;
        }

    if (SUCCESS != m_fileManager->CacheFile(instanceKey, filePath, nullptr, FileCache::Persistent, DateTime::GetCurrentTimeUtc(), copyFile))
        {
        BeAssert(false);
        return ERROR;
        };

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey ChangeManager::CreateRelationship
(
ECRelationshipClassCR relationshipClass,
ECInstanceKeyCR source,
ECInstanceKeyCR target,
SyncStatus syncStatus
)
    {
    return CreateRelationship(relationshipClass, source, target, syncStatus, 0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey ChangeManager::CreateRelationship
(
ECRelationshipClassCR relationshipClass,
ECInstanceKeyCR source,
ECInstanceKeyCR target,
SyncStatus syncStatus,
uint64_t changeNumber
)
    {
    if (!m_objectInfoManager->ReadInfo(source).IsInCache() ||
        !m_objectInfoManager->ReadInfo(target).IsInCache())
        {
        return ECInstanceKey();
        }

    if (m_dbAdapter->FindRelationship(&relationshipClass, source, target).IsValid())
        {
        BeAssert(false && "Such relationship already exists");
        return ECInstanceKey();
        }

    RelationshipInfo info = m_relationshipInfoManager->ReadInfo(relationshipClass, source, target);
    if (info.IsInCache())
        {
        BeAssert(false && "Such cached relationship already exists");
        return ECInstanceKey();
        }

    ECInstanceKey relationship = m_hierarchyManager->RelateInstances(source, target, &relationshipClass);
    if (!relationship.IsValid())
        {
        return ECInstanceKey();
        }

    if (0 == changeNumber)
        {
        if (SUCCESS != m_changeInfoManager->SetupChangeNumber(info))
            {
            return ECInstanceKey();
            }
        }
    else
        {
        info.SetChangeNumber(changeNumber);
        }

    info.SetRelationshipInstanceId(relationship.GetECInstanceId());
    info.SetRemoteId(CreateRemoteId());
    info.SetChangeStatus(ChangeStatus::Created);
    info.SetSyncStatus(syncStatus);

    if (SUCCESS != m_relationshipInfoManager->SaveInfo(info))
        {
        return ECInstanceKey();
        }

    return info.GetRelationshipKey();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::DeleteRelationship
(
ECInstanceKeyCR relationshipKey,
SyncStatus syncStatus
)
    {
    RelationshipInfo info = m_relationshipInfoManager->FindInfo(relationshipKey);
    if (!info.IsInCache())
        {
        BeAssert(false && "Relationship does not exist in cache");
        return ERROR;
        }

    if (info.GetChangeStatus() != ChangeStatus::NoChange && IsSyncActive())
        {
        BeAssert(false && "Cannot delete relationship while syncing");
        return ERROR;
        }

    if (info.GetChangeStatus() == ChangeStatus::Deleted)
        {
        BeAssert(false && "Relationship is already deleted");
        return ERROR;
        }

    if (info.GetChangeStatus() == ChangeStatus::Created)
        {
        if (SUCCESS != m_hierarchyManager->DeleteRelationship(info.GetRelationshipKey()))
            {
            return ERROR;
            }
        }
    else
        {
        if (SUCCESS != m_relationshipInfoManager->DeleteRelationshipLeavingInfo(info))
            {
            return ERROR;
            }

        if (SUCCESS != m_changeInfoManager->SetupChangeNumber(info))
            {
            return ERROR;
            }

        info.SetChangeStatus(ChangeStatus::Deleted);
        info.SetSyncStatus(syncStatus);

        if (SUCCESS != m_relationshipInfoManager->SaveInfo(info))
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Benjamin.Brown  01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::SetSyncStatus(ECInstanceKeyCR instanceKey, SyncStatus syncStatus)
    {
    bool updated = false;

    ObjectInfo objectInfo = m_objectInfoManager->ReadInfo(instanceKey);
    if (objectInfo.IsInCache())
        {
        if (objectInfo.GetChangeStatus() != ChangeStatus::NoChange)
            {
            objectInfo.SetSyncStatus(syncStatus);
            if (SUCCESS != m_objectInfoManager->UpdateInfo(objectInfo))
                {
                return ERROR;
                }
            updated = true;
            }
        }

    FileInfo fileInfo = m_fileInfoManager->ReadInfo(instanceKey);
    if (fileInfo.IsInCache())
        {
        if (fileInfo.GetChangeStatus() != ChangeStatus::NoChange)
            {
            fileInfo.SetSyncStatus(syncStatus);
            if (SUCCESS != m_fileInfoManager->SaveInfo(fileInfo))
                {
                return ERROR;
                }
            updated = true;
            }
        }

    RelationshipInfo relationshipInfo = m_relationshipInfoManager->FindInfo(instanceKey);
    if (relationshipInfo.IsInCache())
        {
        if (relationshipInfo.GetChangeStatus() != ChangeStatus::NoChange)
            {
            relationshipInfo.SetSyncStatus(syncStatus);
            if (SUCCESS != m_relationshipInfoManager->SaveInfo(relationshipInfo))
                {
                return ERROR;
                }
            updated = true;
            }
        }

    if (!updated)
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChangeManager::HasChanges()
    {
    return m_changeInfoManager->HasChanges();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeManager::GetChanges(Changes& changesOut, bool onlyReadyToSync)
    {
    return m_changeInfoManager->GetChanges(changesOut, onlyReadyToSync);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeManager::GetChanges(ECInstanceKeyCR instanceKey, Changes& changesOut)
    {
    auto objectChange = GetObjectChange(instanceKey);
    if (ChangeStatus::NoChange != objectChange.GetChangeStatus())
        {
        changesOut.AddChange(objectChange);
        }

    auto relationshipChange = GetRelationshipChange(instanceKey);
    if (ChangeStatus::NoChange != relationshipChange.GetChangeStatus())
        {
        changesOut.AddChange(relationshipChange);
        }

    auto fileChange = GetFileChange(instanceKey);
    if (ChangeStatus::NoChange != fileChange.GetChangeStatus())
        {
        changesOut.AddChange(fileChange);
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeManager::GetCreatedRelationships(ECInstanceKeyCR endInstancekey, bvector<RelationshipChange>& changesOut)
    {
    ECInstanceKeyMultiMap relationships;

    ECInstanceFinder& finder = m_dbAdapter->GetECInstanceFinder();
    if (SUCCESS != finder.FindRelatedInstances(nullptr, &relationships, endInstancekey, ECInstanceFinder::RelatedDirection_All))
        {
        return ERROR;
        }

    for (auto& pair : relationships)
        {
        ECInstanceKey relationship(pair.first, pair.second);
        auto relationshipChange = GetRelationshipChange(relationship);
        if (ChangeStatus::Created == relationshipChange.GetChangeStatus())
            {
            changesOut.push_back(relationshipChange);
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::ObjectChange ChangeManager::GetObjectChange(ECInstanceKeyCR instanceKey)
    {
    return m_changeInfoManager->GetObjectChange(instanceKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::RelationshipChange ChangeManager::GetRelationshipChange(ECInstanceKeyCR instanceKey)
    {
    return m_changeInfoManager->GetRelationshipChange(instanceKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::FileChange ChangeManager::GetFileChange(ECInstanceKeyCR instanceKey)
    {
    return m_changeInfoManager->GetFileChange(instanceKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::ChangeStatus ChangeManager::GetObjectChangeStatus(ECInstanceKeyCR instance)
    {
    return m_changeInfoManager->GetObjectChangeStatus(instance);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ChangeManager::SyncStatus ChangeManager::GetObjectSyncStatus(ECInstanceKeyCR instance)
    {
    return m_changeInfoManager->GetObjectSyncStatus(instance);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::CommitCreationChanges(const std::map<ECInstanceKey, Utf8String>& newRemoteIds)
    {
    for (auto& pair : newRemoteIds)
        {
        ECClassCP ecClass = m_dbAdapter->GetECClass(pair.first);
        if (ecClass == nullptr)
            {
            return ERROR;
            }

        if (ecClass->GetRelationshipClassCP() == nullptr)
            {
            if (SUCCESS != CommitObjectCreation(pair.first, pair.second))
                {
                return ERROR;
                }
            }
        else
            {
            if (SUCCESS != CommitRelationshipCreation(pair.first, pair.second))
                {
                return ERROR;
                }
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::CommitObjectCreation(ECInstanceKeyCR instanceKey, Utf8StringCR newRemoteId)
    {
    if (newRemoteId.empty())
        {
        return ERROR;
        }

    ObjectInfo info = m_objectInfoManager->ReadInfo(instanceKey);
    if (info.GetChangeStatus() != ChangeStatus::Created)
        {
        return ERROR;
        }

    ECClassCP ecClass = m_dbAdapter->GetECClass(instanceKey);
    ECInstanceKey oldVersionInstance = m_objectInfoManager->FindCachedInstance(ecClass, newRemoteId);
    if (oldVersionInstance.IsValid() && oldVersionInstance != info.GetCachedInstanceKey())
        {
        // Instance with same remoteId already exists in cache.
        // This usually happens when creating new version of object and referances to latest version need to be preserved.
        // Old instance is invalidated as it is replaced by new one.

        if (SUCCESS != m_responseManager->InvalidateResponsesContainingInstance(oldVersionInstance))
            {
            return ERROR;
            }
        if (SUCCESS != m_rootManager->CopyRootRelationships(oldVersionInstance, info.GetCachedInstanceKey()))
            {
            return ERROR;
            }
        if (SUCCESS != m_hierarchyManager->DeleteInstance(oldVersionInstance))
            {
            return ERROR;
            }
        }

    info.SetRemoteId(newRemoteId);
    info.ClearChangeInfo();

    if (SUCCESS != m_objectInfoManager->UpdateInfo(info))
        {
        return ERROR;
        }

    FileInfo fileInfo = m_fileInfoManager->ReadInfo(info.GetObjectId());
    if (fileInfo.GetChangeStatus() != ChangeStatus::NoChange)
        {
        fileInfo.ClearChangeInfo();
        if (SUCCESS != m_fileInfoManager->SaveInfo(fileInfo))
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::CommitRelationshipCreation(ECInstanceKeyCR instanceKey, Utf8StringCR newRemoteId)
    {
    RelationshipInfo info = m_relationshipInfoManager->FindInfo(instanceKey);
    if (info.GetChangeStatus() != ChangeStatus::Created)
        {
        return ERROR;
        }

    if (newRemoteId.empty())
        {
        if (SUCCESS != m_hierarchyManager->DeleteRelationship(info.GetRelationshipKey()))
            {
            return ERROR;
            }
        return SUCCESS;
        }

    info.SetRemoteId(newRemoteId);
    info.ClearChangeInfo();

    if (SUCCESS != m_relationshipInfoManager->SaveInfo(info))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::CommitObjectChanges(ECInstanceKeyCR instanceKey)
    {
    ECClassCP ecClass = m_dbAdapter->GetECClass(instanceKey);

    if (nullptr != ecClass &&
        nullptr != ecClass->GetRelationshipClassCP())
        {
        if (SUCCESS != CommitRelationshipChange(instanceKey))
            {
            return ERROR;
            }
        }
    else
        {
        if (SUCCESS != CommitObjectChange(instanceKey))
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::CommitObjectChange(ECInstanceKeyCR instanceKey)
    {
    ObjectInfo info = m_objectInfoManager->ReadInfo(instanceKey);
    switch (info.GetChangeStatus())
        {
            case ChangeStatus::Modified:
                info.ClearChangeInfo();
                if (SUCCESS != m_objectInfoManager->UpdateInfo(info))
                    {
                    return ERROR;
                    }
                break;
            case ChangeStatus::Deleted:
                if (SUCCESS != m_hierarchyManager->DeleteInstance(info.GetInfoKey()))
                    {
                    return ERROR;
                    }
                break;
            case ChangeStatus::NoChange:
            case ChangeStatus::Created:
            default:
                BeAssert(false && "Change status is unsupported for commit");
                return ERROR;
                break;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::CommitRelationshipChange(ECInstanceKeyCR instanceKey)
    {
    RelationshipInfo info = m_relationshipInfoManager->FindInfo(instanceKey);
    switch (info.GetChangeStatus())
        {
            case ChangeStatus::Deleted:
                if (SUCCESS != m_hierarchyManager->DeleteInstance(info.GetInfoKey()))
                    {
                    return ERROR;
                    }
                break;
            case ChangeStatus::Modified:
            case ChangeStatus::NoChange:
            case ChangeStatus::Created:
            default:
                BeAssert(false && "Change status is unsupported for commit");
                return ERROR;
                break;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::CommitFileChanges(ECInstanceKeyCR instanceKey)
    {
    FileInfo info = m_fileInfoManager->ReadInfo(instanceKey);
    switch (info.GetChangeStatus())
        {
            case ChangeStatus::Modified:
                info.ClearChangeInfo();
                if (SUCCESS != m_fileInfoManager->SaveInfo(info))
                    {
                    return ERROR;
                    }
                if (SUCCESS != m_fileManager->SetFileCacheLocation(instanceKey, FileCache::Temporary))
                    {
                    return ERROR;
                    }
                break;
            case ChangeStatus::NoChange:
            case ChangeStatus::Created:
            case ChangeStatus::Deleted:
            default:
                BeAssert(false && "Change status is unsupported for commit");
                return ERROR;
                break;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::UpdateCreatedInstance
(
ObjectIdCR oldObjectId,
WSObjectsResponseCR instanceResponse,
bmap<ECInstanceKey, ECInstanceKey>& changedInstanceKeysOut
)
    {
    ObjectInfo oldInfo = m_objectInfoManager->ReadInfo(oldObjectId);
    if (!oldInfo.IsInCache())
        {
        return ERROR;
        }

    auto instances = instanceResponse.GetInstances();
    InstanceCacheHelper::CachedInstances cachedInstances;
    if (SUCCESS != m_instanceHelper->CacheInstances(instances, cachedInstances))
        {
        return ERROR;
        }

    if (cachedInstances.GetCachedInstanceObjectIds().size() != 1)
        {
        return ERROR;
        }

    ObjectId newObjectId = *cachedInstances.GetCachedInstanceObjectIds().begin();
    ObjectInfo newInfo = m_objectInfoManager->ReadInfo(newObjectId);
    if (!newInfo.IsInCache())
        {
        return ERROR;
        }

    ECInstanceKey oldInstanceKey = oldInfo.GetCachedInstanceKey();
    ECInstanceKey newInstanceKey = newInfo.GetCachedInstanceKey();

    if (SUCCESS != m_responseManager->InvalidateResponsesContainingInstance(oldInstanceKey))
        {
        return ERROR;
        }

    if (oldObjectId.remoteId != newObjectId.remoteId)
        {
        return ERROR;
        }

    if (oldObjectId == newObjectId)
        {
        return SUCCESS;
        }

    changedInstanceKeysOut[oldInstanceKey] = newInstanceKey;

    bvector<IChangeManager::RelationshipChange> changes;
    if (SUCCESS != GetCreatedRelationships(oldInstanceKey, changes))
        {
        return ERROR;
        }

    if (SUCCESS != m_rootManager->CopyRootRelationships(oldInstanceKey, newInstanceKey))
        {
        return ERROR;
        }

    if (SUCCESS != m_hierarchyManager->DeleteInstance(oldInstanceKey))
        {
        return ERROR;
        }

    for (auto& change : changes)
        {
        ECInstanceKey relKey;
        auto relClass = m_dbAdapter->GetECRelationshipClass(change.GetInstanceKey());
        if (change.GetSourceKey() == oldInstanceKey)
            {
            relKey = CreateRelationship(*relClass, newInstanceKey, change.GetTargetKey(), change.GetSyncStatus(), change.GetChangeNumber());
            }
        else
            {
            relKey = CreateRelationship(*relClass, change.GetSourceKey(), newInstanceKey, change.GetSyncStatus(), change.GetChangeNumber());
            }
        if (!relKey.IsValid())
            {
            return ERROR;
            }
        changedInstanceKeysOut[change.GetInstanceKey()] = relKey;
        }
    return SUCCESS;
    }