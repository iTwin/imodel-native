/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/mtg/jmdl_mtgmesh.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGGraph_panic                                      |
|                                                                       |
| author        EarlinLutz                               7/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static void jmdlMTGGraph_panic
(
const MTGGraph *  pGraph,
MTGNodeId  id
)
    {
    }


/**
* Check that all nodes in a graph satisfy the successor/predecessor requirements.
* @param pGraph   => graph whose successor/predecessor relations are to be verified.
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGGraph_verifyGraph
(
const MTGGraph *  pGraph
)
    {
    MTGNodeId id1, id2, id3, id4;
    MTGARRAY_SET_LOOP (id0, pGraph)
        {
        id1 = jmdlMTGGraph_getFSucc(pGraph, id0);
        id2 = jmdlMTGGraph_getVSucc(pGraph, id1);
        id3 = jmdlMTGGraph_getFSucc(pGraph, id2);
        id4 = jmdlMTGGraph_getVSucc(pGraph, id3);
        if (    id1 == MTG_NULL_NODEID
            ||  id2 == MTG_NULL_NODEID
            ||  id3 == MTG_NULL_NODEID
            ||  id4 == MTG_NULL_NODEID
            ||  id4 != id0
            ||  id0 == id2
            )
            {
            jmdlMTGGraph_panic(pGraph, id0);
            }
        }
    MTGARRAY_END_SET_LOOP (id0, pGraph)
    }
#ifdef abc
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGGraph_linkExteriorNodes                          |
|                                                                       |
| author        EarlinLutz                               7/97           |
|                                                                       |
| Build a rectangular face grid.                                        |
+----------------------------------------------------------------------*/
static void jmdlMTGGraph_linkExteriorNodes
(
MTG_Node    **ppNode,      // <=> pointers to nodes
MTGNodeId  *pNodeId,       // <=> node indices
int         cornerIndex0,   // => index of corner where side begins.  A 2-noded loop is built here,
                            //    and nodes are assumed numbered consecutively from here.
int         cornerIndex1,   // => index of corner node where side ends.
int         edgeSelect,     // => identifies edge (0=S, 1=E, 2=N, 3=W) being chained
int         nu,             // => number of horizontal edges (face blocks) on complete side
int         nv,             // => number of vertical edges (face blocks) on complete side
MTGMask    boundaryInteriorMask,   // => mask on inside of boundary edges
MTGMask    boundaryExteriorMask,   // => mask on outside of boundary edges
MTGMask    boundaryVertexMask,     // => mask on all nodes along boundary
MTGMask    strutMask,               // => mask on struts attaching to boundary
MTGMask    cornerVertexMask,        // => mask just on corner vertices,
bool        labelVertices,          // => true if vertex labels are to be applied.
int         vertexIdOffset,         // => offset to vertex id label
int         cornerVertexId,         // => id for corner vertex
int         vertex0Id,              // => id for first internal vertex
int         vertexIdStep            // => increment between successive vertex ids.f
)
    {
    MTG_Node *pNode;
    int iStep = 4;
    int jStep = 4 * nu;
    int m0,m, m1;
    int deltaM;
    int numEdge;
    int i, j;       // Face indices for (i,j) form
    int internalK0, internalK1;
    int r0, r1, r, rPrev;           // "rim" (externior) counter.
    int d1, d2;
    int *pLabels;
    int vertexId;

    switch (edgeSelect)
        {
        case 0:
            i = j = 0;
            internalK0 = 0;
            internalK1 = 1;
            deltaM = iStep;
            numEdge = nu;
            d1 = 1;
            d2 = iStep;
            break;
        case 1:
            i = nu - 1;
            j = 0;
            internalK0 = 1;
            internalK1 = 2;
            deltaM = jStep;
            numEdge = nv;
            d1 = 1;
            d2 = jStep;
            break;
        case 2:
            i = nu - 1;
            j = nv - 1;
            internalK0 = 2;
            internalK1 = 3;
            deltaM = -iStep;
            numEdge = nu;
            d1 = 1;
            d2 = -iStep;
            break;
        case 3:
            i = 0;
            j = nv - 1;
            internalK0 = 3;
            internalK1 = 0;
            deltaM = -jStep;
            numEdge = nv;
            d1 = -3;
            d2 = -jStep;
            break;
        default:
            return;
        }

    m0 = i * iStep + j * jStep + internalK0;       // Local index of interior corner node.
    m1 = m0 -internalK0 + (numEdge - 1)* deltaM + internalK1;  // Local index of last interor corner node.


    if (cornerIndex0 != cornerIndex1)     // non-periodic case
        {
        // Link first interior corner node to exterior
        ppNode[m0]->vSucc = pNodeId[cornerIndex0];
        ppNode[m0]->mask |= boundaryInteriorMask | cornerVertexMask | boundaryVertexMask;
        if (labelVertices)
            {
            pLabels = (int *)(ppNode[cornerIndex0] + 1);
            pLabels[vertexIdOffset] = cornerVertexId;
            }

        ppNode[cornerIndex0]->vSucc = pNodeId[m0];
        ppNode[cornerIndex0]->mask |= boundaryExteriorMask | cornerVertexMask | boundaryVertexMask;
        }
    else if (cornerIndex0 == cornerIndex1)    // periodic case
        {
        // Link first interior, last interior, and exterior corner nodes
        ppNode[m0]->vSucc = pNodeId[m1];
        ppNode[m1]->vSucc = pNodeId[cornerIndex0];
        ppNode[m0]->mask |= boundaryInteriorMask | cornerVertexMask | boundaryVertexMask;
        ppNode[m1]->mask |= cornerVertexMask | boundaryVertexMask;
        // seam properties of m1 mask are added by subsequent seam stitcher.
        if (labelVertices)
            {
            pLabels = (int *)(ppNode[cornerIndex0] + 1);
            pLabels[vertexIdOffset] = cornerVertexId;
            }

        ppNode[cornerIndex0]->vSucc = pNodeId[m0];
        ppNode[cornerIndex0]->mask |= boundaryExteriorMask | cornerVertexMask | boundaryVertexMask;
        }

    // face successors along edge point backwards
    r0 = cornerIndex0 + 1;
    r1  = cornerIndex0 + numEdge - 1;
    rPrev = cornerIndex0;
    vertexId = vertex0Id;
    for (r = r0; r <= r1; r++)
        {
        ppNode[r]->fSucc = pNodeId[rPrev];
        rPrev = r;

        if (labelVertices)
            {
            pLabels = (int *)(ppNode[r] + 1);
            pLabels[vertexIdOffset] = vertexId;
            vertexId += vertexIdStep;
            }
        }
    ppNode[cornerIndex1]->fSucc = pNodeId[rPrev];


    // Vertex loops at mid-side nodes
    // The local indexes at a typical step are:
    // |                  |
    // | m            m+d1|m+d2
    // +------------------+----------------------
    //                    r
    // BEWARE THAT THE ABOVE DIAGRAM IS NOT true HORIZONTAL AND VERTICAL.
    // IT MAY BE ROTATED 90, 180, or 270 DEGREES
    // Each step advances r by 1 (because the exterior loops are purely consecutive)
    // and m by deltaM0 (which, like d1 and d2, depends on which global edge we are traversing.
    MTGMask maskR = boundaryExteriorMask | boundaryVertexMask;
    MTGMask maskD1 = strutMask | boundaryVertexMask;
    MTGMask maskD2 = boundaryInteriorMask | boundaryVertexMask;

    for (r = r0, m=m0; r <= r1; r++, m += deltaM)
        {
        pNode = ppNode[r];
        pNode->vSucc = pNodeId[m + d2];
        pNode->mask  |= maskR;

        pNode = ppNode[m + d2];
        pNode->vSucc = pNodeId[m + d1];
        pNode->mask  |= maskD1;

        pNode = ppNode[m + d1];
        pNode->vSucc = pNodeId[r     ];
        pNode->mask  |= maskD2;
        }
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGGraph_linkSeam                                   |
|                                                                       |
| author        EarlinLutz                               7/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static void jmdlMTGGraph_linkSeam
(
MTG_Node    **ppNode,      // <=> pointers to nodes
MTGNodeId  *pNodeId,       // <=> node indices
int         edgeSelect,     // => identifies edge (0=S, 1=E, 2=N, 3=W) being chained
int         nu,             // => number of horizontal edges (face blocks) on complete side
int         nv,             // => number of vertical edges (face blocks) on complete side
int         edgeMask,       // => mask for edge context
int         cornerVertexMask,   // => mask applied only at true corner.
int         midEdgeVertexMask,  // => mask applied at other than true corner.
MTGMask    strutEdgeMask        // => mask on struts attaching to boundary
)
    {
    MTG_Node *pNode;
    int iStep = 4;
    int jStep = 4 * nu;
    int m0, m1, m2, n0, n1, n2;
    int numEdge;
    int dm1, dm2, dn1, dn2;

    switch (edgeSelect)
        {
        case 0:
            numEdge = nu;
            m0 = 0;
            n0 = m0 + jStep * (nv - 1) + 3;
            dm1 = 1;
            dm2 = iStep;
            dn1 = -1;
            dn2 = iStep;
            break;
        case 1:
            numEdge = nv;
            m0 = (nu - 1) * iStep + 1;
            n0 = 0;
            dm1 = 1;
            dm2 = jStep;
            dn1 = 3;
            dn2 = jStep;
            break;
        default:
            return;
        }

    // Vertex loops at mid-side nodes
    // The local indexes at a typical step are:
    // |                  |
    // | m0            m1 | m2
    // +------------------+----------------------
    // | n0            n1 | n2

    MTGMask seamMask = edgeMask | midEdgeVertexMask;
    MTGMask strutMask = strutEdgeMask | midEdgeVertexMask;


    // Special case to set mask from the to the corner vertex:
    pNode = ppNode[n0];
    pNode->mask  |= edgeMask | cornerVertexMask;
    int i;
    for (i = 1; i < numEdge; i++, m0 = m2, n0 = n2)
        {
        m1 = m0 + dm1;
        m2 = m0 + dm2;
        n1 = n0 + dn1;
        n2 = n0 + dn2;

        pNode = ppNode[n1];
        pNode->vSucc = pNodeId[n2];
        pNode->mask  |= seamMask;

        pNode = ppNode[n2];
        pNode->vSucc = pNodeId[m2];
        pNode->mask  |= strutMask;

        pNode = ppNode[m2];
        pNode->vSucc = pNodeId[m1];
        pNode->mask  |= seamMask;

        pNode = ppNode[m1];
        pNode->vSucc = pNodeId[n1];
        pNode->mask  |= strutMask;
        }

    // At loop exit, m0 and n0 are sitting on the vertex just before the right end.
    // Find the right end to add its corner mask
    m1 = m0 + dm1;
    pNode = ppNode[m1];
    pNode->mask  |= edgeMask | cornerVertexMask;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGGraph_linkSingularPoint                                  |
|                                                                       |
| author        EarlinLutz                               7/97           |
|                                                                       |
+----------------------------------------------------------------------*/
static void jmdlMTGGraph_linkSingularPoint
(
MTG_Node    **ppNode,      // <=> pointers to nodes
MTGNodeId  *pNodeId,       // <=> node indices
int         nu,             // => number of horizontal edges (face blocks) on complete side
int         nv,             // => number of vertical edges (face blocks) on complete side
MTGMask    uEdgeMask,      // => mask for (both) u edges at the singular point.
MTGMask    vEdgeMask       // => mask for (both) v edges at the singular point.
)
    {
    MTG_Node *pNode;
    int iStep = 4;
    int jStep = 4 * nu;
    int iJump = (nu - 1) * iStep;   // jump between corresponding nodes on far left and far right faces
    int jJump = (nv - 1) * jStep;   // jump between corresponding nodes on bottom and top faces.

    int k0 = 0;
    int k1 = k0 + iJump + 1;
    int k2 = k0 + iJump + jJump + 2;
    int k3 = k0 + jJump + 3;

    pNode = ppNode[k0];
    pNode->vSucc = pNodeId[k1];
    pNode->mask  |= uEdgeMask;

    pNode = ppNode[k1];
    pNode->vSucc = pNodeId[k2];
    pNode->mask  |= vEdgeMask;

    pNode = ppNode[k2];
    pNode->vSucc = pNodeId[k3];
    pNode->mask  |= uEdgeMask;

    pNode = ppNode[k3];
    pNode->vSucc = pNodeId[k0];
    pNode->mask  |= vEdgeMask;
    }
#endif

// Join upper or vertices of parallel edges
//
void joinEdgesWithVertexCopy (MTGGraphP pGraph, int vertexIdOffset,
        MTGNodeIdPair &edgeA, MTGNodeIdPair &edgeB, bool atTop)
    {
    MTGNodeId nodeIdAA, nodeIdBB;
    jmdlMTGGraph_createEdge (pGraph, &nodeIdAA, &nodeIdBB);
    MTGNodeId nodeIdA, nodeIdB;
    if (atTop)
        {
        nodeIdA = edgeA.nodeId[1];
        nodeIdB = jmdlMTGGraph_getFSucc (pGraph, edgeB.nodeId[0]);
        }
    else
        {
        nodeIdA = jmdlMTGGraph_getFSucc (pGraph, edgeA.nodeId[1]);
        nodeIdB = edgeB.nodeId[0];
        }
    int vertexIndexA, vertexIndexB;
    jmdlMTGGraph_getLabel (pGraph, &vertexIndexA, nodeIdA, vertexIdOffset);
    jmdlMTGGraph_getLabel (pGraph, &vertexIndexB, nodeIdB, vertexIdOffset);

    jmdlMTGGraph_setLabel (pGraph, nodeIdAA, vertexIdOffset, vertexIndexA);
    jmdlMTGGraph_setLabel (pGraph, nodeIdBB, vertexIdOffset, vertexIndexB);
    jmdlMTGGraph_vertexTwist (pGraph, nodeIdA, nodeIdAA);
    jmdlMTGGraph_vertexTwist (pGraph, nodeIdB, nodeIdBB);
    }

void SetEdgeMasksAroundFace (MTGGraph *pGraph, MTGNodeId faceNodeId, MTGMask insideMask, MTGMask mateMask)
    {
    MTGARRAY_FACE_LOOP (currNodeId, pGraph, faceNodeId)
        {
        jmdlMTGGraph_setMask (pGraph, currNodeId, insideMask);
        jmdlMTGGraph_setMask (pGraph,
            jmdlMTGGraph_getEdgeMate (pGraph, currNodeId), insideMask);
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, faceNodeId)
    }


/**
* Build a rectangular face grid.
* @param pGraph    <=> Containing graph
* @param nu => number of edges along constant u direction
* @param nv => number of edges along constant v direction
* @param nuVertexPerRow => number of vertices in stored rows.  Typically nu or nu+1
* @param uPeriodic      => true if seam at left/right
* @param vPeriodic      => true if seam at top/bottom
* @param pMasks         => masks
* @param normalMode     => indicates which labels are to be applied.
* @param vertexIdOffset => offset for vertex id labels.
* @param vertex00Id     => label for lower left vertex.  -1 if no labels needed.
*                          Vertices are labeled in row-major order from here.
* @param sectorIdOffset => offset for sector id labels.  -1 if no labels needed.
* @param sector00Id     => label for lower left vertex sector on main grid.
* @return MTGNodeId
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_buildRectangularGrid
(
MTGGraph *      pGraph,                 // <=> Containing graph
int             nu,                     // => number of edges along constant u direction
int             nv,                     // => number of edges along constant v direction
int             nuVertexPerRow,         // => number of vertices in stored rows.  Typically
                                        //  nu or nu+1
bool            uPeriodic,              // => true if seam at left/right
bool            vPeriodic,              // => true if seam at top/bottom
MTG_RectangularGridMasks *pMasks,       // => masks
MTGFacets_NormalMode normalMode,       // => indicates which labels are to be applied.
int             vertexIdOffset,         // => offset for vertex id labels.
int             vertex00Id,             // => label for lower left vertex.  -1 if no labels needed.
                                        //    Vertices are labeled in row-major order from here.
int             sectorIdOffset,         // => offset for sector id labels.  -1 if no labels needed.
int             sector00Id              // => label for lower left vertex sector on main grid.
)
    {
    MTGNodeId outputNodeId = MTG_NULL_NODEID;
    bvector <bvector <MTGNodeIdPair> > allEdges;    // memory is cheaper than logic
    // In the grid ..
    // Rungs are horizontal.
    // Rails are vertical.
    if (nu < 1 || nv < 1)
        return MTG_NULL_NODEID;
    int numDistinctRail = uPeriodic ? nu : nu + 1;
    for (size_t i = 0; i < (size_t)numDistinctRail; i++)
        {
        MTGNodeIdPair edgeA, edgeB;
        edgeA.Init ();
        edgeB.Init ();
        allEdges.push_back (bvector <MTGNodeIdPair> ());
        bvector<MTGNodeIdPair> &railEdges = allEdges.back ();
        // Create edges " up a rail".
        // edgeB.nodeId[0] == lower left node
        // edgeB.nodeId[1] == upper right node.
        for (size_t j = 0; j < (size_t)nv; j++, edgeA = edgeB)
            {
            jmdlMTGGraph_createEdge (pGraph, &edgeB.nodeId[0], &edgeB.nodeId[1]);
            if (j > 0)
                jmdlMTGGraph_vertexTwist (pGraph, edgeA.nodeId[1], edgeB.nodeId[0]);
            railEdges.push_back (edgeB);
            jmdlMTGGraph_setLabel (pGraph, edgeB.nodeId[0], vertexIdOffset, (int)(j * nuVertexPerRow + i));
            jmdlMTGGraph_setLabel (pGraph, edgeB.nodeId[1], vertexIdOffset, (int)(((j+1) % numDistinctRail) + i));
            }

        if (vPeriodic)
            jmdlMTGGraph_vertexTwist (pGraph, railEdges.front ().nodeId[0], railEdges.back().nodeId[1]);
        }

    // Add a lower rung in each tile.
    // In non-periodic v, top rung is extra step.
    MTGNodeIdPair edgeA, edgeB;
    for (size_t i0 = 0; i0 < (size_t)nu; i0++)
        {
        size_t i1 = (i0 + 1) % numDistinctRail;    // last box gets right side from wrap.
        for (size_t j = 0; j < (size_t)nv; j++)
            {
            edgeA = allEdges[i0][j];  // nodeId[1] is UPPER node at left edge of tile
            edgeB = allEdges[i1][j];  // nodeId[0] is LOWER edge at right edge of tile
            joinEdgesWithVertexCopy (pGraph, vertexIdOffset, edgeA, edgeB, false);
            }
        if (!vPeriodic)
            {
            // edgeA, edgeB are left and right of the top tile.  Add top edge.
            // (A,B reversal is right !!)
            joinEdgesWithVertexCopy (pGraph, vertexIdOffset, edgeA, edgeB, true);
            }
        }
    MTGNodeId lowerLeftNodeId = jmdlMTGGraph_getFSucc (pGraph, allEdges.front ().front ().nodeId[1]);
    MTGNodeId upperRightNodeId = jmdlMTGGraph_getFSucc (pGraph, allEdges.back ().back ().nodeId[0]);
    MTGMask maskB = MTG_BOUNDARY_MASK;
    MTGMask maskBX = MTG_BOUNDARY_MASK | MTG_EXTERIOR_MASK;
    outputNodeId = lowerLeftNodeId;
    if (uPeriodic && vPeriodic)
        {
        // closed torus, no boundaries
        }
    else if (uPeriodic)
        {
        SetEdgeMasksAroundFace (pGraph,
                jmdlMTGGraph_getVPred (pGraph, lowerLeftNodeId), maskBX, maskB);
        SetEdgeMasksAroundFace (pGraph,
                jmdlMTGGraph_getVPred (pGraph, upperRightNodeId), maskBX, maskB);
        }
    else if (vPeriodic)
        {
        SetEdgeMasksAroundFace (pGraph,
                jmdlMTGGraph_getVSucc (pGraph, lowerLeftNodeId), maskBX, maskB);
        SetEdgeMasksAroundFace (pGraph,
                jmdlMTGGraph_getVSucc (pGraph, upperRightNodeId), maskBX, maskB);
        }
    else
        {
        SetEdgeMasksAroundFace (pGraph,
                jmdlMTGGraph_getVSucc (pGraph, lowerLeftNodeId), maskBX, maskB);
        }
    assert (allEdges.size () == numDistinctRail);
    assert (allEdges[0].size () == nv);
    return outputNodeId;
    }
#ifdef abc
/*---------------------------------------------------------------------------------**//**
* Build the connectivity of a basic triangle, viz
*
*   2
*    B
*    |\
*    |1\
*    |  \
*    |   \
*    |5  3\
*    A-----C
*   0       4
*
*
* @bsimethod                                                    EarlinLutz      12/98
+---------------+---------------+---------------+---------------+---------------+------*/
static         void                    jmdlMTGGraph_buildTriangleConnectivity
(
MTG_Node        *pNode0,                // => node structure address per diagram
MTG_Node        *pNode1,                // => node structure address per diagram
MTG_Node        *pNode2,                // => node structure address per diagram
MTG_Node        *pNode3,                // => node structure address per diagram
MTG_Node        *pNode4,                // => node structure address per diagram
MTG_Node        *pNode5,                // => node structure address per diagram
int             nodeId0,                // => nodeId per diagram
int             nodeId1,                // => nodeId per diagram
int             nodeId2,                // => nodeId per diagram
int             nodeId3,                // => nodeId per diagram
int             nodeId4,                // => nodeId per diagram
int             nodeId5                 // => nodeId per diagram
)
    {
    /* Outer face loop ... */
    pNode0->fSucc = nodeId2;
    pNode2->fSucc = nodeId4;
    pNode4->fSucc = nodeId0;

    /* Outer face loop ... */
    pNode1->fSucc = nodeId5;
    pNode3->fSucc = nodeId1;
    pNode5->fSucc = nodeId3;

    /* Vertex 0 */
    pNode0->vSucc = nodeId5;
    pNode5->vSucc = nodeId0;

    /* Vertex 1 */
    pNode2->vSucc = nodeId1;
    pNode1->vSucc = nodeId2;

    /* Vertex 2 */
    pNode3->vSucc = nodeId4;
    pNode4->vSucc = nodeId3;

    pNode0->mask = MTG_NULL_MASK;
    pNode1->mask = MTG_NULL_MASK;
    pNode2->mask = MTG_NULL_MASK;
    pNode3->mask = MTG_NULL_MASK;
    pNode4->mask = MTG_NULL_MASK;
    pNode5->mask = MTG_NULL_MASK;
    }

/*---------------------------------------------------------------------------------**//**
* Store label values at specified offset into nodes of a basic triangle.
*
*
* @bsimethod                                                    EarlinLutz      12/98
+---------------+---------------+---------------+---------------+---------------+------*/
static         void                    jmdlMTGGraph_applyTriangleVertexLabels
(
MTG_Node        *pNode0,                // => node structure address per diagram
MTG_Node        *pNode1,                // => node structure address per diagram
MTG_Node        *pNode2,                // => node structure address per diagram
MTG_Node        *pNode3,                // => node structure address per diagram
MTG_Node        *pNode4,                // => node structure address per diagram
MTG_Node        *pNode5,                // => node structure address per diagram
int             labelOffset,            // => label offset
int             labelValueA,            // => label value for vertex 0
int             labelValueB,            // => label value for vertex 1
int             labelValueC             // => label value for vertex 2
)
    {
    int         *pLabels;

    pLabels = (int *)(pNode0 + 1);
    pLabels[labelOffset] = labelValueA;

    pLabels = (int *)(pNode1 + 1);
    pLabels[labelOffset] = labelValueB;

    pLabels = (int *)(pNode2 + 1);
    pLabels[labelOffset] = labelValueB;

    pLabels = (int *)(pNode3 + 1);
    pLabels[labelOffset] = labelValueC;

    pLabels = (int *)(pNode4 + 1);
    pLabels[labelOffset] = labelValueC;

    pLabels = (int *)(pNode5 + 1);
    pLabels[labelOffset] = labelValueA;
    }

/*---------------------------------------------------------------------------------**//**
* Twist the rightmost node of triangle 0 with the lower left node of triangle 1.
*
*
* @bsimethod                                                    EarlinLutz      12/98
+---------------+---------------+---------------+---------------+---------------+------*/
static         void            jmdlMTGGraph_joinTrianglesWithinRow
(
MTGGraph       *pGraph,
MTGNodeId      *pNodeIdArray,           // => array of node ids
int             nodeIndex0,             // => index to lower left of first triangle
int             nodeIndex1              // => index to lower left node id of second triangle.
)
    {
    jmdlMTGGraph_vertexTwist
                    (
                    pGraph,
                    pNodeIdArray [nodeIndex0 + 4],
                    pNodeIdArray [nodeIndex1]
                    );
    }

/*---------------------------------------------------------------------------------**//**
* Twist the upper nodes of one row of triangles into the lower nodes of the next row.
*
* Upper row:
*   X       X       X       X
*   |\      |\      |\      |\
*   | \     | \     | \     | \
*   |  \    |  \    |  \    |  \
*   |   \   |   \   |   \   |   \
*   |    \  |    \  |    \  |    \
*   |     \ |     \ |     \ |     \
*  K|      \|      \|      \|      \
*   X-------X-------X-------X-------X
*        K+4      K+10  ....
* (K = pNodeIdArray[row1NodeIndex])
*
* Lower Row:
*   J+2    J+8
*   X       X       X       X
*   |\      |\      |\      |\
*   | \     | \     | \     | \
*   |  \    |  \    |  \    |  \
*   |   \   |   \   |   \   |   \
*   |    \  |    \  |    \  |    \
*   |     \ |     \ |     \ |     \
*  J|      \|      \|      \|      \
*   X-------X-------X-------X-------X
*
* (J = pNodeIdArray[row0NodeIndex])
*
* @bsimethod                                                    EarlinLutz      12/98
+---------------+---------------+---------------+---------------+---------------+------*/
static         void            jmdlMTGGraph_joinTriangleRows
(
MTGGraph       *pGraph,
MTG_Node        **ppNodeArray,      // => array of pointers to available nodes
MTGNodeId      *pNodeIdArray,      // => array of node ids
int             row0NodeIndex,      // => reference nodeId on first row
MTGNodeId      row1NodeIndex,      // => reference nodeId on second row
int             numTri1             // => number of triangles in second row.
)
    {
    int tri;
    int j, k;
    if (numTri1 <= 0)
        return;

    jmdlMTGGraph_vertexTwist
                    (
                    pGraph,
                    pNodeIdArray[row0NodeIndex + 2],
                    pNodeIdArray[row1NodeIndex]
                    );

    /* After first triangle, the indices of the nodeId's to twist are at steps of 6 */
    j = row0NodeIndex + 8;
    k = row1NodeIndex + 4;
    for (tri = 0; tri < numTri1; tri++)
        {
        jmdlMTGGraph_vertexTwist
                (
                pGraph,
                pNodeIdArray[j],
                pNodeIdArray[k]
                );
        j += 6;
        k += 6;
        }
    }


/*---------------------------------------------------------------------------------**//**
* Build a complete row of triangles, joined toe to heel
*
*
* @bsimethod                                                    EarlinLutz      12/98
+---------------+---------------+---------------+---------------+---------------+------*/
static         MTGNodeId               jmdlMTGGraph_buildTriangleRow
(
MTGGraph       *pGraph,
MTG_Node        **ppNodeArray,          // => array of pointers to available nodes
MTGNodeId      *pNodeIdArray,           // => array of node ids.
int             nodeIndex0,             // => index of first node within arrays.
int             numTriangle,            // => number of triangles to construct
int             vertex0Id,              // => first vertex id on bottom row
int             vertex1Id,              // => first vertex id on top row
int             vertexLabelOffset       // => offset for placing vertex ids
)
    {
    int numNodePerTriangle = 6;
    int i0, i1;
    int tri;

    /* Make numTri triangles, each using numNodePerTriangle nodes out of the big array provided */
    /* i1 is the index of the base node in the latest triangle *.
    /* i0 is the index of the base node of the previous triangle (when tri > 0) */

    for (tri = 0, i1 = nodeIndex0;
        tri < numTriangle;
        tri++
        )
        {
        /* Make a new triangle .. */
        jmdlMTGGraph_buildTriangleConnectivity
                    (
                    ppNodeArray [i1],
                    ppNodeArray [i1 + 1],
                    ppNodeArray [i1 + 2],
                    ppNodeArray [i1 + 3],
                    ppNodeArray [i1 + 4],
                    ppNodeArray [i1 + 5],

                    pNodeIdArray[i1],
                    pNodeIdArray[i1 + 1],
                    pNodeIdArray[i1 + 2],
                    pNodeIdArray[i1 + 3],
                    pNodeIdArray[i1 + 4],
                    pNodeIdArray[i1 + 5]
                    );
        if (vertexLabelOffset >= 0)
            jmdlMTGGraph_applyTriangleVertexLabels
                    (
                    ppNodeArray [i1],
                    ppNodeArray [i1 + 1],
                    ppNodeArray [i1 + 2],
                    ppNodeArray [i1 + 3],
                    ppNodeArray [i1 + 4],
                    ppNodeArray [i1 + 5],

                    vertexLabelOffset,
                    vertex0Id + tri,
                    vertex1Id + tri,
                    vertex0Id + tri + 1
                    );

        /* Connect it to the previous one */
        if (tri > 0)
            {
            jmdlMTGGraph_joinTrianglesWithinRow (pGraph, pNodeIdArray, i0, i1);
            }
        i0 = i1;
        i1 += numNodePerTriangle;
        }
    return i1;
    }

/**
* Apply masks to triangular grid.
* ONLY THE FOLLOWING MASKS ARE APPLIED:
*       constU == vertical edge
*       constV == horizontal edge
*       boundaryInterior == inside of boundary
*       boundaryExterior == outside of boundary
*       cornerVertex == any node at 3 main vertices
*       boundaryVertex == any node at any boundary vertex (including 3 main corners)
* THAT IS, WE DO NOT APPLY THE FOLLOWING MASKS NAMED IN THE MASK STRUCTURE:
*   interior vertex
*   interior edge
*   u seam
*   v seam
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static         void            jmdlMTGGraph_applyTriangularGridMasks
(
MTGGraph *      pGraph,                 // <=> Containing graph
MTG_Node        **ppNodeArray,          // => array of pointers to available nodes
MTGNodeId      *pNodeIdArray,           // => array of node ids.
int             numEdge,                // => number of edges along each direction
MTG_RectangularGridMasks *pMasks        // => masks
                                        //    Vertices are labeled in row-major order from here.
)
    {
    MTGNodeId baseNodeId = pNodeIdArray[0];
    MTGNodeId mateNodeId;
    int i;
    int totalNodes = 3 * numEdge * (numEdge + 1);
    int counter = 0;


    MTGMask         constUMask           = pMasks ? pMasks->constUMask            : MTG_CONSTU_MASK;
    MTGMask         constVMask           = pMasks ? pMasks->constVMask            : MTG_CONSTV_MASK;
    MTGMask         boundaryInteriorMask = pMasks ? pMasks->boundaryInteriorMask  : MTG_BOUNDARY_MASK;
    MTGMask         boundaryExteriorMask = pMasks ? pMasks->boundaryExteriorMask  : MTG_BOUNDARY_MASK | MTG_EXTERIOR_MASK;
    MTGMask         cornerVertexMask     = pMasks ? pMasks->cornerVertexMask      : MTG_PRIMARY_VERTEX_MASK;
    MTGMask         boundaryVertexMask   = pMasks ? pMasks->boundaryVertexMask    : MTG_BOUNDARY_VERTEX_MASK;


    /* Walk around the outer loop marking exterior, primary and boundary properties. */
    /* The first node is a primary vertex.   Depend on edge counts to tell when we have
        reached the other 2 primary vertices.
    */
    MTGARRAY_FACE_LOOP (currNodeId, pGraph, baseNodeId)
        {
        jmdlMTGGraph_setMask (pGraph, currNodeId, boundaryExteriorMask);

        mateNodeId      = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);

        jmdlMTGGraph_setMask (pGraph, mateNodeId, boundaryInteriorMask);

        if (boundaryVertexMask != MTG_NULL_NODEID)
            jmdlMTGGraph_setMaskAroundVertex (pGraph, currNodeId, boundaryVertexMask);

        if (counter == 0)
            {
            jmdlMTGGraph_setMaskAroundVertex (pGraph, currNodeId, cornerVertexMask);
            counter = numEdge - 1;
            }
        else
            {
            counter--;
            }
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, baseNodeId)

    if (constVMask != MTG_NULL_MASK)
        {
        /* Within each block of 6 nodes for a triangle, nodes 4 and 5 are horizontal */
        for (i = 0; i < totalNodes; i += 6)
            {
            ppNodeArray[i + 4]->mask |= constVMask;
            ppNodeArray[i + 5]->mask |= constVMask;
            }
        }

    if (constUMask != MTG_NULL_MASK)
        {
        /* Within each block of 6 nodes for a triangle, nodes 0 and 1 are vertical */
        for (i = 0; i < totalNodes; i += 6)
            {
            ppNodeArray[i]->mask |= constUMask;
            ppNodeArray[i + 1]->mask |= constUMask;
            }
        }

    }


