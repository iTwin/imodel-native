/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Upgrade/UpgraderFromV5ToCurrent.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "UpgraderFromV5ToCurrent.h"

#include <WebServices/Client/WSInfo.h>

#include "../Core/SchemaContext.h"
#include "../RepositoryInfoStore.h"
#include "Upgrader.h"

#define PROPERTY_RemoteId                                   "DataSourceCache_RemoteId"

#define CachedResultsName_Navigation                        "CachingDataSource.Navigation"
#define CachedResultsName_Schemas                           "CachingDataSource.Schemas"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
UpgraderFromV5ToCurrent::UpgraderFromV5ToCurrent(ECDbAdapter& adapter, CacheEnvironmentCR environment) :
UpgraderBase(adapter),
m_oldCachePath(adapter.GetECDb().GetDbFileName()),
m_newCachePath(Upgrader::GetNewCachePathForUpgrade(m_oldCachePath, m_environment)),
m_environment(environment),
m_statementCache(adapter.GetECDb())
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV5ToCurrent::Upgrade()
    {
    // CREATE NEW CACHE
    DataSourceCache newCache;
    if (m_newCachePath.DoesPathExist())
        {
        // Continue upgrading with same database
        if (SUCCESS != newCache.Open(m_newCachePath, m_environment))
            {
            return ERROR;
            }
        }
    else
        {
        if (SUCCESS != newCache.Create(m_newCachePath, m_environment))
            {
            return ERROR;
            }
        }

    // COPY SCHEMA
    if (SUCCESS != CopySchema(newCache))
        {
        return ERROR;
        }

    // COPY SCHEMA
    if (SUCCESS != SetDefaultServerInfo(newCache))
        {
        return ERROR;
        }

    // COPY DATA
    if (SUCCESS != CopyData(newCache))
        {
        return ERROR;
        }

    if (DbResult::BE_SQLITE_OK != newCache.GetAdapter().GetECDb().SaveChanges() ||
        SUCCESS != newCache.Close())
        {
        return ERROR;
        }

    m_statementCache.Clear();
    m_adapter.GetECDb().CloseDb();

    if (SUCCESS != Upgrader::SetUpgradeFinishedFlag(m_newCachePath))
        {
        return ERROR;
        }

    if (SUCCESS != Upgrader::FinalizeUpgradeIfNeeded(m_oldCachePath, m_environment))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV5ToCurrent::CopySchema(DataSourceCache& newCache)
    {
    Utf8String schemaName;
    if (SUCCESS != ReadSchemaInfo(schemaName))
        {
        return ERROR;
        }

    ECSchemaCP schema = m_adapter.GetECSchema(schemaName);
    if (nullptr == schema)
        {
        return ERROR;
        }

    WString xmlBuffer;
    if (SCHEMA_WRITE_STATUS_Success != schema->WriteToXmlString(xmlBuffer))
        {
        return ERROR;
        }

    auto readContext = SchemaContext::CreateReadContext();
    ECSchemaPtr schemaCopy;
    if (SCHEMA_READ_STATUS_Success != ECSchema::ReadFromXmlString(schemaCopy, xmlBuffer.c_str(), *readContext))
        {
        return ERROR;
        }

    for (ECClassP ecClass : schemaCopy->GetClasses())
        {
        if (nullptr != ecClass->GetPropertyP(PROPERTY_RemoteId))
            {
            if (ECObjectsStatus::ECOBJECTS_STATUS_Success != ecClass->RemoveProperty(WIDEN(PROPERTY_RemoteId)))
                {
                return ERROR;
                }
            }
        if (ecClass->GetCustomAttribute(L"SyncIDSpecification").IsValid())
            {
            if (!ecClass->RemoveCustomAttribute(L"SyncIDSpecification"))
                {
                return ERROR;
                }
            }
        }

    if (SUCCESS != newCache.UpdateSchemas({schemaCopy}))
        {
        return ERROR;
        }

    if (SUCCESS != newCache.UpdateSchemas({SchemaContext::GetCacheSchemasDir().AppendToPath(L"MetaSchema.02.00.ecschema.xml")}))
        {
        return ERROR;
        }

    // Reopen ECDb to reload schema caches
    newCache.Close();
    if (SUCCESS != newCache.Open(m_newCachePath, m_environment))
        {
        return ERROR;
        }

    Utf8String schemaNamespacePrefix(schemaCopy->GetNamespacePrefix());

    RawWSObjectsReader::RawInstance instance;
    instance.json = std::make_shared<rapidjson::Document>();
    instance.objectId = ObjectId("MetaSchema", "ECSchemaDef", "LEGACY_SCHEMA");
    instance.json->SetObject();
    instance.json->AddMember("Name", schemaName.c_str(), instance.json->GetAllocator());
    instance.json->AddMember("NameSpacePrefix", schemaNamespacePrefix.c_str(), instance.json->GetAllocator());
    instance.json->AddMember("VersionMajor", schemaCopy->GetVersionMajor(), instance.json->GetAllocator());
    instance.json->AddMember("VersionMinor", schemaCopy->GetVersionMinor(), instance.json->GetAllocator());

    bvector<RawWSObjectsReader::RawInstance> instances;
    instances.push_back(instance);

    auto response = RawWSObjectsReader::CreateWSObjectsResponse(instances);
    CachedResponseKey responseKey(newCache.FindOrCreateRoot(""), CachedResultsName_Schemas);

    if (SUCCESS != newCache.CacheResponse(responseKey, response))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV5ToCurrent::CopyData(DataSourceCache& newCache)
    {
    Json::Value roots;
    if (SUCCESS != ReadRoots(roots))
        {
        return ERROR;
        }

    for (JsonValueCR root : roots)
        {
        Utf8String rootName = root["Name"].asString();
        ECInstanceKey oldRootKey = m_adapter.GetInstanceKeyFromJsonInstance(root);
        CacheRootPersistence persistence = (CacheRootPersistence) root["Persistance"].asInt();

        if (SUCCESS != newCache.SetupRoot(rootName, persistence))
            {
            return ERROR;
            }

        auto statement = m_statementCache.GetPreparedStatement("RootRelationship", [&]
            {
            return "SELECT TargetECClassId, TargetECInstanceId FROM ONLY [DSCJS].[RootRelationship] WHERE SourceECInstanceId = ?";
            });

        statement->BindId(1, oldRootKey.GetECInstanceId());

        bvector<ECInstanceKey> rootChildrenKeys;
        while (ECSqlStepStatus::HasRow == statement->Step())
            {
            rootChildrenKeys.push_back({statement->GetValueInt64(0), statement->GetValueId<ECInstanceId>(1)});
            }

        for (ECInstanceKeyCR childKey : rootChildrenKeys)
            {
            RawWSObjectsReader::RawInstance instance;
            Json::Value instanceJson, instanceInfo, fileInfo;
            if (SUCCESS != ReadInstanceData(childKey, instance, instanceJson, instanceInfo))
                {
                return ERROR;
                }

            if (SUCCESS != ReadFileInfo(childKey, fileInfo))
                {
                return ERROR;
                }

            auto newChildKey = CopyInstanceToRoot(newCache, instance, instanceJson, instanceInfo, rootName);
            if (!newChildKey.IsValid())
                {
                return ERROR;
                }

            if (SUCCESS != CopyFile(newCache, newChildKey, fileInfo))
                {
                return ERROR;
                }

            if (SUCCESS != CopyInstanceHierarchy(newCache, newChildKey, instance.objectId, childKey, instanceInfo))
                {
                return ERROR;
                }
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV5ToCurrent::SetDefaultServerInfo(DataSourceCache& newCache)
    {
    WSInfo info(BeVersion(1, 0), BeVersion(1, 1), WSInfo::Type::BentleyWSG);

    RepositoryInfoStore store(nullptr, nullptr, nullptr);
    if (SUCCESS != store.CacheServerInfo(newCache, info) ||
        SUCCESS != store.SetCacheInitialized(newCache))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV5ToCurrent::CopyInstanceHierarchy
(
DataSourceCache& newCache,
ECInstanceKeyCR newParentKey,
ObjectIdCR oldParentObjectId,
ECInstanceKeyCR oldParentKey,
JsonValueCR oldParentInfo
)
    {
    ECRelationshipClassCP relationshipClass = nullptr;
    if (oldParentObjectId.IsEmpty())
        {
        relationshipClass = m_adapter.GetECRelationshipClass("DSCacheJoinSchema", "NavigationBaseRelationship");
        }
    else
        {
        ECClassCP parentClass = m_adapter.GetECClass(oldParentKey);
        Utf8String relClassName = "HierarchyRelationship" + Utf8String(parentClass->GetName());
        relationshipClass = m_adapter.GetECRelationshipClass("DSCacheNavigationRelationshipSchema", relClassName);
        }

    if (nullptr == relationshipClass)
        {
        // Nothing to do here
        return SUCCESS;
        }

    Utf8PrintfString statementKey("CopyInstanceHierarchy:%lld", relationshipClass->GetId());
    auto statement = m_statementCache.GetPreparedStatement(statementKey, [&]
        {
        return
            "SELECT TargetECClassId, TargetECInstanceId FROM ONLY " + ECSqlBuilder::ToECSqlSnippet(*relationshipClass) + " "
            "WHERE SourceECClassId = ? AND SourceECInstanceId = ? ";
        });

    statement->BindInt64(1, oldParentKey.GetECClassId());
    statement->BindId(2, oldParentKey.GetECInstanceId());

    bvector<ECInstanceKey> childrenKeys;
    ECSqlStepStatus status;
    while (ECSqlStepStatus::HasRow == (status = statement->Step()))
        {
        childrenKeys.push_back({statement->GetValueInt64(0), statement->GetValueId<ECInstanceId>(1)});
        }

    bvector<RawWSObjectsReader::RawInstance> navigationResponseInstances;
    bvector<std::tuple<ECInstanceKey, ObjectId, Json::Value, ECInstanceKey>> childrenData;

    for (ECInstanceKeyCR childKey : childrenKeys)
        {
        RawWSObjectsReader::RawInstance instance;
        Json::Value instanceJson, instanceInfo;
        if (SUCCESS != ReadInstanceData(childKey, instance, instanceJson, instanceInfo))
            {
            return ERROR;
            }

        ECInstanceKey newChildKey;
        if (SUCCESS != CopyCreatedInstanceToParent(newCache, instanceJson, instanceInfo, newParentKey, newChildKey))
            {
            return ERROR;
            }

        if (!newChildKey.IsValid())
            {
            navigationResponseInstances.push_back(instance);
            }

        childrenData.push_back(std::make_tuple(childKey, instance.objectId, instanceInfo, newChildKey));
        }

    JsonValueCR childrenCacheDate = oldParentInfo["ChildrenInfo"]["CacheDate"];
    JsonValueCR childrenCacheTag = oldParentInfo["ChildrenInfo"]["CacheTag"];

    if (!childrenCacheDate.isNull() || !navigationResponseInstances.empty())
        {
        CachedResponseKey responseKey(newCache.FindInstance(oldParentObjectId), CachedResultsName_Navigation);
        auto response = RawWSObjectsReader::CreateWSObjectsResponse(navigationResponseInstances, childrenCacheTag.asString());

        if (SUCCESS != newCache.CacheResponse(responseKey, response))
            {
            return ERROR;
            }
        }

    for (const auto& childData : childrenData)
        {
        ECInstanceKeyCR oldChildKey = std::get<0>(childData);
        ObjectIdCR oldChildObjectId = std::get<1>(childData);
        JsonValueCR oldChildInfo = std::get<2>(childData);
        ECInstanceKey newChildKey = std::get<3>(childData);

        if (!newChildKey.IsValid())
            {
            newChildKey = newCache.FindInstance(oldChildObjectId);
            }

        Json::Value fileInfo;
        if (SUCCESS != ReadFileInfo(oldChildKey, fileInfo))
            {
            return ERROR;
            }

        if (SUCCESS != CopyFile(newCache, newChildKey, fileInfo))
            {
            return ERROR;
            }

        if (SUCCESS != CopyInstanceHierarchy(newCache, newChildKey, oldChildObjectId, oldChildKey, oldChildInfo))
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey UpgraderFromV5ToCurrent::CopyInstanceToRoot
(
DataSourceCache& newCache,
const RawWSObjectsReader::RawInstance& instance,
JsonValueCR instanceJson,
JsonValueCR instanceInfo,
Utf8StringCR rootName
)
    {
    if (instance.objectId.IsEmpty())
        {
        if (SUCCESS != newCache.LinkInstanceToRoot(rootName, ObjectId()))
            {
            return ECInstanceKey();
            }
        return newCache.FindInstance(ObjectId());
        }

    auto changeStatus = ConvertChangeStatus(instanceInfo["ChangeInfo"]["ChangeStatus"]);

    if (ChangeManager::ChangeStatus::NoChange == changeStatus)
        {
        bvector<RawWSObjectsReader::RawInstance> instances;
        instances.push_back(instance);

        auto response = RawWSObjectsReader::CreateWSObjectsResponse(instances);
        if (SUCCESS != newCache.CacheInstancesAndLinkToRoot(response, rootName))
            {
            return ECInstanceKey();
            }

        return newCache.FindInstance(instance.objectId);
        }
    else if (ChangeManager::ChangeStatus::Created == changeStatus)
        {
        auto syncStatus = ConvertSyncStatus(instanceInfo["ChangeInfo"]["SyncStatus"]);
        ECClassCP ecClass = m_adapter.GetECClass(instance.objectId);
        if (nullptr == ecClass)
            {
            return ECInstanceKey();
            }

        return newCache.GetChangeManager().CreateObject(*ecClass, instanceJson, syncStatus);
        }

    BeAssert(false && "Other changes are not supported for upgrade");
    if (SUCCESS != newCache.LinkInstanceToRoot(rootName, instance.objectId))
        {
        return ECInstanceKey();
        }

    return newCache.FindInstance(instance.objectId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV5ToCurrent::CopyCreatedInstanceToParent
(
DataSourceCache& newCache,
JsonValueCR instanceJson,
JsonValueCR instanceInfo,
ECInstanceKeyCR newParentKey,
ECInstanceKey& newInstanceKey
)
    {
    auto changeStatus = ConvertChangeStatus(instanceInfo["ChangeInfo"]["ChangeStatus"]);

    if (ChangeManager::ChangeStatus::NoChange == changeStatus)
        {
        return SUCCESS;
        }

    if (ChangeManager::ChangeStatus::Created == changeStatus)
        {
        auto syncStatus = ConvertSyncStatus(instanceInfo["ChangeInfo"]["SyncStatus"]);
        ECClassCP ecClass = m_adapter.GetECClass(m_adapter.GetInstanceKeyFromJsonInstance(instanceJson));
        if (nullptr == ecClass)
            {
            return ERROR;
            }

        newInstanceKey = newCache.GetChangeManager().LegacyCreateObject(*ecClass, instanceJson, newParentKey, syncStatus);
        if (!newInstanceKey.IsValid())
            {
            return ERROR;
            }
        }
    else
        {
        BeAssert(false && "Other changes are not supported for upgrade");
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV5ToCurrent::CopyFile
(
DataSourceCache& newCache,
ECInstanceKeyCR newInstanceKey,
JsonValueCR fileInfo
)
    {
    if (fileInfo.isNull())
        {
        return SUCCESS;
        }

    Utf8String eTag = fileInfo["CacheInfo"]["CacheTag"].asString();
    bool isPersistent = fileInfo["IsPersistant"].asBool();
    BeFileName localPath(fileInfo["LocalPath"].asString());
    FileCache cacheLocation;

    BeFileName absolutePath;
    if (isPersistent)
        {
        cacheLocation = FileCache::Persistent;
        absolutePath = m_environment.persistentFileCacheDir;
        }
    else
        {
        cacheLocation = FileCache::Temporary;
        absolutePath = m_environment.temporaryFileCacheDir;
        }

    absolutePath.AppendToPath(BeFileName(m_oldCachePath.GetFileNameAndExtension() + L"f"));
    absolutePath.AppendToPath(localPath);

    if (!absolutePath.DoesPathExist())
        {
        // Nothing to do here
        return SUCCESS;
        }

    auto changeStatus = ConvertChangeStatus(fileInfo["ChangeInfo"]["ChangeStatus"]);

    if (ChangeManager::ChangeStatus::NoChange == changeStatus)
        {
        WSFileResponse response(absolutePath, HttpStatus::OK, eTag);
        if (SUCCESS != newCache.CacheFile(newCache.FindInstance(newInstanceKey), response, cacheLocation))
            {
            return ERROR;
            }
        }
    else if (ChangeManager::ChangeStatus::Created == changeStatus ||
             ChangeManager::ChangeStatus::Modified == changeStatus)
        {
        auto syncStatus = ConvertSyncStatus(fileInfo["ChangeInfo"]["SyncStatus"]);

        if (SUCCESS != newCache.GetChangeManager().ModifyFile(newInstanceKey, absolutePath, false, syncStatus))
            {
            return ERROR;
            }
        }
    else
        {
        BeAssert(false && "Other changes are not supported for upgrade");
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV5ToCurrent::ReadInstanceData
(
ECInstanceKeyCR key,
RawWSObjectsReader::RawInstance& instance,
JsonValueR instanceJson,
JsonValueR instanceInfo
)
    {
    ECClassCP instanceClass = m_adapter.GetECClass(key);
    if (nullptr == instanceClass)
        {
        return ERROR;
        }

    // Instance and InstanceInfo
    Utf8PrintfString statementKey("ReadInstanceData:%lld", key.GetECClassId());
    auto statement = m_statementCache.GetPreparedStatement(statementKey, [&]
        {
        return
            "SELECT * FROM ONLY " + ECSqlBuilder::ToECSqlSnippet(*instanceClass) + " instance "
            "JOIN [DSC].[CachedInstanceInfo] instanceInfo USING [DSCJS].[CachedInstanceInfoRelationship] "
            "WHERE instance.ECInstanceId = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, key.GetECInstanceId());

    ECSqlStepStatus status = statement->Step();
    if (ECSqlStepStatus::HasRow != status)
        {
        return ERROR;
        }

    JsonECSqlSelectAdapter jsonAdapter(*statement, ECValueFormat::RawNativeValues);
    if (!jsonAdapter.GetRowInstance(instanceJson, key.GetECClassId()))
        {
        return ERROR;
        }

    if (!jsonAdapter.GetRowInstance(instanceInfo, m_adapter.GetECClass("DSCacheSchema", "CachedInstanceInfo")->GetId()))
        {
        return ERROR;
        }

    if (instanceClass->GetSchema().GetName().Equals(L"DSCacheSchema") &&
        instanceClass->GetName().Equals(L"NavigationBase"))
        {
        instance.objectId = ObjectId();
        }
    else
        {
        instance.objectId = ObjectId(*instanceClass, instanceJson[PROPERTY_RemoteId].asString());
        }

    instanceJson.removeMember(PROPERTY_RemoteId);
    instance.eTag = instanceInfo["InstanceInfo"]["CacheTag"].asString();

    instance.json = std::make_shared<rapidjson::Document>();
    if (instance.json->Parse<0>(Json::FastWriter::ToString(instanceJson).c_str()).HasParseError())
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV5ToCurrent::ReadFileInfo
(
ECInstanceKeyCR key,
JsonValueR fileInfo
)
    {
    auto statement = m_statementCache.GetPreparedStatement("ReadFileInfo", [&]
        {
        return
            "SELECT * FROM ONLY [DSC].[CachedFileInfo] fileInfo "
            "JOIN [DSCJS].[CachedFileInfoRelationship] rel ON rel.SourceECInstanceId = fileInfo.ECInstanceId "
            "WHERE rel.TargetECInstanceId = ? "
            "LIMIT 1 ";
        });

    statement->BindId(1, key.GetECInstanceId());

    ECSqlStepStatus status = statement->Step();
    if (ECSqlStepStatus::Done == status)
        {
        return SUCCESS;
        }

    JsonECSqlSelectAdapter fileInfoJsonAdapter(*statement, ECValueFormat::RawNativeValues);
    if (!fileInfoJsonAdapter.GetRowInstance(fileInfo, m_adapter.GetECClass("DSCacheSchema", "CachedFileInfo")->GetId()))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV5ToCurrent::ReadSchemaInfo(Utf8String& schemaName)
    {
    ECSqlStatement statement;

    m_adapter.PrepareStatement(statement, "SELECT [DataSourceSchemaName] FROM ONLY [DSC].[Settings] LIMIT 1 ");
    if (ECSqlStepStatus::HasRow != statement.Step())
        {
        return ERROR;
        }

    schemaName = statement.GetValueText(0);
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV5ToCurrent::ReadRoots(JsonValueR roots)
    {
    ECSqlStatement statement;
    JsonECSqlSelectAdapter jsonAdapter(statement, ECValueFormat::RawNativeValues);

    m_adapter.PrepareStatement(statement, "SELECT * FROM ONLY [DSC].[Root]");
    while (ECSqlStepStatus::HasRow == statement.Step())
        {
        if (!jsonAdapter.GetRowInstance(roots.append(Json::objectValue)))
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IChangeManager::ChangeStatus UpgraderFromV5ToCurrent::ConvertChangeStatus(JsonValueCR oldStatusJson)
    {
    if (oldStatusJson.isNull())
        {
        return IChangeManager::ChangeStatus::NoChange;
        }

    switch (oldStatusJson.asInt())
        {
            case 0:
                return IChangeManager::ChangeStatus::NoChange;
            case 1:
                return IChangeManager::ChangeStatus::Created;
            case 2:
                return IChangeManager::ChangeStatus::Modified;
            case 4:
                return IChangeManager::ChangeStatus::Deleted;
            default:
                break;
        };

    BeAssert(false);
    return IChangeManager::ChangeStatus::NoChange;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IChangeManager::SyncStatus UpgraderFromV5ToCurrent::ConvertSyncStatus(JsonValueCR oldStatusJson)
    {
    if (oldStatusJson.isNull())
        {
        return IChangeManager::SyncStatus::NotReady;
        }

    switch (oldStatusJson.asInt())
        {
            case 0:
                return IChangeManager::SyncStatus::Ready;
            case 1:
                return IChangeManager::SyncStatus::NotReady;
            default:
                break;
        };

    BeAssert(false);
    return IChangeManager::SyncStatus::NotReady;
    }
