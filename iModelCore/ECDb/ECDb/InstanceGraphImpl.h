/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/InstanceGraph.h>
#include "ECDbInternalTypes.h"
#include "ClassMap.h"
#include "RelationshipClassMap.h"
#include "LightweightCache.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Describes a relationship that can be traversed from a given entity class
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ApplicableRelationship final
    {
    ECN::ECRelationshipClassCP  m_relClass;         //!< Base relationship class
    ECN::ECRelationshipEnd      m_thisEnd;           //!< Which end "our" class sits on (Source or Target)
    ClassMap::Type              m_mapType;            //!< LinkTable or EndTable
    MapStrategy                 m_mapStrategy;        //!< For EndTable: ForeignKeyRelationshipInSourceTable or TargetTable

    ApplicableRelationship(ECN::ECRelationshipClassCR relClass, ECN::ECRelationshipEnd thisEnd, ClassMap::Type mapType, MapStrategy mapStrategy)
        : m_relClass(&relClass), m_thisEnd(thisEnd), m_mapType(mapType), m_mapStrategy(mapStrategy) {}
    };

//=======================================================================================
//! Key for looking up cached graph SQL in GraphStatementCache
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct GraphStatementKey final
    {
    ECN::ECClassId          m_relClassId;
    ECN::ECRelationshipEnd  m_thisEnd;
    TraversalDirection      m_direction;
    size_t                  m_partitionIdx;

    bool operator==(GraphStatementKey const& rhs) const
        {
        return m_relClassId == rhs.m_relClassId && m_thisEnd == rhs.m_thisEnd
            && m_direction == rhs.m_direction && m_partitionIdx == rhs.m_partitionIdx;
        }
    };

struct GraphStatementKeyHash final
    {
    size_t operator()(GraphStatementKey const& k) const
        {
        size_t h = std::hash<uint64_t>{}(k.m_relClassId.GetValueUnchecked());
        h ^= std::hash<int>{}((int)k.m_thisEnd) << 1;
        h ^= std::hash<int>{}((int)k.m_direction) << 2;
        h ^= std::hash<size_t>{}(k.m_partitionIdx) << 3;
        return h;
        }
    };

//=======================================================================================
//! Cached SQL entry with metadata for processing results
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct GraphStatementEntry final
    {
    Utf8String      m_sql;

    // Column index mapping: -1 means virtual (use static value)
    int             m_relatedInstanceIdColIdx;
    int             m_relatedClassIdColIdx;  //!< -1 if virtual
    ECN::ECClassId  m_staticRelatedClassId;  //!< used when relatedClassIdColIdx == -1

    int             m_relClassIdColIdx;       //!< -1 if virtual
    ECN::ECClassId  m_staticRelClassId;       //!< used when relClassIdColIdx == -1

    TraversalDirection m_direction;
    };

//=======================================================================================
//! ECDb-level cache for graph traversal SQL and relationship discovery.
//! Pinned (not LRU), keyed by relationship + direction + partition.
//! Cleared when ClearECDbCache() is called.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct GraphStatementCache final
    {
    friend struct InstanceGraph;

    public:
        ECDbCR m_ecdb;

    private:
        // SQL text + metadata cache (the expensive part to compute: walking property maps)
        std::unordered_map<GraphStatementKey, GraphStatementEntry, GraphStatementKeyHash> m_entries;

        // Relationship discovery cache: ECClassId → applicable relationships
        bmap<ECN::ECClassId, bvector<ApplicableRelationship>> m_relDiscoveryCache;

        GraphStatementCache(GraphStatementCache const&) = delete;
        GraphStatementCache& operator=(GraphStatementCache const&) = delete;

        // SQL generation helpers
        static void AppendClassHierarchyJoin(Utf8StringR sql, Utf8CP alias, Utf8CP columnExpr, ECN::ECClassId baseClassId, int& joinIdx);
        static void AppendClassHierarchyFilter(Utf8StringR sql, Utf8CP columnName, ECN::ECClassId baseClassId);

        BentleyStatus BuildLinkTableSql(GraphStatementEntry& entry, RelationshipClassMap const& relMap, TraversalDirection dir);
        BentleyStatus BuildEndTableSql(GraphStatementEntry& entry, ForeignKeyPartitionView::Partition const& partition,
                                       ForeignKeyPartitionView const& fkView, ECN::ECRelationshipClassCR relClass,
                                       TraversalDirection dir);

        BentleyStatus DiscoverRelationshipsForClass(bvector<ApplicableRelationship>& out, ECN::ECClassId entityClassId);

    public:
        explicit GraphStatementCache(ECDbCR ecdb) : m_ecdb(ecdb) {}
        ~GraphStatementCache() {}

        void Clear() { m_entries.clear(); m_relDiscoveryCache.clear(); }

        //! Get or discover applicable relationships for an entity class
        bvector<ApplicableRelationship> const& GetApplicableRelationships(ECN::ECClassId entityClassId);

        //! Get or build the SQL entry for a relationship traversal
        GraphStatementEntry const* GetOrBuildEntry(ApplicableRelationship const& rel, TraversalDirection dir, size_t partitionIdx = 0);

        //! Get all partition entries for an end-table relationship
        bvector<GraphStatementEntry const*> GetEndTableEntries(ApplicableRelationship const& rel, TraversalDirection dir);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
