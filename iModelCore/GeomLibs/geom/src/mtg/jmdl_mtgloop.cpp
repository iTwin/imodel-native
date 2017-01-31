/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/mtg/jmdl_mtgloop.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define NOISYx
#ifdef NOISY

#endif
/*----------------------------------------------------------------------+
|                                                                       |
|   Local defines                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Local type definitions                                              |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Private Global variables                                            |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Public Global variables                                             |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   External variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/


/**
* @param pGraph    => containing graph
* @param nodeId => node id
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_countNodesAroundFace
(
const MTGGraph *      pGraph,
MTGNodeId           nodeId
)
    {
    int count = 0;
    MTGARRAY_FACE_LOOP (currNodeId, pGraph, nodeId)
        {
        count++;
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, nodeId)
    return count;
    }



/**
* @param pGraph   => graph containing vertex
* @param nodeId => node id
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_countNodesAroundVertex
(
const MTGGraph *    pGraph,
      MTGNodeId    nodeId
)
    {
    int count = 0;
    MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, nodeId)
        {
        count++;
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, nodeId)
    return count;
    }


/**
* Search the graph for FACE loops.   The i'th face is reported in two ways:
* 1) pStartArray[[i]] is a node on the face i.
* 2) For each node k on the face i, pFaceId[[k]] == i.
*
* @param    pGraph      => graph to search
* @param pStartArray    <=> For each face loop, this array tells one start mtg node.
*                               May be NULL
* @param pFaceId        <=> For each mtg node, this array tells the face loop it is in.
*                               -1 if node id not in use  May be NULL
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_collectAndNumberFaceLoops
(
const MTGGraph     *pGraph,
EmbeddedIntArray            *pStartArray,
EmbeddedIntArray            *pFaceId
)
    {
    int currFaceNumber = 0, visited = -1;
    int nodeIdCount = jmdlMTGGraph_getNodeIdCount(pGraph);
    EmbeddedIntArray *pStartArray1 = pStartArray;
    EmbeddedIntArray *pFaceId1 = pFaceId;

    if (!pStartArray1)
        pStartArray1 = jmdlEmbeddedIntArray_grab();
    if (!pFaceId1)
        pFaceId1 = jmdlEmbeddedIntArray_grab();

    jmdlEmbeddedIntArray_setConstant (pFaceId1, MTG_NOT_VISITED, nodeIdCount);
    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        if (   jmdlEmbeddedIntArray_getInt (pFaceId1, &visited, currNodeId)
            && visited == MTG_NOT_VISITED)
            {
            jmdlEmbeddedIntArray_addInt (pStartArray1, currNodeId);
            MTGARRAY_FACE_LOOP (currFaceId, pGraph, currNodeId)
                {
                jmdlEmbeddedIntArray_setInt (pFaceId1, currFaceNumber, currFaceId);
                }
            MTGARRAY_END_FACE_LOOP (currFaceId, pGraph, currNodeId)
            currFaceNumber++;
            }

        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)

    if (!pStartArray)
        jmdlEmbeddedIntArray_drop(pStartArray1);
    if (!pFaceId)
        jmdlEmbeddedIntArray_drop (pFaceId1);

    return currFaceNumber;
    }


/**
*
* Search the graph for EDGEs.   The i'th edge is reported in
* two ways:
* 1) pStartArray[[i]] is a node on the edge.
* 2) For each node k on the edge i, pEdgeId[[k]] == i.
*
* @param pGraph               => graph to search.
* @param pStartArray    <= For each edge, this array tells one start mtg node. May be NULL
* @param pEdgeId        <= For each mtg node, this array tells the edge on.
*                           -1 if node id not in use May be NULL
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_collectAndNumberEdges
(
const MTGGraph     *pGraph,
EmbeddedIntArray            *pStartArray,
EmbeddedIntArray            *pEdgeId
)
    {
    int currEdgeNumber = 0, visited = -1;
    MTGNodeId  currMateId;
    int nodeIdCount = jmdlMTGGraph_getNodeIdCount(pGraph);
    EmbeddedIntArray *pStartArray1 = pStartArray;
    EmbeddedIntArray *pEdgeId1 = pEdgeId;

    if (!pStartArray1)
        pStartArray1 = jmdlEmbeddedIntArray_grab();
    if (!pEdgeId1)
        pEdgeId1 = jmdlEmbeddedIntArray_grab();

    jmdlEmbeddedIntArray_setConstant (pEdgeId1, MTG_NOT_VISITED, nodeIdCount);

    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        if (   jmdlEmbeddedIntArray_getInt (pEdgeId1, &visited, currNodeId)
            && visited == MTG_NOT_VISITED)
            {
            jmdlEmbeddedIntArray_addInt (pStartArray1, currNodeId);
            currMateId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);
            jmdlEmbeddedIntArray_setInt (pEdgeId1, currEdgeNumber, currNodeId);
            jmdlEmbeddedIntArray_setInt (pEdgeId1, currEdgeNumber, currMateId);
            currEdgeNumber++;
            }

        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)

    if (!pStartArray)
        jmdlEmbeddedIntArray_drop(pStartArray1);
    if (!pEdgeId)
        jmdlEmbeddedIntArray_drop (pEdgeId1);

    return currEdgeNumber;

    }


/**
*
* Search the graph for VERTEX loops.   The i'th vertex is reported in
* two ways
* 1) pStartArray[[i]] is some node at vertex i.
* 2) For each node k on the vertex i, pVertexId[[k]] == i.
*
* @param pGraph               => graph to search.
* @param pStartArray    <= For each vertex loop, this array tells one start node. May be NULL
* @param pVertexId      <= For each mtg array node in the graph, this array tells
*                           the vertex id.          May be NULL
*                           -1 if node id not in use
* @see
* @return number of vertex loops
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_collectAndNumberVertexLoops
(
const MTGGraph     *pGraph,
EmbeddedIntArray            *pStartArray,
EmbeddedIntArray            *pVertexId
)
    {
    int currVLoopNumber = 0,  visited = -1;
    int nodeIdCount = jmdlMTGGraph_getNodeIdCount(pGraph);

    EmbeddedIntArray *pStartArray1 = pStartArray;
    EmbeddedIntArray *pVertexId1 = pVertexId;

    if (!pStartArray1)
        pStartArray1 = jmdlEmbeddedIntArray_grab();
    if (!pVertexId1)
        pVertexId1 = jmdlEmbeddedIntArray_grab();

    jmdlEmbeddedIntArray_setConstant (pVertexId1, MTG_NOT_VISITED, nodeIdCount);
    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        if (   jmdlEmbeddedIntArray_getInt (pVertexId1, &visited, currNodeId)
            && visited == MTG_NOT_VISITED)
            {
            jmdlEmbeddedIntArray_addInt (pStartArray1, currNodeId);
            MTGARRAY_VERTEX_LOOP (currVertexId, pGraph, currNodeId)
                {
                jmdlEmbeddedIntArray_setInt (pVertexId1, currVLoopNumber, currVertexId);
                }
            MTGARRAY_END_VERTEX_LOOP (currVertexId, pGraph, currNodeId)
            currVLoopNumber++;
            }

        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)

    if (!pStartArray)
        jmdlEmbeddedIntArray_drop(pStartArray1);
    if (!pVertexId)
        jmdlEmbeddedIntArray_drop (pVertexId1);

    return currVLoopNumber;
    }


static void        jmdlMTGGraph_extendPrioritizedSpanningTree     // number of vertex loops
(
const MTGGraph *      pGraph,       //  => graph to search
EmbeddedIntArray            *pStack0,       // <=> preferred edge stack
EmbeddedIntArray            *pStack1,       // <=> non-preferred edge stack
EmbeddedIntArray            *pTreeEdge,    // <= Edges in order used for tree
EmbeddedIntArray            *pStartArray,   // <= For each vertex loop, this array tells one start node.
EmbeddedIntArray            *pVertexId,       // <= For each mtg array node in the graph, this array tells
                                    //          the vertex id.
                                    //      (-1 if node id not in use)
MTG_NodeBoolFunc
                    isPreferredEdge, // => test function
MTGMask     mask,           // => mask to pass to test function
void                *pUserData      // => user data for callback
)
    {
    MTGNodeId node0Id, node1Id;
    int vertex0Index, vertex1Index;

    while (  jmdlEmbeddedIntArray_popInt (pStack0, &node0Id)
          || jmdlEmbeddedIntArray_popInt (pStack1, &node0Id)
          )
        {

        if (  jmdlEmbeddedIntArray_getInt (pVertexId, &vertex0Index, node0Id)
            && vertex0Index < 0
            )
            {
            // First exit from the vertex.  Number it and put all its outedges on stacks.
            vertex0Index = jmdlEmbeddedIntArray_getCount (pStartArray);
            jmdlEmbeddedIntArray_setInt (pStartArray, node0Id, vertex0Index);
            MTGARRAY_VERTEX_LOOP (node2Id, pGraph, node0Id)
                {
                jmdlEmbeddedIntArray_setInt (pVertexId, vertex0Index, node2Id);

                if (node2Id != node0Id)
                    jmdlEmbeddedIntArray_addInt (isPreferredEdge (pGraph, node2Id, mask, pUserData) ? pStack0 : pStack1, node2Id);
                }
            MTGARRAY_END_VERTEX_LOOP (node2Id, pGraph, node0Id)
            }
        else
            {
            node1Id = jmdlMTGGraph_getEdgeMate (pGraph, node0Id);
            if (   jmdlEmbeddedIntArray_getInt (pVertexId, &vertex1Index, node1Id)
                && vertex1Index < 0
                )
                {
                // Make the far vertex available for continuation.  It will get numbered
                // when popped (and that may be the very next time around the loop !!!)
                jmdlEmbeddedIntArray_addInt (isPreferredEdge (pGraph, node1Id, mask, pUserData) ? pStack0 : pStack1, node1Id);
                // Record this as a tree edge
                jmdlEmbeddedIntArray_addInt (pTreeEdge, node1Id);
                }
            }
        }
    }



/**
*
* Construct a spanning tree on the MTG graph, with callback used to
* assign candidate edges among several priorities.
*
* @param pGraph               => graph to search
* @param pTreeEdge      <= Edges in order used for tree
* @param pStartArray    <= For each vertex loop, this array tells one start node.
* @param pVertexId      <= For each mtg array node in the graph, this array tells
*                               the vertex id.
*                               -1 if node id not in use
* @param isPreferred    => test function
* @param mask           => mask to pass to test function
* @param pUserData      => user data for callback
* @see
* @return number of vertex loops
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_prioritizedSpanningTree
(
const MTGGraph *      pGraph,
EmbeddedIntArray            *pTreeEdge,
EmbeddedIntArray            *pStartArray,
EmbeddedIntArray            *pVertexId,
MTG_NodeBoolFunc    isPreferred,
MTGMask     mask,
void                *pUserData
)
    {
    int vertexIndex;
    int nodeIdCount = jmdlMTGGraph_getNodeIdCount(pGraph);

    EmbeddedIntArray  *pStartArray1 = pStartArray;
    EmbeddedIntArray  *pVertexId1   = pVertexId;
    EmbeddedIntArray  *pTreeEdge1   = pTreeEdge;
    EmbeddedIntArray  *pStack0      = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray  *pStack1      = jmdlEmbeddedIntArray_grab ();

    if (!pStartArray1)
        pStartArray1 = jmdlEmbeddedIntArray_grab();
    if (!pVertexId1)
        pVertexId1 = jmdlEmbeddedIntArray_grab();
    if (!pTreeEdge1)
        pTreeEdge1 = jmdlEmbeddedIntArray_grab();

    jmdlEmbeddedIntArray_empty (pStartArray1);
    jmdlEmbeddedIntArray_empty (pTreeEdge1);
    jmdlEmbeddedIntArray_setConstant (pVertexId1, -1, nodeIdCount);

    // Outer loop invariants:
    // Both stacks are empty.  Any unvisited, preferred vertex pulled from the graph
    //      is a true seed.
    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        if (   jmdlEmbeddedIntArray_getInt (pVertexId1, &vertexIndex, currNodeId)
            && vertexIndex < 0
            )
            {
            // Seed the search from here.
            if (isPreferred (pGraph, currNodeId, mask, pUserData))
                {
                jmdlEmbeddedIntArray_addInt (pStack0, currNodeId);
                jmdlMTGGraph_extendPrioritizedSpanningTree (pGraph,
                                    pStack0, pStack1, pTreeEdge1,
                                    pStartArray1, pVertexId1,
                                    isPreferred, mask, pUserData);
                }
            }
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)

    int numVertex = jmdlEmbeddedIntArray_getCount (pStartArray1);
    if (!pStartArray)
        jmdlEmbeddedIntArray_drop(pStartArray1);
    if (!pVertexId)
        jmdlEmbeddedIntArray_drop (pVertexId1);
    if (!pTreeEdge)
        jmdlEmbeddedIntArray_drop (pTreeEdge1);

    jmdlEmbeddedIntArray_drop (pStack0);
    jmdlEmbeddedIntArray_drop (pStack1);
    return numVertex;
    }


/**
* Initialize for a search conducted cooperatively by the caller and
* these functions.
* The search is guided by two node arrays.
* pStack is a stack of seeds for future exploration.
* pLoopId is an index from each node id A to the node id B
*   which is in the same vertex loop as A and was the entry point to
*   that vertex loop during the DFS.
* jmdlMTGGraph_seedSearch (... seedNodeId..) sets seedNodeId as the seed
* of its vertex loop, and adds all others around the loop to the search
* stack.
* nodeId = jmdlMTGGraph_harvest (...) extracts one node in an unvisited
*   vertex loop.   The newlyvisited node is planted as a seed for
*   further search.   Face successors are only followed if a mask is
*   set.
* @param pGraph    => graph being searched.
* @param pStack <=> array for seeds of unexplored paths
* @param pLoopId <=> array for loop id's
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_initSearch
(
const MTGGraph *     pGraph,
EmbeddedIntArray            *pStack,
EmbeddedIntArray            *pLoopId
)
    {
    int numNode = jmdlMTGGraph_getNodeIdCount (pGraph);
    jmdlEmbeddedIntArray_setConstant (pLoopId, MTG_NULL_NODEID, numNode);
    jmdlEmbeddedIntArray_empty (pStack);
    }


/**
* (see jmdlMTGGraph_initMaskedDFS)
* @param pGraph               => graph to search
* @param pStack         <=> array for seeds of unexplored paths
* @param pLoopId        <=> array for loop id's
* @param seedNodeId     => seed node id
* @param preTest        => if true, test if the node is already
*                           labeled in the LoopId array.
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_seedSearch
(
const MTGGraph *     pGraph,
EmbeddedIntArray            *pStack,
EmbeddedIntArray            *pLoopId,
MTGNodeId           seedNodeId,
bool                preTest
)
    {
    MTGNodeId entryNodeId;

#ifdef NOISY
        GEOMAPI_PRINTF("    seed node %d\n", seedNodeId);
#endif

    if (   preTest
        && jmdlEmbeddedIntArray_getInt (pLoopId, &entryNodeId, seedNodeId)
        && entryNodeId != MTG_NULL_NODEID
        )
        {
        // nothing to do ...
#ifdef NOISY
        GEOMAPI_PRINTF("            (prior entry to this seed node was %d)\n", entryNodeId);
#endif
        }
    else
        {
        // Enter the entire vertex loop on the stack.
        MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, seedNodeId)
            {
            jmdlEmbeddedIntArray_setInt (pLoopId, seedNodeId, currNodeId);
            jmdlEmbeddedIntArray_addInt (pStack, currNodeId);
#ifdef NOISY
            GEOMAPI_PRINTF("       slave node %d\n", currNodeId);
#endif
            }
        MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, seedNodeId)
        }
    }


/**
* (see jmdlMTGGraph_initMaskedDFS)
* @param pGraph    => graph to search
* @param pStack <=> array for seeds of unexplored paths
* @param pLoopId <=> array for loop id's
* @param mask => mask to restrict search
* @see
* @return MTGNodeId
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_harvestMaskedDFS
(
const MTGGraph     *pGraph,
EmbeddedIntArray            *pStack,
EmbeddedIntArray            *pLoopId,
MTGMask     mask
)
    {
    int node0Id, node1Id;
    int parentNodeId;

    while (jmdlEmbeddedIntArray_popInt (pStack, &node0Id))
        {
        node1Id = jmdlMTGGraph_getFSucc( pGraph, node0Id);
#ifdef NOISY
        GEOMAPI_PRINTF(" pop candidate %d (mask %d, nodeMask %d)\n",
                node0Id, mask, jmdlMTGGraph_getMask (pGraph, node0Id, mask));
#endif

        // Add the newly exposed neighbor to the stack
        if (jmdlMTGGraph_getMask (pGraph, node0Id, mask))
            jmdlMTGGraph_seedSearch (pGraph, pStack, pLoopId, node1Id, true);

        if(     jmdlEmbeddedIntArray_getInt (pLoopId, &parentNodeId, node0Id)
            &&  parentNodeId == node0Id
           )
            {
            // This node was the entry to this vertex -- it counts!!!
#ifdef NOISY
            GEOMAPI_PRINTF(" pop ACCEPT %d ",
                node0Id, mask, jmdlMTGGraph_getMask (pGraph, node0Id, mask));
#endif
            return node0Id;
            }
        }
    return MTG_NULL_NODEID;
    }



/**
* Search (recursively) outward from nodes in pStartStack, applying
* a stopMask maks to each vertex loop.
* @param pGraph    <=> graph to search and mark
* @param pStartStack <=> On input, array of start nodes.
* @param                                         Used internally as a holding stack
*                                   On output, empty array.
* @param outMask => mask to marking 'out' edges from vertex loops
* @param stopMask => mask marking vertices that are already visited
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_spreadVertexMarker
(
MTGGraph *          pGraph,
EmbeddedIntArray            *pStartStack,
MTGMask     outMask,
MTGMask     stopMask
)
    {
    MTGNodeId startNodeId, neighborNodeId;
    MTGMask   currMask;

    for (; jmdlEmbeddedIntArray_popInt (pStartStack, &startNodeId);)
        {
        currMask = jmdlMTGGraph_getMask (pGraph, startNodeId, stopMask);
        if (!stopMask)
            {
            // This is the first encounter with this vertex loop.
            // Mark it and explore adjacent ones.
            MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, startNodeId)
                {
                jmdlMTGGraph_setMask (pGraph, currNodeId, stopMask);
                if (jmdlMTGGraph_getMask (pGraph, currNodeId, outMask))
                    {
                    neighborNodeId = jmdlMTGGraph_getFSucc(pGraph, currNodeId);
                    // We could just push it on the stack and let the stopMask test happen
                    // when it is popped.  However, let's hold the stack size down by
                    // pretesting.
                    if (!jmdlMTGGraph_getMask (pGraph, neighborNodeId, stopMask))
                        {
                        jmdlEmbeddedIntArray_addInt (pStartStack, neighborNodeId);
                        }
                    }
                }
            MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, startNodeId)
            }
        }
    }


/**
* Search the graph for connected components.   The i'th component is
* reported in two ways:
* 1) pStartArray[[i]] is a seed node on component i.
* 2) For each node k on component i, pComponentId[[k]] == i.
* @param pStartArray <= For each component, this array tells one start mtg node. May be NULL
* @param pComponentId <= For each mtg node, this array tells the component loop it is in.
*                               -1 if node id not in use  May be NULL
* @param pGraph   => graph to search
* @see
* @return int
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_collectAndNumberConnectedComponents
(
const MTGGraph     *pGraph,
EmbeddedIntArray            *pStartArray,
EmbeddedIntArray            *pComponentId
)
    {
    EmbeddedIntArray *pStack;
    int currComponentNumber = 0, visited = -1, edgeMateId, fSuccId, stackTopId;
    int nodeIdCount = jmdlMTGGraph_getNodeIdCount(pGraph);

    EmbeddedIntArray *pStartArray1 = pStartArray;
    EmbeddedIntArray *pComponentId1 = pComponentId;

    if (!pStartArray1)
        pStartArray1 = jmdlEmbeddedIntArray_grab();
    if (!pComponentId1)
        pComponentId1 = jmdlEmbeddedIntArray_grab();

    jmdlEmbeddedIntArray_setConstant (pComponentId1, MTG_NOT_VISITED, nodeIdCount);
    pStack = jmdlEmbeddedIntArray_grab();

    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        if (   jmdlEmbeddedIntArray_getInt (pComponentId1, &visited, currNodeId)
            && visited == MTG_NOT_VISITED)
                {
                jmdlEmbeddedIntArray_empty(pStack);
                jmdlEmbeddedIntArray_addInt (pStartArray1, currNodeId);
                int numVertexMarked = 1;
                int numThisVertex = 0;
                MTGARRAY_VERTEX_LOOP (currVertId, pGraph, currNodeId)
                    {
                    jmdlEmbeddedIntArray_addInt (pStack, currVertId);
                    jmdlEmbeddedIntArray_setInt (pComponentId1, currComponentNumber, currVertId);
                    numThisVertex++;
                    }
                MTGARRAY_END_VERTEX_LOOP (currVertId, pGraph, currNodeId)

                while (jmdlEmbeddedIntArray_popInt (pStack, &stackTopId))
                    {
                    edgeMateId = jmdlMTGGraph_getEdgeMate (pGraph, stackTopId);
                    jmdlEmbeddedIntArray_setInt (pComponentId1, currComponentNumber, edgeMateId);
                    if (jmdlMTGGraph_isValidNodeId (pGraph, (fSuccId = jmdlMTGGraph_getFSucc (pGraph, stackTopId))))
                        {
                        if (jmdlEmbeddedIntArray_getInt (pComponentId1, &visited, fSuccId)
                                && visited == MTG_NOT_VISITED)
                            {
                            numVertexMarked++;
                            numThisVertex = 0;
                            MTGARRAY_VERTEX_LOOP (currVertId, pGraph, fSuccId)
                                {
                                jmdlEmbeddedIntArray_addInt (pStack, currVertId);
                                jmdlEmbeddedIntArray_setInt (pComponentId1, currComponentNumber, currVertId);
                                numThisVertex++;
                                }
                            MTGARRAY_END_VERTEX_LOOP (currVertId, pGraph, fSuccId)

                            }

                        }

                    }

                currComponentNumber++;

                }

        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)

    if (!pStartArray)
        jmdlEmbeddedIntArray_drop(pStartArray1);
    if (!pComponentId)
        jmdlEmbeddedIntArray_drop (pComponentId1);
    jmdlEmbeddedIntArray_drop (pStack);

    return  currComponentNumber;

    }



/**
* Search the graph for connected components, crossing only edges
*   not marked by a barrier mask.
* Collect node id's of each component in contiguous arrays, separated by null node ids.
* Barrier mask is assumed to be set completely on each face.
* @param pGraph => graph to search.
* @param pBlockedNodeIdArray => array of blocked node id's, separated by null node ids.
*           May be null pointer.
* @param barrierMask => mask for edges to be excluded.   May be MTG_NULL_MASK
* @return number of components.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_collectFaceLoopsInEdgeConnectedComponents
(
const MTGGraph     *pGraph,
EmbeddedIntArray            *pBlockedNodeIdArray,
MTGMask             barrierMask
)
    {
    EmbeddedIntArray *pStack;
    MTGGraph *pMarkableGraph = (MTGGraph *)pGraph;

    int numComponent = 0;
    MTGNodeId   mateId, stackTopId;
    MTGMask visitMask = jmdlMTGGraph_grabMask (pMarkableGraph);
    MTGMask combinedMask = visitMask | barrierMask;

    jmdlMTGGraph_clearMaskInSet (pMarkableGraph, visitMask);
    if (pBlockedNodeIdArray)
        jmdlEmbeddedIntArray_empty (pBlockedNodeIdArray);


    pStack = jmdlEmbeddedIntArray_grab();

    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, currNodeId, combinedMask))
            {
            jmdlEmbeddedIntArray_empty(pStack);
            numComponent++;
            jmdlEmbeddedIntArray_addInt (pStack, currNodeId);
            while (jmdlEmbeddedIntArray_popInt (pStack, &stackTopId))
                {
                if  (!jmdlMTGGraph_getMask (pGraph, stackTopId, combinedMask))
                    {
                    /* Record the face seed in the current component ... */
                    if (pBlockedNodeIdArray)
                        jmdlEmbeddedIntArray_addInt (pBlockedNodeIdArray, stackTopId);
                    /* For each node around this face ... */
                    MTGARRAY_FACE_LOOP (currNodeId, pGraph, stackTopId)
                        {
                        /* Been here ... */
                        jmdlMTGGraph_setMask (pMarkableGraph, currNodeId, visitMask);
                        /* Considering pushing edge mate onto stack for continuation ... */
                        mateId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);
                        if (!jmdlMTGGraph_getMask (pGraph, mateId, combinedMask))
                            jmdlEmbeddedIntArray_addInt (pStack, mateId);
                        }
                    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, stackTopId)
                    }
                }
            /* Record end-of-component ... */
            if (pBlockedNodeIdArray)
                jmdlEmbeddedIntArray_addInt (pBlockedNodeIdArray, MTG_NULL_NODEID);
            }
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)

    jmdlEmbeddedIntArray_drop (pStack);
    jmdlMTGGraph_dropMask (pMarkableGraph, visitMask);

    return  numComponent;
    }


