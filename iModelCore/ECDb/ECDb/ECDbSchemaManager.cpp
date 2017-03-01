/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECUnits/Units.h>
#include "SchemaImportContext.h"
#include "ECDbExpressionSymbolProvider.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//******************************** ECDbSchemaManager ****************************************
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbSchemaManager::ECDbSchemaManager(ECDbCR ecdb, BeMutex& mutex) : m_ecdb(ecdb), m_dbMap(new ECDbMap(ecdb)), m_schemaReader(new ECDbSchemaReader(ecdb)), m_mutex(mutex) {}

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

    if (m_dbMap != nullptr)
        {
        delete m_dbMap;
        m_dbMap = nullptr;
        }
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                   Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECSchemaCP> ECDbSchemaManager::GetECSchemas(bool loadSchemaEntities) const
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
BentleyStatus ECDbSchemaManager::ImportECSchemas(bvector<ECSchemaCP> const& schemas, ECSchemaImportToken const* schemaImportToken) const
    {
    return ImportECSchemas(schemas, false, schemaImportToken);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                     06/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaManager::ImportECSchemas(bvector<ECSchemaCP> const& schemas, bool doNotFailSchemaValidationForLegacyIssues, ECSchemaImportToken const* schemaImportToken) const
    {
    PERFLOG_START("ECDb", "ECSchema import");
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin ECDbSchemaManager::ImportECSchemas");
    SchemaImportContext ctx(doNotFailSchemaValidationForLegacyIssues);
    const BentleyStatus stat = DoImportECSchemas(ctx, schemas, schemaImportToken);
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("End ECDbSchemaManager::ImportECSchemas");
    m_ecdb.ClearECDbCache();
    m_ecdb.FireAfterECSchemaImportEvent();
    PERFLOG_FINISH("ECDb", "ECSchema import");
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                     06/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaManager::DoImportECSchemas(SchemaImportContext& ctx, bvector<ECSchemaCP> const& schemas, ECSchemaImportToken const* schemaImportToken) const
    {
    ECDbPolicy policy = ECDbPolicyManager::GetPolicy(ECSchemaImportPermissionPolicyAssertion(GetECDb(), schemaImportToken));
    if (!policy.IsSupported())
        {
        LOG.error("Failed to import ECSchemas: Caller has not provided an ECSchemaImportToken.");
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

    if (ViewGenerator::DropECClassViews(GetECDb()) != SUCCESS)
        return ERROR;

    if (ViewGenerator::DropUpdatableViews(GetECDb()) != SUCCESS)
        return ERROR;

    if (SUCCESS != PersistECSchemas(ctx, schemas))
        return ERROR;

    ECSchemaCompareContext& compareContext = ctx.GetECSchemaCompareContext();
    if (compareContext.HasNoSchemasToImport())
        return SUCCESS;

    if (compareContext.ReloadContextECSchemas(*this) == ERROR)
        return ERROR;

    if (SUCCESS != GetDbMap().MapSchemas(ctx))
        return ERROR;

    return ViewGenerator::CreateUpdatableViews(GetECDb());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                   Affan.Khan        29/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbSchemaManager::PersistECSchemas(SchemaImportContext& context, bvector<ECSchemaCP> const& schemas) const
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
            ECSchemaId id = ECDbSchemaPersistenceHelper::GetECSchemaId(m_ecdb, schema->GetName().c_str());
            if (!id.IsValid() || id != schema->GetId())
                {
                m_ecdb.GetECDbImplR().GetIssueReporter().Report("Failed to import ECSchemas. ECSchema %s is owned by some other ECDb file.", schema->GetFullSchemaName().c_str());
                return ERROR;
                }
            }

        BuildDependencyOrderedSchemaList(schemasToImport, schema);
        }

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

    // The dependency order may have *changed* due to supplementation adding new ECSchema references! Re-sort them.
    bvector<ECSchemaCP> dependencyOrderedPrimarySchemas;
    for (ECSchemaCP schema : primarySchemas)
        BuildDependencyOrderedSchemaList(dependencyOrderedPrimarySchemas, schema);

    primarySchemas.clear(); // Just make sure no one tries to use it anymore

    const bool isValid = ECSchemaValidator::ValidateSchemas(m_ecdb.GetECDbImplR().GetIssueReporter(), dependencyOrderedPrimarySchemas, context.DoNotFailSchemaValidationForLegacyIssues());
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
    BeMutexHolder lock(m_mutex);

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

    BeMutexHolder lock(m_mutex);
    return ECDbSchemaPersistenceHelper::GetECSchemaId(m_ecdb, schemaName).IsValid();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECDbSchemaManager::GetECClass(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, ResolveSchema resolveSchema) const
    {
    BeMutexHolder lock(m_mutex);

    const ECClassId id = GetECClassId(schemaNameOrAlias, className, resolveSchema);
    return GetECClass(id);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECDbSchemaManager::GetECClass(ECClassId ecClassId) const
    {
    BeMutexHolder lock(m_mutex);

    return GetReader().GetECClass(ecClassId);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassId ECDbSchemaManager::GetECClassId(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, ResolveSchema resolveSchema) const
    {
    BeMutexHolder lock(m_mutex);

    return GetReader().GetECClassId(schemaNameOrAlias, className, resolveSchema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                       12/13
//---------------------------------------------------------------------------------------
ECDerivedClassesList const& ECDbSchemaManager::GetDerivedECClasses(ECClassCR ecClass) const
    {
    BeMutexHolder lock(m_mutex);

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
    BeMutexHolder lock(m_mutex);

    return GetReader().GetECEnumeration(schemaName, enumName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2016
//+---------------+---------------+---------------+---------------+---------------+------
KindOfQuantityCP ECDbSchemaManager::GetKindOfQuantity(Utf8CP schemaName, Utf8CP koqName) const
    {
    BeMutexHolder lock(m_mutex);

    return GetReader().GetKindOfQuantity(schemaName, koqName);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbSchemaManager::ClearCache() const 
    {
    GetReader().ClearCache(); 
    GetDbMap().ClearCache();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle               09/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECDbSchemaReader const& ECDbSchemaManager::GetReader() const { BeAssert(m_schemaReader != nullptr); return *m_schemaReader; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle               10/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECDbMap const& ECDbSchemaManager::GetDbMap() const { BeAssert(m_dbMap != nullptr); return *m_dbMap; }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle               12/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECDb const& ECDbSchemaManager::GetECDb() const { return m_ecdb; }

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
    BeMutexHolder lock(m_mutex);
    return ViewGenerator::CreateECClassViews(m_ecdb);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle   12/2016
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSchemaManager::CreateECClassViewsInDb(bvector<ECN::ECClassId> const& ecclassids) const
    {
    BeMutexHolder lock(m_mutex);
    return ViewGenerator::CreateECClassViews(m_ecdb, ecclassids);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle   02/2017
//---------------------------------------------------------------------------------------
//static
Utf8CP ECDbSchemaManager::GetValidateDbMappingSql() { return SQL_ValidateDbMapping; }

END_BENTLEY_SQLITE_EC_NAMESPACE


