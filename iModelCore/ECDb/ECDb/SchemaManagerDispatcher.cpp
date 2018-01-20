/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaManagerDispatcher.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*****************************************************************
//SchemaManager::Dispatcher
//*****************************************************************

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TableSpaceSchemaManager const* SchemaManager::Dispatcher::GetManager(Utf8CP tableSpaceName) const
    {
    BeMutexHolder lock(m_mutex);
    if (DbTableSpace::IsAny(tableSpaceName))
        {
        BeAssert(false && "tableSpaceName must not be empty for this call");
        return nullptr;
        }

    if (DbTableSpace::IsMain(tableSpaceName))
        return &Main();

    auto it = m_managers.find(tableSpaceName);
    if (it == m_managers.end())
        return nullptr;

    return it->second.get();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
SchemaManager::Dispatcher::Iterable SchemaManager::Dispatcher::GetIterable(Utf8CP tableSpaceName) const
    {
    BeMutexHolder lock(m_mutex);
    if (DbTableSpace::IsAny(tableSpaceName))
        return Iterable(*this);

    TableSpaceSchemaManager const* manager = GetManager(tableSpaceName);
    if (manager == nullptr)
        return Iterable();

    return Iterable(*manager);
    }
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaManager::Dispatcher::AddManager(DbTableSpace const& tableSpace) const
    {
    BeMutexHolder lock(m_mutex);
    if (!tableSpace.IsAttached())
        {
        BeAssert(tableSpace.IsValid() && "Should have been caught before as this method is expected to be called during attaching the db");
        BeAssert(!tableSpace.IsMain() && "Must not be called for the main table space");
        BeAssert(!tableSpace.IsTemp() && "Must not be called for the temp table space as schemas cannot be persisted in the temp table space");
        return ERROR;
        }

    BeAssert(m_managers.find(tableSpace.GetName()) == m_managers.end());
    BeAssert(DbTableSpace::Exists(m_ecdb, tableSpace.GetName().c_str()));

    std::unique_ptr<TableSpaceSchemaManager> manager = std::make_unique<TableSpaceSchemaManager>(m_ecdb, tableSpace);
    TableSpaceSchemaManager const* managerP = manager.get();
    m_managers[managerP->GetTableSpace().GetName()] = std::move(manager);
    m_orderedManagers.push_back(managerP);
    BeAssert(m_managers.size() == m_orderedManagers.size());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaManager::Dispatcher::RemoveManager(DbTableSpace const& tableSpace) const
    {
    BeMutexHolder lock(m_mutex);
    if (!tableSpace.IsAttached())
        {
        BeAssert(tableSpace.IsAttached());
        return ERROR;
        }

    auto it = m_managers.find(tableSpace.GetName());
    if (it == m_managers.end())
        return SUCCESS;

    TableSpaceSchemaManager const& manager = *it->second;
    for (auto it = m_orderedManagers.begin(); it != m_orderedManagers.end(); ++it)
        {
        if (&manager == *it)
            {
            m_orderedManagers.erase(it);
            break;
            }
        }

    m_managers.erase(it);

    BeAssert(m_managers.size() == m_orderedManagers.size());
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaManager::Dispatcher::InitMain()
    {
    std::unique_ptr<MainSchemaManager> main = std::make_unique<MainSchemaManager>(m_ecdb, m_mutex);
    MainSchemaManager* mainP = main.get();
    m_managers[mainP->GetTableSpace().GetName()] = std::move(main);
    m_orderedManagers.push_back(mainP);
    m_main = mainP;
    BeAssert(m_managers.size() == m_orderedManagers.size());
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaPtr SchemaManager::Dispatcher::LocateSchema(ECN::SchemaKeyR key, ECN::SchemaMatchType matchType, ECN::ECSchemaReadContextR ctx, Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return nullptr;

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        ECSchemaPtr schema = manager->LocateSchema(key, matchType, ctx);
        if (schema != nullptr)
            return schema;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                   Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECSchemaCP> SchemaManager::Dispatcher::GetSchemas(bool loadSchemaEntities, Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return bvector<ECSchemaCP>();

    bvector<ECSchemaCP> schemas;
    for (TableSpaceSchemaManager const* manager : iterable)
        {
        if (SUCCESS != manager->GetSchemas(schemas, loadSchemaEntities))
            return bvector<ECSchemaCP>();
        }

    return schemas;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
bool SchemaManager::Dispatcher::ContainsSchema(Utf8StringCR schemaNameOrAlias, SchemaLookupMode mode, Utf8CP tableSpace) const
    {
    if (schemaNameOrAlias.empty())
        {
        BeAssert(false && "schemaNameOrAlias argument to ContainsSchema must not be null or empty string.");
        return false;
        }
    
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return false;

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        if (manager->ContainsSchema(schemaNameOrAlias, mode))
            return true;
        }

    return false;
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaCP SchemaManager::Dispatcher::GetSchema(Utf8StringCR schemaNameOrAlias, bool loadSchemaEntities, SchemaLookupMode mode, Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return nullptr;

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        ECSchemaCP schema = manager->GetSchema(schemaNameOrAlias, loadSchemaEntities, mode);
        if (schema != nullptr)
            return schema;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECClassCP SchemaManager::Dispatcher::GetClass(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode mode, Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return nullptr;

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        ECClassCP ecClass = manager->GetClass(schemaNameOrAlias, className, mode);
        if (ecClass != nullptr)
            return ecClass;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECClassCP SchemaManager::Dispatcher::GetClass(ECN::ECClassId classId, Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return nullptr;

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        ECClassCP ecClass = manager->GetClass(classId);
        if (ecClass != nullptr)
            return ecClass;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECClassId SchemaManager::Dispatcher::GetClassId(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode mode, Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return ECClassId();

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        ECClassId id = manager->GetClassId(schemaNameOrAlias, className, mode);
        if (id.IsValid())
            return id;
        }

    return ECClassId();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
ClassMap const* SchemaManager::Dispatcher::GetClassMap(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode mode, Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return nullptr;

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        ECClassCP ecClass = manager->GetClass(schemaNameOrAlias, className, mode);
        if (ecClass != nullptr)
            return manager->GetClassMap(*ecClass);
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
ClassMap const* SchemaManager::Dispatcher::GetClassMap(ECClassCR ecClass, Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return nullptr;

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        ClassMap const* classMap = manager->GetClassMap(ecClass);
        if (classMap != nullptr)
            return classMap;
        }

    return nullptr;
    }
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaManager::Dispatcher::LoadDerivedClasses(ECN::ECClassCR baseClass, Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return ERROR;

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        if (SUCCESS == manager->LoadDerivedClasses(baseClass))
            return SUCCESS;
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECEnumerationCP SchemaManager::Dispatcher::GetEnumeration(Utf8StringCR schemaNameOrAlias, Utf8StringCR enumName, SchemaLookupMode mode, Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return nullptr;

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        ECEnumerationCP ecenum = manager->GetEnumeration(schemaNameOrAlias, enumName, mode);
        if (ecenum != nullptr)
            return ecenum;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
KindOfQuantityCP SchemaManager::Dispatcher::GetKindOfQuantity(Utf8StringCR schemaNameOrAlias, Utf8StringCR koqName, SchemaLookupMode mode, Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return nullptr;

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        KindOfQuantityCP koq = manager->GetKindOfQuantity(schemaNameOrAlias, koqName, mode);
        if (koq != nullptr)
            return koq;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
PropertyCategoryCP SchemaManager::Dispatcher::GetPropertyCategory(Utf8StringCR schemaNameOrAlias, Utf8StringCR catName, SchemaLookupMode mode, Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return nullptr;

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        PropertyCategoryCP cat = manager->GetPropertyCategory(schemaNameOrAlias, catName, mode);
        if (cat != nullptr)
            return cat;
        }

    return nullptr;
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaManager::Dispatcher::ClearCache(Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return;

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        manager->ClearCache();
        }
    }

//*****************************************************************
//TableSpaceSchemaManager
//*****************************************************************

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaPtr TableSpaceSchemaManager::LocateSchema(ECN::SchemaKeyR key, ECN::SchemaMatchType matchType, ECN::ECSchemaReadContextR ctx) const
    {
    CachedStatementPtr stmt = nullptr;
    if (m_tableSpace.IsMain())
        stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("SELECT Name,VersionDigit1,VersionDigit2,VersionDigit3,Id FROM main." TABLE_Schema " WHERE Name=?");
    else
        stmt = m_ecdb.GetImpl().GetCachedSqliteStatement(Utf8PrintfString("SELECT Name,VersionDigit1,VersionDigit2,VersionDigit3,Id FROM [%s]." TABLE_Schema " WHERE Name=?", m_tableSpace.GetName().c_str()).c_str());

    if (stmt == nullptr)
        return nullptr;

    if (BE_SQLITE_OK != stmt->BindText(1, key.GetName(), Statement::MakeCopy::No))
        return nullptr;

    if (stmt->Step() != BE_SQLITE_ROW)
        return nullptr;

    SchemaKey foundKey(stmt->GetValueText(0), stmt->GetValueInt(1), stmt->GetValueInt(2), stmt->GetValueInt(3));
    ECSchemaId foundSchemaId = stmt->GetValueId<ECSchemaId>(4);
    if (!foundKey.Matches(key, matchType))
        return nullptr;

    ECSchemaCP schema = m_reader.GetSchema(foundSchemaId, true);
    if (schema == nullptr)
        return nullptr;

    ECSchemaP schemaP = const_cast<ECSchemaP> (schema);
    ctx.GetCache().AddSchema(*schemaP);
    return schemaP;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TableSpaceSchemaManager::LoadDerivedClasses(ECN::ECClassCR baseClass) const
    {
    ECClassId id = m_reader.GetClassId(baseClass);
    if (!id.IsValid())
        {
        LOG.errorv("Cannot call SchemaManager::GetDerivedClasses on ECClass %s. The ECClass does not exist.", baseClass.GetFullName());
        return ERROR;
        }

    return m_reader.EnsureDerivedClassesExist(id);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   01/2016
//+---------------+---------------+---------------+---------------+---------------+------
ClassMap const* TableSpaceSchemaManager::GetClassMap(ECN::ECClassCR ecClass) const
    {
    ClassMapLoadContext ctx;
    ClassMap const* classMap = nullptr;
    if (SUCCESS != TryGetClassMap(classMap, ctx, ecClass) || classMap == nullptr)
        return nullptr;

    return classMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan   06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TableSpaceSchemaManager::TryGetClassMap(ClassMap const*& classMap, ClassMapLoadContext& ctx, ECN::ECClassCR ecClass) const
    {
    ClassMap* classMapP = nullptr;
    if (SUCCESS != TryGetClassMap(classMapP, ctx, ecClass))
        return ERROR;

    classMap = classMapP;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   02/2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TableSpaceSchemaManager::TryGetClassMap(ClassMap*& classMap, ClassMapLoadContext& ctx, ECN::ECClassCR ecClass) const
    {
    //we must use this method here and cannot just see whether ecClass has already an id
    //because the ecClass object can come from an ECSchema deserialized from disk, hence
    //not having the id set, and already imported in the ECSchema. In that case
    //ECDb does not set the ids on the ECClass objects
    if (!m_reader.GetClassId(ecClass).IsValid())
        {
        BeAssert(false && "ECClass must have an ECClassId when mapping to the ECDb.");
        return ERROR;
        }
    {
    BeMutexHolder ecdbMutex(GetECDb().GetImpl().GetMutex());
    classMap = nullptr;
    auto it = m_classMapDictionary.find(ecClass.GetId());
    if (m_classMapDictionary.end() != it)
        {
        classMap = it->second.get();
        return SUCCESS;
        }
    }

    BeSqliteDbMutexHolder dbMutex(const_cast<ECDb&>(GetECDb()));
    BeMutexHolder ecdbMutex(GetECDb().GetImpl().GetMutex());
    auto it = m_classMapDictionary.find(ecClass.GetId());
    if (m_classMapDictionary.end() != it)
        {
        classMap = it->second.get();
        return SUCCESS;
        }

    //lazy loading the class map implemented with const-casting the actual loading so that the 
    //get method itself can remain const (logically const)
    return TryLoadClassMap(classMap, ctx, ecClass);
    }

//---------------------------------------------------------------------------------------
//* @bsimethod                                 Affan.Khan                           07 / 2012
//---------------------------------------------------------------------------------------
BentleyStatus TableSpaceSchemaManager::TryLoadClassMap(ClassMap*& classMap, ClassMapLoadContext& ctx, ECN::ECClassCR ecClass) const
    {
    classMap = nullptr;

    DbClassMapLoadContext classMapLoadContext;
    if (SUCCESS != DbClassMapLoadContext::Load(classMapLoadContext, ctx, m_ecdb, *this, ecClass))
        return ERROR;

    if (!classMapLoadContext.ClassMapExists())
        return SUCCESS; //Class was not yet mapped in a previous import

    MapStrategyExtendedInfo const& mapStrategy = classMapLoadContext.GetMapStrategy();
    std::unique_ptr<ClassMap> classMapPtr;
    if (mapStrategy.GetStrategy() == MapStrategy::NotMapped)
        classMapPtr = ClassMap::Create<NotMappedClassMap>(m_ecdb, *this, ecClass, mapStrategy);
    else
        {
        ECRelationshipClassCP ecRelationshipClass = ecClass.GetRelationshipClassCP();
        if (ecRelationshipClass != nullptr)
            {
            if (MapStrategyExtendedInfo::IsForeignKeyMapping(mapStrategy))
                classMapPtr = ClassMap::Create<RelationshipClassEndTableMap>(m_ecdb, *this, *ecRelationshipClass, mapStrategy);
            else
                classMapPtr = ClassMap::Create<RelationshipClassLinkTableMap>(m_ecdb, *this, *ecRelationshipClass, mapStrategy);
            }
        else
            classMapPtr = ClassMap::Create<ClassMap>(m_ecdb, *this, ecClass, mapStrategy);
        }

    ClassMap* classMapP = AddClassMap(std::move(classMapPtr));
    if (classMapP == nullptr)
        return ERROR;

    if (SUCCESS != classMapP->Load(ctx, classMapLoadContext))
        return ERROR;

    classMap = classMapP;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ClassMap* TableSpaceSchemaManager::AddClassMap(std::unique_ptr<ClassMap> classMap) const
    {
    ECClassId id = classMap->GetClass().GetId();
    auto it = m_classMapDictionary.find(id);
    if (m_classMapDictionary.end() != it)
        {
        BeAssert(false && "Attempted to add a second ClassMap for the same ECClass");
        return nullptr;
        }

    ClassMap* mapP = classMap.get();
    m_classMapDictionary[id] = std::move(classMap);
    return mapP;
    }

//*****************************************************************
//MainSchemaManager
//*****************************************************************

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   11/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus MainSchemaManager::ImportSchemas(bvector<ECN::ECSchemaCP> const& schemas, SchemaManager::SchemaImportOptions options, SchemaImportToken const* token) const
    {
    PERFLOG_START("ECDb", "Schema import");
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin SchemaManager::ImportSchemas");
    SchemaImportContext ctx(m_ecdb, options);
    const BentleyStatus stat = ImportSchemas(ctx, schemas, token);
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("End SchemaManager::ImportSchemas");
    m_ecdb.ClearECDbCache();
    m_ecdb.FireAfterSchemaImportEvent();
    PERFLOG_FINISH("ECDb", "Schema import");
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                     06/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus MainSchemaManager::ImportSchemas(SchemaImportContext& ctx, bvector<ECSchemaCP> const& schemas, SchemaImportToken const* schemaImportToken) const
    {
    if (ctx.GetOptions() == SchemaManager::SchemaImportOptions::Poisoning)
        {
        LOG.error("Failed to import ECSchemas. SchemaImportOption::Poisoning is not supported.");
        return ERROR;
        }

    Policy policy = PolicyManager::GetPolicy(SchemaImportPermissionPolicyAssertion(m_ecdb, schemaImportToken));
    if (!policy.IsSupported())
        {
        LOG.error("Failed to import ECSchemas: Caller has not provided a SchemaImportToken.");
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

    ProfileVersion profileVersion(0, 0, 0, 0);
    if (BE_SQLITE_OK != ProfileManager::ReadProfileVersion(profileVersion, m_ecdb) || ProfileManager::GetExpectedVersion() != profileVersion)
        {
        m_ecdb.GetImpl().Issues().Report("Failed to import ECSchemas. The ECDb file is too new (version: %s). Schema imports with this version of the software can "
                                         "only be done on files with profile version %s.",
                                         profileVersion.ToString().c_str(), ProfileManager::GetExpectedVersion().ToString().c_str());
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

    if (SUCCESS != ViewGenerator::DropECClassViews(m_ecdb))
        return ERROR;

    return MapSchemas(ctx, schemasToMap);
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                   Affan.Khan        29/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MainSchemaManager::PersistSchemas(SchemaImportContext& context, bvector<ECN::ECSchemaCP>& schemasToMap, bvector<ECSchemaCP> const& schemas) const
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
            ECSchemaId id = SchemaPersistenceHelper::GetSchemaId(m_ecdb, GetTableSpace(), schema->GetName().c_str(), SchemaLookupMode::ByName);
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



//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    04/2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus MainSchemaManager::MapSchemas(SchemaImportContext& ctx, bvector<ECN::ECSchemaCP> const& schemas) const
    {
    if (schemas.empty())
        return SUCCESS;

    PERFLOG_START("ECDb", "Schema import> Map schemas");

    if (SUCCESS != DoMapSchemas(ctx, schemas))
        return ERROR;

    PERFLOG_START("ECDb", "Schema import> Persist mappings");
    if (SUCCESS != SaveDbSchema(ctx))
        {
        ClearCache();
        return ERROR;
        }
    PERFLOG_FINISH("ECDb", "Schema import> Persist mappings");

    PERFLOG_START("ECDb", "Schema import> Create or update tables");
    if (SUCCESS != CreateOrUpdateRequiredTables())
        {
        ClearCache();
        return ERROR;
        }
    PERFLOG_FINISH("ECDb", "Schema import> Create or update tables");
    PERFLOG_START("ECDb", "Schema import> Create or update indexes");
    if (SUCCESS != CreateOrUpdateIndexesInDb(ctx))
        {
        ClearCache();
        return ERROR;
        }
    PERFLOG_FINISH("ECDb", "Schema import> Create or update indexes");

    PERFLOG_START("ECDb", "Schema import> Purge orphan tables");
    if (SUCCESS != PurgeOrphanTables())
        {
        ClearCache();
        return ERROR;
        }
    PERFLOG_FINISH("ECDb", "Schema import> Purge orphan tables");

    PERFLOG_START("ECDb", "Schema import> Validate mappings");

    if (SUCCESS != DbMapValidator(ctx).Validate())
        return ERROR;

    PERFLOG_FINISH("ECDb", "Schema import> Validate mappings");

    ClearCache();
    PERFLOG_FINISH("ECDb", "Schema import> Map schemas");
    return SUCCESS;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan         09/2012
//---------------------------------------------------------------------------------------
BentleyStatus MainSchemaManager::DoMapSchemas(SchemaImportContext& ctx, bvector<ECN::ECSchemaCP> const& schemas) const
    {
    ctx.SetPhase(SchemaImportContext::Phase::MappingSchemas);

    // Identify root classes/relationship-classes
    std::set<ECClassCP> doneList;
    std::set<ECClassCP> rootClassSet;
    std::vector<ECClassCP> rootClassList;
    std::vector<ECN::ECEntityClassCP> rootMixins;
    std::vector<ECRelationshipClassCP> rootRelationshipList;

    for (ECSchemaCP schema : schemas)
        {
        if (schema->IsSupplementalSchema())
            continue; // Don't map any supplemental schemas

        for (ECClassCP ecClass : schema->GetClasses())
            {
            if (doneList.find(ecClass) != doneList.end())
                continue;

            GatherRootClasses(*ecClass, doneList, rootClassSet, rootClassList, rootRelationshipList, rootMixins);
            }
        }

    if (GetDbSchemaR().SynchronizeExistingTables() != SUCCESS)
        {
        m_ecdb.GetImpl().Issues().Report("Synchronizing existing table to which classes are mapped failed.");
        return ERROR;
        }

    // Map mixin hierarchy before everything else. It does not map primary hierarchy and all classes map to virtual tables.
    PERFLOG_START("ECDb", "Schema import> Map mixins");
    ctx.SetPhase(SchemaImportContext::Phase::MappingMixins);
    for (ECEntityClassCP mixin : rootMixins)
        {
        if (ClassMappingStatus::Error == MapClass(ctx, *mixin))
            return ERROR;
        }
    PERFLOG_FINISH("ECDb", "Schema import> Map mixins");

    // Starting with the root, recursively map the entire class hierarchy. 
    PERFLOG_START("ECDb", "Schema import> Map entity classes");
    ctx.SetPhase(SchemaImportContext::Phase::MappingEntities);
    for (ECClassCP rootClass : rootClassList)
        {
        if (ClassMappingStatus::Error == MapClass(ctx, *rootClass))
            return ERROR;
        }
    PERFLOG_FINISH("ECDb", "Schema import> Map entity classes");

    PERFLOG_START("ECDb", "Schema import> Map relationships");
    ctx.SetPhase(SchemaImportContext::Phase::MappingRelationships);
    for (ECRelationshipClassCP rootRelationshipClass : rootRelationshipList)
        {
        if (ClassMappingStatus::Error == MapClass(ctx, *rootRelationshipClass))
            return ERROR;
        }

    if (SUCCESS != DbMappingManager::FkRelationships::FinishMapping(ctx))
        return ERROR;

    PERFLOG_FINISH("ECDb", "Schema import> Map relationships");
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   06/12
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus MainSchemaManager::MapClass(SchemaImportContext& ctx, ECClassCR ecClass) const
    {
    ClassMap* existingClassMap = nullptr;
    if (SUCCESS != TryGetClassMap(existingClassMap, ctx.GetClassMapLoadContext(), ecClass))
        return ClassMappingStatus::Error;

    if (existingClassMap == nullptr)
        {
        ClassMappingInfo mappingInfo(ctx, ecClass);
        ClassMappingStatus status = mappingInfo.Initialize();
        if (status == ClassMappingStatus::BaseClassesNotMapped || status == ClassMappingStatus::Error)
            return status;

        return MapClass(ctx, mappingInfo);
        }

    if (SUCCESS != existingClassMap->Update(ctx))
        return ClassMappingStatus::Error;

    return MapDerivedClasses(ctx, ecClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle         10/2017
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus MainSchemaManager::MapClass(SchemaImportContext& ctx, ClassMappingInfo const& mappingInfo) const
    {
    MapStrategyExtendedInfo const& mapStrategy = mappingInfo.GetMapStrategy();
    std::unique_ptr<ClassMap> classMap;
    if (mapStrategy.GetStrategy() == MapStrategy::NotMapped)
        classMap = ClassMap::Create<NotMappedClassMap>(m_ecdb, *this, mappingInfo.GetClass(), mapStrategy);
    else
        {
        ECRelationshipClassCP ecRelationshipClass = mappingInfo.GetClass().GetRelationshipClassCP();
        if (ecRelationshipClass != nullptr)
            {
            if (MapStrategyExtendedInfo::IsForeignKeyMapping(mapStrategy))
                classMap = ClassMap::Create<RelationshipClassEndTableMap>(m_ecdb, *this, *ecRelationshipClass, mapStrategy);
            else
                classMap = ClassMap::Create<RelationshipClassLinkTableMap>(m_ecdb, *this, *ecRelationshipClass, mapStrategy);
            }
        else
            classMap = ClassMap::Create<ClassMap>(m_ecdb, *this, mappingInfo.GetClass(), mapStrategy);
        }

    ClassMap* classMapP = AddClassMap(std::move(classMap));
    if (classMapP == nullptr)
        return ClassMappingStatus::Error;

    ctx.AddClassMapForSaving(mappingInfo.GetClass().GetId());
    ClassMappingStatus status = classMapP->Map(ctx, mappingInfo);
    if (status == ClassMappingStatus::BaseClassesNotMapped || status == ClassMappingStatus::Error)
        return status;

    if (SUCCESS != DbMappingManager::Classes::MapUserDefinedIndexes(ctx, *classMapP))
        return ClassMappingStatus::Error;

    return MapDerivedClasses(ctx, mappingInfo.GetClass());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle         10/2017
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus MainSchemaManager::MapDerivedClasses(SchemaImportContext& ctx, ECN::ECClassCR baseClass) const
    {
    const bool baseClassIsMixin = baseClass.IsEntityClass() && baseClass.GetEntityClassCP()->IsMixin();

    for (ECClassCP derivedClass : baseClass.GetDerivedClasses())
        {
        const bool derivedIsMixin = derivedClass->IsEntityClass() && derivedClass->GetEntityClassCP()->IsMixin();
        //Only map mixin hierarchy but stop if you find a non-mixin class.
        if (baseClassIsMixin && !derivedIsMixin)
            continue;

        if (ClassMappingStatus::Error == MapClass(ctx, *derivedClass))
            return ClassMappingStatus::Error;
        }

    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      12/2011
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus MainSchemaManager::CreateOrUpdateRequiredTables() const
    {
    m_ecdb.GetStatementCache().Empty();

    int nCreated = 0;
    int nUpdated = 0;
    int nWasUpToDate = 0;

    for (DbTable const* table : GetDbSchema().Tables().GetTablesInDependencyOrder())
        {
        const DbSchemaPersistenceManager::CreateOrUpdateTableResult result = DbSchemaPersistenceManager::CreateOrUpdateTable(m_ecdb, *table);
        switch (result)
            {
                case DbSchemaPersistenceManager::CreateOrUpdateTableResult::Created:
                    nCreated++;
                    break;

                case DbSchemaPersistenceManager::CreateOrUpdateTableResult::Updated:
                    nUpdated++;
                    break;

                case DbSchemaPersistenceManager::CreateOrUpdateTableResult::WasUpToDate:
                    nWasUpToDate++;
                    break;

                case DbSchemaPersistenceManager::CreateOrUpdateTableResult::Error:
                    return ERROR;

                default:
                case DbSchemaPersistenceManager::CreateOrUpdateTableResult::Skipped:
                    continue;
            }
        }

    LOG.debugv("Schema Import> Created %d tables, updated %d tables, and %d tables were up-to-date.", nCreated, nUpdated, nWasUpToDate);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2016
//---------------------------------------------------------------------------------------
BentleyStatus MainSchemaManager::CreateOrUpdateIndexesInDb(SchemaImportContext& ctx) const
    {
    if (SUCCESS != m_dbSchema.LoadIndexDefs())
        return ERROR;

    if (BE_SQLITE_OK != m_ecdb.ExecuteSql("DELETE FROM main." TABLE_Index))
        return ERROR;

    bmap<Utf8String, DbIndex const*, CompareIUtf8Ascii> comparableIndexDefs;
    bset<Utf8CP, CompareIUtf8Ascii> usedIndexNames;

    //cache all indexes in local vector as index processing may modify the m_dbSchemas.Tables() iterator
    std::vector<DbIndex const*> indexes;
    for (DbTable const* table : m_dbSchema.Tables())
        {
        for (std::unique_ptr<DbIndex> const& indexPtr : table->GetIndexes())
            {
            if (indexPtr->GetColumns().empty())
                {
                BeAssert(false && "Index definition is not valid");
                return ERROR;
                }

            indexes.push_back(indexPtr.get());
            }
        }

    for (DbIndex const* indexCP : indexes)
        {
        DbIndex const& index = *indexCP;

        {
        //drop index first if it exists, as we always have to recreate them to make sure the class id filter is up-to-date
        Utf8String dropIndexSql;
        dropIndexSql.Sprintf("DROP INDEX [%s]", index.GetName().c_str());
        m_ecdb.TryExecuteSql(dropIndexSql.c_str());
        }

        if (!index.IsAutoGenerated() && index.HasClassId())
            {
            ECClassCP ecClass = m_ecdb.Schemas().GetClass(index.GetClassId());
            if (ecClass == nullptr)
                {
                BeAssert(false);
                return ERROR;
                }

            ClassMap const* classMap = nullptr;
            if (SUCCESS != TryGetClassMap(classMap, ctx.GetClassMapLoadContext(), *ecClass))
                {
                BeAssert(false);
                return ERROR;
                }

            StorageDescription const& storageDesc = classMap->GetStorageDescription();
            if (storageDesc.HasMultipleNonVirtualHorizontalPartitions())
                {
                Issues().Report("Failed to map ECClass '%s'. The index '%s' defined on it spans multiple tables which is not supported. Consider applying the 'TablePerHierarchy' strategy to the ECClass.",
                                ecClass->GetFullName(), index.GetName().c_str());
                return ERROR;
                }
            }

        if (usedIndexNames.find(index.GetName().c_str()) != usedIndexNames.end())
            {
            Issues().Report("Failed to create index %s on table %s. An index with the same name already exists.", index.GetName().c_str(), index.GetTable().GetName().c_str());
            return ERROR;
            }
        else
            usedIndexNames.insert(index.GetName().c_str());

        //indexes on virtual tables are ignored
        if (index.GetTable().GetType() != DbTable::Type::Virtual)
            {
            Utf8String ddl, comparableIndexDef;
            if (SUCCESS != DbSchemaPersistenceManager::BuildCreateIndexDdl(ddl, comparableIndexDef, m_ecdb, index))
                return ERROR;

            auto it = comparableIndexDefs.find(comparableIndexDef);
            if (it != comparableIndexDefs.end())
                {
                Utf8CP errorMessage = "Index '%s'%s on table '%s' has the same definition as the already existing index '%s'%s. ECDb does not create this index.";

                Utf8String provenanceStr;
                if (index.HasClassId())
                    {
                    ECClassCP provenanceClass = m_ecdb.Schemas().GetClass(index.GetClassId());
                    if (provenanceClass == nullptr)
                        {
                        BeAssert(false);
                        return ERROR;
                        }
                    provenanceStr.Sprintf(" [Created for ECClass %s]", provenanceClass->GetFullName());
                    }

                DbIndex const* existingIndex = it->second;
                Utf8String existingIndexProvenanceStr;
                if (existingIndex->HasClassId())
                    {
                    ECClassCP provenanceClass = m_ecdb.Schemas().GetClass(existingIndex->GetClassId());
                    if (provenanceClass == nullptr)
                        {
                        BeAssert(false);
                        return ERROR;
                        }
                    existingIndexProvenanceStr.Sprintf(" [Created for ECClass %s]", provenanceClass->GetFullName());
                    }

                if (!index.IsAutoGenerated())
                    LOG.warningv(errorMessage, index.GetName().c_str(), provenanceStr.c_str(), index.GetTable().GetName().c_str(),
                                 existingIndex->GetName().c_str(), existingIndexProvenanceStr.c_str());
                else
                    {
                    if (LOG.isSeverityEnabled(NativeLogging::LOG_DEBUG))
                        LOG.debugv(errorMessage,
                                   index.GetName().c_str(), provenanceStr.c_str(), index.GetTable().GetName().c_str(),
                                   existingIndex->GetName().c_str(), existingIndexProvenanceStr.c_str());
                    }

                continue;
                }

            comparableIndexDefs[comparableIndexDef] = &index;

            if (SUCCESS != DbSchemaPersistenceManager::CreateIndex(m_ecdb, index, ddl))
                return ERROR;
            }

        //populates the ec_Index table (even for indexes on virtual tables, as they might be necessary
        //if further schema imports introduce subclasses of abstract classes (which map to virtual tables))
        if (SUCCESS != m_dbSchema.PersistIndexDef(index))
            return ERROR;
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2016
//---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus MainSchemaManager::PurgeOrphanTables() const
    {
    //skip ExistingTable and NotMapped
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, "SELECT t.Id, t.Name, t.Type= " SQLVAL_DbTable_Type_Virtual " FROM main.ec_Table t "
                                     "WHERE t.Type NOT IN (" SQLVAL_DbTable_Type_Existing ") AND t.Name<>'" DBSCHEMA_NULLTABLENAME "' AND t.Id NOT IN ("
                                     "SELECT DISTINCT ec_Table.Id FROM main.ec_PropertyMap "
                                     "INNER JOIN main.ec_PropertyPath ON ec_PropertyPath.Id = ec_PropertyMap.PropertyPathId "
                                     "INNER JOIN main.ec_Property ON ec_PropertyPath.RootPropertyId = ec_Property.Id "
                                     "INNER JOIN main.ec_Column ON ec_PropertyMap.ColumnId = ec_Column.Id "
                                     "INNER JOIN main.ec_Table ON ec_Column.TableId = ec_Table.Id)"))
        {
        BeAssert(false && "ECDb profile changed");
        return ERROR;
        }

    IdSet<DbTableId> orphanTables;
    std::vector<Utf8String> tablesToDrop;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        orphanTables.insert(stmt.GetValueId<DbTableId>(0));
        if (!stmt.GetValueBoolean(2))
            tablesToDrop.push_back(stmt.GetValueText(1));
        }
    stmt.Finalize();

    if (orphanTables.empty())
        return SUCCESS;

    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, "DELETE FROM main.ec_Table WHERE InVirtualSet(?,Id)") ||
        BE_SQLITE_OK != stmt.BindVirtualSet(1, orphanTables) ||
        BE_SQLITE_DONE != stmt.Step())
        {
        BeAssert(false);
        return ERROR;
        }

    for (Utf8StringCR tableName : tablesToDrop)
        {
        GetDbSchema().Tables().Remove(tableName);
        }

    stmt.Finalize();

    if (tablesToDrop.empty())
        return SUCCESS;

    if (!m_ecdb.GetECDbSettings().AllowChangesetMergingIncompatibleSchemaImport())
        {
        Utf8String tableNames;
        bool isFirstTable = true;
        for (Utf8StringCR tableName : tablesToDrop)
            {
            if (!isFirstTable)
                tableNames.append(",");

            tableNames.append(tableName);
            isFirstTable = false;
            }

        m_ecdb.GetImpl().Issues().Report(
            "Failed to import schemas: it would change the database schema in a changeset-merging incompatible way. ECDb would have to delete these tables: %s", tableNames.c_str());
        return ERROR;
        }

    for (Utf8StringCR name : tablesToDrop)
        {
        if (m_ecdb.DropTable(name.c_str()) != BE_SQLITE_OK)
            {
            BeAssert(false && "failed to drop a table");
            return ERROR;
            }

        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// Gets the count of tables at the specified end of a relationship class.
// @param  relationshpEnd [in] Constraint at the end of the relationship
// @return Number of tables at the specified end of the relationship. 
// @bsimethod                                 Ramanujam.Raman                05/2012
//---------------------------------------------------------------------------------------
size_t MainSchemaManager::GetRelationshipConstraintTableCount(SchemaImportContext& ctx, ECRelationshipConstraintCR constraint) const
    {
    std::set<ClassMap const*> classMaps = GetRelationshipConstraintClassMaps(ctx, constraint);
    const bool abstractEndPoint = constraint.GetConstraintClasses().size() == 1 && constraint.GetConstraintClasses().front()->GetClassModifier() == ECClassModifier::Abstract;

    std::set<DbTable const*> nonVirtualTables;
    bool hasAtLeastOneVirtualTable = false;
    for (ClassMap const* classMap : classMaps)
        {
        DbTable const* table = abstractEndPoint ? &classMap->GetJoinedOrPrimaryTable() : &classMap->GetPrimaryTable();
        if (classMap->GetPrimaryTable().GetType() == DbTable::Type::Virtual)
            hasAtLeastOneVirtualTable = true;
        else
            nonVirtualTables.insert(table);
        }

    if (!nonVirtualTables.empty())
        return nonVirtualTables.size();

    return hasAtLeastOneVirtualTable ? 1 : 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2015
//+---------------+---------------+---------------+---------------+---------------+------
std::set<DbTable const*> MainSchemaManager::GetRelationshipConstraintPrimaryTables(SchemaImportContext& ctx, ECRelationshipConstraintCR constraint) const
    {
    //WIP_CLEANUP This looks over-complicated. Doing 3 loops to get the final result. E.g. why can't virtual tables be ignored right away?
    std::set<ClassMap const*> classMaps = GetRelationshipConstraintClassMaps(ctx, constraint);

    std::map<DbTable const*, std::set<DbTable const*>> joinedTablesPerPrimaryTable;
    std::set<DbTable const*> tables;
    for (ClassMap const* classMap : classMaps)
        {
        std::vector<DbTable const*> nonOverflowClassMapTables;
        for (DbTable const* table : classMap->GetTables())
            {
            if (table->GetType() != DbTable::Type::Overflow)
                nonOverflowClassMapTables.push_back(table);
            }

        if (nonOverflowClassMapTables.size() == 1)
            {
            tables.insert(nonOverflowClassMapTables[0]);
            continue;
            }

        for (DbTable const* table : nonOverflowClassMapTables)
            {
            if (table->GetType() == DbTable::Type::Joined)
                {
                DbTable::LinkNode const* primaryTable = table->GetLinkNode().GetParent();
                BeAssert(primaryTable != nullptr);

                joinedTablesPerPrimaryTable[&primaryTable->GetTable()].insert(table);
                tables.insert(table);
                }
            }
        }

    for (std::pair<DbTable const*, std::set<DbTable const*>> const& pair : joinedTablesPerPrimaryTable)
        {
        DbTable const* primaryTable = pair.first;
        for (DbTable::LinkNode const* nextTableNode : primaryTable->GetLinkNode().GetChildren())
            tables.erase(&nextTableNode->GetTable());

        tables.insert(primaryTable);
        continue;
        }

    std::set<DbTable const*> finalSetOfTables;
    for (DbTable const* table : tables)
        {
        if (table->GetType() != DbTable::Type::Virtual)
            finalSetOfTables.insert(table);
        }

    return finalSetOfTables;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2015
//+---------------+---------------+---------------+---------------+---------------+------
std::set<ClassMap const*> MainSchemaManager::GetRelationshipConstraintClassMaps(SchemaImportContext& ctx, ECRelationshipConstraintCR constraint) const
    {
    std::set<ClassMap const*> classMaps;
    for (ECClassCP ecClass : constraint.GetConstraintClasses())
        {
        ClassMap const* classMap = nullptr;
        if (SUCCESS != TryGetClassMap(classMap, ctx.GetClassMapLoadContext(), *ecClass))
            {
            BeAssert(false);
            classMaps.clear();
            return classMaps;
            }

        const bool recursive = !classMap->GetMapStrategy().IsTablePerHierarchy() && constraint.GetIsPolymorphic();
        if (SUCCESS != GetRelationshipConstraintClassMaps(ctx, classMaps, *ecClass, recursive))
            {
            BeAssert(false);
            classMaps.clear();
            return classMaps;
            }
        }

    return classMaps;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus MainSchemaManager::GetRelationshipConstraintClassMaps(SchemaImportContext& ctx, std::set<ClassMap const*>& classMaps, ECClassCR ecClass, bool recursive) const
    {
    ClassMap const* classMap = nullptr;
    if (SUCCESS != TryGetClassMap(classMap, ctx.GetClassMapLoadContext(), ecClass))
        {
        BeAssert(classMap != nullptr && "ClassMap should not be null");
        return ERROR;
        }

    if (classMap->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        return SUCCESS;

    classMaps.insert(classMap);

    if (!recursive)
        return SUCCESS;

    for (ECClassCP subclass : m_ecdb.Schemas().GetDerivedClasses(ecClass))
        {
        if (SUCCESS != GetRelationshipConstraintClassMaps(ctx, classMaps, *subclass, recursive))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus MainSchemaManager::SaveDbSchema(SchemaImportContext& ctx) const
    {
    if (m_dbSchema.SaveOrUpdateTables() != SUCCESS)
        {
        BeAssert(false);
        return ERROR;
        }

    DbMapSaveContext saveCtx(m_ecdb);
    for (auto& kvPair : m_classMapDictionary)
        {
        ClassMap& classMap = *kvPair.second;
        if (classMap.GetState() == ObjectState::Persisted)
            continue;

        if (SUCCESS != classMap.Save(ctx, saveCtx))
            {
            Issues().Report("Failed to save mapping for ECClass %s: %s", classMap.GetClass().GetFullName(), m_ecdb.GetLastError().c_str());
            return ERROR;
            }
        }

    if (SUCCESS != DbSchemaPersistenceManager::RepopulateClassHasTableCacheTable(m_ecdb))
        return ERROR;

    m_lightweightCache.Clear();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan   12/2015
//---------------------------------------------------------------------------------------
BentleyStatus MainSchemaManager::CreateClassViews() const
    {
    BeMutexHolder lock(m_mutex);
    return ViewGenerator::CreateECClassViews(m_ecdb);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle   12/2016
//---------------------------------------------------------------------------------------
BentleyStatus MainSchemaManager::CreateClassViews(bvector<ECN::ECClassId> const& ecclassids) const
    {
    BeMutexHolder lock(m_mutex);
    return ViewGenerator::CreateECClassViews(m_ecdb, ecclassids);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle   04/2017
//---------------------------------------------------------------------------------------
BentleyStatus MainSchemaManager::RepopulateCacheTables() const
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
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
//static
bvector<ECN::ECSchemaCP> MainSchemaManager::FindAllSchemasInGraph(bvector<ECN::ECSchemaCP> const& schemas)
    {
    bmap<ECN::SchemaKey, ECN::ECSchemaCP, SchemaKeyLessThan<SchemaMatchType::Exact>> map;
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
//static
bmap<ECN::SchemaKey, ECN::ECSchemaCP, SchemaKeyLessThan<SchemaMatchType::Exact>> MainSchemaManager::FindAllSchemasInGraph(ECN::ECSchemaCR schema, bool includeThisSchema)
    {
    bmap<ECN::SchemaKey, ECN::ECSchemaCP, SchemaKeyLessThan<SchemaMatchType::Exact>> schemaMap;
    if (includeThisSchema)
        schemaMap[schema.GetSchemaKey()] = &schema;

    for (const auto& entry : schema.GetReferencedSchemas())
        FindAllSchemasInGraph(schemaMap, entry.second.get());

    return schemaMap;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void MainSchemaManager::FindAllSchemasInGraph(bmap<ECN::SchemaKey, ECN::ECSchemaCP, SchemaKeyLessThan<SchemaMatchType::Exact>>& schemaMap, ECN::ECSchemaCP schema)
    {
    if (schemaMap.find(schema->GetSchemaKey()) != schemaMap.end())
        return;

    schemaMap[schema->GetSchemaKey()] = schema;
    for (const auto& entry : schema->GetReferencedSchemas())
        FindAllSchemasInGraph(schemaMap, entry.second.get());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
//static
bvector<ECN::ECSchemaCP> MainSchemaManager::Sort(bvector<ECN::ECSchemaCP> const& schemas)
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

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
//static
bvector<ECN::ECSchemaCP> MainSchemaManager::GetNextLayer(bvector<ECN::ECSchemaCP> const& schemas, bvector<ECN::ECSchemaCP> const& referencedBy)
    {
    bvector<ECN::ECSchemaCP> list;
    bmap<ECN::SchemaKey, ECN::ECSchemaCP, SchemaKeyLessThan<SchemaMatchType::Exact>> map;
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan         03/2016
//---------------------------------------------------------------------------------------
//static
void MainSchemaManager::GatherRootClasses(ECClassCR ecclass, std::set<ECClassCP>& doneList, std::set<ECClassCP>& rootClassSet, std::vector<ECClassCP>& rootClassList, std::vector<ECRelationshipClassCP>& rootRelationshipList, std::vector<ECN::ECEntityClassCP>& rootMixins)
    {
    if (doneList.find(&ecclass) != doneList.end())
        return;

    doneList.insert(&ecclass);
    if (!ecclass.HasBaseClasses())
        {
        ECEntityClassCP entityClass = ecclass.IsEntityClass() ? ecclass.GetEntityClassCP() : nullptr;
        if (rootClassSet.find(&ecclass) == rootClassSet.end())
            {
            rootClassSet.insert(&ecclass);
            if (ecclass.IsRelationshipClass())
                rootRelationshipList.push_back(ecclass.GetRelationshipClassCP());
            else
                {
                if (entityClass && entityClass->IsMixin())
                    rootMixins.push_back(entityClass);
                else
                    rootClassList.push_back(&ecclass);
                }
            }

        return;
        }

    //If all baseClasses are mixin then consider the class as root class.
    bool noMixin = false;
    for (ECClassCP baseClass : ecclass.GetBaseClasses())
        {
        if (ECEntityClassCP entityClass = baseClass->GetEntityClassCP())
            {
            if (entityClass->IsMixin())
                continue;
            }

        noMixin = true;
        break;
        }

    if (!noMixin)
        {
        rootClassList.push_back(&ecclass);
        return;
        }

    for (ECClassCP baseClass : ecclass.GetBaseClasses())
        {
        if (baseClass == nullptr)
            continue;

        if (doneList.find(baseClass) != doneList.end())
            return;

        GatherRootClasses(*baseClass, doneList, rootClassSet, rootClassList, rootRelationshipList, rootMixins);
        }
    }



END_BENTLEY_SQLITE_EC_NAMESPACE