/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbExpressionSymbolProvider.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaManager::SchemaManager(ECDbCR ecdb, BeMutex& mutex) : m_dispatcher(new Dispatcher(ecdb, mutex)) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      01/2013
//---------------------------------------------------------------------------------------
SchemaManager::~SchemaManager()
    {
    if (m_dispatcher != nullptr)
        {
        delete m_dispatcher;
        m_dispatcher = nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                     06/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaManager::ImportSchemas(bvector<ECSchemaCP> const& schemas, SchemaImportOptions options, SchemaImportToken const* token) const
    {
    return Main().ImportSchemas(schemas, options, token);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                   Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECSchemaCP> SchemaManager::GetSchemas(bool loadSchemaEntities, Utf8CP tableSpace) const { return m_dispatcher->GetSchemas(loadSchemaEntities, tableSpace); }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaManager::ContainsSchema(Utf8StringCR schemaNameOrAlias, SchemaLookupMode mode, Utf8CP tableSpace)  const { return m_dispatcher->ContainsSchema(schemaNameOrAlias, mode, tableSpace); }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP SchemaManager::GetSchema(Utf8StringCR schemaNameOrAlias, bool loadSchemaEntities, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetSchema(schemaNameOrAlias, loadSchemaEntities, mode, tableSpace); }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP SchemaManager::GetClass(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetClass(schemaNameOrAlias, className, mode, tableSpace); }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP SchemaManager::GetClass(ECClassId ecClassId, Utf8CP tableSpace) const { return m_dispatcher->GetClass(ecClassId, tableSpace); }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassId SchemaManager::GetClassId(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetClassId(schemaNameOrAlias, className, mode, tableSpace); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                       12/13
//---------------------------------------------------------------------------------------
ECDerivedClassesList const& SchemaManager::GetDerivedClasses(ECClassCR ecClass, Utf8CP tableSpace) const 
    {
    if (GetDerivedClassesInternal(ecClass, tableSpace) == nullptr)
        {
        BeAssert(false && "SchemaManager::GetDerivedClasses failed");
        }

    return ecClass.GetDerivedClasses();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                       12/13
//---------------------------------------------------------------------------------------
ECDerivedClassesList const* SchemaManager::GetDerivedClassesInternal(ECClassCR ecClass, Utf8CP tableSpace) const { return m_dispatcher->GetDerivedClasses(ecClass, tableSpace); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    12/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECEnumerationCP SchemaManager::GetEnumeration(Utf8StringCR schemaNameOrAlias, Utf8StringCR enumName, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetEnumeration(schemaNameOrAlias, enumName, mode, tableSpace); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2016
//+---------------+---------------+---------------+---------------+---------------+------
KindOfQuantityCP SchemaManager::GetKindOfQuantity(Utf8StringCR schemaNameOrAlias, Utf8StringCR koqName, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetKindOfQuantity(schemaNameOrAlias, koqName, mode, tableSpace); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    04/2018
//+---------------+---------------+---------------+---------------+---------------+------
ECUnitCP SchemaManager::GetUnit(Utf8StringCR schemaNameOrAlias, Utf8StringCR unitName, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetUnit(schemaNameOrAlias, unitName, mode, tableSpace); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    04/2018
//+---------------+---------------+---------------+---------------+---------------+------
ECFormatCP SchemaManager::GetFormat(Utf8StringCR schemaNameOrAlias, Utf8StringCR formatName, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetFormat(schemaNameOrAlias, formatName, mode, tableSpace); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    05/2018
//+---------------+---------------+---------------+---------------+---------------+------
UnitSystemCP SchemaManager::GetUnitSystem(Utf8StringCR schemaNameOrAlias, Utf8StringCR unitSystemName, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetUnitSystem(schemaNameOrAlias, unitSystemName, mode, tableSpace); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    05/2018
//+---------------+---------------+---------------+---------------+---------------+------
PhenomenonCP SchemaManager::GetPhenomenon(Utf8StringCR schemaNameOrAlias, Utf8StringCR phenName, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetPhenomenon(schemaNameOrAlias, phenName, mode, tableSpace); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
PropertyCategoryCP SchemaManager::GetPropertyCategory(Utf8StringCR schemaNameOrAlias, Utf8StringCR catName, SchemaLookupMode mode, Utf8CP tableSpace) const { return m_dispatcher->GetPropertyCategory(schemaNameOrAlias, catName, mode, tableSpace); }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaManager::ClearCache() const { return m_dispatcher->ClearCache(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      01/2013
//---------------------------------------------------------------------------------------
ECSchemaPtr SchemaManager::_LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) { return m_dispatcher->LocateSchema(key, matchType, schemaContext, nullptr); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan   12/2015
//---------------------------------------------------------------------------------------
BentleyStatus SchemaManager::CreateClassViewsInDb() const { return Main().CreateClassViews(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle   12/2016
//---------------------------------------------------------------------------------------
BentleyStatus SchemaManager::CreateClassViewsInDb(bvector<ECN::ECClassId> const& ecclassids) const { return Main().CreateClassViews(ecclassids); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle   04/2017
//---------------------------------------------------------------------------------------
BentleyStatus SchemaManager::RepopulateCacheTables() const { return Main().RepopulateCacheTables(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle   04/2017
//---------------------------------------------------------------------------------------
BentleyStatus SchemaManager::UpgradeECInstances() const { return Main().UpgradeECInstances() == BE_SQLITE_OK ? SUCCESS : ERROR; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle   11/2017
//---------------------------------------------------------------------------------------
SchemaManager::Dispatcher const& SchemaManager::GetDispatcher() const { BeAssert(m_dispatcher != nullptr); return *m_dispatcher; }

MainSchemaManager const& SchemaManager::Main() const { return GetDispatcher().Main(); }


END_BENTLEY_SQLITE_EC_NAMESPACE


