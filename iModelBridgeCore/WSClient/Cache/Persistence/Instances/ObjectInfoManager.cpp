/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Instances/ObjectInfoManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ObjectInfoManager.h"

#include <WebServices/Cache/Util/ECDbHelper.h>

#include "../../Logging.h"
#include "../Core/CacheSchema.h"
#include "../Hierarchy/HierarchyManager.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectInfoManager::ObjectInfoManager
(
ECDbAdapter& dbAdapter,
WebServices::ECSqlStatementCache& statementCache,
HierarchyManager& hierarchyManager
) :
m_dbAdapter(dbAdapter),
m_statementCache(statementCache),
m_hierarchyManager(hierarchyManager),

m_infoClass(m_dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_CachedObjectInfo)),

m_infoInserter(m_dbAdapter.GetECDb(), *m_infoClass),
m_infoUpdater(m_dbAdapter.GetECDb(), *m_infoClass)
    {
    m_dbAdapter.RegisterDeleteListener(this);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectInfoManager::~ObjectInfoManager()
    {
    m_dbAdapter.UnRegisterDeleteListener(this);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ObjectInfoManager::GetInfoClass() const
    {
    return m_infoClass;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ObjectInfoManager::InsertInfo(ObjectInfoR info)
    {
    if (!info.GetInstanceKey().IsValid())
        {
        BeAssert(false && "ECInstanceId or ECClassId is not set");
        return ERROR;
        }
    if (info.IsInCache())
        {
        BeAssert(false && "Cannot insert duplicates");
        return ERROR;
        }
    if (SUCCESS != m_infoInserter.Get().Insert(info.GetJsonInfo()))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ObjectInfoManager::UpdateInfo(ObjectInfoCR info)
    {
    if (!info.IsInCache())
        {
        BeAssert(false && "Cannot update nonexisting info");
        return ERROR;
        }
    if (SUCCESS != m_infoUpdater.Get().Update(info.GetJsonInfo()))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ObjectInfo ObjectInfoManager::ReadInfo(ObjectIdCR objectId)
    {
    ECClassCP ecClass = m_dbAdapter.GetECClass(objectId);
    if (nullptr == ecClass)
        {
        LOG.errorv("Class for object id '%s' not found", objectId.ToString().c_str());
        BeAssert(false);
        return ObjectInfo();
        }
    return ReadInfo(*ecClass, objectId.remoteId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ObjectInfo ObjectInfoManager::ReadInfo(ECClassCR ecClass, Utf8StringCR remoteId)
    {
    auto statement = m_statementCache.GetPreparedStatement("ObjectInfoManager::ReadInfoByObjectId", [&]
        {
        return
            "SELECT info.* "
            "FROM ONLY " ECSql_CachedObjectInfo " info "
            "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_ClassId "] = ? "
            "  AND info.[" CLASS_CachedObjectInfo_PROPERTY_RemoteId "] = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, ecClass.GetId());
    statement->BindText(2, remoteId.c_str(), IECSqlBinder::MakeCopy::No);

    Json::Value infoJson;

    DbResult status = statement->Step();
    if (status == BE_SQLITE_ROW)
        {
        JsonECSqlSelectAdapter adapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
        adapter.GetRowInstance(infoJson, m_infoClass->GetId());
        }
    else
        {
        // Return valid info object so it could be saved
        infoJson[CLASS_CachedObjectInfo_PROPERTY_ClassId] = BeJsonUtilities::StringValueFromInt64(ecClass.GetId().GetValue());
        infoJson[CLASS_CachedObjectInfo_PROPERTY_RemoteId] = remoteId;
        }

    return ObjectInfo(infoJson, &ecClass, m_infoClass->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectInfo ObjectInfoManager::ReadInfo(ECInstanceKeyCR instanceKey)
    {
    ECClassCP objectClass = m_dbAdapter.GetECClass(instanceKey);
    if (nullptr == objectClass)
        {
        BeAssert(false);
        return ObjectInfo();
        }

    auto statement = m_statementCache.GetPreparedStatement("ObjectInfoManager::ReadInfoByInstanceKey", [&]
        {
        return
            "SELECT info.* "
            "FROM ONLY " ECSql_CachedObjectInfo " info "
            "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_ClassId "] = ? "
            "  AND info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceId "] = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, objectClass->GetId());
    statement->BindId(2, instanceKey.GetECInstanceId());

    DbResult status = statement->Step();
    if (status != BE_SQLITE_ROW)
        {
        return ObjectInfo();
        }

    Json::Value infoJson;
    JsonECSqlSelectAdapter adapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    adapter.GetRowInstance(infoJson, m_infoClass->GetId());

    return ObjectInfo(infoJson, objectClass, m_infoClass->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ObjectInfo ObjectInfoManager::ReadInfo(JsonValueCR infoJson)
    {
    ECClassCP instanceClass = m_dbAdapter.GetECClass(ECClassId((uint64_t) BeJsonUtilities::Int64FromValue(infoJson[CLASS_CachedObjectInfo_PROPERTY_ClassId])));
    return ObjectInfo(infoJson, instanceClass, m_infoClass->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECInstanceKey ObjectInfoManager::FindCachedInstance(ObjectIdCR objectId)
    {
    ECClassCP ecClass = m_dbAdapter.GetECClass(objectId);
    return FindCachedInstance(ecClass, objectId.remoteId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECInstanceKey ObjectInfoManager::FindCachedInstance(ECClassCP ecClass, Utf8StringCR remoteId)
    {
    if (nullptr == ecClass)
        {
        return ECInstanceKey();
        }

    auto statement = m_statementCache.GetPreparedStatement("ObjectInfoManager::FindCachedInstanceByObjectId", [&]
        {
        return
            "SELECT info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceId "] "
            "FROM ONLY " ECSql_CachedObjectInfo " info "
            "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_ClassId "] = ? "
            "  AND info.[" CLASS_CachedObjectInfo_PROPERTY_RemoteId "] = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, ecClass->GetId());
    statement->BindText(2, remoteId.c_str(), IECSqlBinder::MakeCopy::No);

    DbResult status = statement->Step();

    ECInstanceId instanceId;
    if (status == BE_SQLITE_ROW)
        {
        instanceId = statement->GetValueId<ECInstanceId>(0);
        }

    return ECInstanceKey(ecClass->GetId(), instanceId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ObjectId ObjectInfoManager::FindCachedInstance(ECInstanceKeyCR instanceKey)
    {
    ECClassCP objectClass = m_dbAdapter.GetECClass(instanceKey);
    if (nullptr == objectClass)
        {
        return ObjectId();
        }

    auto statement = m_statementCache.GetPreparedStatement("ObjectInfoManager::FindCachedInstanceByInstanceKey", [&]
        {
        return
            "SELECT info.[" CLASS_CachedObjectInfo_PROPERTY_RemoteId "] "
            "FROM ONLY " ECSql_CachedObjectInfo " info "
            "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_ClassId "] = ? "
            "  AND info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceId "] = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, objectClass->GetId());
    statement->BindId(2, instanceKey.GetECInstanceId());

    DbResult status = statement->Step();
    if (status != BE_SQLITE_ROW)
        {
        return ObjectId();
        }

    return ObjectId(*objectClass, statement->GetValueText(0));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ObjectInfoManager::DeleteInstanceLeavingInfo(ObjectInfoR info)
    {
    return m_hierarchyManager.DeleteInstance(info.GetInstanceKey());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
CachedObjectInfoKey ObjectInfoManager::ReadInfoKey(ECInstanceKeyCR instanceKey)
    {
    Utf8PrintfString key("ObjectInfoManager::ReadInfoKey:ECInstanceKey");
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        return
            "SELECT info.ECInstanceId "
            "FROM ONLY " ECSql_CachedObjectInfo " info "
            "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_ClassId "] = ? "
            "  AND info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceId "] = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, instanceKey.GetECClassId());
    statement->BindId(2, instanceKey.GetECInstanceId());

    DbResult status = statement->Step();
    if (status != BE_SQLITE_ROW)
        {
        return CachedObjectInfoKey();
        }

    return CachedObjectInfoKey(m_infoClass->GetId(), statement->GetValueId<ECInstanceId>(0));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
CachedInstanceKey ObjectInfoManager::ReadCachedInstanceKey(ECInstanceKeyCR instanceKey)
    {
    return CachedInstanceKey(ReadInfoKey(instanceKey), instanceKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
CachedInstanceKey ObjectInfoManager::ReadCachedInstanceKey(ObjectIdCR objectId)
    {
    ECClassCP objectClass = m_dbAdapter.GetECClass(objectId);
    if (nullptr == objectClass)
        {
        return CachedInstanceKey();
        }

    Utf8PrintfString key("ObjectInfoManager::ReadCachedInstanceKey:ObjectId");
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        return
            "SELECT "
            "   info.ECInstanceId, "
            "   info.[" CLASS_CachedObjectInfo_PROPERTY_ClassId "], "
            "   info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceId "] "
            "FROM ONLY " ECSql_CachedObjectInfo " info "
            "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_ClassId "] = ? "
            "  AND info.[" CLASS_CachedObjectInfo_PROPERTY_RemoteId "] = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, objectClass->GetId());
    statement->BindText(2, objectId.remoteId.c_str(), IECSqlBinder::MakeCopy::No);

    DbResult status = statement->Step();
    if (status != BE_SQLITE_ROW)
        {
        return CachedInstanceKey();
        }

    CacheNodeKey infoKey(m_infoClass->GetId(), statement->GetValueId<ECInstanceId>(0));
    ECInstanceKey instanceKey(statement->GetValueId<ECClassId>(1), statement->GetValueId<ECInstanceId>(2));
    return CachedInstanceKey(infoKey, instanceKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
CachedInstanceKey ObjectInfoManager::ReadCachedInstanceKey(CacheNodeKeyCR infoKey)
    {
    Utf8PrintfString key("ObjectInfoManager::ReadCachedInstanceKey:InfoKey");
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        return
            "SELECT "
            "   info.[" CLASS_CachedObjectInfo_PROPERTY_ClassId "], "
            "   info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceId "] "
            "FROM ONLY " ECSql_CachedObjectInfo " info "
            "WHERE info.ECInstanceId = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, infoKey.GetECInstanceId());

    DbResult status = statement->Step();
    if (status != BE_SQLITE_ROW)
        {
        return CachedInstanceKey();
        }

    ECInstanceKey instanceKey(statement->GetValueId<ECClassId>(0), statement->GetValueId<ECInstanceId>(1));
    return CachedInstanceKey(infoKey, instanceKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
CachedInstanceKey ObjectInfoManager::ReadCachedInstanceKey(CacheNodeKeyCR relatedKey, ECRelationshipClassCR relClass)
    {
    ECClassCP relatedClass = m_dbAdapter.GetECClass(relatedKey);
    if (nullptr == relatedClass)
        {
        return CachedInstanceKey();
        }

    Utf8PrintfString key("ObjectInfoManager::ReadCachedInstanceKey:%llu:%llu", relatedClass->GetId().GetValue(), relClass.GetId().GetValue());
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        return
            "SELECT "
            "   info.ECInstanceId, "
            "   info.[" CLASS_CachedObjectInfo_PROPERTY_ClassId "], "
            "   info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceId "] "
            "FROM ONLY " ECSql_CachedObjectInfo " info "
            "JOIN " + relatedClass->GetECSqlName() + " related USING " + relClass.GetECSqlName() + " "
            "WHERE related.ECInstanceId = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, relatedKey.GetECInstanceId());

    DbResult status = statement->Step();
    if (status != BE_SQLITE_ROW)
        {
        return CachedInstanceKey();
        }

    CacheNodeKey infoKey(m_infoClass->GetId(), statement->GetValueId<ECInstanceId>(0));
    ECInstanceKey instanceKey(statement->GetValueId<ECClassId>(1), statement->GetValueId<ECInstanceId>(2));
    return CachedInstanceKey(infoKey, instanceKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ObjectInfoManager::ReadCachedInstanceKeys(const ECInstanceKeyMultiMap& infoKeys, ECInstanceKeyMultiMap& instanceKeysOut)
    {
    auto statement = m_statementCache.GetPreparedStatement("ObjectInfoManager::ReadCachedInstanceKeys", [&]
        {
        return
            "SELECT info.[" CLASS_CachedObjectInfo_PROPERTY_ClassId "], info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceId "] "
            "FROM ONLY " ECSql_CachedObjectInfo " info "
            "WHERE ECInstanceId = ? "
            "LIMIT 1 ";
        });

    auto range = infoKeys.equal_range(m_infoClass->GetId());
    for (auto it = range.first; it != range.second; ++it)
        {
        statement->Reset();
        statement->BindId(1, it->second);

        if (BE_SQLITE_ROW != statement->Step())
            return ERROR;

        instanceKeysOut.insert({statement->GetValueId<ECClassId>(0), statement->GetValueId<ECInstanceId>(1)});
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ObjectInfoManager::ReadCachedInstanceKeys
(
CacheNodeKeyCR relatedKey,
ECRelationshipClassCR relClass,
ECInstanceKeyMultiMap& instanceKeysOut
)
    {
    ECClassCP relatedClass = m_dbAdapter.GetECClass(relatedKey);
    if (nullptr == relatedClass)
        {
        return ERROR;
        }

    Utf8PrintfString key("ObjectInfoManager::ReadCachedInstanceKeys:Map:%llu:%llu", relatedClass->GetId().GetValue(), relClass.GetId().GetValue());
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        return
            "SELECT "
            "   info.[" CLASS_CachedObjectInfo_PROPERTY_ClassId "], "
            "   info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceId "] "
            "FROM ONLY " ECSql_CachedObjectInfo " info "
            "JOIN " + relatedClass->GetECSqlName() + " related USING " + relClass.GetECSqlName() + " "
            "WHERE related.ECInstanceId = ? ";
        });

    statement->BindId(1, relatedKey.GetECInstanceId());

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement->Step()))
        {
        instanceKeysOut.Insert(statement->GetValueId<ECClassId>(0), statement->GetValueId<ECInstanceId>(1));
        }
    if (BE_SQLITE_DONE != status)
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ObjectInfoManager::ReadCachedInstanceKeys
(
CacheNodeKeyCR relatedKey,
ECRelationshipClassCR relClass,
bset<CachedInstanceKey>& instanceKeysOut
)
    {
    ECClassCP relatedClass = m_dbAdapter.GetECClass(relatedKey);
    if (nullptr == relatedClass)
        {
        return ERROR;
        }

    Utf8PrintfString key("ObjectInfoManager::ReadCachedInstanceKeys:Set:%llu:%llu", relatedClass->GetId().GetValue(), relClass.GetId().GetValue());
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        return
            "SELECT "
            "   info.GetECClassId(), "
            "   info.ECInstanceId, "
            "   info.[" CLASS_CachedObjectInfo_PROPERTY_ClassId "], "
            "   info.[" CLASS_CachedObjectInfo_PROPERTY_InstanceId "] "
            "FROM ONLY " ECSql_CachedObjectInfo " info "
            "JOIN " + relatedClass->GetECSqlName() + " related USING " + relClass.GetECSqlName() + " "
            "WHERE related.ECInstanceId = ? ";
        });

    statement->BindId(1, relatedKey.GetECInstanceId());

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement->Step()))
        {
        CacheNodeKey infoKey(statement->GetValueId<ECClassId>(0), statement->GetValueId<ECInstanceId>(1));
        ECInstanceKey instanceKey(statement->GetValueId<ECClassId>(2), statement->GetValueId<ECInstanceId>(3));
        instanceKeysOut.insert(CachedInstanceKey(infoKey, instanceKey));
        }
    if (BE_SQLITE_DONE != status)
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ObjectInfoManager::ReadCachedInstanceIds(CacheNodeKeyCR relatedKey, ECRelationshipClassCR relClass, bset<ObjectId>& idsOut)
    {
    ECClassCP relatedClass = m_dbAdapter.GetECClass(relatedKey);
    if (nullptr == relatedClass)
        {
        return ERROR;
        }

    Utf8PrintfString key("ObjectInfoManager::ReadCachedInstanceIds:%llu:%llu", relatedClass->GetId().GetValue(), relClass.GetId().GetValue());
    auto statement = m_statementCache.GetPreparedStatement(key, [&]
        {
        return
            "SELECT "
            "   info.[" CLASS_CachedObjectInfo_PROPERTY_ClassId "], "
            "   info.[" CLASS_CachedObjectInfo_PROPERTY_RemoteId "] "
            "FROM ONLY " ECSql_CachedObjectInfo " info "
            "JOIN " + relatedClass->GetECSqlName() + " related USING " + relClass.GetECSqlName() + " "
            "WHERE related.ECInstanceId = ? ";
        });

    statement->BindId(1, relatedKey.GetECInstanceId());

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement->Step()))
        {
        ECClassCP instanceClass = m_dbAdapter.GetECClass(statement->GetValueId<ECClassId>(0));
        if (nullptr == instanceClass)
            {
            return ERROR;
            }
        idsOut.insert(ObjectId(*instanceClass, statement->GetValueText(1)));
        }
    if (BE_SQLITE_DONE != status)
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ObjectInfoManager::RemoveAllCachedInstances()
    {
    auto statement = m_statementCache.GetPreparedStatement("ObjectInfoManager::RemoveAllCachedInstances:CachedInstances", [&]
        {
        return "SELECT info.ClassId, info.InstanceId FROM ONLY " ECSql_CachedObjectInfo " info ";
        });

    if (SUCCESS != m_hierarchyManager.DeleteInstances(*statement))
        {
        return ERROR;
        }

    statement = m_statementCache.GetPreparedStatement("ObjectInfoManager::RemoveAllCachedInstances:ObjectInfos", [&]
        {
        return "SELECT info.GetECClassId(), info.ECInstanceId FROM ONLY " ECSql_CachedObjectInfo " info ";
        });

    return m_hierarchyManager.DeleteInstances(*statement);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey ObjectInfoManager::ConvertToInstanceKey(ECInstanceKeyCR instanceKey)
    {
    if (m_infoClass->GetId() != instanceKey.GetECClassId())
        return instanceKey;

    CacheNodeKey infoKey (instanceKey);
    return ReadCachedInstanceKey(infoKey).GetInstanceKey();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ObjectInfoManager::OnBeforeDelete(ECN::ECClassCR ecClass, ECInstanceId ecInstanceId, bset<ECInstanceKey>& additionalInstancesOut)
    {
    if (&ecClass != m_infoClass)
        {
        return SUCCESS;
        }

    CacheNodeKey infoKey(m_infoClass->GetId(), ecInstanceId);
    ECInstanceKey instance = ReadCachedInstanceKey(infoKey).GetInstanceKey();
    if (instance.IsValid())
        {
        additionalInstancesOut.insert(instance);
        }

    return SUCCESS;
    }