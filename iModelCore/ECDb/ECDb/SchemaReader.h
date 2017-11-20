/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaReader.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct SchemaDbEntry final
    {
    public:
        ECN::ECSchemaPtr m_cachedSchema = nullptr;    //Contain ECSchema which might be not complete
        int m_typeCountInSchema = -1; //This is read from Db
        uint32_t m_loadedTypeCount = 0; //Every time a class or enum or KOQ is loaded from db for this schema it is incremented

        explicit SchemaDbEntry(ECN::ECSchemaR schema) : m_cachedSchema(&schema) {}
        SchemaDbEntry(ECN::ECSchemaPtr& schema, int typeCountInSchema) : m_cachedSchema(schema), m_typeCountInSchema(typeCountInSchema), m_loadedTypeCount(0)
            {}

        ECN::ECSchemaId GetId() const { return m_cachedSchema->GetId(); }
        bool IsFullyLoaded() const { return m_typeCountInSchema == m_loadedTypeCount; }
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClassDbEntry final
    {
    public:
        ECN::ECClassId m_classId;
        ECN::ECClassP m_cachedClass = nullptr;
        bool m_ensureDerivedClassesExist = false;

        explicit ClassDbEntry(ECN::ECClassCR ecClass) : m_classId(ecClass.GetId()), m_cachedClass(const_cast<ECN::ECClassP> (&ecClass)) {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      12/2015
//+===============+===============+===============+===============+===============+======
struct EnumDbEntry final
    {
public:
    ECN::ECEnumerationId m_enumId;
    ECN::ECEnumerationP m_cachedEnum = nullptr;

    EnumDbEntry(ECN::ECEnumerationId enumId, ECN::ECEnumerationCR ecEnum) : m_enumId(enumId), m_cachedEnum(const_cast<ECN::ECEnumerationP> (&ecEnum)) {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2016
//+===============+===============+===============+===============+===============+======
struct KindOfQuantityDbEntry final
    {
public:
    ECN::KindOfQuantityId m_koqId;
    ECN::KindOfQuantityP m_cachedKoq = nullptr;

    KindOfQuantityDbEntry(ECN::KindOfQuantityId koqId, ECN::KindOfQuantityCR koq) : m_koqId(koqId), m_cachedKoq(const_cast<ECN::KindOfQuantityP> (&koq)) {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2017
//+===============+===============+===============+===============+===============+======
struct PropertyCategoryDbEntry final
    {
    public:
        ECN::PropertyCategoryId m_categoryId;
        ECN::PropertyCategoryP m_cachedCategory = nullptr;

        PropertyCategoryDbEntry(ECN::PropertyCategoryId id, ECN::PropertyCategoryCR cat) : m_categoryId(id), m_cachedCategory(const_cast<ECN::PropertyCategoryP> (&cat)) {}
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct SchemaReader final
    {
    private:
        struct Context final: NonCopyableClass
            {
            private:
                std::vector<ECN::NavigationECProperty*> m_navProps;
                std::vector<ECN::ECSchema*> m_schemasToLoadCAInstancesFor;

            public:
                Context() {}
                void AddNavigationProperty(ECN::NavigationECProperty& navProp) { m_navProps.push_back(&navProp); }
                void AddSchemaToLoadCAInstanceFor(ECN::ECSchemaR schema) { m_schemasToLoadCAInstancesFor.push_back(&schema); }
                BentleyStatus Postprocess(SchemaReader const&) const;
            };

        ECDbCR m_ecdb;
        mutable std::map<ECN::ECSchemaId, std::unique_ptr<SchemaDbEntry>> m_schemaCache;
        mutable std::map<ECN::ECClassId, std::unique_ptr<ClassDbEntry>> m_classCache;
        mutable std::map<ECN::ECEnumerationId, std::unique_ptr<EnumDbEntry>> m_enumCache;
        mutable std::map<ECN::KindOfQuantityId, std::unique_ptr<KindOfQuantityDbEntry>> m_koqCache;
        mutable std::map<ECN::PropertyCategoryId, std::unique_ptr<PropertyCategoryDbEntry>> m_propCategoryCache;
        mutable bmap<Utf8String, bmap<Utf8String, ECN::ECClassId, CompareIUtf8Ascii>, CompareIUtf8Ascii> m_classIdCache;
        ECDbSystemSchemaHelper m_systemSchemaHelper;

        ECN::ECSchemaCP GetSchema(Context&, ECN::ECSchemaId, bool loadSchemaEntities) const;
        ECN::ECClassP GetClass(Context&, ECN::ECClassId) const;
        bool TryGetClassFromCache(ECN::ECClassP&, ECN::ECClassId) const;
        BentleyStatus LoadSchemaEntitiesFromDb(SchemaDbEntry*, Context&, std::set<SchemaDbEntry*>& fullyLoadedSchemas) const;
        BentleyStatus LoadSchemaFromDb(SchemaDbEntry*&, ECN::ECSchemaId) const;
        BentleyStatus LoadClassComponentsFromDb(Context&, ECN::ECClassR) const;
        BentleyStatus LoadPropertiesFromDb(Context&, ECN::ECClassR) const;
        BentleyStatus LoadBaseClassesFromDb(Context&, ECN::ECClassR) const;
        BentleyStatus LoadCAFromDb(ECN::IECCustomAttributeContainerR, Context&, ECContainerId, SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType) const;
        BentleyStatus LoadMixinAppliesToClass(Context&, ECN::ECClassCR mixinClass) const;
        BentleyStatus LoadRelationshipConstraintFromDb(ECN::ECRelationshipClassP&, Context&, ECN::ECClassId constraintClassId, ECN::ECRelationshipEnd) const;
        BentleyStatus LoadRelationshipConstraintClassesFromDb(ECN::ECRelationshipConstraintR, Context&, ECRelationshipConstraintId constraintId) const;
        BentleyStatus LoadSchemaDefinition(SchemaDbEntry*&, bvector<SchemaDbEntry*>& newlyLoadedSchemas, ECN::ECSchemaId) const;

        BentleyStatus ReadSchema(SchemaDbEntry*&, Context&, ECN::ECSchemaId, bool loadSchemaEntities) const;
        BentleyStatus ReadEnumeration(ECN::ECEnumerationP&, Context&, ECN::ECEnumerationId) const;
        BentleyStatus ReadKindOfQuantity(ECN::KindOfQuantityP&, Context&, ECN::KindOfQuantityId) const;
        BentleyStatus ReadPropertyCategory(ECN::PropertyCategoryP&, Context&, ECN::PropertyCategoryId) const;

        BentleyStatus EnsureDerivedClassesExist(Context&, ECN::ECClassId) const;

    public:
        explicit SchemaReader(ECDbCR ecdb) :m_ecdb(ecdb), m_systemSchemaHelper(ecdb) {}
        ~SchemaReader() {}

        bool ContainsSchema(Utf8StringCR schemaNameOrAlias, SchemaLookupMode mode) const { return GetSchemaId(schemaNameOrAlias, mode).IsValid(); }
        ECN::ECSchemaCP GetSchema(ECN::ECSchemaId, bool loadSchemaEntities) const;
        ECN::ECSchemaId GetSchemaId(ECN::ECSchemaCR) const;
        ECN::ECSchemaId GetSchemaId(Utf8StringCR schemaNameOrAlias, SchemaLookupMode mode) const { return SchemaPersistenceHelper::GetSchemaId(m_ecdb, schemaNameOrAlias.c_str(), mode); }
        Utf8String GetSchemaName(ECN::ECSchemaId schemaId) const { return SchemaPersistenceHelper::GetSchemaName(m_ecdb, schemaId); }

        ECN::ECClassCP GetClass(ECN::ECClassId) const;
        ECN::ECClassId GetClassId(ECN::ECClassCR) const;
        ECN::ECClassId GetClassId(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode) const;

        ECN::ECEnumerationCP GetEnumeration(Utf8StringCR schemaName, Utf8StringCR enumName, SchemaLookupMode) const;
        ECN::ECEnumerationId GetEnumerationId(ECN::ECEnumerationCR) const;

        ECN::KindOfQuantityCP GetKindOfQuantity(Utf8StringCR schemaName, Utf8StringCR koqName, SchemaLookupMode) const;
        ECN::KindOfQuantityId GetKindOfQuantityId(ECN::KindOfQuantityCR) const;

        ECN::PropertyCategoryCP GetPropertyCategory(Utf8StringCR schemaName, Utf8StringCR catName, SchemaLookupMode) const;
        ECN::PropertyCategoryId GetPropertyCategoryId(ECN::PropertyCategoryCR) const;

        ECN::ECPropertyId GetPropertyId(ECN::ECPropertyCR) const;
        ECN::ECPropertyId GetPropertyId(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, Utf8StringCR propertyName, SchemaLookupMode mode) const { return SchemaPersistenceHelper::GetPropertyId(m_ecdb, schemaNameOrAlias.c_str(), className.c_str(), propertyName.c_str(), mode); }

        BentleyStatus EnsureDerivedClassesExist(ECN::ECClassId) const;

        ECDbSystemSchemaHelper const& GetSystemSchemaHelper() const { return m_systemSchemaHelper; }
        static std::vector<Utf8CP> GetECDbSchemaNames() { return {"ECDbFileInfo", "ECDbMap", "ECDbMeta", "ECDbSchemaPolicies", "ECDbSystem"}; }

        void ClearCache() const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
