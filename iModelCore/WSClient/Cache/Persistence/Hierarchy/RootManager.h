/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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
* @bsiclass                                                     Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct CacheRootKey : public ECInstanceKey
    {
    CacheRootKey() : ECInstanceKey() {}
    CacheRootKey(ECClassId ecClassId, ECInstanceId const& ecInstanceId) : 
        ECInstanceKey(ecClassId, ecInstanceId) {}
    };

typedef const CacheRootKey& CacheRootKeyCR;
typedef CacheRootKey& CacheRootKeyR;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct RootManager
    {
    private:
        ECDbAdapter&                m_dbAdapter;
        ECSqlStatementCache&        m_statementCache;

        InstanceCacheHelper&        m_instanceCacheHelper;
        HierarchyManager&           m_hierarchyManager;
        ObjectInfoManager&          m_objectInfoManager;

        ECClassCP                   m_rootClass;
        ECRelationshipClassCP       m_rootHoldingRelationshipClass;
        ECRelationshipClassCP       m_rootWeakRelationshipClass;

        ECSqlAdapterLoader<JsonInserter>                        m_rootInserter;
        ECSqlAdapterLoader<JsonUpdater, JsonUpdater::Options>   m_rootUpdater;

    private:
        ECInstanceId FindRootECInstanceId(Utf8StringCR rootName);
        CacheRootKey CreateRoot(Utf8StringCR rootName, CacheRootPersistence persistence = CacheRootPersistence::Default);
        BentleyStatus RemoveRoot(ECInstanceId rootId);
        BentleyStatus RemoveRoots(Utf8CP whereClause = nullptr, Utf8CP parameter = nullptr);
        BentleyStatus CacheFloatingInstance(ObjectIdCR objectId, ObjectInfoR info, const rapidjson::Value* instanceJson);
        void ReadRootInstance(Utf8StringCR rootName, JsonValueR rootInstanceJsonOut);
        ECRelationshipClassCP GetRelClass(bool holding);

    public:
        RootManager
            (
            ECDbAdapter& dbAdapter,
            ECSqlStatementCache& statementCache,
            InstanceCacheHelper& instanceCacheHelper,
            HierarchyManager& hierarchyManager,
            ObjectInfoManager& objectInfoManager
            );

        BentleyStatus SetupRoot(Utf8StringCR rootName, CacheRootPersistence persistence);
        BentleyStatus RenameRoot(Utf8StringCR rootName, Utf8StringCR newRootName);

        BentleyStatus SetRootSyncDate(Utf8StringCR rootName, DateTimeCR utcDateTime);
        DateTime ReadRootSyncDate(Utf8StringCR rootName);

        CacheRootKey FindRoot(Utf8StringCR rootName);
        CacheRootKey FindOrCreateRoot(Utf8StringCR rootName);

        BentleyStatus LinkInstanceToRoot
            (
            CacheRootKeyCR rootKey,
            ObjectIdCR objectId,
            bool holding = true
            );

        BentleyStatus LinkNewInstanceToRoot
            (
            CacheRootKeyCR rootKey,
            ObjectIdCR objectId,
            ObjectInfoR info,
            const rapidjson::Value* instanceProperties = nullptr,
            bool holding = true
            );

        BentleyStatus LinkExistingNodeToRoot(CacheRootKeyCR rootKey, CacheNodeKeyCR nodeKey, bool holding = true);
        BentleyStatus UnlinkNodeFromRoot(CacheRootKeyCR rootKey, CacheNodeKeyCR nodeKey);
        BentleyStatus UnlinkAllInstancesFromRoot(CacheRootKeyCR rootKey);

        BentleyStatus CopyRootRelationships(CacheNodeKeyCR fromNode, CacheNodeKeyCR toNode);

        bool IsNodeConnectedToAnyOfRoots(const bset<CacheRootKey>& rootKeys, CacheNodeKeyCR nodeKey);
        bool IsNodeInRoot(CacheRootKeyCR rootKey, CacheNodeKeyCR nodeKey);
        bool IsRootNode(ECInstanceKeyCR instanceKey);

        BentleyStatus GetNodesConnectedToRoots(const bset<CacheRootKey> roots, ECInstanceKeyMultiMap& nodesOut, uint8_t depth = UINT8_MAX);
        BentleyStatus GetNodesLinkedToRoot(Utf8StringCR rootName, ECInstanceKeyMultiMap& nodesOut);
        BentleyStatus GetNodesByPersistence(CacheRootPersistence persistence, ECInstanceKeyMultiMap& nodesOut);

        BentleyStatus RemoveRoot(Utf8StringCR rootName);
        BentleyStatus RemoveRootsByPrefix(Utf8StringCR rootPrefix);
        BentleyStatus RemoveAllRoots();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
