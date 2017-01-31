/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/mtg/decimatexyz.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
#include "../memory/ptrcache.h"
#include "../memory/jmdl_iarray.fdf"
#include "../memory/jmdl_dpnt3.fdf"
#include <Mtg/decimatexyz.fdf>

#ifdef USE_CACHE
END_BENTLEY_GEOMETRY_NAMESPACE
#include "../bsiinc/Geom/embeddedarraycachemanager.fdf"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#endif

/*---------------------------------------------------------------------------------**//**
* Set a mask at each node in an array.
* @bsimethod                                                      EarlinLutz      02/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlMTGGraph_setMaskInArray
(
MTGGraph*           pGraph,
EmbeddedIntArray*   pEdgeArray,
MTGMask             mask
)
    {
    MTGNodeId       nodeId;
    int             i;

    for (i = 0; jmdlEmbeddedIntArray_getInt (pEdgeArray, &nodeId, i++);)
        {
        jmdlMTGGraph_setMask (pGraph, nodeId, mask);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Check that a node and its two face successors form a strictly interior triangle (i.e. third
* face successor step comes back to the start).  Return the 2 face sucessor nodes.
* @param node1Id => face successor
* @param node2Id => second face successor
* @param node0Id => start node
* @param abortMask => forces a false return if this mask is found in the neighborhood.
* @bsimethod                                                      EarlinLutz      02/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGFacets_getInteriorTriangleNodeIds
(
MTGGraph*       pGraph,
MTGNodeId*      pNode1Id,
MTGNodeId*      pNode2Id,
MTGNodeId       node0Id,
MTGMask         abortMask
)
{
    MTGNodeId       node1Id, node2Id, node3Id;

    node1Id = jmdlMTGGraph_getFSucc (pGraph, node0Id);
    node2Id = jmdlMTGGraph_getFSucc (pGraph, node1Id);
    node3Id = jmdlMTGGraph_getFSucc (pGraph, node2Id);

    if (
        node3Id != node0Id
        || jmdlMTGGraph_getMask (pGraph, node0Id, abortMask)
        || jmdlMTGGraph_getMask (pGraph, node1Id, abortMask)
        || jmdlMTGGraph_getMask (pGraph, node2Id, abortMask)
        )
        {
        *pNode1Id = *pNode2Id = MTG_NULL_NODEID;
        return false;
        }
    else
        {
        *pNode1Id = node1Id;
        *pNode2Id = node2Id;
        return true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                      EarlinLutz      02/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGFacets_crossProduct3Nodes
(
MTGFacets*      pFacets,
DPoint3d*       pCross,
MTGNodeId       node0Id,
MTGNodeId       node1Id,
MTGNodeId       node2Id
)
{
    DPoint3d        point0, point1, point2;
    bool            boolstat = false;
    if (jmdlMTGFacets_getNodeCoordinates (pFacets, &point0, node0Id)
        && jmdlMTGFacets_getNodeCoordinates (pFacets, &point1, node1Id)
        && jmdlMTGFacets_getNodeCoordinates (pFacets, &point2, node2Id)
        )
        {
        if (pCross)
            bsiDPoint3d_crossProduct3DPoint3d (pCross, &point0, &point1, &point2);
        boolstat = true;
        }
    return boolstat;
    }
/*---------------------------------------------------------------------------------**//**
* Search the neighborhood of an edge in a triangulated mesh.
* @param pEdgeNodeIdArray <= edge nodes on the boundary of the neighborhood.
* @param pFaceNodeIdArray <= face nodes, in sets of 3, for all incident faces.
* @param abortMask <= any occurrance of this mask aborts the search.
* @bsimethod                                                      EarlinLutz      02/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGFacets_searchTriangulatedEdgeNeighborhood
(
MTGFacets*          pFacets,
EmbeddedIntArray*   pEdgeNodeIdArray,
EmbeddedIntArray*   pFaceNodeIdArray,
DPoint3d*           pNearNormal,
DPoint3d*           pFarNormal,
MTGNodeId           edgeNodeId,
MTGNodeId           abortMask
)
{
    MTGNodeId       mateNodeId, edgeEndNodeId, mateEndNodeId;
    MTGNodeId       node0Id, node1Id, node2Id;

    MTGGraph       *pGraph = jmdlMTGFacets_getGraph (pFacets);

    edgeEndNodeId = jmdlMTGGraph_getFSucc (pGraph, edgeNodeId);
    mateNodeId = jmdlMTGGraph_getVSucc (pGraph, edgeEndNodeId);
    mateEndNodeId = jmdlMTGGraph_getFSucc (pGraph, mateNodeId);

    /* Sweep around the vertex at the start of the edge */
    for (node0Id = jmdlMTGGraph_getVSucc (pGraph, edgeNodeId);
         node0Id != mateEndNodeId;
         node0Id = jmdlMTGGraph_getVSucc (pGraph, node0Id))
        {
        if (!jmdlMTGFacets_getInteriorTriangleNodeIds (pGraph,
                               &node1Id, &node2Id, node0Id, abortMask))
            return false;
        if (pEdgeNodeIdArray)
            jmdlEmbeddedIntArray_addInt (pEdgeNodeIdArray, node1Id);
        if (pFaceNodeIdArray)
            jmdlEmbeddedIntArray_add3Int (pFaceNodeIdArray, node0Id, node1Id, node2Id);
        }

    /* Far side of internal edge */
    if (!jmdlMTGFacets_getInteriorTriangleNodeIds (pGraph, &node1Id, &node2Id, node0Id, abortMask))
     return false;

    if (pFaceNodeIdArray)
        jmdlEmbeddedIntArray_add3Int (pFaceNodeIdArray, node0Id, node1Id, node2Id);

    if (pFarNormal)
        jmdlMTGFacets_crossProduct3Nodes (pFacets, pFarNormal, node0Id, node1Id, node2Id);

    /* Sweep around the vertex at the end of the edge */
    for (node0Id = jmdlMTGGraph_getVSucc (pGraph, mateNodeId);
         node0Id != edgeEndNodeId;
         node0Id = jmdlMTGGraph_getVSucc (pGraph, node0Id))
        {
        if (!jmdlMTGFacets_getInteriorTriangleNodeIds (pGraph,
                               &node1Id, &node2Id, node0Id, abortMask))
            return false;
        if (pEdgeNodeIdArray)
            jmdlEmbeddedIntArray_addInt (pEdgeNodeIdArray, node1Id);
        if (pFaceNodeIdArray)
            jmdlEmbeddedIntArray_add3Int (pFaceNodeIdArray, node0Id, node1Id, node2Id);
        }

    /* near side of internal edge */
    if (!jmdlMTGFacets_getInteriorTriangleNodeIds (pGraph, &node1Id, &node2Id, node0Id, abortMask))
        return false;

    if (pFaceNodeIdArray)
        jmdlEmbeddedIntArray_add3Int (pFaceNodeIdArray, node0Id, node1Id, node2Id);

    if (pFarNormal)
        jmdlMTGFacets_crossProduct3Nodes (pFacets, pNearNormal, node0Id, node1Id, node2Id);

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* return true if there are no normal reversals due to collapsing this edge to its start point.
*
* @bsimethod                                                      EarlinLutz      02/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool jmdlMTGFacets_edgeCollapseNormalsOK
(
MTGFacets* pFacets,
MTGNodeId nodeIdA0
)
    {
    DVec3d normalA, normalB, normalC;
    // Triangle to left ...
    MTGGraph       *graph = jmdlMTGFacets_getGraph (pFacets);
    MTGNodeId nodeIdA1 = graph->FSucc (nodeIdA0);
    MTGNodeId nodeIdA2 = graph->FSucc (nodeIdA1);
    MTGNodeId nodeIdA3 = graph->FSucc (nodeIdA2);
    if (nodeIdA3 != nodeIdA0)
        return false;
    // triangle to right ...
    MTGNodeId nodeIdB0 = graph->VSucc (nodeIdA1);
    MTGNodeId nodeIdB1 = graph->FSucc (nodeIdB0);
    MTGNodeId nodeIdB2 = graph->FSucc (nodeIdB1);
    MTGNodeId nodeIdB3 = graph->FSucc (nodeIdB2);
    if (nodeIdB3 != nodeIdB0)
        return false;


    jmdlMTGFacets_crossProduct3Nodes (pFacets, &normalA, nodeIdA0, nodeIdA1, nodeIdA2);
    jmdlMTGFacets_crossProduct3Nodes (pFacets, &normalB, nodeIdB0, nodeIdB1, nodeIdB2);

    MTGNodeId seedNodes[2] = {nodeIdA0, nodeIdB0};
    // WE EXPECT (demand) that all triangles from the new center point will have normals compatible with both normalA and normalB.
    for (size_t i = 0; i < 2; i++)
        {
        MTGARRAY_VERTEX_LOOP (nodeIdC0, graph,  seedNodes[i])
            {
            // Far edge of this triangle ...
            MTGNodeId nodeIdC1 = graph->FSucc (nodeIdC0);
            MTGNodeId nodeIdC2 = graph->FSucc (nodeIdC1);

            if (  nodeIdC1 != nodeIdA1
               && nodeIdC1 != nodeIdA2
               && nodeIdC1 != nodeIdB1
               && nodeIdC1 != nodeIdB2
               )
               {
               jmdlMTGFacets_crossProduct3Nodes (pFacets, &normalC, nodeIdA0, nodeIdC1, nodeIdC2);
               if (normalC.DotProduct (normalA) <= 0.0)
                  return false;
               if (normalC.DotProduct (normalB) <= 0.0)
                  return false;
               }
            }
        MTGARRAY_END_VERTEX_LOOP (nodeIdC0, graph,  seedNodes[i])
        }
    return true;  
    }