/*----------------------------------------------------------------------*//**
* For each node around a vertex, push the node onto the stack array
*   and set an index at the node's position in the index array.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static void     recordAndStackVertexLoop
(
const MTGGraph  *pGraph,
MTGNodeId       seedNodeId,
EmbeddedIntArray      *pStackArray,
EmbeddedIntArray        *pNodeToIndexArray,
int             index,
EmbeddedIntArray      *pAllNodeIdArray,
EmbeddedIntArray      *pOneNodeIdArray
)
    {
    MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, seedNodeId)
        {
        jmdlEmbeddedIntArray_addInt (pStackArray, currNodeId);
        if (pNodeToIndexArray)
            jmdlEmbeddedIntArray_setInt (pNodeToIndexArray, index, currNodeId);
        if (pAllNodeIdArray)
            jmdlEmbeddedIntArray_addInt (pAllNodeIdArray, currNodeId);
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, seedNodeId)

    if (pOneNodeIdArray)
        jmdlEmbeddedIntArray_addInt (pOneNodeIdArray, seedNodeId);
    }



/**
* Search the graph for connected components in a subgraph reachable by seed node
* and a mask.
* @param pAllNodeId <= Array of all node ids visited.  May be null.
* @param pOneNodeIdPerVertex <= Array with only one node id from each vertex loop.  May be null.
* @param mask => mask for edges included in the subgraph.
* @param directed => if true, only traverse edges with the mask set in the direction of
*               traversal.  If false, edges can be traversed if either side is masked.
* @return number of components.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_collectMaskedComponent
(
const MTGGraph  *pGraph,
EmbeddedIntArray      *pAllNodeId,
EmbeddedIntArray      *pOneNodeIdPerVertex,
MTGNodeId       seedNodeId,
int             mask,
bool            directed
)
    {
    EmbeddedIntArray *pStack;
    int visited = -1, edgeMateId, stackTopId;
    int nodeIdCount = jmdlMTGGraph_getNodeIdCount(pGraph);
    int currComponentNumber = 1;

    EmbeddedIntArray *pComponentId;

    /* Oh well, order n time here: */
    pComponentId = jmdlEmbeddedIntArray_grab();
    jmdlEmbeddedIntArray_setConstant (pComponentId, MTG_NOT_VISITED, nodeIdCount);

    if (pAllNodeId)
        jmdlEmbeddedIntArray_empty (pAllNodeId);

    if (pOneNodeIdPerVertex)
        jmdlEmbeddedIntArray_empty (pOneNodeIdPerVertex);

    pStack = jmdlEmbeddedIntArray_grab();

    recordAndStackVertexLoop (pGraph, seedNodeId, pStack,
                            pComponentId,
                            currComponentNumber,
                            pAllNodeId,
                            pOneNodeIdPerVertex
                            );

    while (jmdlEmbeddedIntArray_popInt (pStack, &stackTopId))
        {
        edgeMateId = jmdlMTGGraph_getEdgeMate (pGraph, stackTopId);
        if ((   jmdlMTGGraph_getMask (pGraph, stackTopId, mask)
             || (!directed && jmdlMTGGraph_getMask (pGraph, edgeMateId, mask)))
            && visited == MTG_NOT_VISITED)
            {
            recordAndStackVertexLoop (pGraph, edgeMateId, pStack,
                            pComponentId,
                            currComponentNumber,
                            pAllNodeId,
                            pOneNodeIdPerVertex
                            );
            }
        }

    jmdlEmbeddedIntArray_drop (pComponentId);
    jmdlEmbeddedIntArray_drop (pStack);

    return  currComponentNumber;

    }



