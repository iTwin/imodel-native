/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Instances/RelationshipInfoManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "RelationshipInfoManager.h"

#include <WebServices/Cache/Util/ECDbHelper.h>

#include "../Core/CacheSchema.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipInfoManager::RelationshipInfoManager
(
ECDbAdapter& dbAdapter,
ECSqlStatementCache& statementCache,
HierarchyManager& hierarchyManager
) :
m_dbAdapter(&dbAdapter),
m_statementCache(&statementCache),
m_hierarchyManager(&hierarchyManager),

m_cachedRelationshipInfoClass(dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_CachedRelationshipInfo)),

m_infoECInstanceInserter(dbAdapter.GetECDb(), *m_cachedRelationshipInfoClass),
m_infoInserter(dbAdapter.GetECDb(), *m_cachedRelationshipInfoClass),
m_infoUpdater(dbAdapter.GetECDb(), *m_cachedRelationshipInfoClass)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECClassCP RelationshipInfoManager::GetInfoClass() const
    {
    return m_cachedRelationshipInfoClass;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
RelationshipInfo RelationshipInfoManager::ReadInfo(ECRelationshipClassCR relationshipClass, ECInstanceKeyCR source, ECInstanceKeyCR target)
    {
    if (!source.IsValid() || !target.IsValid())
        {
        return RelationshipInfo();
        }

    auto statement = m_statementCache->GetPreparedStatement("RelationshipInfoManager::ReadInfoByEnds", [&]
        {
        return
            "SELECT info.* "
            "FROM ONLY " ECSql_CachedRelationshipInfoClass " info "
            "WHERE info.[" CLASS_CachedRelationshipInfo_PROPERTY_RelClassId "] = ? "
            "  AND info.[" CLASS_CachedRelationshipInfo_PROPERTY_SourceClassId "] = ? "
            "  AND info.[" CLASS_CachedRelationshipInfo_PROPERTY_SourceInstanceId "] = ? "
            "  AND info.[" CLASS_CachedRelationshipInfo_PROPERTY_TargetClassId "] = ? "
            "  AND info.[" CLASS_CachedRelationshipInfo_PROPERTY_TargetInstanceId "] = ? "
            "LIMIT 1";
        });

    statement->BindInt64(1, relationshipClass.GetId());
    statement->BindInt64(2, source.GetECClassId());
    statement->BindId(3, source.GetECInstanceId());
    statement->BindInt64(4, target.GetECClassId());
    statement->BindId(5, target.GetECInstanceId());

    ECInstanceId relationshipInstanceId;
    Json::Value infoJson;

    ECSqlStepStatus status = statement->Step();
    if (ECSqlStepStatus::HasRow == status)
        {
        JsonECSqlSelectAdapter adapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
        adapter.GetRowInstance(infoJson, m_cachedRelationshipInfoClass->GetId());
        relationshipInstanceId = ECDbHelper::ECInstanceIdFromJsonValue(infoJson[CLASS_CachedRelationshipInfo_PROPERTY_RelInstanceId]);
        }
    else
        {
        // Return valid info object so it could be saved
        infoJson[CLASS_CachedRelationshipInfo_PROPERTY_RelClassId] = BeJsonUtilities::StringValueFromInt64(relationshipClass.GetId());
        infoJson[CLASS_CachedRelationshipInfo_PROPERTY_SourceClassId] = BeJsonUtilities::StringValueFromInt64(source.GetECClassId());
        infoJson[CLASS_CachedRelationshipInfo_PROPERTY_TargetClassId] = BeJsonUtilities::StringValueFromInt64(target.GetECClassId());
        infoJson[CLASS_CachedRelationshipInfo_PROPERTY_SourceInstanceId] = ECDbHelper::StringFromECInstanceId(source.GetECInstanceId());
        infoJson[CLASS_CachedRelationshipInfo_PROPERTY_TargetInstanceId] = ECDbHelper::StringFromECInstanceId(target.GetECInstanceId());
        }

    return RelationshipInfo(infoJson, &relationshipClass, relationshipInstanceId, m_cachedRelationshipInfoClass->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
RelationshipInfo RelationshipInfoManager::FindInfo(ECInstanceKeyCR relationshipKey)
    {
    ECRelationshipClassCP relationshipClass = m_dbAdapter->GetECRelationshipClass(relationshipKey.GetECClassId());
    if (nullptr == relationshipClass)
        {
        return RelationshipInfo();
        }

    auto statement = m_statementCache->GetPreparedStatement("RelationshipInfoManager::FindInfoByECInstanceKey", [&]
        {
        return
            "SELECT info.* "
            "FROM ONLY " ECSql_CachedRelationshipInfoClass " info "
            "WHERE info.[" CLASS_CachedRelationshipInfo_PROPERTY_RelClassId "] = ? "
            "  AND info.[" CLASS_CachedRelationshipInfo_PROPERTY_RelInstanceId "] = ? "
            "LIMIT 1 ";
        });

    statement->BindInt64(1, relationshipClass->GetId());
    statement->BindId(2, relationshipKey.GetECInstanceId());

    ECSqlStepStatus status = statement->Step();
    if (status != ECSqlStepStatus::HasRow)
        {
        return RelationshipInfo();
        }

    Json::Value infoJson;
    JsonECSqlSelectAdapter adapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    adapter.GetRowInstance(infoJson, m_cachedRelationshipInfoClass->GetId());

    return RelationshipInfo(infoJson, relationshipClass, relationshipKey.GetECInstanceId(), m_cachedRelationshipInfoClass->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
RelationshipInfo RelationshipInfoManager::ParseInfo(JsonValueCR infoJson)
    {
    ECClassId classId = BeJsonUtilities::Int64FromValue(infoJson[CLASS_CachedRelationshipInfo_PROPERTY_RelClassId]);
    ECRelationshipClassCP relationshipClass = m_dbAdapter->GetECRelationshipClass(classId);
    ECInstanceId relationshipInstanceId = ECDbHelper::ECInstanceIdFromJsonValue(infoJson[CLASS_CachedRelationshipInfo_PROPERTY_RelInstanceId]);
    return RelationshipInfo(infoJson, relationshipClass, relationshipInstanceId, m_cachedRelationshipInfoClass->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ObjectId RelationshipInfoManager::ReadObjectId(ECInstanceKeyCR relationship)
    {
    ECRelationshipClassCP relationshipClass = m_dbAdapter->GetECRelationshipClass(relationship);
    if (nullptr == relationshipClass)
        {
        return ObjectId();
        }

    auto statement = m_statementCache->GetPreparedStatement("RelationshipInfoManager::ReadObjectId", [&]
        {
        return
            "SELECT info.[" CLASS_CachedRelationshipInfo_PROPERTY_RemoteId "] "
            "FROM ONLY " ECSql_CachedRelationshipInfoClass " info "
            "WHERE info.[" CLASS_CachedRelationshipInfo_PROPERTY_RelClassId "] = ? "
            "  AND info.[" CLASS_CachedRelationshipInfo_PROPERTY_RelInstanceId "] = ? "
            "LIMIT 1 ";
        });

    statement->BindInt64(1, relationship.GetECClassId());
    statement->BindId(2, relationship.GetECInstanceId());

    ECSqlStepStatus status = statement->Step();
    if (status != ECSqlStepStatus::HasRow)
        {
        return ObjectId();
        }

    return ObjectId(*relationshipClass, statement->GetValueText(0));;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus RelationshipInfoManager::SaveInfo(RelationshipInfoR info)
    {
    ECRelationshipClassCP relationshipClass = m_dbAdapter->GetECRelationshipClass(info.GetRelationshipKey());
    if (nullptr == relationshipClass)
        {
        BeAssert(false);
        return ERROR;
        }
    if (info.IsInCache())
        {
        if (SUCCESS != m_infoUpdater.Get().Update(info.GetJsonInfo()))
            {
            return ERROR;
            }
        }
    else
        {
        if (SUCCESS != m_infoInserter.Get().Insert(info.GetJsonInfo()))
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipInfoManager::ReadRelationshipEnds
(
ECInstanceKeyCR relationship,
ECInstanceKeyR sourceOut,
ECInstanceKeyR targetOut
)
    {
    ECRelationshipClassCP relationshipClass = m_dbAdapter->GetECRelationshipClass(relationship);
    if (!relationship.IsValid() || nullptr == relationshipClass)
        {
        return ERROR;
        }

    Utf8PrintfString key("RelationshipInfoManager::ReadRelationshipEnds:%lld", relationship.GetECClassId());
    auto statement = m_statementCache->GetPreparedStatement(key, [&]
        {
        return Utf8PrintfString
            (
            "SELECT rel.SourceECClassId, rel.SourceECInstanceId, rel.TargetECClassId, rel.TargetECInstanceId "
            "FROM ONLY %s rel "
            "WHERE rel.[ECInstanceId] = ? "
            "LIMIT 1 ",
            ECSqlBuilder::ToECSqlSnippet(*relationshipClass).c_str()
            );
        });

    statement->BindId(1, relationship.GetECInstanceId());

    ECSqlStepStatus status = statement->Step();
    if (ECSqlStepStatus::HasRow != status)
        {
        return ERROR;
        }

    sourceOut = ECInstanceKey(statement->GetValueInt64(0), statement->GetValueId<ECInstanceId>(1));
    targetOut = ECInstanceKey(statement->GetValueInt64(2), statement->GetValueId<ECInstanceId>(3));

    if (!sourceOut.IsValid() || !targetOut.IsValid())
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedRelationshipKey RelationshipInfoManager::ReadCachedRelationshipKey
(
ECInstanceKeyCR relationship,
Utf8StringCR remoteId
)
    {
    if (!relationship.IsValid())
        {
        return CachedRelationshipKey();
        }

    auto statement = m_statementCache->GetPreparedStatement("RelationshipInfoManager::ReadCachedRelationshipKey", [&]
        {
        return
            "SELECT info.ECInstanceId "
            "FROM ONLY " ECSql_CachedRelationshipInfoClass " info "
            "WHERE info.[" CLASS_CachedRelationshipInfo_PROPERTY_RelClassId "] = ? "
            "  AND info.[" CLASS_CachedRelationshipInfo_PROPERTY_RelInstanceId "] = ?"
            "LIMIT 1";
        });

    statement->BindInt64(1, relationship.GetECClassId());
    statement->BindId(2, relationship.GetECInstanceId());

    ECInstanceId infoId;

    ECSqlStepStatus status = statement->Step();
    if (ECSqlStepStatus::HasRow == status)
        {
        infoId = statement->GetValueId<ECInstanceId>(0);
        }
    else
        {
        IECInstancePtr infoECInstance = m_cachedRelationshipInfoClass->GetDefaultStandaloneEnabler()->CreateInstance();

        infoECInstance->SetValue(WIDEN(CLASS_CachedRelationshipInfo_PROPERTY_RelClassId), ECValue(relationship.GetECClassId()));
        infoECInstance->SetValue(WIDEN(CLASS_CachedRelationshipInfo_PROPERTY_RelInstanceId), ECValue(relationship.GetECInstanceId().GetValue()));
        infoECInstance->SetValue(WIDEN(CLASS_CachedRelationshipInfo_PROPERTY_RemoteId), ECValue(remoteId.c_str(), false));

        ECInstanceKey infoKey;
        if (SUCCESS != m_infoECInstanceInserter.Get().Insert(infoKey, *infoECInstance))
            {
            BeAssert(false);
            return CachedRelationshipKey();
            }
        infoId = infoKey.GetECInstanceId();
        }

    return CachedRelationshipKey(ECInstanceKey(m_cachedRelationshipInfoClass->GetId(), infoId), relationship);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey RelationshipInfoManager::ReadRelationshipKeyByInfo(ECInstanceKeyCR infoKey)
    {
    if (!infoKey.IsValid())
        {
        return ECInstanceKey();
        }

    auto statement = m_statementCache->GetPreparedStatement("RelationshipInfoManager::ReadRelationshipKeyByInfo", [&]
        {
        return
            "SELECT info.[" CLASS_CachedRelationshipInfo_PROPERTY_RelClassId "], "
            "       info.[" CLASS_CachedRelationshipInfo_PROPERTY_RelInstanceId "] "
            "FROM ONLY " ECSql_CachedRelationshipInfoClass " info "
            "WHERE info.ECInstanceId = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, infoKey.GetECInstanceId());

    ECSqlStepStatus status = statement->Step();
    if (ECSqlStepStatus::HasRow != status)
        {
        return ECInstanceKey();
        }

    return ECInstanceKey(statement->GetValueInt64(0), statement->GetValueId<ECInstanceId>(1));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceId RelationshipInfoManager::ReadInfoIdByRelationship(ECInstanceKeyCR relationshipDesc)
    {
    if (!relationshipDesc.IsValid())
        {
        return ECInstanceId();
        }

    auto statement = m_statementCache->GetPreparedStatement("RelationshipInfoManager::ReadInfoIdByRelationship", [&]
        {
        return
            "SELECT info.ECInstanceId "
            "FROM ONLY " ECSql_CachedRelationshipInfoClass " info "
            "WHERE info.[" CLASS_CachedRelationshipInfo_PROPERTY_RelClassId "] = ? "
            "  AND info.[" CLASS_CachedRelationshipInfo_PROPERTY_RelInstanceId "] = ? "
            "LIMIT 1";
        });

    statement->BindInt64(1, relationshipDesc.GetECClassId());
    statement->BindId(2, relationshipDesc.GetECInstanceId());

    ECSqlStepStatus status = statement->Step();
    if (ECSqlStepStatus::HasRow != status)
        {
        return ECInstanceId();
        }

    return statement->GetValueId<ECInstanceId>(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipInfoManager::ReadCachedRelationshipsFromHolder
(
ECInstanceKeyCR holder,
ECRelationshipClassCP holderToInfoRelClass,
bset<CachedRelationshipKey>& cachedRelationshipsOut
)
    {
    ECClassCP holderClass = m_dbAdapter->GetECClass(holder);
    if (!holder.IsValid() || nullptr == holderClass)
        {
        return ERROR;
        }

    Utf8PrintfString key("RelationshipInfoManager::GetRelationshipsForHolder:%lld:%lld", holder.GetECClassId(), holderToInfoRelClass->GetId());
    auto statement = m_statementCache->GetPreparedStatement(key, [&]
        {
        return
            "SELECT "
            "   info.ECInstanceId, "
            "   info.[" CLASS_CachedRelationshipInfo_PROPERTY_RelClassId "], "
            "   info.[" CLASS_CachedRelationshipInfo_PROPERTY_RelInstanceId "] "
            "FROM ONLY " ECSql_CachedRelationshipInfoClass " info "
            "JOIN ONLY " + ECSqlBuilder::ToECSqlSnippet(*holderClass) + " holder "
            "USING " + ECSqlBuilder::ToECSqlSnippet(*holderToInfoRelClass) + " "
            "WHERE holder.ECInstanceId = ? ";
        });

    statement->BindId(1, holder.GetECInstanceId());

    ECSqlStepStatus status;
    while (ECSqlStepStatus::HasRow == (status = statement->Step()))
        {
        ECInstanceKey relInfo(m_cachedRelationshipInfoClass->GetId(), statement->GetValueId<ECInstanceId>(0));
        ECInstanceKey relationship(statement->GetValueInt64(1), statement->GetValueId<ECInstanceId>(2));

        if (!relInfo.IsValid() || !relationship.IsValid())
            {
            return ERROR;
            }

        cachedRelationshipsOut.insert({relInfo, relationship});
        }

    return ECSqlStepStatus::Done == status ? SUCCESS : ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipInfoManager::RelateCachedRelationshipsToHolder
(
ECInstanceKeyCR holder,
ECRelationshipClassCP holderToInfoRelClass,
const bset<CachedRelationshipKey>& cachedRelationships
)
    {
    for (auto& cachedRelationship : cachedRelationships)
        {
        if (!m_hierarchyManager->RelateInstances(holder, cachedRelationship.GetInfoKey(), holderToInfoRelClass).IsValid())
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipInfoManager::RemoveCachedRelationshipsFromHolder
(
ECInstanceKeyCR holder,
ECRelationshipClassCP holderToInfoRelClass,
const bset<CachedRelationshipKey>& cachedRelationshipsToRemove
)
    {
    for (auto& relInfo : cachedRelationshipsToRemove)
        {
        if (SUCCESS != m_hierarchyManager->DeleteRelationship(holder, relInfo.GetInfoKey(), holderToInfoRelClass))
            {
            return ERROR;
            }
        if (m_hierarchyManager->IsInstanceHeldByOtherInstances(relInfo.GetInfoKey()))
            {
            continue;
            }
        if (SUCCESS != DeleteCachedRelationship(relInfo))
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipInfoManager::DeleteRelationshipLeavingInfo(RelationshipInfoR info)
    {
    ECRelationshipClassCP relClass = m_dbAdapter->GetECRelationshipClass(info.GetRelationshipKey());
    if (nullptr == relClass)
        {
        return ERROR;
        }

    ECPersistencePtr persistence = m_dbAdapter->GetECDb().GetEC().GetECPersistence(nullptr, *relClass);
    if (persistence.IsNull() || DELETE_Success != persistence->Delete(ECInstanceId(info.GetRelationshipKey().GetECInstanceId())))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipInfoManager::DeleteCachedRelationship(const CachedRelationshipKey& relationship)
    {
    if (SUCCESS != m_hierarchyManager->DeleteInstance(relationship.GetInfoKey()))
        {
        return ERROR;
        }
    if (SUCCESS != m_hierarchyManager->DeleteRelationship(relationship.GetRelationshipKey()))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipInfoManager::OnBeforeDelete(ECN::ECClassCR ecClass, ECInstanceId ecInstanceId)
    {
    // TODO: optimize when ECDb has API for knowing all instances that will be deleted - no need to delete seperately if it will be deleted.

    // Remove relationship that is held by CachedRelationshipInfo
    if (&ecClass == m_cachedRelationshipInfoClass)
        {
        ECInstanceKey relInfoKey(m_cachedRelationshipInfoClass->GetId(), ecInstanceId);
        ECInstanceKey relationship = ReadRelationshipKeyByInfo(relInfoKey);

        ECRelationshipClassCP relClass = m_dbAdapter->GetECRelationshipClass(relationship);
        ECPersistencePtr persistence = m_dbAdapter->GetECDb().GetEC().GetECPersistence(nullptr, *relClass);
        if (persistence.IsNull() || DELETE_Success != persistence->Delete(ECInstanceId(relationship.GetECInstanceId())))
            {
            return ERROR;
            }
        return SUCCESS;
        }

    // Remove CachedRelationshipInfo for relationship
    if (ecClass.GetRelationshipClassCP() != nullptr &&
        !ecClass.GetSchema().GetName().Equals(WIDEN(SCHEMA_CacheSchema)))
        {
        ECInstanceKey relationship(ecClass.GetId(), ecInstanceId);
        ECInstanceId relInfoId = ReadInfoIdByRelationship(relationship);
        if (!relInfoId.IsValid())
            {
            // Nothing to delete
            return SUCCESS;
            }
        ECPersistencePtr persistence = m_dbAdapter->GetECDb().GetEC().GetECPersistence(nullptr, *m_cachedRelationshipInfoClass);
        if (persistence.IsNull() || DELETE_Success != persistence->Delete(relInfoId))
            {
            return ERROR;
            }
        return SUCCESS;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipInfoManager::OnAfterDelete(bset<ECInstanceKey>& instancesToDeleteOut)
    {
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipInfoManager::RemoveAllCachedRelationships()
    {
    auto statement = m_statementCache->GetPreparedStatement("RelationshipInfoManager::RemoveAllCachedRelationships", [&]
        {
        return "SELECT info.GetECClassId(), info.ECInstanceId FROM ONLY " ECSql_CachedRelationshipInfoClass " info ";
        });

    return m_hierarchyManager->DeleteInstances(*statement);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedRelationshipKey::CachedRelationshipKey()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedRelationshipKey::CachedRelationshipKey(ECInstanceKeyCR infoKey, ECInstanceKey relationshipKey) :
m_infoKey(infoKey),
m_relationshipKey(relationshipKey)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool CachedRelationshipKey::operator < (const CachedRelationshipKey& other) const
    {
    if (m_infoKey < other.m_infoKey)
        {
        return true;
        }
    if (other.m_infoKey < m_infoKey)
        {
        return false;
        }
    return m_relationshipKey < other.m_relationshipKey;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool CachedRelationshipKey::IsInCache() const
    {
    return m_infoKey.IsValid() && m_relationshipKey.IsValid();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKeyCR CachedRelationshipKey::GetInfoKey() const
    {
    return m_infoKey;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKeyCR CachedRelationshipKey::GetRelationshipKey() const
    {
    return m_relationshipKey;
    }
