/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/DataSourceCache.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
#include "Core/DataSourceCacheOpenState.h"
#include "Core/SchemaContext.h"
#include "Core/SchemaManager.h"
#include "Core/Version.h"
#include "Upgrade/Upgrader.h"

#include "../Logging.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ECDbDebugInfoHolder> CreateLoggerHolder(DataSourceCacheOpenState& state, Utf8CP context)
    {
    if (!LOG.isSeverityEnabled(Bentley::NativeLogging::LOG_TRACE))
        {
        return nullptr;
        }

    ECSchemaList schemas;
    state.GetECDbAdapter().GetECDb().GetEC().GetSchemaManager().GetECSchemas(schemas);
    schemas.push_back(state.GetCacheSchema());

    return std::make_shared<ECDbDebugInfoHolder>(state.GetECDbAdapter().GetECDb(), schemas, "DataSourceCache debug information", context);
    }

// Add this macro to start of a method to print out cache state before and after method is done
//#define LogCacheDataForMethod() auto _ecdb_debug_info_holder_ = CreateLoggerHolder (*m_state, __FUNCTION__)
// DEPRECATED: not very useful, disable for now
#define LogCacheDataForMethod()

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
* @bsimethod                                                    Vincas.Razma    06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceCacheOpenState& DataSourceCache::GetState()
    {
    if (nullptr == m_state)
        throw std::runtime_error("DataSourceCache is not open!");
    return *m_state;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::Create
(
BeFileNameCR cacheFilePath,
CacheEnvironmentCR baseEnvironment,
const ECDb::CreateParams& params
)
    {
    BeFileName::CreateNewDirectory(cacheFilePath.GetDirectoryName());

    DbResult status = m_db.CreateNewDb(cacheFilePath, BeDbGuid(), params);
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

        SetupOpenState(baseEnvironment);
        return SUCCESS;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::Open
(
BeFileNameCR cacheFilePath,
CacheEnvironmentCR baseEnvironment,
const ECDb::OpenParams& params
)
    {
    if (SUCCESS != Upgrader::FinalizeUpgradeIfNeeded(cacheFilePath, baseEnvironment))
        {
        return ERROR;
        }

    DbResult status = m_db.OpenBeSQLiteDb(cacheFilePath, params);
    if (BE_SQLITE_OK != status)
        {
        return ERROR;
        }

    if (SUCCESS != UpgradeIfNeeded(cacheFilePath, baseEnvironment, params))
        {
        return ERROR;
        }

    return ExecuteWithinTransaction([&]
        {
        SetupOpenState(baseEnvironment);
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
        // Clear all cached statements and data for DB to close
        m_db.NotifyOnSchemaChangedListeners();
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
CacheEnvironmentCR baseEnvironment,
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
    if (BE_SQLITE_OK != m_db.OpenBeSQLiteDb(cacheFilePath, ECDb::OpenParams(ECDb::OPEN_ReadWrite)))
        {
        return ERROR;
        }

    Upgrader upgrader(m_db, baseEnvironment);
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
void DataSourceCache::SetupOpenState(CacheEnvironmentCR baseEnvironment)
    {
    BeFileName cachePath(m_db.GetDbFileName());
    CacheEnvironment actualEnvironment = FileStorage::CreateCacheEnvironment(cachePath, baseEnvironment);
    m_state = std::make_shared<DataSourceCacheOpenState>(m_db, actualEnvironment);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void DataSourceCache::ClearRuntimeCaches()
    {
    if (m_state)
        {
        GetState().ClearRuntimeCaches();
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
    m_db.NotifyOnSchemaChangedListeners();

    if (SUCCESS != SchemaManager(m_db).ImportSchemas(schemaPaths))
        {
        return ERROR;
        }

    m_db.NotifyOnSchemaChangedListeners();
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::UpdateSchemas(const std::vector<ECSchemaPtr>& schemas)
    {
    m_db.NotifyOnSchemaChangedListeners();

    if (SUCCESS != SchemaManager(m_db).ImportSchemas(schemas))
        {
        return ERROR;
        }

    m_db.NotifyOnSchemaChangedListeners();
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::DeleteCacheFromDisk(BeFileNameCR cacheFilePath, CacheEnvironmentCR baseEnvironment)
    {
    if (SUCCESS != FileStorage::DeleteFileCacheDirectories(FileStorage::CreateCacheEnvironment(cacheFilePath, baseEnvironment)))
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

    if (SUCCESS != FileStorage::DeleteFileCacheDirectories(GetState().GetFileCacheEnvironment()))
        {
        return ERROR;
        }

    // Remove all cached objects
    // TODO: remove this when CachedObjectInfoRelationship will be holding cached objects
    if (SUCCESS != GetState().GetObjectInfoManager().RemoveAllCachedInstances())
        {
        return ERROR;
        }

    // Remove all cached data
    for (ECClassCP ecClass : GetState().GetCacheSchema()->GetClasses())
        {
        if (!ecClass->GetIsDomainClass() ||
            ecClass->GetRelationshipClassCP() != nullptr)
            {
            continue;
            }

        Utf8PrintfString key("DataSourceCache::Reset:%lld", ecClass->GetId());
        auto statement = GetState().GetStatementCache().GetPreparedStatement(key, [&]
            {
            return Utf8PrintfString("SELECT %lld, ECInstanceId FROM ONLY %s", ecClass->GetId(),
                                    ECSqlBuilder::ToECSqlSnippet(*ecClass).c_str());
            });

        if (SUCCESS != GetState().GetHierarchyManager().DeleteInstances(*statement))
            {
            return ERROR;
            }
        }

    m_state = std::make_shared<DataSourceCacheOpenState>(m_db, GetState().GetFileCacheEnvironment());
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
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheEnvironmentCR DataSourceCache::GetEnvironment()
    {
    return GetState().GetFileCacheEnvironment();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IECDbAdapterR DataSourceCache::GetAdapter()
    {
    return GetState().GetECDbAdapter();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IExtendedDataAdapter& DataSourceCache::GetExtendedDataAdapter()
    {
    return GetState().GetExtendedDataAdapter();
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
    return GetState().GetChangeManager();
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
    return GetState().GetObjectInfoManager().ReadInfo(objectId).GetObjectCacheTag();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DataSourceCache::ReadFileCacheTag(ObjectIdCR objectId)
    {
    return GetState().GetFileInfoManager().ReadInfo(objectId).GetFileCacheTag();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime DataSourceCache::ReadInstanceCachedDate(ObjectIdCR objectId)
    {
    return GetState().GetObjectInfoManager().ReadInfo(objectId).GetObjectCacheDate();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::SetResponseAccessDate(CachedResponseKeyCR responseKey, DateTimeCR utcDateTime)
    {
    CachedResponseInfo info = GetState().GetCachedResponseManager().ReadInfo(responseKey);
    if (!info.IsCached())
        {
        return ERROR;
        }
    info.SetAccessDate(utcDateTime);
    return GetState().GetCachedResponseManager().SaveInfo(info);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime DataSourceCache::ReadResponseAccessDate(CachedResponseKeyCR responseKey)
    {
    return GetState().GetCachedResponseManager().ReadInfo(responseKey).GetAccessDate();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bset<CachedResponseKey> DataSourceCache::GetResponsesContainingInstance(ECInstanceKeyCR instance, Utf8StringCR responseName)
    {
    bset<CachedResponseKey> keys;
    if (SUCCESS != GetState().GetCachedResponseManager().GetResponsesContainingInstance(instance, keys, responseName))
        BeAssert(false);

    return keys;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime DataSourceCache::ReadFileCachedDate(ObjectIdCR objectId)
    {
    return GetState().GetFileInfoManager().ReadInfo(objectId).GetFileCacheDate();
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
    return CachedObjectInfo(std::make_shared<ObjectInfo>(GetState().GetObjectInfoManager().ReadInfo(objectId)));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedObjectInfo DataSourceCache::GetCachedObjectInfo(ECInstanceKeyCR instance)
    {
    return CachedObjectInfo(std::make_shared<ObjectInfo>(GetState().GetObjectInfoManager().ReadInfo(instance)));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey DataSourceCache::FindInstance(ObjectIdCR objectId)
    {
    if (objectId.IsEmpty())
        {
        return GetState().GetNavigationBaseManager().FindNavigationBase();
        }

    return GetState().GetObjectInfoManager().FindCachedInstance(objectId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectId DataSourceCache::FindInstance(ECInstanceKeyCR instanceKey)
    {
    return GetState().GetObjectInfoManager().FindCachedInstance(instanceKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey DataSourceCache::FindRelationship(ECRelationshipClassCR relClass, ECInstanceKeyCR source, ECInstanceKeyCR target)
    {
    return GetState().GetECDbAdapter().FindRelationship(&relClass, source, target);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey DataSourceCache::FindRelationship(ECRelationshipClassCR relClass, ObjectIdCR sourceId, ObjectIdCR targetId)
    {
    ECInstanceKey source = GetState().GetObjectInfoManager().FindCachedInstance(sourceId);
    ECInstanceKey target = GetState().GetObjectInfoManager().FindCachedInstance(targetId);
    return GetState().GetECDbAdapter().FindRelationship(&relClass, source, target);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectId DataSourceCache::FindRelationship(ECInstanceKeyCR relationshipKey)
    {
    return GetState().GetRelationshipInfoManager().ReadObjectId(relationshipKey);
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
    ECInstanceKey cachedInstance;

    if (response.IsModified())
        {
        WSObjectsReader::Instances instances = response.GetInstances();

        InstanceCacheHelper::CachedInstances cachedInstances;
        if (SUCCESS != GetState().GetInstanceHelper().CacheInstances(instances, cachedInstances))
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
        cachedInstance = it->second;
        }
    else
        {
        ObjectInfo info = GetState().GetObjectInfoManager().ReadInfo(objectId);
        info.SetObjectCacheDate(DateTime::GetCurrentTimeUtc());
        if (SUCCESS != GetState().GetObjectInfoManager().UpdateInfo(info))
            {
            return ERROR;
            }
        cachedInstance = info.GetCachedInstanceKey();
        }

    if (SUCCESS != GetState().GetRootManager().LinkExistingInstanceToRoot(rootName, cachedInstance))
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
    ObjectInfo info = GetState().GetObjectInfoManager().ReadInfo(objectId);

    info.SetObjectCacheTag(cacheTag);
    info.SetObjectCacheDate(DateTime::GetCurrentTimeUtc());
    info.SetObjectState(CachedInstanceState::Full);

    if (SUCCESS != GetState().GetRootManager().LinkNewInstanceToRoot(rootName, objectId, info, &instancePropertiesJson))
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
    if (SUCCESS != GetState().GetInstanceHelper().CacheInstances(instances, cachedInstances, nullptr, nullptr, ct))
        {
        return ERROR;
        }

    if (SUCCESS != GetState().GetRootManager().LinkExistingInstancesToRoot(rootName, cachedInstances.GetCachedInstances(), !weakLinkToRoot))
        {
        return ERROR;
        }

    if (nullptr != cachedInstanceKeysOut)
        {
        for (ECInstanceKeyCR key : cachedInstances.GetCachedInstances())
            {
            cachedInstanceKeysOut->Insert(key.GetECClassId(), key.GetECInstanceId());
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CacheStatus DataSourceCache::UpdateInstance(ObjectIdCR objectId, WSObjectsResponseCR response)
    {
    LogCacheDataForMethod();

    if (response.IsModified())
        {
        WSObjectsReader::Instances instances = response.GetInstances();

        InstanceCacheHelper::CachedInstances cachedInstances;
        InstanceCacheHelper::UpdateCachingState updateCachingState;
        if (SUCCESS != GetState().GetInstanceHelper().CacheInstances(instances, cachedInstances, nullptr, &updateCachingState))
            return CacheStatus::Error;

        if (1 != cachedInstances.GetCachedInstances().size() || !updateCachingState.GetNotFoundObjectIds().empty())
            return CacheStatus::DataNotCached;
        }
    else
        {
        ObjectInfo info = GetState().GetObjectInfoManager().ReadInfo(objectId);
        if (!info.IsInCache())
            return CacheStatus::DataNotCached;

        info.SetObjectCacheDate(DateTime::GetCurrentTimeUtc());
        if (SUCCESS != GetState().GetObjectInfoManager().UpdateInfo(info))
            return CacheStatus::Error;
        }

    return CacheStatus::OK;
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
    if (SUCCESS != GetState().GetInstanceHelper().CacheInstances(instances, cachedInstances, nullptr, &updateCachingState, ct))
        {
        return ERROR;
        }

    if (nullptr != notFoundOut)
        {
        notFoundOut->insert(updateCachingState.GetNotFoundObjectIds().begin(), updateCachingState.GetNotFoundObjectIds().end());
        }

    if (nullptr != cachedInstancesOut)
        {
        cachedInstancesOut->insert(cachedInstances.GetCachedInstances().begin(), cachedInstances.GetCachedInstances().end());
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

    ECClassCP ecClass = GetState().GetECDbAdapter().GetECClass(objectId);
    if (nullptr == ecClass)
        {
        return CacheStatus::Error;
        }

    auto statement = GetReadInstanceStatement(*ecClass, objectId.remoteId.c_str());

    ECSqlStepStatus status = statement->Step();
    if (ECSqlStepStatus::Error == status)
        {
        return CacheStatus::Error;
        }
    if (ECSqlStepStatus::HasRow != status)
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

    ECClassCP ecClass = GetState().GetECDbAdapter().GetECClass(objectId);
    if (nullptr == ecClass)
        {
        return nullptr;
        }

    auto statement = GetReadInstanceStatement(*ecClass, objectId.remoteId.c_str());
    if (ECSqlStepStatus::HasRow != statement->Step())
        {
        return nullptr;
        }

    IECInstancePtr instance = ECInstanceECSqlSelectAdapter(*statement).GetInstance();
    if (instance.IsNull())
        {
        return nullptr;
        }

    instance->SetInstanceId(WString(objectId.remoteId.c_str(), true).c_str());
    return instance;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ECSqlStatement> DataSourceCache::GetReadInstanceStatement(ECClassCR ecClass, Utf8CP remoteId)
    {
    Utf8PrintfString key("DataSourceCache::GetReadInstanceStatement:RemoteId:%lld", ecClass.GetId());
    auto statement = GetState().GetStatementCache().GetPreparedStatement(key, [&]
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
    Utf8PrintfString key("DataSourceCache::GetReadInstanceStatement:ECInstanceId:%lld", ecClass.GetId());
    auto statement = GetState().GetStatementCache().GetPreparedStatement(key, [&]
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
    ECClassCP ecClass = GetState().GetECDbAdapter().GetECClass(instanceKey);
    if (nullptr == ecClass)
        {
        return nullptr;
        }

    auto statement = GetReadInstanceStatement(*ecClass, instanceKey.GetECInstanceId());
    if (ECSqlStepStatus::HasRow != statement->Step())
        {
        return nullptr;
        }

    IECInstancePtr instance = ECInstanceECSqlSelectAdapter(*statement).GetInstance(ecClass->GetId());
    if (instance.IsNull())
        {
        return nullptr;
        }

    instance->SetInstanceId(WString(statement->GetValueText(0), true).c_str());
    return instance;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Benjamin.Brown  08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CacheStatus DataSourceCache::RemoveInstance(ObjectIdCR objectId)
    {
    LogCacheDataForMethod();
    ECInstanceKey instance;
    if (objectId.IsEmpty())
        {
        instance = GetState().GetNavigationBaseManager().FindNavigationBase();
        }
    else
        {
        instance = GetState().GetObjectInfoManager().FindCachedInstance(objectId);
        }

    if (!instance.IsValid())
        {
        return CacheStatus::DataNotCached;
        }

    if (SUCCESS != GetState().GetCachedResponseManager().InvalidateResponsePagesContainingInstance(instance))
        {
        return CacheStatus::Error;
        }

    if (SUCCESS != GetState().GetHierarchyManager().DeleteInstance(instance))
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
    FileInfo info = GetState().GetFileInfoManager().ReadInfo(objectId);

    if (!info.IsInCache())
        return SUCCESS;

    if (CacheStatus::OK != GetState().GetFileStorage().RemoveStoredFile(info))
        return ERROR;

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
    bmap<ECClassId, bvector<Utf8CP>> ecClassToRemoteIdsMap;

    for (ObjectIdCR objectId : ids)
        {
        if (!objectId.IsValid())
            {
            return ERROR;
            }
        ECClassCP ecClass = GetState().GetECDbAdapter().GetECClass(objectId);
        if (nullptr == ecClass)
            return ERROR;
        ecClassToRemoteIdsMap[ecClass->GetId()].push_back(objectId.remoteId.c_str());
        }

    bvector<ECClassP> classesToSelect;
    for (auto iter : ecClassToRemoteIdsMap)
        {
        classesToSelect.push_back((ECClassP) GetState().GetECDbAdapter().GetECClass(iter.first));
        }

    CacheQueryHelper helper(selectProvider);
    bvector<CacheQueryHelper::ClassReadInfo> readInfos = helper.CreateReadInfos(classesToSelect);

    for (CacheQueryHelper::ClassReadInfo& info : readInfos)
        {
        Utf8String remoteIdList = CreateCommaSeperatedStringList(ecClassToRemoteIdsMap[info.GetECClass().GetId()]);
        Utf8String ecSql = CacheQueryHelper::ECSql::SelectPropertiesByRemoteIds(info, remoteIdList);

        ECSqlStatement statement;
        if (SUCCESS != GetState().GetECDbAdapter().PrepareStatement(statement, ecSql))
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
    if (SUCCESS != GetState().GetRootManager().SetupRoot(rootName, persistence))
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
    return IsInstanceFullyPersisted(GetState().GetObjectInfoManager().FindCachedInstance(objectId));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool DataSourceCache::IsInstanceFullyPersisted(ECInstanceKeyCR instanceKey)
    {
    if (!instanceKey.IsValid())
        {
        return false;
        }

    ECInstanceKeyMultiMap persistedInstances;
    if (SUCCESS != GetState().GetRootManager().GetInstancesByPersistence(CacheRootPersistence::Full, persistedInstances))
        {
        return false;
        }

    return ECDbHelper::IsInstanceInMultiMap(instanceKey, persistedInstances);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::ReadFullyPersistedInstanceKeys(ECInstanceKeyMultiMap& instancesOut)
    {
    return GetState().GetRootManager().GetInstancesByPersistence(CacheRootPersistence::Full, instancesOut);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::LinkInstanceToRoot(Utf8StringCR rootName, ObjectIdCR objectId)
    {
    LogCacheDataForMethod();
    if (objectId.IsEmpty())
        {
        auto baseKey = GetState().GetNavigationBaseManager().FindOrCreateNavigationBase();
        if (SUCCESS != GetState().GetRootManager().LinkExistingInstanceToRoot(rootName, baseKey))
            {
            return ERROR;
            }
        }
    else
        {
        if (SUCCESS != GetState().GetRootManager().LinkInstanceToRoot(rootName, objectId))
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool DataSourceCache::IsInstanceInRoot(Utf8StringCR rootName, ECInstanceKeyCR instance)
    {
    ECInstanceKey root = GetState().GetRootManager().FindRoot(rootName);
    return GetState().GetRootManager().IsInstanceInRoot(instance, root.GetECInstanceId());
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

    ECInstanceKey root = GetState().GetRootManager().FindRoot(rootName);
    if (!root.IsValid())
        {
        return false;
        }

    bset<ECInstanceId> rootIds;
    rootIds.insert(root.GetECInstanceId());

    return GetState().GetRootManager().IsInstanceConnectedToAnyOfRoots(instance, rootIds);
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

    ECInstanceKey root = GetState().GetRootManager().FindRoot(rootName);
    if (!root.IsValid())
        {
        return SUCCESS;
        }

    ECInstanceKeyMultiMap instanceIds;
    if (SUCCESS != GetState().GetHierarchyManager().ReadTargetKeys(root, GetState().GetRootManager().GetRootRelationshipClass(), instanceIds))
        {
        return ERROR;
        }

    CacheQueryHelper helper(selectProvider);
    if (SUCCESS != helper.ReadInstances(GetState().GetECDbAdapter(), instanceIds,
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
ECInstanceKeyMultiMap& instanceMap
)
    {
    ECInstanceKey root = GetState().GetRootManager().FindRoot(rootName);
    if (!root.IsValid())
        {
        return SUCCESS;
        }

    if (SUCCESS != GetState().GetHierarchyManager().ReadTargetKeys(root, GetState().GetRootManager().GetRootRelationshipClass(), instanceMap))
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
    ECInstanceKey root = GetState().GetRootManager().FindRoot(rootName);
    if (!root.IsValid())
        {
        return SUCCESS;
        }

    bset<ECInstanceId> roots;
    roots.insert(root.GetECInstanceId());

    ECInstanceKeyMultiMap instances;
    if (SUCCESS != GetState().GetRootManager().GetInstancesConnectedToRoots(roots, instances, depth))
        {
        return ERROR;
        }

    for (auto pair : instances)
        {
        if (GetState().GetCacheSchema() != &GetState().GetECDbAdapter().GetECClass(pair.first)->GetSchema())
            {
            instancesOut.insert(pair);
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::UnlinkInstanceFromRoot(Utf8StringCR rootName, ObjectIdCR objectId)
    {
    LogCacheDataForMethod();
    ECInstanceKey instance;
    if (objectId.IsEmpty())
        {
        instance = GetState().GetNavigationBaseManager().FindNavigationBase();
        }
    else
        {
        instance = GetState().GetObjectInfoManager().FindCachedInstance(objectId);
        }
    if (SUCCESS != GetState().GetRootManager().UnlinkInstanceFromRoot(rootName, instance))
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
    if (SUCCESS != GetState().GetRootManager().UnlinkAllInstancesFromRoot(rootName))
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
    if (SUCCESS != GetState().GetRootManager().RemoveRoot(rootName))
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
    if (SUCCESS != GetState().GetRootManager().RemoveRootsByPrefix(rootPrefix))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::SetFileCacheLocation(ObjectIdCR objectId, FileCache location, BeFileNameCR externalRelativeDir)
    {
    //! TODO: consider removing FileCache parameter and auotmaically use Root persistence instead
    LogCacheDataForMethod();
    FileInfo info = GetState().GetFileInfoManager().ReadInfo(objectId);
    if (SUCCESS != GetState().GetFileStorage().SetFileCacheLocation(info, location, &externalRelativeDir) ||
        SUCCESS != GetState().GetFileInfoManager().SaveInfo(info))
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
    FileInfo info = GetState().GetFileInfoManager().ReadInfo(objectId);
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

    FileInfo info = GetState().GetFileInfoManager().ReadInfo(objectId);
    if (!info.GetInstanceKey().IsValid())
        return ERROR;

    if (!fileResult.IsModified())
        {
        if (!info.IsInCache())
            return ERROR;
        }
    else
        {
        auto path = fileResult.GetFilePath();
        auto eTag = fileResult.GetETag();
        if (SUCCESS != GetState().GetFileStorage().CacheFile(info, path, eTag.c_str(), cacheLocation, false))
            return ERROR;
        }

    info.SetFileCacheDate(DateTime::GetCurrentTimeUtc());

    if (SUCCESS != GetState().GetFileInfoManager().SaveInfo(info))
        return ERROR;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DataSourceCache::ReadFilePath(ObjectIdCR objectId)
    {
    return ReadFilePath(GetState().GetObjectInfoManager().FindCachedInstance(objectId));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DataSourceCache::ReadFilePath(ECInstanceKeyCR instanceKey)
    {
    return GetState().GetFileInfoManager().ReadFilePath(instanceKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheStatus DataSourceCache::RemoveFilesInTemporaryPersistence(DateTimeCP maxLastAccessDate, AsyncError* errorOut)
    {
    LogCacheDataForMethod();

    ECInstanceKeyMultiMap fullyPersistedInstances;
    if (SUCCESS != GetState().GetRootManager().GetInstancesByPersistence(CacheRootPersistence::Full, fullyPersistedInstances))
        return CacheStatus::Error;

    auto status = GetState().GetFileInfoManager().DeleteFilesNotHeldByInstances(fullyPersistedInstances, maxLastAccessDate, errorOut);
    if (CacheStatus::OK != status)
        return status;

    return CacheStatus::OK;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey DataSourceCache::FindOrCreateRoot(Utf8StringCR rootName)
    {
    return GetState().GetRootManager().FindOrCreateRoot(rootName);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool DataSourceCache::DoesRootExist(Utf8StringCR rootName)
    {
    return GetState().GetRootManager().FindRoot(rootName).IsValid();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::RenameRoot(Utf8StringCR rootName, Utf8StringCR newRootName)
    {
    return GetState().GetRootManager().RenameRoot(rootName, newRootName);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::SetRootSyncDate(Utf8StringCR rootName, DateTimeCR utcDateTime)
    {
    return GetState().GetRootManager().SetRootSyncDate(rootName, utcDateTime);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime DataSourceCache::ReadRootSyncDate(Utf8StringCR rootName)
    {
    return GetState().GetRootManager().ReadRootSyncDate(rootName);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheStatus DataSourceCache::CacheResponse
(
CachedResponseKeyCR responseKey,
WSObjectsResponseCR response,
bset<ObjectId>* rejectedOut,
const WSQuery* query,
uint64_t page,
ICancellationTokenPtr ct
)
    {
    LogCacheDataForMethod();
    if (!responseKey.IsValid())
        return CacheStatus::Error;

    if (!response.IsModified())
        {
        auto status = GetState().GetCachedResponseManager().UpdatePageCachedDate(responseKey, page);
        if (CacheStatus::OK != status)
            return status;
        }
    else
        {
        ECInstanceKeyMultiMap fullyPersistedInstances;
        std::shared_ptr<InstanceCacheHelper::PartialCachingState> partialCachingState;

        if (nullptr != query)
            {
            if (nullptr == rejectedOut)
                return CacheStatus::Error;

            InstanceCacheHelper::QueryAnalyzer analyzer(GetState().GetECDbAdapter(), *query);
            partialCachingState = std::make_shared<InstanceCacheHelper::PartialCachingState>
                (
                analyzer,
                GetState().GetRootManager(),
                *rejectedOut
                );
            }

        WSObjectsReader::Instances instances = response.GetInstances();
        InstanceCacheHelper::CachedInstances cachedInstances;

        if (SUCCESS != GetState().GetInstanceHelper().CacheInstances(instances, cachedInstances, partialCachingState.get(), nullptr, ct) ||
            SUCCESS != GetState().GetCachedResponseManager().SavePage(responseKey, page, response.GetETag(), cachedInstances))
            {
            return CacheStatus::Error;
            }

        if (nullptr != partialCachingState &&
            SUCCESS != GetState().GetCachedResponseManager().InvalidateFullResponsePagesContainingInstances(partialCachingState->GetOverriddenFullInstances()))
            {
            return CacheStatus::Error;
            }
        }

    if (response.IsFinal() && SUCCESS != GetState().GetCachedResponseManager().TrimPages(responseKey, page))
        return CacheStatus::Error;

    bool wasCompleted = GetState().GetCachedResponseManager().IsResponseCompleted(responseKey);
    bool nowCompleted = wasCompleted;

    if (response.IsFinal())
        nowCompleted = true;
    else if (response.IsModified())
        nowCompleted = false;

    if (wasCompleted != nowCompleted)
        {
        auto status = GetState().GetCachedResponseManager().SetResponseCompleted(responseKey, nowCompleted);
        if (CacheStatus::OK != status)
            return status;
        }

    return CacheStatus::OK;
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

    ECInstanceKeyMultiMap fullyPersistedInstances;
    if (SUCCESS != GetState().GetRootManager().GetInstancesByPersistence(CacheRootPersistence::Full, fullyPersistedInstances))
        {
        return ERROR;
        }

    bset<ECInstanceId> temporaryInfos;
    for (CachedResponseKeyCR responseKey : responseKeys)
        {
        ECInstanceKey info = GetState().GetCachedResponseManager().FindInfo(responseKey);
        if (!info.IsValid())
            {
            continue;
            }
        if (!ECDbHelper::IsInstanceInMultiMap(info, fullyPersistedInstances))
            {
            temporaryInfos.insert(info.GetECInstanceId());
            }
        }

    if (SUCCESS != GetState().GetCachedResponseManager().MarkTemporaryInstancesAsPartial(temporaryInfos, fullyPersistedInstances))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::RemoveResponse(CachedResponseKeyCR responseKey)
    {
    LogCacheDataForMethod();
    return GetState().GetCachedResponseManager().DeleteInfo(responseKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::RemoveTemporaryResponses(Utf8StringCR name, DateTimeCR accessedBeforeDateUtc)
    {
    LogCacheDataForMethod();

    ECInstanceKeyMultiMap fullyPersistedInstances;
    if (SUCCESS != GetState().GetRootManager().GetInstancesByPersistence(CacheRootPersistence::Full, fullyPersistedInstances))
        {
        return ERROR;
        }

    return GetState().GetCachedResponseManager().DeleteResponses(name, accessedBeforeDateUtc, fullyPersistedInstances);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DataSourceCache::RemoveResponses(Utf8StringCR name)
    {
    LogCacheDataForMethod();
    return GetState().GetCachedResponseManager().DeleteResponses(name);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheStatus DataSourceCache::ReadResponse
(
CachedResponseKeyCR responseKey,
JsonValueR instancesOut,
ISelectProviderCR selectProvider
)
    {
    if (instancesOut.isNull() || !instancesOut.isArray())
        {
        instancesOut = Json::arrayValue;
        }

    CachedResponseInfo queryInfo = GetState().GetCachedResponseManager().ReadInfo(responseKey);
    if (!queryInfo.IsCached())
        {
        return CacheStatus::DataNotCached;
        }

    if (SUCCESS != GetState().GetCachedResponseManager().ReadResponse(queryInfo, selectProvider,
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
CacheStatus DataSourceCache::ReadResponseInstanceKeys(CachedResponseKeyCR responseKey, ECInstanceKeyMultiMap& instanceKeysOut)
    {
    CachedResponseInfo queryInfo = GetState().GetCachedResponseManager().ReadInfo(responseKey);
    if (!queryInfo.IsCached())
        {
        return CacheStatus::DataNotCached;
        }

    if (SUCCESS != GetState().GetCachedResponseManager().ReadResponseInstanceKeys(queryInfo.GetKey(), instanceKeysOut))
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
CachedResponseKeyCR responseKey,
bset<ObjectId>& instanceObjectIdsOut
)
    {
    ECInstanceKey infoKey = GetState().GetCachedResponseManager().FindInfo(responseKey);
    if (!infoKey.IsValid())
        {
        return CacheStatus::DataNotCached;
        }

    if (SUCCESS != GetState().GetCachedResponseManager().ReadResponseObjectIds(infoKey, instanceObjectIdsOut))
        {
        return CacheStatus::Error;
        }

    return CacheStatus::OK;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DataSourceCache::ReadResponseCacheTag(CachedResponseKeyCR responseKey, uint64_t page)
    {
    return GetState().GetCachedResponseManager().ReadResponsePageCacheTag(responseKey, page);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime DataSourceCache::ReadResponseCachedDate(CachedResponseKeyCR responseKey, uint64_t page)
    {
    return GetState().GetCachedResponseManager().ReadResponsePageCachedDate(responseKey, page);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool DataSourceCache::IsResponseCached(CachedResponseKeyCR responseKey)
    {
    return GetState().GetCachedResponseManager().IsResponseCompleted(responseKey);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DataSourceCache::ReadInstanceLabel(ObjectIdCR objectId)
    {
    ECClassCP objectClass = GetState().GetECDbAdapter().GetECClass(objectId);
    if (nullptr == objectClass)
        {
        return "";
        }

    ECPropertyCP labelProperty = objectClass->GetInstanceLabelProperty();
    if (nullptr == labelProperty)
        {
        return "";
        }

    Utf8PrintfString key("DataSourceCache::ReadInstanceLabel:%lld", objectClass->GetId());
    auto statement = GetState().GetStatementCache().GetPreparedStatement(key, [&]
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

    ECSqlStepStatus status = statement->Step();
    if (status != ECSqlStepStatus::HasRow)
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
    ECClassCP ecClass = GetState().GetECDbAdapter().GetECClass(instanceKey);
    if (nullptr == ecClass)
        {
        return ERROR;
        }

    Utf8PrintfString key("DataSourceCache::ReadFileProperties:%lld:%d:%d", ecClass->GetId(), !!fileNameP, !!fileSizeP);
    auto statement = GetState().GetStatementCache().GetPreparedStatement(key, [&]
        {
        Utf8String fileNamePropertyName, fileSizePropertyName;
        if (nullptr != fileNameP)
            fileNamePropertyName = ECCustomAttributeHelper::GetPropertyName(ecClass, L"FileDependentProperties", L"FileName");

        if (nullptr != fileSizeP)
            fileSizePropertyName = ECCustomAttributeHelper::GetPropertyName(ecClass, L"FileDependentProperties", L"FileSize");

        Utf8PrintfString ecSql
            (
            "SELECT %s n, %s s FROM ONLY %s WHERE ECInstanceId = ? LIMIT 1",
            fileNamePropertyName.empty() ? "NULL" : ("[" + fileNamePropertyName + "]").c_str(),
            fileSizePropertyName.empty() ? "NULL" : ("[" + fileSizePropertyName + "]").c_str(),
            ECSqlBuilder::ToECSqlSnippet(*ecClass).c_str()
            );
        return ecSql;
        });

    statement->BindId(1, instanceKey.GetECInstanceId());

    ECSqlStepStatus status = statement->Step();
    if (ECSqlStepStatus::HasRow != status)
        {
        return ERROR;
        }

    if (nullptr != fileNameP)
        *fileNameP = statement->GetValueText(0);

    if (nullptr != fileSizeP)
        *fileSizeP = statement->GetValueInt64(1);

    return SUCCESS;
    }