/**
* Search the graph for connected components in a subgraph identified by a mask.
*   The i'th component is
* reported in three ways:
* 1) pStartArray[[i]] is a seed node on component i.
* 2) For each node k on component i, pComponentId[[k]] == i.
* @param pStartArray <= For each component, this array tells one start mtg node. May be NULL
* @param pComponentId <= For each mtg node, this array tells the component loop it is in.
*                               -1 if node id not in use  May be NULL
* @param pNodeIdGroupedByComponent <= Array of node ids grouped by component, separated by
*                   null node id.  May be NULL.
* @param pOneNodePerVertex <= array containing one node id per visited vertex.  May be NULL.
* @param mask => mask for edges included in the subgraph.
* @param directed => if true, only traverse edges with the mask set in the direction of
*               traversal.  If false, edges can be traversed if either side is masked.
* @return number of components.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_collectAndNumberMaskedConnectedComponents
(
const MTGGraph  *pGraph,
EmbeddedIntArray      *pStartArray,
EmbeddedIntArray        *pComponentId,
EmbeddedIntArray      *pNodeIdGroupedByComponent,
EmbeddedIntArray      *pOneNodeIdPerVertex,
int             mask,
bool            directed
)
    {
    EmbeddedIntArray *pStack;
    int currComponentNumber = 0, visited = -1, edgeMateId, stackTopId;
    int nodeIdCount = jmdlMTGGraph_getNodeIdCount(pGraph);

    EmbeddedIntArray *pComponentId1 = pComponentId;

    if (!pComponentId1)
        pComponentId1 = jmdlEmbeddedIntArray_grab();

    jmdlEmbeddedIntArray_setConstant (pComponentId1, MTG_NOT_VISITED, nodeIdCount);

    if (pStartArray)
        jmdlEmbeddedIntArray_empty (pStartArray);

    if (pNodeIdGroupedByComponent)
        jmdlEmbeddedIntArray_empty (pNodeIdGroupedByComponent);

    pStack = jmdlEmbeddedIntArray_grab();

    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        if (jmdlEmbeddedIntArray_getInt (pComponentId1, &visited, currNodeId)
            && visited == MTG_NOT_VISITED)
                {
                jmdlEmbeddedIntArray_empty(pStack);
                if (pStartArray)
                    jmdlEmbeddedIntArray_addInt (pStartArray, currNodeId);

                recordAndStackVertexLoop (pGraph, currNodeId, pStack,
                            pComponentId1, currComponentNumber,
                            pNodeIdGroupedByComponent, pOneNodeIdPerVertex);

                while (jmdlEmbeddedIntArray_popInt (pStack, &stackTopId))
                    {
                    edgeMateId = jmdlMTGGraph_getEdgeMate (pGraph, stackTopId);
                    if ((   jmdlMTGGraph_getMask (pGraph, stackTopId, mask)
                         || (!directed && jmdlMTGGraph_getMask (pGraph, edgeMateId, mask)))
                        &&  jmdlEmbeddedIntArray_getInt (pComponentId1, &visited, edgeMateId)
                        && visited == MTG_NOT_VISITED)
                        {
                        recordAndStackVertexLoop (pGraph, edgeMateId, pStack,
                                        pComponentId1, currComponentNumber,
                                        pNodeIdGroupedByComponent, pOneNodeIdPerVertex);
                        }
                    }
                currComponentNumber++;
                if (pNodeIdGroupedByComponent)
                    jmdlEmbeddedIntArray_addInt (pNodeIdGroupedByComponent, MTG_NULL_NODEID);
                }
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)

    if (!pComponentId)
        jmdlEmbeddedIntArray_drop (pComponentId1);
    jmdlEmbeddedIntArray_drop (pStack);

    return  currComponentNumber;

    }



/**
* Assign a partial order to nodes in the graph.
* Assume directionMask is ON at the tail of edges that must be observed
* by the order.   Edges with mask not set at all are ok.
* @param pGraph    => graph to search
* @param pOrder <= Nodes in search order
* @param directionMask => mask which is set on forward edges
* @see
* @return true if the order was computed.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_partialOrder
(
const MTGGraph *      pGraph,
EmbeddedIntArray            *pOrder,
MTGMask      directionMask
)
    {
    int vertexId;
    MTGNodeId nodeId, downStreamNodeId;
    int numVertex;
    int numOut, numIn;
    EmbeddedIntArray *pStack = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray *pNumIntoVertex = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray *pNodeToVertex  = jmdlEmbeddedIntArray_grab ();
    bool    boolstat = true;

    // get arrays with cross ref from node to and from vertex.
    // (contents of pNumIntoVertex will become a counter (num) later)
    jmdlMTGGraph_collectAndNumberVertexLoops (pGraph, pNumIntoVertex, pNodeToVertex);

    numVertex = jmdlEmbeddedIntArray_getCount (pNumIntoVertex);

    jmdlEmbeddedIntArray_empty (pStack);
    jmdlEmbeddedIntArray_empty (pOrder);

    // At each node, count incoming (masked) edges.
    for (vertexId = 0; vertexId < numVertex; vertexId++)
        {
        jmdlEmbeddedIntArray_getInt (pNumIntoVertex, &nodeId, vertexId);
        jmdlMTGGraph_countMateMasksAroundVertex (&numIn, &numOut, pGraph, nodeId, directionMask);
        jmdlEmbeddedIntArray_setInt (pNumIntoVertex, numIn, vertexId);
        if (numIn == 0)
            jmdlEmbeddedIntArray_addInt (pStack, nodeId);
        }


    // Nodes with 0 incoming edges are ready to schedule.
    // As each such node is pulled off the stack,
    //      schedule it and decrment counts at successors.
    //      Push successors with zero count on the stack so they will
    //      be scheduled (and their successors in turn ...)
    //      during a later loop pass.
    while (jmdlEmbeddedIntArray_popInt (pStack, &nodeId))
        {
        jmdlEmbeddedIntArray_addInt (pOrder, nodeId);
        MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, nodeId)
            {
            if (jmdlMTGGraph_getMask (pGraph, currNodeId, directionMask))
                {
                downStreamNodeId = jmdlMTGGraph_getFSucc (pGraph, currNodeId);
                jmdlEmbeddedIntArray_getInt (pNodeToVertex, &vertexId, downStreamNodeId);
                jmdlEmbeddedIntArray_getInt (pNumIntoVertex, &numIn, vertexId);
                numIn--;
                jmdlEmbeddedIntArray_setInt (pNumIntoVertex, numIn, vertexId);
                if (numIn <= 0)
                    jmdlEmbeddedIntArray_addInt (pStack, downStreamNodeId);
                }
            }
        MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, nodeId)
        }

    if (jmdlEmbeddedIntArray_getCount (pOrder) != numVertex)
        {
        boolstat = false;
        }

    jmdlEmbeddedIntArray_drop (pStack);
    jmdlEmbeddedIntArray_drop (pNumIntoVertex);
    jmdlEmbeddedIntArray_drop (pNodeToVertex);

    return boolstat;
    }

typedef struct
    {
    MTGGraph *pGraph;
    MTGMask onStackMask;
    MTGMask directionMask;
    MTGMask componentMask;
    MTGMask bridgeMask;

    EmbeddedIntArray   *pNodeToLowNum;
    EmbeddedIntArray   *pNodeToDFSNum;

    int         totalNodes;
    int         numVertex;
    } DFSParams;

/*----------------------------------------------------------------------*//**
*
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static     void enterVertex
(
DFSParams       *pParams,
MTGNodeId       startNodeId
)
    {
    MTGARRAY_VERTEX_LOOP (currNodeId, pParams->pGraph, startNodeId)
        {
        jmdlMTGGraph_setMask (pParams->pGraph, currNodeId, pParams->onStackMask);
        jmdlEmbeddedIntArray_setInt (pParams->pNodeToDFSNum, pParams->numVertex, currNodeId);
        jmdlEmbeddedIntArray_setInt (pParams->pNodeToLowNum, pParams->numVertex, currNodeId);
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, pParams->pGraph, startNodeId)

    pParams->numVertex++;
    }

/*----------------------------------------------------------------------*//**
*
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static     void leaveVertex
(
DFSParams       *pParams,
MTGNodeId       startNodeId
)
    {
    jmdlMTGGraph_clearMaskAroundVertex
            (
            pParams->pGraph,
            startNodeId,
            pParams->onStackMask
            );
    }

static void setArrayAroundVertex
(
EmbeddedIntArray   *pArray,
int         value,
MTGGraph    *pGraph,
MTGNodeId   baseNodeId
)
    {
    /* This edge is the start of a loop */
    MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, baseNodeId)
        {
        jmdlEmbeddedIntArray_setInt (pArray, value, currNodeId);
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, baseNodeId)
    }

