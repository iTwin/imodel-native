/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(Node)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(Connection)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(WeightedGraph)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ElementPath)

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
//! A Graph node
//=======================================================================================
struct Node : RefCountedBase
    {
    bvector<ConnectionPtr> m_edges;
    BeInt64Id m_id;

    Node(BeInt64Id id) : m_id(id) {};

    bool operator==(Node const& other) const { return m_id == other.m_id; }
    bool operator!=(Node const& other) const { return m_id != other.m_id; }

    void AddEdge(ConnectionPtr edge) { m_edges.push_back(edge); }
    BUILDINGSHAREDDGNUTILS_EXPORT bool IsConnectedTo(BeInt64Id otherId);
    BUILDINGSHAREDDGNUTILS_EXPORT bvector<ConnectionPtr>::iterator GetConnectionTo(BeInt64Id id);

    BUILDINGSHAREDDGNUTILS_EXPORT static void AddTwoElementsToGraph(WeightedGraphPtr& graph, NodePtr& parentNode, NodePtr& childNode, BeInt64Id parent, BeInt64Id child);
    };

//=======================================================================================
//! A connection between graph nodes
//=======================================================================================
struct Connection : RefCountedBase
    {
    NodePtr m_neighbor;
    double m_weight;
    bool m_isPassable = true;

    bvector<ConnectionPtr>& GetNeighbourConnections() { return m_neighbor->m_edges; }
    BeInt64Id GetNeighbourId() { return m_neighbor->m_id; }
    bool IsPassable() { return m_isPassable; }
    void SetPassable(bool isPassable) { m_isPassable = isPassable; }

    Connection(NodePtr neighbor, double weight) : m_neighbor(neighbor), m_weight(weight) {};

    BUILDINGSHAREDDGNUTILS_EXPORT static void ConnectNodes(NodePtr a, NodePtr b, double weightAB, double weightBA);
    };

//=======================================================================================
//! A graph with weighted connections
//=======================================================================================
struct WeightedGraph : RefCountedBase
    {
    bmap<BeInt64Id, NodePtr> nodes;
    
    BUILDINGSHAREDDGNUTILS_EXPORT BentleyStatus FindShortestPath(bvector<NodeCPtr>& shortestPath, NodeCPtr source, NodeCPtr destination) const;

    void AddNode(NodePtr node) { nodes[node->m_id] = node; }
    BUILDINGSHAREDDGNUTILS_EXPORT BentleyStatus RemoveNode(BeInt64Id id);
    bool Contains(BeInt64Id id) { return nodes.end() != nodes.find(id); }
    NodePtr GetNode(BeInt64Id id) { return (Contains(id)) ? nodes[id] : nullptr; }
    BUILDINGSHAREDDGNUTILS_EXPORT void Merge(WeightedGraphCPtr rhs);
    };

END_BUILDING_SHARED_NAMESPACE