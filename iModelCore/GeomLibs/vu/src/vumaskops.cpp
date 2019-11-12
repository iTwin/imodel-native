/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#include <math.h>


/******************************************************************************
**
** Search and mark operations.
** File created 2008.  Compared to older functions, these tend to
**   take all mask values as arguments -- e.g. no VU_BOUNDARY_EDGE and VU_EXTERIOR_EDGE
**   assumptions.  Hence these are "medium size" pieces that used to be hidden
**   as non-reusable parts of larger operations that had implicit assumptions about special edge names.
**
*******************************************************************************/

/*---------------------------------------------------------------------------------**//**
@description Find each edge with mask.
  Spread the mask to all other edges around those faces.
@group "VU Node Masks"
@bsimethod                                                    EarlinLutz      10/08
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_spreadMaskAroundFaces
(
VuSetP pGraph,
VuMask mask
)
    {
    VuMask visitMask = vu_grabMask (pGraph);
    vu_clearMaskInSet (pGraph, visitMask);
    VU_SET_LOOP (pSeedNode, pGraph)
        {
        if (!vu_getMask (pSeedNode, visitMask))
            {
            if (vu_countMaskAroundFace (pSeedNode, mask) > 0)
                vu_setMaskAroundFace (pSeedNode, mask);
            vu_setMaskAroundFace (pSeedNode, visitMask);
            }
        }
    END_VU_SET_LOOP (pSeedNode, pGraph)
    vu_returnMask (pGraph, visitMask);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void       vu_spreadExteriorMasksToAdjacentFaces
(
VuSetP pGraph,
bool negativeSeedsOnly,
VuMask triggerMask, // Identifies candidate faces.
VuMask markMask    //  mask to apply
)
    {
    VuMask  myMask = vu_grabMask (pGraph);
    VuMask  visitMask = vu_grabMask (pGraph);

    /* Set "myMask" around all faces with any edge touching a vertex with any exterior edges. */
    vu_clearMaskInSet (pGraph, myMask);
    vu_clearMaskInSet (pGraph, visitMask);

    VU_SET_LOOP (pBase, pGraph)
        {
        if (!vu_getMask (pBase, visitMask))
            {
            vu_setMaskAroundFace (pBase, visitMask);


            bool isCandidate = 0 != vu_getMask (pBase, triggerMask);
            if (isCandidate && negativeSeedsOnly)
                isCandidate = 0.0 > vu_area (pBase);

            if (isCandidate)
                {
                vu_setMaskAroundFace (pBase, myMask);
                VU_FACE_LOOP (pFace, pBase)
                    {
                    VU_VERTEX_LOOP (pCurr, pFace)
                        {
                        if (!vu_getMask (pCurr, myMask))
                            vu_setMaskAroundFace (pCurr, myMask);
                        }
                    END_VU_VERTEX_LOOP (pCurr, pFace)
                    }
                END_VU_FACE_LOOP (pFace, pBase)
                }
            }
        }
    END_VU_SET_LOOP (pBase, pGraph)

    /* Convert "myMask" edges to exterior  | mask1*/
    VU_SET_LOOP (pBase, pGraph)
        {
        if (vu_getMask (pBase, myMask))
            vu_setMask (pBase, markMask);
        }
    END_VU_SET_LOOP (pBase, pGraph)

    vu_returnMask (pGraph, visitMask);
    vu_returnMask (pGraph, myMask);
    }

