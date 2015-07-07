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
ECDbSchemaManager::~ECDbSchemaManager ()
    {
    }

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


//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                     06/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaManager::ImportECSchemas 
(
ECSchemaCacheR cache, 
ImportOptions const& options,
IImportIssueListener const* userProvidedIssueListener
) const
    {
    SchemaImportContext context (userProvidedIssueListener);

    if (m_ecdb.IsReadonly ())
        {
        context.GetIssueListener ().Report (IImportIssueListener::Severity::Error, "Failed to import ECSchemas. ECDb file is read-only.");
        return ERROR;
        }

    if (cache.GetCount () == 0)
        {
        context.GetIssueListener ().Report (IImportIssueListener::Severity::Error, "Failed to import ECSchemas. List of ECSchemas to import is empty.");
        return ERROR;
        }

    BeMutexHolder lock (m_criticalSection);
    bvector<ECSchemaP> schemas;
    cache.GetSchemas (schemas);
    for (ECSchemaP schema : schemas)
        {
        ECSchemaId id = ECDbSchemaPersistence::GetECSchemaId(this->m_ecdb, *schema);
        if (schema->HasId())
            {
            if (id == 0 || id != schema->GetId())
                {
                LOG.errorv(L"ECSchema %s. is owned by some other ECDB file", schema->GetFullSchemaName().c_str());
                return ERROR;
                }
            }
        }
    // WIP_ECDB: Experiment with removing this on trunk. BatchImportECSchema now does its own dependency ordering. Not changing on Graphite01 in case the order is important for ECDbMapping, below
    bvector<ECSchemaP> schemasToImport;
    for (ECSchemaP schema: schemas)
        {
        BeAssert (schema != nullptr);
        if (schema == nullptr) continue;

        //Checks whether the schema contains classes or properties whose names only differ by case. This is (still) allowed in ECObjects,
        //but not supported by ECDb. (for non-legacy schemas, a check failure means abortion)
        auto& validationResult = context.GetSchemaValidationResultR (*schema);
        bool isValid = ECSchemaValidator::ValidateSchema (validationResult, *schema, options.SupportLegacySchemas ());
        if (validationResult.HasErrors ())
            {
            std::vector<Utf8String> errorMessages;
            validationResult.ToString (errorMessages);

            IImportIssueListener::Severity sev;
            if (options.SupportLegacySchemas ())
                sev = IImportIssueListener::Severity::Warning;
            else
                {
                sev = IImportIssueListener::Severity::Error;
                context.GetIssueListener ().Report (sev, "Failed to import ECSchemas. Details: ");
                }

            for (Utf8StringCR errorMessage : errorMessages)
                context.GetIssueListener ().Report (sev, errorMessage.c_str ());
            }

        if (!isValid)
            return ERROR;

        Utf8String schemaName(schema->GetName());
        if (!ContainsECSchema (schemaName.c_str ()) || options.UpdateExistingSchemas ()) // skip ECSchemas that are already imported(if not updating)
            BuildDependencyOrderedSchemaList (schemasToImport, schema);
        }
    
    if (AssertOnDuplicateCopyOfSchema(schemasToImport))
        {
        return ERROR;
        }

    bvector<ECDiffPtr> diffs; 
    auto stat = BatchImportOrUpdateECSchemas (context, diffs, schemasToImport, options, true);
    if (SUCCESS != stat)
        return ERROR;
  
    bvector<ECSchemaCP> schemasToImportCP;
    for (auto schema : schemasToImport)
        {
        schemasToImportCP.push_back (schema);
        }

    MapStatus mapStatus = m_map.MapSchemas (context, schemasToImportCP, !diffs.empty ());
    if (mapStatus == MapStatus::Error)
        return ERROR;
    //Clear cache in case we have diffs
    if (!diffs.empty ())
        m_ecdb.ClearECDbCache();

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                  Muhammad.Zaighum                      11/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbSchemaManager::AssertOnDuplicateCopyOfSchema(const bvector<ECSchemaP>& schemas)
    {
    std::map<WString, ECSchemaP> myMap;
    for (auto const schema : schemas)
        {
        if (myMap[schema->GetFullSchemaName()] == schema)
            {
            LOG.errorv(L"Found more then one in-memory copy of ECSchema %s. Use single ECSchemaReadContext to Read all Schemas.", schema->GetFullSchemaName().c_str());
            return true;
            }

        myMap[schema->GetFullSchemaName()] = schema;
        }
 
    return false;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      12/2012
//---------------------------------------------------------------------------------------
bool isSupplemental (ECSchemaCR schema)
    {
    SupplementalSchemaMetaDataPtr metaData;
    return (SupplementalSchemaMetaData::TryGetFromSchema(metaData, schema) && metaData.IsValid());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                   Affan.Khan        29/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaManager::BatchImportOrUpdateECSchemas (SchemaImportContext const& context, bvector<ECDiffPtr>&  diffs, bvector<ECSchemaP> const& schemas, ImportOptions const& options, bool addToReaderCache) const
    {
    StopWatch timer (true);
    //1. Only import supplemental schema if doSupplement = True AND saveSupplementals = TRUE
    bvector<ECSchemaP> schemasToImport;
    for (auto primarySchema : schemas)
        {
        if (isSupplemental (*primarySchema))
            continue; // It's not really a primarySchema

        if (options.DoSupplementation ())
            {
            if (primarySchema->IsSupplemented ())
                LOG.warningv (L"Attempted to supplement ECSchema %ls that is already supplemented.", primarySchema->GetFullSchemaName ().c_str ());
            else
                {
                bvector<ECSchemaP> supplementalSchemas;
                GetSupplementalSchemas (supplementalSchemas, schemas, primarySchema->GetSchemaKey ());
                if (supplementalSchemas.size () > 0)
                    {
                    SupplementedSchemaBuilder builder;
                    SupplementedSchemaStatus status = builder.UpdateSchema (*primarySchema, supplementalSchemas, false /*dont create ca copy while supplementing*/);
                    if (status != SUPPLEMENTED_SCHEMA_STATUS_Success)
                        {
                        //TODO: print detail error in log. We cannot revert so we will import what we got
                        LOG.warningv (L"Encountered error while supplementing %ls", primarySchema->GetFullSchemaName ().c_str ());
                        }
                    }
                //All consolidated customattribute must be reference. But Supplemental Provenance in BSCA is not
                //This bug could also be fixed in SupplementSchema builder but its much safer to do it here for now.
                if (primarySchema->GetSupplementalInfo ().IsValid ())
                    {
                    auto provenance = primarySchema->GetCustomAttribute (L"SupplementalProvenance");
                    if (provenance.IsValid ())
                        {
                        auto& bsca = provenance->GetClass ().GetSchema ();
                        if (!ECSchema::IsSchemaReferenced (*primarySchema, bsca))
                            {
                            primarySchema->AddReferencedSchema (const_cast<ECSchemaR>(bsca));
                            }
                        }
                    }
                }
            }

        schemasToImport.push_back (primarySchema);
        }

    // The dependency order may have *changed* due to supplementation adding new ECSchema references! Re-sort them.
    bvector<ECSchemaP> dependencyOrderedSchemas;
    for (ECSchemaP schema : schemasToImport)
        BuildDependencyOrderedSchemaList (dependencyOrderedSchemas, schema);
    schemasToImport.clear (); // Just make sure no one tries to use it anymore

    for (ECSchemaP schema : dependencyOrderedSchemas)
        {
        BentleyStatus stat = SUCCESS;
        Utf8String schemaName (schema->GetName ().c_str ());

        ECDiffPtr diff;
        if (0 != ECDbSchemaPersistence::GetECSchemaId (m_ecdb, schemaName.c_str ()))
            {
            if (!options.UpdateExistingSchemas ())
                continue;

            //Go a head and attempt to update ECSchema
            if (isSupplemental (*schema))
                stat = UpdateECSchema (context, diff, *schema);
            else
                stat = UpdateECSchema (context, diff, *schema);

            if (SUCCESS == stat && diff != nullptr && diff->GetStatus () == DIFFSTATUS_Success && !diff->IsEmpty ())
                diffs.push_back (diff);
            }
        else
            {
            if (isSupplemental (*schema))
                stat = ImportECSchema (context, *schema, addToReaderCache);
            else
                stat = ImportECSchema (context, *schema, addToReaderCache);
            }
        if (SUCCESS != stat)
            return stat;
        }

    if (!diffs.empty ())
        ClearCache ();
    timer.Stop ();
    LOG.infov ("ECSchema Batch Import took %.4f msecs.", timer.GetElapsedSeconds () * 1000.0);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                   Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaManager::ImportECSchema (SchemaImportContext const& context, ECSchemaCR ecSchema, bool addToReaderCache) const
    {
    auto stat = m_ecImporter->Import (ecSchema);
    if (BE_SQLITE_OK != stat)
        {
        context.GetIssueListener ().Report (IImportIssueListener::Severity::Error,
                                            "Failed to import ECSchema '%s'. Please see log for details.",
                                            Utf8String (ecSchema.GetFullSchemaName ()).c_str ());

        return ERROR;
        }

    if (addToReaderCache)
        m_ecReader->AddECSchemaToCache (ecSchema);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                   Affan.Khan        05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaManager::UpdateECSchema (SchemaImportContext const& context, ECDiffPtr& diff, ECSchemaCR ecSchema) const
    {
    Utf8String schemaName (ecSchema.GetName ().c_str ());
    auto existingSchema = GetECSchema (schemaName.c_str (), true);
    if (existingSchema == nullptr)
        {
        context.GetIssueListener ().Report (IImportIssueListener::Severity::Error,
            "Failed to update ECSchema '%s'. ECSchema does not exist in the ECDb file.",
            schemaName.c_str ());
        return ERROR;
        }

    if (existingSchema->GetVersionMajor () != ecSchema.GetVersionMajor () ||
        existingSchema->GetVersionMinor () > ecSchema.GetVersionMinor ())
        {
        if (ecSchema.IsStandardSchema ())
            {
            return BentleyStatus::SUCCESS; 
            }
        else
            {
            ReportUpdateError (context, ecSchema, *existingSchema, "Version mismatch: Major version must be equal, minor version must be greater or equal than version of existing schema.");
            return ERROR;
            }
        }

    if (existingSchema->GetNamespacePrefix () != ecSchema.GetNamespacePrefix ())
        {
        Utf8String reason;
        reason.Sprintf ("Namespace prefixes differ: New prefix: %s - existing prefix : %s.",
            Utf8String (ecSchema.GetNamespacePrefix ()).c_str (), Utf8String (existingSchema->GetNamespacePrefix ()).c_str ());

        ReportUpdateError (context, ecSchema, *existingSchema, reason.c_str ());
        return ERROR;
        }

    diff = ECDiff::Diff (*existingSchema, ecSchema);
    if (diff->GetStatus () != DIFFSTATUS_Success)
        {
        ReportUpdateError (context, ecSchema, *existingSchema, "Could not compute the difference between the new and the existing version.");
        return ERROR;
        }

    if (diff->IsEmpty ())
        return SUCCESS; //nothing to update

    LOG.errorv("The ECSchema '%s' cannot be updated. Updating ECSchemas is not yet supported in this version of ECDb.",
               schemaName.c_str());
    return ERROR;
    /* Until schema update is working again...
    DbResult r = m_ecImporter->Update (*diff, *m_ecReader, m_map);
    if (BE_SQLITE_OK != r)
        {
        ReportUpdateError (context, ecSchema, *existingSchema, "Please see log for details.");
        return ERROR;
        }

    return SUCCESS;
    */
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                   Affan.Khan        12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbSchemaManager::GetSupplementalSchemas (bvector<ECSchemaP>& supplementalSchemas, bvector<ECSchemaP> const& schemas, SchemaKeyCR primarySchemaKey) const
    {
    supplementalSchemas.clear();
    SupplementalSchemaMetaDataPtr metaData;
    std::map<WString, ECSchemaP> supplementalSchemaMap;
    for (ECSchemaPtr const& schema : schemas)
        {
        if (SupplementalSchemaMetaData::TryGetFromSchema(metaData, *schema) && metaData.IsValid())
        if (metaData->IsForPrimarySchema (primarySchemaKey.m_schemaName, primarySchemaKey.m_versionMajor, primarySchemaKey.m_versionMinor, SCHEMAMATCHTYPE_LatestCompatible))
            {
            auto itor = supplementalSchemaMap.find (schema->GetName ());
            if (itor != supplementalSchemaMap.end ())
                {
                if (itor->second->GetSchemaKey ().LessThan (schema->GetSchemaKey (), SCHEMAMATCHTYPE_Exact))
                    {
                    supplementalSchemaMap[schema->GetName ()] = schema.get();
                    }
                }
            else
                supplementalSchemaMap[schema->GetName ()] = schema.get ();

            }
        }

    for (auto& key : supplementalSchemaMap)
        supplementalSchemas.push_back (key.second);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECDbSchemaManager::GetECSchema (Utf8CP schemaName, bool ensureAllClassesLoaded) const
    {
    ECSchemaP schema = nullptr;
    if (m_ecReader->GetECSchema (schema, schemaName, ensureAllClassesLoaded) == BE_SQLITE_ROW)
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
    if (m_ecReader->GetECSchema (schema, schemaId, ensureAllClassesLoaded) == BE_SQLITE_ROW)
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
ECClassCP ECDbSchemaManager::GetECClass (ECClassId ecClassId) const
    {
    return m_ecReader->GetECClass(ecClassId);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECDbSchemaManager::GetECClass (Utf8CP schemaNameOrPrefix, Utf8CP className, ResolveSchema resolveSchema) const // WIP_FNV: probably stays the same... though I expected this to look in memory, first
    {
    switch (resolveSchema)
        {
        case ResolveSchema::AutoDetect:
            return m_ecReader->GetECClass (schemaNameOrPrefix, className);
        case ResolveSchema::BySchemaName:
            {
            ECClassP ecClass = nullptr;
            if (m_ecReader->GetECClassBySchemaName (ecClass, schemaNameOrPrefix, className) == BE_SQLITE_ROW)
                return ecClass;
            else
                return nullptr;
            }
        case ResolveSchema::BySchemaNamespacePrefix:
            {
            ECClassP ecClass = nullptr;
            if (m_ecReader->GetECClassBySchemaNameSpacePrefix (ecClass, schemaNameOrPrefix, className) == BE_SQLITE_ROW)
                return ecClass;
            else
                return nullptr;
            }
        default:
            return nullptr;
        }

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
    return (ECDbSchemaPersistence::GetECSchemaKeys(keys, m_ecdb) == BE_SQLITE_DONE) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaManager::GetECClassKeys (ECClassKeys& keys, Utf8CP schemaName) const
    {
    PRECONDITION(schemaName != nullptr && "schemaName parameter cannot be null", ERROR);
    ECSchemaId schemaId = ECDbSchemaPersistence::GetECSchemaId (m_ecdb, schemaName);
    return (ECDbSchemaPersistence::GetECClassKeys (keys, schemaId, m_ecdb) == BE_SQLITE_DONE) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbSchemaManager::ClearCache () const
    {
    m_ecReader->ClearCache();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle               12/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECDbCR ECDbSchemaManager::GetECDb () const
    {
    return m_ecdb;
    }

/*---------------------------------------------------------------------------------**//**
* Returns true if thisSchema directly references possiblyReferencedSchema                                                                                      
* @bsimethod                                 Ramanujam.Raman                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DirectlyReferences (ECSchemaCP thisSchema, ECSchemaCP possiblyReferencedSchema)
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
bool DependsOn (ECSchemaCP thisSchema, ECSchemaCP possibleDependency)
    {
    if (DirectlyReferences(thisSchema, possibleDependency))
        return true;

    SupplementalSchemaMetaDataPtr metaData;
    if (SupplementalSchemaMetaData::TryGetFromSchema(metaData, *possibleDependency) 
        && metaData.IsValid() 
        && metaData->IsForPrimarySchema (thisSchema->GetName(), 0, 0, SCHEMAMATCHTYPE_Latest))
        {
        return true; // possibleDependency supplements thisSchema. possibleDependency must be imported before thisSchema
        }

    // Maybe possibleDependency supplements one of my references?
    ECSchemaReferenceListCR referencedSchemas = thisSchema->GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator it = referencedSchemas.begin(); it != referencedSchemas.end(); ++it)
        {
        if (DependsOn (it->second.get(), possibleDependency))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void InsertSchemaInDependencyOrderedList (bvector<ECSchemaP>& schemas, ECSchemaP insertSchema)
    {
    if (std::find (schemas.begin(), schemas.end(), insertSchema) != schemas.end())
        return; // This (and its referenced ECSchemas) are already in the list

    bvector<ECSchemaP>::reverse_iterator rit;
    for (rit = schemas.rbegin(); rit < schemas.rend(); ++rit)
        {
        if (DependsOn (insertSchema, *rit))
            {
            schemas.insert (rit.base(), insertSchema); // insert right after the referenced schema in the list
            return;
            }
        }

    schemas.insert (schemas.begin(), insertSchema); // insert at the beginning
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbSchemaManager::BuildDependencyOrderedSchemaList (bvector<ECSchemaP>& schemas, ECSchemaP insertSchema) const
    {
    InsertSchemaInDependencyOrderedList (schemas, insertSchema);
    ECSchemaReferenceListCR referencedSchemas = insertSchema->GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
        {
        ECSchemaR referencedSchema = *iter->second.get();
        InsertSchemaInDependencyOrderedList (schemas, &referencedSchema);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      01/2013
//---------------------------------------------------------------------------------------
ECSchemaPtr ECDbSchemaManager::_LocateSchema (SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext)
    {
    ECSchemaP schema = nullptr;
    Utf8String schemaName(key.m_schemaName);
    if (m_ecReader->GetECSchema (schema, schemaName.c_str (), true) != BE_SQLITE_ROW)
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
ECClassCP ECDbSchemaManager::_LocateClass (WCharCP schemaName, WCharCP className)
    {
    Utf8String schemaUtf8 (schemaName);
    Utf8String classUtf8 (className);
    return GetECClass (schemaUtf8.c_str(), classUtf8.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2013
//---------------------------------------------------------------------------------------
ECClassId ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema (Db& db, ECClassCR ecClass)
    {
    Utf8String schemaName(ecClass.GetSchema().GetName().c_str());
    Utf8String className(ecClass.GetName().c_str());
   
    ECClassId ecClassId = ECDbSchemaPersistence::GetECClassIdBySchemaName(db, schemaName.c_str(), className.c_str());
    const_cast<ECClassR>(ecClass).SetId(ecClassId);
    return ecClassId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2013
//---------------------------------------------------------------------------------------
ECPropertyId ECDbSchemaManager::GetPropertyIdForECPropertyFromDuplicateECSchema (Db& db, ECPropertyCR ecProperty)
    {
    Utf8String schemaName(ecProperty.GetClass().GetSchema().GetName().c_str());
    Utf8String className(ecProperty.GetClass().GetName().c_str());
    Utf8String propertyName(ecProperty.GetName().c_str());

    ECPropertyId ecPropertyId = ECDbSchemaPersistence::GetECPropertyId (db, schemaName.c_str(), className.c_str(), propertyName.c_str()); 
    const_cast<ECPropertyR>(ecProperty).SetId(ecPropertyId);
    return ecPropertyId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2013
//---------------------------------------------------------------------------------------
ECSchemaId ECDbSchemaManager::GetSchemaIdForECSchemaFromDuplicateECSchema (Db& db, ECSchemaCR ecSchema)
    {

    ECSchemaId ecSchemaId = ECDbSchemaPersistence::GetECSchemaId(db, ecSchema);
    const_cast<ECSchemaR>(ecSchema).SetId(ecSchemaId);
    return ecSchemaId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan      03/2013
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSchemaManager::EnsureDerivedClassesExist(ECN::ECClassCR ecClass) const
    {
    ECClassId ecClassId = -1LL;
    if (ecClass.HasId ())
        {
        ecClassId = ecClass.GetId ();
        }
    else
        {
        ecClassId = GetClassIdForECClassFromDuplicateECSchema(m_ecdb, ecClass);
        }

    ECDbSchemaPersistence::ECClassIdList derivedClassIds;
    DbResult r = ECDbSchemaPersistence::GetDerivedECClasses(derivedClassIds, ecClassId, m_ecdb);
    if (r != BE_SQLITE_DONE)
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
//static
void ECDbSchemaManager::ReportUpdateError (SchemaImportContext const& context, ECN::ECSchemaCR newSchema, ECN::ECSchemaCR existingSchema, Utf8CP reason)
    {
    auto newVersionMajor = newSchema.GetVersionMajor ();
    auto newVersionMinor = newSchema.GetVersionMinor ();
    auto existingVersionMajor = existingSchema.GetVersionMajor ();
    auto existingVersionMinor = existingSchema.GetVersionMinor ();

    Utf8String str ("Failed to update ECSchema '");
    str.append (Utf8String (newSchema.GetName ()));
    str.append ("' from version ").append (Utf8String (ECSchema::FormatSchemaVersion (existingVersionMajor, existingVersionMinor)));
    str.append (" to ").append (Utf8String (ECSchema::FormatSchemaVersion (newVersionMajor, newVersionMinor)));
    str.append (". ").append (reason);
    
    context.GetIssueListener ().Report (ECDbSchemaManager::IImportIssueListener::Severity::Error,
            str.c_str ());
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

//*********************************************************************************
// ECDbSchemaManager::IImportIssueListener
//*********************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   04/2014
//---------------------------------------------------------------------------------------
ECDbSchemaManager::IImportIssueListener::~IImportIssueListener () {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   04/2014
//---------------------------------------------------------------------------------------
void ECDbSchemaManager::IImportIssueListener::Report (ECDbSchemaManager::IImportIssueListener::Severity severity, Utf8CP message, ...) const
    {
    va_list args;
    va_start (args, message);

    Utf8String formattedMessage;
    formattedMessage.VSprintf (message, args);
    _OnIssueReported (severity, formattedMessage.c_str ());

    const auto logSeverity = severity == Severity::Warning ? NativeLogging::LOG_WARNING : NativeLogging::LOG_ERROR;
    if (LOG.isSeverityEnabled (logSeverity))
        LOG.message (logSeverity, formattedMessage.c_str ());

    va_end (args);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE


