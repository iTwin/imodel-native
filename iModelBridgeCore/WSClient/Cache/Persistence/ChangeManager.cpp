/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/ChangeManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Persistence/ChangeManager.h>

#include <WebServices/Cache/Util/ECDbHelper.h>

#include "Changes/ChangeInfoManager.h"
#include "Core/CacheSchema.h"
#include "Core/SchemaManager.h"
#include "Files/FileStorage.h"
#include "Files/FileInfoManager.h"
#include "Hierarchy/HierarchyManager.h"
#include "Hierarchy/RootManager.h"
#include "Instances/InstanceCacheHelper.h"
#include "Instances/ObjectInfoManager.h"
#include "Instances/RelationshipInfoManager.h"
#include "Responses/CachedResponseManager.h"
#include <WebServices/Cache/Util/JsonUtil.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

Utf8CP ChangeManager::LocalInstanceIdPrefix = "LocalInstance-";

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ChangeManager::ChangeManager
(
ECDbAdapter& dbAdapter,
InstanceCacheHelper& instanceCacheHelper,
HierarchyManager& hierarchyManager,
CachedResponseManager& responseManager,
ObjectInfoManager& objectInfoManager,
RelationshipInfoManager& relationshipInfoManager,
FileInfoManager& fileInfoManager,
ChangeInfoManager& changeInfoManager,
FileStorage& fileStorage,
RootManager& rootManager
) :
m_dbAdapter(dbAdapter),
m_instanceCacheHelper(instanceCacheHelper),
m_hierarchyManager(hierarchyManager),
m_responseManager(responseManager),
m_objectInfoManager(objectInfoManager),
m_relationshipInfoManager(relationshipInfoManager),
m_fileInfoManager(fileInfoManager),
m_changeInfoManager(changeInfoManager),
m_fileStorage(fileStorage),
m_rootManager(rootManager),
m_isSyncActive(false)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Vytenis.Navalinskas  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ChangeManager::CreateRemoteId()
    {
    return LocalInstanceIdPrefix + BeGuid(true).ToString();
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeManager::SetupNewRevision(struct ChangeInfo& info)
    {
    info.IncrementRevision();
    return m_changeInfoManager.SetupChangeNumber(info);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP ChangeManager::GetLegacyParentRelationshipClass(ECClassId parentClassId, ECClassId childClassId, bool createIfNotExists)
    {
    ECClassCP parentClass = m_dbAdapter.GetECClass(parentClassId);
    ECClassCP childClass = m_dbAdapter.GetECClass(childClassId);

    if (nullptr == parentClass || nullptr == childClass || !parentClass->IsEntityClass() || !childClass->IsEntityClass())
        return nullptr;

    Utf8String schemaName;
    schemaName += SCHEMA_CacheLegacySupport_PREFIX;
    schemaName += "_";
    schemaName += parentClass->GetSchema().GetName();
    schemaName += "_";
    schemaName += parentClass->GetName();
    schemaName += "_";
    schemaName += childClass->GetSchema().GetName();
    schemaName += "_";
    schemaName += childClass->GetName();

    Utf8String relClassName = CLASS_CacheLegacySupport_ParentToChildRelationship;

    auto existingRelClass = m_dbAdapter.GetECRelationshipClass(schemaName.c_str(), relClassName.c_str());
    if (nullptr != existingRelClass || !createIfNotExists)
        {
        return existingRelClass;
        }

    ECSchemaPtr ecSchema;
    ECSchema::CreateSchema(ecSchema, schemaName, schemaName, 1, 0, 0);
    if (ecSchema.IsNull())
        return nullptr;

    if (ECObjectsStatus::Success != ecSchema->AddReferencedSchema((ECSchemaR) parentClass->GetSchema()))
        return nullptr;

    auto status = ecSchema->AddReferencedSchema((ECSchemaR) childClass->GetSchema());
    if (ECObjectsStatus::Success != status && ECObjectsStatus::NamedItemAlreadyExists != status)
        return nullptr;

    ECRelationshipClassP relClass = nullptr;
    ecSchema->CreateRelationshipClass(relClass, relClassName);
    if (nullptr == relClass)
        return nullptr;

    if (ECObjectsStatus::Success != relClass->GetSource().AddClass(*parentClass->GetEntityClassCP()) ||
        ECObjectsStatus::Success != relClass->GetTarget().AddClass(*childClass->GetEntityClassCP()))
        return nullptr;

    ObservableECDb& db = m_dbAdapter.GetECDb();

    if (SUCCESS != SchemaManager(db).ImportSchemas({ecSchema}))
        return nullptr;

    return db.Schemas().GetECClass(schemaName.c_str(), relClassName.c_str())->GetRelationshipClassCP();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECInstanceKey ChangeManager::CreateObject(ECClassCR ecClass, JsonValueCR properties, SyncStatus syncStatus)
    {
    ObjectId newObjectId(ecClass, CreateRemoteId());
    ObjectInfo info = m_objectInfoManager.ReadInfo(newObjectId);
    if (info.IsInCache())
        {
        BeAssert(false && "Object with same ObjectId already exists");
        return ECInstanceKey();
        }

    if (SUCCESS != SetupNewRevision(info))
        {
        return ECInstanceKey();
        }

    info.SetObjectCacheDate(DateTime::GetCurrentTimeUtc());
    info.SetRemoteId(newObjectId.remoteId);
    info.SetChangeStatus(ChangeStatus::Created);
    info.SetSyncStatus(syncStatus);
    info.SetIsLocal(true);

    rapidjson::Document propertiesRapidJson;
    JsonUtil::ToRapidJson(properties, propertiesRapidJson);

    if (SUCCESS != m_instanceCacheHelper.CacheInstance(info, propertiesRapidJson))
        {
        return ECInstanceKey();
        }

    return info.GetInstanceKey();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeManager::ModifyObject(ECInstanceKeyCR instanceKey, JsonValueCR properties, SyncStatus syncStatus)
    {
    ObjectInfo info = m_objectInfoManager.ReadInfo(instanceKey);
    if (!info.IsInCache())
        {
        BeAssert(false && "Object is not it cache");
        return ERROR;
        }

    if (info.GetChangeStatus() == ChangeStatus::Deleted)
        {
        BeAssert(false && "Cannot modify object that is deleted");
        return ERROR;
        }

    Json::Value instanceJson;
    if (SUCCESS != m_dbAdapter.GetJsonInstance(instanceJson, instanceKey))
        return ERROR;

    if (info.GetChangeStatus() == ChangeStatus::NoChange)
        {
        if (SUCCESS != m_changeInfoManager.SaveBackupInstance(info, instanceJson))
            return ERROR;
        }

    if (SUCCESS != SetupNewRevision(info))
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
    JsonUtil::ToRapidJson(properties, propertiesRapidJson);

    rapidjson::Document instanceRapidJson;
    JsonUtil::ToRapidJson(instanceJson, instanceRapidJson);

    JsonUtil::DeepCopy(propertiesRapidJson, instanceRapidJson);

    if (SUCCESS != m_instanceCacheHelper.UpdateExistingInstanceData(info, instanceRapidJson))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeManager::RevertModifiedObject(ECInstanceKeyCR instance)
    {
    ObjectInfo info = m_objectInfoManager.ReadInfo(instance);
    if (info.GetChangeStatus() != ChangeStatus::Modified)
        {
        BeAssert(false && "Cannot revert non-modified instance");
        return ERROR;
        }

    if (IsSyncActive())
        {
        BeAssert(false && "Change reverting while syncing is not implemented");
        return ERROR;
        }

    info.ClearChangeInfo();

    rapidjson::Document instanceJson;
    if (SUCCESS != m_changeInfoManager.ReadBackupInstance(info, instanceJson) ||
        SUCCESS != m_instanceCacheHelper.UpdateExistingInstanceData(info, instanceJson))
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
    ObjectInfo info = m_objectInfoManager.ReadInfo(instanceKey);
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

    // Reset change number
    info.SetChangeNumber(0);
    if (SUCCESS != SetupNewRevision(info))
        {
        return ERROR;
        }

    info.SetChangeStatus(ChangeStatus::Deleted);
    info.SetSyncStatus(syncStatus);

    if (SUCCESS != m_objectInfoManager.DeleteInstanceLeavingInfo(info))
        {
        return ERROR;
        }

    if (SUCCESS != m_objectInfoManager.UpdateInfo(info))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeManager::MarkFileAsModified(FileInfo& info, SyncStatus syncStatus)
    {
    if (SUCCESS != SetupNewRevision(info))
        return ERROR;

    info.SetFileCacheDate(DateTime::GetCurrentTimeUtc());
    info.SetChangeStatus(ChangeStatus::Modified);
    info.SetSyncStatus(syncStatus);

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeManager::DetectFileModification(ECInstanceKeyCR instanceKey, bool& outIsModified)
    {
    ObjectInfo objInfo = m_objectInfoManager.ReadInfo(instanceKey);
    if (!objInfo.IsInCache())
        return ERROR;

    FileInfo info = m_fileInfoManager.ReadInfo(objInfo.GetCachedInstanceKey());
    if (info.GetChangeStatus() == ChangeStatus::Modified)
        return SUCCESS;

    time_t modifiedSeconds;
    if (BeFileNameStatus::Success != info.GetFilePath().GetFileTime(nullptr, nullptr, &modifiedSeconds))
        return ERROR;

    int64_t modifiedMs = modifiedSeconds * 1000;

    int64_t updatedMs;
    if (SUCCESS != info.GetFileUpdateDate().ToUnixMilliseconds(updatedMs))
        return ERROR;

    outIsModified = modifiedMs > updatedMs;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeManager::ModifyFile(ECInstanceKeyCR instanceKey, BeFileNameCR filePath, bool copyFile, SyncStatus syncStatus)
    {
    if (!filePath.DoesPathExist())
        return ERROR;
        
    ObjectInfo objInfo = m_objectInfoManager.ReadInfo(instanceKey);
    if (!objInfo.IsInCache())
        return ERROR;
        
    FileInfo info = m_fileInfoManager.ReadInfo(objInfo.GetCachedInstanceKey());
    if (info.GetChangeStatus() != ChangeStatus::NoChange && IsSyncActive())
        {
        BeAssert(false && "Cannot change modified file while syncing");
        return ERROR;
        }
    
    if (info.GetFilePath() != filePath)
        {
        FileCache location = info.GetLocation(FileCache::Persistent);
        if (SUCCESS != m_fileStorage.CacheFile(info, filePath, nullptr, location, copyFile))
            return ERROR;
        }

    if (SUCCESS != MarkFileAsModified(info, syncStatus))
        return ERROR;

    if (SUCCESS != m_fileInfoManager.SaveInfo(info))
        return ERROR;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::ModifyFileName(ECInstanceKeyCR instanceKey, Utf8StringCR newFileName)
    {
    ObjectInfo objInfo = m_objectInfoManager.ReadInfo(instanceKey);
    if (!objInfo.IsInCache())
        {
        return ERROR;
        }

    FileInfo info = m_fileInfoManager.ReadInfo(objInfo.GetCachedInstanceKey());

    if (IsSyncActive())
        {
        BeAssert(false && "Cannot rename file while syncing");
        return ERROR;
        }

    if (SUCCESS != m_fileStorage.RenameCachedFile(info, newFileName) ||
        SUCCESS != m_fileInfoManager.SaveInfo(info))
        {
        return ERROR;
        }

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
    if (!m_objectInfoManager.ReadInfo(source).IsInCache() ||
        !m_objectInfoManager.ReadInfo(target).IsInCache())
        {
        return ECInstanceKey();
        }

    if (m_dbAdapter.FindRelationship(&relationshipClass, source, target).IsValid())
        {
        BeAssert(false && "Such relationship already exists");
        return ECInstanceKey();
        }

    RelationshipInfo info = m_relationshipInfoManager.ReadInfo(relationshipClass, source, target);
    if (info.IsInCache())
        {
        BeAssert(false && "Such cached relationship already exists");
        return ECInstanceKey();
        }

    ECInstanceKey relationship = m_hierarchyManager.RelateInstances(source, target, &relationshipClass);
    if (!relationship.IsValid())
        {
        return ECInstanceKey();
        }

    if (0 == changeNumber)
        {
        if (SUCCESS != SetupNewRevision(info))
            {
            return ECInstanceKey();
            }
        }
    else
        {
        info.SetChangeNumber(changeNumber);
        }

    info.SetInstanceId(relationship.GetECInstanceId());
    info.SetRemoteId(CreateRemoteId());
    info.SetChangeStatus(ChangeStatus::Created);
    info.SetSyncStatus(syncStatus);
    info.SetIsLocal(true);

    if (SUCCESS != m_relationshipInfoManager.SaveInfo(info))
        {
        return ECInstanceKey();
        }

    return info.GetInstanceKey();
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
    RelationshipInfo info = m_relationshipInfoManager.FindInfo(relationshipKey);
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

    if (SUCCESS != m_relationshipInfoManager.DeleteRelationshipLeavingInfo(info))
        {
        return ERROR;
        }

    if (SUCCESS != SetupNewRevision(info))
        {
        return ERROR;
        }

    info.SetChangeStatus(ChangeStatus::Deleted);
    info.SetSyncStatus(syncStatus);

    if (SUCCESS != m_relationshipInfoManager.SaveInfo(info))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Benjamin.Brown  01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::SetSyncStatus(ECInstanceKeyCR instanceKey, SyncStatus syncStatus)
    {
    bool updated = false;

    ObjectInfo objectInfo = m_objectInfoManager.ReadInfo(instanceKey);
    if (objectInfo.IsInCache())
        {
        if (objectInfo.GetChangeStatus() != ChangeStatus::NoChange)
            {
            objectInfo.SetSyncStatus(syncStatus);
            if (SUCCESS != m_objectInfoManager.UpdateInfo(objectInfo))
                {
                return ERROR;
                }
            updated = true;
            }
        }

    FileInfo fileInfo = m_fileInfoManager.ReadInfo(objectInfo.GetCachedInstanceKey());
    if (fileInfo.IsInCache())
        {
        if (fileInfo.GetChangeStatus() != ChangeStatus::NoChange)
            {
            fileInfo.SetSyncStatus(syncStatus);
            if (SUCCESS != m_fileInfoManager.SaveInfo(fileInfo))
                {
                return ERROR;
                }
            updated = true;
            }
        }

    RelationshipInfo relationshipInfo = m_relationshipInfoManager.FindInfo(instanceKey);
    if (relationshipInfo.IsInCache())
        {
        if (relationshipInfo.GetChangeStatus() != ChangeStatus::NoChange)
            {
            relationshipInfo.SetSyncStatus(syncStatus);
            if (SUCCESS != m_relationshipInfoManager.SaveInfo(relationshipInfo))
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeManager::AddCreatedInstanceToResponse(CachedResponseKeyCR cachedResponseKey, ECInstanceKeyCR instanceKey)
    {
    ResponseKey responseKey = m_responseManager.ConvertResponseKey(cachedResponseKey);
    if (!responseKey.IsValid())
        return ERROR;

    ObjectInfo info = m_objectInfoManager.ReadInfo(instanceKey);
    if (ChangeStatus::Created != info.GetChangeStatus())
        return ERROR;

    return m_responseManager.AddAdditionalInstance(responseKey, info.GetInfoKey());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeManager::RemoveCreatedInstanceFromResponse(CachedResponseKeyCR cachedResponseKey, ECInstanceKeyCR instanceKey)
    {
    ResponseKey responseKey = m_responseManager.ConvertResponseKey(cachedResponseKey);
    if (!responseKey.IsValid())
        return ERROR;

    ObjectInfo info = m_objectInfoManager.ReadInfo(instanceKey);
    if (ChangeStatus::Created != info.GetChangeStatus())
        return ERROR;

    return m_responseManager.RemoveAdditionalInstance(responseKey, info.GetInfoKey());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChangeManager::HasChanges()
    {
    return m_changeInfoManager.HasChanges();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeManager::GetChanges(Changes& changesOut, bool onlyReadyToSync)
    {
    return m_changeInfoManager.GetChanges(changesOut, onlyReadyToSync);
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

    ECInstanceFinder& finder = m_dbAdapter.GetECInstanceFinder();
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
    return m_changeInfoManager.GetObjectChange(instanceKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::RelationshipChange ChangeManager::GetRelationshipChange(ECInstanceKeyCR instanceKey)
    {
    return m_changeInfoManager.GetRelationshipChange(instanceKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::FileChange ChangeManager::GetFileChange(ECInstanceKeyCR instanceKey)
    {
    return m_changeInfoManager.GetFileChange(instanceKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::ChangeStatus ChangeManager::GetObjectChangeStatus(ECInstanceKeyCR instance)
    {
    return m_changeInfoManager.GetObjectChangeStatus(instance);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ChangeManager::SyncStatus ChangeManager::GetObjectSyncStatus(ECInstanceKeyCR instance)
    {
    return m_changeInfoManager.GetObjectSyncStatus(instance);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ChangeManager::InstanceRevisionPtr ChangeManager::ReadInstanceRevision(ECInstanceKeyCR instanceKey)
    {
    ECClassCP ecClass = m_dbAdapter.GetECClass(instanceKey);
    if (nullptr == ecClass)
        {
        return std::make_shared<InstanceRevision>();
        }

    if (nullptr != ecClass->GetRelationshipClassCP())
        {
        return ReadRelationshipRevision(instanceKey);
        }
    else
        {
        return ReadObjectRevision(instanceKey);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ChangeManager::InstanceRevisionPtr ChangeManager::ReadObjectRevision(ECInstanceKeyCR instanceKey)
    {
    auto revision = std::make_shared<InstanceRevision>();
    revision->SetInstanceKey(instanceKey);

    auto info = m_objectInfoManager.ReadInfo(instanceKey);
    revision->SetObjectId(info.GetObjectId());

    if (info.GetChangeStatus() != ChangeStatus::NoChange)
        {
        SetupRevisionChanges(info, *revision);
        revision->SetChangedProperties(ReadChangeProperties(info.GetChangeStatus(), instanceKey));
        }

    return revision;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ChangeManager::InstanceRevisionPtr ChangeManager::ReadRelationshipRevision(ECInstanceKeyCR instanceKey)
    {
    auto revision = std::make_shared<InstanceRevision>();
    revision->SetInstanceKey(instanceKey);

    auto info = m_relationshipInfoManager.FindInfo(instanceKey);
    revision->SetObjectId(info.GetObjectId());

    if (info.GetChangeStatus() != ChangeStatus::NoChange)
        {
        SetupRevisionChanges(info, *revision);
        }

    return revision;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ChangeManager::FileRevisionPtr ChangeManager::ReadFileRevision(ECInstanceKeyCR instanceKey)
    {
    ObjectInfo objInfo = m_objectInfoManager.ReadInfo(instanceKey);

    auto revision = std::make_shared<FileRevision>();
    revision->SetInstanceKey(instanceKey);
    revision->SetObjectId(objInfo.GetObjectId());

    FileInfo info = m_fileInfoManager.ReadInfo(objInfo.GetCachedInstanceKey());
    if (info.GetChangeStatus() != ChangeStatus::NoChange)
        {
        SetupRevisionChanges(info, *revision);
        revision->SetFilePath(info.GetFilePath());
        }

    return revision;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ChangeManager::SetupRevisionChanges(ChangeInfoCR info, Revision& revision)
    {
    revision.SetChangeStatus(info.GetChangeStatus());
    revision.SetSyncStatus(info.GetSyncStatus());
    revision.SetChangeNumber(info.GetChangeNumber());
    revision.SetRevisionNumber(info.GetRevision());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeManager::ReadModifiedProperties(ECInstanceKeyCR instance, JsonValueR propertiesOut)
    {
    auto info = m_objectInfoManager.ReadInfo(instance);
    if (info.GetChangeStatus() != ChangeStatus::Modified)
        {
        return ERROR;
        }

    rapidjson::Document changesJson, t1, t2;
    if (SUCCESS != m_changeInfoManager.ReadInstanceChanges(info, changesJson, t1, t2))
        {
        return ERROR;
        }

    JsonUtil::ToJsonValue(changesJson, propertiesOut);
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::CommitLocalDeletions()
    {
    return m_changeInfoManager.RemoveLocalDeletedInfos();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::CommitInstanceRevision(InstanceRevisionCR revision)
    {
    ECClassCP ecClass = m_dbAdapter.GetECClass(revision.GetInstanceKey());
    if (nullptr == ecClass)
        {
        return ERROR;
        }

    if (nullptr != ecClass->GetRelationshipClassCP())
        {
        return CommitRelationshipChange(revision);
        }
    else
        {
        return CommitInstanceChange(revision);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::CommitInstanceChange(InstanceRevisionCR revision)
    {
    ObjectInfo info = m_objectInfoManager.ReadInfo(revision.GetInstanceKey());
    if (info.GetChangeStatus() == ChangeStatus::NoChange)
        {
        BeAssert(false && "Nothing to commit");
        return ERROR;
        }

    bool currentRevision = info.GetRevision() == revision.GetRevisionNumber();
    if (currentRevision && revision.GetChangeStatus() != info.GetChangeStatus())
        {
        BeAssert(false && "Revision state invalid");
        return ERROR;
        }

    // Deleted
    if (revision.GetChangeStatus() == ChangeStatus::Deleted && currentRevision)
        {
        return m_hierarchyManager.DeleteInstance(info.GetInfoKey());
        }

    // Modified
    if (revision.GetChangeStatus() == ChangeStatus::Modified)
        {
        if (currentRevision)
            {
            info.ClearChangeInfo();
            if (SUCCESS != m_objectInfoManager.UpdateInfo(info) ||
                SUCCESS != m_changeInfoManager.DeleteBackupInstance(info))
                {
                return ERROR;
                }
            return SUCCESS;
            }
        else if (info.GetChangeStatus() == ChangeStatus::Modified)
            {
            return m_changeInfoManager.ApplyChangesToBackup(info, *revision.GetChangedProperties());
            }
        else if (info.GetChangeStatus() == ChangeStatus::Deleted)
            {
            return SUCCESS;
            }
        return ERROR;
        }

    // Created
    if (revision.GetChangeStatus() != ChangeStatus::Created)
        {
        BeAssert(false && "Change status is unsupported for commit");
        return ERROR;
        }

    if (SUCCESS != m_responseManager.RemoveAdditionalInstance(info.GetInfoKey()))
        return ERROR;

    ObjectId newId = revision.GetObjectId();
    if (newId.remoteId.empty() || newId.remoteId == info.GetObjectId().remoteId)
        {
        BeAssert(false && "Set remote id with new value");
        return ERROR;
        }

    CachedInstanceKey oldVersionInstance = m_objectInfoManager.ReadCachedInstanceKey(newId);
    if (oldVersionInstance.IsValid() && oldVersionInstance != info.GetCachedInstanceKey())
        {
        // Instance with same remoteId already exists in cache.
        // This usually happens when creating new version of object and referances to latest version need to be preserved.
        // Old instance is invalidated as it is replaced by new one.

        if (SUCCESS != m_responseManager.InvalidateResponsePagesContainingInstance(oldVersionInstance) ||
            SUCCESS != m_rootManager.CopyRootRelationships(oldVersionInstance.GetInfoKey(), info.GetInfoKey()) ||
            SUCCESS != m_hierarchyManager.DeleteInstance(oldVersionInstance))
            {
            return ERROR;
            }
        }

    info.SetRemoteId(newId.remoteId);
    info.SetIsLocal(false);

    if (currentRevision)
        {
        info.ClearChangeInfo();
        }
    else if (info.GetChangeStatus() == ChangeStatus::Created)
        {
        // Instance was modified before commiting creation revision
        if (SUCCESS != m_changeInfoManager.ApplyChangesToInstanceAndBackupIt(info, *revision.GetChangedProperties()))
            {
            return ERROR;
            }
        info.SetChangeStatus(ChangeStatus::Modified);
        }
    else if (info.GetChangeStatus() == ChangeStatus::Deleted)
        {
        // Leave as deleted
        }

    return m_objectInfoManager.UpdateInfo(info);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::CommitRelationshipChange(InstanceRevisionCR revision)
    {
    RelationshipInfo info = m_relationshipInfoManager.FindInfo(revision.GetInstanceKey());
    if (info.GetChangeStatus() == ChangeStatus::NoChange)
        {
        BeAssert(false && "Nothing to commit");
        return ERROR;
        }

    bool currentRevision = info.GetRevision() == revision.GetRevisionNumber();

    if (revision.GetChangeStatus() == ChangeStatus::Deleted)
        {
        return m_hierarchyManager.DeleteInstance(info.GetInfoKey());
        }

    if (revision.GetChangeStatus() != ChangeStatus::Created)
        {
        BeAssert(false && "Change status is unsupported for commit");
        return ERROR;
        }

    Utf8String newRemoteId = revision.GetObjectId().remoteId;

    info.SetRemoteId(newRemoteId);
    info.SetIsLocal(false);

    if (currentRevision && newRemoteId.empty())
        {
        return m_hierarchyManager.DeleteRelationship(info.GetInstanceKey());
        }
    else if (currentRevision)
        {
        info.ClearChangeInfo();
        }
    else if (info.GetChangeStatus() == ChangeStatus::Deleted)
        {
        // Leave as deleted
        }

    return m_relationshipInfoManager.SaveInfo(info);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::CommitFileRevision(FileRevisionCR revision)
    {
    CachedInstanceKey cachedKey = m_objectInfoManager.ReadCachedInstanceKey(revision.GetInstanceKey());
    FileInfo info = m_fileInfoManager.ReadInfo(cachedKey);
    if (info.GetChangeStatus() == ChangeStatus::NoChange)
        {
        BeAssert(false && "Nothing to commit");
        return ERROR;
        }

    if (info.GetChangeStatus() != ChangeStatus::Modified ||
        revision.GetChangeStatus() != ChangeStatus::Modified)
        {
        BeAssert(false && "Change status is unsupported for commit");
        return ERROR;
        }

    bool currentRevision = info.GetRevision() == revision.GetRevisionNumber();
    if (!currentRevision)
        {
        // Leave as modified
        return SUCCESS;
        }

    info.ClearChangeInfo();
    if (SUCCESS != m_fileInfoManager.SaveInfo(info))
        return ERROR;

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
    ObjectInfo oldInfo = m_objectInfoManager.ReadInfo(oldObjectId);
    if (!oldInfo.IsInCache())
        {
        return ERROR;
        }

    auto instances = instanceResponse.GetInstances();
    InstanceCacheHelper::CachedInstances cachedInstances;
    if (SUCCESS != m_instanceCacheHelper.CacheInstances(instances, cachedInstances))
        {
        return ERROR;
        }

    if (cachedInstances.GetCachedInstanceObjectIds().size() != 1)
        {
        return ERROR;
        }

    ObjectId newObjectId = *cachedInstances.GetCachedInstanceObjectIds().begin();
    ObjectInfo newInfo = m_objectInfoManager.ReadInfo(newObjectId);
    if (!newInfo.IsInCache())
        {
        return ERROR;
        }

    if (SUCCESS != m_responseManager.InvalidateResponsePagesContainingInstance(oldInfo.GetCachedInstanceKey()))
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

    CachedInstanceKey oldKey = oldInfo.GetCachedInstanceKey();
    CachedInstanceKey newKey = newInfo.GetCachedInstanceKey();

    changedInstanceKeysOut[oldKey.GetInstanceKey()] = newKey.GetInstanceKey();

    bvector<IChangeManager::RelationshipChange> changes;
    if (SUCCESS != GetCreatedRelationships(oldKey.GetInstanceKey(), changes))
        {
        return ERROR;
        }

    if (SUCCESS != m_rootManager.CopyRootRelationships(oldInfo.GetInfoKey(), newInfo.GetInfoKey()))
        {
        return ERROR;
        }

    if (SUCCESS != m_hierarchyManager.DeleteInstance(oldKey))
        {
        return ERROR;
        }

    for (auto& change : changes)
        {
        ECInstanceKey relKey;
        auto relClass = m_dbAdapter.GetECRelationshipClass(change.GetInstanceKey());
        if (change.GetSourceKey() == oldKey.GetInstanceKey())
            {
            relKey = CreateRelationship(*relClass, newKey.GetInstanceKey(), change.GetTargetKey(), change.GetSyncStatus(), change.GetChangeNumber());
            }
        else
            {
            relKey = CreateRelationship(*relClass, change.GetSourceKey(), newKey.GetInstanceKey(), change.GetSyncStatus(), change.GetChangeNumber());
            }
        if (!relKey.IsValid())
            {
            return ERROR;
            }
        changedInstanceKeysOut[change.GetInstanceKey()] = relKey;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValuePtr ChangeManager::ReadChangeProperties(ChangeStatus status, ECInstanceKeyCR instance)
    {
    ECClassCP ecClass = m_dbAdapter.GetECClass(instance);
    if (nullptr != ecClass && nullptr != ecClass->GetRelationshipClassCP())
        {
        return nullptr;
        }

    auto properties = std::make_shared<Json::Value>();

    if (ChangeStatus::Created == status)
        {
        if (SUCCESS != ReadObjectPropertiesForCreation(instance, *properties))
            {
            return nullptr;
            }
        return properties;
        }
    else if (ChangeStatus::Modified == status)
        {
        if (SUCCESS != ReadObjectPropertiesForModification(instance, *properties))
            {
            return nullptr;
            }
        return properties;
        }

    return nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::ReadObjectPropertiesForCreation(ECInstanceKeyCR instanceKey, JsonValueR propertiesJsonOut)
    {
    if (SUCCESS != ReadObjectProperties(instanceKey, propertiesJsonOut))
        {
        return ERROR;
        }

    ECClassCP ecClass = m_dbAdapter.GetECClass(instanceKey.GetECClassId());
    RemoveCalculatedProperties(propertiesJsonOut, *ecClass);

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::ReadObjectPropertiesForModification(ECInstanceKeyCR instanceKey, JsonValueR propertiesJsonOut)
    {
    if (SUCCESS != ReadModifiedProperties(instanceKey, propertiesJsonOut))
        {
        return ERROR;
        }

    ECClassCP ecClass = m_dbAdapter.GetECClass(instanceKey);
    RemoveReadOnlyProperties(propertiesJsonOut, *ecClass);

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ChangeManager::ReadObjectProperties(ECInstanceKeyCR instanceKey, JsonValueR propertiesJsonOut)
    {
    if (SUCCESS != m_dbAdapter.GetJsonInstance(propertiesJsonOut, instanceKey))
        {
        return ERROR;
        }

    RemoveCacheSpecificProperties(propertiesJsonOut);

    // Graphite03 ECDb is incapable of storing NULLABLE structs and arrays so it returns structs with empty array properties.
    // This causes errors to be returned from the server.
    // Still needed on Graphite0505 
    RemoveEmptyMembersRecursively(propertiesJsonOut);

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeManager::RemoveCacheSpecificProperties(JsonValueR propertiesJson)
    {
    for (Utf8StringCR member : propertiesJson.getMemberNames())
        {
        // Remove ECDb ECInstance JSON properties
        if (member[0] == '$')
            {
            propertiesJson.removeMember(member);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeManager::RemoveReadOnlyProperties(JsonValueR propertiesJson, ECClassCR ecClass)
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
void ChangeManager::RemoveCalculatedProperties(JsonValueR propertiesJson, ECClassCR ecClass)
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
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeManager::RemoveEmptyMembersRecursively(JsonValueR jsonObject)
    {
    for (Utf8StringCR memberName : jsonObject.getMemberNames())
        {
        RemoveEmptyMembersRecursively(jsonObject[memberName], memberName, jsonObject);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeManager::RemoveEmptyMembersRecursively(JsonValueR childJson, Utf8StringCR childMemberNameInParent, JsonValueR parentJson)
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
