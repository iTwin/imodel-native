/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/PublicApi/ShortestPath.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//#include <ConstraintSystem/Domain/ConstraintModelMacros.h>
//#include <DgnPlatform/DgnPlatformApi.h>
//#include <DgnPlatform/DgnElement.h>

BUILDING_REFCOUNTED_PTR_AND_TYPEDEFS(Node)
BUILDING_REFCOUNTED_PTR_AND_TYPEDEFS(Connection)
BUILDING_REFCOUNTED_PTR_AND_TYPEDEFS(WeightedGraph)
BUILDING_REFCOUNTED_PTR_AND_TYPEDEFS(ElementPath)

BEGIN_BUILDING_NAMESPACE

//=======================================================================================
//! A Graph node
//=======================================================================================
struct Node : RefCountedBase
    {
    bvector<ConnectionPtr> m_edges;
    BeInt64Id m_id;

    BUILDINGUTILS_EXPORT Node(BeInt64Id id) : m_id(id) {};

    BUILDINGUTILS_EXPORT bool operator==(Node const& other) const { return m_id == other.m_id; }
    BUILDINGUTILS_EXPORT bool operator!=(Node const& other) const { return m_id != other.m_id; }

    BUILDINGUTILS_EXPORT void AddEdge(ConnectionPtr edge) { m_edges.push_back(edge); }
    BUILDINGUTILS_EXPORT bool IsConnectedTo(BeInt64Id otherId);
    BUILDINGUTILS_EXPORT bvector<ConnectionPtr>::iterator GetConnectionTo(BeInt64Id id);

    BUILDINGUTILS_EXPORT static void AddTwoElementsToGraph(WeightedGraphPtr& graph, NodePtr& parentNode, NodePtr& childNode, BeInt64Id parent, BeInt64Id child);
    };

//=======================================================================================
//! A connection between graph nodes
//=======================================================================================
struct Connection : RefCountedBase
    {
    NodePtr m_neighbor;
    double m_weight;
    bool m_isPassable = true;

    BUILDINGUTILS_EXPORT bvector<ConnectionPtr>& GetNeighbourConnections() { return m_neighbor->m_edges; }
    BUILDINGUTILS_EXPORT BeInt64Id GetNeighbourId() { return m_neighbor->m_id; }
    BUILDINGUTILS_EXPORT bool IsPassable() { return m_isPassable; }
    BUILDINGUTILS_EXPORT void SetPassable(bool isPassable) { m_isPassable = isPassable; }

    BUILDINGUTILS_EXPORT Connection(NodePtr neighbor, double weight) : m_neighbor(neighbor), m_weight(weight) {};

    BUILDINGUTILS_EXPORT static void ConnectNodes(NodePtr a, NodePtr b, double weightAB, double weightBA);
    };

//=======================================================================================
//! A graph with weighted connections
//=======================================================================================
struct WeightedGraph : RefCountedBase
    {
    bmap<BeInt64Id, NodePtr> nodes;
    
    BUILDINGUTILS_EXPORT BentleyStatus FindShortestPath(bvector<NodeCPtr>& shortestPath, NodeCPtr source, NodeCPtr destination) const;

    BUILDINGUTILS_EXPORT void AddNode(NodePtr node) { nodes[node->m_id] = node; }
    BUILDINGUTILS_EXPORT BentleyStatus RemoveNode(BeInt64Id id);
    BUILDINGUTILS_EXPORT bool Contains(BeInt64Id id) { return nodes.end() != nodes.find(id); }
    BUILDINGUTILS_EXPORT NodePtr GetNode(BeInt64Id id) { return (Contains(id)) ? nodes[id] : nullptr; }
    BUILDINGUTILS_EXPORT void Merge(WeightedGraphCPtr rhs);
    };
END_BUILDING_NAMESPACE