/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECInstanceId.h>
#include <BeSQLite/BeSQLite.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Direction of relationship traversal
// @bsienum
//+===============+===============+===============+===============+===============+======
enum class TraversalDirection : uint8_t
    {
    Forward  = 1,
    Backward = 2,
    Both     = 3
    };

//=======================================================================================
//! A related instance discovered during graph traversal
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct RelatedInstance final
    {
    private:
        ECInstanceKey       m_key;
        ECN::ECClassId      m_relClassId;
        TraversalDirection  m_direction;

    public:
        RelatedInstance() {}
        RelatedInstance(ECInstanceKeyCR key, ECN::ECClassId relClassId, TraversalDirection direction)
            : m_key(key), m_relClassId(relClassId), m_direction(direction) {}

        ECInstanceKeyCR GetKey() const { return m_key; }
        ECN::ECClassId GetRelClassId() const { return m_relClassId; }
        TraversalDirection GetDirection() const { return m_direction; }
    };

//=======================================================================================
//! Fast instance graph traversal that bypasses ECSql, going directly to raw SQLite
//! prepared statements built from property maps.
//!
//! @remarks The InstanceGraph lazily discovers applicable relationships, generates
//! raw SQLite SQL from property maps/class maps, and caches prepared statements at the
//! ECDb level for reuse across multiple InstanceGraph instances.
//!
//! Usage:
//! @code
//!     InstanceGraph graph(ecdb);
//!     graph.AddSeed(ECInstanceKey(classId, instanceId));
//!     graph.ExpandAll(3);  // expand up to 3 hops
//!     auto* related = graph.GetRelated(seedKey);
//! @endcode
//!
//! @see ECInstanceFinder for the older ECSql-based traversal
//! @ingroup ECDbGroup
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct InstanceGraph final
    {
    private:
        ECDbCR m_ecdb;
        bset<ECInstanceKey> m_visited;
        bmap<ECInstanceKey, bvector<RelatedInstance>> m_adjacency;
        bvector<ECInstanceKey> m_seeds;

        BentleyStatus ExpandNodeInternal(ECInstanceKeyCR key, TraversalDirection dir);

        //not copyable
        InstanceGraph(InstanceGraph const&) = delete;
        InstanceGraph& operator=(InstanceGraph const&) = delete;

        // Private ctor for set operations — creates graph without ECDb for pure data containers
        InstanceGraph(ECDbCR ecdb, bset<ECInstanceKey>&& visited, bmap<ECInstanceKey, bvector<RelatedInstance>>&& adjacency);

    public:
        //! Constructs a new InstanceGraph for the given ECDb
        ECDB_EXPORT explicit InstanceGraph(ECDbCR ecdb);
        ECDB_EXPORT ~InstanceGraph();

        //--- Building the graph ---

        //! Add a seed instance. No SQL is executed until Expand is called.
        //! @remarks Adding the same seed more than once has no additional effect.
        ECDB_EXPORT void AddSeed(ECInstanceKeyCR seed);

        //! Expand from a specific node in the given direction(s).
        //! @remarks The node becomes part of the graph even if it was never added as a seed.
        //! Re-expanding a node replaces its edges rather than duplicating them.
        //! @remarks A relationship whose source and target constraints overlap (e.g. an
        //! Element-to-Element relationship) is traversable in both directions. A self loop
        //! (source and target are the same instance) is therefore reported twice when
        //! TraversalDirection::Both is used: once as Forward and once as Backward.
        //! @return SUCCESS or ERROR if a statement fails
        ECDB_EXPORT BentleyStatus ExpandNode(ECInstanceKeyCR key, TraversalDirection dir = TraversalDirection::Both);

        //! Expand the full graph via BFS up to maxDepth hops.
        //! @param[in] maxDepth number of hops to traverse. 0 expands no relationship at all and
        //! only records the seeds, UINT8_MAX (the maximum) traverses up to 255 hops. Because
        //! already expanded nodes are never expanded again, traversal always terminates.
        //! @remarks Can be called repeatedly to deepen a graph. Nodes already expanded by a
        //! previous ExpandAll/ExpandNode call are not expanded again, but their neighbours are
        //! still traversed.
        //! @return SUCCESS or ERROR
        ECDB_EXPORT BentleyStatus ExpandAll(uint8_t maxDepth = UINT8_MAX);

        //--- Querying ---

        //! Check if a node exists in the graph
        bool Contains(ECInstanceKeyCR key) const { return m_visited.find(key) != m_visited.end(); }

        //! Number of unique nodes in the graph
        size_t NodeCount() const { return m_visited.size(); }

        //! Get all related instances of a node (after expansion). Returns nullptr if not expanded.
        bvector<RelatedInstance> const* GetRelated(ECInstanceKeyCR key) const
            {
            auto it = m_adjacency.find(key);
            return it != m_adjacency.end() ? &it->second : nullptr;
            }

        //! Get the visited set
        bset<ECInstanceKey> const& GetVisited() const { return m_visited; }

        //--- Set Operations ---
        //! @remarks All set operations require both graphs to belong to the same ECDb - instance
        //! keys of different files are not comparable. Passing graphs of different ECDbs asserts
        //! and returns false/nullptr.

        //! Do two graphs share any instance?
        ECDB_EXPORT static bool Overlaps(InstanceGraph const& a, InstanceGraph const& b);

        //! Instances present in both graphs (adjacency edges preserved only if both endpoints survive, duplicates removed)
        //! @return nullptr if the graphs belong to different ECDbs
        ECDB_EXPORT static std::unique_ptr<InstanceGraph> Intersection(InstanceGraph const& a, InstanceGraph const& b);

        //! Instances present in either graph (adjacency merged, duplicates deduplicated)
        //! @return nullptr if the graphs belong to different ECDbs
        ECDB_EXPORT static std::unique_ptr<InstanceGraph> Union(InstanceGraph const& a, InstanceGraph const& b);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
