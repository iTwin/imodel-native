/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Responses/CachedResponseManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Persistence/CacheQueryHelper.h>
#include <WebServices/Cache/Persistence/CachedResponseKey.h>
#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>

#include "CachedResponseInfo.h"
#include "../Hierarchy/HierarchyManager.h"
#include "../Instances/RelationshipInfoManager.h"
#include "../Instances/ObjectInfoManager.h"
#include "../Instances/InstanceCacheHelper.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CachedResponseManager : public IDeleteHandler
    {
    private:
        ECDbAdapter*                m_dbAdapter;
        HierarchyManager*           m_hierarchyManager;
        RelationshipInfoManager*    m_relationshipInfoManager;
        ObjectInfoManager*          m_objectInfoManager;

        ECClassCP               m_responseClass;
        ECRelationshipClassCP   m_responseToParentClass;
        ECRelationshipClassCP   m_responseToHolderClass;
        ECRelationshipClassCP   m_responseToResultClass;
        ECRelationshipClassCP   m_responseToResultWeakClass;
        ECRelationshipClassCP   m_responseToRelInfoClass;

        ECSqlStatementCache* m_statementCache;
        ECSqlAdapterLoader<JsonInserter> m_responseInserter;
        ECSqlAdapterLoader<JsonUpdater>  m_responseUpdater;

        bset<ECInstanceKey> m_responsesToDelete;

    private:
        BentleyStatus RelateResultInstancesToCachedResponseInfo
            (
            CachedResponseInfoCR queryKey,
            const InstanceCacheHelper::CachedInstances& cachedInstances
            );

        BentleyStatus RelateResultRelationshipInstancesToCachedResponseInfo
            (
            ECInstanceKeyCR queryKey,
            const InstanceCacheHelper::CachedInstances& cachedInstances
            );

        BentleyStatus ReadResponseObjectIds
            (
            ECInstanceId cachedQueryInfoId,
            ECRelationshipClassCP relationshipClass,
            bset<ObjectId>& objectIdsOut
            );

        BentleyStatus MarkTemporaryInstancesAsPartial
            (
            const bset<ECInstanceId>& cachedQueryInfoKeys,
            ECRelationshipClassCP resultRelationshipClass,
            const ECInstanceKeyMultiMap& fullyPersistedInstances
            );

    public:
        CachedResponseManager
            (
            ECDbAdapter& dbAdapter,
            ECSqlStatementCache& statementCache,
            HierarchyManager& hierarchyManager,
            RelationshipInfoManager& relationshipInfoManager,
            ObjectInfoManager& objectInfoManager
            );

        //! IDeleteHandler
        BentleyStatus OnBeforeDelete(ECClassCR ecClass, ECInstanceId ecInstanceId) override;
        BentleyStatus OnAfterDelete(bset<ECInstanceKey>& instancesToDeleteOut) override;

        //! Read query info instance
        CachedResponseInfo ReadInfo(CachedResponseKeyCR key);
        //! Find query info in cache
        ECInstanceKey FindInfo(ECInstanceKeyCR parent, Utf8StringCR queryName);
        //! Delete query info and any response instances that are not held anything else
        BentleyStatus DeleteInfo(ECInstanceKeyCR parent, Utf8StringCR queryName);
        //! Delete responses that were accessed before date specified
        BentleyStatus DeleteResponses(Utf8StringCR name, DateTimeCR accessedBeforeDateUtc, const ECInstanceKeyMultiMap& instancesToLeave);
        //! Delete responses with same name
        BentleyStatus DeleteResponses(Utf8StringCR name);
        //! Save query info instance
        BentleyStatus SaveInfo(CachedResponseInfoR queryInfo);
        //! Save query info instance and relate response instances
        BentleyStatus SaveInfo(CachedResponseInfoR queryInfo, const InstanceCacheHelper::CachedInstances& cachedInstances);
        //! Read response instances. Callback will be called for each successfull query, caller is responsible for extracting data.
        BentleyStatus ReadResponse
            (
            CachedResponseInfoCR queryInfo,
            ISelectProviderCR selectProvider,
            const CacheQueryHelper::ReadCallback& readCallback
            );

        //! Read keys from response instances.
        BentleyStatus ReadResponseInstanceKeys
            (
            ECInstanceKeyCR cachedQueryInfoDesc,
            ECInstanceKeyMultiMap& keysOut
            );

        //! Read ObjectIds from response instances
        BentleyStatus ReadResponseObjectIds
            (
            ECInstanceId cachedResponseInfoId,
            bset<ObjectId>& objectIdsOut
            );

        //! Read query results cache tag
        Utf8String ReadResponseCacheTag(ECInstanceKeyCR parent, Utf8StringCR queryName);
        //! Read query results cached date
        DateTime ReadResponseCachedDate(ECInstanceKeyCR parent, Utf8StringCR queryName);
        //! Change temporary cached response statuses from Full to Partial
        BentleyStatus MarkTemporaryInstancesAsPartial
            (
            const bset<ECInstanceId>& cachedQueryInfoIds,
            const ECInstanceKeyMultiMap& fullyPersistedInstances
            );

        //! Invalidates cache tag for responses that contain specified instance
        BentleyStatus InvalidateResponsesContainingInstance(ECInstanceKeyCR instanceKey);
    };

typedef const CachedResponseManager& QueryManagerCR;
typedef CachedResponseManager& QueryManagerR;

END_BENTLEY_WEBSERVICES_NAMESPACE
