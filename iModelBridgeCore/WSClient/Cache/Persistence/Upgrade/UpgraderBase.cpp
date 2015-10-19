/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Upgrade/UpgraderBase.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "UpgraderBase.h"

#include "../Core/SchemaContext.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
UpgraderBase::UpgraderBase(ECDbAdapter& adapter) :
m_adapter(adapter)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderBase::UpgradeCacheSchema(int versionMajor, int versionMinor)
    {
    ECSchemaReadContextPtr schemaContext = SchemaContext::CreateReadContext();

    SchemaKey cacheSchemaKey = SchemaKey("DSCacheSchema", versionMajor, versionMinor);
    ECSchemaPtr cacheSchema = schemaContext->LocateSchema(cacheSchemaKey, SchemaMatchType::SCHEMAMATCHTYPE_Exact);
    if (cacheSchema.IsNull())
        {
        return ERROR;
        }

    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    schemaCache->AddSchema(*cacheSchema);

    return m_adapter.GetECDb().GetEC().Schemas()
        .ImportECSchemas(*schemaCache, ECDbSchemaManager::ImportOptions(true, true));
    }
