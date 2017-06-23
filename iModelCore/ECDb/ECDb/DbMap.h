/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbMap.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ClassMap.h"
#include "SchemaImportContext.h"
#include "DbSchema.h"
#include "IssueReporter.h"
#include "LightweightCache.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
struct RelationshipClassMap;
struct RelationshipClassLinkTableMap;

/*=================================================================================**//**
* @bsiclass                                                     Casey.Mullen      11/2011
+===============+===============+===============+===============+===============+======*/
struct DbMap final : NonCopyableClass
    {
    struct Validator
        {
        enum class Filter
            {
            InMemory,
            All
            };

        private:
            DbMap const& m_map;
            mutable BentleyStatus m_status;
            mutable bool m_onErrorFail;
            mutable std::vector<Utf8String> m_errors;
            SchemaImportContext & m_schemaImportContext;

        private:
            BentleyStatus Error(Utf8CP, ...) const;
            bool IsError(BentleyStatus) const;
            BentleyStatus CheckDbSchema(DbSchema const&, Filter) const;
            BentleyStatus CheckDbMap(DbMap  const& map, Filter) const;
            BentleyStatus CheckMapCount(DbMap  const&) const;
            BentleyStatus CheckDbIndexes(DbSchema const&) const;
            BentleyStatus CheckDbTables(std::vector<DbTable const*> const&) const;
            BentleyStatus CheckDbTable(DbTable const&) const;
            BentleyStatus CheckDbConstraints(DbTable const&) const;
            BentleyStatus CheckDbConstraint(DbConstraint const&) const;
            BentleyStatus CheckClassMap(ClassMap const&) const;
            BentleyStatus CheckNotMappedClassMap(NotMappedClassMap const&) const;
            BentleyStatus CheckDbTriggers(DbTable const&) const;
            BentleyStatus CheckDbTrigger(DbTrigger const&) const;
            BentleyStatus CheckDbColumn(DbColumn const&) const;
            BentleyStatus CheckDbIndex(DbIndex const&) const;
            BentleyStatus CheckForeignKeyDbConstraint(ForeignKeyDbConstraint const&) const;
            BentleyStatus CheckPrimaryKeyDbConstraint(PrimaryKeyDbConstraint const&) const;
            BentleyStatus CheckPropertyMaps(PropertyMapContainer const&) const;
            BentleyStatus CheckPropertyMap(PropertyMap const&) const;
            BentleyStatus CheckRelationshipClassMap(RelationshipClassMap const&) const;
            BentleyStatus CheckRelationshipClassEndTableMap(RelationshipClassEndTableMap const&) const;
            BentleyStatus CheckRelationshipClassLinkTableMap(RelationshipClassLinkTableMap const&) const;
            BentleyStatus CheckIfAllPropertiesAreMapped(ClassMap const&) const;
            BentleyStatus ReportIssues() const;

        public:
            Validator(DbMap const& map, SchemaImportContext & ctx)
                :m_map(map), m_schemaImportContext(ctx)
                {}
            ~Validator() {}
            BentleyStatus CheckAndReportIssues( bool onErrorFail = false, Filter classMapFilter = Filter::All, Filter tableFilter = Filter::All) const;
        };
    
    private:
        ECDbCR m_ecdb;
        DbSchema m_dbSchema;
        mutable bmap<ECN::ECClassId, ClassMapPtr> m_classMapDictionary;
        mutable LightweightCache m_lightweightCache;

        BentleyStatus TryGetClassMap(ClassMap const*& classMap, ClassMapLoadContext& ctx, ECN::ECClassCR ecClass) const
            {
            ClassMapPtr classMapPtr = nullptr;
            if (SUCCESS != TryGetClassMap(classMapPtr, ctx, ecClass))
                return ERROR;

            classMap = classMapPtr.get();
            return SUCCESS;
            }

        BentleyStatus TryGetClassMap(ClassMapPtr&, ClassMapLoadContext&, ECN::ECClassCR) const;
        ClassMapPtr DoGetClassMap(ECN::ECClassCR) const;
        BentleyStatus TryLoadClassMap(ClassMapPtr&, ClassMapLoadContext& ctx, ECN::ECClassCR) const;
        BentleyStatus DoMapSchemas(SchemaImportContext&, bvector<ECN::ECSchemaCP> const&) const;
        ClassMappingStatus MapClass(SchemaImportContext&, ECN::ECClassCR) const;
        BentleyStatus SaveDbSchema(SchemaImportContext&) const;
        BentleyStatus CreateOrUpdateRequiredTables() const;
        BentleyStatus CreateOrUpdateIndexesInDb(SchemaImportContext&) const;
        BentleyStatus PurgeOrphanTables() const;
        ClassMappingStatus AddClassMap(ClassMapPtr&) const;
        BentleyStatus GetClassMapsFromRelationshipEnd(SchemaImportContext&, std::set<ClassMap const*>&, ECN::ECClassCR, bool recursive) const;
        
        static void GatherRootClasses(ECN::ECClassCR ecclass, std::set<ECN::ECClassCP>& doneList, std::set<ECN::ECClassCP>& rootClassSet, std::vector<ECN::ECClassCP>& rootClassList, std::vector<ECN::ECRelationshipClassCP>& rootRelationshipList, std::vector<ECN::ECEntityClassCP>& rootMixins);

        BentleyStatus ValidateClassMap(SchemaImportContext& ctx, ClassMapCR classMap) const;
        BentleyStatus ValidateDbMappings(SchemaImportContext& ctx, bool failOnError) const;

    public:
        explicit DbMap(ECDbCR ecdb) : m_ecdb(ecdb), m_dbSchema(ecdb), m_lightweightCache(ecdb) {}
        ~DbMap() {}

        void ClearCache() const;
        ClassMap const* GetClassMap(ECN::ECClassCR) const;

        BentleyStatus MapSchemas(SchemaImportContext&, bvector<ECN::ECSchemaCP> const&) const;
        ClassMappingStatus MapRelationshipClass(SchemaImportContext& ctx, ECN::ECRelationshipClassCR r) const { return MapClass(ctx, r); }

        //!Loads the class maps if they were not loaded yet
        size_t GetTableCountOnRelationshipEnd(SchemaImportContext&, ECN::ECRelationshipConstraintCR) const;
        std::set<ClassMap const*> GetClassMapsFromRelationshipEnd(SchemaImportContext&, ECN::ECRelationshipConstraintCR, bool* hasAnyClass) const;

        DbSchema const& GetDbSchema() const { return m_dbSchema; }
        DbSchema& GetDbSchemaR() const { return const_cast<DbSchema&> (m_dbSchema); }
        LightweightCache const& GetLightweightCache() const { return m_lightweightCache; }
        ECDbCR GetECDb() const { return m_ecdb; }
        IssueReporter const& Issues() const { return m_ecdb.GetECDbImplR().GetIssueReporter(); }
    };
//=======================================================================================
// @bsiclass                                                 Krischan.Eberle  04/2017
//+===============+===============+===============+===============+===============+======
struct ClassMapFactory final : NonCopyableClass
    {
private:
    ClassMapFactory();
    ~ClassMapFactory();

public:
    template<typename TClassMap>
    static ClassMapPtr CreateForLoading(ECDb const& ecdb, ECN::ECClassCR ecClass, MapStrategyExtendedInfo const& mapStrategy) { return new TClassMap(ecdb, ecClass, mapStrategy); }

    template<typename TClassMap>
    static ClassMapPtr CreateForMapping(ECDb const& ecdb, ECN::ECClassCR ecClass, MapStrategyExtendedInfo const& mapStrategy) { return new TClassMap(ecdb, ecClass, mapStrategy); }
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
