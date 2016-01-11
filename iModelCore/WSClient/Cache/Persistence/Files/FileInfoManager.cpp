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
WebServices::ECSqlStatementCache& statementCache,
FileStorage& fileStorage,
ObjectInfoManager& objectInfoManager,
HierarchyManager& hierarchyManager
) :
m_dbAdapter(dbAdapter),
m_statementCache(statementCache),
m_fileStorage(fileStorage),
m_hierarchyManager(hierarchyManager),
m_objectInfoManager(objectInfoManager),

m_infoClass(dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_CachedFileInfo)),
m_infoRelationshipClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_ObjectInfoToFileInfo)),
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
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
FileInfo FileInfoManager::ReadInfo(ObjectIdCR objectId)
    {
    return ReadInfo(m_objectInfoManager.ReadCachedInstanceKey(objectId));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
FileInfo FileInfoManager::ReadInfo(CachedInstanceKeyCR cachedKey)
    {
    Json::Value cachedFileInfoJson = ReadCachedInfoJson(cachedKey);
    Json::Value externalFileInfoJson = ReadExternalFileInfo(cachedKey);
    return FileInfo(cachedFileInfoJson, externalFileInfoJson, cachedKey, this);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
FileInfo FileInfoManager::ReadInfo(JsonValueCR infoJson)
    {
    CacheNodeKey fileInfoKey(m_infoClass->GetId(), ECDbHelper::ECInstanceIdFromJsonInstance(infoJson));
    CachedInstanceKey key = m_objectInfoManager.ReadCachedInstanceKey(fileInfoKey, *m_infoRelationshipClass);
    Json::Value externalFileInfoJson = ReadExternalFileInfo(key);
    return FileInfo(infoJson, externalFileInfoJson, key, this);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileInfoManager::SaveInfo(FileInfoR info)
    {
    if (!info.IsValid())
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
        ECInstanceKey fileInfoKey(m_infoClass->GetId(), ECDbHelper::ECInstanceIdFromJsonInstance(info.GetJsonInfo()));
        if (!m_dbAdapter.RelateInstances(m_infoRelationshipClass, info.GetCachedInstanceKey().GetInfoKey(), fileInfoKey).IsValid())
            {
            return ERROR;
            }

        if (SUCCESS != m_externalInfoInserter.Get().Insert(info.GetExternalFileInfoJson()))
            {
            return ERROR;
            }
        ECInstanceKey externalInfoKey(m_externalFileInfoClass->GetId(), ECDbHelper::ECInstanceIdFromJsonInstance(info.GetExternalFileInfoJson()));
        if (!m_dbAdapter.RelateInstances(m_externalFileInfoRelationshipClass, info.GetCachedInstanceKey().GetInstanceKey(), externalInfoKey).IsValid())
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName FileInfoManager::GetAbsoluteFilePath(bool isPersistent, BeFileNameCR relativePath) const
    {
    return m_fileStorage.GetAbsoluteFilePath(isPersistent, relativePath);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileInfoManager::DeleteFilesNotHeldByNodes(const ECInstanceKeyMultiMap& holdingNodes)
    {
    // WIP06
    //ECInstanceKeyMultiMap holdingInstances;
    //if (SUCCESS != m_objectInfoManager.ReadCachedInstanceKeys(holdingNodes, holdingInstances))
    //    {
    //    return ERROR;
    //    }

    //auto statement = m_statementCache.GetPreparedStatement("FileInfoManager::DeleteFilesNotHeldByNodes", [&]
    //    {
    //    return
    //        "SELECT efiRel.SourceECClassId, efiRel.SourceECInstanceId, efi.*, cfi.* "
    //        "FROM ONLY " ECSql_ExternalFileInfoClass " efi "
    //        "LEFT JOIN ONLY " ECSql_InstanceHasFileInfoClass " efiRel ON efiRel.TargetECInstanceId = efi.ECInstanceId ";
    //    });

    //JsonECSqlSelectAdapter adapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));

    //while (BE_SQLITE_ROW == statement->Step())
    //    {
    //    Json::Value cachedFileInfoJson;
    //    Json::Value externalFileInfoJson;

    //    if (!adapter.GetRowInstance(cachedFileInfoJson, m_infoClass->GetId()))
    //        {
    //        return ERROR;
    //        }
    //    if (!adapter.GetRowInstance(externalFileInfoJson, m_externalFileInfoClass->GetId()))
    //        {
    //        return ERROR;
    //        }

    //    ECInstanceKey instanceKey(statement->GetValueInt64(0), statement->GetValueId<ECInstanceId>(1));
    //    FileInfo fileInfo(cachedFileInfoJson, externalFileInfoJson, instanceKey, this);

    //    if (ECDbHelper::IsInstanceInMultiMap(instanceKey, holdingInstances))
    //        {
    //        continue;
    //        }

    //    if (IChangeManager::ChangeStatus::NoChange != fileInfo.GetChangeStatus())
    //        {
    //        continue;
    //        }

    //    if (SUCCESS != m_fileStorage.CleanupCachedFile(fileInfo.GetFilePath()))
    //        {
    //        return ERROR;
    //        }
    //    }
    //return SUCCESS;
    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName FileInfoManager::ReadFilePath(CachedInstanceKeyCR instanceKey)
    {
    FileInfo fileInfo = ReadInfo(instanceKey);
    BeFileName path = fileInfo.GetFilePath();

    if (path.empty())
        {
        return path;
        }

    if (!path.DoesPathExist())
        {
        path.clear();
        return path;
        }

    return path;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileInfoManager::OnBeforeDelete(ECClassCR ecClass, ECInstanceId ecInstanceId, bset<ECInstanceKey>& additionalInstancesOut)
    {
    if (&ecClass != m_externalFileInfoClass)
        {
        return SUCCESS;
        }

    Json::Value infoJson;
    JsonReader reader(m_dbAdapter.GetECDb(), ecClass.GetId());
    reader.ReadInstance(infoJson, ecInstanceId, ECValueFormat::RawNativeValues);

    FileInfo info(Json::nullValue, infoJson, CachedInstanceKey(), this);

    m_fileStorage.CleanupCachedFile(info.GetFilePath());
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value FileInfoManager::ReadExternalFileInfo(CachedInstanceKeyCR cachedKey)
    {
    Json::Value infoJson;

    ECInstanceKeyCR instanceKey = cachedKey.GetInstanceKey();
    if (!instanceKey.IsValid())
        {
        return infoJson;
        }

    auto statement = m_statementCache.GetPreparedStatement("FileInfoManager::ReadExternalFileInfo", [&]
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

    DbResult status = statement->Step();
    if (status != BE_SQLITE_ROW)
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
Json::Value FileInfoManager::ReadCachedInfoJson(CachedInstanceKeyCR cachedKey)
    {
    ECInstanceKeyCR infoKey = cachedKey.GetInfoKey();
    if (!infoKey.IsValid())
        {
        return Json::nullValue;
        }

    auto statement = m_statementCache.GetPreparedStatement("FileInfoManager::ReadCachedInfoJson", [&]
        {
        return
            "SELECT info.* "
            "FROM " ECSql_CachedFileInfo " info "
            "LEFT JOIN ONLY " ECSql_ObjectInfoToFileInfo " infoRel ON infoRel.SourceECInstanceId = info.ECInstanceId "
            "WHERE infoRel.TargetECClassId = ? AND infoRel.TargetECInstanceId = ? "
            "LIMIT 1 ";
        });

    statement->BindInt64(1, infoKey.GetECClassId());
    statement->BindId(2, infoKey.GetECInstanceId());

    DbResult status = statement->Step();
    if (status != BE_SQLITE_ROW)
        {
        return Json::nullValue;
        }

    JsonECSqlSelectAdapter adapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));

    Json::Value currentObj;
    adapter.GetRowInstance(currentObj, m_infoClass->GetId());

    return currentObj;
    }
