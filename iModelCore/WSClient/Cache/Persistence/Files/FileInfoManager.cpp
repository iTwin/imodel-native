/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Files/FileInfoManager.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "FileInfoManager.h"

#include <WebServices/Cache/Util/ECDbHelper.h>

#include "../Core/CacheSchema.h"
#include "../Core/ECDbFileInfoSchema.h"
#include "../Hierarchy/HierarchyManager.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
FileInfoManager::FileInfoManager
(
ECDbAdapter& dbAdapter,
ECSqlStatementCache& statementCache,
FileStorage& fileStorage,
ObjectInfoManager& objectInfoManager,
HierarchyManager& hierarchyManager
) :
m_dbAdapter(&dbAdapter),
m_statementCache(&statementCache),
m_fileStorage(&fileStorage),
m_hierarchyManager(&hierarchyManager),
m_objectInfoManager(&objectInfoManager),

m_infoClass(dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_CachedFileInfo)),
m_infoRelationshipClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_CachedFileInfoToInstance)),
m_externalFileInfoClass(dbAdapter.GetECClass(SCHEMA_ECDbFileInfo, CLASS_ExternalFileInfo)),
m_externalFileInfoRelationshipClass(dbAdapter.GetECRelationshipClass(SCHEMA_ECDbFileInfo, CLASS_InstanceHasFileInfo)),

