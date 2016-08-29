/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/LightweightCache.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
struct StorageDescription;

struct LightweightCache final: NonCopyableClass
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
            Source = (int) MapStrategy::ForeignKeyRelationshipInSourceTable,
            Target = (int) MapStrategy::ForeignKeyRelationshipInTargetTable,
            };


        typedef bmap<ECN::ECClassId, RelationshipType> RelationshipTypeByClassId;
        typedef bmap<DbTable const*, std::vector<ECN::ECClassId>> ClassIdsPerTableMap;
        typedef bmap<DbTable const*, RelationshipTypeByClassId> RelationshipPerTable;

    private:
        mutable ClassIdsPerTableMap m_classIdsPerTable;
        mutable bmap<ECN::ECClassId, ClassIdsPerTableMap> m_horizontalPartitions;
        mutable bmap<ECN::ECClassId, bmap<ECN::ECClassId, RelationshipEnd>> m_relationshipClassIdsPerConstraintClassIds;
        mutable bmap<ECN::ECClassId, bmap<ECN::ECClassId, RelationshipEnd>> m_constraintClassIdsPerRelClassIds;
        mutable std::map<ECN::ECClassId, std::unique_ptr<StorageDescription>> m_storageDescriptions;
        mutable RelationshipPerTable m_relationshipPerTable;
        mutable bmap<ECN::ECClassId, bset<DbTable const*>> m_tablesPerClassId;

        ECDbMap const& m_map;
        ClassIdsPerTableMap const& LoadHorizontalPartitions(ECN::ECClassId classId)  const;
        bset<DbTable const*> const& LoadClassIdsPerTable(ECN::ECClassId iid) const;
        std::vector<ECN::ECClassId> const& LoadClassIdsPerTable(DbTable const& tbl) const;
        bmap<ECN::ECClassId, RelationshipEnd> const& LoadRelationshipConstraintClasses(ECN::ECClassId relationshipId) const;
        bmap<ECN::ECClassId, RelationshipEnd> const& LoadConstraintClassesForRelationships(ECN::ECClassId constraintClassId) const;

    public:
        explicit LightweightCache(ECDbMap const& map);
        ~LightweightCache() {}
        std::vector<ECN::ECClassId> const& GetClassesForTable(DbTable const&) const;
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
//=======================================================================================
//! Hold detail about how table partition is described for this class
// @bsiclass                                               Affan.Khan           05/2015
//+===============+===============+===============+===============+===============+======
struct Partition final
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
struct StorageDescription  final: NonCopyableClass
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
        static std::unique_ptr<StorageDescription> Create(ClassMap const&, LightweightCache const& lwmc);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

