/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Instances/RelationshipInfoManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>

#include "../Hierarchy/HierarchyManager.h"
#include "RelationshipInfo.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CachedRelationshipKey
    {
    private:
        ECInstanceKey m_infoKey;
        ECInstanceKey m_relationshipKey;

    public:
        CachedRelationshipKey();
        CachedRelationshipKey(ECInstanceKeyCR infoKey, ECInstanceKey relationshipKey);

        bool operator < (const CachedRelationshipKey& other) const;

        bool IsInCache() const;
        ECInstanceKeyCR GetInfoKey() const;
        ECInstanceKeyCR GetRelationshipKey() const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelationshipInfoManager : public IDeleteHandler
    {
    private:
        ECDbAdapter*            m_dbAdapter;
        ECSqlStatementCache*    m_statementCache;
        HierarchyManager*       m_hierarchyManager;

        ECClassCP               m_cachedRelationshipInfoClass;

        ECSqlAdapterLoader<ECInstanceInserter>  m_infoECInstanceInserter;
        ECSqlAdapterLoader<JsonInserter>        m_infoInserter;
        ECSqlAdapterLoader<JsonUpdater>         m_infoUpdater;

    private:
        BentleyStatus DeleteCachedRelationship(const CachedRelationshipKey& relationship);
        ECInstanceKey ReadRelationshipKeyByInfo(ECInstanceKeyCR infoKey);
        ECInstanceId ReadInfoIdByRelationship(ECInstanceKeyCR relationshipDesc);

    public:
        RelationshipInfoManager
            (
            ECDbAdapter& dbAdapter,
            ECSqlStatementCache& statementCache,
            HierarchyManager& hierarchyManager
            );

        ECClassCP GetInfoClass() const;

        //! IDeleteHandler
        BentleyStatus OnBeforeDelete(ECClassCR ecClass, ECInstanceId ecInstanceId) override;
        BentleyStatus OnAfterDelete(bset<ECInstanceKey>& instancesToDeleteOut) override;

        //! Read existing info or create RelationshipInfo for saving
        RelationshipInfo ReadInfo(ECRelationshipClassCR relationshipClass, ECInstanceKeyCR source, ECInstanceKeyCR target);
        //! Find existing info
        RelationshipInfo FindInfo(ECInstanceKeyCR relationshipKey);
        //! Parse existing info
        RelationshipInfo ParseInfo(JsonValueCR infoJson);
        //! Save existing or new info
        BentleyStatus SaveInfo(RelationshipInfoR info);

        //! READ relationship or INSERT new cached relationship key
        CachedRelationshipKey ReadCachedRelationshipKey(ECInstanceKeyCR relationship, Utf8StringCR remoteId);

        //! Read object id for existing cached relationship
        ObjectId ReadObjectId(ECInstanceKeyCR relationship);
        //! Read ends for existing cached relationship
        BentleyStatus ReadRelationshipEnds(ECInstanceKeyCR relationship, ECInstanceKeyR sourceOut, ECInstanceKeyR targetOut);

        BentleyStatus RelateCachedRelationshipsToHolder
            (
            ECInstanceKeyCR holder,
            ECRelationshipClassCP holderToInfoRelClass,
            const bset<CachedRelationshipKey>& cachedRelationships
            );

        BentleyStatus ReadCachedRelationshipsFromHolder
            (
            ECInstanceKeyCR holder,
            ECRelationshipClassCP holderToInfoRelClass,
            bset<CachedRelationshipKey>& cachedRelationshipsOut
            );

        BentleyStatus RemoveCachedRelationshipsFromHolder
            (
            ECInstanceKeyCR holder,
            ECRelationshipClassCP holderToInfoRelClass,
            const bset<CachedRelationshipKey>& cachedRelationshipsToRemove
            );

        BentleyStatus DeleteRelationshipLeavingInfo(RelationshipInfoR info);
        BentleyStatus RemoveAllCachedRelationships();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
