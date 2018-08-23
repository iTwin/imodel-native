/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaReader.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
        bool m_derivedClassesAreLoaded = false;

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

        struct LegacyUnitsHelper final
            {
            private:
                ECDbCR m_ecdb;
                DbTableSpace const& m_tableSpace;
                mutable bool m_isInitialized = false;
                mutable ECN::ECSchemaPtr m_formatsSchema = nullptr;
                mutable ECN::ECSchemaPtr m_unitsSchema = nullptr;

                BentleyStatus AddReferences(ECN::ECSchemaCR koqSchema) const;

            public:
                LegacyUnitsHelper(ECDbCR ecdb, DbTableSpace const& tableSpace) : m_ecdb(ecdb), m_tableSpace(tableSpace) {}

                BentleyStatus Initialize() const;

                ECN::ECSchemaCP GetUnitsSchema() const { Initialize(); return m_unitsSchema.get(); }
                ECN::ECSchemaCP GetFormatsSchema() const { Initialize(); return m_formatsSchema.get(); }

                static bool IsValidUnitsSchemaName(Utf8StringCR schemaNameOrAlias, SchemaLookupMode mode)
                    {
                    if (schemaNameOrAlias.EqualsIAscii("u") && (mode == SchemaLookupMode::AutoDetect || mode == SchemaLookupMode::ByAlias))
                        return true;

                    if (schemaNameOrAlias.EqualsIAscii("Units") && (mode == SchemaLookupMode::AutoDetect || mode == SchemaLookupMode::ByName))
                        return true;

                    return false;
                    }

                static bool IsValidFormatsSchemaName(Utf8StringCR schemaNameOrAlias, SchemaLookupMode mode)
                    {
                    if (schemaNameOrAlias.EqualsIAscii("f") && (mode == SchemaLookupMode::AutoDetect || mode == SchemaLookupMode::ByAlias))
                        return true;

                    if (schemaNameOrAlias.EqualsIAscii("Formats") && (mode == SchemaLookupMode::AutoDetect || mode == SchemaLookupMode::ByName))
                        return true;

                    return false;
                    }


                BentleyStatus AssignPersistenceUnitAndPresentationFormats(ECN::KindOfQuantityR koq, Utf8CP legacyPersUnit, bvector<Utf8CP> const& legacyPresUnits) const;

                void ClearCache() const { m_isInitialized = false; m_formatsSchema = nullptr; m_unitsSchema = nullptr; }
            };

        struct ReaderCache final
            {
            public:
                mutable bool m_areUnitsAndFormatsLoaded = false;

            private:
                ECDb const& m_ecdb;
                mutable std::map<ECN::ECSchemaId, std::unique_ptr<SchemaDbEntry>> m_schemaCache;
                mutable std::map<ECN::ECClassId, std::unique_ptr<ClassDbEntry>> m_classCache;
                mutable std::map<ECN::ECEnumerationId, ECN::ECEnumerationCP> m_enumCache;
                mutable std::map<ECN::KindOfQuantityId, ECN::KindOfQuantityCP> m_koqCache;
                mutable std::map<ECN::PropertyCategoryId, ECN::PropertyCategoryCP> m_propCategoryCache;
                mutable std::map<ECN::UnitSystemId, ECN::UnitSystemCP> m_unitSystemCache;
                mutable std::map<ECN::PhenomenonId, ECN::PhenomenonCP> m_phenomenonCache;
                mutable std::map<ECN::UnitId, ECN::ECUnitCP> m_unitCache;
                mutable std::map<ECN::FormatId, ECN::ECFormatCP> m_formatCache;
                mutable bmap<Utf8String, bmap<Utf8String, ECN::ECClassId, CompareIUtf8Ascii>, CompareIUtf8Ascii> m_classIdCache;

                LegacyUnitsHelper m_legacyUnitsHelper;

            public:
                ReaderCache(ECDb const& ecdb, DbTableSpace const& tableSpace) : m_ecdb(ecdb), m_legacyUnitsHelper(ecdb, tableSpace) {}
                void Clear() const;
                SchemaDbEntry* Find(ECN::ECSchemaId) const;
                ClassDbEntry* Find(ECN::ECClassId) const;
                ECN::ECEnumerationCP Find(ECN::ECEnumerationId id) const { auto it = m_enumCache.find(id); return it != m_enumCache.end() ? it->second : nullptr; }
                ECN::KindOfQuantityCP Find(ECN::KindOfQuantityId id) const { auto it = m_koqCache.find(id); return it != m_koqCache.end() ? it->second : nullptr; }
                ECN::PropertyCategoryCP Find(ECN::PropertyCategoryId id) const { auto it = m_propCategoryCache.find(id); return it != m_propCategoryCache.end() ? it->second : nullptr; }
                ECN::UnitSystemCP Find(ECN::UnitSystemId id) const { auto it = m_unitSystemCache.find(id); return it != m_unitSystemCache.end() ? it->second : nullptr;}
                ECN::PhenomenonCP Find(ECN::PhenomenonId id) const { auto it = m_phenomenonCache.find(id); return it != m_phenomenonCache.end() ? it->second : nullptr; }
                ECN::ECUnitCP Find(ECN::UnitId id) const { auto it = m_unitCache.find(id); return it != m_unitCache.end() ? it->second : nullptr; }
                ECN::ECFormatCP Find(ECN::FormatId id) const { auto it = m_formatCache.find(id); return it != m_formatCache.end() ? it->second : nullptr; }

                ECN::ECClassId Find(Utf8StringCR schemaName, Utf8StringCR className) const;
                bool HasClassEntry(ECN::ECClassId id) const;
                void SetClassEntryToNull(ECN::ECClassId id) const;
                bool Insert(Utf8StringCR schemaName, Utf8StringCR className, ECN::ECClassId id) const;
                bool Insert(std::unique_ptr<SchemaDbEntry> entry) const;
                bool Insert(std::unique_ptr<ClassDbEntry> entry) const;
                void Insert(ECN::ECEnumerationCR ecEnum) const { m_enumCache.insert(std::make_pair(ecEnum.GetId(), &ecEnum)); }
                void Insert(ECN::KindOfQuantityCR koq) const { m_koqCache.insert(std::make_pair(koq.GetId(), &koq)); }
                void Insert(ECN::PropertyCategoryCR propCat) const { m_propCategoryCache.insert(std::make_pair(propCat.GetId(), &propCat)); }
                void Insert(ECN::UnitSystemCR us) const { m_unitSystemCache.insert(std::make_pair(us.GetId(), &us)); }
                void Insert(ECN::PhenomenonCR ph) const { m_phenomenonCache.insert(std::make_pair(ph.GetId(), &ph)); }
                void Insert(ECN::ECUnitCR unit) const { m_unitCache.insert(std::make_pair(unit.GetId(), &unit)); }
                void Insert(ECN::ECFormatCR format) const { m_formatCache.insert(std::make_pair(format.GetId(), &format)); }

                LegacyUnitsHelper const& GetLegacyUnitsHelper() const { return m_legacyUnitsHelper; }
            };

        ReaderCache m_cache;
        //not copyable
        SchemaReader(SchemaReader const&) = delete;
        SchemaReader& operator=(SchemaReader const&) = delete;

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
        BentleyStatus LoadUnitsAndFormats(Context&) const;

        BentleyStatus ReadSchema(SchemaDbEntry*&, Context&, ECN::ECSchemaId, bool loadSchemaEntities) const;
        BentleyStatus ReadEnumeration(ECN::ECEnumerationCP&, Context&, ECN::ECEnumerationId) const;

        BentleyStatus ReadKindOfQuantity(ECN::KindOfQuantityCP&, Context&, ECN::KindOfQuantityId) const;

        BentleyStatus ReadUnits(Context&) const;
        BentleyStatus ReadUnitSystems(Context&) const;
        BentleyStatus ReadPhenomena(Context&) const;
        BentleyStatus ReadFormats(Context&) const;
        BentleyStatus ReadFormatComposite(Context&, ECN::ECFormat&, ECN::FormatId, Utf8CP compositeSpacer) const;
        BentleyStatus ReadPropertyCategory(ECN::PropertyCategoryCP&, Context&, ECN::PropertyCategoryId) const;

        BentleyStatus EnsureDerivedClassesExist(Context&, ECN::ECClassId) const;

        ECDbCR GetECDb() const;
        ECDb& GetECDbR() const;
        BeMutex& GetECDbMutex() const;
        DbTableSpace const& GetTableSpace() const;
        CachedStatementPtr GetCachedStatement(Utf8CP sql) const;

    public:
        explicit SchemaReader(TableSpaceSchemaManager const& manager);
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

        ECN::UnitSystemCP GetUnitSystem(Utf8StringCR schemaName, Utf8StringCR systemName, SchemaLookupMode) const;
        ECN::UnitSystemId GetUnitSystemId(ECN::UnitSystemCR) const;
        ECN::PhenomenonCP GetPhenomenon(Utf8StringCR schemaName, Utf8StringCR phenName, SchemaLookupMode) const;
        ECN::PhenomenonId GetPhenomenonId(ECN::PhenomenonCR) const;
        ECN::ECUnitCP GetUnit(Utf8StringCR schemaName, Utf8StringCR unitName, SchemaLookupMode) const;
        ECN::UnitId GetUnitId(ECN::ECUnitCR) const;
        ECN::ECFormatCP GetFormat(Utf8StringCR schemaName, Utf8StringCR formatName, SchemaLookupMode) const;
        ECN::FormatId GetFormatId(ECN::ECFormatCR) const;

        BentleyStatus EnsureDerivedClassesExist(ECN::ECClassId) const;

        void ClearCache() const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
