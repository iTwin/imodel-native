/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/DataSourceCache.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Persistence/DataSourceCache.h>

#include <Bentley/BeTimeUtilities.h>
#include <Bentley/DateTime.h>
#include <WebServices/Cache/Persistence/CacheQueryHelper.h>
#include <WebServices/Cache/Util/ECDbDebugInfo.h>
#include <WebServices/Cache/Util/ECDbHelper.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>
#include <WebServices/Cache/Util/ECCustomAttributeHelper.h>

#include "Core/CacheSchema.h"
#include "Core/CacheSettings.h"
#include "Core/WSCacheState.h"
#include "Core/SchemaContext.h"
#include "Core/SchemaManager.h"
#include "Core/Version.h"
#include "Upgrade/Upgrader.h"

#include "../Logging.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ECDbDebugInfoHolder> CreateLoggerHolder(WSCacheState& state, Utf8CP context)
    {
    if (!LOG.isSeverityEnabled(BentleyApi::NativeLogging::LOG_TRACE))
        {
        return nullptr;
        }

    bvector<ECN::ECSchemaCP> schemas;
    state.GetECDbAdapter().GetECDb().Schemas().GetECSchemas(schemas);

    return std::make_shared<ECDbDebugInfoHolder>(state.GetECDbAdapter().GetECDb(), schemas, "DataSourceCache debug information", context);
    }

