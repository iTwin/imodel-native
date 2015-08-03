/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Hierarchy/RootManager.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

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
InstanceCacheHelper&  instanceHelper,
HierarchyManager& hierarchyManager,
ObjectInfoManager& objectInfoManager
) :
m_dbAdapter(dbAdapter),
m_statementCache(&statementCache),

m_instanceHelper(instanceHelper),
m_hierarchyManager(hierarchyManager),
m_objectInfoManager(objectInfoManager),

m_rootClass(dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_Root)),
m_rootHoldingRelationshipClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_REL_RootRelationship)),
m_rootWeakRelationshipClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_REL_WeakRootRelationship)),

m_rootInserter(dbAdapter.GetECDb(), *m_rootClass),
m_rootUpdater(dbAdapter.GetECDb(), *m_rootClass)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP RootManager::GetRootRelationshipClass() const
    {
    return m_rootHoldingRelationshipClass;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceId RootManager::FindRootECInstanceId(Utf8StringCR rootName)
    {
    auto statement = m_statementCache->GetPreparedStatement("RootManager:FindRootECInstanceId", [&]
        {
        return
            "SELECT ECInstanceId "
            "FROM " ECSql_RootClass " "
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
ECInstanceKey RootManager::FindRoot(Utf8StringCR rootName)
    {
    return ECInstanceKey(m_rootClass->GetId(), FindRootECInstanceId(rootName));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey RootManager::FindOrCreateRoot(Utf8StringCR rootName)
    {
    ECInstanceKey root = FindRoot(rootName);
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
    auto statement = m_statementCache->GetPreparedStatement("RootManager:SetupRoot", [&]
        {
        return
            "SELECT * "
            "FROM " ECSql_RootClass " "
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
    if (SUCCESS != m_rootUpdater.Get().Update(root))
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
    if (SUCCESS != m_rootUpdater.Get().Update(root))
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

    root[CLASS_Root_PROPERTY_SyncDate] = utcDateTime.ToUtf8String();
    if (SUCCESS != m_rootUpdater.Get().Update(root))
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
    auto statement = m_statementCache->GetPreparedStatement("RootManager:ReadRootSyncDate", [&]
        {
        return
            "SELECT [" CLASS_Root_PROPERTY_SyncDate "]"
            "FROM " ECSql_RootClass " "
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
BentleyStatus RootManager::GetInstancesByPersistence(CacheRootPersistence persistence, ECInstanceKeyMultiMap& instancesOut)
    {
    auto statement = m_statementCache->GetPreparedStatement("RootManager:GetInstancesByPersistence", [&]
        {
        return
            "SELECT ECInstanceId "
            "FROM " ECSql_RootClass " "
            "WHERE [" CLASS_Root_PROPERTY_Persistence "] = ? ";
        });

    statement->BindInt(1, static_cast<int>(persistence));

    ECInstanceKeyMultiMap ansestorInstances;
    if (SUCCESS != m_dbAdapter.ExtractECInstanceKeyMultiMapFromStatement(*statement, 0, m_rootClass->GetId(), ansestorInstances))
        {
        return ERROR;
        }

    ECInstanceFinder::FindOptions findOptions(ECInstanceFinder::RelatedDirection::RelatedDirection_HeldChildren, UINT8_MAX);
    if (SUCCESS != m_dbAdapter.GetECInstanceFinder().FindInstances(instancesOut, ansestorInstances, findOptions))
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
Utf8StringCR rootName,
ObjectIdCR objectId,
bool holding
)
    {
    ObjectInfo info = m_objectInfoManager.ReadInfo(objectId);
    if (SUCCESS != CacheFloatingInstance(objectId, info, nullptr))
        {
        return ERROR;
        }

    if (SUCCESS != LinkExistingInstanceToRoot(rootName, info.GetCachedInstanceKey(), holding))
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
Utf8StringCR rootName,
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

    if (SUCCESS != LinkExistingInstanceToRoot(rootName, info.GetCachedInstanceKey(), holding))
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

        Json::Value cachedInstanceInfo;
        if (SUCCESS != m_instanceHelper.CacheNewInstance(info, *instanceJsonToSave))
            {
            return ERROR;
            }
        }
    else if (nullptr != instanceJson)
        {
        if (SUCCESS != m_instanceHelper.CacheExistingInstance(info, *instanceJson))
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::LinkExistingInstanceToRoot(Utf8StringCR rootName, ECInstanceKeyCR instance, bool holding)
    {
    bset<ECInstanceKey> instances;
    instances.insert(instance);
    return LinkExistingInstancesToRoot(rootName, instances, holding);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::LinkExistingInstancesToRoot(Utf8StringCR rootName, const bset<ECInstanceKey>& instances, bool holding)
    {
    if (instances.empty())
        {
        return SUCCESS;
        }

    ECInstanceKey root = FindOrCreateRoot(rootName);

    for (ECInstanceKeyCR instance : instances)
        {
        if (holding)
            {
            if (!m_hierarchyManager.RelateInstances(root, instance, m_rootHoldingRelationshipClass).IsValid())
                {
                return ERROR;
                }
            }
        else
            {
            if (!m_hierarchyManager.RelateInstances(root, instance, m_rootWeakRelationshipClass).IsValid())
                {
                return ERROR;
                }
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::UnlinkInstanceFromRoot(Utf8StringCR rootName, ECInstanceKeyCR instance)
    {
    if (!instance.IsValid())
        {
        return SUCCESS;
        }

    ECInstanceKey root = FindRoot(rootName);
    if (!root.IsValid())
        {
        return SUCCESS;
        }

    if (SUCCESS != m_hierarchyManager.RemoveChildFromParent(root, instance, m_rootHoldingRelationshipClass))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::UnlinkAllInstancesFromRoot(Utf8StringCR rootName)
    {
    ECInstanceKey root = FindRoot(rootName);
    if (!root.IsValid())
        {
        return SUCCESS;
        }
    if (SUCCESS != m_hierarchyManager.RemoveAllChildrenFromParent(root, m_rootHoldingRelationshipClass))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::CopyRootRelationships(ECInstanceKeyCR fromInstance, ECInstanceKeyCR toInstance)
    {
    bvector<ECInstanceId> roots;
    if (SUCCESS != m_dbAdapter.GetRelatedSourceIds(m_rootHoldingRelationshipClass, m_rootClass, roots, fromInstance))
        {
        return ERROR;
        }

    for (ECInstanceId rootECId : roots)
        {
        if (!m_dbAdapter.RelateInstances(m_rootHoldingRelationshipClass, ECInstanceKey(m_rootClass->GetId(), rootECId), toInstance).IsValid())
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool RootManager::IsInstanceConnectedToAnyOfRoots(ECInstanceKeyCR instance, const bset<ECInstanceId>& rootIds)
    {
    if (!instance.IsValid())
        {
        return false;
        }

    ECInstanceKeyMultiMap foundInstances;

    ECInstanceKeyMultiMap seedInstances;
    seedInstances.insert(ECDbHelper::ToPair(instance));

    ECInstanceFinder::FindOptions findOptions(ECInstanceFinder::RelatedDirection::RelatedDirection_HoldingParents, UINT8_MAX);
    if (SUCCESS != m_dbAdapter.GetECInstanceFinder().FindInstances(foundInstances, seedInstances, findOptions))
        {
        return false;
        }

    auto range = foundInstances.equal_range(m_rootClass->GetId());
    for (auto it = range.first; it != range.second; it++)
        {
        if (rootIds.find(it->second) != rootIds.end())
            {
            return true;
            }
        }

    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool RootManager::IsInstanceInRoot(ECInstanceKeyCR instance, ECInstanceId rootECId)
    {
    if (!instance.IsValid())
        {
        return false;
        }

    auto relId = m_dbAdapter.FindRelationship(m_rootHoldingRelationshipClass, ECInstanceKey(m_rootClass->GetId(), rootECId), instance);
    if (!relId.IsValid())
        {
        return false;
        }

    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::GetInstancesConnectedToRoots(const bset<ECInstanceId> roots, ECInstanceKeyMultiMap& instancesOut, uint8_t depth)
    {
    ECInstanceKeyMultiMap seedInstances;
    for (ECInstanceId rootId : roots)
        {
        seedInstances.insert(bpair<ECClassId, ECInstanceId>(m_rootClass->GetId(), rootId));
        }

    ECInstanceFinder::FindOptions findOptions(ECInstanceFinder::RelatedDirection::RelatedDirection_HeldChildren, depth);
    if (SUCCESS != m_dbAdapter.GetECInstanceFinder().FindInstances(instancesOut, seedInstances, findOptions))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey RootManager::CreateRoot(Utf8StringCR rootName, CacheRootPersistence persistence)
    {
    ECInstanceKey existingRoot = FindRoot(rootName);
    if (existingRoot.IsValid())
        {
        BeAssert(false);
        return existingRoot;
        }

    // create new root
    Json::Value newRoot;
    newRoot[CLASS_Root_PROPERTY_Name] = rootName;
    newRoot[CLASS_Root_PROPERTY_Persistence] = static_cast<int> (persistence);

    if (SUCCESS != m_rootInserter.Get().Insert(newRoot))
        {
        BeAssert(false && "Inserting cache root");
        return ECInstanceKey();
        }

    return ECInstanceKey(m_rootClass->GetId(), ECDbHelper::ECInstanceIdFromJsonInstance(newRoot));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::RemoveRoot(ECInstanceId rootId)
    {
    ECInstanceKey root(m_rootClass->GetId(), rootId);

    if (SUCCESS != m_hierarchyManager.RemoveAllChildrenFromParent(root, m_rootWeakRelationshipClass))
        {
        return ERROR;
        }
    if (SUCCESS != m_hierarchyManager.DeleteInstance(root))
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
    return RemoveRoots(Utf8PrintfString("[" CLASS_Root_PROPERTY_Name "] = '%s'", rootName.c_str()));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootManager::RemoveRootsByPrefix(Utf8StringCR rootPrefix)
    {
    return RemoveRoots(Utf8PrintfString("[" CLASS_Root_PROPERTY_Name "] LIKE '%s%%'", rootPrefix.c_str()));
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
BentleyStatus RootManager::RemoveRoots(Utf8CP whereClause)
    {
    ECSqlSelectBuilder builder;
    builder.Select("ECInstanceId").From(*m_rootClass);
    if (nullptr != whereClause)
        {
        builder.Where(whereClause);
        }

    ECSqlStatement statement;
    if (SUCCESS != m_dbAdapter.PrepareStatement(statement, builder))
        {
        return ERROR;
        }

    ECSqlStepStatus status;
    while (ECSqlStepStatus::HasRow == (status = statement.Step()))
        {
        if (SUCCESS != RemoveRoot(statement.GetValueId<ECInstanceId>(0)))
            {
            return ERROR;
            }
        }

    if (ECSqlStepStatus::Done != status)
        {
        return ERROR;
        }

    return SUCCESS;
    }
