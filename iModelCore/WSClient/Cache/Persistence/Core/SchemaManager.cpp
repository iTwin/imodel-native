/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "SchemaManager.h"

#include "../../Logging.h"

#include <WebServices/Cache/Persistence/DataSourceCacheCommon.h>
#include <WebServices/Cache/Util/ECDbHelper.h>

#include "CacheSchema.h"
#include "SchemaContext.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

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
    SchemaKey cacheSchemaKey = SchemaKey
        (
        SCHEMA_CacheSchema,
        SCHEMA_CacheSchema_Major,
        SCHEMA_CacheSchema_Minor
        );

    ECSchemaReadContextPtr context = SchemaContext::CreateReadContext();
    context->AddSchemaLocater(m_db.GetSchemaLocater());
    ECSchemaPtr cacheSchema = LoadSchema(cacheSchemaKey, *context);
    return ImportSchemas(std::vector<ECSchemaPtr> {cacheSchema});
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaManager::ImportExternalSchemas(const std::vector<BeFileName>& schemaPaths)
    {
    std::vector<ECSchemaPtr> schemas;
    if (SUCCESS != LoadSchemas(schemaPaths, schemas) ||
        SUCCESS != ImportExternalSchemas(schemas))
        {
        BeAssert(false);
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaManager::ImportExternalSchemas(const std::vector<ECSchemaPtr>& schemas)
    {
    ECSchemaReadContextPtr context = SchemaContext::CreateReadContext();
    for (auto schema : schemas)
        {
        if (schema.IsNull())
            return ERROR;

        if (SUCCESS != FixLegacySchema(*schema, *context))
            return ERROR;
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
    EC::SchemaManager::SchemaImportOptions options = EC::SchemaManager::SchemaImportOptions::None;
    for (ECSchemaPtr schema : schemas)
        {
        if (schema.IsNull())
            {
            LOG.error("One or more supplied schemas are null - check depenendcies and assets");
            return ERROR;
            }
        schemaCache->AddSchema(*schema);
        if (schema->GetFullSchemaName().Equals("Issue.01.00.01"))
            options = EC::SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues;
        }

    if (LOG.isSeverityEnabled(NativeLogging::LOG_TRACE))
        {
        LOG.tracev("Importing %d ECSchemas to ECDb", schemas.size());

        for (ECSchemaPtr schema : schemas)
            {
            Utf8String schemaXml;
            SchemaWriteStatus swstatus = schema->WriteToXmlString(schemaXml);
            BeAssert(SchemaWriteStatus::Success == swstatus);

            LOG.trace(schemaXml.c_str());
            }
        }

    m_db.NotifyOnSchemaChangedListeners();
    if (SUCCESS != m_db.Schemas().ImportSchemas(schemaCache->GetSchemas(), options, nullptr))
        {
        LOG.errorv("Failed to import one or more schemas: %s", ToFullNameListString(schemas).c_str());
        BeAssert(false);
        return ERROR;
        }

    m_db.NotifyOnSchemaChangedListeners();

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SchemaManager::ToFullNameListString(const std::vector<ECSchemaPtr>& schemas)
    {
    Utf8String listStr;
    for (auto schema : schemas)
        {
        if (!listStr.empty())
            listStr += ", ";
        listStr += schema->GetFullSchemaName();
        }
    return listStr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr SchemaManager::LoadSchema(SchemaKey key, ECSchemaReadContext& context)
    {
    ECSchemaPtr schema = context.LocateSchema(key, SchemaMatchType::Exact);
    if (!schema.IsValid())
        {
        LOG.errorv("Could not load schema: %s.%s. Check assets or dependencies",
            key.m_schemaName.c_str(),
            ECSchema::FormatSchemaVersion(key.m_versionRead, key.m_versionMinor).c_str());
        BeAssert(false);
        }
    return schema;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr SchemaManager::LoadSchema(BeFileNameCR schemaPath, ECSchemaReadContext& context)
    {
    SchemaKey key;
    SchemaKey::ParseSchemaFullName(key, Utf8String(schemaPath.GetFileNameAndExtension()).c_str());
    ECSchemaPtr schema = context.LocateSchema(key, SchemaMatchType::Exact);

    if (!schema.IsNull())
        return schema;

    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, schemaPath.GetName(), context);
    if (SchemaReadStatus::Success != status)
        return nullptr;

    return schema;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaManager::LoadSchemas
(
    const std::vector<BeFileName>& schemaPaths,
    std::vector<ECSchemaPtr>& schemasOut
)
    {
    ECSchemaReadContextPtr context = SchemaContext::CreateReadContext();
    for (BeFileNameCR schemaPath : schemaPaths)
        context->AddSchemaPath(schemaPath.GetDirectoryName());

    context->AddSchemaLocater(m_db.GetSchemaLocater());

    for (BeFileName schemaPath : schemaPaths)
        {
        ECSchemaPtr schema = LoadSchema(schemaPath, *context);
        if (schema.IsNull())
            return ERROR;

        schemasOut.push_back(schema);
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaManager::FixLegacySchema(ECSchema& schema, ECSchemaReadContextR context)
    {
    // Convert ECDbMap.01.00 to ECDbMap.02.00 attributes
    CustomECSchemaConverterPtr schemaConverter = CustomECSchemaConverter::Create();
    schemaConverter->AddSchemaReadContext(context);
    IECCustomAttributeConverterPtr classMapConverter = new ECDbClassMapConverter();
    schemaConverter->AddConverter(ECDbClassMapConverter::GetSchemaName(), ECDbClassMapConverter::GetClassName(), classMapConverter);
    if (!schemaConverter->Convert(schema))
        {
        LOG.errorv("Failed to convert schema '%s'.", schema.GetFullSchemaName().c_str());
        return ERROR;
        }

    Utf8String versionStr = SchemaKey::FormatLegacySchemaVersion(schema.GetVersionRead(), schema.GetVersionMinor());
    Utf8String supplName = schema.GetName() + "_Supplemental_ECDbMapping." + versionStr + ".ecschema.xml";

    BeFileName supplPath = SchemaContext::GetSupportMappingDir();
    supplPath.AppendToPath(BeFileName(supplName));

    if (!supplPath.DoesPathExist())
        return SUCCESS;

    ECSchemaPtr supplSchema = LoadSchema(supplPath, context);
    if (supplSchema.IsNull())
        return ERROR;

    bvector<ECSchemaP> supplSchemas;
    supplSchemas.push_back(supplSchema.get());

    SupplementedSchemaBuilder builder;
    if (SupplementedSchemaStatus::Success != builder.UpdateSchema(schema, supplSchemas))
        return ERROR;

    if (!schema.IsSupplemented())
        {
        LOG.errorv(
            "Failed to supplement schema. Check if supplemental schema '%s' is properly configured for schema '%s'.",
            supplName.c_str(),
            schema.GetFullSchemaName().c_str());
        return SUCCESS;
        }

    LOG.warningv(
        "Adjustements for server schema '%s' were applied due to compatibility issues to 06xx ECv3 ECDb by supplementing with '%s'. "
        "Some data may not be possible to cache - consider verifying required functionality.",
        schema.GetFullSchemaName().c_str(),
        supplName.c_str());

    return SUCCESS;
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaManager::RemoveReferences(ECSchema& schema, SchemaKeyCR referencedSchemaKey)
    {
    auto it = schema.GetReferencedSchemas().find(referencedSchemaKey);
    if (it == schema.GetReferencedSchemas().end())
        return SUCCESS;

    for (auto& ecClass : schema.GetClasses())
        {
        for (auto& instance : ecClass->GetCustomAttributes(false))
            {
            if (instance->GetClass().GetSchema().GetSchemaKey() != referencedSchemaKey)
                continue;
                
            if (!ecClass->RemoveCustomAttribute(instance->GetClass()))
                return ERROR;
                
            LOG.warningv(
                "Incompatible '%s' custom attribute was removed from class '%s' ",
                instance->GetClass().GetFullName(),
                ecClass->GetFullName());
            }
        }

    auto status = schema.RemoveReferencedSchema(referencedSchemaKey);
    if (ECObjectsStatus::Success != status)
        return ERROR;
        
    LOG.warningv(
        "Incompatible '%s' reference was removed from schema '%s'. ",
        referencedSchemaKey.GetFullSchemaName().c_str(),
        schema.GetFullSchemaName().c_str());

    return SUCCESS;
    }

END_BENTLEY_WEBSERVICES_NAMESPACE