// Add this macro to start of a method to print out cache state before and after method is done
#define LogCacheDataForMethod() auto _ecdb_debug_info_holder_ = CreateLoggerHolder (*m_state, __FUNCTION__)

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceCache::DataSourceCache()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceCache::~DataSourceCache()
    {
    Close();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::Create
(
BeFileNameCR cacheFilePath,
CacheEnvironmentCR environment,
const ECDb::CreateParams& params
)
    {
    BeFileName::CreateNewDirectory(cacheFilePath.GetDirectoryName());

    DbResult status = m_db.CreateNewDb(cacheFilePath, BeSQLite::BeGuid(), params);
    if (BE_SQLITE_OK != status)
        {
        LOG.error("Failed to create new ECDb");
        BeAssert(false);
        return ERROR;
        }

    return ExecuteWithinTransaction([&]
        {
        if (SUCCESS != InitializeCreatedDb())
            {
            return ERROR;
            }

        SetupOpenState(environment);
        return SUCCESS;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::Open
(
BeFileNameCR cacheFilePath,
CacheEnvironmentCR environment,
const ECDb::OpenParams& params
)
    {
    if (SUCCESS != Upgrader::FinalizeUpgradeIfNeeded(cacheFilePath, environment))
        {
        return ERROR;
        }

    DbResult status = m_db.OpenBeSQLiteDb(cacheFilePath, params);
    if (BE_SQLITE_OK != status)
        {
        return ERROR;
        }

    if (SUCCESS != UpgradeIfNeeded(cacheFilePath, environment, params))
        {
        return ERROR;
        }

    return ExecuteWithinTransaction([&]
        {
        SetupOpenState(environment);
        return SUCCESS;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::Close()
    {
    m_state = nullptr;
    if (m_db.IsDbOpen())
        {
        m_db.CloseDb();
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::UpgradeIfNeeded
(
BeFileNameCR cacheFilePath,
CacheEnvironmentCR environment,
const ECDb::OpenParams& params
)
    {
    int version = 0;

    auto status = ExecuteWithinTransaction([&]
        {
        CacheSettings settings(m_db);
        if (SUCCESS != settings.Read())
            {
            return ERROR;
            }
        version = settings.GetVersion();
        return SUCCESS;
        });

    if (SUCCESS != status)
        {
        // DB is not initialized
        return ExecuteWithinTransaction([&]
            {
            return InitializeCreatedDb();
            });
        }

    if (version == WSCACHE_FORMAT_VERSION)
        {
        return SUCCESS;
        }

    m_db.CloseDb();
    if (BE_SQLITE_OK != m_db.OpenBeSQLiteDb(cacheFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)))
        {
        return ERROR;
        }

    Upgrader upgrader(m_db, environment);
    if (SUCCESS != upgrader.Upgrade(version))
        {
        BeAssert(false && "<Error> Failed to upgrade to current version");
        return ERROR;
        }

    m_db.CloseDb();
    if (BE_SQLITE_OK != m_db.OpenBeSQLiteDb(cacheFilePath, params))
        {
        return ERROR;
        }

    return ExecuteWithinTransaction([&]
        {
        CacheSettings settings(m_db);
        settings.SetVersion(WSCACHE_FORMAT_VERSION);
        return settings.Save();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::InitializeCreatedDb()
    {
    if (SUCCESS != SchemaManager(m_db).ImportCacheSchemas())
        {
        return ERROR;
        }

    CacheSettings settings(m_db);
    settings.SetVersion(WSCACHE_FORMAT_VERSION);
    return settings.Save();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::ExecuteWithinTransaction(std::function<BentleyStatus()> execute)
    {
    Savepoint sp(m_db, "ExecuteWithinTransaction", true, BeSQLiteTxnMode::Immediate);

    if (SUCCESS != execute())
        {
        sp.Cancel();
        return ERROR;
        }

    if (BE_SQLITE_OK != sp.Commit())
        {
        return ERROR;
        }

    if (0 != m_db.GetCurrentSavepointDepth())
        {
        // Default transaction is enabled, save changes to it
        if (BE_SQLITE_OK != m_db.SaveChanges())
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void DataSourceCache::SetupOpenState(CacheEnvironmentCR environment)
    {
    BeFileName cachePath(m_db.GetDbFileName());
    m_state = std::make_shared<WSCacheState>(m_db, FileStorage::CreateCacheEnvironment(cachePath, environment));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void DataSourceCache::ClearRuntimeCaches()
    {
    if (m_state)
        {
        m_state->ClearRuntimeCaches();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DataSourceCache::RegisterSchemaChangeListener(IECDbSchemaChangeListener* listener)
    {
    m_db.RegisterSchemaChangeListener(listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DataSourceCache::UnRegisterSchemaChangeListener(IECDbSchemaChangeListener* listener)
    {
    m_db.UnRegisterSchemaChangeListener(listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::UpdateSchemas(const std::vector<BeFileName>& schemaPaths)
    {
    return SchemaManager(m_db).ImportSchemas(schemaPaths);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::UpdateSchemas(const std::vector<ECSchemaPtr>& schemas)
    {
    return SchemaManager(m_db).ImportSchemas(schemas);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::DeleteCacheFromDisk(BeFileNameCR cacheFilePath, CacheEnvironmentCR environment)
    {
    if (SUCCESS != FileStorage::DeleteFileCacheDirectories(FileStorage::CreateCacheEnvironment(cacheFilePath, environment)))
        {
        return ERROR;
        }

    if (cacheFilePath.DoesPathExist() && BeFileNameStatus::Success != BeFileName::BeDeleteFile(cacheFilePath))
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::Reset()
    {
    LogCacheDataForMethod();

    if (SUCCESS != FileStorage::DeleteFileCacheDirectories(m_state->GetFileCacheEnvironment()))
        {
        return ERROR;
        }

    // Remove all cached objects
    // TODO: remove this when CachedObjectInfoRelationship will be holding cached objects
    if (SUCCESS != m_state->GetObjectInfoManager().RemoveAllCachedInstances())
        {
        return ERROR;
        }

    // Remove all data
    bvector<ECN::ECSchemaCP> ecSchemas;
    if (SUCCESS != m_db.Schemas().GetECSchemas(ecSchemas))
        {
        BeAssert(false);
        return ERROR;
        }

    bset<ECClassCP> ignoreClasses;
    ignoreClasses.insert(m_db.Schemas().GetECClass("Bentley_Standard_Classes", "AnyClass"));
    ignoreClasses.insert(m_db.Schemas().GetECClass("Bentley_Standard_Classes", "InstanceCount"));

    bset<ECSchemaCP> ignoreSchemas;
    ignoreSchemas.insert(m_db.Schemas().GetECSchema("ECDb_System"));
    ignoreSchemas.insert(m_db.Schemas().GetECSchema("MetaSchema"));

    for (ECSchemaCP ecSchema : ecSchemas)
        {
        if (ignoreSchemas.count(ecSchema))
            {
            continue;
            }

        for (ECClassCP ecClass : ecSchema->GetClasses())
            {
            if (!ecClass->IsEntityClass() ||
                ecClass->IsRelationshipClass() ||
                ignoreClasses.count(ecClass))
                {
                continue;
                }

            Utf8PrintfString key("DataSourceCache::Reset:%llu", ecClass->GetId().GetValue());
            auto statement = m_state->GetStatementCache().GetPreparedStatement(key, [&]
                {
                return Utf8PrintfString("SELECT %llu, ECInstanceId FROM ONLY %s", ecClass->GetId().GetValue(),
                                        ecClass->GetECSqlName().c_str());
                });

            if (SUCCESS != m_state->GetHierarchyManager().DeleteInstances(*statement))
                {
                BeAssert(false);
                return ERROR;
                }
            }
        }

    m_state = std::make_shared<WSCacheState>(m_db, m_state->GetFileCacheEnvironment());
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DataSourceCache::GetCacheDatabasePath()
    {
    return BeFileName(m_db.GetDbFileName(), true);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IECDbAdapterR DataSourceCache::GetAdapter()
    {
    return m_state->GetECDbAdapter();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IExtendedDataAdapter& DataSourceCache::GetExtendedDataAdapter()
    {
    return m_state->GetExtendedDataAdapter();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ObservableECDb& DataSourceCache::GetECDb()
    {
    return m_db;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IChangeManagerR DataSourceCache::GetChangeManager()
    {
    LogCacheDataForMethod();
    return m_state->GetChangeManager();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectId DataSourceCache::ObjectIdFromJsonInstance(JsonValueCR instance) const
    {
    ObjectId objectId;

    ECDbHelper::ParseECClassKey(instance[DataSourceCache_PROPERTY_ClassKey].asString(), objectId.schemaName, objectId.className);
    objectId.remoteId = instance[DataSourceCache_PROPERTY_RemoteId].asString();

    return objectId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DataSourceCache::ReadInstanceCacheTag(ObjectIdCR objectId)
    {
    return m_state->GetObjectInfoManager().ReadInfo(objectId).GetObjectCacheTag();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DataSourceCache::ReadFileCacheTag(ObjectIdCR objectId)
    {
    return m_state->GetFileInfoManager().ReadInfo(objectId).GetFileCacheTag();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime DataSourceCache::ReadInstanceCachedDate(ObjectIdCR objectId)
    {
    return m_state->GetObjectInfoManager().ReadInfo(objectId).GetObjectCacheDate();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::SetResponseAccessDate(CachedResponseKeyCR key, DateTimeCR utcDateTime)
    {
    ResponseKey responseKey = m_state->GetCachedResponseManager().ConvertResponseKey(key);
    CachedResponseInfo info = m_state->GetCachedResponseManager().ReadInfo(responseKey);
    if (!info.IsCached())
        {
        return ERROR;
        }
    info.SetAccessDate(utcDateTime);
    return m_state->GetCachedResponseManager().SaveInfo(info);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime DataSourceCache::ReadResponseAccessDate(CachedResponseKeyCR key)
    {
    ResponseKey responseKey = m_state->GetCachedResponseManager().ConvertResponseKey(key);
    return m_state->GetCachedResponseManager().ReadInfo(responseKey).GetAccessDate();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime DataSourceCache::ReadFileCachedDate(ObjectIdCR objectId)
    {
    return m_state->GetFileInfoManager().ReadInfo(objectId).GetFileCacheDate();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CachedObjectInfo DataSourceCache::GetCachedObjectInfo(ObjectIdCR objectId)
    {
    if (objectId.IsEmpty())
        {
        return CachedObjectInfo();
        }
    return CachedObjectInfo(std::make_shared<ObjectInfo>(m_state->GetObjectInfoManager().ReadInfo(objectId)));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedObjectInfo DataSourceCache::GetCachedObjectInfo(ECInstanceKeyCR instance)
    {
    return CachedObjectInfo(std::make_shared<ObjectInfo>(m_state->GetObjectInfoManager().ReadInfo(instance)));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey DataSourceCache::FindInstance(ObjectIdCR objectId)
    {
    if (objectId.IsEmpty())
        {
        return m_state->GetNavigationBaseManager().FindNavigationBase();
        }

    return m_state->GetObjectInfoManager().FindCachedInstance(objectId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectId DataSourceCache::FindInstance(ECInstanceKeyCR instanceKey)
    {
    return m_state->GetObjectInfoManager().FindCachedInstance(instanceKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey DataSourceCache::FindRelationship(ECRelationshipClassCR relClass, ECInstanceKeyCR source, ECInstanceKeyCR target)
    {
    return m_state->GetECDbAdapter().FindRelationship(&relClass, source, target);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey DataSourceCache::FindRelationship(ECRelationshipClassCR relClass, ObjectIdCR sourceId, ObjectIdCR targetId)
    {
    ECInstanceKey source = m_state->GetObjectInfoManager().FindCachedInstance(sourceId);
    ECInstanceKey target = m_state->GetObjectInfoManager().FindCachedInstance(targetId);
    return m_state->GetECDbAdapter().FindRelationship(&relClass, source, target);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectId DataSourceCache::FindRelationship(ECInstanceKeyCR relationshipKey)
    {
    return m_state->GetRelationshipInfoManager().ReadObjectId(relationshipKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::CacheInstanceAndLinkToRoot
(
ObjectIdCR objectId,
WSObjectsResponseCR response,
Utf8StringCR rootName
)
    {
    LogCacheDataForMethod();
    CachedInstanceKey key;

    if (response.IsModified())
        {
        WSObjectsReader::Instances instances = response.GetInstances();

        InstanceCacheHelper::CachedInstances cachedInstances;
        if (SUCCESS != m_state->GetInstanceHelper().CacheInstances(instances, cachedInstances))
            {
            return ERROR;
            }
        if (1 != cachedInstances.GetCachedInstances().size())
            {
            return ERROR;
            }
        auto it = cachedInstances.GetCachedInstancesByObjectId().find(objectId);
        if (cachedInstances.GetCachedInstancesByObjectId().end() == it)
            {
            return ERROR;
            }
        key = *cachedInstances.GetCachedInstances().begin();
        }
    else
        {
        ObjectInfo info = m_state->GetObjectInfoManager().ReadInfo(objectId);
        info.SetObjectCacheDate(DateTime::GetCurrentTimeUtc());
        if (SUCCESS != m_state->GetObjectInfoManager().UpdateInfo(info))
            {
            return ERROR;
            }
        key = info.GetCachedInstanceKey();
        }

    CacheRootKey rootKey = m_state->GetRootManager().FindOrCreateRoot(rootName);
    if (SUCCESS != m_state->GetRootManager().LinkExistingNodeToRoot(rootKey, key.GetInfoKey()))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::CacheInstanceAndLinkToRoot
(
ObjectIdCR objectId,
RapidJsonValueCR instancePropertiesJson,
Utf8StringCR cacheTag,
Utf8StringCR rootName
)
    {
    LogCacheDataForMethod();
    ObjectInfo info = m_state->GetObjectInfoManager().ReadInfo(objectId);

    info.SetObjectCacheTag(cacheTag);
    info.SetObjectCacheDate(DateTime::GetCurrentTimeUtc());
    info.SetObjectState(CachedInstanceState::Full);

    CacheRootKey rootKey = m_state->GetRootManager().FindOrCreateRoot(rootName);
    if (SUCCESS != m_state->GetRootManager().LinkNewInstanceToRoot(rootKey, objectId, info, &instancePropertiesJson))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::CacheInstancesAndLinkToRoot
(
WSObjectsResponseCR response,
Utf8StringCR rootName,
ECInstanceKeyMultiMap* cachedInstanceKeysOut,
bool weakLinkToRoot,
ICancellationTokenPtr ct
)
    {
    LogCacheDataForMethod();

    if (!response.IsModified())
        {
        BeAssert(false && "NotModified is not supported for this method ");
        return ERROR;
        }

    WSObjectsReader::Instances instances = response.GetInstances();
    InstanceCacheHelper::CachedInstances cachedInstances;
    if (SUCCESS != m_state->GetInstanceHelper().CacheInstances(instances, cachedInstances, nullptr, nullptr, ct))
        {
        return ERROR;
        }

    CacheRootKey rootKey = m_state->GetRootManager().FindOrCreateRoot(rootName);
    for (CachedInstanceKeyCR key : cachedInstances.GetCachedInstances())
        {
        if (SUCCESS != m_state->GetRootManager().LinkExistingNodeToRoot(rootKey, key.GetInfoKey(), !weakLinkToRoot))
            {
            return ERROR;
            }
        }

    if (nullptr != cachedInstanceKeysOut)
        {
        for (CachedInstanceKeyCR key : cachedInstances.GetCachedInstances())
            {
            cachedInstanceKeysOut->Insert(key.GetInstanceKey().GetECClassId(), key.GetInstanceKey().GetECInstanceId());
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::UpdateInstance(ObjectIdCR objectId, WSObjectsResponseCR response)
    {
    LogCacheDataForMethod();

    if (response.IsModified())
        {
        WSObjectsReader::Instances instances = response.GetInstances();

        InstanceCacheHelper::CachedInstances cachedInstances;
        InstanceCacheHelper::UpdateCachingState updateCachingState;
        if (SUCCESS != m_state->GetInstanceHelper().CacheInstances(instances, cachedInstances, nullptr, &updateCachingState))
            {
            return ERROR;
            }
        if (1 != cachedInstances.GetCachedInstances().size() || !updateCachingState.GetNotFoundObjectIds().empty())
            {
            return ERROR;
            }
        }
    else
        {
        ObjectInfo info = m_state->GetObjectInfoManager().ReadInfo(objectId);
        info.SetObjectCacheDate(DateTime::GetCurrentTimeUtc());
        if (SUCCESS != m_state->GetObjectInfoManager().UpdateInfo(info))
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::UpdateInstances
(
WSObjectsResponseCR response,
bset<ObjectId>* notFoundOut,
bset<ECInstanceKey>* cachedInstancesOut,
ICancellationTokenPtr ct
)
    {
    LogCacheDataForMethod();

    if (!response.IsModified())
        {
        BeAssert(false && "NotModified is not supported for this method ");
        return ERROR;
        }

    WSObjectsReader::Instances instances = response.GetInstances();

    InstanceCacheHelper::CachedInstances cachedInstances;
    InstanceCacheHelper::UpdateCachingState updateCachingState;
    if (SUCCESS != m_state->GetInstanceHelper().CacheInstances(instances, cachedInstances, nullptr, &updateCachingState, ct))
        {
        return ERROR;
        }

    if (nullptr != notFoundOut)
        {
        notFoundOut->insert(updateCachingState.GetNotFoundObjectIds().begin(), updateCachingState.GetNotFoundObjectIds().end());
        }

    if (nullptr != cachedInstancesOut)
        {
        for (auto& cachedKey : cachedInstances.GetCachedInstances())
            {
            cachedInstancesOut->insert(cachedKey.GetInstanceKey());
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CacheStatus DataSourceCache::ReadInstance(ObjectIdCR objectId, JsonValueR instanceDataOut, JsonFormat format)
    {
    instanceDataOut = Json::nullValue;

    if (objectId.IsEmpty())
        {
        return CacheStatus::Error;
        }

    ECClassCP ecClass = m_state->GetECDbAdapter().GetECClass(objectId);
    if (nullptr == ecClass)
        {
        return CacheStatus::Error;
        }

    auto statement = GetReadInstanceStatement(*ecClass, objectId.remoteId.c_str());

    DbResult status = statement->Step();
    if (BE_SQLITE_ROW != status && BE_SQLITE_DONE != status)
        {
        return CacheStatus::Error;
        }
    if (BE_SQLITE_ROW != status)
        {
        return CacheStatus::DataNotCached;
        }

    if (JsonFormat::Display == format)
        {
        instanceDataOut[DataSourceCache_PROPERTY_ClassKey] = ECDbHelper::ECClassKeyFromClass(*ecClass);
        instanceDataOut[DataSourceCache_PROPERTY_RemoteId] = objectId.remoteId;

        JsonECSqlSelectAdapter adapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::FormattedStrings));

        JsonValueR instanceDisplayData = instanceDataOut[DataSourceCache_PROPERTY_DisplayData];
        adapter.GetRowDisplayInfo(instanceDataOut[DataSourceCache_PROPERTY_DisplayInfo]);

        if (!adapter.GetRowInstance(instanceDisplayData, ecClass->GetId()))
            {
            instanceDataOut = Json::nullValue;
            return CacheStatus::Error;
            }

        instanceDisplayData[DataSourceCache_PROPERTY_RemoteId] = objectId.remoteId;
        }

    JsonValueR instanceRawData = JsonFormat::Display == format ? instanceDataOut[DataSourceCache_PROPERTY_RawData] : instanceDataOut;

    JsonECSqlSelectAdapter adapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    if (!adapter.GetRowInstance(instanceRawData, ecClass->GetId()))
        {
        instanceDataOut = Json::nullValue;
        return CacheStatus::Error;
        }

    instanceRawData[DataSourceCache_PROPERTY_RemoteId] = objectId.remoteId;

    return CacheStatus::OK;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr DataSourceCache::ReadInstance(ObjectIdCR objectId)
    {
    if (!objectId.IsValid())
        {
        return nullptr;
        }

    ECClassCP ecClass = m_state->GetECDbAdapter().GetECClass(objectId);
    if (nullptr == ecClass)
        {
        return nullptr;
        }

    auto statement = GetReadInstanceStatement(*ecClass, objectId.remoteId.c_str());
    if (BE_SQLITE_ROW != statement->Step())
        {
        return nullptr;
        }

    IECInstancePtr instance = ECInstanceECSqlSelectAdapter(*statement).GetInstance();
    if (instance.IsNull())
        {
        return nullptr;
        }

    instance->SetInstanceId(objectId.remoteId.c_str());
    return instance;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ECSqlStatement> DataSourceCache::GetReadInstanceStatement(ECClassCR ecClass, Utf8CP remoteId)
    {
    Utf8PrintfString key("DataSourceCache::GetReadInstanceStatement:RemoteId:%llu", ecClass.GetId().GetValue());
    auto statement = m_state->GetStatementCache().GetPreparedStatement(key, [&]
        {
        return CacheQueryHelper::ECSql::SelectAllPropertiesByRemoteId(ecClass);
        });
    statement->BindText(1, remoteId, IECSqlBinder::MakeCopy::No);
    return statement;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ECSqlStatement> DataSourceCache::GetReadInstanceStatement(ECClassCR ecClass, ECInstanceId ecInstanceId)
    {
    Utf8PrintfString key("DataSourceCache::GetReadInstanceStatement:ECInstanceId:%llu", ecClass.GetId().GetValue());
    auto statement = m_state->GetStatementCache().GetPreparedStatement(key, [&]
        {
        return CacheQueryHelper::ECSql::SelectAllPropertiesAndRemoteIdByECInstanceId(ecClass);
        });
    statement->BindId(1, ecInstanceId);
    return statement;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr DataSourceCache::ReadInstance(ECInstanceKeyCR instanceKey)
    {
    ECClassCP ecClass = m_state->GetECDbAdapter().GetECClass(instanceKey);
    if (nullptr == ecClass)
        {
        return nullptr;
        }

    auto statement = GetReadInstanceStatement(*ecClass, instanceKey.GetECInstanceId());
    if (BE_SQLITE_ROW != statement->Step())
        {
        return nullptr;
        }

    IECInstancePtr instance = ECInstanceECSqlSelectAdapter(*statement).GetInstance(ecClass->GetId());
    if (instance.IsNull())
        {
        return nullptr;
        }

    instance->SetInstanceId(statement->GetValueText(0));
    return instance;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Benjamin.Brown  08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CacheStatus DataSourceCache::RemoveInstance(ObjectIdCR objectId)
    {
    LogCacheDataForMethod();
    if (objectId.IsEmpty())
        {
        CacheNodeKey key = m_state->GetNavigationBaseManager().FindNavigationBase();
        if (!key.IsValid())
            {
            return CacheStatus::DataNotCached;
            }
        if (SUCCESS != m_state->GetHierarchyManager().DeleteInstance(key))
            {
            return CacheStatus::Error;
            }
        return CacheStatus::OK;
        }

    CachedInstanceKey cachedKey = m_state->GetObjectInfoManager().ReadCachedInstanceKey(objectId);
    if (!cachedKey.IsValid())
        {
        return CacheStatus::DataNotCached;
        }

    if (SUCCESS != m_state->GetCachedResponseManager().InvalidateResponsePagesContainingInstance(cachedKey) ||
        SUCCESS != m_state->GetHierarchyManager().DeleteInstance(cachedKey.GetInfoKey()))
        {
        return CacheStatus::Error;
        }

    return CacheStatus::OK;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Benjamin.Brown  09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::RemoveFile(ObjectIdCR objectId)
    {
    LogCacheDataForMethod();
    FileInfo fileInfo = m_state->GetFileInfoManager().ReadInfo(objectId);

    if (!fileInfo.IsInCache())
        {
        return SUCCESS;
        }

    if (SUCCESS != m_state->GetFileStorage().CleanupCachedFile(fileInfo.GetFilePath()))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CreateCommaSeperatedStringList(const bvector<Utf8CP>& strings)
    {
    Utf8String list;
    if (strings.size() == 0)
        {
        return list;
        }

    for (Utf8CP string : strings)
        {
        if (!list.empty())
            {
            list += ",";
            }
        list += Utf8PrintfString("'%s'", string);
        }

    return list;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::ReadInstances
(
const bset<ObjectId>& ids,
JsonValueR instancesArrayOut,
ISelectProviderCR selectProvider
)
    {
    bmap<ECClassCP, bvector<Utf8CP>> ecClassToRemoteIdsMap;

    for (ObjectIdCR objectId : ids)
        {
        if (!objectId.IsValid())
            {
            return ERROR;
            }
        ECClassCP ecClass = m_state->GetECDbAdapter().GetECClass(objectId);
        ecClassToRemoteIdsMap[ecClass].push_back(objectId.remoteId.c_str());
        }

    bvector<ECClassP> classesToSelect;
    for (auto iter : ecClassToRemoteIdsMap)
        {
        classesToSelect.push_back((ECClassP) iter.first);
        }

    CacheQueryHelper helper(selectProvider);
    bvector<CacheQueryHelper::ClassReadInfo> readInfos = helper.CreateReadInfos(classesToSelect);

    for (CacheQueryHelper::ClassReadInfo& info : readInfos)
        {
        Utf8String remoteIdList = CreateCommaSeperatedStringList(ecClassToRemoteIdsMap[&info.GetECClass()]);
        Utf8String ecSql = CacheQueryHelper::ECSql::SelectPropertiesByRemoteIds(info, remoteIdList);

        ECSqlStatement statement;
        if (SUCCESS != m_state->GetECDbAdapter().PrepareStatement(statement, ecSql))
            {
            return ERROR;
            }

        if (SUCCESS != CacheQueryHelper::ReadJsonInstances(info, statement, instancesArrayOut))
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::SetupRoot(Utf8StringCR rootName, CacheRootPersistence persistence)
    {
    LogCacheDataForMethod();
    if (SUCCESS != m_state->GetRootManager().SetupRoot(rootName, persistence))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool DataSourceCache::IsInstanceFullyPersisted(ObjectIdCR objectId)
    {
    CachedInstanceKey key = m_state->GetObjectInfoManager().ReadCachedInstanceKey(objectId);
    if (!key.IsValid())
        {
        return false;
        }

    ECInstanceKeyMultiMap persistedNodes;
    m_state->GetRootManager().GetNodesByPersistence(CacheRootPersistence::Full, persistedNodes);

    return ECDbHelper::IsInstanceInMultiMap(key.GetInfoKey(), persistedNodes);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool DataSourceCache::IsInstanceFullyPersisted(ECInstanceKeyCR instanceKey)
    {
    CacheNodeKey infoKey = m_state->GetObjectInfoManager().ReadInfoKey(instanceKey);
    if (!infoKey.IsValid())
        {
        return false;
        }

    ECInstanceKeyMultiMap persistedNodes;
    m_state->GetRootManager().GetNodesByPersistence(CacheRootPersistence::Full, persistedNodes);

    return ECDbHelper::IsInstanceInMultiMap(infoKey, persistedNodes);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::ReadFullyPersistedInstanceKeys(ECInstanceKeyMultiMap& instancesOut)
    {
    ECInstanceKeyMultiMap persistedNodes;
    if (SUCCESS != m_state->GetRootManager().GetNodesByPersistence(CacheRootPersistence::Full, persistedNodes) ||
        SUCCESS != m_state->GetObjectInfoManager().ReadCachedInstanceKeys(persistedNodes, instancesOut))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::LinkInstanceToRoot(Utf8StringCR rootName, ObjectIdCR objectId)
    {
    LogCacheDataForMethod();

    CacheRootKey rootKey = m_state->GetRootManager().FindOrCreateRoot(rootName);

    if (objectId.IsEmpty())
        {
        CacheNodeKey baseKey = m_state->GetNavigationBaseManager().FindOrCreateNavigationBase();
        return m_state->GetRootManager().LinkExistingNodeToRoot(rootKey, baseKey);
        }

    return m_state->GetRootManager().LinkInstanceToRoot(rootKey, objectId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool DataSourceCache::IsInstanceInRoot(Utf8StringCR rootName, ECInstanceKeyCR instance)
    {
    CacheRootKey rootKey = m_state->GetRootManager().FindRoot(rootName);
    CacheNodeKey infoKey = m_state->GetObjectInfoManager().ReadInfoKey(instance);
    return m_state->GetRootManager().IsNodeInRoot(rootKey, infoKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool DataSourceCache::IsInstanceConnectedToRoot(Utf8StringCR rootName, ECInstanceKeyCR instance)
    {
    if (!instance.IsValid())
        {
        return false;
        }

    CacheRootKey rootKey = m_state->GetRootManager().FindRoot(rootName);
    if (!rootKey.IsValid())
        {
        return false;
        }

    bset<CacheRootKey> rootKeys;
    rootKeys.insert(rootKey);

    CacheNodeKey infoKey = m_state->GetObjectInfoManager().ReadInfoKey(instance);

    return m_state->GetRootManager().IsNodeConnectedToAnyOfRoots(rootKeys, infoKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::ReadInstancesLinkedToRoot
(
Utf8StringCR rootName,
JsonValueR instancesOut,
ISelectProviderCR selectProvider
)
    {
    if (instancesOut.isNull())
        {
        instancesOut = Json::arrayValue;
        }

    ECInstanceKeyMultiMap nodeKeys, instanceKeys;
    if (SUCCESS != m_state->GetRootManager().GetNodesLinkedToRoot(rootName, nodeKeys) ||
        SUCCESS != m_state->GetObjectInfoManager().ReadCachedInstanceKeys(nodeKeys, instanceKeys))
        {
        return ERROR;
        }

    CacheQueryHelper helper(selectProvider);
    if (SUCCESS != helper.ReadInstances(m_state->GetECDbAdapter(), instanceKeys,
        [&] (const CacheQueryHelper::ClassReadInfo& info, ECSqlStatement& statement)
        {
        return CacheQueryHelper::ReadJsonInstances(info, statement, instancesOut);
        }))
        {
        return ERROR;
        }

        return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::ReadInstancesLinkedToRoot
(
Utf8StringCR rootName,
ECInstanceKeyMultiMap& instanceKeys
)
    {
    ECInstanceKeyMultiMap nodeKeys;
    if (SUCCESS != m_state->GetRootManager().GetNodesLinkedToRoot(rootName, nodeKeys) ||
        SUCCESS != m_state->GetObjectInfoManager().ReadCachedInstanceKeys(nodeKeys, instanceKeys))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::ReadInstancesConnectedToRootMap(Utf8StringCR rootName, ECInstanceKeyMultiMap& instancesOut, uint8_t depth)
    {
    CacheRootKey rootKey = m_state->GetRootManager().FindRoot(rootName);
    if (!rootKey.IsValid())
        {
        return SUCCESS;
        }

    bset<CacheRootKey> rootKeys;
    rootKeys.insert(rootKey);

    ECInstanceKeyMultiMap nodeKeys;
    if (SUCCESS != m_state->GetRootManager().GetNodesConnectedToRoots(rootKeys, nodeKeys, depth) ||
        SUCCESS != m_state->GetObjectInfoManager().ReadCachedInstanceKeys(nodeKeys, instancesOut))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::ReadInstanceHierarchy(ECInstanceKeyCR instance, ECInstanceKeyMultiMap& instancesOut)
    {
    auto cachedKey = m_state->GetObjectInfoManager().ReadCachedInstanceKey(instance);
    if (!cachedKey.IsValid())
        return ERROR;

    ECInstanceKeyMultiMap nodeKeys;
    ECInstanceKeyMultiMap ansestorNodes;
    ansestorNodes.insert(ECDbHelper::ToPair(cachedKey.GetInfoKey()));
    ECInstanceFinder::FindOptions findOptions(ECInstanceFinder::RelatedDirection::RelatedDirection_HeldChildren, UINT8_MAX);

    if (SUCCESS != m_state->GetECDbAdapter().GetECInstanceFinder().FindInstances(nodeKeys, ansestorNodes, findOptions))
        return ERROR;

    // TODO: ECDb ECInstanceFinder::FindInstances() also returns seed instances, neeed to remove them
    ECDbHelper::Erase(nodeKeys, cachedKey.GetInfoKey());

    if (SUCCESS != m_state->GetObjectInfoManager().ReadCachedInstanceKeys(nodeKeys, instancesOut))
        return ERROR;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::UnlinkInstanceFromRoot(Utf8StringCR rootName, ObjectIdCR objectId)
    {
    LogCacheDataForMethod();
    CacheNodeKey nodeKey;
    if (objectId.IsEmpty())
        {
        nodeKey = m_state->GetNavigationBaseManager().FindNavigationBase();
        }
    else
        {
        nodeKey = m_state->GetObjectInfoManager().ReadCachedInstanceKey(objectId).GetInfoKey();
        }

    CacheRootKey rootKey = m_state->GetRootManager().FindRoot(rootName);
    if (!rootKey.IsValid())
        {
        return SUCCESS;
        }

    if (SUCCESS != m_state->GetRootManager().UnlinkNodeFromRoot(rootKey, nodeKey))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::UnlinkAllInstancesFromRoot(Utf8StringCR rootName)
    {
    LogCacheDataForMethod();
    CacheRootKey rootKey = m_state->GetRootManager().FindRoot(rootName);
    if (!rootKey.IsValid())
        {
        return SUCCESS;
        }
    if (SUCCESS != m_state->GetRootManager().UnlinkAllInstancesFromRoot(rootKey))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::RemoveRoot(Utf8StringCR rootName)
    {
    LogCacheDataForMethod();
    if (SUCCESS != m_state->GetRootManager().RemoveRoot(rootName))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::RemoveRootsByPrefix(Utf8StringCR rootPrefix)
    {
    LogCacheDataForMethod();
    if (SUCCESS != m_state->GetRootManager().RemoveRootsByPrefix(rootPrefix))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::SetFileCacheLocation(ObjectIdCR objectId, FileCache cacheLocation, BeFileNameCR externalRelativePath)
    {
    //! TODO: consider removing FileCache parameter and auotmaically use Root persistence instead
    LogCacheDataForMethod();
    FileInfo info = m_state->GetFileInfoManager().ReadInfo(objectId);
    if (SUCCESS != m_state->GetFileStorage().SetFileCacheLocation(info, cacheLocation, &externalRelativePath) ||
        SUCCESS != m_state->GetFileInfoManager().SaveInfo(info))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
FileCache DataSourceCache::GetFileCacheLocation(ObjectIdCR objectId, FileCache defaultLocation)
    {
    FileInfo info = m_state->GetFileInfoManager().ReadInfo(objectId);
    return info.GetLocation(defaultLocation);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
BentleyStatus DataSourceCache::CacheFile
(
ObjectIdCR objectId,
WSFileResponseCR fileResult,
FileCache cacheLocation
)
    {
    LogCacheDataForMethod();

    FileInfo info = m_state->GetFileInfoManager().ReadInfo(objectId);
    if (!info.IsValid())
        {
        return ERROR;
        }

    if (!fileResult.IsModified())
        {
        if (!info.IsInCache())
            {
            return ERROR;
            }

        info.SetFileCacheDate(DateTime::GetCurrentTimeUtc());
        }
    else
        {
        auto path = fileResult.GetFilePath();
        auto eTag = fileResult.GetETag();
        auto time = DateTime::GetCurrentTimeUtc();
        if (SUCCESS != m_state->GetFileStorage().CacheFile(info, path, eTag.c_str(), cacheLocation, time, false))
            {
            return ERROR;
            }
        }

    if (SUCCESS != m_state->GetFileInfoManager().SaveInfo(info))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DataSourceCache::ReadFilePath(ObjectIdCR objectId)
    {
    CachedInstanceKey cachedKey = m_state->GetObjectInfoManager().ReadCachedInstanceKey(objectId);
    return m_state->GetFileInfoManager().ReadFilePath(cachedKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DataSourceCache::ReadFilePath(ECInstanceKeyCR instanceKey)
    {
    CachedObjectInfoKey infoKey = m_state->GetObjectInfoManager().ReadInfoKey(instanceKey);
    return m_state->GetFileInfoManager().ReadFilePath({infoKey, instanceKey});
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::RemoveFilesInTemporaryPersistence()
    {
    LogCacheDataForMethod();

    ECInstanceKeyMultiMap fullyPersistedNodes;
    if (SUCCESS != m_state->GetRootManager().GetNodesByPersistence(CacheRootPersistence::Full, fullyPersistedNodes) ||
        SUCCESS != m_state->GetFileInfoManager().DeleteFilesNotHeldByNodes(fullyPersistedNodes))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey DataSourceCache::FindOrCreateRoot(Utf8StringCR rootName)
    {
    return m_state->GetRootManager().FindOrCreateRoot(rootName);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool DataSourceCache::DoesRootExist(Utf8StringCR rootName)
    {
    return m_state->GetRootManager().FindRoot(rootName).IsValid();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::RenameRoot(Utf8StringCR rootName, Utf8StringCR newRootName)
    {
    return m_state->GetRootManager().RenameRoot(rootName, newRootName);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::SetRootSyncDate(Utf8StringCR rootName, DateTimeCR utcDateTime)
    {
    return m_state->GetRootManager().SetRootSyncDate(rootName, utcDateTime);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime DataSourceCache::ReadRootSyncDate(Utf8StringCR rootName)
    {
    return m_state->GetRootManager().ReadRootSyncDate(rootName);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::CacheResponse
(
CachedResponseKeyCR key,
WSObjectsResponseCR response,
bset<ObjectId>* rejectedOut,
const WSQuery* query,
uint64_t page,
ICancellationTokenPtr ct
)
    {
    LogCacheDataForMethod();

    ResponseKey responseKey = m_state->GetCachedResponseManager().ConvertResponseKey(key);
    if (!responseKey.IsValid())
        {
        return ERROR;
        }

    if (!response.IsModified())
        {
        if (SUCCESS != m_state->GetCachedResponseManager().UpdatePageCachedDate(responseKey, page))
            {
            return ERROR;
            }
        }
    else
        {
        ECInstanceKeyMultiMap fullyPersistedInstances;
        std::shared_ptr<InstanceCacheHelper::PartialCachingState> partialCachingState;

        if (nullptr != query)
            {
            if (nullptr == rejectedOut)
                {
                BeAssert(false);
                return ERROR;
                }

            ECInstanceKeyMultiMap fullyPersistedNodes;
            if (SUCCESS != m_state->GetRootManager().GetNodesByPersistence(CacheRootPersistence::Full, fullyPersistedNodes) ||
                SUCCESS != m_state->GetObjectInfoManager().ReadCachedInstanceKeys(fullyPersistedNodes, fullyPersistedInstances))
                {
                return ERROR;
                }

            partialCachingState = std::make_shared<InstanceCacheHelper::PartialCachingState>
                (
                m_state->GetECDbAdapter(),
                *query,
                fullyPersistedInstances,
                *rejectedOut
                );
            }

        WSObjectsReader::Instances instances = response.GetInstances();
        InstanceCacheHelper::CachedInstances cachedInstances;

        if (SUCCESS != m_state->GetInstanceHelper().CacheInstances(instances, cachedInstances, partialCachingState.get(), nullptr, ct) ||
            SUCCESS != m_state->GetCachedResponseManager().SavePage(responseKey, page, response.GetETag(), cachedInstances))
            {
            return ERROR;
            }
        }

    if (response.IsFinal())
        {
        if (SUCCESS != m_state->GetCachedResponseManager().TrimPages(responseKey, page))
            {
            return ERROR;
            }
        }

    bool wasCompleted = m_state->GetCachedResponseManager().IsResponseCompleted(responseKey);
    bool nowCompleted = wasCompleted;

    if (response.IsFinal())
        {
        nowCompleted = true;
        }
    else if (response.IsModified())
        {
        nowCompleted = false;
        }

    if (wasCompleted != nowCompleted)
        {
        if (SUCCESS != m_state->GetCachedResponseManager().SetResponseCompleted(responseKey, nowCompleted))
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::MarkTemporaryInstancesAsPartial(const std::vector<CachedResponseKey>& responseKeys)
    {
    LogCacheDataForMethod();
    if (responseKeys.empty())
        {
        return SUCCESS;
        }

    ECInstanceKeyMultiMap fullyPersistedNodes;
    if (SUCCESS != m_state->GetRootManager().GetNodesByPersistence(CacheRootPersistence::Full, fullyPersistedNodes))
        {
        return ERROR;
        }

    bset<ECInstanceId> temporaryInfos;
    for (CachedResponseKeyCR key : responseKeys)
        {
        ResponseKey responseKey = m_state->GetCachedResponseManager().ConvertResponseKey(key);
        CacheNodeKey infoKey = m_state->GetCachedResponseManager().FindInfo(responseKey);
        if (!infoKey.IsValid())
            {
            continue;
            }
        if (!ECDbHelper::IsInstanceInMultiMap(infoKey, fullyPersistedNodes))
            {
            temporaryInfos.insert(infoKey.GetECInstanceId());
            }
        }

    if (SUCCESS != m_state->GetCachedResponseManager().MarkTemporaryInstancesAsPartial(temporaryInfos, fullyPersistedNodes))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::RemoveResponse(CachedResponseKeyCR key)
    {
    LogCacheDataForMethod();
    ResponseKey responseKey = m_state->GetCachedResponseManager().ConvertResponseKey(key);
    return m_state->GetCachedResponseManager().DeleteInfo(responseKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::RemoveTemporaryResponses(Utf8StringCR name, DateTimeCR accessedBeforeDateUtc)
    {
    LogCacheDataForMethod();

    ECInstanceKeyMultiMap fullyPersistedNodes;
    if (SUCCESS != m_state->GetRootManager().GetNodesByPersistence(CacheRootPersistence::Full, fullyPersistedNodes) ||
        SUCCESS != m_state->GetCachedResponseManager().DeleteResponses(name, accessedBeforeDateUtc, fullyPersistedNodes))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::RemoveResponses(Utf8StringCR name)
    {
    LogCacheDataForMethod();
    return m_state->GetCachedResponseManager().DeleteResponses(name);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheStatus DataSourceCache::ReadResponse
(
CachedResponseKeyCR key,
JsonValueR instancesOut,
ISelectProviderCR selectProvider
)
    {
    if (instancesOut.isNull() || !instancesOut.isArray())
        {
        instancesOut = Json::arrayValue;
        }

    ResponseKey responseKey = m_state->GetCachedResponseManager().ConvertResponseKey(key);
    CachedResponseInfo queryInfo = m_state->GetCachedResponseManager().ReadInfo(responseKey);
    if (!queryInfo.IsCached())
        {
        return CacheStatus::DataNotCached;
        }

    if (SUCCESS != m_state->GetCachedResponseManager().ReadResponse(queryInfo, selectProvider,
        [&] (const CacheQueryHelper::ClassReadInfo& info, ECSqlStatement& statement)
        {
        return CacheQueryHelper::ReadJsonInstances(info, statement, instancesOut);
        }))
        {
        return CacheStatus::Error;
        }

        return CacheStatus::OK;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheStatus DataSourceCache::ReadResponseInstanceKeys(CachedResponseKeyCR key, ECInstanceKeyMultiMap& instanceKeysOut)
    {
    ResponseKey responseKey = m_state->GetCachedResponseManager().ConvertResponseKey(key);
    CachedResponseInfo queryInfo = m_state->GetCachedResponseManager().ReadInfo(responseKey);
    if (!queryInfo.IsCached())
        {
        return CacheStatus::DataNotCached;
        }

    if (SUCCESS != m_state->GetCachedResponseManager().ReadResponseInstanceKeys(queryInfo.GetInfoKey(), instanceKeysOut))
        {
        return CacheStatus::Error;
        }

    return CacheStatus::OK;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheStatus DataSourceCache::ReadResponseObjectIds
(
CachedResponseKeyCR key,
bset<ObjectId>& instanceObjectIdsOut
)
    {
    ResponseKey responseKey = m_state->GetCachedResponseManager().ConvertResponseKey(key);
    CacheNodeKey infoKey = m_state->GetCachedResponseManager().FindInfo(responseKey);
    if (!infoKey.IsValid())
        {
        return CacheStatus::DataNotCached;
        }

    if (SUCCESS != m_state->GetCachedResponseManager().ReadResponseObjectIds(infoKey, instanceObjectIdsOut))
        {
        return CacheStatus::Error;
        }

    return CacheStatus::OK;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DataSourceCache::ReadResponseCacheTag(CachedResponseKeyCR key, uint64_t page)
    {
    ResponseKey responseKey = m_state->GetCachedResponseManager().ConvertResponseKey(key);
    return m_state->GetCachedResponseManager().ReadResponsePageCacheTag(responseKey, page);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime DataSourceCache::ReadResponseCachedDate(CachedResponseKeyCR key, uint64_t page)
    {
    ResponseKey responseKey = m_state->GetCachedResponseManager().ConvertResponseKey(key);
    return m_state->GetCachedResponseManager().ReadResponsePageCachedDate(responseKey, page);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool DataSourceCache::IsResponseCached(CachedResponseKeyCR key)
    {
    ResponseKey responseKey = m_state->GetCachedResponseManager().ConvertResponseKey(key);
    return m_state->GetCachedResponseManager().IsResponseCompleted(responseKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DataSourceCache::ReadInstanceLabel(ObjectIdCR objectId)
    {
    ECClassCP objectClass = m_state->GetECDbAdapter().GetECClass(objectId);
    if (nullptr == objectClass)
        {
        return "";
        }

    ECPropertyCP labelProperty = objectClass->GetInstanceLabelProperty();
    if (nullptr == labelProperty)
        {
        return "";
        }

    Utf8PrintfString key("DataSourceCache::ReadInstanceLabel:%llu", objectClass->GetId().GetValue());
    auto statement = m_state->GetStatementCache().GetPreparedStatement(key, [&]
        {
        Utf8String classKey = ECDbHelper::ECClassKeyFromClass(*objectClass);

        struct LabelSelectProvider : ISelectProvider
            {
            private:
                ECPropertyCP m_labelProperty;

            public:
                LabelSelectProvider(ECPropertyCP labelProperty) : m_labelProperty(labelProperty)
                    {};

                virtual std::shared_ptr<SelectProperties> GetSelectProperties(ECClassCR ecClass) const
                    {
                    if (!ecClass.Is(&m_labelProperty->GetClass()))
                        {
                        return nullptr;
                        }

                    auto selectProperties = std::make_shared<SelectProperties>();
                    selectProperties->AddProperty(m_labelProperty);
                    selectProperties->SetSelectAll(false);
                    selectProperties->SetSelectInstanceId(false);
                    return selectProperties;
                    }
            };

        LabelSelectProvider selectProvider(labelProperty);
        auto infos = CacheQueryHelper(selectProvider).CreateReadInfos(objectClass);

        Utf8String ecSql;
        if (!infos.empty())
            {
            ecSql = CacheQueryHelper::ECSql::SelectPropertiesByRemoteId(infos.front());
            }
        return ecSql;
        });

    statement->BindText(1, objectId.remoteId.c_str(), IECSqlBinder::MakeCopy::No);

    DbResult status = statement->Step();
    if (status != BE_SQLITE_ROW)
        {
        return "";
        }

    return statement->GetValueText(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::ReadFileProperties(ECInstanceKeyCR instanceKey, Utf8String* fileNameP, uint64_t* fileSizeP)
    {
    ECClassCP ecClass = m_state->GetECDbAdapter().GetECClass(instanceKey);
    if (nullptr == ecClass)
        {
        return ERROR;
        }

    Utf8PrintfString key("DataSourceCache::ReadFileProperties:%llu:%d:%d", ecClass->GetId().GetValue(), !!fileNameP, !!fileSizeP);
    auto statement = m_state->GetStatementCache().GetPreparedStatement(key, [&]
        {
        Utf8String fileNamePropertyName, fileSizePropertyName;
        if (nullptr != fileNameP)
            fileNamePropertyName = ECCustomAttributeHelper::GetPropertyName(ecClass, "FileDependentProperties", "FileName");

        if (nullptr != fileSizeP)
            fileSizePropertyName = ECCustomAttributeHelper::GetPropertyName(ecClass, "FileDependentProperties", "FileSize");

        if (fileNamePropertyName.empty())
            {
            ECPropertyCP labelProperty = ecClass->GetInstanceLabelProperty();
            if (nullptr != labelProperty)
                {
                fileNamePropertyName = Utf8String(labelProperty->GetName());
                }
            }

        Utf8PrintfString ecSql
            (
            "SELECT %s n, %s s FROM ONLY %s WHERE ECInstanceId = ? LIMIT 1",
            fileNamePropertyName.empty() ? "NULL" : ("[" + fileNamePropertyName + "]").c_str(),
            fileSizePropertyName.empty() ? "NULL" : ("[" + fileSizePropertyName + "]").c_str(),
            ecClass->GetECSqlName().c_str()
            );
        return ecSql;
        });

    statement->BindId(1, instanceKey.GetECInstanceId());

    DbResult status = statement->Step();
    if (BE_SQLITE_ROW != status)
        {
        return ERROR;
        }

    if (nullptr != fileNameP)
        *fileNameP = statement->GetValueText(0);

    if (nullptr != fileSizeP)
        *fileSizeP = statement->GetValueInt64(1);

    return SUCCESS;
    }
