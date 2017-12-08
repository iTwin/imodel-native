/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaReader.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include "SchemaPersistenceHelper.h"
#include "DbUtilities.h"

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

struct TableSpaceSchemaManager;

//=======================================================================================
// @bsimethod                                                    Affan.Khan        05/2012
//+===============+===============+===============+===============+===============+======
struct SchemaReader final
    {
    public:
        struct Context final
            {
            private:
                std::vector<ECN::NavigationECProperty*> m_navProps;
                std::vector<ECN::ECSchema*> m_schemasToLoadCAInstancesFor;

                //not copyable
                Context(Context const&) = delete;
                Context& operator=(Context const&) = delete;

            public:
                Context() {}
                void AddNavigationProperty(ECN::NavigationECProperty& navProp) { m_navProps.push_back(&navProp); }
                void AddSchemaToLoadCAInstanceFor(ECN::ECSchemaR schema) { m_schemasToLoadCAInstancesFor.push_back(&schema); }
                BentleyStatus Postprocess(SchemaReader const&) const;
            };

    private:
        TableSpaceSchemaManager const& m_schemaManager;
        struct ReaderCache final
            {
            private:
                ECDb const& m_ecdb;
                mutable std::map<ECN::ECSchemaId, std::unique_ptr<SchemaDbEntry>> m_schemaCache;
                mutable std::map<ECN::ECClassId, std::unique_ptr<ClassDbEntry>> m_classCache;
                mutable std::map<ECN::ECEnumerationId, std::unique_ptr<EnumDbEntry>> m_enumCache;
                mutable std::map<ECN::KindOfQuantityId, std::unique_ptr<KindOfQuantityDbEntry>> m_koqCache;
                mutable std::map<ECN::PropertyCategoryId, std::unique_ptr<PropertyCategoryDbEntry>> m_propCategoryCache;
                mutable bmap<Utf8String, bmap<Utf8String, ECN::ECClassId, CompareIUtf8Ascii>, CompareIUtf8Ascii> m_classIdCache;
            public:
                explicit ReaderCache(ECDb const& ecdb):m_ecdb(ecdb)
                    {}
                void Clear() const;
                SchemaDbEntry* Find(ECN::ECSchemaId id) const;
                ClassDbEntry* Find(ECN::ECClassId id) const;
                EnumDbEntry* Find(ECN::ECEnumerationId id) const;
                KindOfQuantityDbEntry* Find(ECN::KindOfQuantityId id) const;
                PropertyCategoryDbEntry* Find(ECN::PropertyCategoryId id) const;
                ECN::ECClassId Find(Utf8StringCR schemaName, Utf8StringCR className) const;
                bool HasClassEntry(ECN::ECClassId id) const;
                bool InsertNullClassEntry(ECN::ECClassId id) const;
                bool Insert(std::unique_ptr<SchemaDbEntry> entry) const;
                bool Insert(std::unique_ptr<ClassDbEntry> entry) const;
                bool Insert(std::unique_ptr<EnumDbEntry> entry) const;
                bool Insert(std::unique_ptr<KindOfQuantityDbEntry> entry) const;
                bool Insert(std::unique_ptr<PropertyCategoryDbEntry> entry) const;
                bool Insert(Utf8StringCR schemaName, Utf8StringCR className, ECN::ECClassId id) const;
            };

        ReaderCache m_cache;
        //not copyable
        SchemaReader(SchemaReader const&) = delete;
        SchemaReader& operator=(SchemaReader const&) = delete;

        std::unique_ptr<BeMutexHolder> LockECDb() const;
        std::unique_ptr<BeSqliteDbMutex> LockDb() const;

        ECN::ECSchemaCP GetSchema(Context&, ECN::ECSchemaId, bool loadSchemaEntities) const;
        ECN::ECClassP GetClass(Context&, ECN::ECClassId) const;
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

        ECDbCR GetECDb() const;
        DbTableSpace const& GetTableSpace() const;
        CachedStatementPtr GetCachedStatement(Utf8CP sql) const;

    public:
        SchemaReader(TableSpaceSchemaManager const& manager);
        ~SchemaReader() {}

        BentleyStatus GetSchemas(bvector<ECN::ECSchemaCP>&, bool loadSchemaEntities) const;
        bool ContainsSchema(Utf8StringCR schemaNameOrAlias, SchemaLookupMode mode) const { return SchemaPersistenceHelper::GetSchemaId(GetECDb(), GetTableSpace(), schemaNameOrAlias.c_str(), mode).IsValid(); }
        ECN::ECSchemaCP GetSchema(Utf8StringCR schemaNameOrAlias, bool loadSchemaEntities, SchemaLookupMode) const;
        ECN::ECSchemaCP GetSchema(ECN::ECSchemaId, bool loadSchemaEntities) const;
        ECN::ECSchemaId GetSchemaId(ECN::ECSchemaCR) const;

        ECN::ECClassCP GetClass(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode) const;
        ECN::ECClassCP GetClass(ECN::ECClassId) const;
        ECN::ECClassId GetClassId(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode) const;
        ECN::ECClassId GetClassId(ECN::ECClassCR) const;

        ECN::ECEnumerationCP GetEnumeration(Utf8StringCR schemaName, Utf8StringCR enumName, SchemaLookupMode) const;
        ECN::ECEnumerationId GetEnumerationId(ECN::ECEnumerationCR) const;

        ECN::KindOfQuantityCP GetKindOfQuantity(Utf8StringCR schemaName, Utf8StringCR koqName, SchemaLookupMode) const;
        ECN::KindOfQuantityId GetKindOfQuantityId(ECN::KindOfQuantityCR) const;

        ECN::PropertyCategoryCP GetPropertyCategory(Utf8StringCR schemaName, Utf8StringCR catName, SchemaLookupMode) const;
        ECN::PropertyCategoryId GetPropertyCategoryId(ECN::PropertyCategoryCR) const;

        ECN::ECPropertyId GetPropertyId(ECN::ECPropertyCR) const;
        ECN::ECPropertyId GetPropertyId(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, Utf8StringCR propertyName, SchemaLookupMode mode) const { return SchemaPersistenceHelper::GetPropertyId(GetECDb(), GetTableSpace(), schemaNameOrAlias.c_str(), className.c_str(), propertyName.c_str(), mode); }

        BentleyStatus EnsureDerivedClassesExist(ECN::ECClassId) const;

        void ClearCache() const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