/**
* Build a rectangular face grid.
* @param pGraph    <=> Containing graph
* @param nu => number of edges along constant u direction
* @param nv => number of edges along constant v direction
* @param pMasks         => masks
* @param normalMode     => indicates which labels are to be applied.
* @param vertexIdOffset => offset for vertex id labels.
* @param vertex00Id     => label for lower left vertex.  -1 if no labels needed.
*                          Vertices are numbered along u direction first.
* @return MTGNodeId of outside node at 00 point.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_buildTriangularGrid
(
MTGGraph *      pGraph,                 // <=> Containing graph
int             numEdge,                // => number of edges along each direction
MTG_RectangularGridMasks *pMasks,       // => masks
int             vertexLabelOffset,      // => offset for vertex labels.
int             vertex00Id              // => label for lower left vertex.  -1 if no labels needed.
                                        //    Vertices are labeled in row-major order from here.
)
    {

    MTGNodeId *pNodeId = NULL;          // Id's of new nodes, in local index order.
    MTG_Node   **ppNode = NULL;        // Pointers to new nodes, in local index order.
    MTGNodeId  baseNodeId = MTG_NULL_NODEID;
    int         currBase, oldBase, nextBase;
    int         numEdgeCurrRow, numVertexCurrRow;
    int         currVertex0;

    int totalNodes = 3 * numEdge * (numEdge + 1);

    if (jmdlMTGGraph_grabNodeArrays (pGraph, &ppNode, &pNodeId, totalNodes))
        {
        int row;
        currBase = 0;
        currVertex0 = 0;
        for (row = 0; row < numEdge; row++)
            {
            numEdgeCurrRow = numEdge - row;
            numVertexCurrRow = numEdgeCurrRow + 1;
            nextBase = jmdlMTGGraph_buildTriangleRow
                        (
                        pGraph,
                        ppNode,
                        pNodeId,
                        currBase,
                        numEdgeCurrRow,
                        currVertex0,
                        currVertex0 + numVertexCurrRow,
                        vertexLabelOffset
                        );
            if (row > 0)
                {
                jmdlMTGGraph_joinTriangleRows
                        (
                        pGraph,
                        ppNode,
                        pNodeId,
                        oldBase,
                        currBase,
                        numEdgeCurrRow
                        );
                }
            currVertex0 += numVertexCurrRow;
            oldBase  = currBase;
            currBase = nextBase;
            }
        baseNodeId = pNodeId[0];
        jmdlMTGGraph_applyTriangularGridMasks (pGraph, ppNode, pNodeId, numEdge, pMasks);
        jmdlMTGGraph_dropNodeArrays (pGraph, &ppNode, &pNodeId);
        }
    jmdlMTGGraph_verifyGraph (pGraph);
    return baseNodeId;
    }



/**
* Build a rectangular face grid.
* @param    pGraph          <=> Containing graph
* @param numRay             => number of rays in fan.
* @param periodic           => true if seam at left/right
* @param pMasks             => masks
* @param normalMode         => indicates which labels are to be applied.
* @param vertexIdOffset     => offset for vertex id labels.
* @param vertex00Id         => label for lower left vertex.  -1 if no labels needed.
*                               Vertices are labeled in row-major order from here.
* @param sectorIdOffset     => offset for sector id labels.  -1 if no labels needed.
* @param sector00Id         => label for lower left vertex sector on main grid.
* @see
* @return MTGNodeId
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_buildPlanarFan
(
MTGGraph *      pGraph,
int             numRay,
bool            periodic,
MTG_RectangularGridMasks *pMasks,
MTGFacets_NormalMode normalMode,
int             vertexIdOffset,
int             vertex00Id,
int             sectorIdOffset,
int             sector00Id
)
    {
    MTGNodeId outputNode = MTG_NULL_NODEID;


    int totalNodes;

    MTGMask         maskExteriorCircle  = MTG_BOUNDARY_MASK | MTG_EXTERIOR_MASK;
    MTGMask         maskInteriorCircle  = MTG_BOUNDARY_MASK;
    MTGMask         maskInteriorApex     = MTG_NULL_MASK;
    MTGMask         maskExteriorApex    = MTG_PRIMARY_VERTEX_MASK | MTG_BOUNDARY_MASK | MTG_EXTERIOR_MASK;
    MTGMask         maskInteriorUp      = MTG_NULL_MASK | MTG_BOUNDARY_MASK;
    MTGMask         maskExteriorUp      = MTG_BOUNDARY_MASK | MTG_EXTERIOR_MASK;
    MTGMask         maskArcVertex       = MTG_PRIMARY_VERTEX_MASK;


    if (periodic)
        {
        totalNodes = 4 * numRay;
        }
    else
        {
        totalNodes = 4 * numRay - 2;
        }

    int j, k;

    MTGNodeId *pNodeId = NULL;          // Id's of new nodes, in local index order.
    MTG_Node   **ppNode = NULL;        // Pointers to new nodes, in local index order.
    MTG_Node    *pNode;
    int         *pLabels;
    bool        labelVertices = (vertexIdOffset >= 0 && vertexIdOffset < pGraph->numLabelPerNode);

    // Local index identities:
    //
    //                                              |L-1
    //                   ---- ray numRay ----  L- 2 |
    //          *-----------------------------------+
    //                                              |L - 3
    //                                              |
    //          *-----------------------------------+
    //                                              |
    //                                              |
    //          *-----------------------------------+
    //                                              |
    //                                              |
    //           node 2k                            |
    //          *-----------------------------------+
    //                  ---- ray k ----   node 2k+1 | K+3
    //                                              |
    //           node2                          K+2 |
    //          *-----------------------------------+
    //                   ---- ray 1 ----  node 3    | K + 1
    //                                              |
    //           node 0                           K |
    //          *-----------------------------------+
    //                   ---- ray 0 ----  node 1
    // where K = 2 * numRay
    //       L = 4 * numRay
    // NOTE: the edge containing nodes L-1 and L-2 is omitted from non-periodic mesh

    //
    // The exterior loop has consecutively numbered nodes, with indices increasing in a CW (!!) loop
    // starting at the 00 corner.  The corner nodes have local indices corner00, corner10, corner11, corner01.
    //
    if (jmdlMTGGraph_grabNodeArrays (pGraph, &ppNode, &pNodeId, totalNodes))
        {
        int nodeK = 2 * numRay;
        int nodeL = 4 * numRay;

        // Clear all the masks:
        int dummyLabels = true;
        for (k = 0; k < totalNodes; k++)
            {
            pNode = ppNode[k];
            pNode->fSucc = -1;
            pNode->vSucc = -1;
            pNode->mask = 0;
            if (dummyLabels)
                {
                pLabels = (int*)(pNode + 1);
                pLabels[vertexIdOffset] = -2;
                }
            }


#define SETNODE(_k, _fSucc, _vSucc, _vertexId, _mask)  \
            {                                   \
            pNode = ppNode[_k];                 \
            pNode->vSucc = _vSucc;              \
            pNode->fSucc = _fSucc;              \
            pNode->mask  = _mask;               \
            if (labelVertices)                  \
                {                               \
                pLabels = (int *) (pNode + 1);  \
                pLabels[vertexIdOffset] = _vertexId;    \
                }                               \
            }


        // Center vertex loop
        for (j = 0; j < numRay - 1; j++)
            {
            k = 2 * j;
            SETNODE (k, nodeK + k, k + 2, vertex00Id, maskInteriorApex);
            }

        // interior circle edges
        for (j = 0; j < numRay - 1; j++)
            {
            k = nodeK + 2 * j;
            SETNODE (k, 2 * j + 3, 2 * j + 1, vertex00Id + j + 1, maskInteriorCircle);
            }

        // base of inward rays
        for (j = 1; j < numRay; j++)
            {
            k = 1 + 2 * j;
            SETNODE (k, 2 * j - 2,nodeK + k - 2, vertex00Id + j + 1, maskInteriorUp);
            }

        // exterior cricle edges
        for (j = 0; j < numRay - 1; j++)
            {
            k = nodeK + 2 * j + 1;
            SETNODE (k, k -2, k + 1, vertex00Id + j + 2, maskExteriorCircle);
            }

        if (periodic)
            {
            // closure triangle, at apex
            SETNODE (nodeK - 2, nodeL - 2, 0, vertex00Id, maskInteriorApex);
            // outside closure edge
            SETNODE (nodeL - 1, nodeL -3, nodeK, vertex00Id + 1, maskExteriorCircle);
            // inside closure edge
            SETNODE (nodeL - 2, 1, nodeK - 1, vertex00Id + numRay, maskInteriorCircle);
            // upstroke of closure face
            SETNODE (        1, nodeK - 2, nodeL - 1, vertex00Id + 1, maskInteriorUp);
            // one more outside fixup
            SETNODE (nodeK + 1, nodeL -1, nodeK + 2, 2, maskExteriorCircle);
            }
        else
            {
            SETNODE (nodeL - 3, nodeL - 5, nodeK - 1, vertex00Id + numRay, maskExteriorCircle);
            SETNODE (1,         nodeK - 2, nodeK    , 1, maskExteriorUp);
            SETNODE (nodeK - 2, nodeL - 3, 0, vertex00Id, maskExteriorApex);
            // one more outside fixup
            SETNODE (nodeK + 1,         1, nodeK + 2, 2, maskExteriorCircle);
            jmdlMTGGraph_setMaskAroundVertex (pGraph, 1, maskArcVertex);
            jmdlMTGGraph_setMaskAroundVertex (pGraph, nodeK - 1, maskArcVertex);
            }
        outputNode = 0;
        }
    jmdlMTGGraph_verifyGraph (pGraph);

    return outputNode;
    }


/**
* Build a cube topology.  Uses standard numbering (see Earlin's doc
* set for pictures.)
* @param    pGraph <=> Containing graph
* @param nodeMask => mask to apply to all nodes
* @param normalMode => Indicates how normal data is to be constructed.
*                       For vertexOnly, vertex indices 0..7 are assigned
*                       For normalPerVertex, vertex indices 0..23 are assigned
*                           exactly matching the node ids.
* @param vertex0Id          => index to use for the zero'th vertex of 6
* @param vertexIdOffset     => offset for vertex id labels.
* @param vertex00Id         => label for first vertex
* @param sectorIdOffset     => offset for sector id labels.  -1 if no labels needed.
* @param sector00Id         => label for first sector
* @return MTGNodeId
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_buildLabeledCubeTopology
(
MTGGraph *      pGraph,
MTGMask nodeMask,
MTGFacets_NormalMode normalMode,
int             vertex0Id,
int             vertexIdOffset,
int             vertex00Id,
int             sectorIdOffset,
int             sector00Id
)
    {
    // Cube has 8 vertices, 6 faces, 24 vu nodes.
    // The face successors of the various nodes are:
    static int fs[24] =
        {
         1,  2,  3,  0,         // face 0
         5,  6,  7,  4,         // face 1
         9, 10, 11,  8,         // face 2
        13, 14, 15, 12,         // face 3
        17, 18, 19, 16,         // face 4
        21, 22, 23, 20          // face 5
        };
    // ... vertex successors
    static int vs[24] =
        {
         8, 19, 15, 20,
        17, 10, 22, 13,
        16,  3, 23,  4,
         1, 18,  6, 21,
         0, 11,  7, 12,
         9,  2, 14,  5
        };
    // .... vertex ids (0..8)
    static int vertId[24] =
        {
        0, 2, 3, 1,
        4, 5, 7, 6,
        0, 1, 5, 4,
        2, 6, 7, 3,
        0, 4, 6, 2,
        1, 3, 7, 5
        };

    MTGNodeId *pNodeId = NULL;          // Id's of new nodes, in local index order.
    MTG_Node   **ppNode = NULL;        // Pointers to new nodes, in local index order.
    MTGNodeId  returnNode = MTG_NULL_NODEID;
    MTG_Node    *pNode;
    int         totalNodes = 24;
    int         *pLabels;
    int         k;


    if (jmdlMTGGraph_grabNodeArrays (pGraph, &ppNode, &pNodeId, totalNodes))
        {
        for (k = 0; k < totalNodes; k++)
            {
            pNode = ppNode[k];
            pNode->fSucc = pNodeId[fs[k]];
            pNode->vSucc = pNodeId[vs[k]];
            pNode->mask = nodeMask;
            }

        if (normalMode == MTG_Facets_VertexOnly)
            {
            for (k = 0; k < totalNodes; k++)
                {
                pNode = ppNode[k];
                pLabels = (int*)(pNode + 1);
                pLabels[vertexIdOffset] = vertex0Id + vertId[k];
                }
            }
        else if (normalMode == MTG_Facets_NormalPerVertex)
            {
            for (k = 0; k < totalNodes; k++)
                {
                pNode = ppNode[k];
                pLabels = (int*)(pNode + 1);
                pLabels[vertexIdOffset] = vertex0Id + k;
                }
            }


        returnNode = pNodeId[0];
        }

    return returnNode;
    }
#endif
END_BENTLEY_GEOMETRY_NAMESPACE

