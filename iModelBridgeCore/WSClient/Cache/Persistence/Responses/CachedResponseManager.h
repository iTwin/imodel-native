/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Responses/CachedResponseManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
        ECRelationshipClassCP   m_responseToResponsePageClass;
        ECRelationshipClassCP   m_responseToAdditionalInstance;

        ECClassCP               m_responsePageClass;
        ECRelationshipClassCP   m_responsePageToResultClass;
        ECRelationshipClassCP   m_responsePageToResultWeakClass;
        ECRelationshipClassCP   m_responsePageToRelInfoClass;

        ECSqlStatementCache* m_statementCache;
        ECSqlAdapterLoader<JsonInserter> m_responseInserter;
        ECSqlAdapterLoader<JsonUpdater>  m_responseUpdater;

        bset<ECInstanceKey> m_responsesToDelete;

    private:
        BentleyStatus RelateResultInstancesToPage
            (
            ECInstanceKeyCR responseParentKey,
            ECInstanceKeyCR pageKey,
            const InstanceCacheHelper::CachedInstances& instances
            );

        BentleyStatus RelateResultRelationshipInstancesToPage
            (
            ECInstanceKeyCR pageKey,
            const InstanceCacheHelper::CachedInstances& instances
            );

        BentleyStatus ReadTargetObjectIds
            (
            ECInstanceKeyCR sourceKey,
            ECRelationshipClassCP relationshipClass,
            bset<ObjectId>& targetIdsOut
            );

        BentleyStatus MarkTemporaryInstancesAsPartial
            (
            const bset<ECInstanceId>& responseKeys,
            ECRelationshipClassCP resultRelationshipClass,
            const ECInstanceKeyMultiMap& fullyPersistedInstances
            );

        BentleyStatus InsertInfo(CachedResponseInfoR info);
        ECInstanceKey SavePage(CachedResponseInfoCR info, uint64_t page, Utf8StringCR cacheTag);
        ECInstanceKey FindPage(CachedResponseInfoCR info, uint64_t page);
        bvector<ECInstanceKey> FindPages(ECInstanceKeyCR responseKey);
        ECInstanceKey InsertPage(CachedResponseInfoCR info, uint64_t page, Utf8StringCR cacheTag);
        BentleyStatus UpdatePage(ECInstanceId pageId, Utf8StringCR cacheTag);
        BentleyStatus UpdatePageCacheDate(ECInstanceId pageId);
        BentleyStatus ClearPageCacheTag(ECInstanceId pageId);
        ECSqlStatementPtr GetSelectPagePropertyStatement
            (
            CachedResponseKeyCR responseKey,
            uint64_t page,
            Utf8StringCR propertyName
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
        CachedResponseInfo ReadInfo(CachedResponseKeyCR responseKey);
        //! Find query info in cache
        ECInstanceKey FindInfo(CachedResponseKeyCR responseKey);
        //! Delete query info and any response instances that are not held anything else
        BentleyStatus DeleteInfo(CachedResponseKeyCR responseKey);
        //! Delete responses that were accessed before date specified
        BentleyStatus DeleteResponses(Utf8StringCR responseName, DateTimeCR accessedBeforeDateUtc, const ECInstanceKeyMultiMap& instancesToLeave);
        //! Delete responses with same name
        BentleyStatus DeleteResponses(Utf8StringCR responseName);
        //! Save query info
        BentleyStatus SaveInfo(CachedResponseInfoR info);
        //! Insert query info and update page cache date
        BentleyStatus UpdatePageCachedDate(CachedResponseKeyCR responseKey, uint64_t page);
        //! Insert query info and update page cache date and relate response instances
        BentleyStatus SavePage(CachedResponseKeyCR responseKey, uint64_t page, Utf8StringCR cacheTag, const InstanceCacheHelper::CachedInstances& instances);
        
        //! Add additional instance to response that will not be managed with it but will be returned as one of cached
        BentleyStatus AddAdditionalInstance(CachedResponseKeyCR responseKey, ECInstanceKeyCR instanceKey);
        //! Remove additional instance
        BentleyStatus RemoveAdditionalInstance(CachedResponseKeyCR responseKey, ECInstanceKeyCR instanceKey);
        //! Remove additional instance from any responses, will keep responses intact
        BentleyStatus RemoveAdditionalInstance(ECInstanceKeyCR instanceKey);

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
            ECInstanceKeyCR responseKey,
            ECInstanceKeyMultiMap& keysOut
            );

        //! Read response instance ObjectIds. Relationships not included.
        BentleyStatus ReadResponseObjectIds
            (
            ECInstanceKeyCR responseKey,
            bset<ObjectId>& objectIdsOut
            );

        //! Set response completed when it has all pages cached
        BentleyStatus SetResponseCompleted(CachedResponseKeyCR responseKey, bool isCompleted);

        //! Check if response has all pages cached
        bool IsResponseCompleted(CachedResponseKeyCR responseKey);

        //! Read response page cache tag
        Utf8String ReadResponsePageCacheTag(CachedResponseKeyCR responseKey, uint64_t page);
        //! Read response page cache date
        DateTime ReadResponsePageCachedDate(CachedResponseKeyCR responseKey, uint64_t page);

        //! Trim pages to page count by index
        BentleyStatus TrimPages(CachedResponseKeyCR responseKey, uint64_t maxPageIndex);

        //! Change temporary cached response statuses from Full to Partial
        BentleyStatus MarkTemporaryInstancesAsPartial
            (
            const bset<ECInstanceId>& responseIds,
            const ECInstanceKeyMultiMap& fullyPersistedInstances
            );

        //! Invalidates cache tag for responses that contain specified instance
        BentleyStatus InvalidateResponsePagesContainingInstance(ECInstanceKeyCR instanceKey);
    };

typedef const CachedResponseManager& QueryManagerCR;
typedef CachedResponseManager& QueryManagerR;

END_BENTLEY_WEBSERVICES_NAMESPACE
