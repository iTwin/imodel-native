/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/mtg/jmdl_mtgmask.cpp $
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
* Drop all edges that match a given mask condition.
* @param pGraph    <=> containing graph
* @param mask => identifying mask
* @param value => 0 to drop unmasked, 1 to drop masked
* @see
* @return number of dropped edges.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_dropByMask
(
MTGGraphP   pGraph,
MTGMask    mask,
int         value
)
    {
    int numDrop = 0;
    if (value)
        {
        MTGARRAY_SET_LOOP (nodeId, pGraph)
            {
            if (jmdlMTGGraph_getMask (pGraph, nodeId, mask))
                {
                jmdlMTGGraph_dropEdge (pGraph, nodeId);
                numDrop++;
                }
            }
        MTGARRAY_END_SET_LOOP (nodeId, pGraph)
        }
    else
        {
        MTGARRAY_SET_LOOP (nodeId, pGraph)
            {
            if (   !jmdlMTGGraph_getMask (pGraph, nodeId, mask)
                && !jmdlMTGGraph_getMask (pGraph, jmdlMTGGraph_getEdgeMate (pGraph, nodeId), mask))
                {
                jmdlMTGGraph_dropEdge (pGraph, nodeId);
                numDrop++;
                }
            }
        MTGARRAY_END_SET_LOOP (nodeId, pGraph)
        }
    return numDrop;
    }


