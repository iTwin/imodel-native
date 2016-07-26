/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Instances/ObjectInfoManager.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>
#include "ObjectInfo.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

struct HierarchyManager;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ObjectInfoManager : public IECDbAdapter::DeleteListener
    {
    private:
        ECDbAdapter&            m_dbAdapter;
        ECSqlStatementCache&    m_statementCache;
        HierarchyManager&       m_hierarchyManager;

        ECClassCP               m_infoClass;

        ECSqlAdapterLoader<JsonInserter>    m_infoInserter;
        ECSqlAdapterLoader<JsonUpdater>     m_infoUpdater;

    public:
        ObjectInfoManager
            (
            ECDbAdapter& dbAdapter,
            ECSqlStatementCache& statementCache,
            HierarchyManager& hierarchyManager
            );
        ~ObjectInfoManager();

        ECClassCP GetInfoClass() const;

        //! IECDbAdapter::DeleteListener
        BentleyStatus OnBeforeDelete(ECClassCR ecClass, ECInstanceId ecInstanceId, bset<ECInstanceKey>& additionalInstancesOut) override;

        BentleyStatus InsertInfo(ObjectInfoR info);
        BentleyStatus UpdateInfo(ObjectInfoCR info);

        ObjectInfo ReadInfo(ObjectIdCR objectId);
        ObjectInfo ReadInfo(ECClassCR ecClass, Utf8StringCR remoteId);
        ObjectInfo ReadInfo(ECInstanceKeyCR instance);
        ObjectInfo ReadInfo(JsonValueCR infoJson);

        ECInstanceKey FindCachedInstance(ObjectIdCR objectId);
        ECInstanceKey FindCachedInstance(ECClassCP ecClass, Utf8StringCR remoteId);
        ObjectId FindCachedInstance(ECInstanceKeyCR instance);

        CachedObjectInfoKey ReadInfoKey(ECInstanceKeyCR instanceKey);
        CachedInstanceKey ReadCachedInstanceKey(ECInstanceKeyCR instanceKey);
        CachedInstanceKey ReadCachedInstanceKey(ObjectIdCR objectId);
        CachedInstanceKey ReadCachedInstanceKey(CacheNodeKeyCR relatedKey, ECRelationshipClassCR relClass);
        CachedInstanceKey ReadCachedInstanceKey(CacheNodeKeyCR infoKey);
        BentleyStatus ReadCachedInstanceKeys(CacheNodeKeyCR relatedKey, ECRelationshipClassCR relClass, ECInstanceKeyMultiMap& instanceKeysOut);
        BentleyStatus ReadCachedInstanceKeys(CacheNodeKeyCR relatedKey, ECRelationshipClassCR relClass, bset<CachedInstanceKey>& instanceKeysOut);
        BentleyStatus ReadCachedInstanceKeys(const ECInstanceKeyMultiMap& infoKeys, ECInstanceKeyMultiMap& instanceKeysOut);
        BentleyStatus ReadCachedInstanceIds(CacheNodeKeyCR relatedKey, ECRelationshipClassCR relClass, bset<ObjectId>& idsOut);

        BentleyStatus DeleteInstanceLeavingInfo(ObjectInfoR info);
        BentleyStatus RemoveAllCachedInstances();

        //! Return cached instance key when CachedObjectInfoKey is passed. Return same if else 
        ECInstanceKey ConvertToInstanceKey(ECInstanceKeyCR instanceKey);
    };

typedef const ObjectInfoManager& ObjectInfoManagerCR;
typedef ObjectInfoManager& ObjectInfoManagerR;

END_BENTLEY_WEBSERVICES_NAMESPACE