/*---------------------------------------------------------------------------------**//**
* @description  Clear mask throughout set.
*    Find all negative area faces.   Recursively apply mask to all faces reached without crossing
*    barrier mask.
* @param pGraph IN graph to search
* @param barrierMask IN mask of uncrossable edges.
* @param floodMask IN mask to apply
* @group "VU Node Masks"
@bsimethod                                                    EarlinLutz      10/08
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_floodFromNegativeAreaFaces
(
VuSetP pGraph,
VuMask barrierMask,
VuMask floodMask
)
    {
    VuArrayP pStack = vu_grabArray (pGraph);
    VuMask visitMask = vu_grabMask (pGraph);
    double a, aMin;
    vu_clearMaskInSet (pGraph, floodMask | visitMask);
    aMin = 0.0;
    // Collect negative area faces ...
    VU_SET_LOOP (pSeedNode, pGraph)
        {
        if (!vu_getMask (pSeedNode, visitMask))
            {
            vu_setMaskAroundFace (pSeedNode, visitMask);
            a = vu_area (pSeedNode);
            if (a < 0.0)
                {
                vu_arrayAdd (pStack, pSeedNode);
                if (a < aMin)
                    aMin = a;
                }
            }
        }
    END_VU_SET_LOOP (pSeedNode, pGraph)

    // Now that we know a big (negative) area for comparison,
    //  we can toss the small ones and mark the good ones....
    VuP pExteriorSeed;
    double aTol = 1.0e-12 * aMin;   // That's negative !!!
    for (vu_arrayOpen (pStack); vu_arrayRead (pStack, &pExteriorSeed);)
        {
        a = vu_area (pExteriorSeed);
        if (a > aTol)   // it's a sliver ...
            vu_arrayRemoveCurrent (pStack);
        else
            vu_setMaskAroundFace (pExteriorSeed, floodMask);
        }


    // Recurseively search across non-boundary edges ...
    while (NULL != (pExteriorSeed = vu_arrayRemoveLast (pStack)))
        {
        // Look for neighbors separated by boundaries ...
        VU_FACE_LOOP (pCurr, pExteriorSeed)
            {
            VuP pMate = vu_edgeMate (pCurr);
            if (   !vu_getMask (pCurr, barrierMask)
                && !vu_getMask (pMate, barrierMask)
                && !vu_getMask (pMate, floodMask))
                {
                vu_setMaskAroundFace (pMate, floodMask);
                vu_arrayAdd (pStack, pMate);
                }
            }
        END_VU_FACE_LOOP (pCurr, pExteriorSeed)
        }
    vu_returnMask (pGraph, visitMask);
    vu_returnArray (pGraph, pStack);
    }

/*---------------------------------------------------------------------------------**//**
@description
 For each face
    recursively search to neightbors without crossing any edge with barrier mask.
    (This visits a connected component bounded by the specified mask.)
    Delete the edges crossed in the search.
 (I.E.: Form a spanning tree over edge-connected faces.   Delete the edges crossed by the tree.
    Leave non-tree edges in place.)
@param pGraph IN graph to search
@param barrierMask IN mask of uncrossable edges.
@remarks
    In a graph marked with VU_BOUNDARY_EDGE and VU_EXTERIOR_EDGE,
    passing VU_BOUNDARY_EDGE will expand both interior and exterior faces, leaving the minimal number of
    edges (both interior and exterior) needed to provide connectivity among the boundary edges.
    Passing VU_BOUNDARY_EDGE | VU_EXTERIOR_EDGE will only expand interior faces.
@group "VU Node Masks"
@bsimethod                                                    EarlinLutz      10/08
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_expandFacesToBarrier
(
VuSetP pGraph,
VuMask barrierMask
)
    {
    VuArrayP pStack = vu_grabArray (pGraph);
    VuMask visitMask = vu_grabMask (pGraph);
    VuMask deleteMask = vu_grabMask (pGraph);
    vu_clearMaskInSet (pGraph, visitMask | deleteMask);
    VuMask visitedOrBarrierMask = visitMask | barrierMask;

    VU_SET_LOOP (pSeedNode, pGraph)
        {
        if (!vu_getMask (pSeedNode, visitMask))
            {
            vu_setMaskAroundFace (pSeedNode, visitMask);
            vu_arrayClear (pStack);
            vu_arrayAdd (pStack, pSeedNode);
            VuP pPopNode;
            while (NULL != (pPopNode = vu_arrayRemoveLast (pStack)))
                {
                vu_setMaskAroundFace (pPopNode, visitMask);
                VU_FACE_LOOP (pFaceNode, pPopNode)
                    {
                    VuP pEdgeMate = vu_edgeMate (pFaceNode);

                    if (!vu_getMask (pEdgeMate, visitedOrBarrierMask))
                        {
                        vu_setMask (pFaceNode, deleteMask);
                        vu_setMask (pEdgeMate, deleteMask);
                        // Mate will be visited from stack, not by edge cross.
                        vu_setMaskAroundFace (pEdgeMate, visitMask);
                        vu_arrayAdd (pStack, pEdgeMate);
                        }
                    }
                END_VU_FACE_LOOP (pFaceNode, pPopNode)
                }
            }
        }
    END_VU_SET_LOOP (pSeedNode, pGraph)

    vu_freeMarkedEdges (pGraph, deleteMask);
    vu_returnMask (pGraph, deleteMask);
    vu_returnMask (pGraph, visitMask);
    vu_returnArray (pGraph, pStack);
    }

//! Maintain a mask on complete vertex loops.
//! "Mark vertex" means both (a) set a mask around the vertex and (b) record the (single) seed node in an array.
//! The mask allows any node at the marked vertex to be immediately recognized.
//! The array allows quick return to all marked verts.
struct MarkedVertexSet
{
bvector<VuP>mNodes;
VuMask mMask;
VuSetP mGraph;
MarkedVertexSet (VuSetP pGraph)
    {
    mGraph = pGraph;
    mMask = vu_grabMask (mGraph);
    vu_clearMaskInSet (mGraph, mMask);
    }
~MarkedVertexSet ()
    {
    vu_returnMask (mGraph, mMask);
    }

void MarkVertex (VuP node)
    {
    if (!vu_getMask (node, mMask))
        {
        vu_setMaskAroundVertex (node, mMask);
        mNodes.push_back (node);
        }
    }

void MarkAllVertsAroundFace (VuP faceSeed)
    {
    VU_FACE_LOOP (node, faceSeed)
        {
        MarkVertex (node);
        }
    END_VU_FACE_LOOP (node, faceSeed)
    }


bool IsVertexMarked (VuP node)
    {
    return 0 != vu_getMask (node, mMask);
    }


// Extend this edge both forward and backward beyond degree 2 vertices.
// Search the remaining nodes for any in the markset.
bool IsAnyVertexAroundFaceMarkedOtherThanThisExtendedEdge (VuP edgeNode)
    {
    size_t numAroundFace = (size_t)vu_countEdgesAroundFace (edgeNode);
    if (numAroundFace <= 2)
        return false;
    size_t numIncluded = 2;
    
    VuP nodeA = edgeNode;
    VuP nodeB = vu_fsucc (edgeNode);
    // Advance B end forward to nontrivial vertex ...
    while (numIncluded < numAroundFace && vu_vsucc (vu_vsucc (nodeB)) == nodeB)
        {
        nodeB = vu_fsucc (nodeB);
        numIncluded++;
        }
    // Advance A end backward to nontrivial vertex ...
    while (numIncluded < numAroundFace && vu_vsucc (vu_vsucc (nodeB)) == nodeB)
        {
        nodeA = vu_fpred (nodeA);
        numIncluded++;
        }

    VuP pTest = nodeB;
    while (numIncluded < numAroundFace)
        {
        pTest = vu_fsucc (pTest);
        if (vu_getMask (pTest, mMask))
            return true;
        numIncluded++;
        }

    return false;
    }

void Reset ()
    {
    for (size_t i = 0; i < mNodes.size (); i++)
        {
        VuP vertexSeed = mNodes[i];
        VU_VERTEX_LOOP (node, vertexSeed)
            {
            vu_clrMask (node, mMask);
            }
        END_VU_VERTEX_LOOP (node, vertexSeed)
        }
    mNodes.clear ();
    }
};
/*---------------------------------------------------------------------------------**//**
@description
 For each face 
    recursively search to neightbors without crossing any edge with barrier mask.
    (This visits a connected component bounded by the specified mask.)
    Delete the edges crossed in the search.
 (I.E.: Form a spanning tree over edge-connected faces, but without using any vertex twice within a face.
    Delete the edges crossed by the tree.
    Leave non-tree edges in place.)
@param pGraph IN graph to search
@param barrierMask IN mask of uncrossable edges.
@remarks
    In a graph marked with VU_BOUNDARY_EDGE and VU_EXTERIOR_EDGE,
    passing VU_BOUNDARY_EDGE will expand both interior and exterior faces, leaving the minimal number of
    edges (both interior and exterior) needed to provide connectivity among the boundary edges.
    Passing VU_BOUNDARY_EDGE | VU_EXTERIOR_EDGE will only expand interior faces.
@group "VU Node Masks"
@bsimethod                                                    EarlinLutz      10/08
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_expandFacesToBarrierWithDistinctVertices
(
VuSetP pGraph,
VuMask barrierMask
)
    {
    static int sMaxDelete = 0;
    VuArrayP pStack = vu_grabArray (pGraph);
    VuMask visitMask = vu_grabMask (pGraph);
    VuMask deleteMask = vu_grabMask (pGraph);
    vu_clearMaskInSet (pGraph, visitMask | deleteMask);
    VuMask visitedOrBarrierMask = visitMask | barrierMask;
    MarkedVertexSet activeVerts (pGraph);
    int numDelete = 0;
    VU_SET_LOOP (pSeedNode, pGraph)
        {
        if (!vu_getMask (pSeedNode, visitMask))
            {
            activeVerts.Reset ();
            vu_setMaskAroundFace (pSeedNode, visitMask);
            vu_arrayClear (pStack);
            vu_arrayAdd (pStack, pSeedNode);
            VuP pPopNode;
            while (NULL != (pPopNode = vu_arrayRemoveLast (pStack)))
                {
                vu_setMaskAroundFace (pPopNode, visitMask);
                activeVerts.MarkAllVertsAroundFace (pPopNode);
                VU_FACE_LOOP (pFaceNode, pPopNode)
                    {
                    VuP pEdgeMate = vu_edgeMate (pFaceNode);

                    if (!vu_getMask (pEdgeMate, visitedOrBarrierMask))
                        {
                        if (!activeVerts.IsAnyVertexAroundFaceMarkedOtherThanThisExtendedEdge (pEdgeMate))
                            {
                            numDelete++;
                            vu_setMask (pFaceNode, deleteMask);
                            vu_setMask (pEdgeMate, deleteMask);
                            // Mate will be visited from stack, not by edge cross.
                            vu_setMaskAroundFace (pEdgeMate, visitMask);
                            vu_arrayAdd (pStack, pEdgeMate);
                            if (sMaxDelete > 0 && numDelete > sMaxDelete)
                                goto cleanup;
                            }
                        }
                    }
                END_VU_FACE_LOOP (pFaceNode, pPopNode)
                }
            }
        }
    END_VU_SET_LOOP (pSeedNode, pGraph)

cleanup:
    vu_freeMarkedEdges (pGraph, deleteMask);
    vu_returnMask (pGraph, deleteMask);
    vu_returnMask (pGraph, visitMask);
    vu_returnArray (pGraph, pStack);
    }



// Given a seed node on an unvisited component.
// Mark the seed and its face with mExterior.
// Flood the component, marking mExterior as updated by parity rules during the markup.
// Based on the most negative (largest) face, flip all markups if needed.
static bool AssignComponentParityByFlood
(
VuSetP pGraph,
VuP    pComponentSeed,   // First node to visit.
VuMask mBoundary,   // Mask showing boundary.
VuMask mExterior,   // Mask to apply to exterior faces.
VuMask mClassified, // Mask to apply to indicate faces reached by flooding.
                    // It is assumed that this mask is clear throughout.
VuArrayP pStack,    // Work array for recursion stack
VuArrayP pFaceSeeds // Work array.  On return, contains one node per face of the component.
)
    {
    VuP pFaceSeed;
    bool bConsistentMasks = true;
    double minArea = DBL_MAX;
    VuP pMinAreaFace = NULL;
    VuMask mVisitAsInterior = mClassified;
    VuMask mVisitAsExterior = mClassified | mExterior;
    vu_arrayClear (pStack);
    vu_arrayClear (pFaceSeeds);

    // Tentatively classify this face as exterior.
    // As part of the component flood, we will find the face with the most-negative area.
    //   That face is REALLY exterior -- may require toggling the tentative classificiation.

    vu_setMaskAroundFace (pComponentSeed, mVisitAsExterior);
    vu_arrayAdd (pStack, pComponentSeed);
    // Each face is added to the stack once and removed once.

    while (NULL != (pFaceSeed = vu_arrayRemoveLast (pStack)))
        {
        vu_arrayAdd (pFaceSeeds, pFaceSeed);
        double area = vu_area (pFaceSeed);
        if (area < minArea)
            {
            minArea = area;
            pMinAreaFace = pFaceSeed;
            }

        // pFaceSeed is a classified face. (It got marked when stacked)
        bool bExterior = vu_getMask (pFaceSeed, mExterior) != 0;
        // This provides a marking for all neighbors.
        // For each unvisited neighbor, we can mark and stack.
        // For each visited neighbor, we can confirm marking to check that boundaries are valid.
        VU_FACE_LOOP (pCurr, pFaceSeed)
            {
            VuP pMate = vu_edgeMate (pCurr);
            bool bMateExterior = vu_getMask (pCurr, mBoundary) == 0
                    ? bExterior
                    : !bExterior;

            if (vu_getMask (pMate, mClassified))
                {
                // The neighbor was previously classified.  We can compare old and new ...
                bool bPriorMateExterior = vu_getMask (pMate, mExterior) != 0;
                if (bPriorMateExterior != bMateExterior)
                    bConsistentMasks = false;
                }
            else
                {
                // this is the first encounter with the neighbor.
                // Mark it and push so that flood or comparison proceeds from the neighbor later...
                if (bMateExterior)
                    vu_setMaskAroundFace (pMate, mVisitAsExterior);
                else
                    vu_setMaskAroundFace (pMate, mVisitAsInterior);
                vu_arrayAdd (pStack, pMate);
                }
            }
        END_VU_FACE_LOOP (pCurr, pFaceSeed)
        }

    if (!vu_getMask (pMinAreaFace, mExterior))
        {
        // The marking is consistently inverted.
        // Go back to each face and toggle ...
        VuP pFaceSeed;
        for (vu_arrayOpen (pFaceSeeds); vu_arrayRead (pFaceSeeds, &pFaceSeed);)
            {
            VU_FACE_LOOP (pCurr, pFaceSeed)
                {
                if (vu_getMask (pCurr, mExterior))
                    vu_clrMask (pCurr, mExterior);
                else
                    vu_setMask (pCurr, mExterior);
                }
            END_VU_FACE_LOOP (pCurr, pFaceSeed)
            }
        }

    return bConsistentMasks;
    }


/*---------------------------------------------------------------------------------**//**
@description
 Apply parity markup to entire graph.
@param pGraph IN graph to search
@param mBoundary IN mask of uncrossable edges.
@param mExterior IN mask to apply as "exterior"
@remarks
   In an edge-connected component, there is (only!!) one face with negative area.
   The negative area face itself is marked mExterior.
   Parity changes at each boundary crossing.
@group "VU Node Masks"
@bsimethod                                                    EarlinLutz      10/08
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_markExteriorByParity
(
VuSetP pGraph,
VuMask mBoundary,
VuMask mExterior
)
    {
    VuMask mClassified  = vu_grabMask (pGraph);
    VuArrayP pStack = vu_grabArray (pGraph);
    VuArrayP pFaceSeeds = vu_grabArray (pGraph);
    vu_clearMaskInSet (pGraph, mClassified | mExterior);
    VU_SET_LOOP (pSeed, pGraph)
        {
        if (!vu_getMask (pSeed, mClassified))
            AssignComponentParityByFlood (pGraph, pSeed, mBoundary, mExterior, mClassified, pStack, pFaceSeeds);
        }
    END_VU_SET_LOOP (pSeed, pGraph)
    vu_returnArray (pGraph, pFaceSeeds);
    vu_returnArray (pGraph, pStack);
    vu_returnMask (pGraph, mClassified);
    }

Public GEOMDLLIMPEXP void vu_countFacesWithMask
(
VuSetP pGraph,
VuMask mask,
int &num0,
int &numMixed,
int &numAll
)
    {
    num0 = numMixed = numAll = 0;
    VuMask mVisit = vu_grabMask (pGraph);
    vu_clearMaskInSet (pGraph, mVisit);
    VU_SET_LOOP (pFace, pGraph)
        {
        if (!vu_getMask (pFace, mVisit))
            {
            vu_setMaskAroundFace (pFace, mVisit);
            int numNode = vu_countEdgesAroundFace (pFace);
            int numMasked = vu_countMaskAroundFace (pFace, mask);
            if (numMasked == 0)
                num0++;
            else if (numMasked < numNode)
                numMixed++;
            else
                numAll++;
            }
        }
    END_VU_SET_LOOP (pFace, pGraph)
    vu_returnMask (pGraph, mVisit);
    }

void vu_exchangeNullFacesToBringMaskInside (VuSetP pGraph, VuMask mask)
    {
    size_t numNull = 0;
    size_t numReversed = 0;
    // When two input faces "butt against each other along an edge,
    // the angular sort logic (in vu_merge) has no way of knowing which
    // of the geometrically identical edges is logicaly "first" in the sort order.
    // The caller tells us a mask that is preferred "inside" when present once on each edge.
    // Look for the mask appearing "outside" twice and reverse the edges.
    //
    // If the input partitions the plane and has no overlaps, the resulting graph is "exactly" the original
    // polygons glued together with double edges everywhere.
    VU_SET_LOOP (nodeA, pGraph)
        {
        VuP nodeB = vu_fsucc (nodeA);
        if (vu_fsucc (nodeB) == nodeA)
            {
            numNull++;
            VuP nodeA1 = vu_vsucc (nodeA);
            VuP nodeB1 = vu_vsucc (nodeB);
            if (   vu_getMask (nodeA1, mask)
                && !vu_getMask (nodeA, mask)
                && vu_getMask (nodeB1, mask)
                && !vu_getMask (nodeB, mask))
                {
                VuP nodeB0 = vu_vpred (nodeB);
                VuP nodeA0 = vu_vpred (nodeA);

                vu_vertexTwist (pGraph, nodeB, nodeB0);
                vu_vertexTwist (pGraph, nodeA, nodeA1);

                vu_vertexTwist (pGraph, nodeB, nodeB1);
                vu_vertexTwist (pGraph, nodeA1, nodeA0);
                numReversed++;
                }
            }
        }
    END_VU_SET_LOOP (nodeA, pGraph)
    }

bool IsSlitFace (VuP nodeA)
    {
    VuP nodeB = nodeA->FSucc ();
    VuP nodeC = nodeB->FSucc ();
    return nodeB != nodeA && nodeC == nodeA;
    }
void vu_sortMaskToFrontOfBundle (VuSetP pGraph, VuMask mask, bool targetMask)
    {
    // nodeA is a candidate as running along the to be OUTSIDE of a bundle, and at the "top"
    // nodeB, at the other end of the nodeA, is also outside of the bundle.
    // The forward vertex loop from nodeB is where the mask is tested.
    VU_SET_LOOP (nodeA, pGraph)
        {
        if (!IsSlitFace (nodeA))
            {
            VuP nodeB = vu_fsucc (nodeA);
            VuP nodeQ = nodeB;  // hot edges move to nodeQ and nodeQ mvoes to the (just moved) hot sector.
            VuP nodeP0 = nodeQ;  // nodeP will walk to each edge at this end of the bundle.
            VuP nodeP1 = nodeP0->VSucc ();
            if (IsSlitFace (nodeP1))
                {
                // Always: nodeP1 is an edge in the bundle (possibly at the bottom of a null face, possibly (once) the non-null sector at the last bundle edge
                do {
                    // consider moving nodeP1 as successor of nodeQ (and if so make it nodeQ)
                    if (!nodeP1->HasMask (mask) == targetMask)
                        {
                        // leave it in place .. advance P0 to P1
                        nodeP0 = nodeP1;
                        }
                    else if (nodeQ == nodeP0)
                        {
                        // nodeP1 is masked but already at the leading edge of the maskees.
                        nodeP0 = nodeP1;
                        }
                    else
                        {
                        // Yank each end of edge P1 from its sector ...
                        auto nodeR1 = nodeP1->FSucc ();  // other end of the slit face.
                        auto nodeR2 = nodeR1->VSucc (); // edge mate of P1. (P1, R2) is the edge being moved.
                        auto nodeQ1 = nodeQ->FPred ();       // insertion point for the P2 end of the (P, P2) edge
                        pGraph->VertexTwist (nodeP0, nodeP1);
                        pGraph->VertexTwist (nodeR1, nodeR2);
                        // insert back at Q
                        pGraph->VertexTwist (nodeP1, nodeQ);
                        pGraph->VertexTwist (nodeR2, nodeQ1);
                        // nodeP0 stays where it is -- we have made progress by throwing the edge ahead to somewhere behind.
                        }
                    nodeP1 = nodeP0->VSucc ();
                    } while (IsSlitFace (nodeP0));
                }
            }
        }
    END_VU_SET_LOOP (nodeA, pGraph)
    }


END_BENTLEY_GEOMETRY_NAMESPACE
