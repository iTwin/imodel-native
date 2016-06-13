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
#include "DbSchema.h"
#include "IssueReporter.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct StorageDescription;

/*=================================================================================**//**
* @bsiclass                                                     Casey.Mullen      11/2011
+===============+===============+===============+===============+===============+======*/
struct ECDbMap :NonCopyableClass
    {
    public:
        typedef bmap<DbTable*, bset<ClassMap*>> ClassMapsByTable;

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
                    Source = (int) ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable,
                    Target = (int) ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable,
                    };


                typedef bmap<ECN::ECClassId, RelationshipType> RelationshipTypeByClassId;
                typedef bmap<DbTable const*, std::vector<ECN::ECClassId>> ClassIdsPerTableMap;
                typedef bmap<DbTable const*, RelationshipTypeByClassId> RelationshipPerTable;

            private:
                mutable ClassIdsPerTableMap m_classIdsPerTable;
                mutable ClassIdsPerTableMap m_nonAbstractClassIdsPerTable;
                mutable bmap<ECN::ECClassId, ClassIdsPerTableMap> m_horizontalPartitions;
                mutable bmap<ECN::ECClassId, bmap<ECN::ECClassId, RelationshipEnd>> m_relationshipClassIdsPerConstraintClassIds;
                mutable bmap<ECN::ECClassId, bmap<ECN::ECClassId, RelationshipEnd>> m_constraintClassIdsPerRelClassIds;
                mutable std::map<ECN::ECClassId, std::unique_ptr<StorageDescription>> m_storageDescriptions;
                mutable RelationshipPerTable m_relationshipPerTable;
                mutable bmap<ECN::ECClassId, bset<DbTable const*>> m_tablesPerClassId;
                mutable struct
                    {
                    bool m_horizontalPartitionsIsLoaded : 1;
                    bool m_classIdsPerTableIsLoaded : 2;
                    bool m_relationshipCacheIsLoaded : 3;
                    bool m_relationshipPerTableLoaded : 4;
                    } m_loadedFlags;

                ECDbMap const& m_map;
                void LoadHorizontalPartitions() const;
                void LoadClassIdsPerTable() const;
                void LoadRelationshipCache() const;
            public:
                explicit LightweightCache(ECDbMap const& map);
                ~LightweightCache() {}
                std::vector<ECN::ECClassId> const& GetClassesForTable(DbTable const&) const;
                std::vector<ECN::ECClassId> const& GetNonAbstractClassesForTable(DbTable const&) const;
                bset<DbTable const*> const& GetVerticalPartitionsForClass(ECN::ECClassId classId) const;
                ClassIdsPerTableMap const& GetHorizontalPartitionsForClass(ECN::ECClassId) const;
                //Gets all the constraint class ids plus the constraint end that make up the relationship with the given class id.
                //@remarks: AnyClass constraints are ignored.
                bmap<ECN::ECClassId, RelationshipEnd> const& GetConstraintClassesForRelationshipClass(ECN::ECClassId relClassId) const;
                //For a end table relationship class map, the storage description provides horizontal partitions
                //For the end table's constraint classes - not for the relationship itself.
                StorageDescription const& GetStorageDescription(ClassMap const&)  const;
                void Reset();
            };

    private:
        mutable BeMutex m_mutex;

        ECDbCR m_ecdb;
        DbSchema m_dbSchema;

        mutable bmap<ECN::ECClassId, ClassMapPtr> m_classMapDictionary;
        mutable LightweightCache m_lightweightCache;
        SchemaImportContext* m_schemaImportContext;

        BentleyStatus TryGetClassMap(ClassMapPtr&, ClassMapLoadContext&, ECN::ECClassCR) const;
        ClassMapPtr DoGetClassMap(ECN::ECClassCR) const;

        BentleyStatus TryLoadClassMap(ClassMapPtr&, ClassMapLoadContext& ctx, ECN::ECClassCR) const;

        MappingStatus DoMapSchemas();
        MappingStatus MapClass(ECN::ECClassCR);
        BentleyStatus SaveDbSchema() const;
        BentleyStatus CreateOrUpdateRequiredTables() const;
        BentleyStatus EvaluateColumnNotNullConstraints() const;
        BentleyStatus CreateOrUpdateIndexesInDb() const;
        BentleyStatus PurgeOrphanTables() const;
        BentleyStatus PurgeOrphanColumns() const;
        BentleyStatus FinishTableDefinitions(bool onlyCreateClassIdColumns = false) const;
        BentleyStatus CreateClassIdColumnIfNecessary(DbTable&, bset<ClassMap*> const&) const;
        MappingStatus AddClassMap(ClassMapPtr&) const;
        ClassMapsByTable GetClassMapsByTable() const;
        BentleyStatus GetClassMapsFromRelationshipEnd(std::set<ClassMap const*>&, ECN::ECClassCR, bool recursive) const;
        std::vector<ECN::ECClassCP> GetBaseClassesNotAlreadyMapped(ECN::ECClassCR ecclass) const;
        static void GatherRootClasses(ECN::ECClassCR ecclass, std::set<ECN::ECClassCP>& doneList, std::set<ECN::ECClassCP>& rootClassSet, std::vector<ECN::ECClassCP>& rootClassList, std::vector<ECN::ECRelationshipClassCP>& rootRelationshipList);

    public:
        explicit ECDbMap(ECDbCR ecdb);
        ~ECDbMap() {}

        ClassMap const* GetClassMap(ECN::ECClassCR) const;
        ClassMap const* GetClassMap(ECN::ECClassId) const;
        std::vector<ECN::ECClassCP> GetClassesFromRelationshipEnd(ECN::ECRelationshipConstraintCR) const;
        std::set<ClassMap const*> GetClassMapsFromRelationshipEnd(ECN::ECRelationshipConstraintCR, bool* hasAnyClass) const;
        DbTable const* GetPrimaryTable(DbTable const& joinedTable) const;
        //!Loads the class maps if they were not loaded yet
        size_t GetTableCountOnRelationshipEnd(ECN::ECRelationshipConstraintCR) const;
        MappingStatus MapSchemas(SchemaImportContext&);
        BentleyStatus CreateECClassViewsInDb() const;
        DbTable* FindOrCreateTable(SchemaImportContext*, Utf8CP tableName, DbTable::Type, bool isVirtual, Utf8CP primaryKeyColumnName);
        void ClearCache();
        bool IsImportingSchema() const;
        SchemaImportContext* GetSchemaImportContext() const;
        bool AssertIfIsNotImportingSchema() const;
        DbTable* FindOrCreateTable(SchemaImportContext*, Utf8CP tableName, DbTable::Type, bool isVirtual, Utf8CP primaryKeyColumnName, DbTable const* baseTable);
        DbSchema const& GetDbSchema() const { return m_dbSchema; }
        DbSchema& GetDbSchemaR() const { return const_cast<DbSchema&> (m_dbSchema); }
        LightweightCache const& GetLightweightCache() const { return m_lightweightCache; }
        ECDbCR GetECDb() const { return m_ecdb; }
        IssueReporter const& Issues() const;
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
        DbTable const* m_table;
        std::vector<ECN::ECClassId> m_partitionClassIds;
        std::vector<ECN::ECClassId> m_inversedPartitionClassIds;
        bool m_hasInversedPartitionClassIds;

        bool IsSharedTable() const { return m_partitionClassIds.size() + m_inversedPartitionClassIds.size() > 1; }

        void AddClassId(ECN::ECClassId classId) { m_partitionClassIds.push_back(classId); }
        void GenerateClassIdFilter(std::vector<ECN::ECClassId> const& tableClassIds);

    public:
        explicit Partition(DbTable const& table) : m_table(&table), m_hasInversedPartitionClassIds(false) {}
        ~Partition() {}
        Partition(Partition const&);
        Partition& operator=(Partition const& rhs);
        Partition(Partition&& rhs);

        DbTable const& GetTable() const { return *m_table; }
        ECN::ECClassId GetRootClassId() const { BeAssert(!m_partitionClassIds.empty()); return m_partitionClassIds[0]; }
        std::vector<ECN::ECClassId> const& GetClassIds() const { return m_partitionClassIds; }
        bool NeedsECClassIdFilter() const;
        void AppendECClassIdFilterSql(Utf8StringR filterSql, Utf8CP classIdColName) const;
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

        explicit StorageDescription(ECN::ECClassId classId) : m_classId(classId), m_rootHorizontalPartitionIndex(0), m_rootVerticalPartitionIndex(0) {}

        Partition* AddHorizontalPartition(DbTable const&, bool isRootPartition);
        Partition* AddVerticalPartition(DbTable const&, bool isRootPartition);


    public:
        ~StorageDescription() {}
        StorageDescription(StorageDescription&&);
        std::vector<Partition> const& GetVerticalPartitions() const { return m_verticalPartitions; }

        //! Returns nullptr, if more than one non-virtual partitions exist.
        //! If polymorphic is true or has no non-virtual partitions, gets root horizontal partition.
        //! If has a single non-virtual partition returns that.
        Partition const* GetHorizontalPartition(bool polymorphic) const;
        Partition const& GetRootHorizontalPartition() const;
        Partition const* GetVerticalPartition(DbTable const&) const;
        Partition const* GetHorizontalPartition(DbTable const&) const;

        std::vector<Partition> const& GetHorizontalPartitions() const { return m_horizontalPartitions; }
        bool HasNonVirtualPartitions() const { return !m_nonVirtualHorizontalPartitionIndices.empty(); }
        bool HierarchyMapsToMultipleTables() const { return m_nonVirtualHorizontalPartitionIndices.size() > 1; }
        ECN::ECClassId GetClassId() const { return m_classId; }

        BentleyStatus GenerateECClassIdFilter(Utf8StringR filterSqlExpression, DbTable const&, DbColumn const& classIdColumn, bool polymorphic, bool fullyQualifyColumnName = false, Utf8CP tableAlias = nullptr) const;
        static std::unique_ptr<StorageDescription> Create(ClassMap const&, ECDbMap::LightweightCache const& lwmc);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
