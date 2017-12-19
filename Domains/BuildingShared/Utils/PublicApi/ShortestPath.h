/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/PublicApi/ShortestPath.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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

    BUILDINGSHAREDUTILS_EXPORT Node(BeInt64Id id) : m_id(id) {};

    BUILDINGSHAREDUTILS_EXPORT bool operator==(Node const& other) const { return m_id == other.m_id; }
    BUILDINGSHAREDUTILS_EXPORT bool operator!=(Node const& other) const { return m_id != other.m_id; }

    BUILDINGSHAREDUTILS_EXPORT void AddEdge(ConnectionPtr edge) { m_edges.push_back(edge); }
    BUILDINGSHAREDUTILS_EXPORT bool IsConnectedTo(BeInt64Id otherId);
    BUILDINGSHAREDUTILS_EXPORT bvector<ConnectionPtr>::iterator GetConnectionTo(BeInt64Id id);

    BUILDINGSHAREDUTILS_EXPORT static void AddTwoElementsToGraph(WeightedGraphPtr& graph, NodePtr& parentNode, NodePtr& childNode, BeInt64Id parent, BeInt64Id child);
    };

//=======================================================================================
//! A connection between graph nodes
//=======================================================================================
struct Connection : RefCountedBase
    {
    NodePtr m_neighbor;
    double m_weight;
    bool m_isPassable = true;

    BUILDINGSHAREDUTILS_EXPORT bvector<ConnectionPtr>& GetNeighbourConnections() { return m_neighbor->m_edges; }
    BUILDINGSHAREDUTILS_EXPORT BeInt64Id GetNeighbourId() { return m_neighbor->m_id; }
    BUILDINGSHAREDUTILS_EXPORT bool IsPassable() { return m_isPassable; }
    BUILDINGSHAREDUTILS_EXPORT void SetPassable(bool isPassable) { m_isPassable = isPassable; }

    BUILDINGSHAREDUTILS_EXPORT Connection(NodePtr neighbor, double weight) : m_neighbor(neighbor), m_weight(weight) {};

    BUILDINGSHAREDUTILS_EXPORT static void ConnectNodes(NodePtr a, NodePtr b, double weightAB, double weightBA);
    };

//=======================================================================================
//! A graph with weighted connections
//=======================================================================================
struct WeightedGraph : RefCountedBase
    {
    bmap<BeInt64Id, NodePtr> nodes;
    
    BUILDINGSHAREDUTILS_EXPORT BentleyStatus FindShortestPath(bvector<NodeCPtr>& shortestPath, NodeCPtr source, NodeCPtr destination) const;

    BUILDINGSHAREDUTILS_EXPORT void AddNode(NodePtr node) { nodes[node->m_id] = node; }
    BUILDINGSHAREDUTILS_EXPORT BentleyStatus RemoveNode(BeInt64Id id);
    BUILDINGSHAREDUTILS_EXPORT bool Contains(BeInt64Id id) { return nodes.end() != nodes.find(id); }
    BUILDINGSHAREDUTILS_EXPORT NodePtr GetNode(BeInt64Id id) { return (Contains(id)) ? nodes[id] : nullptr; }
    BUILDINGSHAREDUTILS_EXPORT void Merge(WeightedGraphCPtr rhs);
    };

END_BUILDING_SHARED_NAMESPACE