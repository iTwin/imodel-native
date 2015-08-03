/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Instances/InstanceCacheHelper.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "InstanceCacheHelper.h"

#include <WebServices/Cache/Persistence/DataSourceCacheCommon.h>
#include <WebServices/Cache/Persistence/CachedObjectInfo.h>
#include <WebServices/Cache/Util/ECDbHelper.h>

#include "../Core/CacheSchema.h"
#include "../Hierarchy/HierarchyManager.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceCacheHelper::InstanceCacheHelper
(
ECDbAdapter& dbAdapter,
HierarchyManager& hierarchyManager,
ObjectInfoManager& objectInfoManager,
RelationshipInfoManager& relationshipInfoManager
) :
m_dbAdapter(dbAdapter),
m_hierarchyManager(hierarchyManager),
m_objectInfoManager(objectInfoManager),
m_relationshipInfoManager(relationshipInfoManager),
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
ICancellationTokenPtr cancellationToken
)
    {
    if (!instances.IsValid())
        {
        return ERROR;
        }

    bvector<SelectPathElement> rootPath;
    for (WSObjectsReader::Instance instance : instances)
        {
        if (SUCCESS != CacheInstance(instance, rootPath, cachedInstancesInOut, partialCachingState, updateCachingState, nullptr, cancellationToken))
            {
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
ICancellationTokenPtr cancellationToken
)
    {
    if (cancellationToken && cancellationToken->IsCanceled())
        {
        return ERROR;
        }

    if (!instance.IsValid())
        {
        return ERROR;
        }

    ECInstanceKey cachedInstance;
    if (cachedInstancesInOut.HasCachedInstance(instance.GetObjectId()))
        {
        ObjectInfo info = m_objectInfoManager.ReadInfo(instance.GetObjectId());

        if (nullptr != partialCachingState && partialCachingState->ShouldReject(info, path))
            {
            partialCachingState->AddRejected(instance.GetObjectId());
            }
        else if (nullptr != updateCachingState && !info.IsInCache())
            {
            updateCachingState->AddNotFound(instance.GetObjectId());
            return SUCCESS;
            }
        else
            {
            if (nullptr != partialCachingState && !partialCachingState->AreAllPropertiesSelected(path))
                {
                info.SetObjectState(CachedInstanceState::Partial);
                }
            else
                {
                info.SetObjectState(CachedInstanceState::Full);
                }

            info.SetObjectCacheDate(DateTime::GetCurrentTimeUtc());
            info.SetObjectCacheTag(instance.GetETag());

            if (SUCCESS != CacheInstance(info, instance.GetProperties()))
                {
                return ERROR;
                }
            }

        cachedInstance = info.GetCachedInstanceKey();
        cachedInstancesInOut.AddInstance(instance.GetObjectId(), cachedInstance);
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
            updateCachingState, &relatedInstance, cancellationToken))
            {
            return ERROR;
            }

        if (SUCCESS != CacheRelationshipInstance(relationshipInstance, cachedInstance, relatedInstance, cachedInstancesInOut))
            {
            return ERROR;
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
    // NOTE: relationship instance properties not yet supported
    BeAssert(relationshipInstance.GetProperties().MemberBegin() == relationshipInstance.GetProperties().MemberEnd());

    ObjectIdCR relationshipObjectId = relationshipInstance.GetObjectId();

    ECRelationshipClassCP relClass = m_dbAdapter.GetECRelationshipClass(relationshipObjectId);
    if (nullptr == relClass)
        {
        return ERROR;
        }

    ECInstanceKey cachedRelationship;
    if (BentleyApi::ECN::ECRelatedInstanceDirection::Forward == relationshipInstance.GetDirection())
        {
        cachedRelationship = m_hierarchyManager.RelateInstances(relatedInstanceA, relatedInstanceB, relClass);
        }
    else
        {
        cachedRelationship = m_hierarchyManager.RelateInstances(relatedInstanceB, relatedInstanceA, relClass);
        }

    if (!cachedRelationship.IsValid())
        {
        BeAssert("Failed to cache relationship instance. Check schema");
        return ERROR;
        }

    CachedRelationshipKey relInfo = m_relationshipInfoManager.ReadCachedRelationshipKey(cachedRelationship, relationshipObjectId.remoteId.c_str());

    cachedInstancesInOut.AddRelationshipInstance(relationshipObjectId, relInfo);

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceCacheHelper::CacheInstance(ObjectInfoR infoInOut, RapidJsonValueCR properties)
    {
    if (infoInOut.IsInCache())
        {
        return CacheExistingInstance(infoInOut, properties);
        }
    else
        {
        return CacheNewInstance(infoInOut, properties);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceCacheHelper::CacheNewInstance(ObjectInfoR infoInOut, RapidJsonValueCR properties)
    {
    ECClassCP instanceClass = m_dbAdapter.GetECClass(infoInOut.GetCachedInstanceKey());
    if (nullptr == instanceClass)
        {
        BeAssert(false && "Unknown class passed to cache");
        return ERROR;
        }

    ECInstanceKey newInstanceKey;
    JsonInserter inserter(m_dbAdapter.GetECDb(), *instanceClass);

    if (SUCCESS != inserter.Insert(newInstanceKey, properties))
        {
        BeAssert("Failed to cache instance. Check schema");
        return ERROR;
        }
    infoInOut.SetCachedInstanceId(newInstanceKey.GetECInstanceId());

    if (SUCCESS != m_objectInfoManager.InsertInfo(infoInOut))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceCacheHelper::CacheExistingInstance(ObjectInfoCR info, RapidJsonValueCR properties)
    {
    if (!info.GetCachedInstanceKey().IsValid())
        {
        BeAssert(false && "Invalid existing instance key");
        return ERROR;
        }

    ECClassCP ecClass = m_dbAdapter.GetECClass(info.GetCachedInstanceKey());
    if (SUCCESS != m_updaters.Get(*ecClass).Update(info.GetCachedInstanceKey().GetECInstanceId(), properties))
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
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCacheHelper::CachedInstances::AddInstance(ObjectIdCR objectId, ECInstanceKeyCR cachedInstance)
    {
    m_cachedInstances.insert(cachedInstance);
    m_cachedInstancesByObjectId.insert({objectId, cachedInstance});
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceCacheHelper::CachedInstances::AddRelationshipInstance(ObjectIdCR objectId, const CachedRelationshipKey& relationshipInfo)
    {
    m_cachedInstances.insert(relationshipInfo.GetRelationshipKey());
    m_cachedInstancesByObjectId.insert({objectId, relationshipInfo.GetRelationshipKey()});
    m_cachedRelationships.insert(relationshipInfo);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceCacheHelper::CachedInstances::HasCachedInstance(ObjectIdCR objectId) const
    {
    return m_cachedInstancesByObjectId.find(objectId) == m_cachedInstancesByObjectId.end();
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
const bset<ECInstanceKey>& InstanceCacheHelper::CachedInstances::GetCachedInstances() const
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
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const bset<CachedRelationshipKey> InstanceCacheHelper::CachedInstances::GetCachedRelationshipInfos() const
    {
    return m_cachedRelationships;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceCacheHelper::PartialCachingState::PartialCachingState
(
ECDbAdapterR dbAdapter,
const WSQuery* query,
const ECInstanceKeyMultiMap& fullyPersistedInstances,
bset<ObjectId>& rejected
) :
m_query(query),
m_fullyPersistedInstances(fullyPersistedInstances),
m_rejected(rejected)
    {
    if (nullptr != query)
        {
        BuildAllPropertiesSelectedPaths(dbAdapter, *m_query, m_allPropertiesSelectedPaths);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceCacheHelper::PartialCachingState::BuildAllPropertiesSelectedPaths
(
ECDbAdapterR dbAdapter,
WSQueryCR query,
bset<bvector<SelectPathElement>>& allPropertiesSelectedPathsOut
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

    for (Utf8StringCR select : selects)
        {
        if (select.empty())
            {
            BeAssert(false);
            return ERROR;
            }

        if ("*" == select)
            {
            allPropertiesSelectedPathsOut.insert(bvector<SelectPathElement>());
            continue;
            }

        if (select.find("-") == Utf8String::npos)
            {
            // Property select
            continue;
            }

        bvector<Utf8String> nestedRelationships;
        BeStringUtilities::Split(select.c_str(), "/", nestedRelationships);

        bvector<SelectPathElement> path;
        for (Utf8StringCR nestedRelationship : nestedRelationships)
            {
            bool lastRelationshipInPath = (nestedRelationships.size() - 1) == path.size();

            bvector<Utf8String> relationshipSelectTokens;
            BeStringUtilities::Split(nestedRelationship.c_str(), "-", relationshipSelectTokens);

            if (relationshipSelectTokens.size() != 3)
                {
                BeAssert(false);
                return ERROR;
                }

            Utf8StringCR relationshipClassToken = relationshipSelectTokens[0];
            Utf8StringCR directionToken = relationshipSelectTokens[1];
            Utf8StringR relatedClassToken = relationshipSelectTokens[2];

            if (lastRelationshipInPath)
                {
                Utf8String selectAllPropertiesMarker = ".*";
                if (relatedClassToken.EndsWith(selectAllPropertiesMarker))
                    {
                    relatedClassToken.resize(relatedClassToken.size() - selectAllPropertiesMarker.size());
                    }
                else
                    {
                    continue;
                    }
                }

            SelectPathElement element;
            element.relationshipClass = GetECClassFromClassToken(dbAdapter, mainSchemaName, relationshipClassToken);
            element.relationshipClassPolymorphic = IsClassTokenPolymorphic(relationshipClassToken);
            element.selectedClass = GetECClassFromClassToken(dbAdapter, mainSchemaName, relatedClassToken);
            element.selectedClassPolymorphically = IsClassTokenPolymorphic(relatedClassToken);
            element.direction = GetDirection(directionToken);

            if (!element.IsValid())
                {
                BeAssert(false);
                return ERROR;
                }

            path.push_back(element);

            if (lastRelationshipInPath)
                {
                allPropertiesSelectedPathsOut.insert(path);
                }
            }
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
bool InstanceCacheHelper::PartialCachingState::ShouldReject(ObjectInfoCR info, const bvector<SelectPathElement>& path)
    {
    return IsFullyPersisted(info) && !AreAllPropertiesSelected(path);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceCacheHelper::PartialCachingState::AreAllPropertiesSelected(const bvector<SelectPathElement>& instancePath)
    {
    if (nullptr == m_query)
        {
        return false;
        }

    auto it = std::find_if(m_allPropertiesSelectedPaths.begin(), m_allPropertiesSelectedPaths.end(),
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

    return it != m_allPropertiesSelectedPaths.end();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceCacheHelper::PartialCachingState::IsFullyPersisted(ObjectInfoCR info)
    {
    return
        info.IsFullyCached() &&
        ECDbHelper::IsInstanceInMultiMap(info.GetCachedInstanceKey(), m_fullyPersistedInstances);
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
