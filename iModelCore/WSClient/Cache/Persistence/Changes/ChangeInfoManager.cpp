/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Changes/ChangeInfoManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ChangeInfoManager.h"

#include <WebServices/Cache/Util/ECDbHelper.h>
#include "../Core/CacheSchema.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
ChangeInfoManager::ChangeInfoManager
(
ECDbAdapter& dbAdapter,
WebServices::ECSqlStatementCache& statementCache,
ObjectInfoManager& objectInfoManager,
RelationshipInfoManager& relationshipInfoManager,
FileInfoManager& fileInfoManager
) :
m_statementCache(&statementCache),
m_objectInfoManager(&objectInfoManager),
m_relationshipInfoManager(&relationshipInfoManager),
m_fileInfoManager(&fileInfoManager)
    {
    ECClassCP changeInfoNumberClass = dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_ChangeSequenceInfo);
    ECPropertyCP lastChangeNumberProperty = changeInfoNumberClass->GetPropertyP(CLASS_ChangeSequenceInfo_PROPERTY_LastChangeNumber);
    m_changeNumberIncrementor = std::make_shared<ValueIncrementor>(dbAdapter.GetECDb(), statementCache, *lastChangeNumberProperty);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool ChangeInfoManager::HasChanges()
    {
    auto statement = m_statementCache->GetPreparedStatement("ChangeInfoManager::HasChanges", [&]
        {
        return Utf8PrintfString
            (
            "SELECT NULL FROM " ECSql_ChangeInfoClass " info "
            "WHERE info.[" CLASS_ChangeInfo_PROPERTY_ChangeStatus "] IS NOT NULL "
            "  AND info.[" CLASS_ChangeInfo_PROPERTY_ChangeStatus "] != %d "
            "LIMIT 1 ",
            IChangeManager::ChangeStatus::NoChange
            );
        });
    return statement->Step() == BE_SQLITE_ROW;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeInfoManager::GetChanges(IChangeManager::Changes& changesOut, bool onlyReadyToSync)
    {
    if (SUCCESS != GetObjectChanges(changesOut, onlyReadyToSync))
        {
        return ERROR;
        }
    if (SUCCESS != GetRelationshipChanges(changesOut, onlyReadyToSync))
        {
        return ERROR;
        }
    if (SUCCESS != GetFileChanges(changesOut, onlyReadyToSync))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeInfoManager::GetObjectChanges(IChangeManager::Changes& changesOut, bool onlyReadyToSync)
    {
    auto statement = GetPreparedStatementForGetChanges(m_objectInfoManager->GetInfoClass(), onlyReadyToSync);
    JsonECSqlSelectAdapter adapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    while (statement->Step() == BE_SQLITE_ROW)
        {
        Json::Value infoJson;
        if (!adapter.GetRowInstance(infoJson, m_objectInfoManager->GetInfoClass()->GetId()))
            {
            return ERROR;
            }
        IChangeManager::ObjectChange change = GetObjectChange(m_objectInfoManager->ReadInfo(infoJson));
        if (!change.IsValid())
            {
            return ERROR;
            }
        changesOut.AddChange(change);
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeInfoManager::GetRelationshipChanges(IChangeManager::Changes& changesOut, bool onlyReadyToSync)
    {
    auto statement = GetPreparedStatementForGetChanges(m_relationshipInfoManager->GetInfoClass(), onlyReadyToSync);
    JsonECSqlSelectAdapter adapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    while (statement->Step() == BE_SQLITE_ROW)
        {
        Json::Value infoJson;
        if (!adapter.GetRowInstance(infoJson, m_relationshipInfoManager->GetInfoClass()->GetId()))
            {
            return ERROR;
            }
        IChangeManager::RelationshipChange change = GetRelationshipChange(m_relationshipInfoManager->ParseInfo(infoJson));
        if (!change.IsValid())
            {
            return ERROR;
            }
        changesOut.AddChange(change);
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeInfoManager::GetFileChanges(IChangeManager::Changes& changesOut, bool onlyReadyToSync)
    {
    auto statement = GetPreparedStatementForGetChanges(m_fileInfoManager->GetInfoClass(), onlyReadyToSync);
    JsonECSqlSelectAdapter adapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    while (statement->Step() == BE_SQLITE_ROW)
        {
        Json::Value infoJson;
        if (!adapter.GetRowInstance(infoJson, m_fileInfoManager->GetInfoClass()->GetId()))
            {
            return ERROR;
            }
        IChangeManager::FileChange change = GetFileChange(m_fileInfoManager->ReadInfo(infoJson));
        if (!change.IsValid())
            {
            return ERROR;
            }
        changesOut.AddChange(change);
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
std::shared_ptr<ECSqlStatement> ChangeInfoManager::GetPreparedStatementForGetChanges(ECClassCP infoClass, bool onlyReadyToSync)
    {
    Utf8PrintfString key("ChangeInfoManager::GetPreparedStatementForGetChanges:%lld:%d", infoClass->GetId(), onlyReadyToSync);
    auto statement = m_statementCache->GetPreparedStatement(key, [&]
        {
        Utf8String syncStatusCriteria;
        if (onlyReadyToSync)
            {
            syncStatusCriteria.Sprintf("AND info.[" CLASS_ChangeInfo_PROPERTY_SyncStatus "] = %d", IChangeManager::SyncStatus::Ready);
            }

        return Utf8PrintfString
            (
            "SELECT info.* FROM ONLY %s info "
            "WHERE (info.[" CLASS_ChangeInfo_PROPERTY_ChangeStatus "] IS NOT NULL "
            "   AND info.[" CLASS_ChangeInfo_PROPERTY_ChangeStatus "] != %d) %s ",
            ECSqlBuilder::ToECSqlSnippet(*infoClass).c_str(),
            IChangeManager::ChangeStatus::NoChange,
            syncStatusCriteria.c_str()
            );
        });
    return statement;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::ObjectChange ChangeInfoManager::GetObjectChange(ECInstanceKeyCR instanceKey)
    {
    return GetObjectChange(m_objectInfoManager->ReadInfo(instanceKey));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::ObjectChange ChangeInfoManager::GetObjectChange(ObjectInfoCR info)
    {
    return IChangeManager::ObjectChange(info.GetCachedInstanceKey(), info.GetChangeStatus(), info.GetSyncStatus(), info.GetChangeNumber());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::RelationshipChange ChangeInfoManager::GetRelationshipChange(ECInstanceKeyCR instanceKey)
    {
    RelationshipInfo info = m_relationshipInfoManager->FindInfo(instanceKey);
    return GetRelationshipChange(info);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::RelationshipChange ChangeInfoManager::GetRelationshipChange(RelationshipInfoCR info)
    {
    ECInstanceKey source, target;
    m_relationshipInfoManager->ReadRelationshipEnds(info.GetRelationshipKey(), source, target);
    return IChangeManager::RelationshipChange(info.GetRelationshipKey(), source, target,
                                              info.GetChangeStatus(), info.GetSyncStatus(), info.GetChangeNumber());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::FileChange ChangeInfoManager::GetFileChange(ECInstanceKeyCR instanceKey)
    {
    FileInfo info = m_fileInfoManager->ReadInfo(instanceKey);
    return IChangeManager::FileChange(instanceKey, info.GetChangeStatus(), info.GetSyncStatus(), info.GetChangeNumber());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::FileChange ChangeInfoManager::GetFileChange(FileInfoCR info)
    {
    return IChangeManager::FileChange(info.GetInstanceKey(), info.GetChangeStatus(), info.GetSyncStatus(), info.GetChangeNumber());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::ChangeStatus ChangeInfoManager::GetObjectChangeStatus(ECInstanceKeyCR instanceKey)
    {
    int statusInt = ReadStatusProperty(instanceKey, CLASS_ChangeInfo_PROPERTY_ChangeStatus);
    return static_cast<IChangeManager::ChangeStatus> (statusInt);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
IChangeManager::SyncStatus ChangeInfoManager::GetObjectSyncStatus(ECInstanceKeyCR instanceKey)
    {
    int statusInt = ReadStatusProperty(instanceKey, CLASS_ChangeInfo_PROPERTY_SyncStatus);
    return static_cast<IChangeManager::SyncStatus> (statusInt);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
int ChangeInfoManager::ReadStatusProperty(ECInstanceKeyCR instanceKey, Utf8CP statusPropertyName)
    {
    if (!instanceKey.IsValid())
        {
        return 0;
        }

    Utf8PrintfString key("ObjectInfoManager::ReadObjectStatusProperty:%s", statusPropertyName);
    auto statement = m_statementCache->GetPreparedStatement(key, [&]
        {
        return Utf8PrintfString(
            "SELECT info.[%s] FROM " ECSql_ChangeInfoClass " info "
            "JOIN ONLY " ECSql_CachedObjectInfoRelationshipClass " infoRel ON infoRel.SourceECInstanceId = info.ECInstanceId "
            "WHERE infoRel.TargetECClassId = ? AND infoRel.TargetECInstanceId = ? "
            "LIMIT 1 ",
            statusPropertyName
            );
        });

    statement->BindInt64(1, instanceKey.GetECClassId());
    statement->BindId(2, instanceKey.GetECInstanceId());
    DbResult status = statement->Step();

    if (BE_SQLITE_ROW != status)
        {
        return 0;
        }

    return statement->GetValueInt(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus ChangeInfoManager::SetupChangeNumber(ChangeInfoR info)
    {
    if (info.GetChangeNumber() != 0)
        {
        // No need to set change number as it is already set
        return SUCCESS;
        }

    Utf8String newNumberStr;
    if (SUCCESS != m_changeNumberIncrementor->IncrementWithoutSaving(newNumberStr))
        {
        return ERROR;
        }

    uint64_t newNumber;
    if (SUCCESS != BeStringUtilities::ParseUInt64(newNumber, newNumberStr.c_str()))
        {
        return ERROR;
        }

    info.SetChangeNumber(newNumber);

    return SUCCESS;
    }