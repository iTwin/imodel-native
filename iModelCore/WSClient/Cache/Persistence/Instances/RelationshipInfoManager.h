/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
struct RelationshipInfoManager : public IECDbAdapter::DeleteListener
    {
    private:
        ECDbAdapter&            m_dbAdapter;
        ECSqlStatementCache&    m_statementCache;
        HierarchyManager&       m_hierarchyManager;

        ECClassCP               m_cachedRelationshipInfoClass;

        ECSqlAdapterLoader<ECInstanceInserter>                  m_infoECInstanceInserter;
        ECSqlAdapterLoader<JsonInserter>                        m_infoInserter;
        ECSqlAdapterLoader<JsonUpdater, JsonUpdater::Options>   m_infoUpdater;

    private:
        ECInstanceKey ReadRelationshipKeyByInfo(CacheNodeKeyCR infoKey);
        ECInstanceId ReadInfoIdByRelationship(ECInstanceKeyCR relationship);

    public:
        RelationshipInfoManager
            (
            ECDbAdapter& dbAdapter,
            ECSqlStatementCache& statementCache,
            HierarchyManager& hierarchyManager
            );

        ECClassCP GetInfoClass() const;

        //! IECDbAdapter::DeleteListener
        BentleyStatus OnBeforeDelete(ECClassCR ecClass, ECInstanceId ecInstanceId, bset<ECInstanceKey>& additionalInstancesOut) override;

        //! Read existing info or create RelationshipInfo for saving
        RelationshipInfo ReadInfo(ECRelationshipClassCR relationshipClass, ECInstanceKeyCR source, ECInstanceKeyCR target);
        //! Find existing info
        RelationshipInfo FindInfo(ECInstanceKeyCR relationshipKey);
        //! Parse existing info
        RelationshipInfo ParseInfo(JsonValueCR infoJson);
        //! Save existing or new info
        BentleyStatus SaveInfo(RelationshipInfoR info);

        //! READ relationship or INSERT new cached relationship key
        CachedInstanceKey ReadOrInsertCachedRelationshipKey(ECInstanceKeyCR relationship, Utf8StringCR remoteId);
        //! Read cached relationship key
        CachedInstanceKey ReadCachedRelationshipKey(ECInstanceKeyCR relationship);

        //! Read object id for existing cached relationship
        ObjectId ReadObjectId(ECInstanceKeyCR relationship);
        //! Read ends for existing cached relationship
        BentleyStatus ReadRelationshipEnds(ECInstanceKeyCR relationship, ECInstanceKeyR sourceOut, ECInstanceKeyR targetOut);

        BentleyStatus ReadCachedRelationshipsFromHolder
            (
            ECInstanceKeyCR holder,
            ECRelationshipClassCP holderToInfoRelClass,
            bset<CachedInstanceKey>& cachedRelationshipsOut
            );

        BentleyStatus DeleteRelationshipLeavingInfo(RelationshipInfoCR info);
        BentleyStatus DeleteRelationshipOnlyWithInfo(RelationshipInfoCR info);
        BentleyStatus RemoveAllCachedRelationships();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