/*----------------------------------------------------------------------*//**
* Compare lowNum's at a node and its mate.  Reduce baseNodeId low num if it is higher.
* Set component or bridge masks on this edge (only).
* baseNodeId is assumed to be part of the directed graph.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static     void updateLowNumAndMasksAfterRecursion
(
DFSParams       *pParams,
MTGNodeId       baseNodeId,
MTGNodeId       mateNodeId
)
    {
    int baseLowNum, mateLowNum;
    jmdlEmbeddedIntArray_getInt (pParams->pNodeToLowNum, &baseLowNum, baseNodeId);
    jmdlEmbeddedIntArray_getInt (pParams->pNodeToLowNum, &mateLowNum, mateNodeId);

    if (mateLowNum < baseLowNum)
        {
        setArrayAroundVertex (pParams->pNodeToLowNum, mateLowNum, pParams->pGraph, baseNodeId);
        }

    if (mateLowNum <= baseLowNum)
        {
        if (pParams->componentMask)
            jmdlMTGGraph_setMask (pParams->pGraph, baseNodeId, pParams->componentMask);
        }
    else
        {
        if (pParams->bridgeMask)
            jmdlMTGGraph_setMask (pParams->pGraph, baseNodeId, pParams->bridgeMask);
        }
    }

/*----------------------------------------------------------------------*//**
*
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static     void updateForwardEdgeMasks
(
DFSParams       *pParams,
MTGNodeId       baseNodeId,
MTGNodeId       mateNodeId
)
    {
    int baseLowNum, mateLowNum;
    jmdlEmbeddedIntArray_getInt (pParams->pNodeToLowNum, &baseLowNum, baseNodeId);
    jmdlEmbeddedIntArray_getInt (pParams->pNodeToLowNum, &mateLowNum, mateNodeId);

    if (mateLowNum <= baseLowNum)
        {
        if (pParams->componentMask)
            jmdlMTGGraph_setMask (pParams->pGraph, baseNodeId, pParams->componentMask);
        }
    else
        {
        if (pParams->bridgeMask)
            jmdlMTGGraph_setMask (pParams->pGraph, baseNodeId, pParams->bridgeMask);
        }
    }

/*----------------------------------------------------------------------*//**
*
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static     void updateCrossEdgeMasks
(
DFSParams       *pParams,
MTGNodeId       baseNodeId,
MTGNodeId       mateNodeId
)
    {
    if (pParams->bridgeMask)
        jmdlMTGGraph_setMask (pParams->pGraph, baseNodeId, pParams->bridgeMask);
    }

/*----------------------------------------------------------------------*//**
*
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static  void   updateLowNumAndMasksOnLoopEdge
(
DFSParams       *pParams,
MTGNodeId       baseNodeId,
MTGNodeId       mateNodeId
)
    {
    int mateDFSNum, baseLowNum;
    jmdlEmbeddedIntArray_getInt (pParams->pNodeToDFSNum, &mateDFSNum, mateNodeId);
    jmdlEmbeddedIntArray_getInt (pParams->pNodeToLowNum, &baseLowNum, baseNodeId);

    if (mateDFSNum < baseLowNum)
        {
        setArrayAroundVertex (pParams->pNodeToLowNum, mateDFSNum, pParams->pGraph, baseNodeId);
        }

    /* Loop edge is always part of component */
    if (pParams->componentMask)
        jmdlMTGGraph_setMask (pParams->pGraph, baseNodeId, pParams->componentMask);
    }

