/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
// Context for use in detecting loops in graphs defined by (int,int) edges, where each int is a vertex index.
// The vertex indices are NOT expected to be "packed" -- e.g. a 4-vertex graph might have vertex indices 1003,36, 28, 92
//
//  The caller's use pattern is:
//
//  MTGLoopDetector detector;
//  bvector<int> vertexIndicesAroundLoop;
//  for each loop search
//    {
//    detector.Clear ();
//    for each edge defined by int paur    (vertexIndex0, vertexIndex1)
//        {
//        if (AddEdgeAndTestForCycle (vertexIndex0, vertexIndex1, vertexIndicesAroundLoop))
//                {
//                DONE !!!!! process the loop.
//                }
//        }
// REMARK The expected use case is
// 1) loops appear after 6 to 30 edges.
// 2) vertex indices are from a much larger set (say #V)
// 3) The detector will be reused for many searchs -- related to #V.
// The reinitialization between searches (with Clear()) should NOT require reallocation of anything in the MTG.
//   However, it is not know if the bmap used to look up nodeIdAtVertexIndex will 
//       will reuse its map.   Hence there may be allocations at each use.  Correcting this will require
//       a more specialized map<int,MTGNodeId>.  This will not be difficult code to write.
//
// MTGGraph notes
//   Each MTGGraph edge has two nodes -- one at "back" of each side.
//   Each node has a single integer label.  This is the vertexIndex.
//   When an edge is added, its two integer labels are stored at the two ends.
//   At each end, if there is a prior node with the same vertex, the new node is twisted into that vertex loop.
//   Cycle detection properties:
//     1) If one or both vertexIndex of a new edge is "new" -- first use of that index -- the edge cannot possibly close a cycle.
//     2) If both have appeared before, the new edge will either (a) join previously separate connected components or
//          (b) close a cycle within a connected component.   Walking around the face loop from one attachment will
//               reveal if the other attachment is part of the same component.
//     3) If a cycle was just closed, the connected component will have exactly 2 face loops.
//          masking logic in CollectVertexIndicesInLoop identify which edges are part of the cycle and which
//          are part of branches dangling inside or outside the loop.

