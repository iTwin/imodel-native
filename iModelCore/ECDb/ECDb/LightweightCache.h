/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/LightweightCache.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct TableSpaceSchemaManager;
struct StorageDescription;

//======================================================================================
// @bsiclass                                                 Affan.Khan         09/2014
//======================================================================================
struct LightweightCache final
    {
    public:
        enum class RelationshipEnd
            {
            Source = 1,
            Target = 2,
            Both = Source | Target
            };

        enum class RelationshipType
            {
            Link = 0,
            Source = (int) MapStrategy::ForeignKeyRelationshipInSourceTable,
            Target = (int) MapStrategy::ForeignKeyRelationshipInTargetTable,
            };

        struct CompareDbTableById
            {
            bool operator()(DbTable const * lhs, DbTable const * rhs) const { return lhs->GetId() < rhs->GetId(); }
            };

        typedef bmap<ECN::ECClassId, RelationshipType> RelationshipTypeByClassId;
        typedef bmap<DbTable const*, std::vector<ECN::ECClassId>, CompareDbTableById> ClassIdsPerTableMap;
        typedef bmap<DbTable const*, RelationshipTypeByClassId, CompareDbTableById> RelationshipPerTable;

    private:
        TableSpaceSchemaManager const& m_schemaManager;

        mutable ClassIdsPerTableMap m_classIdsPerTable;
        mutable bmap<ECN::ECClassId, ClassIdsPerTableMap> m_horizontalPartitions;
        mutable bmap<ECN::ECClassId, bmap<ECN::ECClassId, RelationshipEnd>> m_constraintClassIdsPerRelClassIds;
        mutable std::map<ECN::ECClassId, std::unique_ptr<StorageDescription>> m_storageDescriptions;
        mutable RelationshipPerTable m_relationshipPerTable;
        mutable bmap<ECN::ECClassId, bset<DbTable const*>> m_tablesPerClassId;

        LightweightCache(LightweightCache const&) = delete;
        LightweightCache& operator=(LightweightCache const&) = delete;

        ClassIdsPerTableMap const& LoadHorizontalPartitions(ECN::ECClassId)  const;
        bset<DbTable const*> const& LoadTablesForClassId(ECN::ECClassId) const;
        std::vector<ECN::ECClassId> const& LoadClassIdsPerTable(DbTable const&) const;
        bmap<ECN::ECClassId, RelationshipEnd> const& LoadConstraintClassesForRelationships(ECN::ECClassId constraintClassId) const;
        
        CachedStatementPtr GetCachedStatement(Utf8CP sql) const;

        BeMutex& GetECDbMutex() const;
        ECDb& GetECDbR() const;

    public:
        explicit LightweightCache(TableSpaceSchemaManager const&);
        ~LightweightCache() {}
        std::vector<ECN::ECClassId> const& GetClassesForTable(DbTable const&) const;
        bset<DbTable const*> const& GetVerticalPartitionsForClass(ECN::ECClassId) const;
        ClassIdsPerTableMap const& GetHorizontalPartitionsForClass(ECN::ECClassId) const;
        //Gets all the constraint class ids plus the constraint end that make up the relationship with the given class id.
        bmap<ECN::ECClassId, RelationshipEnd> const& GetConstraintClassesForRelationshipClass(ECN::ECClassId relClassId) const;

        //For a end table relationship class map, the storage description provides horizontal partitions
        //For the end table's constraint classes - not for the relationship itself.
        StorageDescription const& GetStorageDescription(ClassMap const&)  const;
        void Clear();
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

        void AddClassId(ECN::ECClassId classId) { m_partitionClassIds.push_back(classId); }
        void GenerateClassIdFilter(std::vector<ECN::ECClassId> const& tableClassIds);

    public:
        explicit Partition(DbTable const& table) : m_table(&table), m_hasInversedPartitionClassIds(false) {}
        ~Partition() {}
        Partition(Partition const&);
        Partition& operator=(Partition const& rhs);
        Partition(Partition&& rhs);
        bool IsSharedTable() const { return m_partitionClassIds.size() + m_inversedPartitionClassIds.size() > 1; }
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
struct StorageDescription final
    {
    private:
        ECN::ECClassId m_classId;
        std::vector<Partition> m_horizontalPartitions;
        std::vector<size_t> m_nonVirtualHorizontalPartitionIndices;
        std::vector<Partition> m_verticalPartitions;
        size_t m_rootHorizontalPartitionIndex;
        size_t m_rootVerticalPartitionIndex;

        explicit StorageDescription(ECN::ECClassId classId) : m_classId(classId), m_rootHorizontalPartitionIndex(0), m_rootVerticalPartitionIndex(0) {}
        //not copyable
        StorageDescription(StorageDescription const&) = delete;
        StorageDescription& operator=(StorageDescription const&) = delete;

        Partition* AddHorizontalPartition(DbTable const&, bool isRootPartition);
        Partition* AddVerticalPartition(DbTable const&, bool isRootPartition);

    public:
        static std::unique_ptr<StorageDescription> Create(ClassMap const&, LightweightCache const& lwmc);

        ~StorageDescription() {}

        std::vector<Partition> const& GetVerticalPartitions() const { return m_verticalPartitions; }

        Partition const& GetRootHorizontalPartition() const;
        Partition const* GetVerticalPartition(DbTable const&) const;
        Partition const* GetHorizontalPartition(DbTable const&) const;
        //!First attempt to get horizontal partition if fails then try vertical partition and if that fail it assert and return nullptr
        Partition const* GetPartition(DbTable const&) const;

        std::vector<Partition> const& GetHorizontalPartitions() const { return m_horizontalPartitions; }
        bool HasNonVirtualHorizontalPartitions() const { return !m_nonVirtualHorizontalPartitionIndices.empty(); }
        bool HasMultipleNonVirtualHorizontalPartitions() const { return m_nonVirtualHorizontalPartitionIndices.size() > 1; }
        ECN::ECClassId GetClassId() const { return m_classId; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