/*----------------------------------------------------------------------*//**
* Recursive search step.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static void     exploreStrongComponentFromRootCandidate
(
DFSParams       *pParams,
MTGNodeId       rootNodeId
)
    {
    MTGNodeId mateNodeId;
    int       mateDFSNum;
    int       rootDFSNum;
    jmdlEmbeddedIntArray_getInt (pParams->pNodeToDFSNum, &rootDFSNum, rootNodeId);

    if (rootDFSNum < 0)
        {
        /* This is the first entry to this vertex. */
        rootDFSNum = pParams->numVertex;
        enterVertex (pParams, rootNodeId);

        MTGARRAY_VERTEX_LOOP (currNodeId, pParams->pGraph, rootNodeId)
            {
            if (jmdlMTGGraph_getMask (pParams->pGraph, currNodeId, pParams->directionMask))
                {
                mateNodeId = jmdlMTGGraph_getEdgeMate (pParams->pGraph, currNodeId);
                if (jmdlMTGGraph_getMask (pParams->pGraph, mateNodeId, pParams->onStackMask))
                    {
                    /* Edge goes back up the stack.  This creates a loop!!! */
                    updateLowNumAndMasksOnLoopEdge (pParams, rootNodeId, mateNodeId);
                    }
                else
                    {
                    jmdlEmbeddedIntArray_getInt (pParams->pNodeToDFSNum, &mateDFSNum, mateNodeId);
                    if (mateDFSNum < 0)
                        {
                        /* First visit to the neighbor vertex. Continue the search and back out
                            what was discovered. */
                        exploreStrongComponentFromRootCandidate (pParams, mateNodeId);
                        updateLowNumAndMasksAfterRecursion (pParams, currNodeId, mateNodeId);
                        }
                    else if (mateDFSNum > rootDFSNum)
                        {
                        /* Forward edge, provides an alternate path to another vertex
                                previously reached from this root. */
                        updateForwardEdgeMasks (pParams, currNodeId, mateNodeId);
                        }
                    else if (mateDFSNum < rootDFSNum)
                        {
                        /* Cross edge to another branch.   Prior complete exploration
                            from that node failed to reach this vertex, so it is in
                            a different component. */
                        updateCrossEdgeMasks (pParams, currNodeId, mateNodeId);
                        }
                    else /* mateDFSNum == dfsNum.  Edge to same vertex.  Treat it
                            as a loop edge. */
                        {
                        updateLowNumAndMasksOnLoopEdge (pParams, currNodeId, mateNodeId);
                        }
                    }
                }
            }
        MTGARRAY_END_VERTEX_LOOP (currNodeId, pParams->pGraph, rootNodeId)
        leaveVertex (pParams, rootNodeId);
        }
    }



