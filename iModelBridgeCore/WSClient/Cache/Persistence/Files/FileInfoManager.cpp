/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Files/FileInfoManager.cpp $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "FileInfoManager.h"

#include <WebServices/Cache/Util/ECDbHelper.h>

#include "../Core/CacheSchema.h"
#include "../Core/ECDbFileInfoSchema.h"
#include "../Hierarchy/HierarchyManager.h"
#include "../DataSourceCache.xliff.h"

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

m_cachedFileInfoClass(dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_CachedFileInfo)),
m_objectInfoToCachedFileInfoClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_ObjectInfoToCachedFileInfo)),
m_cachedFileInfoToFileInfoClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_CachedFileInfoToFileInfo)),
m_cachedFileInfoToFileInfoOwnershipClass(dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_CachedFileInfoToFileInfoOwnership)),
m_externalFileInfoClass(dbAdapter.GetECClass(SCHEMA_ECDbFileInfo, CLASS_ExternalFileInfo)),
m_externalFileInfoOwnershipClass(dbAdapter.GetECClass(SCHEMA_ECDbFileInfo, CLASS_FileInfoOwnership)),

m_cachedFileInfoInserter(dbAdapter.GetECDb(), *m_cachedFileInfoClass),
m_cachedFileInfoUpdater(dbAdapter.GetECDb(), *m_cachedFileInfoClass, ECSqlUpdater_Options_IgnoreSystemAndFailReadOnlyProperties),
m_externalFileInfoInserter(dbAdapter.GetECDb(), *m_externalFileInfoClass),
m_externalFileInfoUpdater(dbAdapter.GetECDb(), *m_externalFileInfoClass, ECSqlUpdater_Options_IgnoreSystemAndFailReadOnlyProperties)
    {
    dbAdapter.RegisterDeleteListener(this);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP FileInfoManager::GetInfoClass() const
    {
    return m_cachedFileInfoClass;
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
    Json::Value cachedFileInfoJson = ReadCachedFileInfo(cachedKey);
    Json::Value externalFileInfoJson = ReadExternalFileInfo(cachedKey);
    return FileInfo(cachedFileInfoJson, externalFileInfoJson, cachedKey, this);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
FileInfo FileInfoManager::ReadInfo(JsonValueCR cachedFileInfoJson)
    {
    CacheNodeKey fileInfoKey(m_cachedFileInfoClass->GetId(), ECDbHelper::ECInstanceIdFromJsonInstance(cachedFileInfoJson));
    CachedInstanceKey cachedKey = m_objectInfoManager.ReadCachedInstanceKey(fileInfoKey, *m_objectInfoToCachedFileInfoClass);
    Json::Value externalFileInfoJson = ReadExternalFileInfo(cachedKey);
    return FileInfo(cachedFileInfoJson, externalFileInfoJson, cachedKey, this);
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
        if (BE_SQLITE_OK != m_cachedFileInfoUpdater.Get().Update(ECDbHelper::ECInstanceIdFromJsonInstance(info.GetJsonInfo()), info.GetJsonInfo()) ||
            BE_SQLITE_OK != m_externalFileInfoUpdater.Get().Update(ECDbHelper::ECInstanceIdFromJsonInstance(info.GetExternalFileInfoJson()), info.GetExternalFileInfoJson()))
            {
            return ERROR;
            }
        }
    else
        {
        Json::Value& instance = info.GetJsonInfo();
        instance[CLASS_CachedFileInfo_PROPERTY_ObjectInfo][ECJsonUtilities::json_navId()] = info.GetCachedInstanceKey().GetInfoKey().GetInstanceId().ToHexStr();

        if (BE_SQLITE_OK != m_cachedFileInfoInserter.Get().Insert(instance))
            return ERROR;
        
        if (BE_SQLITE_OK != m_externalFileInfoInserter.Get().Insert(info.GetExternalFileInfoJson()))
            return ERROR;

        ECInstanceKey cachedFileInfoKey(m_cachedFileInfoClass->GetId(), ECDbHelper::ECInstanceIdFromJsonInstance(info.GetJsonInfo()));
        ECInstanceKey externalFileInfoKey(m_externalFileInfoClass->GetId(), ECDbHelper::ECInstanceIdFromJsonInstance(info.GetExternalFileInfoJson()));
        ECInstanceKey fileInfoOwnershipKey = InsertFileInfoOwnership(info.GetCachedInstanceKey().GetInstanceKey(), externalFileInfoKey);

        if (!m_dbAdapter.RelateInstances(m_cachedFileInfoToFileInfoClass, cachedFileInfoKey, externalFileInfoKey).IsValid() ||
            !m_dbAdapter.RelateInstances(m_cachedFileInfoToFileInfoOwnershipClass, cachedFileInfoKey, fileInfoOwnershipKey).IsValid())
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
    return m_fileStorage.GetAbsoluteFilePath(location, relativePath);
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
    int64_t accessTimeMs = static_cast<int64_t>(accessTime) * 1000;
    DateTime lastAccessDate;
    if (SUCCESS != DateTime::FromUnixMilliseconds(lastAccessDate, accessTimeMs))
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
CacheStatus FileInfoManager::DeleteFilesNotHeldByNodes
(
const ECInstanceKeyMultiMap& holdingNodes,
DateTimeCP maxLastAccessDate,
AsyncError* errorOut
)
    {
    auto statement = m_statementCache.GetPreparedStatement("FileInfoManager::DeleteFilesNotHeldByNodes", [&]
        {
        return
            "SELECT cfi.[" CLASS_ChangeInfo_PROPERTY_ChangeNumber "], efi.ECInstanceId, efi.* "
            "FROM ONLY " ECSql_ExternalFileInfoClass " efi "
            "JOIN ONLY " ECSql_CachedFileInfo " cfi USING " ECSql_CachedFileInfoToFileInfo " ";
        });

    JsonECSqlSelectAdapter adapter(*statement);
    CacheStatus returnValue = CacheStatus::OK;

    while (BE_SQLITE_ROW == statement->Step())
        {
        auto changeStatus = static_cast<IChangeManager::ChangeStatus>(statement->GetValueInt(0));
        if (IChangeManager::ChangeStatus::NoChange != changeStatus)
            continue;

        ECInstanceKey externalFileInfoKey(m_externalFileInfoClass->GetId(), statement->GetValueId<ECInstanceId>(1));
        if (ECDbHelper::IsInstanceInMultiMap(externalFileInfoKey, holdingNodes))
            continue;

        Json::Value externalFileInfoJson;
        if (SUCCESS != adapter.GetRowInstance(externalFileInfoJson, m_externalFileInfoClass->GetId()))
            return CacheStatus::Error;

        FileInfo fileInfo(Json::nullValue, externalFileInfoJson, CachedInstanceKey(), this);
        auto filePath = fileInfo.GetFilePath();

        bool shouldSkip;
        if (SUCCESS != CheckMaxLastAccessDate(fileInfo.GetFilePath(), maxLastAccessDate, shouldSkip))
            {
            // Return error from the function when we eventually finish, but continue processing
            // files anyway.
            returnValue = CacheStatus::Error;
            }

        if (shouldSkip)
            continue;

        auto status = m_fileStorage.RemoveStoredFile(fileInfo);
        if (CacheStatus::OK != status)
            {
            if (errorOut != nullptr && CacheStatus::FileLocked == status)
                {
            	*errorOut = AsyncError(Utf8PrintfString(
                    DataSourceCacheLocalizedString(ERROR_FileIsLocked).c_str(),
                    Utf8String(fileInfo.GetFileName()).c_str()));
                }
            return status;
            }
        }
    return returnValue;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName FileInfoManager::ReadFilePath(CachedInstanceKeyCR cachedKey)
    {
    FileInfo fileInfo = ReadInfo(cachedKey);
    BeFileName path = fileInfo.GetFilePath();

    if (!path.empty() && !path.DoesPathExist())
        path.clear();

    return path;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileInfoManager::OnBeforeDelete(ECClassCR ecClass, ECInstanceId ecInstanceId, bset<ECInstanceKey>& additionalInstancesOut)
    {
    if (ecClass.GetId() != m_externalFileInfoClass->GetId())
        return SUCCESS;

    Json::Value externalFileInfoJson;
    m_dbAdapter.GetJsonInstance(externalFileInfoJson, {ecClass.GetId(), ecInstanceId});

    FileInfo info(Json::nullValue, externalFileInfoJson, CachedInstanceKey(), this);
    if (CacheStatus::OK != m_fileStorage.RemoveStoredFile(info))
        return ERROR;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value FileInfoManager::ReadCachedFileInfo(CachedInstanceKeyCR cachedKey)
    {
    Json::Value infoJson;

    ECInstanceKeyCR infoKey = cachedKey.GetInfoKey();
    if (!infoKey.IsValid())
        {
        return infoJson;
        }

    auto statement = m_statementCache.GetPreparedStatement("FileInfoManager::ReadCachedFileInfo", [&]
        {
        return
            "SELECT info.* "
            "FROM ONLY " ECSql_CachedFileInfo " info "
            "JOIN ONLY " ECSql_ObjectInfoToCachedFileInfo " infoRel ON infoRel.TargetECInstanceId = info.ECInstanceId "
            "WHERE infoRel.SourceECClassId = ? AND infoRel.SourceECInstanceId = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, infoKey.GetClassId());
    statement->BindId(2, infoKey.GetInstanceId());

    DbResult status = statement->Step();
    if (status != BE_SQLITE_ROW)
        {
        return infoJson;
        }

    JsonECSqlSelectAdapter adapter(*statement);

    adapter.GetRow(infoJson);

    return infoJson;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value FileInfoManager::ReadExternalFileInfo(CachedInstanceKeyCR cachedKey)
    {
    Json::Value infoJson;

    CacheNodeKey cachedObjectInfoKey = cachedKey.GetInfoKey();
    if (!cachedObjectInfoKey.IsValid())
        {
        return infoJson;
        }

    auto statement = m_statementCache.GetPreparedStatement("FileInfoManager::ReadExternalFileInfo", [&]
        {
        return
            "SELECT externalFileInfo.* "
            "FROM " ECSql_ExternalFileInfoClass " externalFileInfo "
            "JOIN " ECSql_CachedFileInfo " cachedFileInfo USING " ECSql_CachedFileInfoToFileInfo " "
            "JOIN " ECSql_CachedObjectInfo " cachedObjectInfo USING " ECSql_ObjectInfoToCachedFileInfo " "
            "WHERE cachedObjectInfo.ECInstanceId = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, cachedObjectInfoKey.GetInstanceId());

    DbResult status = statement->Step();
    if (status != BE_SQLITE_ROW)
        {
        return infoJson;
        }

    JsonECSqlSelectAdapter adapter(*statement);
    adapter.GetRow(infoJson);

    return infoJson;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey FileInfoManager::InsertFileInfoOwnership(ECInstanceKeyCR ownerKey, ECInstanceKeyCR fileInfoKey)
    {
    auto statement = m_statementCache.GetPreparedStatement("InsertFileInfoOwnership", [&]
        {
        return "INSERT INTO " ECSql_FileInfoOwnership " (OwnerECClassId, OwnerId, FileInfoECClassId, FileInfoId) VALUES (?,?,?,?)";
        });

    statement->BindId(1, ownerKey.GetClassId());
    statement->BindId(2, ownerKey.GetInstanceId());
    statement->BindId(3, fileInfoKey.GetClassId());
    statement->BindId(4, fileInfoKey.GetInstanceId());

    ECInstanceKey ownershipKey;
    if (BE_SQLITE_DONE != statement->Step(ownershipKey))
        {
        return ECInstanceKey();
        }

    return ownershipKey;
    }
