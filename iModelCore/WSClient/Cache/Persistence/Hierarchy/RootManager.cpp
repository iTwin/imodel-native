/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "RootManager.h"

#include <WebServices/Cache/Util/ECDbHelper.h>
#include "../Core/CacheSchema.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RootManager::RootManager
(
ECDbAdapter& dbAdapter,
WebServices::ECSqlStatementCache& statementCache,
InstanceCacheHelper& instanceCacheHelper,
HierarchyManager& hierarchyManager,
ObjectInfoManager& objectInfoManager
) :
m_dbAdapter(dbAdapter),
m_statementCache(statementCache),

m_instanceCacheHelper(instanceCacheHelper),
m_hierarchyManager(hierarchyManager),
m_objectInfoManager(objectInfoManager),

m_rootClass(m_dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_Root)),
m_rootHoldingRelationshipClass(m_dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_RootToNode)),
m_rootWeakRelationshipClass(m_dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_RootToNodeWeak)),

m_rootInserter(dbAdapter.GetECDb(), *m_rootClass),
m_rootUpdater(m_dbAdapter.GetECDb(), *m_rootClass, ECSqlUpdater_Options_IgnoreSystemAndFailReadOnlyProperties)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool RootManager::IsRootNode(ECInstanceKeyCR instanceKey)
    {
    return m_rootClass->GetId() == instanceKey.GetClassId();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP RootManager::GetRelClass(bool holding)
    {
    if (holding)
        return m_rootHoldingRelationshipClass;
    return m_rootWeakRelationshipClass;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceId RootManager::FindRootECInstanceId(Utf8StringCR rootName)
    {
    auto statement = m_statementCache.GetPreparedStatement("RootManager:FindRootECInstanceId", [&]
        {
        return
            "SELECT ECInstanceId "
            "FROM " ECSql_Root " "
            "WHERE [" CLASS_Root_PROPERTY_Name "] = ? "
            "LIMIT 1 ";
        });

    statement->BindText(1, rootName.c_str(), IECSqlBinder::MakeCopy::No);
    statement->Step();

    return statement->GetValueId<ECInstanceId>(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CacheRootKey RootManager::FindRoot(Utf8StringCR rootName)
    {
    return CacheRootKey(m_rootClass->GetId(), FindRootECInstanceId(rootName));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheRootKey RootManager::FindOrCreateRoot(Utf8StringCR rootName)
    {
    CacheRootKey root = FindRoot(rootName);
    if (!root.IsValid())
        {
        root = CreateRoot(rootName);
        }
    return root;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RootManager::ReadRootInstance(Utf8StringCR rootName, JsonValueR rootInstanceJsonOut)
    {
    auto statement = m_statementCache.GetPreparedStatement("RootManager:SetupRoot", [&]
        {
        return
            "SELECT * "
            "FROM " ECSql_Root " "
            "WHERE [" CLASS_Root_PROPERTY_Name "] = ? "
            "LIMIT 1 ";
        });
    statement->BindText(1, rootName.c_str(), IECSqlBinder::MakeCopy::No);

    m_dbAdapter.ExtractJsonInstanceFromStatement(*statement, m_rootClass, rootInstanceJsonOut);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::SetupRoot(Utf8StringCR rootName, CacheRootPersistence persistence)
    {
    Json::Value root;
    ReadRootInstance(rootName, root);

    if (root.isNull())
        {
        return CreateRoot(rootName, persistence).IsValid() ? SUCCESS : ERROR;
        }

    root[CLASS_Root_PROPERTY_Persistence] = static_cast<int> (persistence);
    if (BE_SQLITE_OK != m_rootUpdater.Get().Update(ECDbHelper::ECInstanceIdFromJsonInstance(root), root))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::RenameRoot(Utf8StringCR rootName, Utf8StringCR newRootName)
    {
    if (FindRoot(newRootName).IsValid())
        {
        return ERROR;
        }

    Json::Value root;
    ReadRootInstance(rootName, root);

    if (root.isNull())
        {
        return CreateRoot(newRootName).IsValid() ? SUCCESS : ERROR;
        }

    root[CLASS_Root_PROPERTY_Name] = newRootName;
    if (BE_SQLITE_OK != m_rootUpdater.Get().Update(ECDbHelper::ECInstanceIdFromJsonInstance(root), root))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::SetRootSyncDate(Utf8StringCR rootName, DateTimeCR utcDateTime)
    {
    if (!utcDateTime.IsValid() || utcDateTime.GetInfo().GetKind() != DateTime::Kind::Utc)
        {
        BeAssert(false);
        return ERROR;
        }

    if (!FindOrCreateRoot(rootName).IsValid())
        {
        return ERROR;
        };

    Json::Value root;
    ReadRootInstance(rootName, root);

    root[CLASS_Root_PROPERTY_SyncDate] = utcDateTime.ToString();
    if (BE_SQLITE_OK != m_rootUpdater.Get().Update(ECDbHelper::ECInstanceIdFromJsonInstance(root), root))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime RootManager::ReadRootSyncDate(Utf8StringCR rootName)
    {
    auto statement = m_statementCache.GetPreparedStatement("RootManager:ReadRootSyncDate", [&]
        {
        return
            "SELECT [" CLASS_Root_PROPERTY_SyncDate "]"
            "FROM " ECSql_Root " "
            "WHERE [" CLASS_Root_PROPERTY_Name "] = ? "
            "LIMIT 1 ";
        });

    statement->BindText(1, rootName.c_str(), IECSqlBinder::MakeCopy::No);
    statement->Step();

    if (statement->IsValueNull(0))
        {
        return DateTime();
        }
    return statement->GetValueDateTime(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::GetNodesByPersistence(CacheRootPersistence persistence, ECInstanceKeyMultiMap& nodesOut)
    {
    auto statement = m_statementCache.GetPreparedStatement("RootManager:GetNodesByPersistence", [&]
        {
        return
            "SELECT ECInstanceId "
            "FROM " ECSql_Root " "
            "WHERE [" CLASS_Root_PROPERTY_Persistence "] = ? ";
        });

    statement->BindInt(1, static_cast<int>(persistence));

    ECInstanceKeyMultiMap rootKeys;
    ECInstanceFinder::FindOptions findOptions
        (
        ECInstanceFinder::RelatedDirection::RelatedDirection_HeldChildren |
        ECInstanceFinder::RelatedDirection::RelatedDirection_EmbeddedChildren,
        UINT8_MAX
        );

    if (SUCCESS != m_dbAdapter.ExtractECInstanceKeyMultiMapFromStatement(*statement, 0, m_rootClass->GetId(), rootKeys) ||
        SUCCESS != m_dbAdapter.GetECInstanceFinder().FindInstances(nodesOut, rootKeys, findOptions))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::LinkInstanceToRoot
(
CacheRootKeyCR rootKey,
ObjectIdCR objectId,
bool holding
)
    {
    ObjectInfo info = m_objectInfoManager.ReadInfo(objectId);
    if (SUCCESS != CacheFloatingInstance(objectId, info, nullptr))
        {
        return ERROR;
        }
    if (SUCCESS != LinkExistingNodeToRoot(rootKey, info.GetInfoKey(), holding))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::LinkNewInstanceToRoot
(
CacheRootKeyCR rootKey,
ObjectIdCR objectId,
ObjectInfoR info,
const rapidjson::Value* optionalInstanceJson,
bool holding
)
    {
    if (SUCCESS != CacheFloatingInstance(objectId, info, optionalInstanceJson))
        {
        return ERROR;
        }
    if (SUCCESS != LinkExistingNodeToRoot(rootKey, info.GetInfoKey(), holding))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::CacheFloatingInstance
(
ObjectIdCR objectId,
ObjectInfoR info,
const rapidjson::Value* instanceJson
)
    {
    if (!info.IsInCache())
        {
        const rapidjson::Value* instanceJsonToSave = instanceJson;

        rapidjson::Value dummyInstanceJson(rapidjson::kObjectType);
        if (nullptr == instanceJsonToSave)
            {
            instanceJsonToSave = &dummyInstanceJson;
            }

        info.SetRemoteId(objectId.remoteId);
        if (SUCCESS != m_instanceCacheHelper.CacheInstance(info, *instanceJsonToSave))
            {
            return ERROR;
            }
        }
    else if (nullptr != instanceJson)
        {
        if (SUCCESS != m_instanceCacheHelper.CacheInstance(info, *instanceJson))
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::LinkExistingNodeToRoot(CacheRootKeyCR rootKey, CacheNodeKeyCR nodeKey, bool holding)
    {
    if (!m_hierarchyManager.RelateInstances(rootKey, nodeKey, GetRelClass(holding)).IsValid())
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::UnlinkNodeFromRoot(CacheRootKeyCR rootKey, CacheNodeKeyCR nodeKey)
    {
    if (!rootKey.IsValid() || !nodeKey.IsValid())
        {
        return SUCCESS;
        }
    if (SUCCESS != m_hierarchyManager.RemoveChildFromParent(rootKey, nodeKey, m_rootHoldingRelationshipClass))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::UnlinkAllInstancesFromRoot(CacheRootKeyCR rootKey)
    {
    if (!rootKey.IsValid())
        {
        return SUCCESS;
        }
    if (SUCCESS != m_hierarchyManager.RemoveAllChildrenFromParent(rootKey, m_rootHoldingRelationshipClass))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::CopyRootRelationships(CacheNodeKeyCR fromNode, CacheNodeKeyCR toNode)
    {
    bvector<ECInstanceId> roots;
    if (SUCCESS != m_dbAdapter.GetRelatedSourceIds(m_rootHoldingRelationshipClass, m_rootClass, roots, fromNode))
        {
        return ERROR;
        }

    for (ECInstanceId rootECId : roots)
        {
        if (!m_dbAdapter.RelateInstances(m_rootHoldingRelationshipClass, ECInstanceKey(m_rootClass->GetId(), rootECId), toNode).IsValid())
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool RootManager::IsNodeConnectedToAnyOfRoots(const bset<CacheRootKey>& rootKeys, CacheNodeKeyCR nodeKey)
    {
    if (!nodeKey.IsValid())
        {
        return false;
        }

    ECInstanceKeyMultiMap foundInstances;

    ECInstanceKeyMultiMap seedInstances;
    seedInstances.insert(ECDbHelper::ToPair(nodeKey));

    ECInstanceFinder::FindOptions findOptions(ECInstanceFinder::RelatedDirection::RelatedDirection_HoldingParents, UINT8_MAX);
    if (SUCCESS != m_dbAdapter.GetECInstanceFinder().FindInstances(foundInstances, seedInstances, findOptions))
        {
        return false;
        }

    auto range = foundInstances.equal_range(m_rootClass->GetId());
    for (auto it = range.first; it != range.second; it++)
        {
        if (rootKeys.find(CacheRootKey(m_rootClass->GetId(), it->second)) != rootKeys.end())
            {
            return true;
            }
        }

    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool RootManager::IsNodeInRoot(CacheRootKeyCR rootKey, CacheNodeKeyCR nodeKey)
    {
    if (!nodeKey.IsValid())
        {
        return false;
        }

    auto relId = m_dbAdapter.FindRelationship(m_rootHoldingRelationshipClass, rootKey, nodeKey);
    if (!relId.IsValid())
        {
        return false;
        }

    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::GetNodesConnectedToRoots(const bset<CacheRootKey> rootKeys, ECInstanceKeyMultiMap& nodesOut, uint8_t depth)
    {
    ECInstanceKeyMultiMap seedInstances;
    for (CacheRootKeyCR rootKey : rootKeys)
        {
        seedInstances.insert(ECDbHelper::ToPair(rootKey));
        }

    ECInstanceFinder::FindOptions findOptions(ECInstanceFinder::RelatedDirection::RelatedDirection_HeldChildren, depth);
    if (SUCCESS != m_dbAdapter.GetECInstanceFinder().FindInstances(nodesOut, seedInstances, findOptions))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod    
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::GetNodesLinkedToRoot(Utf8StringCR rootName, ECInstanceKeyMultiMap& nodesOut)
    {
    CacheRootKey rootKey = FindRoot(rootName);
    if (!rootKey.IsValid())
        {
        return SUCCESS;
        }

    return m_hierarchyManager.ReadTargetKeys(rootKey, m_rootHoldingRelationshipClass, nodesOut);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CacheRootKey RootManager::CreateRoot(Utf8StringCR rootName, CacheRootPersistence persistence)
    {
    CacheRootKey existingRoot = FindRoot(rootName);
    if (existingRoot.IsValid())
        {
        BeAssert(false);
        return existingRoot;
        }

    // create new root
    Json::Value newRoot;
    newRoot[CLASS_Root_PROPERTY_Name] = rootName;
    newRoot[CLASS_Root_PROPERTY_Persistence] = static_cast<int> (persistence);

    if (BE_SQLITE_OK != m_rootInserter.Get().Insert(newRoot))
        {
        return CacheRootKey();
        }

    return CacheRootKey(m_rootClass->GetId(), ECDbHelper::ECInstanceIdFromJsonInstance(newRoot));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::RemoveRoot(ECInstanceId rootId)
    {
    ECInstanceKey root(m_rootClass->GetId(), rootId);

    if (SUCCESS != m_hierarchyManager.RemoveAllChildrenFromParent(root, m_rootWeakRelationshipClass) ||
        SUCCESS != m_hierarchyManager.DeleteInstance(root))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::RemoveRoot(Utf8StringCR rootName)
    {
    return RemoveRoots("[" CLASS_Root_PROPERTY_Name "] = ?", rootName.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::RemoveRootsByPrefix(Utf8StringCR rootPrefix)
    {
    return RemoveRoots("instr([" CLASS_Root_PROPERTY_Name "], ?) = 1", rootPrefix.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::RemoveAllRoots()
    {
    return RemoveRoots();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::RemoveRoots(Utf8CP whereClause, Utf8CP parameter)
    {
    Utf8String ecsql = "SELECT ECInstanceId FROM ONLY " ECSql_Root " ";
    if (nullptr != whereClause)
        {
        ecsql += "WHERE ";
        ecsql += whereClause;
        }

    ECSqlStatement statement;
    if (SUCCESS != m_dbAdapter.PrepareStatement(statement, ecsql))
        {
        return ERROR;
        }

    if (nullptr != parameter)
        {
        statement.BindText(1, parameter, IECSqlBinder::MakeCopy::No);
        }

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement.Step()))
        {
        if (SUCCESS != RemoveRoot(statement.GetValueId<ECInstanceId>(0)))
            {
            return ERROR;
            }
        }

    if (BE_SQLITE_DONE != status)
        {
        return ERROR;
        }

    return SUCCESS;
    }
