/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "PublicApi/BuildingDgnUtilsApi.h"

BEGIN_BUILDING_SHARED_NAMESPACE

#define UNDEFINED nullptr
#define DISTANCE_INFINITY DBL_MAX

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              05/2017
//--------------+---------------+---------------+---------------+---------------+--------
NodePtr FindNearestToSourceInQueue(bvector<NodePtr> queue, bmap<BeInt64Id, double> distancesToSource)
    {
    double minDistance = DISTANCE_INFINITY;
    NodePtr returnElem = nullptr;

    for (NodePtr node : queue)
        if (distancesToSource.end() != distancesToSource.find(node->m_id))
            if (distancesToSource[node->m_id] < minDistance)
                {
                minDistance = distancesToSource[node->m_id];
                returnElem = node;
                }

    return returnElem;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              05/2017
//--------------+---------------+---------------+---------------+---------------+--------
bvector<NodeCPtr> ReadShortestPath(bmap<BeInt64Id, NodePtr> previousNodes, NodeCPtr destination)
    {
    bvector<NodeCPtr> shortestPath;

    NodeCPtr last = destination;
    while (previousNodes.end() != previousNodes.find(last->m_id) && UNDEFINED != previousNodes[last->m_id].get())
        {
        shortestPath.insert(shortestPath.begin(), last);
        last = previousNodes[last->m_id];
        }
    shortestPath.insert(shortestPath.begin(), last);

    return shortestPath;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              05/2017
//--------------+---------------+---------------+---------------+---------------+--------
BentleyStatus FindAndErase(bvector<NodePtr>& nodes, NodePtr node)
    {
    for (bvector<NodePtr>::iterator it = nodes.begin(); it != nodes.end(); ++it)
        if (*it == node)
            {
            nodes.erase(it);
            return BentleyStatus::SUCCESS;
            }
            
    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              05/2017
//--------------+---------------+---------------+---------------+---------------+--------
BentleyStatus WeightedGraph::FindShortestPath
(
    bvector<NodeCPtr>& shortestPath,
    NodeCPtr source,
    NodeCPtr destination
) const
    {
    shortestPath = bvector<NodeCPtr>();
    bmap<BeInt64Id, double> distances; //distances from source to node with given ID
    bmap<BeInt64Id, NodePtr> previousNode; //a map with a node pointing to a previous node in the path. Later used to create the shortest path list
    bvector<NodePtr> queue; //All graph nodes to check

    //initialize
    for (bpair<BeInt64Id, NodePtr> pair : nodes)
        {
        //At first all distances between source and node n are infinate. Later each node will update it's neighbors' distances to source
        distances[pair.first] = DISTANCE_INFINITY; 
        previousNode[pair.first] = UNDEFINED;
        queue.push_back(pair.second);
        }

    //distance from source to itself is 0
    if (distances.end() != distances.find(source->m_id))
        distances[source->m_id] = 0;
    else
        return BentleyStatus::ERROR;

    while (queue.size() > 0)
        {
        NodePtr nearest = FindNearestToSourceInQueue(queue, distances); //First we will find the source node, then one of the visited node's neighbors

        if (!nearest.IsValid()) //no more nodes to go through. Path cannot be found
            return BentleyStatus::ERROR;

        if (*nearest == *destination) //we have reached the destination
            break;

        if (BentleyStatus::ERROR == FindAndErase(queue, nearest))
            return BentleyStatus::ERROR;

        for (ConnectionPtr connection : nearest->m_edges) //go through all node's neighbors and update their distances to source
            {
            if (!connection->IsPassable()) // ignore impassable connections
                continue;

            double distanceFromNearestToNeighbor = connection->m_weight;
            if (distances.end() != distances.find(nearest->m_id))
                {
                double alternativeDistance = distances[nearest->m_id] + distanceFromNearestToNeighbor;
                if (distances.end() != distances.find(connection->GetNeighbourId()) && alternativeDistance < distances[connection->GetNeighbourId()])
                    {
                    distances[connection->GetNeighbourId()] = alternativeDistance;
                    previousNode[connection->GetNeighbourId()] = nearest; //update that the shortest path to neighbor node is through the current node
                    }
                }
            }
        }

    shortestPath = ReadShortestPath(previousNode, destination);

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              05/2017
//--------------+---------------+---------------+---------------+---------------+--------
void Connection::ConnectNodes(NodePtr a, NodePtr b, double weightAB, double weightBA)
    {
    ConnectionPtr aToB = new Connection(b, weightAB);

    ConnectionPtr bToA = new Connection(a, weightBA);

    a->AddEdge(aToB);
    b->AddEdge(bToA);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              08/2017
//--------------+---------------+---------------+---------------+---------------+--------
bvector<ConnectionPtr>::iterator Node::GetConnectionTo(BeInt64Id id) 
    {
    return std::find_if(m_edges.begin(), m_edges.end(), [&](ConnectionPtr connection) 
        {
        return connection->GetNeighbourId() == id; 
        }); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              05/2017
//--------------+---------------+---------------+---------------+---------------+--------
bool Node::IsConnectedTo(BeInt64Id otherId)
    {
    for (ConnectionPtr connection : m_edges)
        if (connection->GetNeighbourId() == otherId)
            return true;

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              06/2017
//--------------+---------------+---------------+---------------+---------------+--------
BentleyStatus WeightedGraph::RemoveNode(BeInt64Id id)
    {
    if (!Contains(id))
        return BentleyStatus::ERROR;

    NodePtr node = GetNode(id);

    //remove connections to this node
    for (ConnectionPtr connection : node->m_edges)
        {
        bvector<ConnectionPtr>::iterator it = std::find_if(connection->GetNeighbourConnections().begin(), connection->GetNeighbourConnections().end(), [&](ConnectionPtr connection) {return *connection->m_neighbor == *node; });
        
        if (it != connection->GetNeighbourConnections().end())
            connection->GetNeighbourConnections().erase(it);
        }

    //remove the node itself
    nodes.erase(node->m_id);

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              06/2017
//--------------+---------------+---------------+---------------+---------------+--------
void WeightedGraph::Merge(WeightedGraphCPtr rhs)
    {
    for (bpair<BeInt64Id, NodePtr> pair : rhs->nodes)
        AddNode(pair.second);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              07/2017
//--------------+---------------+---------------+---------------+---------------+--------
void Node::AddTwoElementsToGraph(WeightedGraphPtr& graph, NodePtr& parentNode, NodePtr& childNode, BeInt64Id parent, BeInt64Id child)
    {
    if (!graph->Contains(parent))
        {
        parentNode = new Node(parent);
        graph->AddNode(parentNode);
        }
    else
        parentNode = graph->GetNode(parent);

    if (!graph->Contains(child))
        {
        childNode = new Node(child);
        graph->AddNode(childNode);
        }
    else
        childNode = graph->GetNode(child);
    }
END_BUILDING_SHARED_NAMESPACE