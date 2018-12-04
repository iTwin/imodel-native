/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Hierarchy/HierarchyManager.cpp $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
WebServices::ECSqlStatementCache& statementCache
) :
m_dbAdapter(ecdbAdapter),
m_statementCache(statementCache)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HierarchyManager::~HierarchyManager()
    {}

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
* @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::ReadSourceKeys
(
ECInstanceKeyCR target,
ECRelationshipClassCP relationshipClass,
bvector<ECInstanceKey>& sourcesOut
)
    {
    if (nullptr == relationshipClass)
        {
        return ERROR;
        }

    ECInstanceKeyMultiMap sourcesMap;
    if (SUCCESS != ReadSourceKeys(target, relationshipClass, sourcesMap))
        {
        return ERROR;
        }

    for (auto& pair : sourcesMap)
        {
        sourcesOut.push_back(ECInstanceKey(pair.first, pair.second));
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::RelateCachedInstancesToHolder
(
CacheNodeKeyCR holder,
ECRelationshipClassCP holderToInfoRelClass,
const bset<CachedInstanceKey>& cachedInstances
)
    {
    for (auto& cachedInstance : cachedInstances)
        {
        if (!RelateInstances(holder, cachedInstance.GetInfoKey(), holderToInfoRelClass).IsValid())
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::RemoveCachedInstancesFromHolder
(
CacheNodeKeyCR holder,
ECRelationshipClassCP holderToInfoRelClass,
const bset<CachedInstanceKey>& cachedInstancesToRemove
)
    {
    for (auto& cachedInstance : cachedInstancesToRemove)
        {
        if (SUCCESS != DeleteRelationship(holder, cachedInstance.GetInfoKey(), holderToInfoRelClass))
            {
            return ERROR;
            }
        if (IsInstanceHeldByOtherInstances(cachedInstance.GetInfoKey()))
            {
            continue;
            }
        if (SUCCESS != DeleteInstance(cachedInstance))
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
    return m_dbAdapter.DeleteInstance(instance);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::DeleteInstance(CachedInstanceKeyCR instance)
    {
    // WIP06: deletions
    if (SUCCESS != DeleteInstance(instance.GetInfoKey()) ||
        SUCCESS != DeleteInstance(instance.GetInstanceKey()))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::DeleteInstances(ECSqlStatement& ecInstanceKeyStatement)
    {
    ECInstanceKeyMultiMap instances;

    DbResult status;
    while (BE_SQLITE_ROW == (status = ecInstanceKeyStatement.Step()))
        {
        ECClassId ecClassId = ecInstanceKeyStatement.GetValueId<ECClassId>(0);
        ECInstanceId ecInstanceId = ecInstanceKeyStatement.GetValueId<ECInstanceId>(1);
        instances.Insert(ecClassId, ecInstanceId);
        }

    if (BE_SQLITE_DONE != status)
        {
        return ERROR;
        }

    return m_dbAdapter.DeleteInstances(instances);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::DeleteInstances(bset<ECInstanceKey> instancesSet)
    {
    ECInstanceKeyMultiMap instances;
    for (ECInstanceKeyCR instance : instancesSet)
        {
        instances.Insert(instance.GetClassId(), instance.GetInstanceId());
        }
    return m_dbAdapter.DeleteInstances(instances);
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
* @bsimethod                                                 julius.cepukenas    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::DeleteForCardinalityViolatingRelate
(
ECInstanceKeyCR source,
ECInstanceKeyCR target,
ECRelationshipClassCP relationshipClass,
bset<ECInstanceKey>& deletedInstancesSetOut
)
    {
    auto deleteFunction = static_cast<BentleyStatus(HierarchyManager::*)(ECInstanceKeyCR, ECInstanceKeyCR, ECRelationshipClassCP, ECInstanceKeyR)>
        (&HierarchyManager::DeleteRelationshipOnly);

    ECInstanceKey deletedInstanceOut;
    auto status = DeleteForCardinalityViolatingRelate
        (
        relationshipClass,
        relationshipClass->GetTarget().GetMultiplicity(),
        source,
        LookupKeys::TargetKeys,
        std::bind(deleteFunction, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
        target,
        deletedInstanceOut
        );

    if (SUCCESS != status)
        return status;

    if (deletedInstanceOut.IsValid())
        deletedInstancesSetOut.insert(deletedInstanceOut);

    status = DeleteForCardinalityViolatingRelate
        (
        relationshipClass,
        relationshipClass->GetSource().GetMultiplicity(),
        target,
        LookupKeys::SourceKeys,
        //Reverse deletion function to correctly delete found source instance and given target instance
        std::bind(deleteFunction, this, std::placeholders::_2, std::placeholders::_1, std::placeholders::_3, std::placeholders::_4),
        source,
        deletedInstanceOut
        );

    if (deletedInstanceOut.IsValid())
        deletedInstancesSetOut.insert(deletedInstanceOut);

    return status;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Julius.Senkus        07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::GetRelatedKeys
(
ECRelationshipClassCP relationshipClass,
ECInstanceKeyCR instance,
LookupKeys type,
ECInstanceKeyMultiMap& keysOut
)
    {
    if (!instance.IsValid())
        return ERROR;
    
    Utf8PrintfString key("GetRelatedKeys:SelectIds:%lld:%lld", type, relationshipClass->GetId().GetValue());
    ECSqlStatementPtr statement = m_statementCache.GetPreparedStatement(key, [=]()
        {
        if (type == TargetKeys)
            return Utf8PrintfString(
                "SELECT rel.TargetECClassId, rel.TargetECInstanceId "
                "FROM ONLY %s rel "
                "WHERE rel.SourceECClassId = ? AND rel.SourceECInstanceId = ?",
                relationshipClass->GetECSqlName().c_str()
                );

        return Utf8PrintfString(
            "SELECT rel.SourceECClassId, rel.SourceECInstanceId "
            "FROM ONLY %s rel "
            "WHERE rel.TargetECClassId = ? AND rel.TargetECInstanceId = ?",
            relationshipClass->GetECSqlName().c_str()
            );
        });

    statement->BindId(1, instance.GetClassId());
    statement->BindId(2, instance.GetInstanceId());

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement->Step()))
        {
        ECClassId relatedClassId = statement->GetValueId<ECClassId>(0);
        ECInstanceId relatedInstanceId = statement->GetValueId<ECInstanceId>(1);
        keysOut.insert({ relatedClassId, relatedInstanceId });
        }

    if (BE_SQLITE_DONE != status)
        return ERROR;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::DeleteForCardinalityViolatingRelate
(
ECRelationshipClassCP relationshipClass,
RelationshipMultiplicityCR relatedInstanceCardinality,
ECInstanceKeyCR instance,
LookupKeys type,
std::function<BentleyStatus(ECInstanceKeyCR, ECInstanceKeyCR, ECRelationshipClassCP, ECInstanceKeyR)> deleteRelathionshipFunction,
ECInstanceKeyCR newRelatedInstance,
ECInstanceKeyR deletedInstanceOut
)
    {
    //Check if related instance has to-one cardinality
    if (1 != relatedInstanceCardinality.GetUpperLimit())
        return SUCCESS;

    ECInstanceKeyMultiMap keysOut;

    if (SUCCESS != GetRelatedKeys(relationshipClass, instance, type, keysOut))
        return ERROR;

    if (1 < keysOut.size())
        return ERROR;

    if (0 == keysOut.size())
        return SUCCESS;

    ECInstanceKey resultKey(keysOut.begin()->first, keysOut.begin()->second);
    if (resultKey == newRelatedInstance)
        return SUCCESS;

    if (SUCCESS != deleteRelathionshipFunction(instance, resultKey, relationshipClass, deletedInstanceOut))
        return ERROR;

    return SUCCESS;
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
BentleyStatus HierarchyManager::DeleteRelationshipOnly
(
ECInstanceKeyCR source,
ECInstanceKeyCR target,
ECRelationshipClassCP relationshipClass,
ECInstanceKeyR deletedRelathionshipInstanceOut
)
    {
    // TODO: not optmized, deletes not direclty
    ECInstanceKey relationship = m_dbAdapter.FindRelationship(relationshipClass, source, target);
    if (!relationship.IsValid())
        {
        // Nothing to delete
        return SUCCESS;
        }

     auto statement = m_statementCache.GetPreparedStatement("GetNavigationProperty", [&]
            {
            return "SELECT Name FROM meta.ECPropertyDef WHERE NavigationRelationshipClass.Id = ?";
            });

        Utf8String navPropertyName;
        statement->BindId(1, relationshipClass->GetId());
        DbResult result = statement->Step();

        if (BE_SQLITE_DONE != result)
            return ERROR;

        if (BE_SQLITE_ROW == result)
            navPropertyName = statement->GetValueText(0);

    Utf8PrintfString key("DeleteRelationshipInstanceUsingECSQL:%llu", relationshipClass->GetId().GetValue());
    if (!navPropertyName.empty())
        {
        statement = m_statementCache.GetPreparedStatement(key, [&]
            {
            return "UPDATE " + relationshipClass->GetECSqlName() + " SET " + navPropertyName + " = NULL WHERE ECInstanceId = ?";
            });
        }
    else
        {
        statement = m_statementCache.GetPreparedStatement(key, [&]
            {
            return "DELETE FROM ONLY " + relationshipClass->GetECSqlName() + " WHERE ECInstanceId = ?";
            });
        }

    statement->BindId(1, relationship.GetInstanceId());
    result = statement->Step();
    if (BE_SQLITE_DONE != result)
        return ERROR;

    deletedRelathionshipInstanceOut = relationship;
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
    // WIP06: deletions
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

    Utf8PrintfString key("HierarchyManager::ReadTargetKeys:%llu", relationshipClass->GetId().GetValue());
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        return 
            "SELECT TargetECClassId, TargetECInstanceId "
            "FROM ONLY " + relationshipClass->GetECSqlName() +  " "
            "WHERE SourceECClassId = ? AND SourceECInstanceId = ? ";
        });

    statement->BindId(1, source.GetClassId());
    statement->BindId(2, source.GetInstanceId());

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement->Step()))
        {
        ECClassId ecClassId = statement->GetValueId<ECClassId>(0);
        ECInstanceId ecId = statement->GetValueId<ECInstanceId>(1);

        keysOut.insert({ecClassId, ecId});
        }

    return BE_SQLITE_DONE == status ? SUCCESS : ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HierarchyManager::ReadSourceKeys
(
ECInstanceKeyCR target,
ECRelationshipClassCP relationshipClass,
ECInstanceKeyMultiMap& keysOut
)
    {
    if (!target.IsValid() || nullptr == relationshipClass)
        {
        return ERROR;
        }

    Utf8PrintfString key("HierarchyManager::ReadSourceKeys:%llu", relationshipClass->GetId().GetValue());
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        return
            "SELECT SourceECClassId, SourceECInstanceId "
            "FROM ONLY " + relationshipClass->GetECSqlName() + 
            "WHERE TargetECClassId = ? AND TargetECInstanceId = ?";
        });

    statement->BindId(1, target.GetClassId());
    statement->BindId(2, target.GetInstanceId());

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement->Step()))
        {
        auto ecClassId = statement->GetValueId<ECClassId>(0);
        auto ecId = statement->GetValueId<ECInstanceId>(1);

        keysOut.Insert(ecClassId, ecId);
        }

    return BE_SQLITE_DONE == status ? SUCCESS : ERROR;
    }
