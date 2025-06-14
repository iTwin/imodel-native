/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

Utf8String ECSchemaOwnershipClaimAppData::s_key = "ecdb.owned_by";

//*****************************************************************
//VirtualSchemaManager
//*****************************************************************
/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr VirtualSchemaManager::_LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) {
    return m_cache->LocateSchema(key, matchType, schemaContext);
}

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus VirtualSchemaManager::AddAndValidateVirtualSchema(Utf8StringCR schemaXml, bool validate) const{
   // BeMutexHolder lock(m_ecdb.GetImpl().GetMutex());
    auto readerContext = ECSchemaReadContext::CreateContext();
    readerContext->AddSchemaLocater(const_cast<VirtualSchemaManager&>(*this));
    ECSchemaPtr schema;
    if (ECN::ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *readerContext) != SchemaReadStatus::Success) {
        return ERROR;
    }
    if (validate) {
        Utf8String err;
        if (IsValidVirtualSchema(*schema, err)) {
            // log err
            return ERROR;
        }
    }
    SetVirtualTypeIds(*schema);
    // schema.SetImmutable(true);
    m_cache->AddSchema(*schema);
    m_schemas[schema->GetName()] = schema.get();
    return SUCCESS;
}

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t VirtualSchemaManager::GetNextId() const{
    return m_idSeq++;
}

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void VirtualSchemaManager::SetVirtualTypeIds (ECN::ECSchemaR schema) const{
    schema.SetId(ECSchemaId(GetNextId()));
    for (auto& schemaClass : schema.GetClasses()) {
        const_cast<ECN::ECClassP>(schemaClass)->SetId(ECClassId(GetNextId()));
        for(auto& classProp : schemaClass->GetProperties(false)) {
            const_cast<ECN::ECPropertyP>(classProp)->SetId(ECPropertyId(GetNextId()));
        }
    }
}

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void VirtualSchemaManager::AddECDbVirtualSchema() const{
    auto schemaXml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema
                schemaName="ECDbVirtual"
                alias="ecdbvir"
                version="1.0.0"
                xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECCustomAttributeClass typeName="VirtualType" modifier="Sealed" appliesTo="EntityClass"/>
            <ECCustomAttributeClass typeName="VirtualSchema" modifier="Sealed" appliesTo="Schema"/>
            <ECCustomAttributeClass typeName="AnyPrimitiveType" modifier="Sealed" appliesTo="PrimitiveProperty"/>
        </ECSchema>)xml";
    if (AddAndValidateVirtualSchema(schemaXml, false) != SUCCESS) {
        throw std::runtime_error("unable to load ECDbVirtual schema");
    }
}

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void VirtualSchemaManager::AddSystemVirtualSchemas() const{
    auto schemaXml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema
                schemaName="json1"
                alias="json1"
                version="1.0.0"
                xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbVirtual" version="01.00.00" alias="ecdbvir" />
            <ECCustomAttributes>
                <VirtualSchema xmlns="ECDbVirtual.01.00.00"/>
            </ECCustomAttributes>
            <ECEntityClass typeName="json_tree" modifier="Abstract">
                <ECCustomAttributes>
                    <VirtualType xmlns="ECDbVirtual.01.00.00"/>
                </ECCustomAttributes>
                <ECProperty propertyName="key"  typeName="string"/>
                <ECProperty propertyName="value" typeName="string"/>
                <ECProperty propertyName="type" typeName="string"/>
                <ECProperty propertyName="atom" typeName="string"/>
                <ECProperty propertyName="parent" typeName="int"/>
                <ECProperty propertyName="fullkey" typeName="string"/>
                <ECProperty propertyName="path" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="json_each" modifier="Abstract">
                <ECCustomAttributes>
                    <VirtualType xmlns="ECDbVirtual.01.00.00"/>
                </ECCustomAttributes>
                <ECProperty propertyName="key"  typeName="string"/>
                <ECProperty propertyName="value" typeName="string"/>
                <ECProperty propertyName="type" typeName="string"/>
                <ECProperty propertyName="atom" typeName="string"/>
                <ECProperty propertyName="parent" typeName="int"/>
                <ECProperty propertyName="fullkey" typeName="string"/>
                <ECProperty propertyName="path" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml";
    if (Add(schemaXml) != SUCCESS) {
        throw std::runtime_error("unable to load json1 schema");
    }
}

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
VirtualSchemaManager::VirtualSchemaManager(ECDbCR ecdb): m_cache(ECSchemaCache::Create()),m_idSeq(0xde0b6b3a7640000),m_ecdb(ecdb){
    AddECDbVirtualSchema();
    AddSystemVirtualSchemas();
}

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool VirtualSchemaManager::IsValidVirtualSchema(ECN::ECSchemaR schema, Utf8StringR err) const {
    if (!schema.IsDefined("ECDbVirtual", "VirtualSchema")) {
        err = "schema does not have 'ECDbVirtual::VirtualSchema' customattribute";
        return false;
    }
    for(auto& schemaRef : schema.GetReferencedSchemas()) {
        if (!schemaRef.first.GetName().EqualsIAscii("ECDbVirtual")) {
            err = "only 'ECDbVirtual' schema is allowed as reference schema";
            return false;
        }
    }
    for (auto& schemaClass : schema.GetClasses()) {
        if (!schemaClass->IsEntityClass()) {
            err = "only entity classes are allowed in virtual schema";
            return false;
        }
        if (schemaClass->GetClassModifier() != ECClassModifier::Abstract) {
            err = "only abstract entity classes are allowed in virtual schema";
            return false;
        }
        if (schemaClass->IsDefinedLocal("ECDbVirtual", "VirtualType")) {
            err = "entity classes must have ECDbVirtual::VirtualType customattribute";
            return false;
        }
        if (schemaClass->HasBaseClasses()) {
            err = "virtual attributed class should not have any base classes";
            return false;
        }
    }
    err.clear();
    return true;
}

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP VirtualSchemaManager::GetSchema(Utf8StringCR schemaName) const{
    BeMutexHolder lock(m_ecdb.GetImpl().GetMutex());
    auto it = m_schemas.find(schemaName);
    if (it != m_schemas.end()) {
        return it->second;
    }
    return nullptr;
}

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP VirtualSchemaManager::GetClass(Utf8StringCR schemaName, Utf8StringCR className) const{
    BeMutexHolder lock(m_ecdb.GetImpl().GetMutex());
    auto schema = GetSchema(schemaName.c_str());
    if (schema == nullptr) {
        return nullptr;
    }
    return schema->GetClassCP(className.c_str());
}

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP VirtualSchemaManager::FindClass(Utf8StringCR className, size_t& numberOfClasses) const{
    BeMutexHolder lock(m_ecdb.GetImpl().GetMutex());
    std::vector<ECClassCP> v;
    for(auto it = m_schemas.begin(); it != m_schemas.end(); ++it)
    {
        ECClassCP tempClass = GetClass(it->first, className);
        if(tempClass != nullptr)
            v.push_back(tempClass);
    }
    numberOfClasses = v.size();
    if(v.size() == 0)
        return nullptr;
    else if(v.size() == 1)
        return v[0];
    else
        return nullptr;
}

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus VirtualSchemaManager::Add(Utf8StringCR schemaXml) const{
    return AddAndValidateVirtualSchema(schemaXml, true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////

//*****************************************************************
//SchemaManager::Dispatcher
//*****************************************************************

//---------------------------------------------------------------------------------------
//@bsimethod
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
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool SchemaManager::Dispatcher::OwnsSchema(ECSchemaCR schema) const {
    return ECSchemaOwnershipClaimAppData::IsOwnedBy(m_ecdb, schema);
}

//---------------------------------------------------------------------------------------
//@bsimethod
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
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool SchemaManager::Dispatcher::ExistsManager(Utf8StringCR tableSpace) const
    {
    BeMutexHolder lock(m_mutex);
    return m_managers.find(tableSpace) != m_managers.end();
    }

//---------------------------------------------------------------------------------------
//@bsimethod
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
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaManager::Dispatcher::RemoveManager(DbTableSpace const& tableSpace) const
    {
    BeMutexHolder lock(m_mutex);
    if (!tableSpace.IsAttached())
        {
        BeAssert(tableSpace.IsAttached());
        return ERROR;
        }

    auto itManager = m_managers.find(tableSpace.GetName());
    if (itManager == m_managers.end())
        return SUCCESS;

    TableSpaceSchemaManager const& manager = *itManager->second;
    for (auto itOrder = m_orderedManagers.begin(); itOrder != m_orderedManagers.end(); ++itOrder)
        {
        if (&manager == *itOrder)
            {
            m_orderedManagers.erase(itOrder);
            break;
            }
        }

    m_managers.erase(itManager);

    BeAssert(m_managers.size() == m_orderedManagers.size());
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
//@bsimethod
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
//@bsimethod
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
            {
            LOG.debugv("SchemaManager::Dispatcher::LocateSchema - Found schema %s (%s)", schema->GetName().c_str(), schema->GetId().IsValid() ? schema->GetId().ToString().c_str() : "0");
            return schema;
            }
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
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
//@bsimethod
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
//@bsimethod
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
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ClassMapStrategy SchemaManager::Dispatcher::GetClassMapStrategy(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode mode, Utf8CP tableSpace) const {
    ClassMap const *classMap = GetClassMap(schemaNameOrAlias, className, mode, tableSpace);
    if (classMap == nullptr)
        return ClassMapStrategy();

    return ClassMapStrategy(static_cast<ClassMapStrategy::MapStrategy>(classMap->GetMapStrategy().GetStrategy()), classMap->GetClass());
}
//---------------------------------------------------------------------------------------
//@bsimethod
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
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool SchemaManager::Dispatcher::IsSubClassOf(ECN::ECClassId subClassId, ECN::ECClassId parentClassId, Utf8CP tableSpace) {
    CachedStatementPtr stmt;
    if( tableSpace == nullptr || DbTableSpace::IsMain(tableSpace) ) {
        stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("SELECT 1 FROM [main].[ec_cache_ClassHierarchy] WHERE [ClassId] = ? AND [BaseClassId] = ?");
    } else {
        stmt = m_ecdb.GetImpl().GetCachedSqliteStatement(SqlPrintfString("SELECT 1 FROM [%s].[ec_cache_ClassHierarchy] WHERE [ClassId] = ? AND [BaseClassId] = ?", tableSpace));
    }
    if (stmt == nullptr) {
        return false;
    }

    stmt->BindId(1, subClassId);
    stmt->BindId(2, parentClassId);
    return stmt->Step() == BE_SQLITE_ROW;
}

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool SchemaManager::Dispatcher::IsSubClassOf(Utf8StringCR subClassECSqlName, Utf8StringCR parentClassECSqlName, Utf8CP tableSpace) {
    ECClassCP subClass = FindClass(subClassECSqlName, tableSpace);
    ECClassCP parentClass = FindClass(parentClassECSqlName, tableSpace);
    if (subClass == nullptr || parentClass == nullptr) {
        return false;
    }
    return IsSubClassOf(subClass->GetId(), parentClass->GetId(), tableSpace);
}
//---------------------------------------------------------------------------------------
//@bsimethod
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
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECN::ECClassCP SchemaManager::Dispatcher::FindClass(Utf8StringCR className, Utf8CP tableSpace) const {

    Utf8String schemaToken;
    Utf8String classToken;
    for(auto i=0;i<className.length(); ++i) {
        if (className[i]=='.' || className[i]==':') {
            schemaToken = className.substr(0, i);
            classToken = className.substr(i + 1);
            break;
        }
    }
    if (schemaToken.empty() || classToken.empty()) {
        return nullptr;
    }
    return GetClass(schemaToken, classToken, SchemaLookupMode::AutoDetect, tableSpace);
}
//---------------------------------------------------------------------------------------
//@bsimethod
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
//@bsimethod
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
//@bsimethod
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
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECDerivedClassesList const* SchemaManager::Dispatcher::GetDerivedClasses(ECN::ECClassCR baseClass, Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return nullptr;

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        ECDerivedClassesList const* subClasses = manager->GetDerivedClasses(baseClass);
        if (subClasses != nullptr)
            return subClasses;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Nullable<ECN::ECDerivedClassesList> SchemaManager::Dispatcher::GetAllDerivedClasses(ECN::ECClassCR baseClass, Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return nullptr;

    for (const TableSpaceSchemaManager* manager : iterable)
        {
        if (const auto subClasses = manager->GetAllDerivedClasses(baseClass); subClasses.IsValid())
            return subClasses;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
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
//@bsimethod
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
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECUnitCP SchemaManager::Dispatcher::GetUnit(Utf8StringCR schemaNameOrAlias, Utf8StringCR unitName, SchemaLookupMode mode, Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return nullptr;

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        ECUnitCP unit = manager->GetUnit(schemaNameOrAlias, unitName, mode);
        if (unit != nullptr)
            return unit;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECFormatCP SchemaManager::Dispatcher::GetFormat(Utf8StringCR schemaNameOrAlias, Utf8StringCR formatName, SchemaLookupMode mode, Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return nullptr;

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        ECFormatCP format = manager->GetFormat(schemaNameOrAlias, formatName, mode);
        if (format != nullptr)
            return format;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
UnitSystemCP SchemaManager::Dispatcher::GetUnitSystem(Utf8StringCR schemaNameOrAlias, Utf8StringCR systemName, SchemaLookupMode mode, Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return nullptr;

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        UnitSystemCP system = manager->GetUnitSystem(schemaNameOrAlias, systemName, mode);
        if (system != nullptr)
            return system;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PhenomenonCP SchemaManager::Dispatcher::GetPhenomenon(Utf8StringCR schemaNameOrAlias, Utf8StringCR phenName, SchemaLookupMode mode, Utf8CP tableSpace) const
    {
    Iterable iterable = GetIterable(tableSpace);
    if (!iterable.IsValid())
        return nullptr;

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        PhenomenonCP phen = manager->GetPhenomenon(schemaNameOrAlias, phenName, mode);
        if (phen != nullptr)
            return phen;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
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
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaManager::Dispatcher::ClearCache() const
    {
    Iterable iterable = GetIterable(nullptr);
    if (!iterable.IsValid())
        return;

    for (TableSpaceSchemaManager const* manager : iterable)
        {
        manager->ClearCache();
        }

        {
        BeMutexHolder lock(m_mutex);
        m_unsupportedClassIdCache.clear();
        m_unsupportedClassesLoaded = false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool SchemaManager::Dispatcher::IsClassUnsupported(ECClassId classId) const
    {
    if(!m_unsupportedClassesLoaded)
        {
        BeMutexHolder lock(m_mutex);
        m_unsupportedClassesLoaded = true;

        ECClassId useRequiresVersionClassId = GetClassId("ECDbMap", "UseRequiresVersion", SchemaLookupMode::ByName, nullptr);
        if(!useRequiresVersionClassId.IsValid())
            return false; //can drop out here, the CA class wasn't found so the set will be empty

        IdSet<ECClassId> baseUnsupportedClasses;
        //ClassId refers to the ca class to collect entities for
        //boolean parameter specifies whether we are using the UseRequiresVersion CA indirectly (class which has a ca which has the ca), if this is set true, we can assume
        //everything we find is unsupported
        std::function<void(ECClassId,bool)> collectEntitiesThatHaveCA;
        collectEntitiesThatHaveCA = [&baseUnsupportedClasses, &collectEntitiesThatHaveCA, this](ECClassId caClassId,bool indirect) -> void {
            Statement stmt;
            if (stmt.Prepare(m_ecdb, SqlPrintfString("SELECT ContainerId FROM main." TABLE_CustomAttribute " WHERE ContainerType = %d AND ClassId = ?", (int)SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class)) != DbResult::BE_SQLITE_OK)
                return;

            if (stmt.BindId(1, caClassId) != DbResult::BE_SQLITE_OK)
                return;

            while (stmt.Step() == BE_SQLITE_ROW)
                {
                auto classId = stmt.GetValueId<ECClassId>(0);
                ECClassCP classWithCA = GetClass(classId, nullptr);
                if (classWithCA == nullptr)
                    continue;

                if(!indirect)
                    {
                    UseRequiresVersionCustomAttribute ca;
                    if (!ECDbMapCustomAttributeHelper::TryGetUseRequiresVersion(ca, *classWithCA) || !ca.IsValid())
                        continue;

                    Utf8PrintfString classCaption("ECClass %s", classWithCA->GetFullName());
                    if (ca.Verify(m_ecdb.GetImpl().Issues(), classCaption.c_str()) == BentleyStatus::SUCCESS)
                        continue; //step out, our current code passes the CA requirements
                    }

                if (classWithCA->IsEntityClass() || classWithCA->IsRelationshipClass()) //ignoring structs here for now, the CA has no effect on those
                    {
                    baseUnsupportedClasses.insert(classId);
                    continue;
                    }
                else if (classWithCA->IsCustomAttributeClass())
                    { //recurse into this function with the new custom attribute as classId
                    collectEntitiesThatHaveCA(classId, true);
                    }
                }
            };

        collectEntitiesThatHaveCA(useRequiresVersionClassId, false);
        //we first filled a list with all unsupported entities. Now we need to fill them, and their derived classes into m_unsupportedClassIdCache.
        if(!baseUnsupportedClasses.empty())
            {
            Statement expandClassIdsStmt;
            if (expandClassIdsStmt.Prepare(m_ecdb, "SELECT DISTINCT ClassId FROM main." TABLE_ClassHierarchyCache " WHERE InVirtualSet(?, BaseClassId)") != DbResult::BE_SQLITE_OK)
                return false;

            if (expandClassIdsStmt.BindVirtualSet(1, baseUnsupportedClasses) != DbResult::BE_SQLITE_OK)
                return false;

            while (expandClassIdsStmt.Step() == BE_SQLITE_ROW)
                {
                auto classIdInsert = expandClassIdsStmt.GetValueId<ECClassId>(0);
                m_unsupportedClassIdCache.insert(classIdInsert);
                }
            }
        }

    return m_unsupportedClassIdCache.find(classId) != m_unsupportedClassIdCache.end();
    }

//*****************************************************************
//TableSpaceSchemaManager
//*****************************************************************

//---------------------------------------------------------------------------------------
//@bsimethod
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
    ctx.GetCache().AddSchema(*schemaP); // TODO: Adding to cache directly and always returning the schema despite the status code is nonstandard behavior. This should be fixed in the future.
    return schemaP;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECDerivedClassesList const* TableSpaceSchemaManager::GetDerivedClasses(ECN::ECClassCR baseClass) const
    {
    ECClassId id = m_reader.GetClassId(baseClass);
    if (!id.IsValid())
        {
        LOG.errorv("SchemaManager::GetDerivedClasses failed for ECClass %s. The ECClass does not exist.", baseClass.GetFullName());
        return nullptr;
        }

    if (SUCCESS != m_reader.EnsureDerivedClassesExist(id))
        {
        LOG.errorv("SchemaManager::GetDerivedClasses failed for ECClass %s. Its subclasses could not be loaded.", baseClass.GetFullName());
        return nullptr;
        }

    return &baseClass.GetDerivedClasses();
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Nullable<ECDerivedClassesList> TableSpaceSchemaManager::GetAllDerivedClasses(ECN::ECClassCR baseClass) const
    {
    ECClassId id = m_reader.GetClassId(baseClass);
    if (!id.IsValid())
        {
        LOG.errorv("SchemaManager::GetAllDerivedClasses failed for ECClass %s. The ECClass does not exist.", baseClass.GetFullName());
        return nullptr;
        }

    if (SUCCESS != m_reader.EnsureDerivedClassesExist(id))
        {
        LOG.errorv("SchemaManager::GetAllDerivedClasses failed for ECClass %s. Its subclasses could not be loaded.", baseClass.GetFullName());
        return nullptr;
        }

    return Nullable<ECDerivedClassesList>(m_reader.GetAllDerivedClasses(id));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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

    BeMutexHolder ecdbMutex(GetECDb().GetImpl().GetMutex());
    classMap = nullptr;
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
//* @bsimethod
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
* @bsimethod
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
/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DropSchemaResult MainSchemaManager::DropSchema(Utf8StringCR name, SchemaImportToken const* schemaImportToken, bool logIssue) const {
    return DropSchemas({ name }, schemaImportToken, logIssue);
}

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DropSchemaResult MainSchemaManager::DropSchemas(bvector<Utf8String> schemaNames, SchemaImportToken const* schemaImportToken, bool logIssue) const
    {
    ECDB_PERF_LOG_SCOPE("Drop multiple schemas");
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin SchemaManager::DropSchemas");
    OnBeforeSchemaChanges().RaiseEvent(m_ecdb, SchemaChangeType::SchemaImport);
    SchemaImportContext ctx(m_ecdb, SchemaManager::SchemaImportOptions());

    if (const auto policy = PolicyManager::GetPolicy(SchemaImportPermissionPolicyAssertion(m_ecdb, schemaImportToken)); !policy.IsSupported())
        {
        LOG.error("Failed to drop ECSchemas: Caller has not provided a SchemaImportToken.");
        return DropSchemaResult(DropSchemaResult::Status::Error);
        }
    if (m_ecdb.IsReadonly())
        {
        m_ecdb.GetImpl().Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0278, "Failed to drop ECSchemas. ECDb file is read-only.");
        return DropSchemaResult(DropSchemaResult::Status::Error);
        }

    if (m_ecdb.GetECDbProfileVersion().CompareTo(ECDb::CurrentECDbProfileVersion(), ProfileVersion::VERSION_MajorMinorSub1) > 0)
        {
        m_ecdb.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0280,
            "Failed to drop ECSchemas. Cannot drop schemas from a file which was created with a higher version of this softwares. The file's version, however, is %s.",
            ECDb::CurrentECDbProfileVersion().ToString().c_str(),
            m_ecdb.GetECDbProfileVersion().ToString().c_str());
        return DropSchemaResult(DropSchemaResult::Status::Error);
        }

    BeMutexHolder lock(m_mutex);
    if (auto rc =  SchemaWriter::DropSchemas(schemaNames, ctx, logIssue); rc.IsError())
        return rc;

    if (SUCCESS != ViewGenerator::DropECClassViews(m_ecdb))
        return DropSchemaResult(DropSchemaResult::Status::Error);

    if (SUCCESS != CreateOrUpdateIndexesInDb(ctx))
        {
        ClearCache();
        return DropSchemaResult(DropSchemaResult::Status::Error);
        }

    if (SUCCESS != PurgeOrphanTables(ctx))
        {
        ClearCache();
        return DropSchemaResult(DropSchemaResult::Status::Error);
        }

    m_ecdb.ClearECDbCache();
    if (SUCCESS != DbMapValidator(ctx).Validate())
        {
        ClearCache();
        return DropSchemaResult(DropSchemaResult::Status::Error);
        }

    m_ecdb.ClearECDbCache();
    OnAfterSchemaChanges().RaiseEvent(m_ecdb, SchemaChangeType::SchemaImport);
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("End SchemaManager::DropSchemas");
    return DropSchemaResult::Success;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaImportResult MainSchemaManager::ImportSchemas(bvector<ECN::ECSchemaCP> const& schemas, SchemaManager::SchemaImportOptions options, SchemaImportToken const* token, SchemaSync::SyncDbUri schemaSync) const
    {
    ECDB_PERF_LOG_SCOPE("Schema import");
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("Begin SchemaManager::ImportSchemas");
    OnBeforeSchemaChanges().RaiseEvent(m_ecdb, SchemaChangeType::SchemaImport);
    SchemaImportContext ctx(m_ecdb, options);
    const SchemaImportResult stat = ImportSchemas(ctx, schemas, token, schemaSync);
    ResetIds(schemas);
    m_ecdb.ClearECDbCache();
    OnAfterSchemaChanges().RaiseEvent(m_ecdb, SchemaChangeType::SchemaImport);
    STATEMENT_DIAGNOSTICS_LOGCOMMENT("End SchemaManager::ImportSchemas");
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void MainSchemaManager::ResetIds(bvector<ECN::ECSchemaCP> const& schemas) const {
    // We remove temprory information required by ecdb on schema
    auto cache = ECN::ECSchemaCache::Create();
    for (auto schema: schemas)
        cache->AddSchema(*const_cast<ECN::ECSchemaP>(schema));

    for (auto schema: cache->GetSchemas()) {
        if (ECSchemaOwnershipClaimAppData::HasOwnershipClaim(*schema) && !ECSchemaOwnershipClaimAppData::IsOwnedBy(GetECDb(), *schema))
            continue;
        const_cast<ECSchemaP>(schema)->ResetId();
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
VirtualSchemaManager const& MainSchemaManager::GetVirtualSchemaManager() const {
    return m_vsm;
}

//#define ALLOW_ECDB_SCHEMAIMPORT_DUMP
#if defined(ALLOW_ECDB_SCHEMAIMPORT_DUMP)
void DumpSchemasToFile(BeFileName const& parentDirectory, bvector<ECSchemaCP> const& schemas, Utf8CP suffix)
    {
    BeFileName directory(parentDirectory);
    uint64_t ticks = BeTimeUtilities::QueryMillisecondsCounter();
    directory.AppendUtf8(Utf8PrintfString("%" PRIu64 "_%s\\", ticks, suffix).c_str());
	BeFileName::CreateNewDirectory(directory.c_str()); // create the directory
    BeFileName::EmptyDirectory(directory.c_str()); // clear the directory
    for (auto schema: schemas)
        {
        if(schema == nullptr)
            continue;
        BeFileName fileName(directory);
        fileName.AppendUtf8(schema->GetName().c_str());
        fileName.append(L".ecschema.xml");
        schema->WriteToXmlFile(fileName.c_str());
        }
    };
#endif

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaImportResult MainSchemaManager::ImportSchemas(SchemaImportContext& ctx, bvector<ECSchemaCP> const& schemas, SchemaImportToken const* schemaImportToken, SchemaSync::SyncDbUri syncDbUri) const
    {
    #if defined(ALLOW_ECDB_SCHEMAIMPORT_DUMP)
    /*
    In Debug builds, the environment variable can be set to a directory path to
    dump existing and incoming schemas to for every ImportSchemas call.
    */
    Utf8CP envVarName = "ECDB_SCHEMAIMPORT_DUMP_TO";
    size_t requiredSize;
    if (getenv_s(&requiredSize, NULL, 0, envVarName) == 0 && requiredSize != 0)
        {
        BeFileName dumpSchemaDir;
        std::vector<char> chars(requiredSize);
        if (getenv_s(&requiredSize, chars.data(), requiredSize, envVarName) == 0)
            {
            dumpSchemaDir.AssignUtf8(chars.data());
            DumpSchemasToFile(dumpSchemaDir, m_ecdb.Schemas().GetSchemas(true), "existing");
            DumpSchemasToFile(dumpSchemaDir, schemas, "incoming");
            }
        }
    #endif

    if (!GetECDb().GetImpl().GetIdFactory().Reset())
        {
        LOG.error("Failed to import ECSchemas: Failed to create id factory.");
        return SchemaImportResult::ERROR;
        }

    Policy policy = PolicyManager::GetPolicy(SchemaImportPermissionPolicyAssertion(m_ecdb, schemaImportToken));
    if (!policy.IsSupported())
        {
        LOG.error("Failed to import ECSchemas: Caller has not provided a SchemaImportToken.");
        return SchemaImportResult::ERROR;
        }

    if (m_ecdb.IsReadonly())
        {
        m_ecdb.GetImpl().Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0281, "Failed to import ECSchemas. ECDb file is read-only.");
        return SchemaImportResult::ERROR;
        }

    if (schemas.empty())
        {
        m_ecdb.GetImpl().Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0282, "Failed to import ECSchemas. List of ECSchemas to import is empty.");
        return SchemaImportResult::ERROR;
        }

    auto& schemaSync = m_ecdb.Schemas().GetSchemaSync();
    const auto isSchemaSyncDisabled = schemaSync.IsSchemaSyncDisabled();
    auto resolvedSyncDbUri = syncDbUri.IsEmpty() ? schemaSync.GetDefaultSyncDbUri() : syncDbUri;
    const auto localDbInfo = schemaSync.GetInfo();

    if (!isSchemaSyncDisabled)
        {
        if (localDbInfo.IsEmpty() && !resolvedSyncDbUri.IsEmpty())
            {
            m_ecdb.GetImpl().Issues().ReportV(
                IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0585,
                "Failed to import ECSchemas. Cannot import schemas into a file which is not setup to use schema sync but sync db uri was provided. Sync-Id: {%s}, uri: {%s}.",
                localDbInfo.GetSyncId().c_str(),
                resolvedSyncDbUri.GetUri().c_str()
            );
            return SchemaImportResult::ERROR;
            }

        if (!localDbInfo.IsEmpty())
            {
            if (resolvedSyncDbUri.IsEmpty())
                {
                m_ecdb.GetImpl().Issues().ReportV(
                    IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0586,
                    "Failed to import ECSchemas. Cannot import schemas into a file which is setup to use schema sync but sync db uri was not provided. Sync-Id: {%s}.",

                    localDbInfo.GetSyncId().c_str()
                );
                return SchemaImportResult::ERROR;
                }

            if (schemaSync.Pull(resolvedSyncDbUri, schemaImportToken) != SchemaSync::Status::OK)
                {
                m_ecdb.GetImpl().Issues().ReportV(
                    IssueSeverity::Error, IssueCategory::SchemaSync, IssueType::ECDbIssue, ECDbIssueId::ECDb_0587,
                    "Failed to import ECSchemas. Unable to pull changes from Sync-Id: {%s}, uri: {%s}.",
                    localDbInfo.GetSyncId().c_str(),
                    resolvedSyncDbUri.GetUri().c_str()
                );
                return SchemaImportResult::ERROR;
                }
            if (!GetECDb().GetImpl().GetIdFactory().Reset())
                {
                LOG.error("Failed to import ECSchemas: Failed to create id factory.");
                return SchemaImportResult::ERROR;
                }
            }
        }
    for (auto schema: schemas) {
        if (ECSchemaOwnershipClaimAppData::HasOwnershipClaim(*schema) && !ECSchemaOwnershipClaimAppData::IsOwnedBy(GetECDb(), *schema)) {
            m_ecdb.GetImpl().Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0283, "Failed to import ECSchemas. Cannot import schema owned by another ECDb connection");
            return SchemaImportResult::ERROR;
        }
    }
    // Import into new files is not supported unless it only differs in version sub2. Import into older files is only supported
    // if the schemas to import are EC3.1 schemas. This will be checked downstream.
    const int majorMinorSub1Comp = m_ecdb.GetECDbProfileVersion().CompareTo(ECDb::CurrentECDbProfileVersion(), ProfileVersion::VERSION_MajorMinorSub1);
    if (majorMinorSub1Comp > 0)
        {
        m_ecdb.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0284,
            "Failed to import ECSchemas. Cannot import schemas into a file which was created with a higher version of this softwares. The file's version, however, is %s.",
            ECDb::CurrentECDbProfileVersion().ToString().c_str(),
            m_ecdb.GetECDbProfileVersion().ToString().c_str()
        );
        return SchemaImportResult::ERROR;
        }

    BeMutexHolder lock(m_mutex);
    ECDbExpressionSymbolContext symbolsContext(m_ecdb);
    bvector<ECSchemaCP> schemasToMap;

    auto rc = SchemaWriter::ImportSchemas(schemasToMap, ctx, schemas);
    if (SchemaImportResult::OK != rc)
        {
        LOG.debug("MainSchemaManager::ImportSchemas - failed in SchemaWriter::ImportSchemas");
        return rc;
        }

    if (schemasToMap.empty())
        return SchemaImportResult::OK;

    if (SUCCESS != ctx.GetSchemaPoliciesR().ReadPolicies(m_ecdb))
        {
        LOG.debug("MainSchemaManager::ImportSchemas - failed to ReadPolicies");
        return SchemaImportResult::ERROR;
        }

    if (SUCCESS != ViewGenerator::DropECClassViews(m_ecdb))
        {
        LOG.debug("MainSchemaManager::ImportSchemas - failed to DropECClassViews");
        return SchemaImportResult::ERROR;
        }

    rc = MapSchemas(ctx, schemasToMap);
    if (!rc.IsOk())
        {
        LOG.debug("MainSchemaManager::ImportSchemas - failed to MapSchemas");
        return rc;
        }

    if (!isSchemaSyncDisabled && !localDbInfo.IsEmpty() && rc.IsOk())
        {
        if (schemaSync.Push(resolvedSyncDbUri) != SchemaSync::Status::OK)
            {
            m_ecdb.GetImpl().Issues().ReportV(
                IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0587,
                "Failed to import ECSchemas. Unable to push changes to Sync-Id: {%s}, uri: {%s}.",
                localDbInfo.GetSyncId().c_str(),
                resolvedSyncDbUri.GetUri().c_str());
            return SchemaImportResult::ERROR;
            }
        }

    return SchemaImportResult::OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaImportResult MainSchemaManager::MapSchemas(SchemaImportContext& ctx, bvector<ECN::ECSchemaCP> const& schemas) const {
    if (schemas.empty()) {
        return  SchemaImportResult::OK;
    }

    auto failedToMap = [&]() {
        ClearCache();
        return  SchemaImportResult::ERROR;
    };

#ifndef NDEBUG
    // Record any DML sql against data tables for validation purpose.
    // No changes should be made to data tables.
    DataChangeSqlListener dataChangeListener(ctx.GetECDb());
#endif

    if (SUCCESS != ctx.RemapManager().CleanModifiedMappings()) {
        return failedToMap();
    }

    m_lightweightCache.Clear();

    //necessary so .DerivedClasses() knows all leaf nodes that need mapping, even if they are not inside "schemas"
    if(SUCCESS != ctx.RemapManager().EnsureInvolvedSchemasAreLoaded(schemas)) {
        return failedToMap();
    }

    if (SUCCESS != DoMapSchemas(ctx, schemas)) {
        return failedToMap();
    }

    if (SUCCESS != SaveDbSchema(ctx) || SUCCESS != ctx.RemapManager().RestoreAndProcessCleanedPropertyMaps(ctx)) {
        return failedToMap();
    }

    if (SUCCESS != SaveDbSchema(ctx)) {
        return failedToMap();
    }

    if (SUCCESS != CreateOrUpdateRequiredTables()) {
        return failedToMap();
    }

    if (SUCCESS != CreateOrUpdateIndexesInDb(ctx)) {
        return failedToMap();;
    }

    if (SUCCESS != PurgeOrphanTables(ctx)) {
        return failedToMap();
    }

    if (SUCCESS != DbMapValidator(ctx).Validate()) {
        return failedToMap();
    }

    if (SUCCESS != ctx.RemapManager().UpgradeExistingECInstancesWithRemappedProperties(ctx)) {
        return failedToMap();
    }

#ifndef NDEBUG
    dataChangeListener.Stop();
    if (!dataChangeListener.GetDataChangeSqlList().empty()) {
        ctx.Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0588,
            "Detected data changes during schema mapping which should be deferred and recorded by sql transform api.");

        for(auto& sql : dataChangeListener.GetDataChangeSqlList()) {
            ctx.Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECDbIssue,
                ECDbIssueId::ECDb_0589,
                "Data change sql executed is '%s'. ", sql.c_str());
        }
        return failedToMap();
    }
#endif

    if (!ctx.GetDataTransform().IsEmpty()) {
        if (!ctx.GetDataTransform().Validate(m_ecdb)) {
            return failedToMap();
        }

        if (!ctx.AllowDataTransform()) {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0590, "Import ECSchema failed. Data transform is required which is rejected by default unless explicitly allowed.");
            return SchemaImportResult::ERROR_DATA_TRANSFORM_REQUIRED;
        }
        if (BE_SQLITE_OK != ctx.GetDataTransform().Execute(m_ecdb)) {
            return failedToMap();
        }
    }

    if (BE_SQLITE_OK != UpgradeExistingECInstancesWithNewPropertiesMapToOverflowTable(m_ecdb, nullptr)){
        return failedToMap();
    }

    ClearCache();
    return  SchemaImportResult::OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult MainSchemaManager::UpgradeExistingECInstancesWithNewPropertiesMapToOverflowTable(ECDbCR ecdb, SchemaImportContext* ctx)
    {
    ECDB_PERF_LOG_SCOPE("Schema import> Upgrading existing ECInstances with new property map to overflow tables");
    Statement stmt;
    DbResult st = stmt.Prepare(ecdb, "SELECT PRI.Id, OVR.Id FROM ec_Table PRI INNER JOIN ec_Table OVR ON OVR.ParentTableId = PRI.Id WHERE OVR.Type = " SQLVAL_DbTable_Type_Overflow );
    if (st != BE_SQLITE_OK)
        return st;

    // We need to go over all the overflow table and ensure that if a existing class is
    // mapped to overflow it must have a corresponding row.
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        const DbTable* primaryTable = ecdb.Schemas().Main().GetDbSchema().FindTable(stmt.GetValueId<DbTableId>(0));
        const DbTable* overflowTable = ecdb.Schemas().Main().GetDbSchema().FindTable(stmt.GetValueId<DbTableId>(1));
        if (primaryTable == nullptr || overflowTable == nullptr)
            {
            BeAssert(primaryTable != nullptr && overflowTable != nullptr);
            return BE_SQLITE_ERROR;
            }

        //Overflow table
        Utf8CP overflowTableName = overflowTable->GetName().c_str();
        Utf8CP overflowId = overflowTable->FindFirst(DbColumn::Kind::ECInstanceId)->GetName().c_str();
        Utf8CP overflowClassId = overflowTable->FindFirst(DbColumn::Kind::ECClassId)->GetName().c_str();

        //Primary table
        Utf8CP primaryTableName = primaryTable->GetName().c_str();
        Utf8CP primaryId = primaryTable->FindFirst(DbColumn::Kind::ECInstanceId)->GetName().c_str();
        Utf8CP primaryClassId = primaryTable->FindFirst(DbColumn::Kind::ECClassId)->GetName().c_str();



        //Generated query for a given overflow
        Utf8String transformSql;

        //Replace template with current overflow/primary table column a table name
        transformSql.Sprintf(R"sql(
            INSERT INTO [%s] ([%s], [%s])
              SELECT P.[%s], P.[%s]
              FROM [%s] P
                   LEFT JOIN [%s] O ON O.[%s] = P.[%s]
              WHERE O.[%s] IS NULL AND
                P.[%s] IN (
                SELECT M.ClassId
                FROM ec_PropertyMap M
                       INNER JOIN ec_Column C ON C.Id = M.ColumnId
                       INNER JOIN ec_Table O  ON O.Id = C.TableId AND O.Type = %s
                GROUP BY M.ClassId ))sql",
                             overflowTableName, overflowId, overflowClassId,
                             primaryId, primaryClassId,
                             primaryTableName,
                             overflowTableName, overflowId, primaryId,
                             overflowId,
                             primaryClassId, SQLVAL_DbTable_Type_Overflow);

        if (ctx != nullptr )
            {
            const Utf8String description = SqlPrintfString("upgrade instances in '%s' by inserting a corresponding row in '%s'.", primaryTableName, overflowTableName).GetUtf8CP();
            ctx->GetDataTransform().Append(description, transformSql);
            }
        else
            {
            st = ecdb.TryExecuteSql(transformSql.c_str());
            if (st != BE_SQLITE_OK)
                return st;

            const int modifiedCount = ecdb.GetModifiedRowCount();
            if (modifiedCount > 0)
                {
                LOG.infov("Schema Import/Upgrade inserted '%d' empty rows in '%s' overflow table corresponding to rows in '%s'", modifiedCount, overflowTableName, primaryTableName);
                }
            }
    }
    return BE_SQLITE_OK;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus MainSchemaManager::DoMapSchemas(SchemaImportContext& ctx, bvector<ECN::ECSchemaCP> const& schemas) const
    {
    ECDB_PERF_LOG_SCOPE("Schema import> Persist mappings");
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
            GatherRootClasses(*ecClass, doneList, rootClassSet, rootClassList, rootRelationshipList, rootMixins);
        }

    if (GetDbSchemaR().SynchronizeExistingTables() != SUCCESS)
        {
        m_ecdb.GetImpl().Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0285, "Synchronizing existing table to which classes are mapped failed.");
        return ERROR;
        }

    // Map mixin hierarchy before everything else. It does not map primary hierarchy and all classes map to virtual tables.
    ECDB_PERF_LOG_SCOPE_BEGIN(mapMixins, "Schema import> Map mixins");
    for (ECEntityClassCP mixin : rootMixins)
        {
        if (ClassMappingStatus::Error == MapClass(ctx, *mixin))
            return ERROR;
        }
    ECDB_PERF_LOG_SCOPE_END(mapMixins);

    // Starting with the root, recursively map the entire class hierarchy.
    ECDB_PERF_LOG_SCOPE_BEGIN(logRootClasses, "Schema import> Map entity classes");
    for (ECClassCP rootClass : rootClassList)
        {
        if (ClassMappingStatus::Error == MapClass(ctx, *rootClass))
            return ERROR;
        }
    ECDB_PERF_LOG_SCOPE_END(logRootClasses);

    ECDB_PERF_LOG_SCOPE_BEGIN(logRootRels, "Schema import> Map relationships");
    for (ECRelationshipClassCP rootRelationshipClass : rootRelationshipList)
        {
        if (ClassMappingStatus::Error == MapClass(ctx, *rootRelationshipClass))
            return ERROR;
        }

    ECDB_PERF_LOG_SCOPE_END(logRootRels);
    if (SUCCESS != DbMappingManager::FkRelationships::FinishMapping(ctx))
        return ERROR;

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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

    if (SUCCESS != DbMappingManager::Classes::MapIndexes(ctx, *classMapP, false))
        return ClassMappingStatus::Error;

    return MapDerivedClasses(ctx, mappingInfo.GetClass());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus MainSchemaManager::CheckForPerTableColumnLimit() const
    {
    ECDB_PERF_LOG_SCOPE("Schema import> Check for per table column limit");
    // Note: maxColumns should never be less than the limit defined by SQLITE_MAX_COLUMN; previous limit was 2000
    const int maxColumns = 2000;

    Utf8CP sql = R"(
        SELECT
            [pm].[ClassId] [ClassId],
            [tb].[Name] [TableName],
            COUNT (*) [PersistedColumns]
        FROM
            [ec_PropertyMap] [pm]
            JOIN [ec_Column] [co] ON [co].[Id] = [pm].[ColumnId]
            JOIN [ec_Table] [tb] ON [tb].[Id] = [co].[TableId]
            JOIN [ec_Class] [cl] ON [cl].[Id] = [pm].[ClassId]
            JOIN [ec_ClassMap] [cm] ON [cm].[ClassId] = [cl].[id]
        WHERE
            [co].[IsVirtual] = 0
            AND [cm].[MapStrategy] <> 3
        GROUP BY [cl].[Id], [tb].[Id]
        HAVING COUNT (*) > ?;)";

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, sql))
        {
        BeAssert(false);
        return ERROR;
        }
    stmt.BindInt(1, maxColumns);
    if (BE_SQLITE_ROW == stmt.Step())
        {
        ECClassId classId = stmt.GetValueId<ECClassId>(0);
        ECClassCP ecClass = m_ecdb.Schemas().GetClass(classId, nullptr);
        m_ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0286,
            "Schema Import> Error importing %s class. Could not create or update %s table as there are %d persisted columns, but a maximum of %d columns is allowed for a table.",
            ecClass->GetFullName(), stmt.GetValueText(1), stmt.GetValueInt64(2), maxColumns);
        return ERROR;
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus MainSchemaManager::CheckForSelectWildCardLimit() const
    {
    ECDB_PERF_LOG_SCOPE("Schema import> Check for select wild card limit");
    const int selectWildCardLimit = GetECDb().GetLimit(DbLimits::Column);
    Utf8CP sql = R"(
        SELECT
            [pm].[ClassId] [ClassId],
            COUNT (*) + 2 [ColumnCount]
        FROM
            [ec_PropertyMap] [pm]
            JOIN [ec_PropertyPath] [pp] ON [pp].[Id] = [pm].[PropertyPathId]
        WHERE
            [pp].[AccessString] NOT IN ('ECInstanceId', 'ECClassId')
        GROUP BY [pm].[ClassId]
        HAVING [ColumnCount] > ?;)";

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, sql))
        {
        BeAssert(false);
        return ERROR;
        }
    stmt.BindInt(1, selectWildCardLimit);
    bool limitExceeded = false;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        limitExceeded = true;
        ECClassId classId = stmt.GetValueId<ECClassId>(0);
        const int mappedColumns = stmt.GetValueInt(1);
        ECClassCP ecClass = m_ecdb.Schemas().GetClass(classId, nullptr);
        Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0680,
            "Schema Import> Error importing %s class. The %s class has properties mapped to %d columns, which exceeds the current limit of %d defined by SQLITE_MAX_COLUMN.",
            ecClass->GetFullName(), ecClass->GetFullName(), mappedColumns, selectWildCardLimit);
        }
    return limitExceeded ? ERROR : SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus MainSchemaManager::CanCreateOrUpdateRequiredTables() const
    {
    if (SUCCESS != CheckForPerTableColumnLimit())
        return ERROR;

    if (SUCCESS != CheckForSelectWildCardLimit())
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus MainSchemaManager::CreateOrUpdateRequiredTables() const
    {
    ECDB_PERF_LOG_SCOPE("Schema import> Create or update tables");
    m_ecdb.GetStatementCache().Empty();

    int nCreated = 0;
    int nUpdated = 0;
    int nWasUpToDate = 0;

    if (SUCCESS != CanCreateOrUpdateRequiredTables())
        return ERROR;

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
// @bsimethod
//---------------------------------------------------------------------------------------
 BentleyStatus MainSchemaManager::FindIndexes(std::vector<DbIndex const*>& indexes) const
    {
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
     return SUCCESS;
    }

 //---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
 BentleyStatus MainSchemaManager::LoadIndexesSQL(std::map<Utf8String, Utf8String, CompareIUtf8Ascii>& sqliteIndexes) const
    {
     Statement stmt;
     stmt.Prepare(m_ecdb, "SELECT sqlite_master.name, sqlite_master.sql FROM main.ec_index LEFT JOIN main.sqlite_master ON sqlite_master.name=ec_index.name where sqlite_master.type='index'");
     while (stmt.Step() == BE_SQLITE_ROW)
        {
        sqliteIndexes.insert(std::make_pair(stmt.GetValueText(0), stmt.GetValueText(1)));
        }
     return SUCCESS;
     }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus MainSchemaManager::CreateOrUpdateIndexesInDb(SchemaImportContext& ctx) const
    {
    ECDB_PERF_LOG_SCOPE("Schema import> Create or update indexes");
    if (SUCCESS != m_dbSchema.LoadIndexDefs())
        return ERROR;

    std::vector<DbIndex const*> indexes;
    if (FindIndexes(indexes) != SUCCESS)
        return ERROR;

    std::map<Utf8String, Utf8String, CompareIUtf8Ascii> sqliteIndexes;
    if (LoadIndexesSQL(sqliteIndexes) != SUCCESS)
        return ERROR;

    bmap<Utf8String, DbIndex const*, CompareIUtf8Ascii> comparableIndexDefs;
    bset<Utf8CP, CompareIUtf8Ascii> usedIndexNames;
    for (DbIndex const* indexCP : indexes)
        {
        DbIndex const& index = *indexCP;
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
                Issues().ReportV(
                    IssueSeverity::Error,
                    IssueCategory::BusinessProperties,
                    IssueType::ECDbIssue,
                    ECDbIssueId::ECDb_0287,
                    "Failed to map ECClass '%s'. The index '%s' defined on it spans multiple tables which is not supported. Consider applying the 'TablePerHierarchy' strategy to the ECClass.",
                    ecClass->GetFullName(),
                    index.GetName().c_str()
                );
                return ERROR;
                }
            }

        if (usedIndexNames.find(index.GetName().c_str()) != usedIndexNames.end())
            {
            Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0288,
                "Failed to create index %s on table %s. An index with the same name already exists.", index.GetName().c_str(), index.GetTable().GetName().c_str());
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
            // Here we check if we need to recreate the index.
            auto sqliteIndexItor = sqliteIndexes.find(index.GetName());
            if (sqliteIndexItor != sqliteIndexes.end() && !sqliteIndexItor->second.empty())
                {
                if (!sqliteIndexItor->second.EqualsIAscii(ddl))
                    {
                    LOG.debugv("Schema Import> Recreating index '%s'. The index definition has changed.", index.GetName().c_str());
                    if (!ctx.IsSynchronizeSchemas())
                        {
                        // Delete its entry from ec_index table
                        if (BE_SQLITE_OK != m_ecdb.ExecuteSql(SqlPrintfString("DELETE FROM main." TABLE_Index " WHERE Name = '%s'", index.GetName().c_str())))
                            return ERROR;
                        }

                    // Drop the existing index as its defintion has modified and need to be recreated.
                    if (BE_SQLITE_OK != m_ecdb.GetImpl().ExecuteDDL(SqlPrintfString("DROP INDEX IF EXISTS [%s]", index.GetName().c_str())))
                        return ERROR;

                    if (SUCCESS != DbSchemaPersistenceManager::CreateIndex(m_ecdb, index, ddl))
                        return ERROR;

                    if (!ctx.IsSynchronizeSchemas())
                        {
                        if (SUCCESS != m_dbSchema.PersistIndexDef(index))
                            return ERROR;
                        }
                    }
                else
                    {
                    LOG.debugv("Schema Import> Skipping index '%s'. Not changed", index.GetName().c_str());
                    }
                }
            else
                {
                // This is for safety.
                if (BE_SQLITE_OK != m_ecdb.GetImpl().ExecuteDDL(SqlPrintfString("DROP INDEX IF EXISTS [%s]", index.GetName().c_str())))
                    return ERROR;

                LOG.debugv("Schema Import> Creating Index '%s'.", index.GetName().c_str());
                if (SUCCESS != DbSchemaPersistenceManager::CreateIndex(m_ecdb, index, ddl))
                    return ERROR;

                if (!ctx.IsSynchronizeSchemas())
                    {
                    // Delete its entry from ec_index table
                    if (BE_SQLITE_OK != m_ecdb.ExecuteSql(SqlPrintfString("DELETE FROM main." TABLE_Index " WHERE Name = '%s'", index.GetName().c_str())))
                        return ERROR;

                    if (SUCCESS != m_dbSchema.PersistIndexDef(index))
                        return ERROR;
                    }
                }
            }
        else
            {
            if (!ctx.IsSynchronizeSchemas())
                {
                //populates the ec_Index table (even for indexes on virtual tables, as they might be necessary
                //if further schema imports introduce subclasses of abstract classes (which map to virtual tables))
                                // Delete its entry from ec_index table
                if (BE_SQLITE_OK != m_ecdb.ExecuteSql(SqlPrintfString("DELETE FROM main." TABLE_Index " WHERE Name = '%s'", index.GetName().c_str())))
                    return ERROR;

                LOG.debugv("Schema Import> Virtual index '%s'. NOP SQLite Index", index.GetName().c_str());
                if (SUCCESS != m_dbSchema.PersistIndexDef(index))
                    return ERROR;
                }
            }
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus MainSchemaManager::PurgeOrphanTables(SchemaImportContext& ctx) const
    {
    ECDB_PERF_LOG_SCOPE("Schema import> Purge orphan tables");
    //skip ExistingTable and NotMapped
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(m_ecdb, "SELECT t.Id, t.Name, t.Type FROM main.ec_Table t "
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
     /*
        RULE for purge
        1. Skip overflow tables.
        2. Traverse table to be deleted to leaf node and add those to table to be deleted.
        3. Delete entries from ec_table including virtual.
        4. Drop non-virtual table
     */
    struct TableInfo {
        DbTableId id;
        DbTable::Type type;
        Utf8String tableName;
    };

    auto appendChildTablesIfAny = [&](TableInfo const& parentTableInfo) {
        auto stmt= m_ecdb.GetCachedStatement(R"sql(
            with child_tables(root_id, parent_id, id) as (
                select id,id,id from ec_table where parentTableId is null
                union select c.root_id, t.parentTableId, t.id from ec_table t, child_tables c where c.id=t.parentTableId
            ) select t.id,t.name,t.type from child_tables c join ec_table t on t.id=c.id where c.parent_id!=c.id and c.root_id=?)sql");
        stmt->BindId(1, parentTableInfo.id);
        bvector<TableInfo> childTables;
        while(stmt->Step() == BE_SQLITE_ROW) {
            TableInfo info;
            info.id = stmt->GetValueId<DbTableId>(0);
            info.tableName = stmt->GetValueText(1);
            info.type = Enum::FromInt<DbTable::Type>(stmt->GetValueInt(2));
            childTables.emplace_back(std::move(info));
        }
        childTables.push_back(parentTableInfo);
        return childTables;
    };

    IdSet<DbTableId> orphanTables;
    std::set<Utf8String,CompareIUtf8Ascii> tablesToDrop;
    while (stmt.Step() == BE_SQLITE_ROW) {
        TableInfo tableInfo;
        tableInfo.id = stmt.GetValueId<DbTableId>(0);
        tableInfo.tableName = stmt.GetValueText(1);
        tableInfo.type = Enum::FromInt<DbTable::Type>(stmt.GetValueInt(2));
        if (tableInfo.type == DbTable::Type::Overflow) {
            continue;
        }
        const auto infos = appendChildTablesIfAny(tableInfo);
        for(auto& info: infos) {
            orphanTables.insert(info.id);
            if (info.type != DbTable::Type::Virtual) {
                tablesToDrop.insert(info.tableName);
            }
        }
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

    if (Enum::Contains(ctx.GetOptions(), SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade))
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
        if (!Enum::Contains(ctx.GetOptions(), SchemaManager::SchemaImportOptions::AllowMajorSchemaUpgradeForDynamicSchemas))
            {
            m_ecdb.GetImpl().Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECDbIssue,
                ECDbIssueId::ECDb_0289,
                "Failed to import schemas: it would change the database schema in a backwards incompatible way, so that older versions of the software could not work with the file anymore. ECDb would have to delete these tables: %s",
                tableNames.c_str()
            );
            return ERROR;
            }
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
// @bsimethod
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
// @bsimethod
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

    for (auto const& pair : joinedTablesPerPrimaryTable)
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
// @bsimethod
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

        if (classMap == nullptr) // Class has not been mapped, yet, so do it now.
          {
          ClassMappingInfo mappingInfo(ctx, *ecClass);
          ClassMappingStatus status = mappingInfo.Initialize();
          if (status == ClassMappingStatus::BaseClassesNotMapped || status == ClassMappingStatus::Error)
            {
            BeAssert(false);
            classMaps.clear();
            return classMaps;
            }

          status = MapClass(ctx, mappingInfo);
          if (status == ClassMappingStatus::BaseClassesNotMapped || status == ClassMappingStatus::Error)
            {
            BeAssert(false);
            classMaps.clear();
            return classMaps;
            }

          classMap = GetClassMap(*ecClass);
          if (classMap == nullptr)
             {
             BeAssert(false);
             classMaps.clear();
             return classMaps;
             }
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus MainSchemaManager::GetRelationshipConstraintClassMaps(SchemaImportContext& ctx, std::set<ClassMap const*>& classMaps, ECClassCR ecClass, bool recursive) const
    {
    ClassMap const* classMap = nullptr;
    if (SUCCESS != TryGetClassMap(classMap, ctx.GetClassMapLoadContext(), ecClass) || classMap == nullptr)
        {
        BeAssert(classMap != nullptr && "ClassMap should not be null");
        return ERROR;
        }

    if (classMap->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
        return SUCCESS;

    classMaps.insert(classMap);

    if (!recursive)
        return SUCCESS;

    ECDerivedClassesList const* subclasses = m_ecdb.Schemas().GetDerivedClassesInternal(ecClass);
    if (subclasses == nullptr)
        return ERROR;

    for (ECClassCP subclass : *subclasses)
        {
        if (SUCCESS != GetRelationshipConstraintClassMaps(ctx, classMaps, *subclass, recursive))
            return ERROR;
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus MainSchemaManager::SaveDbSchema(SchemaImportContext& ctx) const
    {
    ECDB_PERF_LOG_SCOPE("Schema import> Persist mappings");
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
            Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0290,
                "Failed to save mapping for ECClass %s: %s", classMap.GetClass().GetFullName(), m_ecdb.GetLastError().c_str());
            return ERROR;
            }
        }

    if (SUCCESS != DbSchemaPersistenceManager::RepopulateClassHasTableCacheTable(m_ecdb))
        return ERROR;

    m_lightweightCache.Clear();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus MainSchemaManager::CreateClassViews() const
    {
    BeMutexHolder lock(m_mutex);
    return ViewGenerator::CreateECClassViews(m_ecdb);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus MainSchemaManager::CreateClassViews(bvector<ECN::ECClassId> const& ecclassids) const
    {
    BeMutexHolder lock(m_mutex);
    return ViewGenerator::CreateECClassViews(m_ecdb, ecclassids);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
void MainSchemaManager::GatherRootClasses(
    ECClassCR ecclass,
    std::set<ECClassCP>& doneList,
    std::set<ECClassCP>& rootClassSet,
    std::vector<ECClassCP>& rootClassList,
    std::vector<ECRelationshipClassCP>& rootRelationshipList,
    std::vector<ECN::ECEntityClassCP>& rootMixins
) {
    if (doneList.find(&ecclass) != doneList.end())
        return;

    doneList.insert(&ecclass);

    const auto insertRootClass = [&](ECClassCR ecClass)
        {
        rootClassSet.insert(&ecClass);
        if (ecClass.IsRelationshipClass())
            rootRelationshipList.push_back(ecClass.GetRelationshipClassCP());
        else if (ecclass.IsMixin())
            rootMixins.push_back(ecclass.GetEntityClassCP());
        else
            rootClassList.push_back(&ecclass);
        };

    if (!ecclass.HasBaseClasses() && rootClassSet.find(&ecclass) == rootClassSet.end())
        {
        insertRootClass(ecclass);
        return;
        }

    const bool allBaseClassesAreMixins = std::all_of(ecclass.GetBaseClasses().begin(), ecclass.GetBaseClasses().end(),
                                                     [](auto const& b){ return b->IsMixin(); });
    if (allBaseClassesAreMixins)
        {
        insertRootClass(ecclass);
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


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult ECDbModule::_OnRegister() {
    auto& vm = GetECDb().Schemas().Main().GetVirtualSchemaManager();
    if (SUCCESS != vm.Add(m_ecSchema)) {
        return BE_SQLITE_ERROR;
    }
    return BE_SQLITE_OK;
}
END_BENTLEY_SQLITE_EC_NAMESPACE