/*----------------------------------------------------------------------*//**
* Search a (directed) graph for strong components, applying masks to identify
* edges that join components from edges within components.
*
*<ul>
*<li>Clear bridge and component masks everywhere.</li>
*<li></li>
*</ul>
* @param directionMask => mask which identifies the forward direction
*           of the directed edges.
* @param componentMask => mask to be applied to all nodes which have direction
*               mask set and whose face successors are in the same strong component.
*               May be zero.
* @param bridgeMask => mask to be applied to all nodes which have the direction
*               mask set and whose successors are in a different connected
*               component.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_markEdgesWithinStrongComponents
(
MTGGraph        *pGraph,
MTGMask         directionMask,
MTGMask         componentMask,
MTGMask         bridgeMask
)
    {
    DFSParams params;

    params.onStackMask = jmdlMTGGraph_grabMask (pGraph);

    params.directionMask = directionMask;
    params.componentMask = componentMask;
    params.bridgeMask    = bridgeMask;

    params.pNodeToLowNum = jmdlEmbeddedIntArray_grab ();
    params.pNodeToDFSNum = jmdlEmbeddedIntArray_grab ();

    params.totalNodes = jmdlMTGGraph_getNodeIdCount (pGraph);
    params.numVertex = 0;
    params.pGraph    = pGraph;

    jmdlMTGGraph_clearMaskInSet (pGraph,
            params.onStackMask & bridgeMask & componentMask);

    jmdlEmbeddedIntArray_setConstant (params.pNodeToLowNum, -1, params.totalNodes);
    jmdlEmbeddedIntArray_setConstant (params.pNodeToDFSNum, -1, params.totalNodes);

    MTGARRAY_SET_LOOP (rootNodeId, pGraph)
        {
        exploreStrongComponentFromRootCandidate (&params, rootNodeId);
        }
    MTGARRAY_END_SET_LOOP (rootNodeId, pGraph)

    jmdlMTGGraph_dropMask (pGraph, params.onStackMask);
    jmdlEmbeddedIntArray_drop (params.pNodeToLowNum);
    jmdlEmbeddedIntArray_drop (params.pNodeToDFSNum);
    }

typedef struct
    {
        MTGGraph    *pGraph;
        MTGMask     activeEdgeMask;     /* Mask identifying FSucc paths to follow.   If null,
                                                                                    cross all edges as if undirected graph.
                                                                    */
        MTGMask     pathEdgeMask;       /* Mask to be applied to path edges. */
        MTGMask     pathVertexMask;     /* Mask to be applied to path edges. */
        MTGMask     visitMask;              /* Mask to show prior visit on THIS pass */
        MTGMask     visitMask1;             /* Mask to show prior visit on first pass */
        MTGMask     visitMask2;             /* Mask to show prior visit on first pass */
        } AnyPathSearchParams;

/*----------------------------------------------------------------------*//**
* Search and mark an undirected graph for vertices and edges that are
* on any path from nodeIdA to nodeIdB.  This does not enumerate the paths;
* it marks which edges and vertices are part of the paths.
* @param pathEdgeMask => mask to place on (both sides of) all edges crossed on any path.
* @param pathVertexMask => mask to place around every vertex along any path.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static MTGMask  jmdlMTGGraph_recursive_markPathsToMarkedVertices
(
AnyPathSearchParams *pParams,
MTGNodeId                   nodeId
)
    {
    MTGMask targetReached = false;
    if (!jmdlMTGGraph_getMask (pParams->pGraph, nodeId, pParams->visitMask))
        {
        targetReached = jmdlMTGGraph_getMask
                            (
                            pParams->pGraph,
                            nodeId,
                            pParams->pathVertexMask
                            );

        jmdlMTGGraph_setMaskAroundVertex (pParams->pGraph, nodeId, pParams->visitMask);

        MTGARRAY_VERTEX_LOOP (currNodeId, pParams->pGraph, nodeId)
            {
            if (   !pParams->activeEdgeMask
                || jmdlMTGGraph_getMask (pParams->pGraph, currNodeId, pParams->activeEdgeMask))
                {
                if (jmdlMTGGraph_recursive_markPathsToMarkedVertices (pParams,
                            jmdlMTGGraph_getFSucc (pParams->pGraph, currNodeId)))
                    {
                    if (!targetReached)
                        {
                        jmdlMTGGraph_setMaskAroundVertex (pParams->pGraph, nodeId,
                                                    pParams->pathVertexMask);
                        targetReached = true;
                        }
                    jmdlMTGGraph_setMaskAroundEdge (pParams->pGraph, currNodeId,
                                                pParams->pathEdgeMask);

                    }
                }
            }
        MTGARRAY_END_VERTEX_LOOP (currNodeId, pParams->pGraph, nodeId)
        }
    return targetReached;
    }


/*----------------------------------------------------------------------*//**
* Search and mark an undirected graph for vertices and edges that are
* on any path from nodeIdA to nodeIdB.  This does not enumerate the paths;
* it marks which edges and vertices are part of the paths.
* @param pathEdgeMask => mask to place on (both sides of) all edges crossed on any path.
* @param pathVertexMask => mask to place around every vertex along any path.
* @param activeEdgeMask => mask identifying face successor pointers that are to be crossed.
*                   If this is a null mask, all edges are crossed, i.e. the entire graph
*                           is undirected.
* @return true if any paths were found.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGMask    jmdlMTGGraph_markPathsBetweenNodes
(
MTGGraph        *pGraph,
MTGNodeId   nodeIdA,
MTGNodeId   nodeIdB,
MTGMask     pathEdgeMask,
MTGMask     pathVertexMask,
MTGMask     activeEdgeMask
)
    {
    AnyPathSearchParams params;
    MTGMask             foundAnyPath = false;

    params.pathVertexMask   = pathVertexMask;
    params.pathEdgeMask     = pathEdgeMask;
    params.activeEdgeMask   = activeEdgeMask;
    params.pGraph = pGraph;

    params.visitMask1 = jmdlMTGGraph_grabMask (pGraph);
    params.visitMask2 = jmdlMTGGraph_grabMask (pGraph);

    if (   params.visitMask1
        && params.visitMask2
        && params.pathVertexMask
        )
        {
        jmdlMTGGraph_clearMaskInSet (pGraph,
                                          params.pathVertexMask
                                        | params.pathEdgeMask
                                        | params.visitMask1
                                        | params.visitMask2
                                    );
            /* Sweep 1: Seed nodeB as the only target. Mark main path */
        params.visitMask = params.visitMask1;
        jmdlMTGGraph_setMaskAroundVertex (pGraph, nodeIdB, pathVertexMask);

        if (jmdlMTGGraph_recursive_markPathsToMarkedVertices (&params, nodeIdA))
            {
            /* Sweep 2: Mark side paths */
            params.visitMask = params.visitMask2;
            foundAnyPath = jmdlMTGGraph_recursive_markPathsToMarkedVertices (&params, nodeIdA);
            }
        }
    jmdlMTGGraph_dropMask (pGraph, params.visitMask1);
    jmdlMTGGraph_dropMask (pGraph, params.visitMask2);
    return foundAnyPath;
    }

