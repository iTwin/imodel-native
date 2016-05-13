/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbMap.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ClassMap.h"
#include "SchemaImportContext.h"
#include "ECDbSql.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct StorageDescription;

/*=================================================================================**//**
* @bsiclass                                                     Casey.Mullen      11/2011
+===============+===============+===============+===============+===============+======*/
struct ECDbMap :NonCopyableClass
    {
public:
    typedef bmap<ECDbSqlTable*, bset<ClassMap*>> ClassMapsByTable;

    struct LightweightCache : NonCopyableClass
        {
        public:
            enum class RelationshipEnd : int
                {
                None = 0,
                Source = 1,
                Target = 2,
                Both = Source | Target
                };
            enum class RelationshipType : int
                {
                Link = 0,
                Source = (int)ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable,
                Target = (int)ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable,
                };


            typedef bmap<ECN::ECClassId, RelationshipEnd> RelationshipClassIds;
            typedef bmap<ECN::ECClassId, RelationshipEnd> ConstraintClassIds;
            typedef bmap<ECN::ECClassId, RelationshipType> RelationshipTypeByClassId;
            typedef bmap<ECDbSqlTable const*, std::vector<ECN::ECClassId>> ClassIdsPerTableMap;
            typedef bmap<ECDbSqlTable const*, RelationshipTypeByClassId> RelationshipPerTable;
        private:
            mutable ClassIdsPerTableMap m_classIdsPerTable;
            mutable ClassIdsPerTableMap m_nonAbstractClassIdsPerTable;
            mutable bmap<ECN::ECClassId, ClassIdsPerTableMap> m_horizontalPartitions;
            mutable bmap<ECN::ECClassId, RelationshipClassIds> m_relationshipEndsByClassIdRev;
            mutable RelationshipClassIds m_anyClassRelationships;
            mutable bmap<ECN::ECClassId, RelationshipClassIds> m_relationshipClassIdsPerConstraintClassIds;
            mutable bmap<ECN::ECClassId, ConstraintClassIds> m_nonAnyClassConstraintClassIdsPerRelClassIds;
            mutable std::vector<ECN::ECClassId> m_anyClassReplacements;
            mutable ECN::ECClassId m_anyClassId;
            mutable std::map<ECN::ECClassId, std::unique_ptr<StorageDescription>> m_storageDescriptions;
            mutable RelationshipPerTable m_relationshipPerTable;
            mutable bmap<ECN::ECClassId, bset<ECDbSqlTable const*>> m_tablesPerClassId;
            mutable struct
                {
                bool m_horizontalPartitionsIsLoaded : 1;
                bool m_classIdsPerTableIsLoaded : 2;
                bool m_relationshipCacheIsLoaded : 3;
                bool m_anyClassRelationshipsIsLoaded : 4;
                bool m_anyClassReplacementsLoaded : 5;
                bool m_relationshipPerTableLoaded : 6;
                } m_loadedFlags;

            ECDbMapCR m_map;
            void LoadRelationshipByTable () const;
            void LoadHorizontalPartitions () const;
            void LoadClassIdsPerTable () const;
            void LoadAnyClassRelationships () const;
            void LoadRelationshipCache () const;
            void LoadAnyClassReplacements () const;
        public:
            explicit LightweightCache (ECDbMapCR map);
            ~LightweightCache () {}
            std::vector<ECN::ECClassId> const& GetClassesForTable (ECDbSqlTable const&) const;
            std::vector<ECN::ECClassId> const& GetNonAbstractClassesForTable(ECDbSqlTable const&) const;
            RelationshipTypeByClassId GetRelationshipsMapToTable (ECDbSqlTable const& table) const;
            RelationshipPerTable GetRelationshipsMapToTables () const;
            bset<ECDbSqlTable const*> const& GetVerticalPartitionsForClass(ECN::ECClassId classId) const;
            ClassIdsPerTableMap const& GetHorizontalPartitionsForClass (ECN::ECClassId) const;
            RelationshipClassIds const& GetRelationships (ECN::ECClassId relationshipId) const;
            RelationshipClassIds const& GetRelationshipsForConstraintClass (ECN::ECClassId constraintClassId) const;
            //Gets all the constraint class ids plus the constraint end that make up the relationship with the given class id.
            //@remarks: AnyClass constraints are ignored.
            ConstraintClassIds const& GetConstraintClassesForRelationship (ECN::ECClassId relClassId) const;
            RelationshipClassIds const& GetAnyClassRelationships () const;
            ECN::ECClassId GetAnyClassId () const;
            std::vector<ECN::ECClassId> const& GetAnyClassReplacements () const;
            //bmap<ECN::ECClassId, bset<ECDbSqlTable const*> const& GetTablesPerClass()
            //For a end table relationship class map, the storage description provides horizontal partitions
            //For the end table's constraint classes - not for the relationship itself.
            StorageDescription const& GetStorageDescription (IClassMap const&)  const;

            void Load (bool forceReload);
            void Reset ();
        };


private:
    mutable BeMutex m_mutex;

    ECDbR m_ecdb;
    ECDbSQLManager m_ecdbSqlManager;
    mutable bmap<ECN::ECClassId, ClassMapPtr> m_classMapDictionary;
    mutable LightweightCache m_lightweightCache;
    SchemaImportContext* m_schemaImportContext;

    bool TryGetClassMap(ClassMapPtr&, ClassMapLoadContext&, ECN::ECClassCR) const;
    ClassMapPtr DoGetClassMap(ECN::ECClassCR) const;

    ClassMapPtr LoadClassMap(ClassMapLoadContext& ctx, ECN::ECClassCR) const;

    MapStatus DoMapSchemas(bvector<ECN::ECSchemaCP> const&);
    MapStatus MapClass(ECN::ECClassCR);
    BentleyStatus FinishTableDefinition() const;
    BentleyStatus SaveMappings() const;
    BentleyStatus CreateOrUpdateRequiredTables() const;
    BentleyStatus EvaluateColumnNotNullConstraints() const;
    BentleyStatus CreateOrUpdateIndexesInDb() const;

    MapStatus AddClassMap(ClassMapPtr&) const;

    DbResult UpdateHoldingView();

    ClassMapsByTable GetClassMapsByTable() const;
    BentleyStatus GetClassMapsFromRelationshipEnd(std::set<ClassMap const*>&, ECN::ECClassCR, bool recursive) const;

public:
    explicit ECDbMap(ECDbR ecdb);
    ~ECDbMap() {}

    ClassMap const* GetClassMap(ECN::ECClassCR) const;
    ClassMap const* GetClassMap(ECN::ECClassId) const;

    std::vector<ECN::ECClassCP> GetClassesFromRelationshipEnd(ECN::ECRelationshipConstraintCR) const;
    std::set<ClassMap const*> GetClassMapsFromRelationshipEnd(ECN::ECRelationshipConstraintCR, bool* hasAnyClass) const;
    ECDbSqlTable const* GetPrimaryTable(ECDbSqlTable const& joinedTable) const;
    //!Loads the class maps if they were not loaded yet
    size_t GetTableCountOnRelationshipEnd(ECN::ECRelationshipConstraintCR) const;
    ECDbSqlTable const* GetFirstTableFromRelationshipEnd(ECN::ECRelationshipConstraintCR) const;

    ECDbSQLManager const& GetSQLManager() const { return m_ecdbSqlManager; }

    MapStatus MapSchemas(SchemaImportContext&, bvector<ECN::ECSchemaCP> const&);

    BentleyStatus CreateECClassViewsInDb() const;
    ECDbSqlTable* FindOrCreateTable(SchemaImportContext*, Utf8CP tableName, TableType, bool isVirtual, Utf8CP primaryKeyColumnName);

    void ClearCache();

    bool IsImportingSchema() const;
    SchemaImportContext* GetSchemaImportContext() const;
    bool AssertIfIsNotImportingSchema() const;
    ECDbSqlTable*  FindOrCreateTable(SchemaImportContext*, Utf8CP tableName, TableType, bool isVirtual, Utf8CP primaryKeyColumnName, ECDbSqlTable const* baseTable);

    LightweightCache const& GetLightweightCache() const { return m_lightweightCache; }
    ECDbR GetECDbR() const { return m_ecdb; }
    ECDbCR GetECDb()  const { return m_ecdb; }
    std::set<ECDbSqlTable const*> GetTablesFromRelationshipEnd(ECN::ECRelationshipConstraintCR relationshipEnd, EndTablesOptimizationOptions options) const;
    std::set<ECDbSqlTable const*> GetTablesFromRelationshipEndWithColumn(ECN::ECRelationshipConstraintCR relationshipEnd, Utf8CP column) const;

    static void ParsePropertyAccessString(bvector<Utf8String>&, Utf8CP propAccessString);
    };


    struct StorageDescription;
    //=======================================================================================
    //! Hold detail about how table partition is described for this class
    // @bsiclass                                               Affan.Khan           05/2015
    //+===============+===============+===============+===============+===============+======
    struct Partition
        {
    friend struct StorageDescription;

    private:
        ECDbSqlTable const* m_table;
        std::vector<ECN::ECClassId> m_partitionClassIds;
        std::vector<ECN::ECClassId> m_inversedPartitionClassIds;
        bool m_hasInversedPartitionClassIds;

        bool IsSharedTable() const { return m_partitionClassIds.size() + m_inversedPartitionClassIds.size() > 1; }

        void AddClassId(ECN::ECClassId classId) { m_partitionClassIds.push_back(classId); }
        void GenerateClassIdFilter(std::vector<ECN::ECClassId> const& tableClassIds);

    public:
        explicit Partition (ECDbSqlTable const& table) : m_table (&table), m_hasInversedPartitionClassIds (false) {}
        ~Partition() {}
        Partition(Partition const&);
        Partition& operator=(Partition const& rhs);
        Partition(Partition&& rhs);
        Partition& operator=(Partition&& rhs);

        ECDbSqlTable const& GetTable () const { return *m_table; }
        ECN::ECClassId GetRootClassId() const { BeAssert(!m_partitionClassIds.empty()); return m_partitionClassIds[0]; }
        std::vector<ECN::ECClassId> const& GetClassIds () const { return m_partitionClassIds; }
        bool NeedsECClassIdFilter() const;
        void AppendECClassIdFilterSql(Utf8CP classIdColName, NativeSqlBuilder&) const;
        };


    //=======================================================================================
    //! Represents storage description for a given class map and its derived classes for polymorphic queries
    // @bsiclass                                               Affan.Khan           05/2015
    //+===============+===============+===============+===============+===============+======
    struct StorageDescription : NonCopyableClass
        {
    private:
        ECN::ECClassId m_classId;
        std::vector<Partition> m_horizontalPartitions;
        std::vector<size_t> m_nonVirtualHorizontalPartitionIndices;
        std::vector<Partition> m_verticalPartitions;
        size_t m_rootHorizontalPartitionIndex;
        size_t m_rootVerticalPartitionIndex;

        explicit StorageDescription (ECN::ECClassId classId) : m_classId (classId), m_rootHorizontalPartitionIndex (0), m_rootVerticalPartitionIndex(0) {}

        Partition* AddHorizontalPartition(ECDbSqlTable const&, bool isRootPartition);
        Partition* AddVerticalPartition(ECDbSqlTable const&, bool isRootPartition);


    public:
        ~StorageDescription (){}
        StorageDescription (StorageDescription&&);
        StorageDescription& operator=(StorageDescription&&);
        std::vector<Partition> const& GetVerticalPartitions() const { return m_verticalPartitions; }

        //! Returns nullptr, if more than one non-virtual partitions exist.
        //! If polymorphic is true or has no non-virtual partitions, gets root horizontal partition.
        //! If has a single non-virtual partition returns that.
        Partition const* GetHorizontalPartition(bool polymorphic) const;
        Partition const& GetRootHorizontalPartition() const;
        Partition const& GetRootVerticalPartition() const;
        Partition const* GetVerticalPartition(ECDbSqlTable const&) const;
        Partition const* GetHorizontalPartition(ECDbSqlTable const&) const;

        std::vector<Partition> const& GetHorizontalPartitions() const { return m_horizontalPartitions; }
        bool HasNonVirtualPartitions() const { return !m_nonVirtualHorizontalPartitionIndices.empty(); }
        bool HierarchyMapsToMultipleTables() const { return m_nonVirtualHorizontalPartitionIndices.size() > 1; }
        ECN::ECClassId GetClassId () const { return m_classId; }

        BentleyStatus GenerateECClassIdFilter(NativeSqlBuilder& filter, ECDbSqlTable const&, ECDbSqlColumn const& classIdColumn, bool polymorphic, bool fullyQualifyColumnName = false, Utf8CP tableAlias =nullptr) const;
        static std::unique_ptr<StorageDescription> Create(IClassMap const&, ECDbMap::LightweightCache const& lwmc);
        };
    

//=======================================================================================
//!Purge Holding type relationship end.
// @bsiclass                                               Affan.Khan           01/2016
//+===============+===============+===============+===============+===============+======
struct RelationshipPurger
    {
private:
    typedef bmap<Utf8CP, Utf8String, CompareUtf8> SqlPerTableMap;

    std::vector<std::unique_ptr<Statement>> m_stmts;

    BentleyStatus Initialize(ECDbR);
    static Utf8String BuildSql(Utf8CP tableName, Utf8CP pkColumnName);

public:
    RelationshipPurger() {}
    ~RelationshipPurger() { Finalize(); }

    BentleyStatus Purge(ECDbR);
    void Finalize();
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
