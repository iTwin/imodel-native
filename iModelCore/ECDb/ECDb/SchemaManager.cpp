/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbExpressionSymbolProvider.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//******************************** SchemaManager ****************************************
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaManager::SchemaManager(ECDbCR ecdb, BeMutex& mutex) : m_ecdb(ecdb), m_dbMap(new DbMap(ecdb)), m_schemaReader(new SchemaReader(ecdb)), m_mutex(mutex) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      01/2013
//---------------------------------------------------------------------------------------
SchemaManager::~SchemaManager()
    {
    if (m_schemaReader != nullptr)
        {
        delete m_schemaReader;
        m_schemaReader = nullptr;
        }

    if (m_dbMap != nullptr)
        {
        delete m_dbMap;
        m_dbMap = nullptr;
        }
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                   Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECSchemaCP> SchemaManager::GetSchemas(bool loadSchemaEntities) const
    {
    BeMutexHolder lock(m_mutex);

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
        ECSchemaCP out = GetSchema(schemaId, loadSchemaEntities);
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
void InsertSchemaInDependencyOrderedList(bvector<ECSchemaCP>& schemas, ECSchemaCP insertSchema)
    {
    if (std::find(schemas.begin(), schemas.end(), insertSchema) != schemas.end())
        return; // This (and its referenced ECSchemas) are already in the list

    bvector<ECSchemaCP>::reverse_iterator rit;
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
void BuildDependencyOrderedSchemaList(bvector<ECSchemaCP>& schemas, ECSchemaCP insertSchema)
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
BentleyStatus SchemaManager::ImportSchemas(bvector<ECSchemaCP> const& schemas, SchemaImportToken const* schemaImportToken) const
    {
    return ImportSchemas(schemas, SchemaImportOptions::None, schemaImportToken);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                     06/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaManager::ImportSchemas(bvector<ECSchemaCP> const& schemas, SchemaImportOptions options, SchemaImportToken const* schemaImportToken) const
    {
    PERFLOG_START("ECDb", "Schema import");
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin SchemaManager::ImportSchemas");
    SchemaImportContext ctx(options);
    const BentleyStatus stat = DoImportSchemas(ctx, schemas, schemaImportToken);
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("End SchemaManager::ImportSchemas");
    m_ecdb.ClearECDbCache();
    m_ecdb.FireAfterSchemaImportEvent();
    PERFLOG_FINISH("ECDb", "Schema import");
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                     06/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaManager::DoImportSchemas(SchemaImportContext& ctx, bvector<ECSchemaCP> const& schemas, SchemaImportToken const* schemaImportToken) const
    {
    Policy policy = PolicyManager::GetPolicy(SchemaImportPermissionPolicyAssertion(GetECDb(), schemaImportToken));
    if (!policy.IsSupported())
        {
        LOG.error("Failed to import ECSchemas: Caller has not provided an SchemaImportToken.");
        return ERROR;
        }

    if (m_ecdb.IsReadonly())
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to import ECSchemas. ECDb file is read-only.");
        return ERROR;
        }

    if (schemas.empty())
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to import ECSchemas. List of ECSchemas to import is empty.");
        return ERROR;
        }

    BeMutexHolder lock(m_mutex);

    bvector<ECSchemaCP> schemasToMap;
    if (SUCCESS != PersistSchemas(ctx, schemasToMap, schemas))
        return ERROR;

    if (schemasToMap.empty())
        return SUCCESS;

    for (ECSchemaCP schema : schemasToMap)
        {
        if (SUCCESS != ctx.GetSchemaPoliciesR().ReadPolicies(m_ecdb, *schema))
            return ERROR;
        }

    if (SUCCESS != ViewGenerator::DropECClassViews(GetECDb()))
        return ERROR;

    if (SUCCESS != ViewGenerator::DropUpdatableViews(GetECDb()))
        return ERROR;

    if (SUCCESS != GetDbMap().MapSchemas(ctx, schemasToMap))
        return ERROR;

    return ViewGenerator::CreateUpdatableViews(GetECDb());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                   Affan.Khan        29/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaManager::PersistSchemas(SchemaImportContext& context, bvector<ECN::ECSchemaCP>& schemasToMap, bvector<ECSchemaCP> const& schemas) const
    {
    bvector<ECSchemaCP> schemasToImport;
    for (ECSchemaCP schema : schemas)
        {
        if (schema == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        //this is the in-memory version of ECSchemas. ECDb only supports the latest in-memory version.
        //Deserializing into older versions is not needed in ECDb and therefore not supported.
        if (schema->GetECVersion() != ECVersion::Latest)
            {
            m_ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to import ECSchemas. The in-memory version of the ECSchema '%s' must be %s, but is %s.", schema->GetFullSchemaName().c_str(), ECSchema::GetECVersionString(ECVersion::Latest), ECSchema::GetECVersionString(schema->GetECVersion()));
            return ERROR;
            }

        if (schema->HasId())
            {
            ECSchemaId id = SchemaPersistenceHelper::GetSchemaId(m_ecdb, schema->GetName().c_str());
            if (!id.IsValid() || id != schema->GetId())
                {
                m_ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to import ECSchemas. ECSchema %s is owned by some other ECDb file.", schema->GetFullSchemaName().c_str());
                return ERROR;
                }
            }

        BuildDependencyOrderedSchemaList(schemasToImport, schema);
        }

    PERFLOG_START("ECDb", "Schema import> Schema supplementation");
    bvector<ECSchemaCP> primarySchemas;
    bvector<ECSchemaP> suppSchemas;
    for (ECSchemaCP schema : schemasToImport)
        {
        if (schema->IsSupplementalSchema())
            {
            if (SchemaLocalizedStrings::IsLocalizationSupplementalSchema(schema))
                {
                LOG.warningv("Localization ECSchema '%s' is ignored as ECDb always persists ECSchemas in the invariant culture.", schema->GetFullSchemaName().c_str());
                continue;
                }

            suppSchemas.push_back(const_cast<ECSchemaP> (schema));
            }
        else
            primarySchemas.push_back(schema);
        }

    if (!suppSchemas.empty())
        {
        for (ECSchemaCP primarySchema : primarySchemas)
            {
            if (primarySchema->IsSupplemented())
                continue;

            ECSchemaP primarySchemaP = const_cast<ECSchemaP> (primarySchema);
            SupplementedSchemaBuilder builder;
            SupplementedSchemaStatus status = builder.UpdateSchema(*primarySchemaP, suppSchemas, false /*dont create ca copy while supplementing*/);
            if (SupplementedSchemaStatus::Success != status)
                {
                m_ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to import ECSchemas. Failed to supplement ECSchema %s. See log file for details.", primarySchema->GetFullSchemaName().c_str());
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
                        primarySchemaP->AddReferencedSchema(const_cast<ECSchemaR>(bsca));
                        }
                    }
                }
            }
        }
    PERFLOG_FINISH("ECDb", "Schema import> Schema supplementation");

    // The dependency order may have *changed* due to supplementation adding new ECSchema references! Re-sort them.
    bvector<ECSchemaCP> primarySchemasOrderedByDependencies;
    for (ECSchemaCP schema : primarySchemas)
        BuildDependencyOrderedSchemaList(primarySchemasOrderedByDependencies, schema);

    primarySchemas.clear(); // Just make sure no one tries to use it anymore
    
      ECDbExpressionSymbolContext symbolsContext(m_ecdb);

    SchemaWriter schemaWriter(m_ecdb, context);
    return schemaWriter.ImportSchemas(schemasToMap, primarySchemasOrderedByDependencies);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP SchemaManager::GetSchema(Utf8CP schemaName, bool loadSchemaEntities) const
    {
    BeMutexHolder lock(m_mutex);

    const ECSchemaId schemaId = SchemaPersistenceHelper::GetSchemaId(GetECDb(), schemaName);
    if (!schemaId.IsValid())
        return nullptr;

    return GetSchema(schemaId, loadSchemaEntities);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP SchemaManager::GetSchema(ECSchemaId schemaId, bool loadSchemaEntities) const
    {
    return GetReader().GetSchema(schemaId, loadSchemaEntities);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaManager::ContainsSchema(Utf8CP schemaName)  const
    {
    if (Utf8String::IsNullOrEmpty(schemaName))
        {
        BeAssert(false && "schemaName argument to ContainsSchema must not be null or empty string.");
        return false;
        }

    BeMutexHolder lock(m_mutex);
    return SchemaPersistenceHelper::GetSchemaId(m_ecdb, schemaName).IsValid();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP SchemaManager::GetClass(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode mode) const
    {
    BeMutexHolder lock(m_mutex);

    const ECClassId id = GetClassId(schemaNameOrAlias, className, mode);
    return GetClass(id);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP SchemaManager::GetClass(ECClassId ecClassId) const
    {
    BeMutexHolder lock(m_mutex);

    return GetReader().GetClass(ecClassId);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassId SchemaManager::GetClassId(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode mode) const
    {
    BeMutexHolder lock(m_mutex);

    return GetReader().GetClassId(schemaNameOrAlias, className, mode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                       12/13
//---------------------------------------------------------------------------------------
ECDerivedClassesList const& SchemaManager::GetDerivedClasses(ECClassCR ecClass) const
    {
    BeMutexHolder lock(m_mutex);

    ECClassId id = GetReader().GetClassId(ecClass);
    if (id.IsValid())
        {
        if (SUCCESS != GetReader().EnsureDerivedClassesExist(id))
            LOG.errorv("Could not load derived classes for ECClass %s.", ecClass.GetFullName());
        }
    else
        LOG.errorv("Cannot call SchemaManager::GetDerivedClasses on ECClass %s. The ECClass does not exist in the ECDb file %s.", ecClass.GetFullName(), m_ecdb.GetDbFileName());

    return ecClass.GetDerivedClasses();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECEnumerationCP SchemaManager::GetEnumeration(Utf8CP schemaName, Utf8CP enumName) const
    {
    BeMutexHolder lock(m_mutex);

    return GetReader().GetEnumeration(schemaName, enumName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2016
//+---------------+---------------+---------------+---------------+---------------+------
KindOfQuantityCP SchemaManager::GetKindOfQuantity(Utf8CP schemaName, Utf8CP koqName) const
    {
    BeMutexHolder lock(m_mutex);

    return GetReader().GetKindOfQuantity(schemaName, koqName);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaManager::ClearCache() const 
    {
    GetReader().ClearCache(); 
    GetDbMap().ClearCache();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle               09/2016
//+---------------+---------------+---------------+---------------+---------------+------
SchemaReader const& SchemaManager::GetReader() const { BeAssert(m_schemaReader != nullptr); return *m_schemaReader; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle               10/2016
//+---------------+---------------+---------------+---------------+---------------+------
DbMap const& SchemaManager::GetDbMap() const { BeAssert(m_dbMap != nullptr); return *m_dbMap; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle               12/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECDb const& SchemaManager::GetECDb() const { return m_ecdb; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      01/2013
//---------------------------------------------------------------------------------------
ECSchemaPtr SchemaManager::_LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext)
    {
    SchemaKey foundKey;
    ECSchemaId foundId;
    if (!SchemaPersistenceHelper::TryGetSchemaKeyAndId(foundKey, foundId, m_ecdb, key.GetName().c_str()))
        return nullptr;

    if (!foundKey.Matches(key, matchType))
        return nullptr;

    ECSchemaCP schema = GetReader().GetSchema(foundId, true);
    if (schema == nullptr)
        return nullptr;

    ECSchemaP schemaP = const_cast<ECSchemaP> (schema);
    schemaContext.GetCache().AddSchema(*schemaP);
    return schemaP;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP SchemaManager::_LocateClass(Utf8CP schemaName, Utf8CP className)
    {
    return GetClass(schemaName, className);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan   12/2015
//---------------------------------------------------------------------------------------
BentleyStatus SchemaManager::CreateClassViewsInDb() const
    {
    BeMutexHolder lock(m_mutex);
    return ViewGenerator::CreateECClassViews(m_ecdb);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle   12/2016
//---------------------------------------------------------------------------------------
BentleyStatus SchemaManager::CreateClassViewsInDb(bvector<ECN::ECClassId> const& ecclassids) const
    {
    BeMutexHolder lock(m_mutex);
    return ViewGenerator::CreateECClassViews(m_ecdb, ecclassids);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle   04/2017
//---------------------------------------------------------------------------------------
BentleyStatus SchemaManager::RepopulateCacheTables() const
    {
    BeMutexHolder lock(m_mutex);
    if (SUCCESS != DbSchemaPersistenceManager::RepopulateClassHierarchyCacheTable(m_ecdb))
        {
        LOG.error("Failed to repopulate ECDb's cache table '" TABLE_ClassHierarchyCache "'.");
        return ERROR;
        }

    if (SUCCESS != DbSchemaPersistenceManager::RepopulateClassHasTableCacheTable(m_ecdb))
        {
        LOG.error("Failed to repopulate ECDb's cache table '" TABLE_ClassHasTablesCache "'.");
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle   02/2017
//---------------------------------------------------------------------------------------
//static
Utf8CP SchemaManager::GetValidateDbMappingSql() { return SQL_ValidateDbMapping; }

END_BENTLEY_SQLITE_EC_NAMESPACE


