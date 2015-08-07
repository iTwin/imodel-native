/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Instances/InstanceCacheHelper.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

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

        ECSqlAdapterCache<JsonInserter> m_inserters;
        ECSqlAdapterCache<JsonUpdater> m_updaters;

    private:
        BentleyStatus CacheInstance
            (
            const WSObjectsReader::Instance& instance,
            const bvector<SelectPathElement>& path,
            CachedInstances& cachedInstancesInOut,
            PartialCachingState* partialCachingState,
            UpdateCachingState* updateCachingState,
            ECInstanceKey* cachedInstanceOut,
            ICancellationTokenPtr cancellationToken
            );

        BentleyStatus CacheRelationshipInstance
            (
            const WSObjectsReader::RelationshipInstance& relationshipInstance,
            ECInstanceKeyCR relatedInstanceA,
            ECInstanceKeyCR relatedInstanceB,
            CachedInstances& cachedInstancesInOut
            );

        BentleyStatus CacheInstance(ObjectInfoR infoInOut, RapidJsonValueCR properties);

    public:
        InstanceCacheHelper
            (
            ECDbAdapter& dbAdapter,
            HierarchyManager& hierarchyManager,
            ObjectInfoManager& objectInfoManager,
            RelationshipInfoManager& relationshipInfoManager
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
            ICancellationTokenPtr cancellationToken = nullptr
            );

        //! Utility for directly saving new instance to cache
        BentleyStatus CacheNewInstance(ObjectInfoR infoInOut, RapidJsonValueCR properties);
        //! Utility for directly saving existing instance to cache
        BentleyStatus CacheExistingInstance(ObjectInfoCR info, RapidJsonValueCR properties);
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceCacheHelper::CachedInstances
    {
    private:
        bset<ECInstanceKey>           m_cachedInstances;
        bset<CachedRelationshipKey>   m_cachedRelationships;
        bmap<ObjectId, ECInstanceKey> m_cachedInstancesByObjectId;

    public:
        void AddInstance(ObjectIdCR objectId, ECInstanceKeyCR cachedInstance);
        void AddRelationshipInstance(ObjectIdCR objectId, const CachedRelationshipKey& relationshipInfo);

        bool HasCachedInstance(ObjectIdCR objectId) const;
        ECInstanceKey GetCachedInstance(ObjectIdCR objectId) const;

        const bset<ECInstanceKey>& GetCachedInstances() const;
        bset<ObjectId> GetCachedInstanceObjectIds() const;
        const bmap<ObjectId, ECInstanceKey>& GetCachedInstancesByObjectId() const;
        const bset<CachedRelationshipKey> GetCachedRelationshipInfos() const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceCacheHelper::PartialCachingState
    {
    private:
        const WSQuery* m_query;
        const ECInstanceKeyMultiMap& m_fullyPersistedInstances;
        bset<ObjectId>& m_rejected;

        bset<bvector<SelectPathElement>> m_allPropertiesSelectedPaths;

    private:
        //! Check if instance is fully persited in cache and needs all properties to be selected
        bool IsFullyPersisted(ObjectInfoCR info);

        static BentleyStatus BuildAllPropertiesSelectedPaths
            (
            ECDbAdapterR dbAdapter,
            WSQueryCR query,
            bset<bvector<SelectPathElement>>& allPropertiesSelectedPathsOut
            );

        static bool IsClassTokenPolymorphic(Utf8StringCR classToken);
        static void GetSchemaAndClassNamesFromClassToken(Utf8StringCR classToken, Utf8StringR schemaNameOut, Utf8StringR classNameOut);
        static ECClassCP GetECClassFromClassToken(ECDbAdapterR dbAdapter, Utf8StringCR mainSchemaName, Utf8StringCR classToken);
        static ECRelatedInstanceDirection GetDirection(Utf8StringCR directionString);

    public:
        //! param[in] query -   [optional] select options will be parsed and used to indentify if all properties were selected for given class.
        //!                     Null will identify that not all properties were selected for all instances.
        PartialCachingState
            (
            ECDbAdapterR dbAdapter,
            const WSQuery* query,
            const ECInstanceKeyMultiMap& fullyPersistedInstances,
            bset<ObjectId>& rejected
            );

        //! Check if instance caching should be rejected.
        bool ShouldReject(ObjectInfoCR info, const bvector<SelectPathElement>& path);

        //! Checks if all properties were selected for instance using supplied WSQuery. Null WSQuery is treated as not all properties selected.
        //! @param[in] instancePath - instance path in response
        bool AreAllPropertiesSelected(const bvector<SelectPathElement>& instancePath);

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
