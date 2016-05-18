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

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//******************************** ECDbSchemaManager ****************************************
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbSchemaManager::ECDbSchemaManager (ECDbR ecdb, ECDbMapR map) :m_ecdb (ecdb), m_map (map)
    {
    m_ecReader = ECDbSchemaReader::Create(ecdb);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      01/2013
//---------------------------------------------------------------------------------------
ECDbSchemaManager::~ECDbSchemaManager () {}

/*---------------------------------------------------------------------------------------
* @bsimethod                                                   Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaManager::GetECSchemas (ECSchemaList& schemas, bool ensureAllClassesLoaded) const
    {
    ECSchemaKeys schemaKeys;
    auto stat = GetECSchemaKeys(schemaKeys);
    if (stat != SUCCESS)
        return stat;

    schemas.clear();
    for (ECSchemaKey const& schemaKey : schemaKeys)
        {
        ECSchemaCP out = GetECSchema (schemaKey.GetECSchemaId (), ensureAllClassesLoaded);
        if (out == nullptr)
            return ERROR;

        schemas.push_back(out);
        }

    return SUCCESS;
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
        && metaData->IsForPrimarySchema(thisSchema->GetName(), 0, 0, SCHEMAMATCHTYPE_Latest))
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
BentleyStatus ECDbSchemaManager::ImportECSchemas 
(
ECSchemaCacheR cache, 
ImportOptions const& options
) const
    {
    if (m_ecdb.IsReadonly ())
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import ECSchemas. ECDb file is read-only.");
        return ERROR;
        }

    if (cache.GetCount () == 0)
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to import ECSchemas. List of ECSchemas to import is empty.");
        return ERROR;
        }

    StopWatch timer(true);

    BeMutexHolder lock (m_criticalSection);
    
    SchemaImportContext context;
    if (SUCCESS != context.Initialize(m_map.GetSQLManager(), m_ecdb))
        return ERROR;
    if (SUCCESS != BatchImportECSchemas (context, cache, options))
        return ERROR;
  
    ECSchemaCompareContext& compareContext = context.GetECSchemaCompareContext();
    if (compareContext.HasNoSchemasToImport())
        return SUCCESS;

    //See if cache need to be cleared. If compareContext.RequireECSchemaUpgrade() == true will clear the cache and reload imported schema.
    if (compareContext.ReloadECSchemaIfRequired(*this) == ERROR)
        return ERROR;

    if (MappingStatus::Error == m_map.MapSchemas (context))
        return ERROR;

    {
    RelationshipPurger purger;
    if (SUCCESS != purger.Purge(m_ecdb))
        return ERROR;
    }

    m_ecdb.ClearECDbCache();

    timer.Stop();
    LOG.infov("Imported ECSchemas in %.4f msecs.",  timer.GetElapsedSeconds() * 1000.0);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                   Affan.Khan        29/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaManager::BatchImportECSchemas(SchemaImportContext const& context, ECSchemaCacheR schemaCache, ImportOptions const& options) const
    {
    bvector<ECSchemaP> schemas;
    schemaCache.GetSchemas(schemas);

    bvector<ECSchemaP> schemasToImport;
    for (ECSchemaP schema : schemas)
        {
        BeAssert(schema != nullptr);
        if (schema == nullptr) continue;

        ECSchemaId id = ECDbSchemaPersistenceHelper::GetECSchemaId(m_ecdb, schema->GetName().c_str());
        if (schema->HasId() && (id == 0 || id != schema->GetId()))
            {
            m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECSchema %s is owned by some other ECDb file.", schema->GetFullSchemaName().c_str());
            return ERROR;
            }

        BuildDependencyOrderedSchemaList(schemasToImport, schema);
        }

    //1. Only import supplemental schema if doSupplement = True AND saveSupplementals = TRUE
    bvector<ECSchemaP> primarySchemas;
    bvector<ECSchemaP> suppSchemas;
    for (ECSchemaP schema : schemasToImport)
        {
        if (schema->IsSupplementalSchema())
            suppSchemas.push_back(schema);
        else
            primarySchemas.push_back(schema);
        }

    if (!suppSchemas.empty() && options.DoSupplementation())
        {
        for (ECSchemaP primarySchema : primarySchemas)
            {
            if (primarySchema->IsSupplemented() || primarySchema->IsStandardSchema() || primarySchema->IsSystemSchema())
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

    ECSchemaCompareContext& schemaPrepareContext = const_cast<SchemaImportContext&>(context).GetECSchemaCompareContext();
    if (schemaPrepareContext.Prepare(*this, dependencyOrderedPrimarySchemas) != SUCCESS)
        return ERROR;
    ECDbSchemaWriter schemaWriter(m_ecdb);
    for (ECSchemaCP schema : schemaPrepareContext.GetImportingSchemas())
        {
        if (SUCCESS != schemaWriter.Import(schemaPrepareContext, *schema))
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECDbSchemaManager::GetECSchema (Utf8CP schemaName, bool ensureAllClassesLoaded) const
    {
    const ECSchemaId schemaId = ECDbSchemaPersistenceHelper::GetECSchemaId(GetECDb(), schemaName); //WIP_FNV: could be more efficient if it first looked through those already cached in memory...
    if (INT64_C(0) == schemaId)
        return nullptr;

    return GetECSchema(schemaId, ensureAllClassesLoaded);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECDbSchemaManager::GetECSchema (ECSchemaId schemaId, bool ensureAllClassesLoaded) const
    {
    return m_ecReader->GetECSchema(schemaId, ensureAllClassesLoaded);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaManager::ContainsECSchema (Utf8CP schemaName)  const
    {
    if (Utf8String::IsNullOrEmpty(schemaName))
        {
        BeAssert(false && "schemaName argument to ContainsECSchema must not be null or empty string.");
        return false;
        }

    return ECDbSchemaPersistenceHelper::GetECSchemaId (m_ecdb, schemaName) > 0ULL;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECDbSchemaManager::GetECClass (Utf8CP schemaNameOrPrefix, Utf8CP className, ResolveSchema resolveSchema) const // WIP_FNV: probably stays the same... though I expected this to look in memory, first
    {
    ECClassId id = ECClass::UNSET_ECCLASSID;
    if (!TryGetECClassId(id, schemaNameOrPrefix, className, resolveSchema))
        return nullptr;

    return GetECClass(id);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECDbSchemaManager::GetECClass(ECClassId ecClassId) const
    {
    return m_ecReader->GetECClass(ecClassId);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaManager::TryGetECClassId(ECClassId& id, Utf8CP schemaNameOrPrefix, Utf8CP className, ResolveSchema resolveSchema) const // WIP_FNV: probably stays the same... though I expected this to look in memory, first
    {
    return m_ecReader->TryGetECClassId(id, schemaNameOrPrefix, className, resolveSchema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                       12/13
//---------------------------------------------------------------------------------------
ECDerivedClassesList const& ECDbSchemaManager::GetDerivedECClasses (ECClassCR baseECClass) const
    {
    if (EnsureDerivedClassesExist (baseECClass) != SUCCESS)
       {
       BeAssert (false);
       }

    return baseECClass.GetDerivedClasses ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECEnumerationCP ECDbSchemaManager::GetECEnumeration(Utf8CP schemaName, Utf8CP enumName) const
    {
    return m_ecReader->GetECEnumeration(schemaName, enumName);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaManager::GetECSchemaKeys (ECSchemaKeys& keys) const
    {
    return ECDbSchemaPersistenceHelper::GetECSchemaKeys(keys, m_ecdb);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaManager::GetECClassKeys (ECClassKeys& keys, Utf8CP schemaName) const
    {
    PRECONDITION(schemaName != nullptr && "schemaName parameter cannot be null", ERROR);
    ECSchemaId schemaId = ECDbSchemaPersistenceHelper::GetECSchemaId (m_ecdb, schemaName);
    return ECDbSchemaPersistenceHelper::GetECClassKeys(keys, schemaId, m_ecdb);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbSchemaManager::ClearCache () const { m_ecReader->ClearCache(); }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle               12/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECDbCR ECDbSchemaManager::GetECDb () const { return m_ecdb; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      01/2013
//---------------------------------------------------------------------------------------
ECSchemaPtr ECDbSchemaManager::_LocateSchema (SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext)
    {
    Utf8String schemaName(key.m_schemaName);
    const ECSchemaId schemaId = ECDbSchemaPersistenceHelper::GetECSchemaId(GetECDb (), schemaName.c_str()); //WIP_FNV: could be more efficient if it first looked through those already cached in memory...
    if (0 == schemaId)
        return nullptr;

    ECSchemaCP schema = m_ecReader->GetECSchema(schemaId, true);
    if (schema == nullptr)
        return nullptr;

    if (schema->GetSchemaKey ().Matches (key, matchType))
        {
        ECSchemaP schemaP = const_cast<ECSchemaP> (schema);
        schemaContext.GetCache ().AddSchema (*schemaP);
        return schemaP;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECDbSchemaManager::_LocateClass (Utf8CP schemaName, Utf8CP className)
    {
    return GetECClass (schemaName, className);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2013
//---------------------------------------------------------------------------------------
//static
ECClassId ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema (ECDbCR db, ECClassCR ecClass)
    {
    ECClassId id = ECClass::UNSET_ECCLASSID;
    db.Schemas().TryGetECClassId(id, ecClass.GetSchema().GetName().c_str(), ecClass.GetName().c_str(), ResolveSchema::BySchemaName);
    const_cast<ECClassR>(ecClass).SetId(id);
    return id;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2013
//---------------------------------------------------------------------------------------
ECPropertyId ECDbSchemaManager::GetPropertyIdForECPropertyFromDuplicateECSchema(ECDbCR db, ECPropertyCR ecProperty)
    {
    ECPropertyId ecPropertyId = ECDbSchemaPersistenceHelper::GetECPropertyId (db, ecProperty.GetClass().GetSchema().GetName().c_str(), ecProperty.GetClass().GetName().c_str(), ecProperty.GetName().c_str()); 
    const_cast<ECPropertyR>(ecProperty).SetId(ecPropertyId);
    return ecPropertyId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2013
//---------------------------------------------------------------------------------------
ECSchemaId ECDbSchemaManager::GetSchemaIdForECSchemaFromDuplicateECSchema(ECDbCR db, ECSchemaCR ecSchema)
    {
    const ECSchemaId ecSchemaId = ECDbSchemaPersistenceHelper::GetECSchemaId(db, ecSchema.GetName().c_str());
    const_cast<ECSchemaR>(ecSchema).SetId(ecSchemaId);
    return ecSchemaId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan      03/2013
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSchemaManager::EnsureDerivedClassesExist(ECN::ECClassCR ecClass) const
    {
    ECClassId ecClassId = ECClass::UNSET_ECCLASSID;
    if (ecClass.HasId())
        ecClassId = ecClass.GetId();
    else
        ecClassId = GetClassIdForECClassFromDuplicateECSchema(m_ecdb, ecClass);

    return m_ecReader->EnsureDerivedClassesExist(ecClassId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan   12/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSchemaManager::CreateECClassViewsInDb() const
    {
    return m_map.CreateECClassViewsInDb();
    }

//*********************************************************************************
// ECDbSchemaManager::ImportOptions
//*********************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   04/2014
//---------------------------------------------------------------------------------------
ECDbSchemaManager::ImportOptions::ImportOptions ()
    : m_doSupplementation (true)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   04/2014
//---------------------------------------------------------------------------------------
ECDbSchemaManager::ImportOptions::ImportOptions (bool doSupplementation)
    : m_doSupplementation (doSupplementation)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   04/2014
//---------------------------------------------------------------------------------------
bool ECDbSchemaManager::ImportOptions::DoSupplementation () const { return m_doSupplementation; }


END_BENTLEY_SQLITE_EC_NAMESPACE


