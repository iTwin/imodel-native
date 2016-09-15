/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECUnits/Units.h>
#include "SchemaImportContext.h"
#include "ECDbExpressionSymbolProvider.h"
#include <Bentley/BeTest.h>     // *** WIP_TEST_PERFORMANCE_PROJECT - this is temporary. Remove when we have cleaned up unit tests

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//******************************** ECDbSchemaManager ****************************************
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbSchemaManager::ECDbSchemaManager(ECDbCR ecdb, ECDbMap& map) :m_ecdb(ecdb), m_map(map), m_schemaReader(new ECDbSchemaReader(ecdb))
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      01/2013
//---------------------------------------------------------------------------------------
ECDbSchemaManager::~ECDbSchemaManager()
    {
    if (m_schemaReader != nullptr)
        {
        delete m_schemaReader;
        m_schemaReader = nullptr;
        }
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                   Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECSchemaCP> ECDbSchemaManager::GetECSchemas(bool loadSchemaEntities) const
    {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT Id FROM ec_Schema");
    if (stmt == nullptr)
        return bvector<ECSchemaCP>();

    std::vector<ECSchemaId> schemaIds;
    while (BE_SQLITE_ROW == stmt->Step())
        {
        schemaIds.push_back(stmt->GetValueId<ECSchemaId>(0));
        }

    stmt = nullptr; // in case the child call needs to reuse this statement

    bvector<ECSchemaCP> schemas;
    for (ECSchemaId schemaId : schemaIds)
        {
        ECSchemaCP out = GetECSchema(schemaId, loadSchemaEntities);
        if (out == nullptr)
            return bvector<ECSchemaCP>();

        schemas.push_back(out);
        }

    return schemas;
    }

/*---------------------------------------------------------------------------------**//**
* Returns true if thisSchema directly references possiblyReferencedSchema
* @bsimethod                                 Ramanujam.Raman                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DirectlyReferences(ECSchemaCP thisSchema, ECSchemaCP possiblyReferencedSchema)
    {
    ECSchemaReferenceListCR referencedSchemas = thisSchema->GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator it = referencedSchemas.begin(); it != referencedSchemas.end(); ++it)
        {
        if (it->second.get() == possiblyReferencedSchema)
            return true;
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
bool DependsOn(ECSchemaCP thisSchema, ECSchemaCP possibleDependency)
    {
    if (DirectlyReferences(thisSchema, possibleDependency))
        return true;

    SupplementalSchemaMetaDataPtr metaData;
    if (SupplementalSchemaMetaData::TryGetFromSchema(metaData, *possibleDependency)
        && metaData.IsValid()
        && metaData->IsForPrimarySchema(thisSchema->GetName(), 0, 0, SchemaMatchType::Latest))
        {
        return true; // possibleDependency supplements thisSchema. possibleDependency must be imported before thisSchema
        }

    // Maybe possibleDependency supplements one of my references?
    ECSchemaReferenceListCR referencedSchemas = thisSchema->GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator it = referencedSchemas.begin(); it != referencedSchemas.end(); ++it)
        {
        if (DependsOn(it->second.get(), possibleDependency))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void InsertSchemaInDependencyOrderedList(bvector<ECSchemaP>& schemas, ECSchemaP insertSchema)
    {
    if (std::find(schemas.begin(), schemas.end(), insertSchema) != schemas.end())
        return; // This (and its referenced ECSchemas) are already in the list

    bvector<ECSchemaP>::reverse_iterator rit;
    for (rit = schemas.rbegin(); rit < schemas.rend(); ++rit)
        {
        if (DependsOn(insertSchema, *rit))
            {
            schemas.insert(rit.base(), insertSchema); // insert right after the referenced schema in the list
            return;
            }
        }

    schemas.insert(schemas.begin(), insertSchema); // insert at the beginning
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildDependencyOrderedSchemaList(bvector<ECSchemaP>& schemas, ECSchemaP insertSchema)
    {
    InsertSchemaInDependencyOrderedList(schemas, insertSchema);
    ECSchemaReferenceListCR referencedSchemas = insertSchema->GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
        {
        ECSchemaR referencedSchema = *iter->second.get();
        InsertSchemaInDependencyOrderedList(schemas, &referencedSchema);
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                     06/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaManager::ImportECSchemas(ECSchemaCacheR cache) const
    {
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin ECDbSchemaManager::ImportECSchemas");

    if (m_ecdb.IsReadonly())
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import ECSchemas. ECDb file is read-only.");
        return ERROR;
        }

    if (cache.GetCount() == 0)
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import ECSchemas. List of ECSchemas to import is empty.");
        return ERROR;
        }

    StopWatch timer(true);

    BeMutexHolder lock(m_criticalSection);

    SchemaImportContext context;
    if (SUCCESS != context.Initialize(m_map.GetDbSchemaR(), m_ecdb))
        return ERROR;

    if (ViewGenerator::DropECClassViews(GetECDb()) != SUCCESS)
        return ERROR;

    if (ViewGenerator::DropUpdatableViews(GetECDb()) != SUCCESS)
        return ERROR;

    if (SUCCESS != BatchImportECSchemas(context, cache))
        return ERROR;

    ECSchemaCompareContext& compareContext = context.GetECSchemaCompareContext();
    if (compareContext.HasNoSchemasToImport())
        return SUCCESS;

    //See if cache need to be cleared. If compareContext.RequireECSchemaUpgrade() == true will clear the cache and reload imported schema.
    if (compareContext.ReloadECSchemaIfRequired(*this) == ERROR)
        return ERROR;

    if (MappingStatus::Error == m_map.MapSchemas(context))
        return ERROR;

    if (ViewGenerator::CreateUpdatableViews(GetECDb()) != SUCCESS)
        return ERROR;

    m_ecdb.ClearECDbCache();

    timer.Stop();
    LOG.infov("Imported ECSchemas in %.4f msecs.", timer.GetElapsedSeconds() * 1000.0);
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("End ECDbSchemaManager::ImportECSchemas");
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                   Affan.Khan        29/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaManager::BatchImportECSchemas(SchemaImportContext& context, ECSchemaCacheR schemaCache) const
    {
    bvector<ECSchemaP> schemas;
    schemaCache.GetSchemas(schemas);

    bvector<ECSchemaP> schemasToImport;
    for (ECSchemaP schema : schemas)
        {
        BeAssert(schema != nullptr);
        if (schema == nullptr) 
            continue;

        if (schema->HasId())
            {
            ECSchemaId id = ECDbSchemaPersistenceHelper::GetECSchemaId(m_ecdb, schema->GetName().c_str());
            if (!id.IsValid() || id != schema->GetId())
                {
                m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECSchema %s is owned by some other ECDb file.", schema->GetFullSchemaName().c_str());
                return ERROR;
                }
            }

        BuildDependencyOrderedSchemaList(schemasToImport, schema);
        }

    bvector<ECSchemaP> primarySchemas;
    bvector<ECSchemaP> suppSchemas;
    for (ECSchemaP schema : schemasToImport)
        {
        if (schema->IsSupplementalSchema())
            suppSchemas.push_back(schema);
        else
            primarySchemas.push_back(schema);
        }

    if (!suppSchemas.empty())
        {
        for (ECSchemaP primarySchema : primarySchemas)
            {
            if (primarySchema->IsSupplemented())
                continue;

            SupplementedSchemaBuilder builder;
            SupplementedSchemaStatus status = builder.UpdateSchema(*primarySchema, suppSchemas, false /*dont create ca copy while supplementing*/);
            if (SupplementedSchemaStatus::Success != status)
                {
                m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to supplement ECSchema %s. See log file for details.", primarySchema->GetFullSchemaName().c_str());
                return ERROR;
                }

            //All consolidated customattribute must be reference. But Supplemental Provenance in BSCA is not
            //This bug could also be fixed in SupplementSchema builder but its much safer to do it here for now.
            if (primarySchema->GetSupplementalInfo().IsValid())
                {
                auto provenance = primarySchema->GetCustomAttribute("SupplementalProvenance");
                if (provenance.IsValid())
                    {
                    auto& bsca = provenance->GetClass().GetSchema();
                    if (!ECSchema::IsSchemaReferenced(*primarySchema, bsca))
                        {
                        primarySchema->AddReferencedSchema(const_cast<ECSchemaR>(bsca));
                        }
                    }
                }
            }
        }

    // The dependency order may have *changed* due to supplementation adding new ECSchema references! Re-sort them.
    bvector<ECSchemaP> dependencyOrderedPrimarySchemas;
    for (ECSchemaP schema : primarySchemas)
        BuildDependencyOrderedSchemaList(dependencyOrderedPrimarySchemas, schema);

    primarySchemas.clear(); // Just make sure no one tries to use it anymore

    ECSchemaValidationResult validationResult;
    bool isValid = ECSchemaValidator::ValidateSchemas(validationResult, dependencyOrderedPrimarySchemas);
    if (validationResult.HasErrors())
        {
        std::vector<Utf8String> errorMessages;
        validationResult.ToString(errorMessages);

        const ECDbIssueSeverity sev = ECDbIssueSeverity::Error;
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(sev, "Failed to import ECSchemas. Details: ");

        for (Utf8StringCR errorMessage : errorMessages)
            m_ecdb.GetECDbImplR().GetIssueReporter().Report(sev, errorMessage.c_str());
        }

    if (!isValid)
        return ERROR;

    ECSchemaCompareContext& schemaPrepareContext = context.GetECSchemaCompareContext();
    if (schemaPrepareContext.Prepare(*this, dependencyOrderedPrimarySchemas) != SUCCESS)
        return ERROR;

    ECDbSchemaWriter schemaWriter(m_ecdb);
    ECDbExpressionSymbolContext symbolsContext(m_ecdb);
    for (ECSchemaCP schema : schemaPrepareContext.GetImportingSchemas())
        {
        if (SUCCESS != schemaWriter.Import(schemaPrepareContext, *schema))
            return ERROR;
        }

    return DbSchemaPersistenceManager::RepopulateClassHierarchyCacheTable(m_ecdb);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECDbSchemaManager::GetECSchema(Utf8CP schemaName, bool loadSchemaEntities) const
    {
    const ECSchemaId schemaId = ECDbSchemaPersistenceHelper::GetECSchemaId(GetECDb(), schemaName);
    if (!schemaId.IsValid())
        return nullptr;

    return GetECSchema(schemaId, loadSchemaEntities);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECDbSchemaManager::GetECSchema(ECSchemaId schemaId, bool loadSchemaEntities) const
    {
    return GetReader().GetECSchema(schemaId, loadSchemaEntities);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaManager::ContainsECSchema(Utf8CP schemaName)  const
    {
    if (Utf8String::IsNullOrEmpty(schemaName))
        {
        BeAssert(false && "schemaName argument to ContainsECSchema must not be null or empty string.");
        return false;
        }

    return ECDbSchemaPersistenceHelper::GetECSchemaId(m_ecdb, schemaName).IsValid();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECDbSchemaManager::GetECClass(Utf8CP schemaNameOrAlias, Utf8CP className, ResolveSchema resolveSchema) const
    {
    ECClassId id;
    if (!TryGetECClassId(id, schemaNameOrAlias, className, resolveSchema))
        return nullptr;

    return GetECClass(id);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECDbSchemaManager::GetECClass(ECClassId ecClassId) const
    {
    return GetReader().GetECClass(ecClassId);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaManager::TryGetECClassId(ECClassId& id, Utf8CP schemaNameOrAlias, Utf8CP className, ResolveSchema resolveSchema) const
    {
    id = GetReader().GetECClassId(schemaNameOrAlias, className, resolveSchema);
    return id.IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                       12/13
//---------------------------------------------------------------------------------------
ECDerivedClassesList const& ECDbSchemaManager::GetDerivedECClasses(ECClassCR ecClass) const
    {
    ECClassId id = GetReader().GetECClassId(ecClass);
    if (id.IsValid())
        {
        if (SUCCESS != GetReader().EnsureDerivedClassesExist(id))
            LOG.errorv("Could not load derived classes for ECClass %s.", ecClass.GetFullName());
        }
    else
        LOG.errorv("Cannot call ECDbSchemaManager::GetDerivedECClasses on ECClass %s. The ECClass does not exist in the ECDb file %s.", ecClass.GetFullName(), m_ecdb.GetDbFileName());

    return ecClass.GetDerivedClasses();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECEnumerationCP ECDbSchemaManager::GetECEnumeration(Utf8CP schemaName, Utf8CP enumName) const
    {
    return GetReader().GetECEnumeration(schemaName, enumName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2016
//+---------------+---------------+---------------+---------------+---------------+------
KindOfQuantityCP ECDbSchemaManager::GetKindOfQuantity(Utf8CP schemaName, Utf8CP koqName) const
    {
    return GetReader().GetKindOfQuantity(schemaName, koqName);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbSchemaManager::ClearCache() const { GetReader().ClearCache(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle               09/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECDbSchemaReader const& ECDbSchemaManager::GetReader() const { BeAssert(m_schemaReader != nullptr); return *m_schemaReader; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle               12/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECDbCR ECDbSchemaManager::GetECDb() const { return m_ecdb; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      01/2013
//---------------------------------------------------------------------------------------
ECSchemaPtr ECDbSchemaManager::_LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext)
    {
    SchemaKey foundKey;
    ECSchemaId foundId;
    if (!ECDbSchemaPersistenceHelper::TryGetECSchemaKeyAndId(foundKey, foundId, m_ecdb, key.GetName().c_str()))
        return nullptr;

    if (!foundKey.Matches(key, matchType))
        return nullptr;

    ECSchemaCP schema = GetReader().GetECSchema(foundId, true);
    if (schema == nullptr)
        return nullptr;

    ECSchemaP schemaP = const_cast<ECSchemaP> (schema);
    schemaContext.GetCache().AddSchema(*schemaP);
    return schemaP;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECDbSchemaManager::_LocateClass(Utf8CP schemaName, Utf8CP className)
    {
    return GetECClass(schemaName, className);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan   12/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSchemaManager::CreateECClassViewsInDb() const
    {
    return m_map.CreateECClassViewsInDb();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE


