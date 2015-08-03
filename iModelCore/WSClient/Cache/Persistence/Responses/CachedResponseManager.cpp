/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Responses/CachedResponseManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CachedResponseManager.h"

#include <WebServices/Cache/Util/ECDbHelper.h>

#include "../../Util/StringHelper.h"
#include "../Core/CacheSchema.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedResponseManager::CachedResponseManager
(
ECDbAdapter& dbAdapter,
WebServices::ECSqlStatementCache& statementCache,
HierarchyManager& hierarchyManager,
RelationshipInfoManager& relationshipInfoManager,
ObjectInfoManager& objectInfoManager
) :
m_dbAdapter(&dbAdapter),
m_statementCache(&statementCache),
m_hierarchyManager(&hierarchyManager),
m_relationshipInfoManager(&relationshipInfoManager),
m_objectInfoManager(&objectInfoManager),

m_cachedQueryInfoClass(dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_CachedResponseInfo)),
m_cachedQueryInfoToResultRelationshipClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_REL_CachedResponseInfoToResultRelationship)),
m_cachedQueryInfoToResultWeakRelationshipClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_REL_CachedResponseInfoToResultWeakRelationship)),
m_cachedQueryInfoToParentRelationshipClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_REL_CachedResponseInfoToParentRelationship)),
m_cachedQueryInfoToHolderRelationshipClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_REL_CachedResponseInfoToHolderRelationship)),
m_cachedQueryInfoToRelInfoRelationshipClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_REL_CachedResponseInfoToCachedRelationshipInfo)),

