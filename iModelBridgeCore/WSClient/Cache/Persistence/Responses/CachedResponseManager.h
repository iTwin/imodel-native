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
        ECDbAdapter&                m_dbAdapter;
        HierarchyManager&           m_hierarchyManager;
        RelationshipInfoManager&    m_relationshipInfoManager;
        ObjectInfoManager&          m_objectInfoManager;

        ECClassCP               m_responseClass;
        ECRelationshipClassCP   m_responseToParentClass;
        ECRelationshipClassCP   m_responseToHolderClass;
        ECRelationshipClassCP   m_responseToResponsePageClass;

        ECClassCP               m_responsePageClass;
        ECRelationshipClassCP   m_responsePageToResultClass;
        ECRelationshipClassCP   m_responsePageToResultWeakClass;

        ECSqlStatementCache& m_statementCache;
        ECSqlAdapterLoader<JsonInserter> m_responseInserter;
        ECSqlAdapterLoader<JsonUpdater>  m_responseUpdater;

        bset<ECInstanceKey> m_responsesToDelete;

    private:
        BentleyStatus RelateResultInstancesToPage
            (
            ECInstanceKeyCR responseParentKey,
            CacheNodeKeyCR pageKey,
            const InstanceCacheHelper::CachedInstances& instances
            );

        BentleyStatus RelateResultRelationshipInstancesToPage
            (
            CacheNodeKeyCR pageKey,
            const InstanceCacheHelper::CachedInstances& instances
            );

        BentleyStatus MarkTemporaryInstancesAsPartial
            (
            const bset<ECInstanceId>& responseIds,
            ECRelationshipClassCP resultRelationshipClass,
            const ECInstanceKeyMultiMap& fullyPersistedInstances
            );

        BentleyStatus InsertInfo(CachedResponseInfoR info);
        CacheNodeKey SavePage(CachedResponseInfoCR info, uint64_t page, Utf8StringCR cacheTag);
        CacheNodeKey FindPage(CachedResponseInfoCR info, uint64_t page);
        bvector<CacheNodeKey> FindPages(CacheNodeKeyCR responseKey);
        CacheNodeKey InsertPage(CachedResponseInfoCR info, uint64_t page, Utf8StringCR cacheTag);
        BentleyStatus UpdatePage(ECInstanceId pageId, Utf8StringCR cacheTag);
        BentleyStatus UpdatePageCacheDate(ECInstanceId pageId);
        BentleyStatus ClearPageCacheTag(ECInstanceId pageId);
        ECSqlStatementPtr GetSelectPagePropertyStatement
            (
            ResponseKeyCR responseKey,
            uint64_t page,
            Utf8StringCR propertyName
            );

        //! When instance is of cached data, get CacheNodeKey for it.
        //! Return same if its from cache schema
        CacheNodeKey GetCacheNodeKey(ECInstanceKeyCR instanceKey);

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

        //! Convert between public and internal ResponseKey
        //! WIP06
        ResponseKey ConvertResponseKey(CachedResponseKeyCR cachedResponseKey);

        //! Read query info instance
        CachedResponseInfo ReadInfo(ResponseKeyCR responseKey);
        //! Find query info in cache
        CacheNodeKey FindInfo(ResponseKeyCR responseKey);
        //! Delete query info and any response instances that are not held anything else
        BentleyStatus DeleteInfo(ResponseKeyCR responseKey);
        //! Delete responses that were accessed before date specified
        BentleyStatus DeleteResponses(Utf8StringCR responseName, DateTimeCR accessedBeforeDateUtc, const ECInstanceKeyMultiMap& instancesToLeave);
        //! Delete responses with same name
        BentleyStatus DeleteResponses(Utf8StringCR responseName);
        //! Save query info
        BentleyStatus SaveInfo(CachedResponseInfoR info);
        //! Insert query info and update page cache date
        BentleyStatus UpdatePageCachedDate(ResponseKeyCR responseKey, uint64_t page);
        //! Insert query info and update page cache date and relate response instances
        BentleyStatus SavePage(ResponseKeyCR responseKey, uint64_t page, Utf8StringCR cacheTag, const InstanceCacheHelper::CachedInstances& instances);

        //! Read response instances. Relationships not included. 
        //! @param readCallback will be called for each successfull query, caller is responsible for extracting data.
        BentleyStatus ReadResponse
            (
            CachedResponseInfoCR info,
            ISelectProviderCR selectProvider,
            const CacheQueryHelper::ReadCallback& readCallback
            );

        //! Read response instance keys. Relationships not included.
        BentleyStatus ReadResponseInstanceKeys
            (
            CacheNodeKeyCR responseKey,
            ECInstanceKeyMultiMap& keysOut
            );

        //! Read response instance ObjectIds. Relationships not included.
        BentleyStatus ReadResponseObjectIds
            (
            CacheNodeKeyCR responseKey,
            bset<ObjectId>& objectIdsOut
            );

        //! Set response completed when it has all pages cached
        BentleyStatus SetResponseCompleted(ResponseKeyCR responseKey, bool isCompleted);

        //! Check if response has all pages cached
        bool IsResponseCompleted(ResponseKeyCR responseKey);

        //! Read response page cache tag
        Utf8String ReadResponsePageCacheTag(ResponseKeyCR responseKey, uint64_t page);
        //! Read response page cache date
        DateTime ReadResponsePageCachedDate(ResponseKeyCR responseKey, uint64_t page);

        //! Trim pages to page count by index
        BentleyStatus TrimPages(ResponseKeyCR responseKey, uint64_t maxPageIndex);

        //! Change temporary cached response statuses from Full to Partial
        BentleyStatus MarkTemporaryInstancesAsPartial
            (
            const bset<ECInstanceId>& responseIds,
            const ECInstanceKeyMultiMap& fullyPersistedNodes
            );

        //! Invalidates cache tag for responses that contain specified instance
        BentleyStatus InvalidateResponsePagesContainingInstance(CachedInstanceKeyCR cachedKey);
    };

typedef const CachedResponseManager& QueryManagerCR;
typedef CachedResponseManager& QueryManagerR;

END_BENTLEY_WEBSERVICES_NAMESPACE
