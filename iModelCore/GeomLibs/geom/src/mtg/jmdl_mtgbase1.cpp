/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/mtg/jmdl_mtgbase1.cpp $
|
| Copyright (c) 2001;  Bentley Systems, Inc., 685 Stockton Drive,
|                      Exton PA, 19341-0678, USA.  All Rights Reserved.
|
| This program is confidential, proprietary and unpublished property of Bentley Systems
| Inc. It may NOT be copied in part or in whole on any medium, either electronic or
| printed, without the express written consent of Bentley Systems, Inc.
|
+--------------------------------------------------------------------------------------*/




/*----------------------------------------------------------------------+
|                                                                       |
|
|
|
|
|
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Include Files                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/* @dllName mtg */

/*----------------------------------------------------------------------+
|                                                                       |
|   Local defines                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#define MTG_DELETED_NODEID (-2)     /* also defined in jmdl_mtgmemory.c */
#define MTG_NODE_IS_DELETED(pNode) ((pNode)->vSucc == MTG_DELETED_NODEID)
#define NUM_INTS_IN_BASE_NODE_STRUCT (3)
#define NODE_TO_ARRAY_INDEX(_pGraph,_nodeId) ((_pGraph)->numIntPerNode * (_nodeId))

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
static MTG_Node tempNode;


Public GEOMDLLIMPEXP int jmdlMTGGraph_getNodeCount (MTGGraphCP pGraph)
    {
    return pGraph ? (int)pGraph->GetActiveNodeCount () : 0;
    }

Public GEOMDLLIMPEXP int jmdlMTGGraph_getNodeIdCount (MTGGraphCP pGraph)
    {
    return (int)pGraph->GetNodeIdCount ();
    }

Public GEOMDLLIMPEXP bool    jmdlMTGGraph_isValidNodeId (MTGGraphCP pGraph, MTGNodeId nodeId)
    {
    return pGraph->IsValidNodeId (nodeId);
    }


