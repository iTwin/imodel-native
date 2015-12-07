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

    return m_adapter.GetECDb().Schemas()
        .ImportECSchemas(*schemaCache, ECDbSchemaManager::ImportOptions(true, true));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UpgraderBase::ExecuteStatement(Utf8CP ecSql)
    {
    ECSqlStatement statement;
    if (SUCCESS != m_adapter.PrepareStatement(statement, ecSql))
        {
        return ERROR;
        }

    DbResult status;
    while (BE_SQLITE_ROW == (status = statement.Step()));

    if (BE_SQLITE_OK != status)
        {
        return ERROR;
        }

    return SUCCESS;
    }
