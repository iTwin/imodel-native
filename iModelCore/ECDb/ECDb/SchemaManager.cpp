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
typedef bmap<ECN::SchemaKey, ECN::ECSchemaCP, SchemaKeyLessThan<SchemaMatchType::Exact>> SchemaMap;
typedef bset<ECN::SchemaKey, SchemaKeyLessThan<SchemaMatchType::Exact>> SchemaSet;

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void FindAllSchemasInGraph(ECN::ECSchemaCP schema, SchemaMap& schemaMap)
    {
    if (schemaMap.find(schema->GetSchemaKey()) != schemaMap.end())
        return;

    schemaMap[schema->GetSchemaKey()] = schema;
    for (const auto& entry : schema->GetReferencedSchemas())
        FindAllSchemasInGraph(entry.second.get(), schemaMap);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaMap FindAllSchemasInGraph(ECN::ECSchemaCR schema, bool includeThisSchema)
    {
    SchemaMap schemaMap;
    if (includeThisSchema)
        schemaMap[schema.GetSchemaKey()] = &schema;

    for (const auto& entry : schema.GetReferencedSchemas())
        FindAllSchemasInGraph(entry.second.get(), schemaMap);

    return schemaMap;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECN::ECSchemaCP> GetNextLayer(bvector<ECN::ECSchemaCP> const& schemas, bvector<ECN::ECSchemaCP> const& referencedBy)
    {
    bvector<ECN::ECSchemaCP> list;
    SchemaMap map;
    if (referencedBy.empty())
        {
        for (auto schema : schemas)
            if (map.find(schema->GetSchemaKey()) == map.end())
                map[schema->GetSchemaKey()] = schema;

        for (auto schema : schemas)
            for (const auto& ref : FindAllSchemasInGraph(*schema, false))
                {
                auto itor = map.find(ref.first);
                if (map.end() != itor)
                    map.erase(itor);
                }
        }
    else
        {
        for (auto schema : referencedBy)
            for (const auto& ref : schema->GetReferencedSchemas())
                if (map.end() == map.find(ref.first))
                    map[ref.first] = ref.second.get();


        for (auto const& entry : map)
            for (const auto& ref : FindAllSchemasInGraph(*entry.second, false))
                {
                auto itor = map.find(ref.first);
                if (map.end() != itor)
                    map.erase(itor);
                }
        }

    for (const auto& ref : map)
        list.push_back(ref.second);

    return list;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECN::ECSchemaCP> FindAllSchemasInGraph(bvector<ECN::ECSchemaCP> const& schemas)
    {
    SchemaMap map;
    for (ECN::ECSchemaCP schema : schemas)
        for (const auto& entry : FindAllSchemasInGraph(*schema, true))
            if (map.find(entry.first) == map.end())
                map[entry.first] = entry.second;

    bvector<ECN::ECSchemaCP> temp;
    for (const auto& entry : map)
        temp.push_back(entry.second);

    return temp;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECN::ECSchemaCP> Sort(bvector<ECN::ECSchemaCP> const& schemas)
    {
    bvector<ECN::ECSchemaCP> sortedList;
    bvector<ECN::ECSchemaCP> layer;
    do
        {
        layer = GetNextLayer(schemas, layer);
        std::reverse(layer.begin(), layer.end());
        for (ECN::ECSchemaCP schema : layer)
            sortedList.push_back(schema);

        } while (!layer.empty());

        std::reverse(sortedList.begin(), sortedList.end());
        return sortedList;
    }

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

    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("SELECT Id FROM ec_Schema");
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
    SchemaImportContext ctx(m_ecdb, options);
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
        m_ecdb.GetImpl().Issues().Report("Failed to import ECSchemas. ECDb file is read-only.");
        return ERROR;
        }

    if (schemas.empty())
        {
        m_ecdb.GetImpl().Issues().Report("Failed to import ECSchemas. List of ECSchemas to import is empty.");
        return ERROR;
        }

    BeMutexHolder lock(m_mutex);

    bvector<ECSchemaCP> schemasToMap;
    if (SUCCESS != PersistSchemas(ctx, schemasToMap, schemas))
        return ERROR;

    if (schemasToMap.empty())
        return SUCCESS;

    if (SUCCESS != ctx.GetSchemaPoliciesR().ReadPolicies(m_ecdb))
        return ERROR;

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
    bvector<ECSchemaCP> schemasToImport = FindAllSchemasInGraph(schemas);
    for (ECSchemaCP schema : schemasToImport)
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
            m_ecdb.GetImpl().Issues().Report("Failed to import ECSchemas. The in-memory version of the ECSchema '%s' must be %s, but is %s.", schema->GetFullSchemaName().c_str(), ECSchema::GetECVersionString(ECVersion::Latest), ECSchema::GetECVersionString(schema->GetECVersion()));
            return ERROR;
            }

        if (schema->HasId())
            {
            ECSchemaId id = GetReader().GetSchemaId(schema->GetName(), SchemaLookupMode::ByName);
            if (!id.IsValid() || id != schema->GetId())
                {
                m_ecdb.GetImpl().Issues().Report("Failed to import ECSchemas. ECSchema %s is owned by some other ECDb file.", schema->GetFullSchemaName().c_str());
                return ERROR;
                }
            }
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

    schemasToImport.clear();
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
                m_ecdb.GetImpl().Issues().Report("Failed to import ECSchemas. Failed to supplement ECSchema %s. See log file for details.", primarySchema->GetFullSchemaName().c_str());
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
    bvector<ECSchemaCP> primarySchemasOrderedByDependencies = Sort(primarySchemas);
    primarySchemas.clear(); // Just make sure no one tries to use it anymore
    ECDbExpressionSymbolContext symbolsContext(m_ecdb);
    SchemaWriter schemaWriter(m_ecdb, context);
    return schemaWriter.ImportSchemas(schemasToMap, primarySchemasOrderedByDependencies);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP SchemaManager::GetSchema(Utf8StringCR schemaNameOrAlias, bool loadSchemaEntities, SchemaLookupMode mode) const
    {
    BeMutexHolder lock(m_mutex);

    const ECSchemaId schemaId = GetReader().GetSchemaId(schemaNameOrAlias.c_str(), mode);
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
bool SchemaManager::ContainsSchema(Utf8StringCR schemaNameOrAlias, SchemaLookupMode mode)  const
    {
    if (schemaNameOrAlias.empty())
        {
        BeAssert(false && "schemaNameOrAlias argument to ContainsSchema must not be null or empty string.");
        return false;
        }

    BeMutexHolder lock(m_mutex);
    return GetReader().ContainsSchema(schemaNameOrAlias, mode);
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
ECEnumerationCP SchemaManager::GetEnumeration(Utf8StringCR schemaNameOrAlias, Utf8StringCR enumName, SchemaLookupMode mode) const
    {
    BeMutexHolder lock(m_mutex);
    return GetReader().GetEnumeration(schemaNameOrAlias, enumName, mode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2016
//+---------------+---------------+---------------+---------------+---------------+------
KindOfQuantityCP SchemaManager::GetKindOfQuantity(Utf8StringCR schemaNameOrAlias, Utf8StringCR koqName, SchemaLookupMode mode) const
    {
    BeMutexHolder lock(m_mutex);
    return GetReader().GetKindOfQuantity(schemaNameOrAlias, koqName, mode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
PropertyCategoryCP SchemaManager::GetPropertyCategory(Utf8StringCR schemaNameOrAlias, Utf8StringCR catName, SchemaLookupMode mode) const
    {
    BeMutexHolder lock(m_mutex);
    return GetReader().GetPropertyCategory(schemaNameOrAlias, catName, mode);
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

END_BENTLEY_SQLITE_EC_NAMESPACE


