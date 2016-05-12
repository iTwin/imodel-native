/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Instances/InstanceCacheHelper.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <ECDb/ECDbApi.h>
#include <rapidjson/BeRapidJson.h>
#include <WebServices/Cache/Util/ECSqlAdapterCache.h>
#include <WebServices/Client/Response/WSObjectsResponse.h>
#include <WebServices/Client/WSQuery.h>

#include "ObjectInfoManager.h"
#include "RelationshipInfoManager.h"
#include "../Changes/ChangeInfoManager.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceCacheHelper
    {
    public:
        struct CachedInstances;
        struct PartialCachingState;
        struct UpdateCachingState;
        struct SelectPathElement;

    private:
        ECDbAdapter&                m_dbAdapter;
        HierarchyManager&           m_hierarchyManager;
        ObjectInfoManager&          m_objectInfoManager;
        RelationshipInfoManager&    m_relationshipInfoManager;
        ChangeInfoManager&          m_changeInfoManager;

        ECSqlAdapterCache<JsonInserter> m_inserters;
        ECSqlAdapterCacheWithOptions<JsonUpdater> m_updaters;

    private:
        BentleyStatus CacheInstance
            (
            const WSObjectsReader::Instance& instance,
            const bvector<SelectPathElement>& path,
            CachedInstances& cachedInstancesInOut,
            PartialCachingState* partialCachingState,
            UpdateCachingState* updateCachingState,
            ECInstanceKey* cachedInstanceOut,
            ICancellationTokenPtr ct
            );

        BentleyStatus CacheRelationshipInstance
            (
            const WSObjectsReader::RelationshipInstance& relationshipInstance,
            ECInstanceKeyCR relatedInstanceA,
            ECInstanceKeyCR relatedInstanceB,
            CachedInstances& cachedInstancesInOut
            );

        BentleyStatus SaveNewInstance(ObjectInfoR infoInOut, ECClassCR ecClass, RapidJsonValueCR properties);
        BentleyStatus SaveExistingInstance(ObjectInfoCR infoInOut, ECClassCR ecClass, RapidJsonValueCR properties);
        BentleyStatus MergeAndSaveModifiedInstance(ObjectInfoCR info, ECClassCR ecClass, RapidJsonValueCR properties);

    public:
        InstanceCacheHelper
            (
            ECDbAdapter& dbAdapter,
            HierarchyManager& hierarchyManager,
            ObjectInfoManager& objectInfoManager,
            RelationshipInfoManager& relationshipInfoManager,
            ChangeInfoManager& changeInfoManager
            );

        //! Caches instances, related instances and information to cache. Will update existing or create new instnaces.
        //! Relationship info will be cached and not related to any instance. Calling code is responsible for relating relationship info
        //! instances to some holding object. Relationship infos will be saved to cachedRelationshipToCachedRelationshipInfoOut
        //! If partialCachingState is set, instances will be cached as partial or rejected to same parameter
        //! If updateCachingState is set, only existing instances will be cached
        BentleyStatus CacheInstances
            (
            const WSObjectsReader::Instances& instances,
            CachedInstances& cachedInstancesInOut,
            PartialCachingState* partialCachingState = nullptr,
            UpdateCachingState* updateCachingState = nullptr,
            ICancellationTokenPtr ct = nullptr
            );

        //! Utility for caching new or existing instance to cache and handle modified instance merging
        BentleyStatus CacheInstance(ObjectInfoR infoInOut, RapidJsonValueCR properties);

        //! Utility for directly updating cached instance data
        BentleyStatus UpdateExistingInstanceData(ObjectInfoCR info, RapidJsonValueCR properties);
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceCacheHelper::CachedInstances
    {
    private:
        bset<CachedInstanceKey> m_cachedInstances;
        bmap<ObjectId, ECInstanceKey> m_cachedInstancesByObjectId;
        bset<ECInstanceKey> m_deletedInstances;

    public:
        void AddInstance(ObjectIdCR objectId, CachedInstanceKeyCR key);

        bool HasCachedInstance(ObjectIdCR objectId) const;
        ECInstanceKey GetCachedInstance(ObjectIdCR objectId) const;

        const bset<CachedInstanceKey>& GetCachedInstances() const;
        bset<ObjectId> GetCachedInstanceObjectIds() const;
        const bmap<ObjectId, ECInstanceKey>& GetCachedInstancesByObjectId() const;

        void MarkDeleted(ECInstanceKeyCR instanceKey);
        bool IsDeleted(ECInstanceKeyCR instanceKey) const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceCacheHelper::PartialCachingState
    {
    public:
        enum class Action
            {
            CacheFull,
            CachePartial,
            SkipCached,
            Reject
            };

    private:
        enum class SelectType
            {
            All,
            Id,
            Property
            };

    private:
        const WSQuery* m_query;
        const ECInstanceKeyMultiMap& m_fullyPersistedInstances;
        bset<ObjectId>& m_rejected;
        bset<bvector<SelectPathElement>> m_allPropertiesSelectedPaths;
        bset<bvector<SelectPathElement>> m_idOnlySelectedPaths;

    private:
        //! Check if instance is fully persited in cache and needs all properties to be selected
        bool IsFullyPersisted(ObjectInfoCR info);
        bool DoesRequireAllProperties(ObjectInfoCR info);

        static BentleyStatus BuildSelectedPaths
            (
            ECDbAdapterR dbAdapter,
            WSQueryCR query,
            bset<bvector<SelectPathElement>>& allPropertiesSelectedPathsOut,
            bset<bvector<SelectPathElement>>& m_idSelectedPaths
            );

        static BentleyStatus GetSelectPathAndType
            (
            ECDbAdapterR dbAdapter,
            Utf8StringCR mainSchemaName,
            Utf8StringCR selectStr,
            bvector<SelectPathElement>& pathOut,
            SelectType& selectTypeOut
            );

        static bool IsClassTokenPolymorphic(Utf8StringCR classToken);
        static void GetSchemaAndClassNamesFromClassToken(Utf8StringCR classToken, Utf8StringR schemaNameOut, Utf8StringR classNameOut);
        static ECClassCP GetECClassFromClassToken(ECDbAdapterR dbAdapter, Utf8StringCR mainSchemaName, Utf8StringCR classToken);
        static ECRelatedInstanceDirection GetDirection(Utf8StringCR directionString);
        static bool DoesPathMatches(const bvector<SelectPathElement>& instancePath, const bset<bvector<SelectPathElement>>& matchPaths);

    public:
        //! param[in] query -   [optional] select options will be parsed and used to indentify if all properties were selected for given class.
        //!                     Null will identify that not all properties were selected for all instances.
        PartialCachingState
            (
            ECDbAdapterR dbAdapter,
            WSQueryCR query,
            const ECInstanceKeyMultiMap& fullyPersistedInstances,
            bset<ObjectId>& rejected
            );

        //! Check what caching action should take for this instance
        Action GetAction(ObjectInfoCR info, const bvector<SelectPathElement>& path);

        //! Add Object to rejected list.
        void AddRejected(ObjectIdCR objectId);
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceCacheHelper::UpdateCachingState
    {
    private:
        bset<ObjectId> m_notFoundObjectIds;

    public:
        void AddNotFound(ObjectIdCR objectId);
        const bset<ObjectId>& GetNotFoundObjectIds() const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceCacheHelper::SelectPathElement
    {
    public:
        ECClassCP   selectedClass;
        bool        selectedClassPolymorphically;
        ECClassCP   relationshipClass;
        bool        relationshipClassPolymorphic;
        ECRelatedInstanceDirection  direction;

    public:
        SelectPathElement() :
            selectedClass(nullptr),
            selectedClassPolymorphically(false),
            relationshipClass(nullptr),
            relationshipClassPolymorphic(false),
            direction(ECRelatedInstanceDirection::Forward)
            {};

        bool operator == (const SelectPathElement& other) const
            {
            return
                selectedClass == other.selectedClass &&
                relationshipClass == other.relationshipClass &&
                direction == other.direction &&
                selectedClassPolymorphically == other.selectedClassPolymorphically &&
                relationshipClassPolymorphic == other.relationshipClassPolymorphic;
            }

        bool operator < (const SelectPathElement& other) const
            {
            if (selectedClass < other.selectedClass)
                {
                return true;
                }
            if (selectedClass > other.selectedClass)
                {
                return false;
                }
            if (relationshipClass < other.relationshipClass)
                {
                return true;
                }
            if (relationshipClass > other.relationshipClass)
                {
                return false;
                }
            if (direction < other.direction)
                {
                return true;
                }
            if (direction > other.direction)
                {
                return false;
                }
            if (selectedClassPolymorphically < other.selectedClassPolymorphically)
                {
                return true;
                }
            return relationshipClassPolymorphic < other.relationshipClassPolymorphic;
            }

        bool IsValid() const
            {
            return
                nullptr != selectedClass &&
                nullptr != relationshipClass &&
                (direction == ECRelatedInstanceDirection::Forward || direction == ECRelatedInstanceDirection::Backward);
            }
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
