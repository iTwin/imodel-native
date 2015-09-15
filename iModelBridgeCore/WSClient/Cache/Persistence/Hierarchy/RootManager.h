/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Hierarchy/RootManager.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <ECDb/ECDbApi.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE

#include <WebServices/Cache/Persistence/DataSourceCacheCommon.h>
#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Client/ObjectId.h>

#include "../Hierarchy/HierarchyManager.h"
#include "../Instances/InstanceCacheHelper.h"
#include "../Instances/NavigationBaseManager.h"
#include "../Instances/ObjectInfoManager.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct RootManager
    {
    private:
        ECDbAdapter&                m_dbAdapter;
        ECSqlStatementCache*        m_statementCache;

        InstanceCacheHelper&        m_instanceHelper;
        HierarchyManager&           m_hierarchyManager;
        ObjectInfoManager&          m_objectInfoManager;

        ECClassCP                   m_rootClass;
        ECRelationshipClassCP       m_rootHoldingRelationshipClass;
        ECRelationshipClassCP       m_rootWeakRelationshipClass;

        ECSqlAdapterLoader<JsonInserter>    m_rootInserter;
        ECSqlAdapterLoader<JsonUpdater>     m_rootUpdater;

    private:
        ECInstanceId FindRootECInstanceId(Utf8StringCR rootName);
        ECInstanceKey CreateRoot(Utf8StringCR rootName, CacheRootPersistence persistence = CacheRootPersistence::Default);
        BentleyStatus RemoveRoot(ECInstanceId rootId);
        BentleyStatus RemoveRoots(Utf8CP whereClause = nullptr);
        BentleyStatus CacheFloatingInstance(ObjectIdCR objectId, ObjectInfoR info, const rapidjson::Value* instanceJson);
        void ReadRootInstance(Utf8StringCR rootName, JsonValueR rootInstanceJsonOut);

    public:
        RootManager
            (
            ECDbAdapter& dbAdapter,
            ECSqlStatementCache& statementCache,
            InstanceCacheHelper&  instanceHelper,
            HierarchyManager& hierarchyManager,
            ObjectInfoManager& objectInfoManager
            );

        ECRelationshipClassCP GetRootRelationshipClass() const;

        BentleyStatus SetupRoot(Utf8StringCR rootName, CacheRootPersistence persistence);
        BentleyStatus RenameRoot(Utf8StringCR rootName, Utf8StringCR newRootName);

        BentleyStatus SetRootSyncDate(Utf8StringCR rootName, DateTimeCR utcDateTime);
        DateTime ReadRootSyncDate(Utf8StringCR rootName);

        BentleyStatus GetInstancesByPersistence(CacheRootPersistence persistence, ECInstanceKeyMultiMap& instancesOut);

        ECInstanceKey FindRoot(Utf8StringCR rootName);
        ECInstanceKey FindOrCreateRoot(Utf8StringCR rootName);

        BentleyStatus LinkInstanceToRoot
            (
            Utf8StringCR rootName,
            ObjectIdCR objectId,
            bool holding = true
            );

        BentleyStatus LinkNewInstanceToRoot
            (
            Utf8StringCR rootName,
            ObjectIdCR objectId,
            ObjectInfoR info,
            const rapidjson::Value* optionalInstanceJson,
            bool holding = true
            );

        BentleyStatus LinkExistingInstanceToRoot(Utf8StringCR rootName, ECInstanceKeyCR instance, bool holding = true);
        BentleyStatus LinkExistingInstancesToRoot(Utf8StringCR rootName, const bset<ECInstanceKey>& instances, bool holding = true);

        BentleyStatus UnlinkInstanceFromRoot(Utf8StringCR rootName, ECInstanceKeyCR instanceId);
        BentleyStatus UnlinkAllInstancesFromRoot(Utf8StringCR rootName);

        BentleyStatus CopyRootRelationships(ECInstanceKeyCR fromInstance, ECInstanceKeyCR toInstance);

        bool IsInstanceConnectedToAnyOfRoots(ECInstanceKeyCR instance, const bset<ECInstanceId>& rootIds);
        bool IsInstanceInRoot(ECInstanceKeyCR instance, ECInstanceId rootId);
        BentleyStatus GetInstancesConnectedToRoots(const bset<ECInstanceId> roots, ECInstanceKeyMultiMap& instancesOut, uint8_t depth = UINT8_MAX);

        BentleyStatus RemoveRoot(Utf8StringCR rootName);
        BentleyStatus RemoveRootsByPrefix(Utf8StringCR rootPrefix);
        BentleyStatus RemoveAllRoots();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