struct MTGLoopDetector
{
MTGGraph m_graph;
bmap<size_t, MTGNodeId> m_nodeAtVertex;
bmap<size_t, set<MTGNodeId>> m_nodesAtVertex;
int m_vertexLabelOffset;
bvector<MTGNodeId> m_searchStack; // reused during searches
static const int s_defaultVertexIndex = -1;
MTGLoopDetector ()
  : m_graph()
  {
  m_vertexLabelOffset = m_graph.DefineLabel (1000, MTG_LabelMask_VertexProperty, s_defaultVertexIndex);
  }

// Reinitialize for another cycle construction . .
public: void Clear ()
    {
    m_graph.ClearNodes (true);
    m_nodeAtVertex.clear ();
    }

// Search for any node at specified vertexIndex (there is one node per indicent edge)
private: MTGNodeId AnyNodeAtVertexIndex (int vertexIndex)
    {
    auto count = m_nodeAtVertex.count(vertexIndex);
    auto location = m_nodeAtVertex.find (vertexIndex);
    if (location == m_nodeAtVertex.end ())
        return MTG_NULL_NODEID;
    return location->second;
    }

// Set the vertexIndex (label value) in a node.
private: void SetVertexIndex (MTGNodeId node, int vertexIndex)
    {
    m_graph.TrySetLabel (node, m_vertexLabelOffset, vertexIndex);
    if (MTG_NULL_NODEID == AnyNodeAtVertexIndex (vertexIndex))
      m_nodeAtVertex[vertexIndex] = node;
    }

private: void AddVertexIndex(MTGNodeId node, int vertexIndex)
    {
    m_graph.TrySetLabel(node, m_vertexLabelOffset, vertexIndex);
    //if (MTG_NULL_NODEID == AnyNodeAtVertexIndex(vertexIndex))
        m_nodesAtVertex[vertexIndex].insert(node);
    }
         
// Get the vertex index (label) in a node.
private: int GetVertexIndex (MTGNodeId node)
    {
    int value;
    if (m_graph.TryGetLabel (node, m_vertexLabelOffset, value))
        return value;
    return s_defaultVertexIndex;
    }

// In a "vertex to vertex" sense the graph has one loop.
// In the "Fsucc" sense the graph has (!!!) exactly two loops.
// Each touches every edge of the "vertexIndex" loop exactly once.
// 
// Mark the face of nodeA with a mask (which is clear elsewhere)
// Branches protruding on the nodeA side are double masked.
// Branches protruding from the other side have no masks.
//
// Walk around the face from nodeA (with usual FSucc steps)
// Record all mates that are not masked.
// 
private: void CollectVertexIndicesInLoop (bvector<int> &indices, MTGNodeId nodeA, MTGNodeId nodeB)
    {
    MTGMask mask = MTG_CONSTU_MASK;
    m_graph.ClearMask (mask);
    m_graph.SetMaskAroundFace (nodeA, mask);
    indices.clear ();
    MTGARRAY_FACE_LOOP (node, &m_graph, nodeA)
        {
        if (!m_graph.GetMaskAt (m_graph.EdgeMate (node), mask))
            {
            indices.push_back (GetVertexIndex (node));
            }
        }
    MTGARRAY_END_FACE_LOOP (node, &m_graph, nodeA)
    }

// Does nodeB appear in the face loop for nodeA?
private: bool FindNodeInFaceLoop (MTGNodeId nodeA, MTGNodeId nodeB)
    {
    MTGNodeId mateB = m_graph.EdgeMate(nodeB);
    MTGNodeId vsuccB = m_graph.VSucc(nodeB);
    MTGNodeId fsuccB = m_graph.FSucc(nodeB);
    MTGNodeId vpredB = m_graph.VPred(nodeB);
    MTGNodeId fpredB = m_graph.FPred(nodeB);
    MTGARRAY_FACE_LOOP(node, &m_graph, nodeA)
        {
        MTGNodeId mate = m_graph.EdgeMate(node);
        MTGNodeId vsucc = m_graph.VSucc(node);
        MTGNodeId fsucc = m_graph.FSucc(node);
        MTGNodeId vpred = m_graph.VPred(node);
        MTGNodeId fpred = m_graph.FPred(node);
        if (node == nodeB)
            {
            return true;
            }
        }
    MTGARRAY_END_FACE_LOOP (node, &m_graph, nodeA)
    return false;
    }

private: bool IsNodeInFaceLoop (MTGNodeId start)
    {
    MTGNodeId otherEdge = m_graph.EdgeMate(start);
    MTGARRAY_FACE_LOOP(node, &m_graph, start)
        {
        int vi = GetVertexIndex(node);
        MTGNodeId mate = m_graph.EdgeMate(node);
        MTGNodeId vsucc = m_graph.VSucc(node);
        MTGNodeId fsucc = m_graph.FSucc(node);
        MTGNodeId vpred = m_graph.VPred(node);
        MTGNodeId fpred = m_graph.FPred(node);
        if (node == otherEdge)
            {
            return false;
            }
        }
    MTGARRAY_END_FACE_LOOP(node, &m_graph, start)
        return true;
    }

// Add an edge.  Return true if a loop is create, and report the all vertexIndex in the cycle.
public: bool AddEdgeAndTestForCycle (int vertexIndexA, int vertexIndexB, bvector<int> &vertexAlongCycle)
  {
  bool closed = false;
  vertexAlongCycle.clear ();
  MTGNodeId existingNodeAtVertexA = AnyNodeAtVertexIndex (vertexIndexA);
  MTGNodeId existingNodeAtVertexB = AnyNodeAtVertexIndex (vertexIndexB);

  auto findB = m_nodeAtVertex.find (vertexIndexB);
  MTGNodeId newNodeAtVertexA, newNodeAtVertexB;
  // Create an edge dangling in space ...
  m_graph.CreateEdge (newNodeAtVertexA, newNodeAtVertexB);
  SetVertexIndex (newNodeAtVertexA, vertexIndexA);
  SetVertexIndex (newNodeAtVertexB, vertexIndexB);

  if (m_graph.IsValidNodeId (existingNodeAtVertexA) &&  m_graph.IsValidNodeId (existingNodeAtVertexB))
      {
      // The new edge joins two existing edges.
      // the graph is acyclic.  Hence this join can have two effects:
      // 1) join to previously unconnected components
      // 2) close a loop within one component.
      // The "face" loop from any node goes "all the way" around its (acyclic) component.
      //  So see if that face loop includes both existingNode ...
      closed = FindNodeInFaceLoop (existingNodeAtVertexA, existingNodeAtVertexB);
      }

  int numPrior = 0;
  if (m_graph.IsValidNodeId(existingNodeAtVertexA))
      {
      m_graph.VertexTwist (newNodeAtVertexA, existingNodeAtVertexA);
      numPrior++;
      }

  if (m_graph.IsValidNodeId(existingNodeAtVertexB))
      {
      m_graph.VertexTwist (newNodeAtVertexB, existingNodeAtVertexB);
      numPrior++;
      }

  if (closed)
      CollectVertexIndicesInLoop (vertexAlongCycle, existingNodeAtVertexA, existingNodeAtVertexB);

  return closed;
  }

// Add an edge.
public: void AddEdge(int vertexIndexA, int vertexIndexB)
    {
    MTGNodeId existingNodeAtVertexA = AnyNodeAtVertexIndex(vertexIndexA);
    MTGNodeId existingNodeAtVertexB = AnyNodeAtVertexIndex(vertexIndexB);

    MTGNodeId newNodeAtVertexA, newNodeAtVertexB;
    // Create an edge dangling in space ...
    m_graph.CreateEdge(newNodeAtVertexA, newNodeAtVertexB);
    SetVertexIndex(newNodeAtVertexA, vertexIndexA);
    SetVertexIndex(newNodeAtVertexB, vertexIndexB);

    int numPrior = 0;
    if (existingNodeAtVertexA != MTG_NULL_NODEID)
        {
        m_graph.VertexTwist(newNodeAtVertexA, existingNodeAtVertexA);
        m_graph.ExciseSliverFace(newNodeAtVertexA);
        numPrior++;
        }

    if (existingNodeAtVertexB != MTG_NULL_NODEID)
        {
        m_graph.VertexTwist(newNodeAtVertexB, existingNodeAtVertexB);
        m_graph.ExciseSliverFace(newNodeAtVertexB);
        numPrior++;
        }

    }

// Remove an edge.
public: void RemoveEdge(int vertexIndexA, int vertexIndexB)
    {
    MTGNodeId existingNodeAtVertexA = AnyNodeAtVertexIndex(vertexIndexA);
    MTGNodeId existingNodeAtVertexB = AnyNodeAtVertexIndex(vertexIndexB);

    if (existingNodeAtVertexA != MTG_NULL_NODEID && existingNodeAtVertexB != MTG_NULL_NODEID)
        {
        MTGNodeId vsuccA = m_graph.VSucc(existingNodeAtVertexA);
        MTGNodeId vsuccB = m_graph.VSucc(existingNodeAtVertexB);
        m_graph.DropEdge(vsuccA);
        m_graph.DropEdge(vsuccB);
        }

    }


// Add an edge.  Return true if a loop is create, and report the all vertexIndex in the cycle.
public: bool IsEdgeInCycle(int vertexIndexA, int vertexIndexB, bool* isCycleUnique, bvector<int> &vertexAlongCycle)
    {
    bool closed = false;
    vertexAlongCycle.clear();
    MTGNodeId existingNodeAtVertexA = AnyNodeAtVertexIndex(vertexIndexA);
    MTGNodeId existingNodeAtVertexB = AnyNodeAtVertexIndex(vertexIndexB);

    closed = DFS(existingNodeAtVertexA, isCycleUnique);

    if (closed)
        CollectVertexIndicesInLoop(vertexAlongCycle, existingNodeAtVertexA, existingNodeAtVertexB);

    return closed;
    }

enum { WHITE, GREY, BLACK };
struct DFSNode
    {
    int color = WHITE;
    int index = -1;
    int vertex = -1;
    MTGNodeId edge;
    MTGNodeId mate;
    };
DFSNode startVertex;
private: bool DFS(MTGNodeId start, bool* detectBranch)
    {
    int vi = GetVertexIndex(start);
    bmap<int, DFSNode> dfsNodeMap;
    //startNode.color = GREY;
    startVertex.vertex = vi;
    startVertex.edge = start;
    startVertex.mate = m_graph.EdgeMate(start);
    //dfsNodeMap[vi] = startNode;
    std::stack<DFSNode> dfsStack;
    //dfsStack.push(dfsNodeMap[vi]);
    return DFSVisit(start, dfsStack, dfsNodeMap, detectBranch);
    }

//private: MTGNodeId backTrackToFindNewPath(std::stack<DFSNode>& stack)
//    {
//    while (!stack.empty())
//        {
//        DFSNode node = stack.top();
//        MTGNodeId nId = AnyNodeAtVertexIndex(node.index);
//        stack.pop();
//        MTGNodeId emate = m_graph.EdgeMate(nId);
//        MTGNodeId vsucc = m_graph.VSucc(nId);
//        MTGNodeId fsucc = m_graph.FSucc(nId);
//        MTGNodeId vpred = m_graph.VPred(nId);
//        MTGNodeId fpred = m_graph.FPred(nId);
//        auto nextNode = m_graph.FSucc(emate);
//        while (nextNode != nId)
//            {
//            emate = m_graph.EdgeMate(nextNode);
//            nextNode = m_graph.FSucc(emate);
//            }
//        if (vpred != vsucc) return (vpred != node.edge ? vpred : vsucc);
//        }
//    return MTG_NULL_NODEID;
//    }

private: bool DFSVisit(MTGNodeId nodeId, std::stack<DFSNode>& stack, bmap<int, DFSNode>& dfsNodeMap, bool* detectBranch)
    {
    int vi = GetVertexIndex(nodeId);
    auto location = dfsNodeMap.find(vi);
    if (location == dfsNodeMap.end())
        {
        DFSNode dfsNode;
        dfsNode.color = GREY;
        dfsNode.index = vi;
        dfsNode.edge = nodeId;
        dfsNodeMap[vi] = dfsNode;
        stack.push(dfsNode);
        MTGNodeId vsucc = m_graph.VSucc(nodeId);
        MTGNodeId vpred = m_graph.VPred(nodeId);
        MTGNodeId fpred = m_graph.FPred(nodeId);
        MTGNodeId firstNode = nodeId;
        MTGNodeId firstNodeMate = m_graph.EdgeMate(firstNode);
        MTGNodeId nextNode = firstNode;
        do
            {
            MTGNodeId nodeMate = m_graph.EdgeMate(nextNode);
            if (GetVertexIndex(nextNode) == startVertex.vertex)
                {
                startVertex.edge = nextNode;
                startVertex.mate = nodeMate;
                }
            MTGNodeId fsucc = m_graph.FSucc(nextNode);
            if (fsucc == nodeMate)
                { // we're at a dead end...
                }
            else if (nextNode == startVertex.mate)
                { // we've come around by another loop which the starting node is not part of

                }
            else if (DFSVisit(fsucc, stack, dfsNodeMap, detectBranch)) return true;

            nextNode = m_graph.FSucc(nodeMate);
            } while (nextNode != firstNode);

        return false;
        }
    
    // already visited, there might be a loop
    // check to see if we're back at the beginning
    if (location->second.index == startVertex.vertex) return true;
    return false;
    // step back, turn around and continue searching
    //location->second.color = GREY;
    //MTGNodeId nextNode = backTrackToFindNewPath(stack);
    //DFSNode node = stack.top();
    //MTGNodeId nId = AnyNodeAtVertexIndex(node.index);

    //if (nextNode == MTG_NULL_NODEID) return false;

    //return DFSVisit(m_graph.FSucc(nextNode), stack, dfsNodeMap, detectBranch);
    }
};
