/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Instances/InstanceCacheHelper.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "InstanceCacheHelper.h"

#include <WebServices/Cache/Persistence/DataSourceCacheCommon.h>
#include <WebServices/Cache/Persistence/CachedObjectInfo.h>
#include <WebServices/Cache/Util/ECDbHelper.h>

#include "../Core/CacheSchema.h"
#include "../Hierarchy/HierarchyManager.h"
#include "../../Util/JsonUtil.h"
#include "../../Logging.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceCacheHelper::InstanceCacheHelper
(
ECDbAdapter& dbAdapter,
HierarchyManager& hierarchyManager,
ObjectInfoManager& objectInfoManager,
RelationshipInfoManager& relationshipInfoManager,
ChangeInfoManager& changeInfoManager
) :
m_dbAdapter(dbAdapter),
m_hierarchyManager(hierarchyManager),
m_objectInfoManager(objectInfoManager),
m_relationshipInfoManager(relationshipInfoManager),
m_changeInfoManager(changeInfoManager),
m_inserters(dbAdapter.GetECDb()),
m_updaters(dbAdapter.GetECDb())
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceCacheHelper::CacheInstances
(
const WSObjectsReader::Instances& instances,
CachedInstances& cachedInstancesInOut,
PartialCachingState* partialCachingState,
UpdateCachingState* updateCachingState,
ICancellationTokenPtr ct
)
    {
    if (!instances.IsValid())
        {
        return ERROR;
        }

    bvector<SelectPathElement> rootPath;
    for (WSObjectsReader::Instance instance : instances)
        {
        if (SUCCESS != CacheInstance(instance, rootPath, cachedInstancesInOut, partialCachingState, updateCachingState, nullptr, ct))
            {
            LOG.errorv("Failed to cache instance %s", instance.GetObjectId().ToString().c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceCacheHelper::CacheInstance
(
const WSObjectsReader::Instance& instance,
const bvector<SelectPathElement>& path,
CachedInstances& cachedInstancesInOut,
PartialCachingState* partialCachingState,
UpdateCachingState* updateCachingState,
ECInstanceKey* cachedInstanceOut,
ICancellationTokenPtr ct
)
    {
    if (ct && ct->IsCanceled())
        {
        return ERROR;
        }

    if (!instance.IsValid())
        {
        return ERROR;
        }

    ECInstanceKey cachedInstance;
    if (!cachedInstancesInOut.HasCachedInstance(instance.GetObjectId()))
        {
        ObjectInfo info = m_objectInfoManager.ReadInfo(instance.GetObjectId());
        auto changeStatus = info.GetChangeStatus();

        auto action = PartialCachingState::Action::CacheFull;
        if (nullptr != partialCachingState)
            {
            action = partialCachingState->GetAction(info, path);
            }

        if (IChangeManager::ChangeStatus::Created == changeStatus)
            {
            return ERROR; // Instance cannot be cached - its not pushed to server yet
            }
        else if (IChangeManager::ChangeStatus::Deleted == changeStatus)
            {
            // Nothing to do here
            }
        else if (nullptr != updateCachingState && !info.IsInCache())
            {
            updateCachingState->AddNotFound(instance.GetObjectId());
            return SUCCESS;
            }
        else if (PartialCachingState::Action::SkipCached == action)
            {
            // Nothing to do here
            }
        else if (PartialCachingState::Action::Reject == action)
            {
            partialCachingState->AddRejected(instance.GetObjectId());
            }
        else
            {
            if (PartialCachingState::Action::CachePartial == action)
                {
                info.SetObjectState(CachedInstanceState::Partial);
                }
            else if (PartialCachingState::Action::CacheFull == action)
                {
                info.SetObjectState(CachedInstanceState::Full);
                }
            else
                {
                BeAssert(false);
                return ERROR;
                }

            info.SetObjectCacheDate(DateTime::GetCurrentTimeUtc());
            info.SetObjectCacheTag(instance.GetETag());

            if (SUCCESS != CacheInstance(info, instance.GetProperties()))
                {
                return ERROR;
                }
            }

        auto key = info.GetCachedInstanceKey();
        cachedInstance = key.GetInstanceKey();
        cachedInstancesInOut.AddInstance(instance.GetObjectId(), key);

        if (IChangeManager::ChangeStatus::Deleted == changeStatus)
            {
            cachedInstancesInOut.MarkDeleted(cachedInstance);
            }
        }
    else
        {
        cachedInstance = cachedInstancesInOut.GetCachedInstance(instance.GetObjectId());
        }

    if (nullptr != cachedInstanceOut)
        {
        *cachedInstanceOut = cachedInstance;
        }

    auto relationshipInstances = instance.GetRelationshipInstances();
    if (!relationshipInstances.IsValid())
        {
        return ERROR;
        }

    for (auto relationshipInstance : relationshipInstances)
        {
        if (!relationshipInstance.IsValid())
            {
            return ERROR;
            }

        SelectPathElement element;
        element.relationshipClass = m_dbAdapter.GetECRelationshipClass(relationshipInstance.GetObjectId());
        element.selectedClass = m_dbAdapter.GetECClass(relationshipInstance.GetRelatedInstance().GetObjectId());
        element.direction = relationshipInstance.GetDirection();

        bvector<SelectPathElement> relatedPath = path;
        relatedPath.push_back(element);

        ECInstanceKey relatedInstance;
        if (SUCCESS != CacheInstance(relationshipInstance.GetRelatedInstance(), relatedPath, cachedInstancesInOut, partialCachingState,
            updateCachingState, &relatedInstance, ct))
            {
            return ERROR;
            }

        if (!cachedInstancesInOut.IsDeleted(cachedInstance) && 
            !cachedInstancesInOut.IsDeleted(relatedInstance))
            {
            if (SUCCESS != CacheRelationshipInstance(relationshipInstance, cachedInstance, relatedInstance, cachedInstancesInOut))
                {
                LOG.errorv("Failed to cache relationship instance %s", relationshipInstance.GetObjectId().ToString().c_str());
                return ERROR;
                }
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceCacheHelper::CacheRelationshipInstance
(
const WSObjectsReader::RelationshipInstance& relationshipInstance,
ECInstanceKeyCR relatedInstanceA,
ECInstanceKeyCR relatedInstanceB,
CachedInstances& cachedInstancesInOut
)
    {
    if (!relationshipInstance.IsValid() ||
        !relatedInstanceA.IsValid() ||
        !relatedInstanceB.IsValid())
        {
        return ERROR;
        }

    // NOTE: instanceId is ignored

    ObjectIdCR relationshipObjectId = relationshipInstance.GetObjectId();

    ECRelationshipClassCP relClass = m_dbAdapter.GetECRelationshipClass(relationshipObjectId);
    if (nullptr == relClass)
        {
        return ERROR;
        }

    ECInstanceKeyCP source, target = nullptr;
    if (BentleyApi::ECN::ECRelatedInstanceDirection::Forward == relationshipInstance.GetDirection())
        {
        source = &relatedInstanceA;
        target = &relatedInstanceB;
        }
    else
        {
        source = &relatedInstanceB;
        target = &relatedInstanceA;
        }

    // TODO: insert/update whole relationship with properties at once?
    ECInstanceKey relationshipKey = m_hierarchyManager.RelateInstances(*source, *target, relClass);
    if (!relationshipKey.IsValid())
        {
        BeAssert("Failed to cache relationship instance. Check schema");
        return ERROR;
        }

    if (0 != relClass->GetPropertyCount())
        {
        if (SUCCESS != m_updaters.Get(*relClass).Update(relationshipKey.GetECInstanceId(),
            relationshipInstance.GetProperties(), *source, *target))
            {
            return ERROR;
            }
        }

    CachedInstanceKey relInfo = m_relationshipInfoManager
        .ReadOrInsertCachedRelationshipKey(relationshipKey, relationshipObjectId.remoteId.c_str());

    cachedInstancesInOut.AddInstance(relationshipObjectId, relInfo);

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceCacheHelper::CacheInstance(ObjectInfoR infoInOut, RapidJsonValueCR properties)
    {
    ECClassCP ecClass = m_dbAdapter.GetECClass(infoInOut.GetInstanceKey());
    if (nullptr == ecClass)
        {
        BeAssert(false && "Unknown instance class");
        return ERROR;
        }

    // Save instance properties 
    BentleyStatus status = ERROR;
    if (infoInOut.GetChangeStatus() == IChangeManager::ChangeStatus::Modified)
        {
        status = MergeAndSaveModifiedInstance(infoInOut, *ecClass, properties);
        }
    else if (infoInOut.IsInCache())
        {
        status = SaveExistingInstance(infoInOut, *ecClass, properties);
        }
    else
        {
        status = SaveNewInstance(infoInOut, *ecClass, properties);
        }

    if (SUCCESS != status)
        {
        LOG.errorv("Failed to cache instance %s", infoInOut.GetObjectId().ToString().c_str());
        return ERROR;
        }

    // Save info
    if (infoInOut.GetInfoKey().IsValid())
        {
        status = m_objectInfoManager.UpdateInfo(infoInOut);
        }
    else
        {
        status = m_objectInfoManager.InsertInfo(infoInOut);
        }

    return status;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceCacheHelper::SaveNewInstance(ObjectInfoR infoInOut, ECClassCR ecClass, RapidJsonValueCR properties)
    {
    ECInstanceKey ecInstanceKey;
    if (SUCCESS != m_inserters.Get(ecClass).Insert(ecInstanceKey, properties))
        {
        return ERROR;
        }

    infoInOut.SetInstanceId(ecInstanceKey.GetECInstanceId());
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceCacheHelper::SaveExistingInstance(ObjectInfoCR info, ECClassCR ecClass, RapidJsonValueCR properties)
    {
    return m_updaters.Get(ecClass).Update(info.GetInstanceKey().GetECInstanceId(), properties);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceCacheHelper::MergeAndSaveModifiedInstance(ObjectInfoCR info, ECClassCR ecClass, RapidJsonValueCR newPropertiesJson)
    {
    // Merge changes
    rapidjson::Document mergedJson, t1, t2;
    JsonUtil::DeepCopy(newPropertiesJson, mergedJson);

    if (SUCCESS != m_changeInfoManager.ReadInstanceChanges(info, mergedJson, t1, t2))
        {
        return ERROR;
        }

    // Save merged instance
    if (SUCCESS != m_updaters.Get(ecClass).Update(info.GetInstanceKey().GetECInstanceId(), mergedJson))
        {
        return ERROR;
        }

    // Save latest version as backup
    if (SUCCESS != m_changeInfoManager.SaveBackupInstance(info, newPropertiesJson))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceCacheHelper::UpdateExistingInstanceData(ObjectInfoCR info, RapidJsonValueCR properties)
    {
    if (!info.GetCachedInstanceKey().IsValid())
        {
        BeAssert(false && "Invalid existing instance key");
        return ERROR;
        }

    ECClassCP ecClass = m_dbAdapter.GetECClass(info.GetInstanceKey());
    if (SUCCESS != SaveExistingInstance(info, *ecClass, properties))
        {
        LOG.errorv("Failed to update instance %s", info.GetObjectId().ToString().c_str());
        return ERROR;
        }

    if (SUCCESS != m_objectInfoManager.UpdateInfo(info))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCacheHelper::CachedInstances::AddInstance(ObjectIdCR objectId, CachedInstanceKeyCR key)
    {
    m_cachedInstances.insert(key);
    m_cachedInstancesByObjectId.insert({objectId, key.GetInstanceKey()});
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceCacheHelper::CachedInstances::HasCachedInstance(ObjectIdCR objectId) const
    {
    return m_cachedInstancesByObjectId.find(objectId) != m_cachedInstancesByObjectId.end();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey InstanceCacheHelper::CachedInstances::GetCachedInstance(ObjectIdCR objectId) const
    {
    auto it = m_cachedInstancesByObjectId.find(objectId);
    if (m_cachedInstancesByObjectId.end() == it)
        {
        return ECInstanceKey();
        }
    return it->second;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const bset<CachedInstanceKey>& InstanceCacheHelper::CachedInstances::GetCachedInstances() const
    {
    return m_cachedInstances;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bset<ObjectId> InstanceCacheHelper::CachedInstances::GetCachedInstanceObjectIds() const
    {
    bset<ObjectId> instances;
    for (auto& pair : m_cachedInstancesByObjectId)
        {
        instances.insert(pair.first);
        }
    return instances;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const bmap<ObjectId, ECInstanceKey>& InstanceCacheHelper::CachedInstances::GetCachedInstancesByObjectId() const
    {
    return m_cachedInstancesByObjectId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCacheHelper::CachedInstances::MarkDeleted(ECInstanceKeyCR instanceKey)
    {
    m_deletedInstances.insert(instanceKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceCacheHelper::CachedInstances::IsDeleted(ECInstanceKeyCR instanceKey) const
    {
    return m_deletedInstances.find(instanceKey) != m_deletedInstances.end();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceCacheHelper::PartialCachingState::PartialCachingState
(
ECDbAdapterR dbAdapter,
WSQueryCR query,
const ECInstanceKeyMultiMap& fullyPersistedInstances,
bset<ObjectId>& rejected
) :
m_query(&query),
m_fullyPersistedInstances(fullyPersistedInstances),
m_rejected(rejected)
    {
    BuildSelectedPaths(dbAdapter, *m_query, m_allPropertiesSelectedPaths, m_idOnlySelectedPaths);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceCacheHelper::PartialCachingState::BuildSelectedPaths
(
ECDbAdapterR dbAdapter,
WSQueryCR query,
bset<bvector<SelectPathElement>>& allPropertiesSelectedPathsOut,
bset<bvector<SelectPathElement>>& idOnlySelectedPathsOut
)
    {
    Utf8StringCR mainSchemaName = query.GetSchemaName();

    Utf8String selectOption = query.GetSelect();
    if (selectOption.empty())
        {
        allPropertiesSelectedPathsOut.insert(bvector<SelectPathElement>());
        return SUCCESS;
        }

    for (auto pair : query.GetAliasMapping())
        {
        selectOption.ReplaceAll(pair.second.c_str(), pair.first.c_str());
        }

    bvector<Utf8String> selects;
    BeStringUtilities::Split(selectOption.c_str(), ",", selects);
    bmap<bvector<SelectPathElement>, SelectType> paths;

    for (Utf8StringCR select : selects)
        {
        bvector<SelectPathElement> path;
        SelectType selectType = SelectType::Property;

        if (SUCCESS != GetSelectPathAndType(dbAdapter, mainSchemaName, select, path, selectType))
            {
            BeAssert(false);
            return ERROR;
            }

        auto it = paths.find(path);
        if (it == paths.end() ||
            SelectType::All == selectType ||
            SelectType::Id == it->second)
            {
            paths[path] = selectType;
            }
        }

    for (auto& pair : paths)
        {
        if (SelectType::All == pair.second)
            {
            allPropertiesSelectedPathsOut.insert(pair.first);
            }
        else if (SelectType::Id == pair.second)
            {
            idOnlySelectedPathsOut.insert(pair.first);
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceCacheHelper::PartialCachingState::GetSelectPathAndType
(
ECDbAdapterR dbAdapter,
Utf8StringCR mainSchemaName,
Utf8StringCR selectStr,
bvector<SelectPathElement>& pathOut,
SelectType& selectTypeOut
)
    {
    pathOut.clear();

    if (selectStr.empty())
        {
        return ERROR;
        }

    if ("*" == selectStr)
        {
        selectTypeOut = SelectType::All;
        return SUCCESS;
        }

    if ("$id" == selectStr)
        {
        selectTypeOut = SelectType::Id;
        return SUCCESS;
        }

    if (Utf8String::npos == selectStr.find("-"))
        {
        selectTypeOut = SelectType::Property;
        return SUCCESS;
        }

    // Related select   
    bvector<Utf8String> nestedRelationships;
    BeStringUtilities::Split(selectStr.c_str(), "/", nestedRelationships);

    for (Utf8StringCR nestedRelationship : nestedRelationships)
        {
        bvector<Utf8String> relationshipSelectTokens;
        BeStringUtilities::Split(nestedRelationship.c_str(), "-", relationshipSelectTokens);

        if (relationshipSelectTokens.size() != 3)
            {
            return ERROR;
            }

        Utf8StringCR relationshipClassToken = relationshipSelectTokens[0];
        Utf8StringCR directionToken = relationshipSelectTokens[1];
        Utf8StringR relatedClassToken = relationshipSelectTokens[2];

        bool lastRelationshipInPath = (nestedRelationships.size() - 1) == pathOut.size();
        if (lastRelationshipInPath)
            {
            auto selectPos = relatedClassToken.find_last_of(".");
            if (Utf8String::npos == selectPos)
                {
                return ERROR;
                }

            if (0 == strcmp(relatedClassToken.c_str() + selectPos, ".*"))
                {
                selectTypeOut = SelectType::All;
                }
            else if (0 == strcmp(relatedClassToken.c_str() + selectPos, ".$id"))
                {
                selectTypeOut = SelectType::Id;
                }

            relatedClassToken.resize(selectPos);
            }

        SelectPathElement element;
        element.relationshipClass = GetECClassFromClassToken(dbAdapter, mainSchemaName, relationshipClassToken);
        element.relationshipClassPolymorphic = IsClassTokenPolymorphic(relationshipClassToken);
        element.selectedClass = GetECClassFromClassToken(dbAdapter, mainSchemaName, relatedClassToken);
        element.selectedClassPolymorphically = IsClassTokenPolymorphic(relatedClassToken);
        element.direction = GetDirection(directionToken);

        if (!element.IsValid())
            {
            return ERROR;
            }

        pathOut.push_back(element);
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceCacheHelper::PartialCachingState::IsClassTokenPolymorphic(Utf8StringCR classToken)
    {
    return Utf8String::npos != classToken.find("!poly");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCacheHelper::PartialCachingState::GetSchemaAndClassNamesFromClassToken
(
Utf8StringCR classToken,
Utf8StringR schemaNameOut,
Utf8StringR classNameOut
)
    {
    size_t schemaSeperatorIndex = classToken.find('.');
    size_t polySeperatorIndex = classToken.find('!');

    size_t classNameStart = schemaSeperatorIndex != Utf8String::npos ? schemaSeperatorIndex + 1 : 0;
    size_t classNameEnd = polySeperatorIndex != Utf8String::npos ? polySeperatorIndex : classToken.size();

    classNameOut.assign(classToken.begin() + classNameStart, classToken.begin() + classNameEnd);

    if (schemaSeperatorIndex != Utf8String::npos)
        {
        schemaNameOut.assign(classToken.begin(), classToken.begin() + schemaSeperatorIndex);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP InstanceCacheHelper::PartialCachingState::GetECClassFromClassToken(ECDbAdapterR dbAdapter, Utf8StringCR mainSchemaName, Utf8StringCR classToken)
    {
    Utf8String schemaName, className;
    GetSchemaAndClassNamesFromClassToken(classToken, schemaName, className);

    if (schemaName.empty())
        {
        return dbAdapter.GetECClass(mainSchemaName, className);
        }

    return dbAdapter.GetECClass(schemaName, className);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelatedInstanceDirection InstanceCacheHelper::PartialCachingState::GetDirection(Utf8StringCR directionString)
    {
    if ("forward" == directionString)
        {
        return ECRelatedInstanceDirection::Forward;
        }
    else if ("backward" == directionString)
        {
        return ECRelatedInstanceDirection::Backward;
        }
    BeAssert(false);
    return (ECRelatedInstanceDirection) 0;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCacheHelper::PartialCachingState::AddRejected(ObjectIdCR objectId)
    {
    m_rejected.insert(objectId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceCacheHelper::PartialCachingState::Action InstanceCacheHelper::PartialCachingState::GetAction
(
ObjectInfoCR info,
const bvector<SelectPathElement>& path
)
    {
    bool allRequired = DoesRequireAllProperties(info);
    bool allSelected = DoesPathMatches(path, m_allPropertiesSelectedPaths);

    if (allRequired)
        {
        if (allSelected)
            {
            return Action::CacheFull;
            }
        bool idOnlySelected = DoesPathMatches(path, m_idOnlySelectedPaths);
        if (idOnlySelected)
            {
            return Action::SkipCached;
            }
        return Action::Reject;
        }

    if (allSelected)
        {
        return Action::CacheFull;
        }

    return Action::CachePartial;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceCacheHelper::PartialCachingState::DoesPathMatches
(
const bvector<SelectPathElement>& instancePath,
const bset<bvector<SelectPathElement>>& matchPaths
)
    {
    auto it = std::find_if(matchPaths.begin(), matchPaths.end(),
                           [&] (const bvector<SelectPathElement>& candidatePath)
        {
        if (candidatePath.size() != instancePath.size())
            {
            return false;
            }

        return std::equal(candidatePath.begin(), candidatePath.end(), instancePath.begin(),
                          [&] (const SelectPathElement& candidateElement, const SelectPathElement& element)
            {
            return
                candidateElement.direction == element.direction &&
                ((candidateElement.relationshipClassPolymorphic && element.relationshipClass->Is(candidateElement.relationshipClass)) ||
                (!candidateElement.relationshipClassPolymorphic && element.relationshipClass == candidateElement.relationshipClass)) &&
                ((candidateElement.selectedClassPolymorphically && element.selectedClass->Is(candidateElement.selectedClass)) ||
                (!candidateElement.selectedClassPolymorphically && element.selectedClass == candidateElement.selectedClass));
            });
        });

    return it != matchPaths.end();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceCacheHelper::PartialCachingState::DoesRequireAllProperties(ObjectInfoCR info)
    {
    if (!info.IsInCache())
        {
        return false;
        }

    if (IChangeManager::ChangeStatus::Modified == info.GetChangeStatus())
        {
        return true;
        }

    if (IsFullyPersisted(info))
        {
        return true;
        }

    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceCacheHelper::PartialCachingState::IsFullyPersisted(ObjectInfoCR info)
    {
    return
        info.IsFullyCached() &&
        ECDbHelper::IsInstanceInMultiMap(info.GetInstanceKey(), m_fullyPersistedInstances);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCacheHelper::UpdateCachingState::AddNotFound(ObjectIdCR objectId)
    {
    m_notFoundObjectIds.insert(objectId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
const bset<ObjectId>& InstanceCacheHelper::UpdateCachingState::GetNotFoundObjectIds() const
    {
    return m_notFoundObjectIds;
    }
