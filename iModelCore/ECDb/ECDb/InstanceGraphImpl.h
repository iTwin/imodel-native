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

    //! Set when no SQL could be generated for this relationship mapping. Such entries are
    //! cached so that the (failed) analysis is not repeated, and are skipped during traversal
    //! instead of failing the whole traversal.
    bool            m_unsupported = false;

    // Column index mapping: -1 means virtual (use static value)
    int             m_relatedInstanceIdColIdx = -1;
    int             m_relatedClassIdColIdx = -1;  //!< -1 if virtual
    ECN::ECClassId  m_staticRelatedClassId;       //!< used when relatedClassIdColIdx == -1

    int             m_relClassIdColIdx = -1;      //!< -1 if virtual
    ECN::ECClassId  m_staticRelClassId;           //!< used when relClassIdColIdx == -1

    int             m_relInstanceIdColIdx = -1;   //!< ECInstanceId of the relationship instance, -1 if not selectable

    //! SQL parameter index the seed's exact ECClassId must be bound to, or -1 when the SQL
    //! does not filter on it.
    int             m_seedClassIdParamIdx = -1;

    TraversalDirection m_direction = TraversalDirection::Both;
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
        mutable BeMutex m_mutex;

        // SQL text + metadata cache (the expensive part to compute: walking property maps)
        std::unordered_map<GraphStatementKey, GraphStatementEntry, GraphStatementKeyHash> m_entries;

        // Relationship discovery cache: ECClassId → applicable relationships
        bmap<ECN::ECClassId, bvector<ApplicableRelationship>> m_relDiscoveryCache;

        GraphStatementCache(GraphStatementCache const&) = delete;
        GraphStatementCache& operator=(GraphStatementCache const&) = delete;

        // SQL generation helpers
        static void AppendClassHierarchyFilter(Utf8StringR sql, Utf8CP columnExpr, ECN::ECClassId baseClassId);
        static void AppendQualifiedTableName(Utf8StringR sql, DbTable const& table);

        BentleyStatus BuildLinkTableSql(GraphStatementEntry& entry, RelationshipClassMap const& relMap, TraversalDirection dir);
        BentleyStatus BuildEndTableSql(GraphStatementEntry& entry, ForeignKeyPartitionView::Partition const& partition,
                                       ForeignKeyPartitionView const& fkView, ECN::ECRelationshipClassCR relClass,
                                       TraversalDirection dir);

        BentleyStatus DiscoverRelationshipsForClass(bvector<ApplicableRelationship>& out, ECN::ECClassId entityClassId);

        //! Caller must hold m_mutex.
        BentleyStatus GetOrBuildEntryUnsafe(GraphStatementEntry const*& out, ApplicableRelationship const& rel, TraversalDirection dir, size_t partitionIdx);

    public:
        explicit GraphStatementCache(ECDbCR ecdb) : m_ecdb(ecdb) {}
        ~GraphStatementCache() {}

        void Clear() { BeMutexHolder holder(m_mutex); m_entries.clear(); m_relDiscoveryCache.clear(); }

        //! Get or discover applicable relationships for an entity class.
        //! @remarks The result is copied out, but the ECRelationshipClass pointers it contains
        //! are owned by the schema cache. Callers must therefore not clear the ECDb cache or
        //! import schemas while a traversal is in progress.
        //! @return ERROR if discovery failed. Failed discoveries are not cached.
        BentleyStatus GetApplicableRelationships(bvector<ApplicableRelationship>& out, ECN::ECClassId entityClassId);

        //! Get or build the SQL entry for a relationship traversal. The entry is copied out so
        //! that the caller can keep using it after the cache was cleared.
        //! @remarks Relationship mappings for which no SQL can be generated yield an entry with
        //! GraphStatementEntry::m_unsupported set rather than an error.
        BentleyStatus GetOrBuildEntry(GraphStatementEntry& out, ApplicableRelationship const& rel, TraversalDirection dir, size_t partitionIdx = 0);

        //! Get all partition entries for an end-table relationship. The entries are copied out.
        BentleyStatus GetEndTableEntries(bvector<GraphStatementEntry>& out, ApplicableRelationship const& rel, TraversalDirection dir);
    };

//=======================================================================================
//! Identifies a single edge of the graph. Used to suppress duplicates.
//! Duplicates arise naturally because a relationship class and its base classes can all be
//! applicable to the same seed, and each of them matches the very same persisted row.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct GraphEdgeKey final
    {
    ECInstanceKey       m_related;
    ECN::ECClassId      m_relClassId;
    ECInstanceId        m_relInstanceId;
    TraversalDirection  m_direction;

    explicit GraphEdgeKey(RelatedInstance const& rel)
        : m_related(rel.GetKey()), m_relClassId(rel.GetRelClassId()), m_relInstanceId(rel.GetRelInstanceId()), m_direction(rel.GetDirection()) {}

    bool operator<(GraphEdgeKey const& rhs) const
        {
        if (m_related != rhs.m_related)
            return m_related < rhs.m_related;
        if (m_relClassId != rhs.m_relClassId)
            return m_relClassId < rhs.m_relClassId;
        if (m_relInstanceId != rhs.m_relInstanceId)
            return m_relInstanceId < rhs.m_relInstanceId;
        return (uint8_t) m_direction < (uint8_t) rhs.m_direction;
        }
    };

//=======================================================================================
//! Streams the related instances of a single seed instance one row at a time, without
//! materializing the whole result set.
//! @remarks Duplicate edges are suppressed, so a (comparatively small) set of already
//! returned edge keys is kept. The related instances themselves are never accumulated.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct GraphTraversalIterator final
    {
    private:
        ECDbCR              m_ecdb;
        ECInstanceKey       m_seed;
        bvector<bpair<GraphStatementEntry, TraversalDirection>> m_plan;
        size_t              m_planIdx = 0;
        CachedStatementPtr  m_stmt;
        bset<GraphEdgeKey>  m_seen;
        RelatedInstance     m_current;
        bool                m_eof = true;

        GraphTraversalIterator(GraphTraversalIterator const&) = delete;
        GraphTraversalIterator& operator=(GraphTraversalIterator const&) = delete;

    public:
        explicit GraphTraversalIterator(ECDbCR ecdb) : m_ecdb(ecdb) {}

        //! Prepares the iterator for a new seed. The iterator is positioned before the first row;
        //! call MoveNext() to advance to it.
        BentleyStatus Reset(ECInstanceKeyCR seed, TraversalDirection dir);

        //! Advances to the next distinct edge. Sets EOF when there is none left.
        BentleyStatus MoveNext();

        bool IsEof() const { return m_eof; }
        RelatedInstance const& GetCurrent() const { return m_current; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