m_infoInserter(dbAdapter.GetECDb(), *m_cachedQueryInfoClass),
m_infoUpdater(dbAdapter.GetECDb(), *m_cachedQueryInfoClass)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::OnBeforeDelete(ECN::ECClassCR ecClass, ECInstanceId ecInstanceId)
    {
    // TODO: can this be taken care of by ECDb with configuration that would tread referencing relationship as holding?
    if (&ecClass != m_cachedQueryInfoToParentRelationshipClass)
        {
        return SUCCESS;
        }

    auto statement = m_statementCache->GetPreparedStatement("CachedResponseManager::_OnBeforeDelete", [=]
        {
        ECSqlSelectBuilder sqlBuilder;
        sqlBuilder
            .Select("rel.SourceECClassId, rel.SourceECInstanceId")
            .From(*m_cachedQueryInfoToParentRelationshipClass, "rel", false)
            .Where("rel.ECInstanceId = ? ")
            .Limit("1");
        return sqlBuilder.ToString();
        });

    statement->BindId(1, ecInstanceId);
    statement->Step();

    ECInstanceKey cachedResponseInfoKey(statement->GetValueInt64(0), statement->GetValueId<ECInstanceId>(1));
    if (!cachedResponseInfoKey.IsValid())
        {
        BeAssert(false);
        return ERROR;
        }

    m_cachedResponsesToDelete.insert(cachedResponseInfoKey);
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::OnAfterDelete(bset<ECInstanceKey>& instancesToDeleteOut)
    {
    instancesToDeleteOut.insert(m_cachedResponsesToDelete.begin(), m_cachedResponsesToDelete.end());
    m_cachedResponsesToDelete.clear();
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedResponseInfo CachedResponseManager::ReadInfo(CachedResponseKeyCR key)
    {
    ECClassCP parentClass = m_dbAdapter->GetECClass(key.GetParent());
    if (!key.GetParent().IsValid() || nullptr == parentClass)
        {
        return CachedResponseInfo();
        }

    Utf8PrintfString statementKey("CachedResponseManager::ReadInfo:%lld", key.GetParent().GetECClassId());
    auto statement = m_statementCache->GetPreparedStatement(statementKey, [=]
        {
        ECSqlSelectBuilder sqlBuilder;
        sqlBuilder
            .Select("query.*")
            .From(*m_cachedQueryInfoClass, "query", false)
            .Join(*parentClass, "parent", false)
            .Using(*m_cachedQueryInfoToParentRelationshipClass, JoinDirection::Forward)
            .Where("parent.ECInstanceId = ? AND query.[" CLASS_CachedResponseInfo_PROPERTY_Name "] = ?")
            .Limit("1");
        return sqlBuilder.ToString();
        });

    statement->BindId(1, key.GetParent().GetECInstanceId());
    statement->BindText(2, key.GetName().c_str(), IECSqlBinder::MakeCopy::No);

    ECSqlStepStatus status = statement->Step();
    if (status != ECSqlStepStatus::HasRow)
        {
        return CachedResponseInfo(key.GetParent(), key.GetHolder(), key.GetName(), m_cachedQueryInfoClass->GetId());
        }

    Json::Value queryInstanceJson;

    JsonECSqlSelectAdapter adapter(*statement, ECValueFormat::RawNativeValues);
    if (!adapter.GetRowInstance(queryInstanceJson, m_cachedQueryInfoClass->GetId()))
        {
        return CachedResponseInfo();
        }

    return CachedResponseInfo(key.GetParent(), key.GetHolder(), queryInstanceJson, m_cachedQueryInfoClass->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECInstanceKey CachedResponseManager::FindInfo(ECInstanceKeyCR parent, Utf8StringCR queryName)
    {
    ECClassCP parentClass = m_dbAdapter->GetECClass(parent);
    if (!parent.IsValid() || nullptr == parentClass)
        {
        return ECInstanceKey();
        }

    Utf8PrintfString key("CachedResponseManager::FindInfo:%lld", parent.GetECClassId());
    auto statement = m_statementCache->GetPreparedStatement(key, [=]
        {
        ECSqlSelectBuilder sqlBuilder;
        sqlBuilder
            .Select("query.ECInstanceId")
            .From(*m_cachedQueryInfoClass, "query", false)
            .Join(*parentClass, "parent", false)
            .Using(*m_cachedQueryInfoToParentRelationshipClass, JoinDirection::Forward)
            .Where("parent.ECInstanceId = ? AND query.[" CLASS_CachedResponseInfo_PROPERTY_Name "] = ?")
            .Limit("1");
        return sqlBuilder.ToString();
        });

    statement->BindId(1, parent.GetECInstanceId());
    statement->BindText(2, queryName.c_str(), IECSqlBinder::MakeCopy::No);

    ECSqlStepStatus status = statement->Step();
    if (status == ECSqlStepStatus::Done)
        {
        // Nothing to delete
        return ECInstanceKey();
        }

    return ECInstanceKey(m_cachedQueryInfoClass->GetId(), statement->GetValueId<ECInstanceId>(0));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::DeleteInfo(ECInstanceKeyCR parent, Utf8StringCR queryName)
    {
    if (!parent.IsValid())
        {
        return ERROR;
        }

    ECInstanceKey queryInfoKey = FindInfo(parent, queryName);
    if (!queryInfoKey.IsValid())
        {
        // Nothing to delete
        return SUCCESS;
        };

    return m_hierarchyManager->DeleteInstance(queryInfoKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::DeleteResponses(Utf8StringCR name, DateTimeCR accessedBeforeDateUtc, const ECInstanceKeyMultiMap& instancesToLeave)
    {
    if (accessedBeforeDateUtc.GetInfo().GetKind() != DateTime::Kind::Utc)
        {
        return ERROR;
        }

    auto statement = m_statementCache->GetPreparedStatement("CachedResponseManager::DeleteResponsesByDate", [=]
        {
        return
            "SELECT ECInstanceId "
            "FROM ONLY " ECSql_CachedResponseInfoClass " "
            "WHERE [" CLASS_CachedResponseInfo_PROPERTY_Name "] = ? AND "
            "     ([" CLASS_CachedResponseInfo_PROPERTY_AccessDate "] IS NULL OR "
            "      [" CLASS_CachedResponseInfo_PROPERTY_AccessDate "] < ?) ";
        });

    statement->BindText(1, name.c_str(), IECSqlBinder::MakeCopy::No);
    statement->BindDateTime(2, accessedBeforeDateUtc);

    bset<ECInstanceKey> responsesToDelete;
    while (ECSqlStepStatus::HasRow == statement->Step())
        {
        ECInstanceKey responseKey(m_cachedQueryInfoClass->GetId(), statement->GetValueId<ECInstanceId>(0));
        if (ECDbHelper::IsInstanceInMultiMap(responseKey, instancesToLeave))
            {
            continue;
            }
        responsesToDelete.insert(responseKey);
        }

    return m_hierarchyManager->DeleteInstances(responsesToDelete);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::DeleteResponses(Utf8StringCR name)
    {
    auto statement = m_statementCache->GetPreparedStatement("CachedResponseManager::DeleteResponses", [=]
        {
        return
            "SELECT GetECClassId(), ECInstanceId "
            "FROM ONLY " ECSql_CachedResponseInfoClass " "
            "WHERE [" CLASS_CachedResponseInfo_PROPERTY_Name "] = ? ";
        });

    statement->BindText(1, name.c_str(), IECSqlBinder::MakeCopy::No);

    return m_hierarchyManager->DeleteInstances(*statement);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::SaveInfo(CachedResponseInfoR queryInfo)
    {
    if (!queryInfo.IsCached())
        {
        if (SUCCESS != m_infoInserter.Get().Insert(queryInfo.GetJsonData()))
            {
            return ERROR;
            }

        if (!m_hierarchyManager->RelateInstances(queryInfo.GetKey(), queryInfo.GetParent(), m_cachedQueryInfoToParentRelationshipClass).IsValid())
            {
            return ERROR;
            }

        if (!m_hierarchyManager->RelateInstances(queryInfo.GetKey(), queryInfo.GetHolder(), m_cachedQueryInfoToHolderRelationshipClass).IsValid())
            {
            return ERROR;
            }
        }
    else
        {
        if (SUCCESS != m_infoUpdater.Get().Update(queryInfo.GetJsonData()))
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::SaveInfo(CachedResponseInfoR queryInfo, const InstanceCacheHelper::CachedInstances& cachedInstances)
    {
    // Insert/update query
    if (SUCCESS != SaveInfo(queryInfo))
        {
        return ERROR;
        }

    if (SUCCESS != RelateResultInstancesToCachedResponseInfo(queryInfo, cachedInstances))
        {
        return ERROR;
        }

    ECInstanceKey queryKey = queryInfo.GetKey();
    if (SUCCESS != RelateResultRelationshipInstancesToCachedResponseInfo(queryKey, cachedInstances))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::RelateResultRelationshipInstancesToCachedResponseInfo
(
ECInstanceKeyCR queryKey,
const InstanceCacheHelper::CachedInstances& cachedInstances
)
    {
    const bset<CachedRelationshipKey>& newCachedRelInfos = cachedInstances.GetCachedRelationshipInfos();
    bset<CachedRelationshipKey> oldCachedRelInfos;

    if (SUCCESS != m_relationshipInfoManager
        ->ReadCachedRelationshipsFromHolder(queryKey, m_cachedQueryInfoToRelInfoRelationshipClass, oldCachedRelInfos))
        {
        return ERROR;
        }

    bset<CachedRelationshipKey> outdatedRelInfos;
    bset<CachedRelationshipKey> newRelInfos;

    std::set_difference(oldCachedRelInfos.begin(), oldCachedRelInfos.end(),
                        newCachedRelInfos.begin(), newCachedRelInfos.end(),
                        std::inserter(outdatedRelInfos, outdatedRelInfos.end()));

    std::set_difference(newCachedRelInfos.begin(), newCachedRelInfos.end(),
                        oldCachedRelInfos.begin(), oldCachedRelInfos.end(),
                        std::inserter(newRelInfos, newRelInfos.end()));

    if (SUCCESS != m_relationshipInfoManager
        ->RelateCachedRelationshipsToHolder(queryKey, m_cachedQueryInfoToRelInfoRelationshipClass, newRelInfos))
        {
        return ERROR;
        }

    if (SUCCESS != m_relationshipInfoManager
        ->RemoveCachedRelationshipsFromHolder(queryKey, m_cachedQueryInfoToRelInfoRelationshipClass, outdatedRelInfos))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::RelateResultInstancesToCachedResponseInfo
(
CachedResponseInfoCR queryInfo,
const InstanceCacheHelper::CachedInstances& cachedInstances
)
    {
    ECInstanceKey queryInfoKey = queryInfo.GetKey();
    ECInstanceKey parent = queryInfo.GetParent();

    for (ECInstanceKeyCR resultInstance : cachedInstances.GetCachedInstances())
        {
        if (nullptr != m_dbAdapter->GetECRelationshipClass(resultInstance))
            {
            continue;
            }

        ECRelationshipClassCP resultRelClass = m_cachedQueryInfoToResultRelationshipClass;
        if (parent == resultInstance)
            {
            resultRelClass = m_cachedQueryInfoToResultWeakRelationshipClass;
            }

        if (!m_hierarchyManager->RelateInstances(queryInfoKey, resultInstance, resultRelClass).IsValid())
            {
            return ERROR;
            }
        }

    // Remove old instances from query
    if (SUCCESS != m_hierarchyManager->ReleaseOldChildren(queryInfoKey, cachedInstances.GetCachedInstances(), m_cachedQueryInfoToResultRelationshipClass))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::ReadResponse
(
CachedResponseInfoCR queryInfo,
ISelectProviderCR selectProvider,
const CacheQueryHelper::ReadCallback& readCallback
)
    {
    if (!queryInfo.IsCached())
        {
        return SUCCESS;
        }

    ECInstanceKey queryInfoKey = queryInfo.GetKey();

    // Read result keys
    ECInstanceKeyMultiMap resultKeys;
    if (SUCCESS != ReadResponseInstanceKeys(queryInfoKey, resultKeys))
        {
        return ERROR;
        }

    CacheQueryHelper helper(selectProvider);
    return helper.ReadInstances(*m_dbAdapter, resultKeys, readCallback);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus CachedResponseManager::ReadResponseInstanceKeys(ECInstanceKeyCR cachedQueryInfoDesc, ECInstanceKeyMultiMap& keysOut)
    {
    if (SUCCESS != m_hierarchyManager->ReadTargetKeys(cachedQueryInfoDesc, m_cachedQueryInfoToResultRelationshipClass, keysOut))
        {
        return ERROR;
        }
    if (SUCCESS != m_hierarchyManager->ReadTargetKeys(cachedQueryInfoDesc, m_cachedQueryInfoToResultWeakRelationshipClass, keysOut))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::ReadResponseObjectIds
(
ECInstanceId cachedQueryInfoId,
bset<ObjectId>& objectIdsOut
)
    {
    if (SUCCESS != ReadResponseObjectIds(cachedQueryInfoId, m_cachedQueryInfoToResultRelationshipClass, objectIdsOut))
        {
        return ERROR;
        }
    if (SUCCESS != ReadResponseObjectIds(cachedQueryInfoId, m_cachedQueryInfoToResultWeakRelationshipClass, objectIdsOut))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CachedResponseManager::ReadResponseObjectIds
(
ECInstanceId cachedQueryInfoId,
ECRelationshipClassCP relationshipClass,
bset<ObjectId>& objectIdsOut
)
    {
    Utf8PrintfString key("CachedResponseManager::ReadResponseObjectIds:%lld", relationshipClass->GetId());
    auto statement = m_statementCache->GetPreparedStatement(key, [=]
        {
        return CacheQueryHelper::ECSql::SelectRemoteIdsByRelatedSourceECInstanceId(*relationshipClass);
        });

    statement->BindId(1, cachedQueryInfoId);

    ECSqlStepStatus status;
    while (ECSqlStepStatus::HasRow == (status = statement->Step()))
        {
        ECClassCP resultClass = m_dbAdapter->GetECClass(statement->GetValueInt64(0));
        if (nullptr == resultClass)
            {
            return ERROR;
            }

        ObjectId objectId
            {
            Utf8String(resultClass->GetSchema().GetName()),
            Utf8String(resultClass->GetName()),
            statement->GetValueText(1)
            };

        objectIdsOut.insert(objectId);
        }

    if (ECSqlStepStatus::Done != status)
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CachedResponseManager::ReadResponseCacheTag(ECInstanceKeyCR parent, Utf8StringCR queryName)
    {
    ECClassCP parentClass = m_dbAdapter->GetECClass(parent);
    if (!parent.IsValid() || nullptr == parentClass)
        {
        return "";
        }

    Utf8PrintfString key("CachedResponseManager::ReadResponseCacheTag:%lld", parent.GetECClassId());
    auto statement = m_statementCache->GetPreparedStatement(key, [=]
        {
        ECSqlSelectBuilder sqlBuilder;
        sqlBuilder
            .Select("query.[" CLASS_CachedResponseInfo_PROPERTY_CacheTag "]")
            .From(*m_cachedQueryInfoClass, "query", false)
            .Join(*parentClass, "parent", false)
            .Using(*m_cachedQueryInfoToParentRelationshipClass, JoinDirection::Forward)
            .Where("parent.ECInstanceId = ? AND query.[" CLASS_CachedResponseInfo_PROPERTY_Name "] = ?")
            .Limit("1");
        return sqlBuilder.ToString();
        });

    statement->BindId(1, parent.GetECInstanceId());
    statement->BindText(2, queryName.c_str(), IECSqlBinder::MakeCopy::No);

    statement->Step();

    return statement->GetValueText(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
DateTime CachedResponseManager::ReadResponseCachedDate(ECInstanceKeyCR parent, Utf8StringCR queryName)
    {
    ECClassCP parentClass = m_dbAdapter->GetECClass(parent);
    if (!parent.IsValid() || nullptr == parentClass)
        {
        return DateTime();
        }

    Utf8PrintfString key("CachedResponseManager::ReadResponseCachedDate:%lld", parent.GetECClassId());
    auto statement = m_statementCache->GetPreparedStatement(key, [&]
        {
        ECSqlSelectBuilder sqlBuilder;
        sqlBuilder
            .Select("query.[" CLASS_CachedResponseInfo_PROPERTY_CacheDate "]")
            .From(*m_cachedQueryInfoClass, "query", false)
            .Join(*parentClass, "parent", false)
            .Using(*m_cachedQueryInfoToParentRelationshipClass, JoinDirection::Forward)
            .Where("parent.ECInstanceId = ? AND query.[" CLASS_CachedResponseInfo_PROPERTY_Name "] = ?")
            .Limit("1");
        return sqlBuilder.ToString();
        });

    statement->BindId(1, parent.GetECInstanceId());
    statement->BindText(2, queryName.c_str(), IECSqlBinder::MakeCopy::No);

    if (ECSqlStepStatus::HasRow != statement->Step())
        {
        return DateTime();
        }

    return statement->GetValueDateTime(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus CachedResponseManager::MarkTemporaryInstancesAsPartial
(
const bset<ECInstanceId>& cachedQueryInfoIds,
const ECInstanceKeyMultiMap& fullyPersistedInstances
)
    {
    if (SUCCESS != MarkTemporaryInstancesAsPartial(cachedQueryInfoIds, m_cachedQueryInfoToResultRelationshipClass, fullyPersistedInstances))
        {
        return ERROR;
        }
    if (SUCCESS != MarkTemporaryInstancesAsPartial(cachedQueryInfoIds, m_cachedQueryInfoToResultWeakRelationshipClass, fullyPersistedInstances))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus CachedResponseManager::MarkTemporaryInstancesAsPartial
(
const bset<ECInstanceId>& cachedQueryInfoIds,
ECRelationshipClassCP resultRelationshipClass,
const ECInstanceKeyMultiMap& fullyPersistedInstances
)
    {
    ECClassCP instanceInfoClass = m_objectInfoManager->GetInfoClass();
    ECClassCP instanceInfoRelationshipClass = m_objectInfoManager->GetInfoRelationshipClass();

    Utf8String ecsql =
        "SELECT infoRel.TargetECClassId, infoRel.TargetECInstanceId, info.* "
        "FROM ONLY " + ECSqlBuilder::ToECSqlSnippet(*instanceInfoClass) + " info "
        "JOIN ONLY " + ECSqlBuilder::ToECSqlSnippet(*instanceInfoRelationshipClass) + " infoRel ON info.ECInstanceId = infoRel.SourceECInstanceId "
        "JOIN ONLY " + ECSqlBuilder::ToECSqlSnippet(*resultRelationshipClass) + " resultRel ON infoRel.TargetECInstanceId = resultRel.TargetECInstanceId "
        "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceState "] = ? "
        "AND resultRel.SourceECInstanceId IN (" + StringHelper::Join(cachedQueryInfoIds.begin(), cachedQueryInfoIds.end(), ',') + ")";

    ECSqlStatement statement;
    if (SUCCESS != m_dbAdapter->PrepareStatement(statement, ecsql))
        {
        return ERROR;
        }
    statement.BindInt(1, static_cast<int> (CachedInstanceState::Full));

    JsonECSqlSelectAdapter adapter(statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));

    ECSqlStepStatus status;
    while (ECSqlStepStatus::HasRow == (status = statement.Step()))
        {
        ECInstanceKey instanceKey(statement.GetValueInt64(0), statement.GetValueId<ECInstanceId>(1));

        if (ECDbHelper::IsInstanceInMultiMap(instanceKey, fullyPersistedInstances))
            {
            // Not temporary
            continue;
            }

        Json::Value instanceInfoJson;
        adapter.GetRowInstance(instanceInfoJson, instanceInfoClass->GetId());
        ECClassCP instanceClass = m_dbAdapter->GetECClass(instanceKey);
        ObjectInfo instanceInfo(instanceInfoJson, instanceClass, m_objectInfoManager->GetInfoClass()->GetId());

        instanceInfo.SetObjectState(CachedInstanceState::Partial);

        if (SUCCESS != m_objectInfoManager->UpdateInfo(instanceInfo))
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

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus CachedResponseManager::InvalidateResponsesContainingInstance(ECInstanceKeyCR instanceKey)
    {
    Json::Value infos;

    // Result relationship
    auto statement = m_statementCache->GetPreparedStatement("CachedResponseManager::InvalidateResponsesContainingInstance:Rel", [&]
        {
        return
            "SELECT info.* "
            "FROM ONLY " ECSql_CachedResponseInfoClass " info "
            "JOIN " ECSql_CachedResponseInfoToResultRelationshipClass " rel ON info.ECInstanceId = rel.SourceECInstanceId "
            "WHERE rel.TargetECClassId = ? AND rel.TargetECInstanceId = ? ";
        });

    statement->BindInt64(1, instanceKey.GetECClassId());
    statement->BindId(2, instanceKey.GetECInstanceId());

    if (SUCCESS != m_dbAdapter->ExtractJsonInstanceArrayFromStatement(*statement, m_cachedQueryInfoClass, infos))
        {
        return ERROR;
        }

    // Result weak relationship
    statement = m_statementCache->GetPreparedStatement("CachedResponseManager::InvalidateResponsesContainingInstance:Weak", [&]
        {
        return
            "SELECT info.* "
            "FROM ONLY " ECSql_CachedResponseInfoClass " info "
            "JOIN " ECSql_CachedResponseInfoToResultWeakRelationshipClass " rel ON info.ECInstanceId = rel.SourceECInstanceId "
            "WHERE rel.TargetECClassId = ? AND rel.TargetECInstanceId = ? ";
        });

    statement->BindInt64(1, instanceKey.GetECClassId());
    statement->BindId(2, instanceKey.GetECInstanceId());

    if (SUCCESS != m_dbAdapter->ExtractJsonInstanceArrayFromStatement(*statement, m_cachedQueryInfoClass, infos))
        {
        return ERROR;
        }

    // Invalidate
    for (JsonValueR info : infos)
        {
        info[CLASS_CachedResponseInfo_PROPERTY_CacheTag] = "";
        if (SUCCESS != m_infoUpdater.Get().Update(info))
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }
