/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Hierarchy/HierarchyManager.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "HierarchyManager.h"

#include <WebServices/Cache/Persistence/ChangeManager.h>
#include <WebServices/Cache/Util/ECDbHelper.h>

#include "../../Logging.h"
#include "../Changes/ChangeInfoManager.h"
#include "../Core/CacheSchema.h"
#include "../Instances/ObjectInfoManager.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HierarchyManager::HierarchyManager
(
ECDbAdapterR ecdbAdapter,
ECSqlStatementCache& statementCache,
ObjectInfoManager& objectInfoManager,
ChangeInfoManager& changeInfoManager,
std::vector<IDeleteHandler*> deleteHandlers
) :
m_dbAdapter(ecdbAdapter),
m_statementCache(&statementCache),
m_objectInfoManager(objectInfoManager),
m_changeInfoManager(changeInfoManager),
m_deleteHandlers(deleteHandlers)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HierarchyManager::~HierarchyManager()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyManager::_OnBeforeDelete(ECN::ECClassCR ecClass, ECInstanceId ecInstanceId, ECDbR ecDb)
    {
    for (auto handler : m_deleteHandlers)
        {
        if (SUCCESS != handler->OnBeforeDelete(ecClass, ecInstanceId))
            {
            BeAssert(false);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::GetAdditonalInstancesToDelete(bset<ECInstanceKey>& instancesToDeleteOut)
    {
    for (auto handler : m_deleteHandlers)
        {
        if (SUCCESS != handler->OnAfterDelete(instancesToDeleteOut))
            {
            BeAssert(false);
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::ReadTargetKeys
(
ECInstanceKeyCR source,
ECRelationshipClassCP relationshipClass,
bvector<ECInstanceKey>& targetsOut
)
    {
    if (nullptr == relationshipClass)
        {
        return ERROR;
        }

    ECInstanceKeyMultiMap targetsMap;
    if (SUCCESS != ReadTargetKeys(source, relationshipClass, targetsMap))
        {
        return ERROR;
        }

    for (auto& pair : targetsMap)
        {
        targetsOut.push_back(ECInstanceKey(pair.first, pair.second));
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::ReleaseOldChildren
(
ECInstanceKeyCR parent,
const bset<ECInstanceKey>& newChildren,
ECRelationshipClassCP relationshipClass
)
    {
    bvector<ECInstanceKey> currentChildren;
    if (SUCCESS != ReadTargetKeys(parent, relationshipClass, currentChildren))
        {
        return ERROR;
        }

    bvector<ECInstanceKey> obsoleteInstances;
    for (ECInstanceKey child : currentChildren)
        {
        if (newChildren.find(child) != newChildren.end())
            {
            continue;
            }

        auto changeStatus = m_changeInfoManager.GetObjectChangeStatus(child);
        if (IChangeManager::ChangeStatus::Created == changeStatus)
            {
            // Dont remove created objects
            continue;
            }

        if (SUCCESS != DeleteRelationship(parent, child, relationshipClass))
            {
            return ERROR;
            }

        BeAssert((IChangeManager::ChangeStatus::NoChange == changeStatus || 
                  !m_objectInfoManager.FindCachedInstance(child).IsEmpty()) && "<Warning> Local change will be removed");

        obsoleteInstances.push_back(child);
        }

    for (ECInstanceKeyCR instance : obsoleteInstances)
        {
        if (SUCCESS != CheckAndCleanupHiearchy(instance))
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::CheckAndCleanupHiearchy(ECInstanceKeyCR instance)
    {
    if (IsInstanceHeldByOtherInstances(instance))
        {
        return SUCCESS;
        }

    return DeleteInstance(instance);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::DeleteInstance(ECInstanceKeyCR instance)
    {
    ECClassCP ecClass = m_dbAdapter.GetECClass(instance);
    if (nullptr == ecClass)
        {
        return ERROR;
        }
    ECPersistencePtr persistence = m_dbAdapter.GetECDb().GetEC().GetECPersistence(nullptr, *ecClass);
    if (persistence.IsNull() || DELETE_Success != persistence->Delete(instance.GetECInstanceId(), this))
        {
        return ERROR;
        }

    bset<ECInstanceKey> additionalInstances;
    if (SUCCESS != GetAdditonalInstancesToDelete(additionalInstances))
        {
        return ERROR;
        }
    return DeleteInstances(additionalInstances);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::DeleteInstances(ECSqlStatement& ecInstanceKeyStatement)
    {
    ECSqlStepStatus status;
    while (ECSqlStepStatus::HasRow == (status = ecInstanceKeyStatement.Step()))
        {
        ECClassId ecClassId = ecInstanceKeyStatement.GetValueInt64(0);
        ECInstanceId ecInstanceId = ecInstanceKeyStatement.GetValueId<ECInstanceId>(1);
        if (SUCCESS != DeleteInstance(ECInstanceKey(ecClassId, ecInstanceId)))
            {
            return ERROR;
            }
        }
    if (ECSqlStepStatus::Done != status)
        {
        return ERROR;
        }

    bset<ECInstanceKey> additionalInstances;
    if (SUCCESS != GetAdditonalInstancesToDelete(additionalInstances))
        {
        return ERROR;
        }
    return DeleteInstances(additionalInstances);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::DeleteInstances(bset<ECInstanceKey> instances)
    {
    while (!instances.empty())
        {
        for (ECInstanceKeyCR instance : instances)
            {
            ECClassCP ecClass = m_dbAdapter.GetECClass(instance);
            if (nullptr == ecClass)
                {
                return ERROR;
                }
            ECPersistencePtr persistence = m_dbAdapter.GetECDb().GetEC().GetECPersistence(nullptr, *ecClass);
            if (persistence.IsNull() || DELETE_Success != persistence->Delete(instance.GetECInstanceId(), this))
                {
                return ERROR;
                }
            }

        instances.clear();
        if (SUCCESS != GetAdditonalInstancesToDelete(instances))
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool HierarchyManager::IsInstanceHeldByOtherInstances(ECInstanceKeyCR instance)
    {
    auto direction =
        ECInstanceFinder::RelatedDirection_HoldingParents |
        ECInstanceFinder::RelatedDirection_EmbeddingParent;

    ECInstanceKeyMultiMap parents;
    if (SUCCESS != m_dbAdapter.GetECInstanceFinder().FindRelatedInstances(&parents, nullptr, instance, direction))
        {
        return false;
        }

    return !parents.empty();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey HierarchyManager::RelateInstances
(
ECInstanceKeyCR source,
ECInstanceKeyCR target,
ECRelationshipClassCP relationshipClass
)
    {
    return m_dbAdapter.RelateInstances(relationshipClass, source, target);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::CopyRelationshipsBySource
(
ECInstanceKeyCR instanceFrom,
ECInstanceKeyCR instanceTo,
ECRelationshipClassCP relationshipClass
)
    {
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::DeleteRelationship
(
ECInstanceKeyCR source,
ECInstanceKeyCR target,
ECRelationshipClassCP relationshipClass
)
    {
    if (SUCCESS != m_dbAdapter.DeleteRelationship(relationshipClass, source, target))
        {
        BeAssert(false && "Unable delete relationship from cache");
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::DeleteRelationship(ECInstanceKeyCR relationship)
    {
    if (!relationship.IsValid() || nullptr == m_dbAdapter.GetECRelationshipClass(relationship))
        {
        BeAssert(false);
        return ERROR;
        }
    return DeleteInstance(relationship);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::RemoveChildFromParent(ECInstanceKeyCR parent, ECInstanceKeyCR child, ECRelationshipClassCP relationshipClass)
    {
    ECInstanceKey relationshipKey = m_dbAdapter.FindRelationship(relationshipClass, parent, child);
    if (!relationshipKey.IsValid())
        {
        return SUCCESS;
        }
    if (SUCCESS != DeleteInstance(relationshipKey))
        {
        return ERROR;
        }
    if (SUCCESS != CheckAndCleanupHiearchy(child))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::RemoveAllChildrenFromParent(ECInstanceKeyCR parent, ECRelationshipClassCP relationshipClass)
    {
    bvector<ECInstanceKey> children;
    if (SUCCESS != ReadTargetKeys(parent, relationshipClass, children))
        {
        return ERROR;
        }
    if (SUCCESS != DeleteRelationships(parent, children, relationshipClass))
        {
        return ERROR;
        }
    for (ECInstanceKeyCR instance : children)
        {
        if (SUCCESS != CheckAndCleanupHiearchy(instance))
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::DeleteRelationships
(
ECInstanceKeyCR source,
const bvector<ECInstanceKey>& targets,
ECRelationshipClassCP relationshipClass
)
    {
    for (ECInstanceKeyCR target : targets)
        {
        if (SUCCESS != DeleteRelationship(source, target, relationshipClass))
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::ReadTargetKeys
(
ECInstanceKeyCR source,
ECRelationshipClassCP relationshipClass,
ECInstanceKeyMultiMap& keysOut
)
    {
    if (!source.IsValid() || nullptr == relationshipClass)
        {
        return ERROR;
        }

    Utf8PrintfString key("HierarchyManager::ReadTargetKeys:%lld", relationshipClass->GetId());
    auto statement = m_statementCache->GetPreparedStatement(key, [&]
        {
        ECSqlSelectBuilder sqlBuilder;
        sqlBuilder
            .Select("rel.TargetECClassId, rel.TargetECInstanceId")
            .From(*relationshipClass, "rel", false)
            .Where("rel.SourceECClassId = ? AND rel.SourceECInstanceId = ?");
        return sqlBuilder.ToString();
        });

    statement->BindInt64(1, source.GetECClassId());
    statement->BindId(2, source.GetECInstanceId());

    ECSqlStepStatus status;
    while (ECSqlStepStatus::HasRow == (status = statement->Step()))
        {
        ECClassId ecClassId = statement->GetValueInt64(0);
        ECInstanceId ecId = statement->GetValueId<ECInstanceId>(1);

        keysOut.insert({ecClassId, ecId});
        }

    return ECSqlStepStatus::Done == status ? SUCCESS : ERROR;
    }
