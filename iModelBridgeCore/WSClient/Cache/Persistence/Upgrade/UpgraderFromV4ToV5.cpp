/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Upgrade/UpgraderFromV4ToV5.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "UpgraderFromV4ToV5.h"

#include <WebServices/Cache/Util/ECDbHelper.h>

#include "../Core/CacheSettings.h"
#include "../Core/SchemaManager.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct UpgraderFromV4ToV5::UpgradeInstance
    {
    public:
        ECClassId ecClassId;
        Json::Value json;

        UpgradeInstance(ECClassId ecClassId) : ecClassId(ecClassId)
            {};
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
UpgraderFromV4ToV5::UpgraderFromV4ToV5(ECDbAdapter& adapter) :
UpgraderBase(adapter)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV4ToV5::Upgrade()
    {
    // Get upgraded instances to write them later
    bvector<UpgradeInstance> changedInstances;
    if (SUCCESS != GetInstancesWithAppliedChanges(changedInstances))
        {
        return ERROR;
        }

    // Schema update
    if (SUCCESS != UpgradeCacheSchema(1, 3))
        {
        return ERROR;
        }

    // Update upgraded instances
    if (SUCCESS != UpdateInstances(changedInstances))
        {
        return ERROR;
        }

    // Add WeakRootRelationship class
    Json::Value settingsJson;
    ECClassCP settingsClass = m_adapter.GetECClass("DSCacheSchema", "Settings");
    if (SUCCESS != m_adapter.GetJsonInstance(settingsJson, settingsClass))
        {
        return ERROR;
        }

    ECSchemaCP dataSourceSchema = m_adapter.GetECSchema(settingsJson["DataSourceSchemaName"].asString());
    ECSchemaCP cacheSchema = m_adapter.GetECSchema("DSCacheSchema");
    ECSchemaCP originalJoinSchema = m_adapter.GetECSchema("DSCacheJoinSchema");

    if (nullptr == dataSourceSchema || nullptr == cacheSchema || nullptr == originalJoinSchema)
        {
        return ERROR;
        }

    ECSchemaPtr joinSchema = ECDbHelper::CopySchema(*originalJoinSchema, &m_adapter.GetECDb().GetEC().GetSchemaLocater());
    if (joinSchema.IsNull())
        {
        return ERROR;
        }

    auto nodeClasses = GetDataSourceNodeClasses(*dataSourceSchema);
    CreateWeakRootRelationship(*joinSchema, *cacheSchema, nodeClasses);

    // Import
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    schemaCache->AddSchema(*joinSchema);

    return m_adapter.GetECDb().GetEC().GetSchemaManager().ImportECSchemas(*schemaCache, ECDbSchemaManager::ImportOptions(true, true));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECClassCP> UpgraderFromV4ToV5::GetDataSourceNodeClasses(ECSchemaCR ecSchema)
    {
    bvector<ECClassCP> extractedClasses;
    for (ECClassCP ecClass : ecSchema.GetClasses())
        {
        if (!IsDataSourceObjectClass(ecClass))
            {
            continue;
            }
        extractedClasses.push_back(ecClass);
        }
    return extractedClasses;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool UpgraderFromV4ToV5::IsDataSourceObjectClass(ECClassCP ecClass)
    {
    return
        !ecClass->GetIsCustomAttributeClass() &&
        !ecClass->GetIsStruct() &&
        ecClass->GetIsDomainClass() &&
        nullptr == ecClass->GetRelationshipClassCP();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassP CreateRelationshipHoldingClasses
(
ECSchemaR schema,
WStringCR relationshipName,
ECClassCP parentClass,
const bvector<ECClassCP>& childClasses,
StrengthType strength = StrengthType::STRENGTHTYPE_Holding
)
    {
    ECRelationshipClassP relationshipClass = nullptr;
    schema.CreateRelationshipClass(relationshipClass, relationshipName);

    relationshipClass->SetStrength(strength);
    relationshipClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationshipClass->SetIsDomainClass(true);

    relationshipClass->GetSource().SetCardinality(RelationshipCardinality::ZeroMany());
    relationshipClass->GetTarget().SetCardinality(RelationshipCardinality::ZeroMany());

    relationshipClass->GetSource().AddClass(*parentClass);
    for (ECClassCP childClass : childClasses)
        {
        relationshipClass->GetTarget().AddClass(*childClass);
        }
    return relationshipClass;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void UpgraderFromV4ToV5::CreateWeakRootRelationship(ECSchemaR schema, ECSchemaCR cacheSchema, const bvector<ECClassCP>& childClasses)
    {
    ECClassCP parentClass = cacheSchema.GetClassCP(L"Root");
    ECRelationshipClassP relClass = CreateRelationshipHoldingClasses(schema, L"WeakRootRelationship", parentClass, childClasses, StrengthType::STRENGTHTYPE_Referencing);
    relClass->GetTarget().AddClass(*cacheSchema.GetClassCP(L"NavigationBase"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV4ToV5::GetInstancesWithAppliedChanges(bvector<UpgradeInstance>& changedInstancesOut)
    {
    ECClassCP rootClass = m_adapter.GetECClass("DSCacheSchema", "Root");
    ECClassCP cachedFileInfoClass = m_adapter.GetECClass("DSCacheSchema", "CachedFileInfo");
    ECClassCP cachedInstanceInfoClass = m_adapter.GetECClass("DSCacheSchema", "CachedInstanceInfo");

    if (SUCCESS != ReadInstances(changedInstancesOut, rootClass))
        {
        return ERROR;
        }
    if (SUCCESS != ReadInstances(changedInstancesOut, cachedFileInfoClass))
        {
        return ERROR;
        }
    if (SUCCESS != ReadInstances(changedInstancesOut, cachedInstanceInfoClass))
        {
        return ERROR;
        }

    for (UpgradeInstance& instance : changedInstancesOut)
        {
        if (rootClass->GetId() == instance.ecClassId)
            {
            ApplyChangesToRootInstance(instance.json);
            }
        else if (cachedFileInfoClass->GetId() == instance.ecClassId)
            {
            ApplyChangesToCachedFileInfoInstance(instance.json);
            }
        else if (cachedInstanceInfoClass->GetId() == instance.ecClassId)
            {
            ApplyChangesToCachedInstanceInfoInstance(instance.json);
            }
        else
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void UpgraderFromV4ToV5::ApplyChangesToRootInstance(JsonValueR root)
    {
    root["Persistance"] = 0; // Default to "Full" persistence
    root.removeMember("PersistanceDepth");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void UpgraderFromV4ToV5::ApplyChangesToCachedFileInfoInstance(JsonValueR info)
    {
    ApplyChangesToChangeInfoStruct(info["ChangeInfo"]);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void UpgraderFromV4ToV5::ApplyChangesToCachedInstanceInfoInstance(JsonValueR info)
    {
    ApplyChangesToChangeInfoStruct(info["ChangeInfo"]);

    int instanceState = 0;

    if (((JsonValueCR) info)["InstanceInfo"]["CacheDate"].empty())
        {
        instanceState = 0; // "Placeholder"
        }
    else
        {
        instanceState = 2; // "Full"
        }

    info["InstanceState"] = instanceState;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void UpgraderFromV4ToV5::ApplyChangesToChangeInfoStruct(JsonValueR changeInfoStruct)
    {
    if (!((JsonValueCR) changeInfoStruct)["Status"].isNull())
        {
        changeInfoStruct["SyncStatus"] = 0;
        changeInfoStruct["ChangeStatus"] = changeInfoStruct["Status"];
        changeInfoStruct.removeMember("Status");
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV4ToV5::ReadInstances(bvector<UpgradeInstance>& instancesOut, ECClassCP ecClass)
    {
    if (nullptr == ecClass)
        {
        return ERROR;
        }

    ECSqlSelectBuilder builder;
    builder.SelectAll().From(*ecClass, false);

    ECSqlStatement statement;
    if (SUCCESS != m_adapter.PrepareStatement(statement, builder))
        {
        return ERROR;
        }

    JsonECSqlSelectAdapter adapter(statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    ECSqlStepStatus status;
    while (ECSqlStepStatus::HasRow == (status = statement.Step()))
        {
        instancesOut.push_back(UpgradeInstance(ecClass->GetId()));
        if (!adapter.GetRowInstance(instancesOut.back().json, ecClass->GetId()))
            {
            return ERROR;
            }
        }

    if (ECSqlStepStatus::Done != status)
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderFromV4ToV5::UpdateInstances(const bvector<UpgradeInstance>& instances)
    {
    for (const UpgradeInstance& instance : instances)
        {
        JsonUpdater updater(m_adapter.GetECDb(), *m_adapter.GetECClass(instance.ecClassId));
        if (SUCCESS != updater.Update(instance.json))
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