/*-----------------------------------------------------------------*//**
* Search for the highest degree (number of vertices) of a face
* with given masking conditions.
* @bsihdr                                       EarlinLutz      10/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlMTGGraph_maxFaceDegreeInSet
(
MTGGraph     *pGraph,
MTGMask             includeMask,
MTGMask             skipMask
)
    {
    MTGMask visitMask = jmdlMTGGraph_grabMask (pGraph);
    MTGMask mySkipMask  = visitMask | skipMask;
    MTGMask currMask;
    int currFaceDegree;
    int maxFaceDegree = 0;
    jmdlMTGGraph_clearMaskInSet (pGraph, visitMask);

    MTGARRAY_SET_LOOP (startNodeId, pGraph)
        {
        currMask = jmdlMTGGraph_getMask (pGraph, startNodeId, mySkipMask);
        if (   !(currMask & visitMask)
            && (!includeMask || (includeMask & currMask))
            && (!skipMask || !(skipMask & currMask))
            )
            {
            jmdlMTGGraph_setMaskAroundFace (pGraph, startNodeId, visitMask);
            currFaceDegree = jmdlMTGGraph_countNodesAroundFace (pGraph, startNodeId);
            if (currFaceDegree > maxFaceDegree)
                maxFaceDegree = currFaceDegree;
            }
        }
    MTGARRAY_END_SET_LOOP (startNodeId, pGraph)
    jmdlMTGGraph_dropMask (pGraph, visitMask);

    return maxFaceDegree;
    }

/*---------------------------------------------------------------------------------**//**
* @description Query a subset of the faces of the given graph to determine if the type of
*       the given label is MTG_LabelMask_FaceProperty.  The label type as stored in the MTG
*       header is ignored.
* <P>
* The optional boundaryMask restricts node search around each vertex to one side of the manifold.
*       A boundary mask may be set throughout the MTG by jmdlMTGGraph_setEdgeStarClassificationMasks.
*
* @param    pGraph          IN OUT  graph to search
* @param    labelOffset     IN      offset of label whose mask to compute
* @param    boundaryMask    IN      edge mask preset along MTG boundary (or zero to treat MTG as 2-manifold)
* @param    pSeedArray      IN      array as returned by jmdlMTGGraph_collectAndNumberFaceLoops,
*                                   jmdlMTGGraph_collectFaceLoopsInEdgeConnectedComponents, etc.
* @param    startIndex      IN      first face loop seed to consider
* @param    endIndex        IN      last face loop seed to consider
* @return true if label is constant across each face in the subset of the graph
* @bsimethod                                                    DavidAssaf      02/03
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGGraph_isLabelMaskFaceProperty
(
const MTGGraph*         pGraph,
int                     labelOffset,
MTGMask                 boundaryMask,
const EmbeddedIntArray* pSeedArray,
int                     startIndex,
int                     endIndex
)
    {
    MTGNodeId   seedId;
    const int*  pSeed;
    int         i, nSeed, faceLabel, label;

    if (   !pGraph
        || !pGraph->IsValidLabelIndex (labelOffset)
        || !pSeedArray
        || NULL == (pSeed = jmdlEmbeddedIntArray_getConstPtr (pSeedArray, 0)))
        return false;

    if (startIndex < 0)
        startIndex = 0;
    if (endIndex >= (nSeed = jmdlEmbeddedIntArray_getCount (pSeedArray)))
        endIndex = nSeed - 1;

    // loop over subset of interior faces
    for (i = startIndex; i <= endIndex; i++)
        {
        if ((seedId = pSeed[i]) < 0 || jmdlMTGGraph_getMask (pGraph, seedId, MTG_EXTERIOR_MASK))
            continue;

        jmdlMTGGraph_getLabel (pGraph, &faceLabel, seedId, labelOffset);

        MTGARRAY_FACE_LOOP (nodeId, pGraph, seedId)
            {
            jmdlMTGGraph_getLabel (pGraph, &label, nodeId, labelOffset);
            if (label != faceLabel)
                return false;
            }
        MTGARRAY_END_FACE_LOOP (nodeId, pGraph, seedId)
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description Query the vertices of a subset of the faces of the given graph to determine if the type of
*       the given label is MTG_LabelMask_VertexProperty.  The label type as stored in the MTG
*       header is ignored.
* @remarks The optional boundaryMask restricts node search around each vertex to one side of the manifold.
*       A boundary mask may be set throughout the MTG by jmdlMTGGraph_setEdgeStarClassificationMasks.
* @param    pGraph          IN OUT  graph to search (grabs/uses/drops a temporary mask)
* @param    labelOffset     IN      offset of label whose mask to compute
* @param    boundaryMask    IN      edge mask preset along MTG boundary (or zero to treat MTG as 2-manifold)
* @param    pSeedArray      IN      array as returned by jmdlMTGGraph_collectAndNumberFaceLoops,
*                                   jmdlMTGGraph_collectFaceLoopsInEdgeConnectedComponents, etc.
* @param    startIndex      IN      first face loop seed to consider
* @param    endIndex        IN      last face loop seed to consider
* @return true if label is constant across each vertex loop in the subset of the graph
* @bsimethod                                                    DavidAssaf      02/03
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGGraph_isLabelMaskVertexProperty
(
MTGGraph*               pGraph,
int                     labelOffset,
MTGMask                 boundaryMask,
const EmbeddedIntArray* pSeedArray,
int                     startIndex,
int                     endIndex
)
    {
    MTGNodeId   seedId;
    MTGMask     visitMask = MTG_NULL_MASK;
    const int*  pSeed;
    int         i, nSeed, vertexLabel, label;

    if (   !pGraph
        || !pGraph->IsValidLabelIndex (labelOffset)
        || !pSeedArray
        || NULL == (pSeed = jmdlEmbeddedIntArray_getConstPtr (pSeedArray, 0)))
        return false;

    if (startIndex < 0)
        startIndex = 0;
    if (endIndex >= (nSeed = jmdlEmbeddedIntArray_getCount (pSeedArray)))
        endIndex = nSeed - 1;

    visitMask = jmdlMTGGraph_grabMask (pGraph);
    if (!visitMask)
        return false;

    jmdlMTGGraph_clearMaskInSet (pGraph, visitMask);

    // loop over vertex loops of a subset of interior faces
    for (i = startIndex; i <= endIndex; i++)
        {
        if ((seedId = pSeed[i]) < 0 || jmdlMTGGraph_getMask (pGraph, seedId, MTG_EXTERIOR_MASK))
            continue;

        MTGARRAY_FACE_LOOP (nodeId, pGraph, seedId)
            {
            if (!jmdlMTGGraph_getMask (pGraph, nodeId, visitMask))
                {
                jmdlMTGGraph_getLabel (pGraph, &vertexLabel, nodeId, labelOffset);

                MTG_MANIFOLD_VERTEX_LOOP_BEGIN (nnodeId, pGraph, nodeId, boundaryMask)
                    {
                    jmdlMTGGraph_setMask (pGraph, nnodeId, visitMask);

                    jmdlMTGGraph_getLabel (pGraph, &label, nnodeId, labelOffset);
                    if (label != vertexLabel)
                        return false;
                    }
                MTG_MANIFOLD_VERTEX_LOOP_END (nnodeId, pGraph, nodeId, boundaryMask)
                }
            }
        MTGARRAY_END_FACE_LOOP (nodeId, pGraph, seedId)
        }

    jmdlMTGGraph_dropMask (pGraph, visitMask);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description Query the edges of a subset of the faces of the given graph to determine if the type of
*       the given label is MTG_LabelMask_EdgeProperty.  The label type as stored in the MTG
*       header is ignored.
* <P>
* The optional boundaryMask restricts node search around each vertex to one side of the manifold.
*       A boundary mask may be set throughout the MTG by jmdlMTGGraph_setEdgeStarClassificationMasks.
*
* @param    pGraph          IN OUT  graph to search
* @param    labelOffset     IN      offset of label whose mask to compute
* @param    boundaryMask    IN      edge mask preset along MTG boundary (or zero to treat MTG as 2-manifold)
* @param    pSeedArray      IN      array as returned by jmdlMTGGraph_collectAndNumberFaceLoops,
*                                   jmdlMTGGraph_collectFaceLoopsInEdgeConnectedComponents, etc.
* @param    startIndex      IN      first face loop seed to consider
* @param    endIndex        IN      last face loop seed to consider
* @return true if label is constant across each edge in the subset of the graph
* @bsimethod                                                    DavidAssaf      02/03
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGGraph_isLabelMaskEdgeProperty
(
const MTGGraph*         pGraph,
int                     labelOffset,
MTGMask                 boundaryMask,
const EmbeddedIntArray* pSeedArray,
int                     startIndex,
int                     endIndex
)
    {
    MTGNodeId   seedId, mateId;
    const int*  pSeed;
    int         i, nSeed, edgeLabel, label;

    if (   !pGraph
        || !pGraph->IsValidLabelIndex (labelOffset)
        || !pSeedArray
        || NULL == (pSeed = jmdlEmbeddedIntArray_getConstPtr (pSeedArray, 0)))
        return false;

    if (startIndex < 0)
        startIndex = 0;
    if (endIndex >= (nSeed = jmdlEmbeddedIntArray_getCount (pSeedArray)))
        endIndex = nSeed - 1;

    // loop over subset of interior faces
    for (i = startIndex; i <= endIndex; i++)
        {
        if ((seedId = pSeed[i]) < 0 || jmdlMTGGraph_getMask (pGraph, seedId, MTG_EXTERIOR_MASK))
            continue;

        MTGARRAY_FACE_LOOP (nodeId, pGraph, seedId)
            {
            mateId = jmdlMTGGraph_getEdgeMate (pGraph, nodeId);
            if (nodeId < mateId)
                {
                jmdlMTGGraph_getLabel (pGraph, &edgeLabel, nodeId, labelOffset);
                jmdlMTGGraph_getLabel (pGraph, &label, mateId, labelOffset);
                if (label != edgeLabel)
                    return false;
                }
            }
        MTGARRAY_END_FACE_LOOP (nodeId, pGraph, seedId)
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description Query (a subset of) the faces of the given graph to determine the type of
*       the given label.  The label type as stored in the MTG header is ignored.
* @remarks If label values are constant around each face, then return MTG_LabelMask_FaceProperty;
*       if constant around each vertex, MTG_LabelMask_VertexProperty; if constant around each
*       edge, MTG_LabelMask_EdgeProperty; otherwise, return MTG_LabelMask_SectorProperty.
* <P>
* The optional boundaryMask restricts node search around each vertex to one side of the manifold.
*       A boundary mask may be set throughout the MTG by jmdlMTGGraph_setEdgeStarClassificationMasks.
*
* @param    pGraph              IN OUT  graph to search
* @param    labelOffset         IN      offset of label whose mask (e.g., MTG_LabelMask_SectorProperty) to compute
* @param    boundaryMask        IN      edge mask preset along MTG boundary (or zero to treat MTG as 2-manifold)
* @param    pFaceLoopSeedArray  IN      optimization; array as returned by jmdlMTGGraph_collectAndNumberFaceLoops,
*                                       jmdlMTGGraph_collectFaceLoopsInEdgeConnectedComponents, etc. (or NULL)
* @param    startIndex          IN      (if pFaceLoopSeedArray) first face loop seed to consider
* @param    endIndex            IN      (if pFaceLoopSeedArray) last face loop seed to consider
* @return label mask as computed from the labels, or MTG_LabelMask_None if error
* @bsimethod                                                    DavidAssaf      02/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGLabelMask    jmdlMTGGraph_computeLabelMask
(
const MTGGraph*         pGraph,
int                     labelOffset,
MTGMask                 boundaryMask,
const EmbeddedIntArray* pFaceLoopSeedArray,
int                     startIndex,
int                     endIndex
)
    {
    EmbeddedIntArray*   pSeedArray = const_cast<EmbeddedIntArray*>(pFaceLoopSeedArray);
    MTGLabelMask        labelMask;

    if (!pGraph || !pGraph->IsValidLabelIndex (labelOffset))
        return MTG_LabelMask_None;

    if (!pSeedArray)
        {
        if (NULL == (pSeedArray = jmdlEmbeddedIntArray_grab()))
            return MTG_LabelMask_None;

        if (0 == (endIndex = jmdlMTGGraph_collectAndNumberFaceLoops (pGraph, pSeedArray, NULL)))
            {
            jmdlEmbeddedIntArray_drop (pSeedArray);
            return MTG_LabelMask_None;
            }

        startIndex = 0;
        endIndex -= 1;
        }

    if (jmdlMTGGraph_isLabelMaskFaceProperty (pGraph, labelOffset, boundaryMask, pSeedArray, startIndex, endIndex))
        labelMask = MTG_LabelMask_FaceProperty;
    else if (jmdlMTGGraph_isLabelMaskVertexProperty (const_cast<MTGGraph*>(pGraph), labelOffset, boundaryMask, pSeedArray, startIndex, endIndex))
        labelMask = MTG_LabelMask_VertexProperty;
    else if (jmdlMTGGraph_isLabelMaskEdgeProperty (pGraph, labelOffset, boundaryMask, pSeedArray, startIndex, endIndex))
        labelMask = MTG_LabelMask_EdgeProperty;
    else
        labelMask = MTG_LabelMask_SectorProperty;

    if (pSeedArray != pFaceLoopSeedArray)
        jmdlEmbeddedIntArray_drop (pSeedArray);

    return labelMask;
    }

/*-----------------------------------------------------------------*//**
Set a label at each node around a vertex.
@param pGraph IN containing graph
@param vertexNodeId IN any node at the vertex
@param labelOffset IN offset to address the label
@param labelValue IN value to store.
* @bsihdr                                       EarlinLutz      07/07
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setLabelAroundVertex
(
MTGGraph     *pGraph,
MTGNodeId    vertexNodeId,
int          labelOffset,
int          labelValue
)
    {
    pGraph->SetLabelAroundVertex (vertexNodeId, labelOffset, labelValue);
    }

/*-----------------------------------------------------------------*//**
Set a label at each node around a face
@param pGraph IN containing graph
@param vertexNodeId IN any node at the face
@param labelOffset IN offset to address the label
@param labelValue IN value to store.
* @bsihdr                                       EarlinLutz      07/07
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setLabelAroundFace
(
MTGGraph     *pGraph,
MTGNodeId    vertexNodeId,
int          labelOffset,
int          labelValue
)
    {
    pGraph->SetLabelAroundFace (vertexNodeId, labelOffset, labelValue);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