m_cachedInfoInserter(dbAdapter.GetECDb(), *m_infoClass),
m_cachedInfoUpdater(dbAdapter.GetECDb(), *m_infoClass),
m_externalInfoInserter(dbAdapter.GetECDb(), *m_externalFileInfoClass),
m_externalInfoUpdater(dbAdapter.GetECDb(), *m_externalFileInfoClass)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP FileInfoManager::GetInfoClass() const
    {
    return m_infoClass;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP FileInfoManager::GetInfoRelationshipClass() const
    {
    return m_infoRelationshipClass;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
FileInfo FileInfoManager::ReadInfo(ECInstanceKeyCR fileKey)
    {
    Json::Value cachedFileInfoJson = ReadCachedInfoJson(fileKey);
    Json::Value externalFileInfoJson = ReadExternalFileInfo(fileKey);
    return FileInfo(cachedFileInfoJson, externalFileInfoJson, fileKey, this);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
FileInfo FileInfoManager::ReadInfo(ObjectIdCR fileId)
    {
    ECInstanceKey instance = m_objectInfoManager->FindCachedInstance(fileId);
    return ReadInfo(instance);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
FileInfo FileInfoManager::ReadInfo(JsonValueCR infoJson)
    {
    auto statement = m_statementCache->GetPreparedStatement("FileInfoManager::ReadInfoByJson", [&]
        {
        return
            "SELECT infoRel.TargetECClassId, infoRel.TargetECInstanceId "
            "FROM ONLY " ECSql_CachedFileInfoToInstanceClass " infoRel "
            "WHERE infoRel.SourceECInstanceId = ? "
            "LIMIT 1 ";
        });

    statement->BindInt64(1, BeJsonUtilities::Int64FromValue(infoJson["$ECInstanceId"]));
    statement->Step();

    ECInstanceKey instanceKey(statement->GetValueInt64(0), statement->GetValueId<ECInstanceId>(1));
    Json::Value externalFileInfoJson = ReadExternalFileInfo(instanceKey);

    return FileInfo(infoJson, externalFileInfoJson, instanceKey, this);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileInfoManager::SaveInfo(FileInfoR info)
    {
    if (!info.GetInstanceKey().IsValid())
        {
        return ERROR;
        }

    if (info.IsInCache())
        {
        if (SUCCESS != m_cachedInfoUpdater.Get().Update(info.GetJsonInfo()))
            {
            return ERROR;
            }

        if (SUCCESS != m_externalInfoUpdater.Get().Update(info.GetExternalFileInfoJson()))
            {
            return ERROR;
            }
        }
    else
        {
        if (SUCCESS != m_cachedInfoInserter.Get().Insert(info.GetJsonInfo()))
            {
            return ERROR;
            }
        ECInstanceKey cachedInfoKey(m_infoClass->GetId(), ECDbHelper::ECInstanceIdFromJsonInstance(info.GetJsonInfo()));
        if (!m_dbAdapter->RelateInstances(m_infoRelationshipClass, cachedInfoKey, info.GetInstanceKey()).IsValid())
            {
            return ERROR;
            }

        if (SUCCESS != m_externalInfoInserter.Get().Insert(info.GetExternalFileInfoJson()))
            {
            return ERROR;
            }
        ECInstanceKey externalInfoKey(m_externalFileInfoClass->GetId(), ECDbHelper::ECInstanceIdFromJsonInstance(info.GetExternalFileInfoJson()));
        if (!m_dbAdapter->RelateInstances(m_externalFileInfoRelationshipClass, info.GetInstanceKey(), externalInfoKey).IsValid())
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName FileInfoManager::GetAbsoluteFilePath(FileCache location, BeFileNameCR relativePath) const
    {
    return m_fileStorage->GetAbsoluteFilePath(location, relativePath);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileInfoManager::CheckMaxLastAccessDate(BeFileNameCR fileName, DateTimeCP maxLastAccessDate, bool &shouldSkip)
    {
    shouldSkip = false;
    if (nullptr == maxLastAccessDate || fileName.IsEmpty() || !fileName.DoesPathExist())
        {
        return SUCCESS;
        }
    time_t accessTime;
    if (BeFileNameStatus::Success != fileName.GetFileTime(nullptr, &accessTime, nullptr))
        {
        BeAssert(false);
        return ERROR;
        }
    DateTime lastAccessDate;
    if (SUCCESS != DateTime::FromUnixMilliseconds(lastAccessDate, accessTime * 1000))
        {
        BeAssert(false);
        return ERROR;
        }
    if (DateTime::Compare(lastAccessDate, *maxLastAccessDate) == DateTime::CompareResult::LaterThan)
        {
        shouldSkip = true;
        }
    return SUCCESS;
    }
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileInfoManager::DeleteFilesNotHeldByInstances(const ECInstanceKeyMultiMap& holdingInstances, DateTimeCP maxLastAccessDate)
    {
    auto statement = m_statementCache->GetPreparedStatement("FileInfoManager::DeleteFilesNotHeldByInstances", [&]
        {
        return
            "SELECT efiRel.SourceECClassId, efiRel.SourceECInstanceId, efi.*, cfi.* "
            "FROM ONLY " ECSql_ExternalFileInfoClass " efi "
            "LEFT JOIN ONLY " ECSql_InstanceHasFileInfoClass " efiRel ON efiRel.TargetECInstanceId = efi.ECInstanceId "
            "LEFT JOIN ONLY " ECSql_CachedFileInfoToInstanceClass " cfiRel ON cfiRel.TargetECInstanceId = efiRel.SourceECInstanceId "
            "LEFT JOIN ONLY " ECSql_CachedFileInfoClass " cfi ON cfi.ECInstanceId = cfiRel.SourceECInstanceId ";
        });

    JsonECSqlSelectAdapter adapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    BentleyStatus returnValue = SUCCESS;

    while (ECSqlStepStatus::HasRow == statement->Step())
        {
        Json::Value cachedFileInfoJson;
        Json::Value externalFileInfoJson;

        if (!adapter.GetRowInstance(cachedFileInfoJson, m_infoClass->GetId()))
            {
            return ERROR;
            }
        if (!adapter.GetRowInstance(externalFileInfoJson, m_externalFileInfoClass->GetId()))
            {
            return ERROR;
            }

        ECInstanceKey instanceKey(statement->GetValueInt64(0), statement->GetValueId<ECInstanceId>(1));
        FileInfo fileInfo(cachedFileInfoJson, externalFileInfoJson, instanceKey, this);

        if (ECDbHelper::IsInstanceInMultiMap(instanceKey, holdingInstances))
            {
            continue;
            }

        if (IChangeManager::ChangeStatus::NoChange != fileInfo.GetChangeStatus())
            {
            continue;
            }

        bool shouldSkip;
        if (SUCCESS != CheckMaxLastAccessDate(fileInfo.GetFilePath(), maxLastAccessDate, shouldSkip))
            {
            // Return error from the function when we eventually finish, but continue processing
            // files anyway.
            returnValue = ERROR;
            }
        if (shouldSkip)
            {
            continue;
            }

        if (SUCCESS != m_fileStorage->CleanupCachedFile(fileInfo))
            {
            return ERROR;
            }
        }
    return returnValue;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey FileInfoManager::FindTargetInstanceKeyForInfo(ECInstanceId infoECId)
    {
    ECInstanceKey infoKey(m_infoClass->GetId(), infoECId);

    ECInstanceKeyMultiMap instanceMap;
    if (SUCCESS != m_hierarchyManager->ReadTargetKeys(infoKey, m_infoRelationshipClass, instanceMap))
        {
        return ECInstanceKey();
        }

    if (instanceMap.size() != 1)
        {
        return ECInstanceKey();
        }

    ECClassId classId = instanceMap.begin()->first;
    ECInstanceId instanceId = instanceMap.begin()->second;

    return ECInstanceKey(classId, instanceId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName FileInfoManager::ReadFilePath(ECInstanceKeyCR instance)
    {
    FileInfo fileInfo = ReadInfo(instance);
    BeFileName path = fileInfo.GetFilePath();

    if (!path.empty() && !path.DoesPathExist())
        path.clear();

    return path;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileInfoManager::OnBeforeDelete(ECClassCR ecClass, ECInstanceId ecInstanceId)
    {
    if (&ecClass != m_externalFileInfoClass)
        {
        return SUCCESS;
        }

    Json::Value infoJson;
    JsonReader reader(m_dbAdapter->GetECDb(), ecClass.GetId());
    reader.ReadInstance(infoJson, ecInstanceId, ECValueFormat::RawNativeValues);

    FileInfo info (Json::nullValue, infoJson, ECInstanceKey(), this);

    m_fileStorage->CleanupCachedFile(info);
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileInfoManager::OnAfterDelete(bset<ECInstanceKey>& instancesToDeleteOut)
    {
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value FileInfoManager::ReadExternalFileInfo(ECInstanceKeyCR instanceKey)
    {
    Json::Value infoJson;

    ECClassCP instanceClass = m_dbAdapter->GetECClass(instanceKey);
    if (!instanceKey.IsValid() || nullptr == instanceClass)
        {
        return infoJson;
        }

    auto statement = m_statementCache->GetPreparedStatement("FileInfoManager::ReadExternalFileInfo", [&]
        {
        return
            "SELECT info.* "
            "FROM " ECSql_ExternalFileInfoClass " info "
            "LEFT JOIN ONLY " ECSql_InstanceHasFileInfoClass " infoRel ON infoRel.TargetECInstanceId = info.ECInstanceId "
            "WHERE infoRel.SourceECClassId = ? AND infoRel.SourceECInstanceId = ? "
            "LIMIT 1 ";
        });

    statement->BindInt64(1, instanceKey.GetECClassId());
    statement->BindId(2, instanceKey.GetECInstanceId());

    ECSqlStepStatus status = statement->Step();
    if (status != ECSqlStepStatus::HasRow)
        {
        return infoJson;
        }

    JsonECSqlSelectAdapter adapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    adapter.GetRowInstance(infoJson, m_externalFileInfoClass->GetId());

    return infoJson;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value FileInfoManager::ReadCachedInfoJson(ECInstanceKeyCR instanceKey)
    {
    ECClassCP instanceClass = m_dbAdapter->GetECClass(instanceKey);
    if (!instanceKey.IsValid() || nullptr == instanceClass)
        {
        return Json::nullValue;
        }

    auto statement = m_statementCache->GetPreparedStatement("FileInfoManager::ReadCachedInfoJson", [&]
        {
        return
            "SELECT info.* "
            "FROM " ECSql_CachedFileInfoClass " info "
            "LEFT JOIN ONLY " ECSql_CachedFileInfoToInstanceClass " infoRel ON infoRel.SourceECInstanceId = info.ECInstanceId "
            "WHERE infoRel.TargetECClassId = ? AND infoRel.TargetECInstanceId = ? "
            "LIMIT 1 ";
        });

    statement->BindInt64(1, instanceKey.GetECClassId());
    statement->BindId(2, instanceKey.GetECInstanceId());

    ECSqlStepStatus status = statement->Step();
    if (status != ECSqlStepStatus::HasRow)
        {
        return Json::nullValue;
        }

    JsonECSqlSelectAdapter adapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));

    Json::Value currentObj;
    adapter.GetRowInstance(currentObj, m_infoClass->GetId());

    return currentObj;
    }
