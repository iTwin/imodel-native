/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Core/SchemaManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "SchemaManager.h"

#include "../../Logging.h"

#include <WebServices/Cache/Persistence/DataSourceCacheCommon.h>
#include <WebServices/Cache/Util/ExtendedDataAdapter.h>
#include <WebServices/Cache/Util/ECDbHelper.h>

#include "CacheSchema.h"
#include "SchemaContext.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaManager::SchemaManager(ObservableECDb& db) :
m_db(db)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaManager::ImportCacheSchemas()
    {
    ECSchemaReadContextPtr schemaContext = SchemaContext::CreateReadContext();

    SchemaKey cacheSchemaKey = SchemaKey(WIDEN(SCHEMA_CacheSchema), 1, 5);
    ECSchemaPtr cacheSchema = schemaContext->LocateSchema(cacheSchemaKey, SchemaMatchType::SCHEMAMATCHTYPE_Exact);

    SchemaKey supportSchemaKey = SchemaKey(WIDEN(SCHEMA_CacheLegacySupportSchema), 1, 0);
    ECSchemaPtr supportSchema = schemaContext->LocateSchema(supportSchemaKey, SchemaMatchType::SCHEMAMATCHTYPE_Exact);

    if (SUCCESS != ImportSchemas(std::vector<ECSchemaPtr> {cacheSchema, supportSchema}))
        {
        return ERROR;
        }

    if (SUCCESS != ExtendedDataAdapter(m_db).ImportSchema())
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaManager::ImportSchemas(const std::vector<BeFileName>& schemaPaths)
    {
    std::vector<ECSchemaPtr> schemas;

    ECSchemaReadContextPtr schemaContext = SchemaContext::CreateReadContext();
    for (BeFileNameCR schemaPath : schemaPaths)
        {
        ECSchemaPtr schema;
        SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, BeFileName(schemaPath).GetName(), *schemaContext);
        if (SchemaReadStatus::SCHEMA_READ_STATUS_Success != status || schema.IsNull())
            {
            return ERROR;
            }
        schemas.push_back(schema);
        }

    return ImportSchemas(schemas);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaManager::ImportSchemas(const std::vector<ECSchemaPtr>& schemas)
    {
    if (schemas.empty())
        {
        return SUCCESS;
        }

    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    for (ECSchemaPtr schema : schemas)
        {
        if (schema.IsNull())
            {
            BeAssert(false);
            return ERROR;
            }
        schemaCache->AddSchema(*schema);
        }

    if (LOG.isSeverityEnabled(NativeLogging::LOG_TRACE))
        {
        LOG.tracev("Importing %d ECSchemas to ECDb", schemas.size());

        for (ECSchemaPtr schema : schemas)
            {
            Utf8String schemaXml;
            SchemaWriteStatus swstatus = schema->WriteToXmlString(schemaXml);
            BeAssert(SchemaWriteStatus::SCHEMA_WRITE_STATUS_Success == swstatus);

            LOG.trace(schemaXml.c_str());
            }
        }

    if (SUCCESS != m_db.GetEC().GetSchemaManager().ImportECSchemas(*schemaCache, ECDbSchemaManager::ImportOptions(true, true)))
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RemoveSchema(ECSchemaList& schemas, ECSchemaCP schema)
    {
    auto schemaIt = std::find(schemas.begin(), schemas.end(), schema);
    if (schemaIt != schemas.end())
        {
        schemas.erase(schemaIt);
        }
    }
