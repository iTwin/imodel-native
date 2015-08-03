/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Instances/ObjectInfoManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
ECSqlStatementCache& statementCache,
HierarchyManager& hierarchyManager
) :
m_dbAdapter(&dbAdapter),
m_statementCache(&statementCache),
m_hierarchyManager(&hierarchyManager),

m_infoClass(dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_CachedObjectInfo)),
m_infoRelationshipClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_REL_CachedObjectInfoRelationship)),

m_infoInserter(dbAdapter.GetECDb(), *m_infoClass),
m_infoUpdater(dbAdapter.GetECDb(), *m_infoClass)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ObjectInfoManager::GetInfoClass() const
    {
    return m_infoClass;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP ObjectInfoManager::GetInfoRelationshipClass() const
    {
    return m_infoRelationshipClass;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ObjectInfoManager::InsertInfo(ObjectInfoR info)
    {
    if (!info.GetCachedInstanceKey().IsValid())
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
    if (!m_dbAdapter->RelateInstances(m_infoRelationshipClass, info.GetInfoKey(), info.GetCachedInstanceKey()).IsValid())
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
    ECClassCP ecClass = m_dbAdapter->GetECClass(objectId);
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
    auto statement = m_statementCache->GetPreparedStatement("ObjectInfoManager::ReadInfoByObjectId", [&]
        {
        return
            "SELECT info.* "
            "FROM ONLY " ECSql_CachedObjectInfoClass " info "
            "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_ClassId "] = ? "
            "  AND info.[" CLASS_CachedObjectInfo_PROPERTY_RemoteId "] = ? "
            "LIMIT 1 ";
        });

    statement->BindInt64(1, ecClass.GetId());
    statement->BindText(2, remoteId.c_str(), IECSqlBinder::MakeCopy::No);

    Json::Value infoJson;

    ECSqlStepStatus status = statement->Step();
    if (status == ECSqlStepStatus::HasRow)
        {
        JsonECSqlSelectAdapter adapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
        adapter.GetRowInstance(infoJson, m_infoClass->GetId());
        }
    else
        {
        // Return valid info object so it could be saved
        infoJson[CLASS_CachedObjectInfo_PROPERTY_ClassId] = BeJsonUtilities::StringValueFromInt64(ecClass.GetId());
        infoJson[CLASS_CachedObjectInfo_PROPERTY_RemoteId] = remoteId;
        }

    return ObjectInfo(infoJson, &ecClass, m_infoClass->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectInfo ObjectInfoManager::ReadInfo(ECInstanceKeyCR instanceKey)
    {
    ECClassCP objectClass = m_dbAdapter->GetECClass(instanceKey);
    if (nullptr == objectClass)
        {
        BeAssert(false);
        return ObjectInfo();
        }

    auto statement = m_statementCache->GetPreparedStatement("ObjectInfoManager::ReadInfoByInstanceKey", [&]
        {
        return
            "SELECT info.* "
            "FROM ONLY " ECSql_CachedObjectInfoClass " info "
            "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_ClassId "] = ? "
            "  AND info.[" CLASS_CachedObjectInfo_PROPERTY_LocalId "] = ? "
            "LIMIT 1 ";
        });

    statement->BindInt64(1, objectClass->GetId());
    statement->BindInt64(2, instanceKey.GetECInstanceId().GetValue());

    ECSqlStepStatus status = statement->Step();
    if (status != ECSqlStepStatus::HasRow)
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
    ECClassCP instanceClass = m_dbAdapter->GetECClass(BeJsonUtilities::Int64FromValue(infoJson[CLASS_CachedObjectInfo_PROPERTY_ClassId]));
    return ObjectInfo(infoJson, instanceClass, m_infoClass->GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECInstanceKey ObjectInfoManager::FindCachedInstance(ObjectIdCR objectId)
    {
    ECClassCP ecClass = m_dbAdapter->GetECClass(objectId);
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

    auto statement = m_statementCache->GetPreparedStatement("ObjectInfoManager::FindCachedInstanceByObjectId", [&]
        {
        return
            "SELECT info.[" CLASS_CachedObjectInfo_PROPERTY_LocalId "] "
            "FROM ONLY " ECSql_CachedObjectInfoClass " info "
            "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_ClassId "] = ? "
            "  AND info.[" CLASS_CachedObjectInfo_PROPERTY_RemoteId "] = ? "
            "LIMIT 1 ";
        });

    statement->BindInt64(1, ecClass->GetId());
    statement->BindText(2, remoteId.c_str(), IECSqlBinder::MakeCopy::No);

    ECSqlStepStatus status = statement->Step();
    if (status != ECSqlStepStatus::HasRow)
        {
        return ECInstanceKey();
        }

    return ECInstanceKey(ecClass->GetId(), ECInstanceId(statement->GetValueInt64(0)));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ObjectId ObjectInfoManager::FindCachedInstance(ECInstanceKeyCR instanceKey)
    {
    ECClassCP objectClass = m_dbAdapter->GetECClass(instanceKey);
    if (nullptr == objectClass)
        {
        return ObjectId();
        }

    auto statement = m_statementCache->GetPreparedStatement("ObjectInfoManager::FindCachedInstanceByInstanceKey", [&]
        {
        return
            "SELECT info.[" CLASS_CachedObjectInfo_PROPERTY_RemoteId "] "
            "FROM ONLY " ECSql_CachedObjectInfoClass " info "
            "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_ClassId "] = ? "
            "  AND info.[" CLASS_CachedObjectInfo_PROPERTY_LocalId "] = ? "
            "LIMIT 1 ";
        });

    statement->BindInt64(1, objectClass->GetId());
    statement->BindId(2, instanceKey.GetECInstanceId());

    ECSqlStepStatus status = statement->Step();
    if (status != ECSqlStepStatus::HasRow)
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
    if (SUCCESS != m_hierarchyManager->DeleteRelationship(info.GetInfoKey(), info.GetCachedInstanceKey(), m_infoRelationshipClass))
        {
        return ERROR;
        }

    if (SUCCESS != m_hierarchyManager->DeleteInstance(info.GetCachedInstanceKey()))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ECInstanceKey ObjectInfoManager::ReadInfoKey(ObjectIdCR objectId)
    {
    ECClassCP objectClass = m_dbAdapter->GetECClass(objectId);
    if (nullptr == objectClass)
        {
        return ECInstanceKey();
        }

    Utf8PrintfString key("ObjectInfoManager::ReadInfoKey");
    auto statement = m_statementCache->GetPreparedStatement(key, [&]
        {
        return
            "SELECT info.ECInstanceId "
            "FROM ONLY " ECSql_CachedObjectInfoClass " info "
            "WHERE info.[" CLASS_CachedObjectInfo_PROPERTY_ClassId "] = ? "
            "  AND info.[" CLASS_CachedObjectInfo_PROPERTY_RemoteId "] = ? "
            "LIMIT 1 ";
        });

    statement->BindInt64(1, objectClass->GetId());
    statement->BindText(2, objectId.remoteId.c_str(), IECSqlBinder::MakeCopy::No);

    ECSqlStepStatus status = statement->Step();
    if (status != ECSqlStepStatus::HasRow)
        {
        return ECInstanceKey();
        }

    return ECInstanceKey(m_infoClass->GetId(), statement->GetValueId<ECInstanceId>(0));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ObjectInfoManager::RemoveAllCachedInstances()
    {
    auto statement = m_statementCache->GetPreparedStatement("ObjectInfoManager::RemoveAllCachedInstances-CachedObjects", [&]
        {
        return "SELECT infoRel.TargetECClassId, infoRel.TargetECInstanceId FROM ONLY " ECSql_CachedObjectInfoRelationshipClass " infoRel ";
        });

    if (SUCCESS != m_hierarchyManager->DeleteInstances(*statement))
        {
        return ERROR;
        }

    statement = m_statementCache->GetPreparedStatement("ObjectInfoManager::RemoveAllCachedInstances-CachedObjectInfos", [&]
        {
        return "SELECT info.GetECClassId(), info.ECInstanceId FROM ONLY " ECSql_CachedObjectInfoClass " info ";
        });

    return m_hierarchyManager->DeleteInstances(*statement);
    }
