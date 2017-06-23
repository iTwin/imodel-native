/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/PublicApi/ShortestPath.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <BuildingCore.h>
#include <DgnPlatform/DgnElement.h>

BUILDING_REFCOUNTED_PTR_AND_TYPEDEFS(Node)
BUILDING_REFCOUNTED_PTR_AND_TYPEDEFS(Connection)
BUILDING_REFCOUNTED_PTR_AND_TYPEDEFS(WeightedGraph)

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
    };

//=======================================================================================
//! A connection between graph nodes
//=======================================================================================
struct Connection : RefCountedBase
    {
    NodePtr m_neighbor;
    double m_weight;

    BUILDINGUTILS_EXPORT bvector<ConnectionPtr>& GetNeighbourConnections() { return m_neighbor->m_edges; }
    BUILDINGUTILS_EXPORT BeInt64Id GetNeighbourId() { return m_neighbor->m_id; }

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

//=======================================================================================
//! Used for returning a shortest path
//=======================================================================================
struct ElementPath
    {
    CurveVectorPtr m_curveVector;
    bvector<Dgn::DgnElementId> m_passedNodes;
    bool m_isShortest;

    ElementPath() :m_curveVector(nullptr), m_passedNodes(bvector<Dgn::DgnElementId>()), m_isShortest(true) {}
    ElementPath(CurveVectorPtr curveVector, bvector<Dgn::DgnElementId> passedNodes, bool isShortest) :m_curveVector(curveVector), m_passedNodes(passedNodes), m_isShortest(isShortest){}
    CurveVectorPtr& CurveVector() { return m_curveVector; }
    bvector<Dgn::DgnElementId>& PassedNodes() { return m_passedNodes; }
    Dgn::DgnElementId& Source() { return m_passedNodes.front(); }
    Dgn::DgnElementId& Target() { return m_passedNodes.back(); }
    bvector<Dgn::DgnElementId> InBetweenNodes()
        {
        bvector<Dgn::DgnElementId> nodes = m_passedNodes;
        nodes.erase(nodes.begin());
        nodes.pop_back();
        return nodes;
        }
    bool& IsShortest() { return m_isShortest; }
    };
END_BUILDING_NAMESPACE