/*---------------------------------------------------------------------------------**//**
* Evaluate the effect of collapsing the far end of the edge at edgeNodeId back to
* its start node.  Optionally abort if a specified edge is found in the neighborhood.
*
* @param pDestFacets <= optional facet set.  If not null, the cone facets are added
*     this facet set.
* @param pDist <= length change due to edge collapse.
* @param pArea <= area change.
* @param pVolume <= volume change.
* @param pUserEdgeArray <= Array of nodes around the boundary.   May be null.
* @param pUserFaceArray <= array of nodes of neighborhood triangles, in groups of 3.
* @param edgeNodeId => either node of the edge to be collapsed.
* @param abortMask => a false return is forced if this mask is encountered in the
*     neighborhood.  This mask almost always includes the exterior edge mask.
*     It may also include a visit mask.
* @param visitMask => mask to be set on all nodes of the neighorhood.  May be the null mask.
*
* @bsimethod                                                      EarlinLutz      02/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGFacets_evaluateEdgeCollapse
(
MTGFacets*      pFacets,
DPoint3d*       pApex,
double*         pDist,
double*         pArea,
double*         pVolume,
DPoint3d*       pLeftNormal,
DPoint3d*       pRightNormal,
double*         pMinNormalCosine,
int*            pNumSharpAnglesMoved,
int*            pNumEdge,
double          sharpAngleRadians,
MTGNodeId       edgeNodeId,
MTGMask         abortMask
)
    {
    MTGGraph       *pGraph = jmdlMTGFacets_getGraph (pFacets);
    MTGNodeId       farNodeId = jmdlMTGGraph_getEdgeMate (pGraph, edgeNodeId);
    MTGNodeId       node1Id, node2Id, node3Id;
    DPoint3d        basePoint, farPoint, baseVector, point1, point2, vector1, vector2,
                    oldCrossProduct, newCrossProduct;
    double          oldArea, newArea;
    double          oldLength, newLength;
    double          volume;
    DPoint3d        leftNormal, rightNormal;
    DPoint3d        currNormal, previousNormal;
    double          minCosine, oldMag, newMag;
    double          shiftLength;
    int             numEdge = 0;

    leftNormal.Zero ();
    rightNormal.Zero ();

    if (jmdlMTGGraph_getMask (pGraph, edgeNodeId, abortMask))
        return false;

    MTGARRAY_VERTEX_LOOP (currNodeId, pGraph, farNodeId)
        {
        if (jmdlMTGGraph_getMask (pGraph, currNodeId, abortMask))
            return false;
        }
    MTGARRAY_END_VERTEX_LOOP (currNodeId, pGraph, farNodeId)

    jmdlMTGFacets_getNodeCoordinates (pFacets, &basePoint, edgeNodeId);
    jmdlMTGFacets_getNodeCoordinates (pFacets, &farPoint, farNodeId);

    if (pApex)
        *pApex = basePoint;

    if (pNumSharpAnglesMoved)
        *pNumSharpAnglesMoved = 0;
    if (pNumEdge)
        *pNumEdge = 0;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&baseVector, &basePoint, &farPoint);
    vector2 = baseVector;
    point2 = basePoint;
    shiftLength = bsiDPoint3d_magnitude (&baseVector);

    oldArea = newArea = volume = oldLength = newLength = 0.0;
    minCosine = 1.0;
    /* To keep the code analyzer from thinking this is uninitialized ... */
    bsiDPoint3d_zero (&previousNormal);
    /* For each triangle on the old cone ... */
    MTGARRAY_VERTEX_LOOP (node0Id, pGraph, farNodeId)
    {
    node1Id = jmdlMTGGraph_getFSucc (pGraph, node0Id);
    node2Id = jmdlMTGGraph_getFSucc (pGraph, node1Id);
    node3Id = jmdlMTGGraph_getFSucc (pGraph, node2Id);
    numEdge++;
    if (node3Id != node0Id)
        return false;
    vector1 = vector2;
    point1 = point2;
    /* Compute the area of the old triangle */
    jmdlMTGFacets_getNodeCoordinates (pFacets, &point2, node2Id);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector2, &point2, &farPoint);
    bsiDPoint3d_crossProduct (&oldCrossProduct, &vector1, &vector2);
    currNormal = oldCrossProduct;
    oldArea += (oldMag = bsiDPoint3d_magnitude (&oldCrossProduct));

    oldLength += bsiDPoint3d_distance (&farPoint, &point2);
    newLength += bsiDPoint3d_distance (&basePoint, &point2);
    /* volume += oldArea * shiftLength; */

    if (pNumSharpAnglesMoved && node0Id != farNodeId)
        {
        if (bsiDPoint3d_angleBetweenVectors (&previousNormal, &currNormal)
            > sharpAngleRadians
            )
                {
                *pNumSharpAnglesMoved += 1;
                }
        }
    previousNormal = currNormal;
    /* First and last triangles of old cone are collapsed, make no volume
       or area contributions in new cone. */
    if (node0Id == farNodeId)
        {
        rightNormal = oldCrossProduct;
        }
    else if (node2Id == edgeNodeId)
        {
        leftNormal = oldCrossProduct;
        }
    else if (node0Id != farNodeId && node2Id != edgeNodeId)
        {
        /* All others contribute to both area and volume of new cone. */
        double          dot, magProduct, newCosine;
        bsiDPoint3d_crossProduct3DPoint3d (&newCrossProduct, &basePoint, &point1, &point2);
        newArea += (newMag = bsiDPoint3d_magnitude (&newCrossProduct));
        volume += fabs (bsiDPoint3d_tripleProduct4Points (&farPoint, &basePoint, &point1, &point2));
        dot = bsiDPoint3d_dotProduct (&oldCrossProduct, &newCrossProduct);
        magProduct = oldMag * newMag;
        if (magProduct <= 0.0)
            {
            minCosine = -1.0;
            }
        else
            {
            newCosine = dot / magProduct;
            }
        }
    }
    MTGARRAY_END_VERTEX_LOOP (node0Id, pGraph, farNodeId)

    if (pDist)
        *pDist = fabs (newLength - oldLength);

    if (pVolume)
        *pVolume = volume;

    if (pArea)
        *pArea = fabs (newArea - oldArea);

    if (pLeftNormal)
        *pLeftNormal = leftNormal;

    if (pRightNormal)
        *pRightNormal = rightNormal;

    if (pMinNormalCosine)
        *pMinNormalCosine = minCosine;

    if (pNumEdge)
        *pNumEdge = numEdge;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Collapse an edge. Optionally delete the (3) excised edges.
