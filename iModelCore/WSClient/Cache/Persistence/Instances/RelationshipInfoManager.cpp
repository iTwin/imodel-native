/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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
WebServices::ECSqlStatementCache& statementCache,
HierarchyManager& hierarchyManager
) :
m_dbAdapter(dbAdapter),
m_statementCache(statementCache),
m_hierarchyManager(hierarchyManager),

m_cachedRelationshipInfoClass(m_dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_CachedRelationshipInfo)),

m_infoECInstanceInserter(m_dbAdapter.GetECDb(), *m_cachedRelationshipInfoClass),
m_infoInserter(m_dbAdapter.GetECDb(), *m_cachedRelationshipInfoClass),
m_infoUpdater(m_dbAdapter.GetECDb(), *m_cachedRelationshipInfoClass, ECSqlUpdater_Options_IgnoreSystemAndFailReadOnlyProperties)
    {
    dbAdapter.RegisterDeleteListener(this);
    }

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

    auto statement = m_statementCache.GetPreparedStatement("RelationshipInfoManager::ReadInfoByEnds", [&]
        {
        return
            "SELECT info.* "
            "FROM ONLY " ECSql_CachedRelationshipInfo " info "
            "WHERE info.[" CLASS_CachedRelationshipInfo_PROPERTY_ClassId "] = ? "
            "  AND info.[" CLASS_CachedRelationshipInfo_PROPERTY_SourceClassId "] = ? "
            "  AND info.[" CLASS_CachedRelationshipInfo_PROPERTY_SourceInstanceId "] = ? "
            "  AND info.[" CLASS_CachedRelationshipInfo_PROPERTY_TargetClassId "] = ? "
            "  AND info.[" CLASS_CachedRelationshipInfo_PROPERTY_TargetInstanceId "] = ? "
            "LIMIT 1";
        });

    statement->BindId(1, relationshipClass.GetId());
    statement->BindId(2, source.GetClassId());
    statement->BindId(3, source.GetInstanceId());
    statement->BindId(4, target.GetClassId());
    statement->BindId(5, target.GetInstanceId());

    ECInstanceId relationshipInstanceId;
    Json::Value infoJson;

    DbResult status = statement->Step();
    if (BE_SQLITE_ROW == status)
        {
        JsonECSqlSelectAdapter adapter(*statement);
        adapter.GetRow(infoJson);
        relationshipInstanceId = ECDbHelper::ECInstanceIdFromJsonValue(infoJson[CLASS_CachedRelationshipInfo_PROPERTY_InstanceId]);
        }
    else
        {
        // Return valid info object so it could be saved
        infoJson[CLASS_CachedRelationshipInfo_PROPERTY_ClassId] = BeJsonUtilities::StringValueFromInt64(relationshipClass.GetId().GetValue());
        infoJson[CLASS_CachedRelationshipInfo_PROPERTY_SourceClassId] = BeJsonUtilities::StringValueFromInt64(source.GetClassId().GetValue());
        infoJson[CLASS_CachedRelationshipInfo_PROPERTY_TargetClassId] = BeJsonUtilities::StringValueFromInt64(target.GetClassId().GetValue());
        infoJson[CLASS_CachedRelationshipInfo_PROPERTY_SourceInstanceId] = ECDbHelper::StringFromECInstanceId(source.GetInstanceId());
        infoJson[CLASS_CachedRelationshipInfo_PROPERTY_TargetInstanceId] = ECDbHelper::StringFromECInstanceId(target.GetInstanceId());
        }

    return RelationshipInfo(infoJson, &relationshipClass, relationshipInstanceId, m_cachedRelationshipInfoClass->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
RelationshipInfo RelationshipInfoManager::FindInfo(ECInstanceKeyCR relationshipKey)
    {
    ECRelationshipClassCP relationshipClass = m_dbAdapter.GetECRelationshipClass(relationshipKey.GetClassId());
    if (nullptr == relationshipClass)
        {
        return RelationshipInfo();
        }

    auto statement = m_statementCache.GetPreparedStatement("RelationshipInfoManager::FindInfoByECInstanceKey", [&]
        {
        return
            "SELECT info.* "
            "FROM ONLY " ECSql_CachedRelationshipInfo " info "
            "WHERE info.[" CLASS_CachedRelationshipInfo_PROPERTY_ClassId "] = ? "
            "  AND info.[" CLASS_CachedRelationshipInfo_PROPERTY_InstanceId "] = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, relationshipClass->GetId());
    statement->BindId(2, relationshipKey.GetInstanceId());

    DbResult status = statement->Step();
    if (status != BE_SQLITE_ROW)
        {
        return RelationshipInfo();
        }

    Json::Value infoJson;
    JsonECSqlSelectAdapter adapter(*statement);
    adapter.GetRow(infoJson);

    return RelationshipInfo(infoJson, relationshipClass, relationshipKey.GetInstanceId(), m_cachedRelationshipInfoClass->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
RelationshipInfo RelationshipInfoManager::ParseInfo(JsonValueCR infoJson)
    {
    ECClassId classId((uint64_t) BeJsonUtilities::Int64FromValue(infoJson[CLASS_CachedRelationshipInfo_PROPERTY_ClassId]));
    ECRelationshipClassCP relationshipClass = m_dbAdapter.GetECRelationshipClass(classId);
    ECInstanceId relationshipInstanceId = ECDbHelper::ECInstanceIdFromJsonValue(infoJson[CLASS_CachedRelationshipInfo_PROPERTY_InstanceId]);
    return RelationshipInfo(infoJson, relationshipClass, relationshipInstanceId, m_cachedRelationshipInfoClass->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ObjectId RelationshipInfoManager::ReadObjectId(ECInstanceKeyCR relationship)
    {
    ECRelationshipClassCP relationshipClass = m_dbAdapter.GetECRelationshipClass(relationship);
    if (nullptr == relationshipClass)
        {
        return ObjectId();
        }

    auto statement = m_statementCache.GetPreparedStatement("RelationshipInfoManager::ReadObjectId", [&]
        {
        return
            "SELECT info.[" CLASS_CachedRelationshipInfo_PROPERTY_RemoteId "] "
            "FROM ONLY " ECSql_CachedRelationshipInfo " info "
            "WHERE info.[" CLASS_CachedRelationshipInfo_PROPERTY_ClassId "] = ? "
            "  AND info.[" CLASS_CachedRelationshipInfo_PROPERTY_InstanceId "] = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, relationship.GetClassId());
    statement->BindId(2, relationship.GetInstanceId());

    DbResult status = statement->Step();
    if (status != BE_SQLITE_ROW)
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
    ECRelationshipClassCP relationshipClass = m_dbAdapter.GetECRelationshipClass(info.GetInstanceKey());
    if (nullptr == relationshipClass)
        {
        BeAssert(false);
        return ERROR;
        }
    if (info.IsInCache())
        {
        if (BE_SQLITE_OK != m_infoUpdater.Get().Update(info.GetInfoKey().GetInstanceId(), info.GetJsonInfo()))
            {
            return ERROR;
            }
        }
    else
        {
        if (BE_SQLITE_OK != m_infoInserter.Get().Insert(info.GetJsonInfo()))
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
    ECRelationshipClassCP relationshipClass = m_dbAdapter.GetECRelationshipClass(relationship);
    if (!relationship.IsValid() || nullptr == relationshipClass)
        {
        return ERROR;
        }

    Utf8PrintfString key("RelationshipInfoManager::ReadRelationshipEnds:%s", relationship.GetClassId().ToString().c_str());
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        return Utf8PrintfString
            (
            "SELECT rel.SourceECClassId, rel.SourceECInstanceId, rel.TargetECClassId, rel.TargetECInstanceId "
            "FROM ONLY %s rel "
            "WHERE rel.[ECInstanceId] = ? "
            "LIMIT 1 ",
            relationshipClass->GetECSqlName().c_str()
            );
        });

    statement->BindId(1, relationship.GetInstanceId());

    DbResult status = statement->Step();
    if (BE_SQLITE_ROW != status)
        {
        return ERROR;
        }

    sourceOut = ECInstanceKey(statement->GetValueId<ECClassId>(0), statement->GetValueId<ECInstanceId>(1));
    targetOut = ECInstanceKey(statement->GetValueId<ECClassId>(2), statement->GetValueId<ECInstanceId>(3));

    if (!sourceOut.IsValid() || !targetOut.IsValid())
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedInstanceKey RelationshipInfoManager::ReadOrInsertCachedRelationshipKey
(
ECInstanceKeyCR relationship,
Utf8StringCR remoteId
)
    {
    CachedInstanceKey key = ReadCachedRelationshipKey(relationship);
    if (key.IsValid())
        {
        return key;
        }

    if (!relationship.IsValid())
        {
        return CachedInstanceKey();
        }

    IECInstancePtr infoECInstance = m_cachedRelationshipInfoClass->GetDefaultStandaloneEnabler()->CreateInstance();

    infoECInstance->SetValue(CLASS_CachedRelationshipInfo_PROPERTY_ClassId, ECValue(relationship.GetClassId().GetValue()));
    infoECInstance->SetValue(CLASS_CachedRelationshipInfo_PROPERTY_InstanceId, ECValue(relationship.GetInstanceId().GetValue()));
    infoECInstance->SetValue(CLASS_CachedRelationshipInfo_PROPERTY_RemoteId, ECValue(remoteId.c_str(), false));

    CacheNodeKey infoKey;
    if (BE_SQLITE_OK != m_infoECInstanceInserter.Get().Insert(infoKey, *infoECInstance))
        {
        return CachedInstanceKey();
        }

    return CachedInstanceKey(infoKey, relationship);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedInstanceKey RelationshipInfoManager::ReadCachedRelationshipKey(ECInstanceKeyCR relationship)
    {
    auto statement = m_statementCache.GetPreparedStatement("RelationshipInfoManager::ReadCachedRelationshipKey", [&]
        {
        return
            "SELECT info.ECInstanceId "
            "FROM ONLY " ECSql_CachedRelationshipInfo " info "
            "WHERE info.[" CLASS_CachedRelationshipInfo_PROPERTY_ClassId "] = ? "
            "  AND info.[" CLASS_CachedRelationshipInfo_PROPERTY_InstanceId "] = ?"
            "LIMIT 1";
        });

    statement->BindId(1, relationship.GetClassId());
    statement->BindId(2, relationship.GetInstanceId());

    CacheNodeKey infoKey;

    DbResult status = statement->Step();
    if (BE_SQLITE_ROW == status)
        {
        infoKey = CacheNodeKey(m_cachedRelationshipInfoClass->GetId(), statement->GetValueId<ECInstanceId>(0));
        }

    return CachedInstanceKey(infoKey, relationship);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey RelationshipInfoManager::ReadRelationshipKeyByInfo(CacheNodeKeyCR infoKey)
    {
    if (!infoKey.IsValid())
        {
        return ECInstanceKey();
        }

    auto statement = m_statementCache.GetPreparedStatement("RelationshipInfoManager::ReadRelationshipKeyByInfo", [&]
        {
        return
            "SELECT info.[" CLASS_CachedRelationshipInfo_PROPERTY_ClassId "], "
            "       info.[" CLASS_CachedRelationshipInfo_PROPERTY_InstanceId "] "
            "FROM ONLY " ECSql_CachedRelationshipInfo " info "
            "WHERE info.ECInstanceId = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, infoKey.GetInstanceId());

    DbResult status = statement->Step();
    if (BE_SQLITE_ROW != status)
        {
        return ECInstanceKey();
        }

    return ECInstanceKey(statement->GetValueId<ECClassId>(0), statement->GetValueId<ECInstanceId>(1));
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

    auto statement = m_statementCache.GetPreparedStatement("RelationshipInfoManager::ReadInfoIdByRelationship", [&]
        {
        return
            "SELECT info.ECInstanceId "
            "FROM ONLY " ECSql_CachedRelationshipInfo " info "
            "WHERE info.[" CLASS_CachedRelationshipInfo_PROPERTY_ClassId "] = ? "
            "  AND info.[" CLASS_CachedRelationshipInfo_PROPERTY_InstanceId "] = ? "
            "LIMIT 1";
        });

    statement->BindId(1, relationshipDesc.GetClassId());
    statement->BindId(2, relationshipDesc.GetInstanceId());

    DbResult status = statement->Step();
    if (BE_SQLITE_ROW != status)
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
bset<CachedInstanceKey>& cachedRelationshipsOut
)
    {
    ECClassCP holderClass = m_dbAdapter.GetECClass(holder);
    if (!holder.IsValid() || nullptr == holderClass)
        {
        return ERROR;
        }

    Utf8PrintfString key("RelationshipInfoManager::GetRelationshipsForHolder:%s:%s", holder.GetClassId().ToString().c_str(), holderToInfoRelClass->GetId().ToString().c_str());
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        return
            "SELECT "
            "   info.ECInstanceId, "
            "   info.[" CLASS_CachedRelationshipInfo_PROPERTY_ClassId "], "
            "   info.[" CLASS_CachedRelationshipInfo_PROPERTY_InstanceId "] "
            "FROM ONLY " ECSql_CachedRelationshipInfo " info "
            "JOIN ONLY " + holderClass->GetECSqlName() + " holder "
            "USING " + holderToInfoRelClass->GetECSqlName() + " "
            "WHERE holder.ECInstanceId = ? ";
        });

    statement->BindId(1, holder.GetInstanceId());

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement->Step()))
        {
        CacheNodeKey relInfo(m_cachedRelationshipInfoClass->GetId(), statement->GetValueId<ECInstanceId>(0));
        ECInstanceKey relationship(statement->GetValueId<ECClassId>(1), statement->GetValueId<ECInstanceId>(2));
        CachedInstanceKey cachedKey(relInfo, relationship);

        if (!cachedKey.IsValid())
            {
            return ERROR;
            }

        cachedRelationshipsOut.insert(cachedKey);
        }

    return BE_SQLITE_DONE == status ? SUCCESS : ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipInfoManager::DeleteRelationshipLeavingInfo(RelationshipInfoCR info)
    {
    ECRelationshipClassCP relClass = m_dbAdapter.GetECRelationshipClass(info.GetInstanceKey());
    if (nullptr == relClass)
        {
        return ERROR;
        }

    ECInstanceDeleter deleter(m_dbAdapter.GetECDb(), *relClass, nullptr);

    if (!deleter.IsValid() || BE_SQLITE_OK != deleter.Delete(info.GetInstanceKey().GetInstanceId()))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipInfoManager::DeleteRelationshipOnlyWithInfo(RelationshipInfoCR info)
    {
    if (SUCCESS != DeleteRelationshipLeavingInfo(info))
        return ERROR;

    return m_dbAdapter.DeleteInstance(info.GetInfoKey());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipInfoManager::OnBeforeDelete(ECN::ECClassCR ecClass, ECInstanceId ecInstanceId, bset<ECInstanceKey>& additionalInstancesOut)
    {
    // Remove relationship that is held by CachedRelationshipInfo
    if (&ecClass == m_cachedRelationshipInfoClass)
        {
        CacheNodeKey relInfoKey(m_cachedRelationshipInfoClass->GetId(), ecInstanceId);
        ECInstanceKey relationship = ReadRelationshipKeyByInfo(relInfoKey);
        if (relationship.IsValid())
            {
            additionalInstancesOut.insert(relationship);
            }
        return SUCCESS;
        }

    // Remove CachedRelationshipInfo that manages relationship
    if (ecClass.IsRelationshipClass() && ecClass.GetSchema().GetId() != m_cachedRelationshipInfoClass->GetSchema().GetId())
        {
        ECInstanceKey relationship(ecClass.GetId(), ecInstanceId);
        CacheNodeKey relInfoKey(m_cachedRelationshipInfoClass->GetId(), ReadInfoIdByRelationship(relationship));
        if (relInfoKey.IsValid())
            {
            additionalInstancesOut.insert(relInfoKey);
            }
        return SUCCESS;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipInfoManager::RemoveAllCachedRelationships()
    {
    auto statement = m_statementCache.GetPreparedStatement("RelationshipInfoManager::RemoveAllCachedRelationships", [&]
        {
        return "SELECT info.ECClassId, info.ECInstanceId FROM ONLY " ECSql_CachedRelationshipInfo " info ";
        });

    return m_hierarchyManager.DeleteInstances(*statement);
    }