/**
* @param pGraph    => Containing graph
* @param candidateNodeid => suggested nodeId.
* @return any valid node id, or null node id if there are no nodes.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_getAnyValidNodeId
(
MTGGraphCP        pGraph,
      MTGNodeId candidateNodeId
)
    {
    if (pGraph->IsValidNodeId (candidateNodeId))
        return candidateNodeId;

    return pGraph->AnyValidNodeId ();
    }


static int s_numAllocatedGraphs = 0;
/**
* Create a new graph.  The return value from this function is the
* to be used as the pGraph argument on all subsequent operations on the
* graph.
* @see
* @return pointer to newly allocated and initialized graph.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGGraphP jmdlMTGGraph_newGraph
(
void
)
    {
    return new MTGGraph ();
    }


/**
* Initialize a graph header.
* @param pGraph    => Containing graph
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_initGraph
(
MTGGraphP pGraph
)
    {
    if (pGraph)
        {
        MTGGraphInternalAccess::DeleteNodes (pGraph);
        MTGGraphInternalAccess::InitNonNodeParts (pGraph, false);
        }
    }



/**
* Eliminate all nodes from the graph, but preserve allocated memory
* to allow quick rebuild.   Label data is destroyed!!!
* @param pGraph                  <=> graph to be emptied.
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_empty
(
MTGGraphP pGraph
)
    {
    if (pGraph)
        {
        MTGGraphInternalAccess::DeleteNodes (pGraph);
        MTGGraphInternalAccess::InitNonNodeParts (pGraph, false);
        }
    }


/**
*
* Eliminate all nodes from the graph, but preserve allocated memory
* to allow quick rebuild.  Optionally preserves label definitions.
*
* @param  pGraph   <=> graph to be emptied.
* @param  preserveLabelDefinitions => true if the graph is going to be reused with
*               the same label structure.
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_emptyNodes
(
MTGGraphP pGraph,
bool        preserveLabelDefinitions
)
    {
    if (pGraph)
        {
        pGraph->ClearNodes (preserveLabelDefinitions);
        }
    }



/**
* Eliminate all nodes from the graph, and free memory that was used
* for them.
* @param pGraph  <=> graph to be emptied.
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_releaseMem
(
MTGGraphP pGraph
)
    {
    if (pGraph)
        {
        MTGGraphInternalAccess::DeleteNodes (pGraph);
        MTGGraphInternalAccess::InitNonNodeParts (pGraph, false);
        }
    }


/**
* @param pDestGraph    <= destination graph
* @param pSourceGraph => source graph
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_copy
(
MTGGraphP   pDestGraph,
MTGGraphCP      pSourceGraph
)
    {
    if (NULL != pDestGraph && NULL != pSourceGraph)
        *pDestGraph = *pSourceGraph; // TODO: confirm destructor calls in destination.  Confirm deep copy of bvector.
    }


/**
* Free graph header and all associated memory.
* @param pGraph  <=> graph to be freed.
* @see
* @return MTGGraphCP
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGGraphP jmdlMTGGraph_freeGraph
(
MTGGraphP pGraph
)
    {
    if (pGraph)
        {
        delete pGraph;
        s_numAllocatedGraphs--;
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @param pGraph    => Containing graph
* @param nodeId     => requested node
* @return a pointer to the requested node's structure.
* @bsihdr                                       EarlinLutz          10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTG_Node     *jmdlMTGGraph_getNodePtr
(
MTGGraph     *pGraph,
MTGNodeId    nodeId
)
    {
    return pGraph->GetNodeP (nodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @param pGraph    => Containing graph
* @param nodeId     => requested node
* @return a pointer to the requested node's structure.
* @bsihdr                                       EarlinLutz          10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP const MTG_Node   *jmdlMTGGraph_getConstNodePtr
(
const   MTGGraph   *pGraph,
const   MTGNodeId  nodeId
)
    {
    return pGraph->GetNodeCP (nodeId);
    }


/**
* Free the edge (2 nodes) originating at a given node.
* If nodeId is a deleted node (e.g. the edge was already freed from
* the other end, nothing is done.
* @param pGraph    <=> containing graph
* @param nodeId => one of the two node ids on the edge.
* @return true if the node id was valid.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        jmdlMTGGraph_dropEdge
(
MTGGraphP pGraph,
MTGNodeId  nodeId
)
    {
    return pGraph->DropEdge (nodeId);
    }


/**
* Inverse of split.   Input nodeId = one of the two nodes added by split.
* Delete both nodes and reconnect the edge.
* @param pGraph    <=> containing graph
* @param node0Id => one of the two node ids on the edge.
* @return true if the input node id was one of (exactly) two nodes at its vertex.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_healEdge
(
MTGGraphP       pGraph,
MTGNodeId  node0Id
)
    {
    return pGraph->HealEdge (node0Id);
    }




/**
* Add to the number of labels on each node of the graph.
* Call this immediately after the graph is created.
* @param pGraph   <=> graph in which label count is set.
* @param userTag => tag for later queries
* @param labelType => special properties to use for this label.
* @param defaultValue => value to assign to new nodes.
* @see
* @return offset that may be used to set and get the label values from nodes.
*                       -1 if the graph is null or cannot store more labels.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_defineLabel
(
MTGGraphP               pGraph,
int             userTag,
MTGLabelMask   labelType,
int             defaultValue
)
    {
    return pGraph->DefineLabel (userTag, labelType, defaultValue);
    }


/**
* @param pGraph    => graph being queried.
* @param userTag    => tag
* @see
* @return label offset for the indicated tag.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_getLabelOffset
(
MTGGraphCP pGraph,
int                 userTag
)
    {
    int offset;
    if (pGraph->TrySearchLabelTag (userTag, offset))
        return offset;
    return -1;
    }


/**
* @param pGraph    => graph whose label count is being queried.
* @see
* @return number of labels per node.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_getLabelCount
(
MTGGraphCP pGraph
)
    {
    return (int)pGraph->GetLabelCount ();
    }


/**
*
* Set the value of one label of one node of the graph.
*
* @param pGraph    <=> graph in which label is applied
* @param nodeId => node whose label is set.
* @param offset => offset within labels of node
* @param value => new label value.
* @see
* @return true unless graph pointer, node id, or offset is invalid.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_setLabel
(
MTGGraphP               pGraph,
MTGNodeId      nodeId,
int             offset,
int             value
)
    {
    return pGraph->TrySetLabel (nodeId, offset, value);
    }


/**

* @param pGraph    <=> graph in which data is copied
* @param MTGNodeId      sourceNodeId <=> destination node
* @param destNodeId => source node
* @see
* @return SUCCESS unless graph pointer or either node id is invalid.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_copyMasksAndLabels
(
MTGGraphP               pGraph,
MTGNodeId      sourceNodeId,
MTGNodeId      destNodeId
)
    {
    bool    boolstat = false;
    if (!pGraph)
        {
        }
    else
        {
        MTG_Node *pSourceNode = pGraph->GetNodeP ( sourceNodeId);
        MTG_Node *pDestNode   = pGraph->GetNodeP ( destNodeId);
        if (pSourceNode && pDestNode)
            {
            pDestNode->mask = pSourceNode->mask;
            int value;
            for (int labelIndex = 0; pGraph->TryGetLabel (sourceNodeId, labelIndex, value); labelIndex++)
                pGraph->TrySetLabel (destNodeId, labelIndex, value);
            boolstat = true;
            }
        }
    return boolstat;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGGraph_copyPartialLabels                          |
|                                                                       |
| author        EarlinLutz                               7/97           |
|                                                                       |
| Copy masks and labels indicated by parameters.  Others are left       |
| unchanged.                                                            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_copyPartialLabels
(
MTGGraphP      pGraph,
MTGNodeId      destNodeId,
MTGNodeId      sourceNodeId,
MTGMask maskSelect,
unsigned long   labelSelect
)
    {
    return MTGGraphInternalAccess::CopyPartialLabels (pGraph, destNodeId, sourceNodeId, maskSelect, labelSelect);
    }


/**
*
* @param pGraph    => containing graph.
* @param pLabel <= label value.
* @param nodeId => node id whose label is queried.
* @param offset => offset of queried label
* @see
* @return true unless graph pointer, node id, or offset is invalid.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_getLabel
(
MTGGraphCP pGraph,
int             *pLabel,
MTGNodeId      nodeId,
int             offset
)
    {
    return pGraph->TryGetLabel (nodeId, offset, *pLabel);
    }



/**
* Create an edge with two VU nodes in two distinct vertex loops.
* Return ids of both new nodes.
* @param pGraph    <=> Containing graph
* @param pId0 <= start id of new edge
* @param pId1 <= end id of new edge
* @return true unless nodes could not be created.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_createEdge
(
MTGGraphP               pGraph,
MTGNodeId       *pId0,
MTGNodeId       *pId1
)
    {
    return pGraph->CreateEdge (*pId0, *pId1);
    }


/**
* Create an edge with two VU nodes at a single node. Return both new
* node id's.
* @param pGraph    <=> Containing graph
* @param pId0 <= 'inside' id
* @param pId1 <= 'outside' id
* @see
* @return SUCCESS unless nodes could not be created
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_createSling
(
MTGGraphP               pGraph,
MTGNodeId       *pId0,
MTGNodeId       *pId1
)
    {
    return pGraph->CreateSling (*pId0, *pId1);
    }


/**
* Create a new vertex that splits an existing edge.  Return node ids
* for the two VU nodes at the vertex.
* If idBase is MTG_NULL_NODEID, create a new sling and return
* pointers to its inside and outside.   This is useful when isolated
* faces  intialize a base pointer to MTG_NULL_NODEID, then apply
* splitEdge, each time advancing the base pointer to one of the
* new nodes.
* @param pGraph    <=> Containing graph
* @param pId0 <= New node on same side as idBase
* @param pId1 <= New node on opposite side
* @param idBase => base of edge being split
* @return true unless nodes could not be created.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_splitEdge
(
MTGGraphP               pGraph,
MTGNodeId       *pId0,
MTGNodeId       *pId1,
MTGNodeId      baseId
)
    {
    return pGraph->SplitEdge (*pId0, *pId1, baseId);
    }


/**
* Create a new edge that joins node A to node B.
* @param pGraph    <=> Containing graph
* @param pIdNewA <= New node at vertex of node A
* @param pIdNewB <= New node at vertex of node B
* @param idA => node A
* @param idB => node B
* @param maskA => mask to apply to NewA
* @param maskB => mask to apply to newB
* @return true if nodes were created.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_join
(
MTGGraphP               pGraph,
MTGNodeId       *pIdNewA,
MTGNodeId       *pIdNewB,
MTGNodeId      idA,
MTGNodeId      idB,
MTGMask maskA,
MTGMask maskB
)
    {
    return pGraph->Join (*pIdNewA, *pIdNewB, idA, idB, maskA, maskB);
    }


/**
* Create multiple edges along a pair of paths.
* @param pGraph             <=> Containing graph
* @param idA        =>  start of first path.  Successive nodes to join are
*                       reached by FACE SUCCESSOR from this point.
* @param idB        =>  start of second path.  Successive nodes to join are
*                       reached by FACE SUCCESSOR from this point.
* @param numJoin    => number of edges to add
* @param maskA      => mask to apply to new nodes on the A path
* @param maskB      => mask to apply to new nodes on the B path
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_multipleJoin
(
MTGGraphP               pGraph,
MTGNodeId      idA,
MTGNodeId      idB,
int             numJoin,
MTGMask maskA,
MTGMask maskB
)
    {
    int i;
    MTGNodeId currIdA = idA;
    MTGNodeId currIdB = idB;
    MTGNodeId newIdA, newIdB;
    bool    boolstat = true;

    for (i = 0; boolstat && i < numJoin; i++)
        {
        currIdA = pGraph->FSucc (currIdA);
        currIdB = pGraph->FSucc (currIdB);
        boolstat = pGraph->Join (newIdA, newIdB, currIdA, currIdB, maskA, maskB);
        }
    return boolstat;
    }


/**
* Add multiple nodes within a given edge.
* @param pGraph    <=> Containing graph
* @param idA => base node of existing edge
* @param numVert => number of internal vertices to create
* @param maskLeft => mask to apply on the idA side.
* @param maskRight => mask to apply on the opposite side.
* @return true unless nodes could not be created.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_multipleSplit
(
MTGGraphP               pGraph,
MTGNodeId      idA,
int             numVert,
MTGMask maskLeft,
MTGMask maskRight
)
    {
    int i;
    MTGNodeId idLeft, idRight;
    MTGNodeId idStart = idA;
    bool    boolstat = true;
    for (i = 0; boolstat && i < numVert; i++)
        {
        boolstat = pGraph->SplitEdge (idLeft, idRight, idStart);
        if (boolstat)
            {
            if (maskLeft)
                pGraph->SetMaskAt (idLeft, maskLeft);
            if (maskRight)
                pGraph->SetMaskAt (idRight, maskRight);
            idStart = idLeft;
            }
        }
    return boolstat;
    }





/**
* @param pGraph    => containing graph.
* @param pFPred <= face predecessor
* @param pFSucc <= face successor
* @param pVPred <= vertex predecessor
* @param pVSucc <= vertex successor
* @param pEdgeMate <= edge mate on face successor edge
* @param pPredEdgeMate <= edge mate on face predecessor edge
* @param nodeId => starting node id.
* @see
* @return true if node id's are valid
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_getNeighbors
(
   MTGGraphCP pGraph,
        MTGNodeId      *pFPred,
        MTGNodeId      *pFSucc,
        MTGNodeId      *pVPred,
        MTGNodeId      *pVSucc,
        MTGNodeId      *pEdgeMate,
        MTGNodeId      *pPredEdgeMate,
        MTGNodeId       nodeId
)
    {
    if (!pGraph->IsValidNodeId (nodeId))
        {
        *pFPred = *pFSucc = *pVPred = *pVSucc = *pEdgeMate = *pPredEdgeMate = MTG_NULL_NODEID;
        return false;
        }
    *pFSucc = pGraph->FSucc (nodeId);
    *pEdgeMate = pGraph->VSucc (*pFSucc);
    *pVPred = pGraph->FSucc (*pEdgeMate);

    *pVSucc = pGraph->VSucc (nodeId);
    *pPredEdgeMate = pGraph->FSucc (*pVSucc);
    *pFPred = pGraph->VSucc (*pPredEdgeMate);
    return true;
    }

Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_getEdgeMate (MTGGraphCP pGraph, MTGNodeId nodeId)
    {return pGraph->EdgeMate (nodeId);}

Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_getVSucc (MTGGraphCP pGraph, MTGNodeId nodeId)
    {return pGraph->VSucc (nodeId);}

Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_getVPred (MTGGraphCP pGraph, MTGNodeId nodeId)
    {return pGraph->VPred (nodeId);}

Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_getFPred (MTGGraphCP pGraph, MTGNodeId nodeId)
    {return pGraph->FPred (nodeId);}

Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_getFSucc (MTGGraphCP pGraph, MTGNodeId nodeId)
    {return pGraph->FSucc (nodeId);}

Public GEOMDLLIMPEXP MTGMask jmdlMTGGraph_grabMask (MTGGraphP pGraph)
    {return pGraph->GrabMask ();}



Public GEOMDLLIMPEXP void jmdlMTGGraph_setMaskProperty
(
MTGGraphP       pGraph,
MTGMask     mask,
MTGLabelMask        selector,
bool                value
)
    {
    pGraph->SetMaskProperty (mask, selector, value);
    }

Public GEOMDLLIMPEXP MTGMask jmdlMTGGraph_getMaskProperty
(
MTGGraphCP pGraph,
MTGMask     queryMask,
MTGLabelMask        selector
)
    {
    return pGraph->GetMaskProperty (queryMask, selector);
    }


Public GEOMDLLIMPEXP void jmdlMTGGraph_dropMask
(
MTGGraphP       pGraph,
MTGMask mask
)
    {
    pGraph->DropMask (mask);
    }

Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_yankEdgeFromVertex
(
MTGGraphP               pGraph,
MTGNodeId       nodeId
)
    {
    return pGraph->YankEdgeFromVertex (nodeId);
    }


Public GEOMDLLIMPEXP bool    jmdlMTGGraph_vertexTwist
(
MTGGraphP               pGraph,
MTGNodeId       node0,
MTGNodeId       node1
)
    {
    return pGraph->VertexTwist (node0, node1);
    }



Public GEOMDLLIMPEXP void jmdlMTGGraph_reverse
(
MTGGraphP               pGraph
)
    {
    pGraph->ReverseFaceAndVertexLoops ();
    }

//#include <geommem.h>



/**
* Build mtg nodes from explicit permutations.
* pFSucc[i] = face successor of node i
* pEdgeMate[i] = edge mate of node i
* Both arrays must be permutations, i.e.  for each i there is only one
*   j such that pFSucc[j]==i and one k such that pEdgeMate[k]==i.
* The edge mate relation must be a "nonsingular involution",
*   i.e. pEdgeMate[i] != i and pEdgeMate[pEdgeMate[i]]==i
*
* @param pGraph    <=> containing graph.
* @param pFSuccArray => array giving the face successor of each node.
* @param pEdgeMateArray     => array giving the edge mate of each node
* @param numNode            => number of nodes.
* @see
* @return SUCCESS unless nodes could not be allocated or permutations are invalid.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    jmdlMTGGraph_buildFromFSuccAndEdgeMatePermutations
(
MTGGraph       *pGraph,
const int       *pFSuccArray,
const int       *pEdgeMateArray,
int             numNode
)
    {
    return MTGGraphInternalAccess::BuildFromFSuccAndEdgeMatePermutations (pGraph, pFSuccArray, pEdgeMateArray, numNode);
    }


END_BENTLEY_GEOMETRY_NAMESPACE
