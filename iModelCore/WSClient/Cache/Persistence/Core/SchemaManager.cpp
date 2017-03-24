/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Core/SchemaManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "SchemaManager.h"

#include "../../Logging.h"

#include <WebServices/Cache/Persistence/DataSourceCacheCommon.h>
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
    SchemaKey cacheSchemaKey = SchemaKey
        (
        SCHEMA_CacheSchema,
        SCHEMA_CacheSchema_Major,
        SCHEMA_CacheSchema_Minor
        );

    ECSchemaReadContextPtr context = SchemaContext::CreateReadContext();
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

        if (SUCCESS != FixLegacySchemaRelationshipCardinalities(*schema))
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
    for (ECSchemaPtr schema : schemas)
        {
        if (schema.IsNull())
            {
            LOG.error("One or more supplied schemas are null - check depenendcies and assets");
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
            BeAssert(SchemaWriteStatus::Success == swstatus);

            LOG.trace(schemaXml.c_str());
            }
        }

    m_db.NotifyOnSchemaChangedListeners();

    if (SUCCESS != m_db.Schemas().ImportECSchemas(*schemaCache))
        {
        LOG.errorv("Failed to import one or more schemas: %s", ToFullNameListString(schemas).c_str());
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
        LOG.errorv(L"Could not load schema: %s.%s. Check assets or dependencies",
            key.m_schemaName.c_str(),
            ECSchema::FormatSchemaVersion(key.m_versionMajor, key.m_versionMinor).c_str());
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
    Utf8String versionStr = SchemaKey::FormatLegacySchemaVersion(schema.GetVersionMajor(), schema.GetVersionMinor());
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

    LOG.warningv(
        "Adjustements for server schema '%s' were applied due to compatibility issues to 06xx ECv3 ECDb. "
        "Some data may not be possible to cache - consider verifying required functionality.",
        schema.GetFullSchemaName().c_str());

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaManager::FixLegacySchemaRelationshipCardinalities(ECSchema& schema)
    {
    // XXX: 2017-03 workaround to support WSG 2.5 schemas on DgnDb61-16Q4 for Navigator app.
    // Relationships with (1,1) cardinality ends are mapped as required and such end class instance cannot be inserted without NavigationProperty.
    // WSCache insert instances seperately, and WSG 2.x schemas have no NavigationProperties, so workaround to change cardinalities from (1,1) to (0,1) is done.

    bset<ECClassCP> modifiedClasses;

    for (ECClassCP ecClass : schema.GetClasses())
        {
        ECRelationshipClassCP ecRelClass = ecClass->GetRelationshipClassCP();
        if (!ecRelClass)
            continue;

        if (0 == RelationshipCardinality::Compare(ecRelClass->GetSource().GetCardinality(), RelationshipCardinality::OneOne()))
            {
            ecRelClass->GetSource().SetCardinality(RelationshipCardinality::ZeroOne());
            modifiedClasses.insert(ecRelClass);
            }

        if (0 == RelationshipCardinality::Compare(ecRelClass->GetTarget().GetCardinality(), RelationshipCardinality::OneOne()))
            {
            ecRelClass->GetTarget().SetCardinality(RelationshipCardinality::ZeroOne());
            modifiedClasses.insert(ecRelClass);
            }
        }

    if (!modifiedClasses.empty())
        {
        Utf8String classesStr;
        for (auto ecClass : modifiedClasses)
            {
            if (!classesStr.empty())
                classesStr += ", ";
            classesStr += ecClass->GetName();
            }

        LOG.warningv(
            "Adjustements for schema '%s' relationship classes were applided due to compatibility issues. "
            "End cardinalities (1,1) were changed to (0,1) for classes: %s",
            schema.GetFullSchemaName().c_str(),
            classesStr.c_str());
        }

    return SUCCESS;
    }