* @param *pCutNode0Id <= first node of final twist
* @param *pCutNode1Id <= last node of final twist
* @param edgeNodeId => base note of edge to remove.
* @param remove     => true to force delete of removed edges
* @bsimethod                                                      EarlinLutz      02/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlMTGFacets_collapseEdge
(
MTGGraph       *pGraph,
MTGNodeId      *pCutNode0Id,
MTGNodeId      *pCutNode1Id,
MTGNodeId       edgeNodeId,
bool            remove
)
    {
    MTGNodeId       id0, id1, id2, id3, id4, id5, id6, id7, id8, id9;

    /* Left side neighborhood */
    id0 = edgeNodeId;
    id1 = jmdlMTGGraph_getFSucc (pGraph, id0);
    id2 = jmdlMTGGraph_getFSucc (pGraph, id1);
    id3 = jmdlMTGGraph_getVSucc (pGraph, id2);
    id4 = jmdlMTGGraph_getFSucc (pGraph, id3);

    /* Right side neighborhood */
    id5 = jmdlMTGGraph_getVSucc (pGraph, id1);
    id6 = jmdlMTGGraph_getFSucc (pGraph, id5);
    id7 = jmdlMTGGraph_getFSucc (pGraph, id6);
    id8 = jmdlMTGGraph_getVSucc (pGraph, id7);
    id9 = jmdlMTGGraph_getFSucc (pGraph, id8);

    /* Yank the far struts of the Z */
    jmdlMTGGraph_vertexTwist (pGraph, id7, id8);
    jmdlMTGGraph_vertexTwist (pGraph, id2, id3);
    /* excise internal cornes of the Z */
    jmdlMTGGraph_vertexTwist (pGraph, id4, id5);
    jmdlMTGGraph_vertexTwist (pGraph, id0, id9);
    /* Rejoin the exposed clusters */
    jmdlMTGGraph_vertexTwist (pGraph, id4, id9);

    if (remove)
        {
        jmdlMTGGraph_dropEdge (pGraph, id8);
        jmdlMTGGraph_dropEdge (pGraph, id0);
        jmdlMTGGraph_dropEdge (pGraph, id1);
        }
    if (pCutNode0Id)
        *pCutNode0Id = id9;
    if (pCutNode1Id)
        *pCutNode1Id = id4;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* One sweep of edge collapse logic.
*<ol>
*<li>     Generate a sorted list of <candidate,distance> pairs.</li>
*<li>     Starting with smallest distance, examine the neighborhood of each candidate.  Reject
*     neighborhoods that fail threshold tests for large changs.  Assign
*     a numeric measure of change to surviving candidates.</li>
*<li>   Sort by the numeric measure.</li>
*<li>     Collapse edges in sort order, but preventing any collapses on overlapping
*     patches.</li>
*</ol>
* @param maxDistance => measure of largest collapse allowed.  This is applied
*       in several ways.  No longer edge may be collapsed.  Collapses which cause
*       area change larger than this distance squared are ruled out, as are collapses
*       which cause volume changes in the range of this distance cubed.  If 0, no distance
*       test is performed.
* @param sharpAngleRadians => measure of sharp angle. Suggested value around 30 degrees
*       (about 0.5 radians).   Vertices incident to 2 edges with larger angles may
*       move along those edges but not to the side.   Vertices incident to 3 or more
*       sharp edge may not be moved at all.  If 0, no angle test is performed.
* @param dummy0 => reserved for future use.  Enter 0.0
* @param dummy1 => reserved for future use.  Enter 0.0
* @param mask0  => reserved for future use.  Enter MTG_NULL_MASK
* @param mask1  => reserved for future use.  Enter MTG_NULL_MASK
* @return the number of collapses performed.
* @bsimethod                                                      EarlinLutz      02/99
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP int      jmdlMTGFacets_decimateDisjointPatches
(
MTGFacets*      pFacets,
double          maxDistance,
double          sharpAngleRadians,
double          dummy0,
double          dummy1,
MTGMask         mask0,
MTGMask         mask1
)
    {
    MTGGraph       *pGraph = jmdlMTGFacets_getGraph (pFacets);
    GraphicsPointArrayP pGPA = jmdlGraphicsPointArray_grab ();
    GraphicsPoint   gp;

    int             vertexLabelOffset = pFacets->vertexLabelOffset;
    DPoint3d        leftNormal, rightNormal;
    double          distance, areaChange;
    double          minNormalCosine;
    double          v1, v2, v3, vSum;
    static double   w1 = 1.0;
    static double   w2 = 1.0;
    static double   w3 = 10.0;
    DPoint3d        apex;
    MTGMask         visitMask = jmdlMTGGraph_grabMask (pGraph);
    MTGMask         skipMask = MTG_EXTERIOR_MASK | MTG_BOUNDARY_MASK;
    EmbeddedIntArray *pFaceArray = jmdlEmbeddedIntArray_grab ();
    int             numCollapse = 0;
    int             k;
    double          q1 = maxDistance * maxDistance * maxDistance;
    double          q2 = q1;
    double          q3 = q1;
    bool            noMetricLimit = maxDistance <= 0.0;
    int             numSharpAnglesMoved = 0;
    int             maxSharpAnglesMoved = sharpAngleRadians > 0.0 ? 2 : 10000;
    static int      minEdgeForCollapsibleVertex = 3;
    int             numEdge;

    /* Build up the array of <point, distance, nodeId> */
    MTGARRAY_SET_LOOP (nodeId, pGraph)
    {
    if (jmdlMTGFacets_evaluateEdgeCollapse (pFacets,
                        &apex,
                        &distance,
                        &areaChange,
                        &v3,
                        &leftNormal, &rightNormal,
                        &minNormalCosine,
                        &numSharpAnglesMoved,
                        &numEdge,
                        sharpAngleRadians,
                        nodeId,
                        skipMask)
        && numEdge > minEdgeForCollapsibleVertex
        && jmdlMTGFacets_edgeCollapseNormalsOK (pFacets, nodeId)
        )
        {
        v1 = distance * distance * distance;
        v2 = areaChange * sqrt (areaChange);
        if (maxSharpAnglesMoved <= 0 || numSharpAnglesMoved < maxSharpAnglesMoved)
            {
            if (noMetricLimit || (v1 < q1 && v2 < q2 && v3 < q3 && minNormalCosine > 0.0))
                {
                vSum = w1 * v1 + w2 * v2 + w3 * v3;
                vSum /= minNormalCosine;
                bsiGraphicsPoint_initFromDPoint3d (&gp, &apex, 1.0, vSum, 0, nodeId);
                jmdlGraphicsPointArray_addGraphicsPoint (pGPA, &gp);
                }
            }
        }
    }
    MTGARRAY_END_SET_LOOP (nodeId, pGraph)

    /* Sort so small collapses come first */
    jmdlGraphicsPointArray_sortByA (pGPA);

    /* Mark neighborhoods to avoid conflict */
    jmdlMTGGraph_clearMaskInSet (pGraph, visitMask);
    for (k = 0; jmdlGraphicsPointArray_getGraphicsPoint (pGPA, &gp, k); k++)
        {
        int             candidateNodeId = gp.userData;
        gp.mask = 0;
        jmdlEmbeddedIntArray_empty (pFaceArray);
        if (jmdlMTGFacets_searchTriangulatedEdgeNeighborhood (pFacets, NULL, pFaceArray,
                               NULL, NULL, candidateNodeId, visitMask))
            {
            jmdlMTGGraph_setMaskInArray (pGraph, pFaceArray, visitMask);
            gp.mask = 1;
            }
        jmdlGraphicsPointArray_setGraphicsPoint (pGPA, &gp, k);
        }

    /* Collapse survivors */
    for (k = 0; jmdlGraphicsPointArray_getGraphicsPoint (pGPA, &gp, k); k++)
        {
        if (gp.mask)
            {
            int             candidateNodeId = gp.userData;
            int             vertexIndex;
            MTGNodeId       cut0Id, cut1Id;
            bsiDPoint4d_cartesianFromHomogeneous (&gp.point, &apex);
            jmdlMTGFacets_collapseEdge (pGraph, &cut0Id, &cut1Id, candidateNodeId, true);
            vertexIndex = jmdlMTGFacets_addVertex (pFacets, &apex, NULL);
            numCollapse++;

            /* Fix up the vertex label. */
            /* NEEDS WORK -- normal label */
            MTGARRAY_VERTEX_LOOP (nodeId, pGraph, cut0Id)
            {
            if (nodeId == MTG_NULL_NODEID)
                break;
            jmdlMTGGraph_setLabel (pGraph, nodeId, vertexLabelOffset, vertexIndex);
            }
            MTGARRAY_END_VERTEX_LOOP (nodeId, pGraph, cut0Id)
            }
        }


    jmdlGraphicsPointArray_drop (pGPA);
    jmdlMTGGraph_dropMask (pGraph, visitMask);
    jmdlEmbeddedIntArray_drop (pFaceArray);
    return numCollapse;
    }

