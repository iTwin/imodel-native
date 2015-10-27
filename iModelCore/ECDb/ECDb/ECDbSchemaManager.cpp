/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
ECDbSchemaManager::ECDbSchemaManager (ECDbR ecdb, ECDbMapR map)
    :m_ecdb (ecdb), m_map (map)
    {
    m_ecReader = ECDbSchemaReader::Create(ecdb);
    m_ecImporter= ECDbSchemaWriter::Create(ecdb);
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
    SchemaImportContext context (m_map.GetSQLManagerR().GetDbSchemaR());
    if (SUCCESS != context.Initialize())
        return ERROR;

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
    
    bvector<ECSchemaCP> importedSchemas;
    bvector<ECDiffPtr> diffs;
    if (SUCCESS != BatchImportOrUpdateECSchemas (context, importedSchemas, diffs, cache, options, true))
        return ERROR;
  
    if (MapStatus::Error == m_map.MapSchemas (context, importedSchemas, !diffs.empty ()))
        return ERROR;

    //Clear cache in case we have diffs
    if (!diffs.empty())
        m_ecdb.ClearECDbCache();

    timer.Stop();
    LOG.infov("Imported ECSchemas in %.4f msecs.",  timer.GetElapsedSeconds() * 1000.0);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                   Affan.Khan        29/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaManager::BatchImportOrUpdateECSchemas (SchemaImportContext const& context, bvector<ECN::ECSchemaCP>& importedSchemas, bvector<ECN::ECDiffPtr>&  diffs, ECSchemaCacheR schemaCache, ImportOptions const& options, bool addToReaderCache) const
    {
    bvector<ECSchemaP> schemas;
    schemaCache.GetSchemas(schemas);

    bvector<ECSchemaP> schemasToImport;
    for (ECSchemaP schema : schemas)
        {
        BeAssert(schema != nullptr);
        if (schema == nullptr) continue;

        ECSchemaId id = ECDbSchemaPersistence::GetECSchemaId(this->m_ecdb, *schema);
        if (schema->HasId() && (id == 0 || id != schema->GetId()))
            {
            m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECSchema %s is owned by some other ECDb file.", schema->GetFullSchemaName().c_str());
            return ERROR;
            }

        if (id <= 0ULL || // skip ECSchemas that are already imported(if not updating)
            options.UpdateExistingSchemas())
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
            if (primarySchema->IsSupplemented())
                continue;

            SupplementedSchemaBuilder builder;
            SupplementedSchemaStatus status = builder.UpdateSchema(*primarySchema, suppSchemas, false /*dont create ca copy while supplementing*/);
            if (SUPPLEMENTED_SCHEMA_STATUS_Success != status)
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
        BuildDependencyOrderedSchemaList (dependencyOrderedPrimarySchemas, schema);
    
    primarySchemas.clear (); // Just make sure no one tries to use it anymore

    ECSchemaValidationResult validationResult;
    bool isValid = ECSchemaValidator::ValidateSchemas(validationResult, dependencyOrderedPrimarySchemas, options.SupportLegacySchemas());
    if (validationResult.HasErrors())
        {
        std::vector<Utf8String> errorMessages;
        validationResult.ToString(errorMessages);

        ECDbIssueSeverity sev;
        if (options.SupportLegacySchemas())
            sev = ECDbIssueSeverity::Warning;
        else
            {
            sev = ECDbIssueSeverity::Error;
            m_ecdb.GetECDbImplR().GetIssueReporter().Report(sev, "Failed to import ECSchemas. Details: ");
            }

        for (Utf8StringCR errorMessage : errorMessages)
            m_ecdb.GetECDbImplR().GetIssueReporter().Report(sev, errorMessage.c_str());
        }

    if (!isValid)
        return ERROR;

    for (ECSchemaCP schema : dependencyOrderedPrimarySchemas)
        {
        importedSchemas.push_back(schema);

        ECDiffPtr diff;
        if (0ULL != ECDbSchemaPersistence::GetECSchemaId(m_ecdb, schema->GetName().c_str()))
            {
            if (!options.UpdateExistingSchemas())
                continue;

            //Go a head and attempt to update ECSchema
            if (SUCCESS != UpdateECSchema(diff, *schema))
                return ERROR;

            if (diff != nullptr && diff->GetStatus() == DIFFSTATUS_Success && !diff->IsEmpty())
                diffs.push_back(diff);
            }
        else
            if (SUCCESS != ImportECSchema(*schema, addToReaderCache))
                return ERROR;
        }

    if (!diffs.empty ())
        ClearCache ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                   Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaManager::ImportECSchema (ECSchemaCR ecSchema, bool addToReaderCache) const
    {
    if (SUCCESS != m_ecImporter->Import (ecSchema))
        return ERROR;

    if (addToReaderCache)
        m_ecReader->AddECSchemaToCache (ecSchema);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                   Affan.Khan        05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaManager::UpdateECSchema (ECDiffPtr& diff, ECSchemaCR ecSchema) const
    {
    auto existingSchema = GetECSchema (ecSchema.GetName().c_str (), true);
    if (existingSchema == nullptr)
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
            "Failed to update ECSchema '%s'. ECSchema does not exist in the ECDb file.",
                                                        ecSchema.GetName().c_str ());
        return ERROR;
        }

    if (existingSchema->GetVersionMajor () != ecSchema.GetVersionMajor () ||
        existingSchema->GetVersionMinor () > ecSchema.GetVersionMinor ())
        {
        if (ecSchema.IsStandardSchema ())
            return BentleyStatus::SUCCESS; 

        ReportUpdateError (ecSchema, *existingSchema, "Version mismatch: Major version must be equal, minor version must be greater or equal than version of existing schema.");
        return ERROR;
        }

    if (existingSchema->GetNamespacePrefix () != ecSchema.GetNamespacePrefix ())
        {
        Utf8String reason;
        reason.Sprintf ("Namespace prefixes differ: New prefix: %s - existing prefix : %s.",
            ecSchema.GetNamespacePrefix ().c_str (), existingSchema->GetNamespacePrefix ().c_str ());

        ReportUpdateError (ecSchema, *existingSchema, reason.c_str ());
        return ERROR;
        }

    diff = ECDiff::Diff (*existingSchema, ecSchema);
    if (diff->GetStatus () != DIFFSTATUS_Success)
        {
        ReportUpdateError (ecSchema, *existingSchema, "Could not compute the difference between the new and the existing version.");
        return ERROR;
        }

    if (diff->IsEmpty ())
        return SUCCESS; //nothing to update

    LOG.errorv("The ECSchema '%s' cannot be updated. Updating ECSchemas is not yet supported in this version of ECDb.",
               ecSchema.GetName().c_str());
    return ERROR;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECDbSchemaManager::GetECSchema (Utf8CP schemaName, bool ensureAllClassesLoaded) const
    {
    const ECSchemaId schemaId = ECDbSchemaPersistence::GetECSchemaId(GetECDb(), schemaName); //WIP_FNV: could be more efficient if it first looked through those already cached in memory...
    if (0 == schemaId)
        return nullptr;

    ECSchemaP schema = nullptr;
    if (m_ecReader->GetECSchema(schema, schemaId, ensureAllClassesLoaded) == SUCCESS)
        return schema;
    else
        return nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECDbSchemaManager::GetECSchema (ECSchemaId schemaId, bool ensureAllClassesLoaded) const
    {
    ECSchemaP schema = nullptr;
    if (m_ecReader->GetECSchema(schema, schemaId, ensureAllClassesLoaded) == SUCCESS)
        return schema;
    else
        return nullptr;
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

    return ECDbSchemaPersistence::GetECSchemaId (m_ecdb, schemaName) > 0ULL;
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

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaManager::GetECSchemaKeys (ECSchemaKeys& keys) const
    {
    return ECDbSchemaPersistence::GetECSchemaKeys(keys, m_ecdb);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaManager::GetECClassKeys (ECClassKeys& keys, Utf8CP schemaName) const
    {
    PRECONDITION(schemaName != nullptr && "schemaName parameter cannot be null", ERROR);
    ECSchemaId schemaId = ECDbSchemaPersistence::GetECSchemaId (m_ecdb, schemaName);
    return ECDbSchemaPersistence::GetECClassKeys(keys, schemaId, m_ecdb);
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
    const ECSchemaId schemaId = ECDbSchemaPersistence::GetECSchemaId(GetECDb (), schemaName.c_str()); //WIP_FNV: could be more efficient if it first looked through those already cached in memory...
    if (0 == schemaId)
        return nullptr;

    ECSchemaP schema = nullptr;
    if (m_ecReader->GetECSchema(schema, schemaId, true) != SUCCESS)
        return nullptr;

    if (schema->GetSchemaKey ().Matches (key, matchType))
        {
        schemaContext.GetCache ().AddSchema (*schema);
        return schema;
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
    ECPropertyId ecPropertyId = ECDbSchemaPersistence::GetECPropertyId (db, ecProperty.GetClass().GetSchema().GetName().c_str(), ecProperty.GetClass().GetName().c_str(), ecProperty.GetName().c_str()); 
    const_cast<ECPropertyR>(ecProperty).SetId(ecPropertyId);
    return ecPropertyId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2013
//---------------------------------------------------------------------------------------
ECSchemaId ECDbSchemaManager::GetSchemaIdForECSchemaFromDuplicateECSchema(ECDbCR db, ECSchemaCR ecSchema)
    {
    const ECSchemaId ecSchemaId = ECDbSchemaPersistence::GetECSchemaId(db, ecSchema);
    const_cast<ECSchemaR>(ecSchema).SetId(ecSchemaId);
    return ecSchemaId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan      03/2013
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSchemaManager::EnsureDerivedClassesExist(ECN::ECClassCR ecClass) const
    {
    ECClassId ecClassId = ECClass::UNSET_ECCLASSID;
    if (ecClass.HasId ())
        ecClassId = ecClass.GetId ();
    else
        ecClassId = GetClassIdForECClassFromDuplicateECSchema(m_ecdb, ecClass);

    ECDbSchemaPersistence::ECClassIdList derivedClassIds;
    if (SUCCESS != ECDbSchemaPersistence::GetDerivedECClasses(derivedClassIds, ecClassId, m_ecdb))
        return ERROR;

    for (ECClassId derivedClassId : derivedClassIds)
        {
        if (GetECClass (derivedClassId) == nullptr)
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   04/2014
//---------------------------------------------------------------------------------------
void ECDbSchemaManager::ReportUpdateError(ECN::ECSchemaCR newSchema, ECN::ECSchemaCR existingSchema, Utf8CP reason) const
    {
    const uint32_t newVersionMajor = newSchema.GetVersionMajor();
    const uint32_t newVersionMinor = newSchema.GetVersionMinor();
    const uint32_t existingVersionMajor = existingSchema.GetVersionMajor();
    const uint32_t existingVersionMinor = existingSchema.GetVersionMinor();

    Utf8String str("Failed to update ECSchema '");
    str.append(newSchema.GetName());
    str.append("' from version ").append(ECSchema::FormatSchemaVersion(existingVersionMajor, existingVersionMinor));
    str.append(" to ").append(ECSchema::FormatSchemaVersion(newVersionMajor, newVersionMinor));
    str.append(". ").append(reason);

    m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, str.c_str());
    }


//*********************************************************************************
// ECDbSchemaManager::ImportOptions
//*********************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   04/2014
//---------------------------------------------------------------------------------------
ECDbSchemaManager::ImportOptions::ImportOptions ()
    : m_doSupplementation (true), m_updateExistingSchemas (false), m_supportLegacySchemas (false)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   04/2014
//---------------------------------------------------------------------------------------
ECDbSchemaManager::ImportOptions::ImportOptions (bool doSupplementation, bool updateExistingSchemas)
    : m_doSupplementation (doSupplementation), m_updateExistingSchemas (updateExistingSchemas), m_supportLegacySchemas (false)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   04/2014
//---------------------------------------------------------------------------------------
bool ECDbSchemaManager::ImportOptions::DoSupplementation () const { return m_doSupplementation; }
bool ECDbSchemaManager::ImportOptions::UpdateExistingSchemas () const { return m_updateExistingSchemas; }
void ECDbSchemaManager::ImportOptions::SetSupportLegacySchemas () { m_supportLegacySchemas = true; }
bool ECDbSchemaManager::ImportOptions::SupportLegacySchemas () const { return m_supportLegacySchemas; }


END_BENTLEY_SQLITE_EC_NAMESPACE