/**
* Drop all edges that match a given mask condition on both sides of the edge
* @param pGraph    <=> containing graph
* @param mask => identifying mask
* @param value => 0 to drop unmasked, 1 to drop masked
* @see
* @return number of dropped edges.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGGraph_dropByDoubleMask
(
MTGGraphP   pGraph,
MTGMask    mask,
int         value
)
    {
    int numDrop = 0;
    if (value)
        {
        MTGARRAY_SET_LOOP (nodeId, pGraph)
            {
            if (  jmdlMTGGraph_getMask (pGraph, nodeId, mask)
               && jmdlMTGGraph_getMask (pGraph,
                            jmdlMTGGraph_getEdgeMate (pGraph, nodeId), mask)
               )
                {
                jmdlMTGGraph_dropEdge (pGraph, nodeId);
                numDrop++;
                }
            }
        MTGARRAY_END_SET_LOOP (nodeId, pGraph)
        }
    else
        {
        MTGARRAY_SET_LOOP (nodeId, pGraph)
            {
            if (  !jmdlMTGGraph_getMask (pGraph, nodeId, mask)
               && !jmdlMTGGraph_getMask (pGraph,
                            jmdlMTGGraph_getEdgeMate (pGraph, nodeId), mask)
               )
                {
                jmdlMTGGraph_dropEdge (pGraph, nodeId);
                numDrop++;
                }
            }
        MTGARRAY_END_SET_LOOP (nodeId, pGraph)
        }
    return numDrop;
    }

/**
* @param pGraph    => containing graph
* @param nodeId => node Id whose mask bits are queried.
* @param mask => selector for mask bits desired.
* @see
* @return MTGMask
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGMask jmdlMTGGraph_getMask
(
MTGGraphCP         pGraph,
        MTGNodeId       nodeId,
        MTGMask mask

)
    {
    return pGraph->GetMaskAt (nodeId, mask);
    }



/**
* Set selected mask bits on a node in a graph.
* @param pGraph    <= graph in which mask is to be set.
* @param nodeId => node id in which mask is to be set.
* @param mask => selector for mask bits to be set.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setMask
(
MTGGraphP       pGraph,
MTGNodeId       nodeId,
MTGMask mask
)
    {
    pGraph->SetMaskAt (nodeId, mask);
    }


/**
* Clear mask bits on all nodes in the graph.
* @param pGraph    <= graph in which mask is to be cleared.
* @param mask => selector for mask bits to be cleared.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_clearMaskInSet
(
MTGGraphP       pGraph,
MTGMask mask
)
    {
    // Brute force here...
    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        jmdlMTGGraph_clearMask (pGraph, currNodeId, mask);
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)
    }

/**
* Set a mask bit on all nodes in the graph.
* @param pGraph    <= graph in which mask is to be set.
* @param mask => selector for mask bits to be set.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setMaskInSet
(
MTGGraphP       pGraph,
MTGMask mask
)
    {
    // Brute force here...
    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        jmdlMTGGraph_setMask (pGraph, currNodeId, mask);
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)
    }


/**
* Toggle the value of a given mask throughout a graph.
* @param pGraph    <= graph in which mask is to be set.
* @param mask => selector for mask bits to be set.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_toggleMaskInSet
(
MTGGraphP       pGraph,
MTGMask mask
)
    {
    // Brute force here...
    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        jmdlMTGGraph_toggleMask (pGraph, currNodeId, mask);
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)
    }


/**
* Perform an AND, OR, XOR, or NAND operation between two specified masks
* on all nodes in the graph.
* @param pGraph    <=> containing graph.
* @param mask0 => first mask to read
* @param mask1 => second mask to read
* @param result00 => result for !mask0, !mask1
* @param result01 => result for !mask0,  mask1
* @param result10 => result for  mask0, !mask1
* @param result11 => result for  mask0,  mask1
* @param mask2 => mask to set when boolOp is satisfied.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_maskedBooleanInSet
(
MTGGraphP   pGraph,
MTGMask    mask0,
MTGMask    mask1,
bool        result00,
bool        result01,
bool        result10,
bool        result11,
MTGMask    mask2
)
    {
    MTGMask combinedMask = mask0 | mask1;
    MTGMask currMask;
    int selector;
    bool    keepIt;
    int boolOp = 0;

    if (result00)
        boolOp |= 0x01;
    if (result01)
        boolOp |= 0x02;
    if (result10)
        boolOp |= 0x04;
    if (result11)
        boolOp |= 0x08;


    MTGARRAY_SET_LOOP (nodeId, pGraph)
        {
        currMask = jmdlMTGGraph_getMask (pGraph, nodeId, combinedMask);
        selector = 1;
        if (currMask & mask0)
            selector = selector << 2;
        if (currMask & mask1)
            selector = selector << 1;

        keepIt = 0 != (boolOp & selector);

        if (keepIt)
            jmdlMTGGraph_setMask (pGraph, nodeId, mask2);
        else
            jmdlMTGGraph_clearMask (pGraph, nodeId, mask2);
        }
    MTGARRAY_END_SET_LOOP (nodeId, pGraph)
    }



/**
* @param pGraph    <= graph in which mask is to be cleared.
* @param nodeId => node id in which mask is to be cleared.
* @param mask => selector for mask bits to be cleared.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_clearMask
(
MTGGraphP       pGraph,
MTGNodeId       nodeId,
MTGMask mask
)
    {
    pGraph->ClearMaskAt (nodeId, mask);
    }



/**
* @param pGraph    <=> containing graph.
* @param nodeId => specific node whose mask is to be changed.
* @param mask => mask selector
* @param offOn => 0 to turn mask off, nonzero to turn on.
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_writeMask
(
MTGGraphP       pGraph,
MTGNodeId       nodeId,
MTGMask mask,
int             offOn
)
    {
    if (offOn)
        pGraph->SetMaskAt (nodeId, mask);
    else
        pGraph->ClearMaskAt (nodeId, mask);
    }



/**
* @param pGraph    <=> containing graph.
* @param nodeId => specific node whose mask is to be changed.
* @param mask => mask selector
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_toggleMask
(
MTGGraphP       pGraph,
MTGNodeId       nodeId,
MTGMask mask
)
    {
    if (pGraph->GetMaskAt (nodeId, mask))
        pGraph->ClearMaskAt (nodeId, mask);
    else
        pGraph->SetMaskAt (nodeId, mask);
    }


/**
* @param pGraph    <=> graph to be changed
* @param nodeId => node id
* @param mask => mask to apply
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setMaskAroundFace
(
MTGGraphP       pGraph,
MTGNodeId      nodeId,
MTGMask mask
)
    {
    MTGARRAY_FACE_LOOP (currNodeId, pGraph, nodeId)
        {
        jmdlMTGGraph_setMask (pGraph, currNodeId, mask);
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, nodeId)
    }


/**
* @param pGraph    <=> graph to be changed
* @param nodeId => node id
* @param mask => mask to apply
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setMaskAroundVertex
(
MTGGraphP       pGraph,
MTGNodeId      nodeId,
MTGMask mask
)
    {
    pGraph->SetMaskAroundVertex (nodeId, mask);
    }


/*------------------------------------------------------------------*//**
* Walk around a face.  Set a mask on the edge mate of each node reached.
* @param pGraph    <=> graph to be changed
* @param nodeId => node id
* @param mask => mask to apply
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setEdgeMateMaskAroundFace
(
MTGGraphP       pGraph,
MTGNodeId       nodeId,
MTGMask         mask
)
    {
    MTGARRAY_FACE_LOOP (currNodeId, pGraph, nodeId)
        {
        jmdlMTGGraph_setMask
                (
                pGraph,
                jmdlMTGGraph_getEdgeMate (pGraph, currNodeId),
                mask
                );
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, nodeId)
    }


/*------------------------------------------------------------------*//**
* Walk around a vertex.  Set a mask on the edge mate of each node reached.
* @param pGraph    <=> graph to be changed
* @param nodeId => node id
* @param mask => mask to apply
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setEdgeMateMaskAroundVertex
(
MTGGraphP       pGraph,
MTGNodeId       nodeId,
MTGMask         mask
)
    {
    MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, nodeId)
        {
        jmdlMTGGraph_setMask
                (
                pGraph,
                jmdlMTGGraph_getEdgeMate (pGraph, currNodeId),
                mask
                );
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, nodeId)
    }


/*------------------------------------------------------------------*//**
* Walk around a face.  Search for a mask on the edge mate of each node reached.
* Return the (edge mate) node id or null node id.
* @param pGraph    <=> graph to be changed
* @param nodeId => node id
* @param mask => mask to search for
* @return node id where mask is found, or MTG_NULL_NODEID
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_findEdgeMateMaskAroundFace
(
MTGGraphP       pGraph,
MTGNodeId       nodeId,
MTGMask         mask
)
    {
    MTGNodeId mateNodeId;
    MTGARRAY_FACE_LOOP (currNodeId, pGraph, nodeId)
        {
        mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);
        if (jmdlMTGGraph_getMask (pGraph, mateNodeId, mask))
            return mateNodeId;
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, nodeId)
    return MTG_NULL_NODEID;
    }


/*------------------------------------------------------------------*//**
* Walk around a vertex.  Search for a mask on the edge mate of each node reached.
* Return the (edge mate) node id or null node id.
* @param pGraph    <=> graph to be changed
* @param nodeId => node id
* @param mask => mask to search for
* @return node id where mask is found, or MTG_NULL_NODEID
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_findMaskOnEdgeMateAroundVertex
(
MTGGraphP       pGraph,
MTGNodeId       nodeId,
MTGMask         mask
)
    {
    MTGNodeId mateNodeId;
    MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, nodeId)
        {
        mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);
        if (jmdlMTGGraph_getMask (pGraph, mateNodeId, mask))
            return mateNodeId;
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, nodeId)
    return MTG_NULL_NODEID;
    }


/**
* Advance one face loop step and 0 or more vertex loop predecessor
* steps to find a node with a given mask.
* @param pGraph    => graph to be changed
* @param nodeId => node id
* @param mask => mask to search for
* @see
* @return MTGNodeId
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_getMaskedFVSucc
(
MTGGraphCP  pGraph,
MTGNodeId      nodeId,
MTGMask mask
)
    {
    MTGMask currMask;
    nodeId = jmdlMTGGraph_getFSucc (pGraph, nodeId);
    MTGARRAY_VERTEX_PREDLOOP (currNodeId, pGraph, nodeId)
        {
        currMask = jmdlMTGGraph_getMask (pGraph, currNodeId, mask);
        if (currMask)
            return currNodeId;
        }
    MTGARRAY_END_VERTEX_PREDLOOP (currNodeId, pGraph, nodeId)
    return MTG_NULL_NODEID;
    }


/**
* Search the given node and its vertex loop for a masked node.
* @param pGraph    => graph
* @param nodeId => first node to examine.
* @param mask => mask to search for
* @return first masked node reached, or MTG_NULL_NODEID
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_findMaskAroundVertex
(
MTGGraphCP  pGraph,
MTGNodeId      nodeId,
MTGMask mask
)
    {
    MTGMask currMask;
    MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, nodeId)
        {
        currMask = jmdlMTGGraph_getMask (pGraph, currNodeId, mask);
        if (currMask)
            return currNodeId;
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, nodeId)
    return MTG_NULL_NODEID;
    }


/**
* Search the given node and its face loop for a masked node.
* @param pGraph    => graph
* @param nodeId => first node to examine.
* @param mask => mask to search for
* @return first masked node reached, or MTG_NULL_NODEID
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_findMaskAroundFace
(
MTGGraphCP  pGraph,
MTGNodeId      nodeId,
MTGMask mask
)
    {
    MTGMask currMask;
    MTGARRAY_FACE_LOOP (currNodeId, pGraph, nodeId)
        {
        currMask = jmdlMTGGraph_getMask (pGraph, currNodeId, mask);
        if (currMask)
            return currNodeId;
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, nodeId)
    return MTG_NULL_NODEID;
    }


/**
* Search the given node and its vertex loop for a non masked node.
* @param pGraph    => graph
* @param nodeId => first node to examine.
* @param mask => mask to search for
* @return first masked node reached, or MTG_NULL_NODEID
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_findNonMaskAroundVertex
(
MTGGraphCP  pGraph,
MTGNodeId      nodeId,
MTGMask mask
)
    {
    MTGMask currMask;
    MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, nodeId)
        {
        currMask = jmdlMTGGraph_getMask (pGraph, currNodeId, mask);
        if (!currMask)
            return currNodeId;
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, nodeId)
    return MTG_NULL_NODEID;
    }


/**
* Search the given node and its face loop for a non masked node.
* @param pGraph    => graph
* @param nodeId => first node to examine.
* @param mask => mask to search for
* @return first masked node reached, or MTG_NULL_NODEID
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_findNonMaskAroundFace
(
MTGGraphCP  pGraph,
MTGNodeId      nodeId,
MTGMask mask
)
    {
    MTGMask currMask;
    MTGARRAY_FACE_LOOP (currNodeId, pGraph, nodeId)
        {
        currMask = jmdlMTGGraph_getMask (pGraph, currNodeId, mask);
        if (!currMask)
            return currNodeId;
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, nodeId)
    return MTG_NULL_NODEID;
    }



/**
* @param  pGraph   <=> graph to be changed
* @param nodeId => node id
* @param mask => mask to apply
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_clearMaskAroundFace
(
MTGGraphP       pGraph,
MTGNodeId      nodeId,
MTGMask mask
)
    {
    pGraph->ClearMaskAroundFace (nodeId, mask);
    }


/**
* @param pGraph    <=> graph to be changed
* @param nodeId => node id
* @param mask => mask to apply
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_clearMaskAroundVertex
(
MTGGraphP       pGraph,
MTGNodeId      nodeId,
MTGMask mask
)
    {
    pGraph->ClearMaskAroundVertex (nodeId, mask);
    }



/**
* @param pGraph    <=> graph to be changed
* @param nodeId => node id
* @param mask => mask to apply
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_setMaskAroundEdge
(
MTGGraphP       pGraph,
MTGNodeId      nodeId,
MTGMask mask
)
    {
    MTGNodeId mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, nodeId);
    jmdlMTGGraph_setMask (pGraph, nodeId, mask);
    jmdlMTGGraph_setMask (pGraph, mateNodeId, mask);
    }


/**
* @param pGraph    <=> graph to be changed
* @param nodeId => node id
* @param mask => mask to apply
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_clearMaskAroundEdge
(
MTGGraphP       pGraph,
MTGNodeId      nodeId,
MTGMask mask
)
    {
    MTGNodeId mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, nodeId);
    jmdlMTGGraph_clearMask (pGraph, nodeId, mask);
    jmdlMTGGraph_clearMask (pGraph, mateNodeId, mask);
    }


/**
* @param pGraph    => graph
* @param pNumOn => number of masked nodes
* @param pNum0ff => number of unmasked nodes
* @param nodeId => node id
* @param mask => mask to check
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_countMasksAroundVertex
(
MTGGraphCP pGraph,
int                 *pNumOn,
int                 *pNum0ff,
MTGNodeId           nodeId,
MTGMask     mask
)
    {
    int numOn = 0;
    int numOff = 0;
    MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, nodeId)
        {
        if (jmdlMTGGraph_getMask (pGraph, currNodeId, mask))
            {
            numOn++;
            }
        else
            {
            numOff++;
            }
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, nodeId)

    if (pNumOn)
        *pNumOn = numOn;
    if (pNum0ff)
        *pNum0ff = numOff;
    }


/**
* @param pNumOn => number of masked edge mate nodes
* @param pNum0ff => number of unmasked edge matenodes
* @param pGraph    => containing graph
* @param nodeId => node id
* @param mask => mask to check
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_countMateMasksAroundVertex
(
int                 *pNumOn,
int                 *pNum0ff,
MTGGraphCP pGraph,
MTGNodeId           nodeId,
MTGMask     mask
)
    {
    int numOn = 0;
    int numOff = 0;
    MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, nodeId)
        {
        if (jmdlMTGGraph_getMask (pGraph, jmdlMTGGraph_getEdgeMate (pGraph, currNodeId), mask))
            {
            numOn++;
            }
        else
            {
            numOff++;
            }
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, nodeId)

    if (pNumOn)
        *pNumOn = numOn;
    if (pNum0ff)
        *pNum0ff = numOff;
    }


/**
* @param pGraph           => containing graph
* @param pNumOn     <= number of masked nodes (or null)
* @param pNum0ff    <= number of unmasked nodes (or null)
* @param nodeId     => start node
* @param mask       => mask to check
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_countMasksAroundFace
(
MTGGraphCP pGraph,
int             *pNumOn,
int             *pNum0ff,
MTGNodeId       nodeId,
MTGMask         mask
)
    {
    int numOn = 0;
    int numOff = 0;
    MTGARRAY_FACE_LOOP (currNodeId, pGraph, nodeId)
        {
        if (jmdlMTGGraph_getMask (pGraph, currNodeId, mask))
            {
            numOn++;
            }
        else
            {
            numOff++;
            }
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, nodeId)

    if (pNumOn)
        *pNumOn = numOn;
    if (pNum0ff)
        *pNum0ff = numOff;
    }


/**
* @param pNumOn => number of masked edge mate nodes
* @param pNum0ff => number of unmasked edge matenodes
* @param pGraph    => containing graph
* @param nodeId => node id
* @param mask => mask to check
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_countMateMasksAroundFace
(
int                 *pNumOn,
int                 *pNum0ff,
MTGGraphCP pGraph,
MTGNodeId           nodeId,
MTGMask     mask
)
    {
    int numOn = 0;
    int numOff = 0;
    MTGARRAY_FACE_LOOP (currNodeId, pGraph, nodeId)
        {
        if (jmdlMTGGraph_getMask (pGraph, jmdlMTGGraph_getEdgeMate (pGraph, currNodeId), mask))
            {
            numOn++;
            }
        else
            {
            numOff++;
            }
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, nodeId)

    if (pNumOn)
        *pNumOn = numOn;
    if (pNum0ff)
        *pNum0ff = numOff;
    }


/**
* @param pNumOn => number of masked edge mate nodes
* @param pNum0ff => number of unmasked edge matenodes
* @param pGraph    => containing graph
* @param mask => mask to check
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_countMasksInSet
(
int                 *pNumOn,
int                 *pNum0ff,
MTGGraphCP pGraph,
MTGMask     mask
)
    {
    int numOn = 0;
    int numOff = 0;
    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        if (jmdlMTGGraph_getMask (pGraph, jmdlMTGGraph_getEdgeMate (pGraph, currNodeId), mask))
            {
            numOn++;
            }
        else
            {
            numOff++;
            }
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)

    if (pNumOn)
        *pNumOn = numOn;
    if (pNum0ff)
        *pNum0ff = numOff;
    }

/*---------------------------------------------------------------------------------**//**
At each node initially marked with an given mask, set the specified mask around each face
incident to the original vertex.
The motivating use of this is that in an xy triangulation with its range expanded by adding
points "outside" the primary points, this step pushes the exterior masks "into" the triangulation
so the artificial exterior points can be removed.
@param pGraph IN OUT graph to mark
@param seedMask IN mask being spread.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void        jmdlMTGGraph_spreadMaskToAdjacentFaces
(
MTGGraph *pGraph,
MTGMask  seedMask
)
    {
    MTGMask     myMask = jmdlMTGGraph_grabMask (pGraph);

    /* Set "myMask" around all faces with any edge touching a vertex with any exterior edges.*/
    jmdlMTGGraph_clearMaskInSet (pGraph, myMask);

    MTGARRAY_SET_LOOP (baseNodeId, pGraph)
        {
        if (jmdlMTGGraph_getMask (pGraph, baseNodeId, seedMask))
            {
            MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, baseNodeId)
                {
                if (!jmdlMTGGraph_getMask (pGraph, currNodeId, myMask))
                    jmdlMTGGraph_setMaskAroundFace (pGraph, currNodeId, myMask);
                }
            MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, baseNodeId)
            }
        }
    MTGARRAY_END_SET_LOOP (baseNodeId, pGraph)

    /* Convert "myMask" edges to exterior */
    MTGARRAY_SET_LOOP (baseNodeId, pGraph)
        {
        if (jmdlMTGGraph_getMask (pGraph, baseNodeId, myMask))
            jmdlMTGGraph_setMask (pGraph, baseNodeId, seedMask);
        }
    MTGARRAY_END_SET_LOOP (baseNodeId, pGraph)

    jmdlMTGGraph_dropMask (pGraph, myMask);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
