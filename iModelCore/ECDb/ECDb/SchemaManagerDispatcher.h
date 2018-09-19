/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaManagerDispatcher.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/SchemaManager.h>
#include "SchemaImportContext.h"
#include "SchemaReader.h"
#include "ECDbSystemSchemaHelper.h"
#include "DbSchema.h"
#include "LightweightCache.h"
#include "DbUtilities.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      11/2017
//+===============+===============+===============+===============+===============+======
struct TableSpaceSchemaManager
    {
protected:
    ECDbCR m_ecdb;
    
    DbTableSpace m_tableSpace;
    SchemaReader m_reader;
    mutable std::map<ECN::ECClassId, std::unique_ptr<ClassMap>> m_classMapDictionary;
    DbSchema m_dbSchema;
    mutable LightweightCache m_lightweightCache;

private:
    //not copyable
    TableSpaceSchemaManager(TableSpaceSchemaManager const&) = delete;
    TableSpaceSchemaManager& operator=(TableSpaceSchemaManager const&) = delete;

    BentleyStatus TryLoadClassMap(ClassMap*&, ClassMapLoadContext& ctx, ECN::ECClassCR) const;

protected:
    BentleyStatus TryGetClassMap(ClassMap const*&, ClassMapLoadContext&, ECN::ECClassCR) const;
    BentleyStatus TryGetClassMap(ClassMap*&, ClassMapLoadContext&, ECN::ECClassCR) const;
    ClassMap* AddClassMap(std::unique_ptr<ClassMap>) const;

public:
    TableSpaceSchemaManager(ECDbCR ecdb, DbTableSpace const& tableSpace) : m_ecdb(ecdb), m_tableSpace(tableSpace), m_reader(*this), m_dbSchema(*this), m_lightweightCache(*this) {}
    virtual ~TableSpaceSchemaManager() {}

    ECN::ECSchemaPtr LocateSchema(ECN::SchemaKeyR, ECN::SchemaMatchType, ECN::ECSchemaReadContextR) const;
    BentleyStatus GetSchemas(bvector<ECN::ECSchemaCP>& schemas, bool loadSchemaEntities = true) const { return m_reader.GetSchemas(schemas, loadSchemaEntities); }
    bool ContainsSchema(Utf8StringCR schemaNameOrAlias, SchemaLookupMode mode = SchemaLookupMode::ByName) const { return m_reader.ContainsSchema(schemaNameOrAlias, mode); }
    ECN::ECSchemaCP GetSchema(Utf8StringCR schemaNameOrAlias, bool loadSchemaEntities = true, SchemaLookupMode mode = SchemaLookupMode::ByName) const { return m_reader.GetSchema(schemaNameOrAlias, loadSchemaEntities, mode); }
    ECN::ECSchemaId GetSchemaId(ECN::ECSchemaCR schema) const { return m_reader.GetSchemaId(schema); }

    ECN::ECClassCP GetClass(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode mode = SchemaLookupMode::ByName) const { return m_reader.GetClass(schemaNameOrAlias, className, mode); }
    ECN::ECClassCP GetClass(ECN::ECClassId classId) const { return m_reader.GetClass(classId); }
    ECN::ECClassId GetClassId(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode mode = SchemaLookupMode::ByName) const { return m_reader.GetClassId(schemaNameOrAlias, className, mode); }
    ECN::ECClassId GetClassId(ECN::ECClassCR ecClass) const { return m_reader.GetClassId(ecClass); }

    // returns nullptr in case of errors
    ECN::ECDerivedClassesList const* GetDerivedClasses(ECN::ECClassCR baseClass) const;

    ECN::ECPropertyId GetPropertyId(ECN::ECPropertyCR prop) const { return m_reader.GetPropertyId(prop); }

    ECN::ECEnumerationCP GetEnumeration(Utf8StringCR schemaNameOrAlias, Utf8StringCR enumName, SchemaLookupMode mode = SchemaLookupMode::ByName) const { return m_reader.GetEnumeration(schemaNameOrAlias, enumName, mode); }
    ECN::ECEnumerationId GetEnumerationId(ECN::ECEnumerationCR ecenum) const { return m_reader.GetEnumerationId(ecenum); }

    ECN::UnitSystemCP GetUnitSystem(Utf8StringCR schemaNameOrAlias, Utf8StringCR systemName, SchemaLookupMode mode = SchemaLookupMode::ByName) const { return m_reader.GetUnitSystem(schemaNameOrAlias, systemName, mode); }
    ECN::UnitSystemId GetUnitSystemId(ECN::UnitSystemCR us) const { return m_reader.GetUnitSystemId(us); }
    ECN::PhenomenonCP GetPhenomenon(Utf8StringCR schemaNameOrAlias, Utf8StringCR phenName, SchemaLookupMode mode = SchemaLookupMode::ByName) const { return m_reader.GetPhenomenon(schemaNameOrAlias, phenName, mode); }
    ECN::PhenomenonId GetPhenomenonId(ECN::PhenomenonCR ph) const { return m_reader.GetPhenomenonId(ph); }
    ECN::UnitId GetUnitId(ECN::ECUnitCR unit) const { return m_reader.GetUnitId(unit); }
    ECN::FormatId GetFormatId(ECN::ECFormatCR format) const { return m_reader.GetFormatId(format); }

    ECN::KindOfQuantityCP GetKindOfQuantity(Utf8StringCR schemaNameOrAlias, Utf8StringCR koqName, SchemaLookupMode mode = SchemaLookupMode::ByName) const { return m_reader.GetKindOfQuantity(schemaNameOrAlias, koqName, mode); }
    ECN::KindOfQuantityId GetKindOfQuantityId(ECN::KindOfQuantityCR koq) const { return m_reader.GetKindOfQuantityId(koq); }

    ECN::ECUnitCP GetUnit(Utf8StringCR schemaNameOrAlias, Utf8StringCR unitName, SchemaLookupMode mode = SchemaLookupMode::ByName) const { return m_reader.GetUnit(schemaNameOrAlias, unitName, mode); }
    ECN::ECFormatCP GetFormat(Utf8StringCR schemaNameOrAlias, Utf8StringCR formatName, SchemaLookupMode mode = SchemaLookupMode::ByName) const { return m_reader.GetFormat(schemaNameOrAlias, formatName, mode); }

    ECN::PropertyCategoryCP GetPropertyCategory(Utf8StringCR schemaNameOrAlias, Utf8StringCR propertyCategoryName, SchemaLookupMode mode = SchemaLookupMode::ByName) const { return m_reader.GetPropertyCategory(schemaNameOrAlias, propertyCategoryName, mode); }
    ECN::PropertyCategoryId GetPropertyCategoryId(ECN::PropertyCategoryCR cat) const { return m_reader.GetPropertyCategoryId(cat); }

    ClassMap const* GetClassMap(ECN::ECClassCR) const;

    std::map<ECN::ECClassId, std::unique_ptr<ClassMap>> const& GetClassMapCache() const { return m_classMapDictionary; }

    ECDbCR GetECDb() const { return m_ecdb; }
    bool IsMain() const { return m_tableSpace.IsMain(); }
    DbTableSpace const& GetTableSpace() const { return m_tableSpace; }
    DbSchema const& GetDbSchema() const { return m_dbSchema; }
    DbSchema& GetDbSchemaR() const { return const_cast<DbSchema&> (m_dbSchema); }
    LightweightCache const& GetLightweightCache() const { return m_lightweightCache; }
    IssueReporter const& Issues() const { return m_ecdb.GetImpl().Issues(); }

    void ClearCache() const { m_reader.ClearCache(); m_classMapDictionary.clear(); m_dbSchema.ClearCache(); m_lightweightCache.Clear(); }

    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      11/2017
//+===============+===============+===============+===============+===============+======
struct MainSchemaManager final : TableSpaceSchemaManager
    {
private:
    BeMutex& m_mutex;
    ECDbSystemSchemaHelper m_systemSchemaHelper;

    BentleyStatus ImportSchemas(SchemaImportContext&, bvector<ECN::ECSchemaCP> const& schemas, SchemaImportToken const*) const;

    BentleyStatus MapSchemas(SchemaImportContext&, bvector<ECN::ECSchemaCP> const&) const;
    BentleyStatus DoMapSchemas(SchemaImportContext&, bvector<ECN::ECSchemaCP> const&) const;
    ClassMappingStatus MapClass(SchemaImportContext&, ClassMappingInfo const&) const;
    ClassMappingStatus MapDerivedClasses(SchemaImportContext&, ECN::ECClassCR baseClass) const;
    BentleyStatus SaveDbSchema(SchemaImportContext&) const;
    BentleyStatus CreateOrUpdateRequiredTables() const;
    BentleyStatus CreateOrUpdateIndexesInDb(SchemaImportContext&) const;
    BentleyStatus PurgeOrphanTables(SchemaImportContext&) const;

    std::set<ClassMap const*> GetRelationshipConstraintClassMaps(SchemaImportContext&, ECN::ECRelationshipConstraintCR) const;
    BentleyStatus GetRelationshipConstraintClassMaps(SchemaImportContext&, std::set<ClassMap const*>&, ECN::ECClassCR, bool recursive) const;

    static void GatherRootClasses(ECN::ECClassCR ecclass, std::set<ECN::ECClassCP>& doneList, std::set<ECN::ECClassCP>& rootClassSet, std::vector<ECN::ECClassCP>& rootClassList, std::vector<ECN::ECRelationshipClassCP>& rootRelationshipList, std::vector<ECN::ECEntityClassCP>& rootMixins);

    static DbResult UpgradeExistingECInstancesWithNewPropertiesMapToOverflowTable(ECDbCR ecdb);
public:
    explicit MainSchemaManager(ECDbCR ecdb, BeMutex& mutex) : TableSpaceSchemaManager(ecdb, DbTableSpace::Main()), m_mutex(mutex), m_systemSchemaHelper(ecdb) {}
    ~MainSchemaManager() {}

    BentleyStatus ImportSchemas(bvector<ECN::ECSchemaCP> const& schemas, SchemaManager::SchemaImportOptions, SchemaImportToken const*) const;
    ClassMappingStatus MapClass(SchemaImportContext&, ECN::ECClassCR) const;
    std::set<DbTable const*> GetRelationshipConstraintPrimaryTables(SchemaImportContext&, ECN::ECRelationshipConstraintCR) const;
    size_t GetRelationshipConstraintTableCount(SchemaImportContext&, ECN::ECRelationshipConstraintCR) const;

    BentleyStatus RepopulateCacheTables() const;
    DbResult UpgradeECInstances() const { return UpgradeExistingECInstancesWithNewPropertiesMapToOverflowTable(GetECDb()); }
    BentleyStatus CreateClassViews() const;
    BentleyStatus CreateClassViews(bvector<ECN::ECClassId> const& ecclassids) const;

    ECDbSystemSchemaHelper const& GetSystemSchemaHelper() const { return m_systemSchemaHelper; }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      11/2017
//+===============+===============+===============+===============+===============+======
struct SchemaManager::Dispatcher final
    {
    private:
        struct Iterable final
            {
            struct const_iterator final : std::iterator<std::forward_iterator_tag, TableSpaceSchemaManager const*>
                {
                enum class Type { Begin, End };

                private:
                    std::vector<TableSpaceSchemaManager const*>::const_iterator m_it;

                public:
                    const_iterator(Iterable const& iterable, Type type)
                        {
                        BeAssert(iterable.m_collection != nullptr);
                        if (type == Type::Begin)
                            m_it = iterable.m_collection->begin();
                        else
                            m_it = iterable.m_collection->end();
                        }

                    ~const_iterator() {}
                    //copyable
                    const_iterator(const_iterator const& rhs) : m_it(rhs.m_it) {}
                    const_iterator& operator=(const_iterator const& rhs)
                        {
                        if (this != &rhs)
                            m_it = rhs.m_it;

                        return *this;
                        }
                    //moveable
                    const_iterator(const_iterator&& rhs) : m_it(std::move(rhs.m_it)) {}
                    const_iterator& operator=(const_iterator&& rhs)
                        {
                        if (this != &rhs)
                            m_it = std::move(rhs.m_it);

                        return *this;
                        }

                    TableSpaceSchemaManager const* operator*() const { return *m_it; }

                    const_iterator& operator++() { m_it++; return *this; }
                    bool operator== (const_iterator const& rhs) const { return m_it == rhs.m_it; }
                    bool operator!= (const_iterator const& rhs) const { return !(*this == rhs); }
                };

            std::vector<TableSpaceSchemaManager const*> const* m_collection = nullptr;
            std::vector<TableSpaceSchemaManager const*> m_filteredCollection;

            explicit Iterable(TableSpaceSchemaManager const& filter)
                {
                m_filteredCollection.push_back(&filter);
                m_collection = &m_filteredCollection;
                }

            Iterable() {}
            explicit Iterable(Dispatcher const& dispatcher) : m_collection(&dispatcher.m_orderedManagers) {}
            bool IsValid() const { return m_collection != nullptr; }
            const_iterator begin() const { return const_iterator(*this, const_iterator::Type::Begin); }
            const_iterator end() const { return const_iterator(*this, const_iterator::Type::End); }
            };

        ECDb const& m_ecdb;
        BeMutex& m_mutex;
        mutable std::map<Utf8String, std::unique_ptr<TableSpaceSchemaManager>, CompareIUtf8Ascii> m_managers;
        mutable std::vector<TableSpaceSchemaManager const*> m_orderedManagers;
        MainSchemaManager const* m_main = nullptr;

        //not copyable
        Dispatcher(Dispatcher const&) = delete;
        Dispatcher& operator=(Dispatcher const&) = delete;

        void InitMain();

        Iterable GetIterable(Utf8CP tableSpaceName) const;
        TableSpaceSchemaManager const* GetManager(Utf8CP tableSpaceName) const;

    public:
        Dispatcher(ECDbCR ecdb, BeMutex& mutex) : m_ecdb(ecdb), m_mutex(mutex) { InitMain(); }
        ~Dispatcher() {}

        MainSchemaManager const& Main() const { BeAssert(m_main != nullptr); return *m_main; }
        BentleyStatus AddManager(DbTableSpace const&) const;
        BentleyStatus RemoveManager(DbTableSpace const&) const;

        bvector<ECN::ECSchemaCP> GetSchemas(bool loadSchemaEntities, Utf8CP tableSpace) const;
        ECN::ECSchemaPtr LocateSchema(ECN::SchemaKeyR, ECN::SchemaMatchType, ECN::ECSchemaReadContextR, Utf8CP tableSpace) const;
        bool ContainsSchema(Utf8StringCR schemaNameOrAlias, SchemaLookupMode, Utf8CP tableSpace) const;
        ECN::ECSchemaCP GetSchema(Utf8StringCR schemaNameOrAlias, bool loadSchemaEntities, SchemaLookupMode, Utf8CP tableSpace) const;

        ECN::ECClassCP GetClass(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode, Utf8CP tableSpace) const;
        ECN::ECClassCP GetClass(ECN::ECClassId classId, Utf8CP tableSpace) const;
        ECN::ECClassId GetClassId(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode, Utf8CP tableSpace) const;

        ClassMap const* GetClassMap(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode, Utf8CP tableSpace) const;
        ClassMap const* GetClassMap(ECN::ECClassCR, Utf8CP tableSpace) const;

        // returns nullptr in case of errors
        ECN::ECDerivedClassesList const* GetDerivedClasses(ECN::ECClassCR baseClass, Utf8CP tableSpace) const;

        ECN::ECEnumerationCP GetEnumeration(Utf8StringCR schemaNameOrAlias, Utf8StringCR enumName, SchemaLookupMode, Utf8CP tableSpace) const;
        ECN::KindOfQuantityCP GetKindOfQuantity(Utf8StringCR schemaNameOrAlias, Utf8StringCR koqName, SchemaLookupMode, Utf8CP tableSpace) const;
        ECN::ECUnitCP GetUnit(Utf8StringCR schemaNameOrAlias, Utf8StringCR unitName, SchemaLookupMode, Utf8CP tableSpace) const;
        ECN::ECFormatCP GetFormat(Utf8StringCR schemaNameOrAlias, Utf8StringCR formatName, SchemaLookupMode, Utf8CP tableSpace) const;
        ECN::UnitSystemCP GetUnitSystem(Utf8StringCR schemaNameOrAlias, Utf8StringCR systemName, SchemaLookupMode, Utf8CP tableSpace) const;
        ECN::PhenomenonCP GetPhenomenon(Utf8StringCR schemaNameOrAlias, Utf8StringCR phenName, SchemaLookupMode, Utf8CP tableSpace) const;
        ECN::PropertyCategoryCP GetPropertyCategory(Utf8StringCR schemaNameOrAlias, Utf8StringCR propertyCategoryName, SchemaLookupMode, Utf8CP tableSpace) const;

        void ClearCache() const;

        static std::vector<Utf8CP> GetECDbSchemaNames() { return {"ECDbFileInfo", "ECDbMap", "ECDbMeta", "ECDbSchemaPolicies", "ECDbSystem", "ECDbChange"}; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
