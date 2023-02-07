/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "Regions/rg_intern.h"
#include <stdio.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


#define DEBUG_MERGE

/*---------------------------------------------------------------------------------**//**
*
* @param pTangent[i] = unit vector from pPoint[i][0] to pPoint[i][1]
* @param pLength[i] = length of segment i
* @param pInvLength[i] = 1 over length of segment i
* @param pLocalCoord[i][j] = coordinates of point j of the other line (1-i) in the local unit
*                       coordinate system of line i.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool       jmdlRGMerge_xyLineContext
(
      DPoint2d  pTangent[2],
      double    pLength[2],
      double    pInvLength[2],
      DPoint2d  pLocalCoord[2][2],
      int       *pILong,
      int       *pIShort,
const DPoint3d  pPoint[2][2],
      double    tol
)
    {
    int j, line, otherLine;
    double dx, dy, x0, y0, tx, ty;
    double a;
    int numShort = 0;
    bool    myStat = false;

    for (line = 0; line < 2; line++)
        {
        dx = pPoint[line][1].x - pPoint[line][0].x;
        dy = pPoint[line][1].y - pPoint[line][0].y;
        /* Remark: Just take the square roots and don't worry about the
            occasional benefit of not doing them */
        pLength[line] = sqrt(dx * dx + dy * dy);
        if (pLength[line] <= tol)
            {
            numShort++;
            }
        else
            {
            a = pInvLength[line] = 1.0 / pLength[line];
            pTangent[line].x = dx * a;
            pTangent[line].y = dy * a;
            }
        }

    if (numShort == 0)
        {
        myStat = true;
        for (line = 0; line < 2; line++)
            {
            x0 = pPoint[line][0].x;
            y0 = pPoint[line][0].y;
            tx = pTangent[line].x;
            ty = pTangent[line].y;
            otherLine = 1 - line;
            for (j = 0; j < 2; j++)
                {
                dx = pPoint[otherLine][j].x - x0;
                dy = pPoint[otherLine][j].y - y0;
                pLocalCoord[line][j].x = dx * tx + dy * ty;
                pLocalCoord[line][j].y = tx * dy - ty * dx;
                }
            }

        *pIShort = (pLength[0] < pLength[1]) ? 0 : 1;
        *pILong = 1 - *pIShort;
        }

    return  myStat;
    }

/*---------------------------------------------------------------------------------**//**
*
* @param    pi0 <= index of nearer line end.
* @param    pi1 <= index of further line end.
* @param    s   => parameter along line.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     jmdlRGMerge_selectOrigin
(
int     *pi0,
int     *pi1,
double  s
)
    {
    if (s <= 0.5)
        {
        *pi0 = 0;
        *pi1 = 1;
        }
    else
        {
        *pi0 = 1;
        *pi1 = 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
*
* Compute intersections between the xy projections of two line segments.
* @param    nodeAId =>
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool        jmdlRGMerge_segmentSegmentIntersection
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
      MTGNodeId     nodeId0,
const DPoint3d      *pPoint00,
const DPoint3d      *pPoint01,
      MTGNodeId     nodeId1,
const DPoint3d      *pPoint10,
const DPoint3d      *pPoint11
)
    {
    double tol = jmdlRG_getTolerance (pRG);
    double tol2 = tol * tol;
    DPoint2d tangent[2];
    double tangentLength[2];
    double inverseLength[2];

    DPoint2d localCoord[2][2];
    DPoint3d point[2][2];
    bool    vertexAssigned[2][2];
    int iShort, iLong;
    double x0, y0, x1, y1, dy, s, t, xx, projectedParameter;
    int i0, i1, j0, j1, iLine, jLine;
    static bool    s_bAnnounceVertexOnVertex = false;

#ifdef DEBUG_MERGE
    static int s_noisy = 0;
#endif

    MTGNodeId nodeId[2];

    point[0][0] = *pPoint00;
    point[0][1] = *pPoint01;
    point[1][0] = *pPoint10;
    point[1][1] = *pPoint11;
    nodeId[0] = nodeId0;
    nodeId[1] = nodeId1;

    /* Analyze coordinates of each line relative to the other.
        This is more computation than you might expect up front, but
        it makes the tolerancing tests remarkably clean.
    */

    if (jmdlRGMerge_xyLineContext (
                tangent,
                tangentLength,
                inverseLength,
                localCoord,
                &iLong, &iShort,
                point,
                tol2))
        {
        /* Extract local coordinates of the short line relative to the long one */
        x0 = localCoord[iLong][0].x;
        y0 = localCoord[iLong][0].y;
        x1 = localCoord[iLong][1].x;
        y1 = localCoord[iLong][1].y;
        dy = y1 - y0;
#ifdef DEBUG_MERGE
        if (s_noisy)
            {
            static double printTol = 0.02;
            double uu0 = x0 * inverseLength[iLong];
            double uu1 = x1 * inverseLength[iLong];
            if (fabs ((fabs (uu0 - 0.5) - 0.5))  < printTol
                && (fabs (fabs (uu1 - 0.5) - 0.5)) < printTol)
                {
                GEOMAPI_PRINTF ("Near coincident lines (%le, %le) (%le,%le) \n", uu0, y0, uu1, y1);
                }
            }
#endif
        if (fabs (dy) < tol)
            {
            /* More or less parallel lines....*/
            if (fabs (y0) < tol || fabs (y1) < tol)
                {
                /* and also colinear.  Test direct equality of endpoints */
                vertexAssigned[0][0]
                        = vertexAssigned[0][1]
                        = vertexAssigned[1][0]
                        = vertexAssigned[1][1] = false;

                for (i0 = 0; i0 < 2; i0++)
                    {
                    for (j0 = 0; j0 < 2; j0++)
                        {
                        if (point[iLong][i0].DistanceSquaredXY (*(&point[iShort][j0]))
                                    <= tol2 )
                            {
                            if (s_bAnnounceVertexOnVertex)
                                jmdlRGIL_declareVertexOnVertex
                                    (pRG, pIL, nodeId[iLong], i0, nodeId[iShort], j0);
                            vertexAssigned[iLong][i0] = true;
                            vertexAssigned[iShort][j0] = true;
                            }
                        }
                    }

                for (iLine = 0; iLine < 2; iLine++)
                    {
                    int i;
                    double xi;
                    jLine = 1 - iLine;
                    for (i = 0; i < 2; i++)
                        {
                        /* is point i of segemnt iLine "on" the interior of segment jLine?? */
                        if (!vertexAssigned[iLine][i])
                            {
                            xi = localCoord[jLine][i].x;
                            if (xi > tol && xi < tangentLength[jLine] - tol)
                                {
                                projectedParameter = xi * inverseLength[jLine];
                                /* GEOMAPI_PRINTF (" vertexOnSegment @ %lf\n", projectedParameter); */
                                jmdlRGIL_declareVertexOnSegmentByProximityTest
                                        (pRG, pIL,
                                        nodeId[iLine], i,
                                        nodeId[jLine], projectedParameter);
                                }
                            }
                        }
                    }

                }
            }
        else
            {
            /* The lines are transverse, i.e. clearly intersecting, and the parameters where
                the intesection occurs can be computed safely.   Comparing each parameter to the
                midpoint tells which endpoint is closer to the other line */
            s = -y0 / dy;
            xx = x0 + s * (x1 - x0);        /* in local coordinates of longer line.  yy is zero */
            t = xx * inverseLength[iLong];
            jmdlRGMerge_selectOrigin (&i0, &i1, t);
            jmdlRGMerge_selectOrigin (&j0, &j1, s);

#ifdef DEBUG_MERGE
                if (s_noisy)
                    GEOMAPI_PRINTF(" line-line: long seg %d at %lf [%2d%2d]  short seg %d at %lf [%2d%2d] \n",
                            iLong, t, i0, i1, iShort, s, j0, j1);
#endif

            if (point[iLong][i0].DistanceSquaredXY (*(&point[iShort][j0]))
                        <= tol2 )
                {
#ifdef DEBUG_MERGE
                if (s_noisy)
                    GEOMAPI_PRINTF("    (vertex-vertex)\n");
#endif
                if (s_bAnnounceVertexOnVertex)
                        jmdlRGIL_declareVertexOnVertex (pRG, pIL, nodeId[iLong], i0, nodeId[iShort], j0);
                }
            else if (  fabs (localCoord[iLong][j0].y) < tol
                    && xx > 0.0
                    && xx < tangentLength[iLong]
                    )
                {
                projectedParameter = localCoord[iLong][j0].x * inverseLength[iLong];
                jmdlRGIL_declareVertexOnSegmentByProximityTest (pRG, pIL, nodeId[iShort], j0, nodeId[iLong], projectedParameter);
#ifdef DEBUG_MERGE
                if (s_noisy)
                    GEOMAPI_PRINTF("    (long at %lf short at end %d)\n", projectedParameter, j0);
#endif
                }
            else if ( fabs (localCoord[iShort][i0].y) < tol
                    && localCoord[iShort][i0].x > 0.0
                    && localCoord[iShort][i0].x < tangentLength[iShort]
                    )
                {
                projectedParameter = localCoord[iShort][i0].x * inverseLength[iShort];
                jmdlRGIL_declareVertexOnSegmentByProximityTest (pRG, pIL, nodeId[iLong], i0, nodeId[iShort], projectedParameter);
#ifdef DEBUG_MERGE
                if (s_noisy)
                    GEOMAPI_PRINTF("    (short at %lf long at end %d)\n", projectedParameter, i0);
#endif
                }
            else if (fabs (s - 0.5) <= 0.5 && fabs (t - 0.5) < 0.5)
                {
#ifdef DEBUG_MERGE
/*
                if (s_noisy)
                    GEOMAPI_PRINTF("    (short at %lf long at %lf)\n", projectedParameter, i0);
*/
#endif
                jmdlRGIL_declareSimpleIntersection (pRG, pIL, nodeId[iLong], t, nodeId[iShort], s);
                }
            else
                {
#ifdef DEBUG_MERGE
                if (s_noisy)
                    GEOMAPI_PRINTF("    NONE\n");
#endif
                }
            }
        }
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
*
* Test for vertex on edge conditions.
* @param pEdge0 => edge whose end is being tested.
* @param endSelect => 0 for start, 1 for end.
* @param pEdge1 => edge whose body is being tested.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool        jmdlRGMerge_testVertexOnEdgeStrictInterior
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
RG_EdgeData         *pEdge0,
int                 endSelect,
RG_EdgeData         *pEdge1
)
    {
    double param1, dsq;
    DPoint3d nearPoint;
    double tol = jmdlRG_getTolerance (pRG);
    double tolsq = tol * tol;
    double d0, d1;

    DPoint3d xyz;
    if (endSelect != 0)
        endSelect = 1;

    xyz = pEdge0->xyz[endSelect];

    if (jmdlRG_getClosestXYPointOnEdge
                (
                pRG,
                &param1,
                &dsq,
                &nearPoint,
                NULL,
                &xyz,
                pEdge1->nodeId[0]
                ))
        {
        if (dsq <= tolsq)
            {
            d0 = xyz.DistanceSquaredXY (*(&pEdge1->xyz[0]));
            d1 = xyz.DistanceSquaredXY (*(&pEdge1->xyz[1]));

            if (d0 > tolsq && d1 > tolsq)
                {
#ifdef abc
                GEOMAPI_PRINTF (" Detect vertex %d of nodeId %d on nodeId %d at %lf\n",
                                endSelect, pEdge0->nodeId[0],
                                pEdge1->nodeId[1], param1);
#endif
                jmdlRGIL_declareVertexOnSegmentByProximityTest (pRG, pIL,
                                pEdge0->nodeId[0], endSelect,
                                pEdge1->nodeId[0], param1);
                }
            }
        }
    return false;
    }
/*---------------------------------------------------------------------------------**//**
*
* Compute intersections between two edges described by the node ids.
* Add the intersections to a (growing) list for later sorting.
* @param    nodeAId =>
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool        jmdlRGMerge_cciEdgeEdge
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
RG_EdgeData         *pEdge0,
RG_EdgeData         *pEdge1
)
    {
    bool    ok = true;

    if (pEdge0 && pEdge0 == pEdge1)
        {
        if (pEdge0->curveIndex != RG_NULL_CURVEID)
            ok = jmdlRG_linkToCurveCurveIntersection (pRG, pIL,
                                        pEdge0, pEdge0);
        }
    else if (pEdge0 && pEdge1)
        {
        if  (   pEdge0->curveIndex != RG_NULL_CURVEID
             || pEdge1->curveIndex != RG_NULL_CURVEID)
            {
            jmdlRGMerge_testVertexOnEdgeStrictInterior (pRG, pIL, pEdge0, 0, pEdge1);
            jmdlRGMerge_testVertexOnEdgeStrictInterior (pRG, pIL, pEdge0, 1, pEdge1);
            jmdlRGMerge_testVertexOnEdgeStrictInterior (pRG, pIL, pEdge1, 0, pEdge0);
            jmdlRGMerge_testVertexOnEdgeStrictInterior (pRG, pIL, pEdge1, 1, pEdge0);
            }

        if  (   pEdge0->curveIndex == RG_NULL_CURVEID
             && pEdge1->curveIndex == RG_NULL_CURVEID)
            {
            ok = jmdlRGMerge_segmentSegmentIntersection (pRG, pIL,
                                        pEdge0->nodeId[0], &pEdge0->xyz[0], &pEdge0->xyz[1],
                                        pEdge1->nodeId[0], &pEdge1->xyz[0], &pEdge1->xyz[1]);
            }
        else if (pEdge0->curveIndex == RG_NULL_CURVEID)
            {
            ok = jmdlRG_linkToSegmentCurveIntersection (pRG, pIL,
                                        pEdge0, pEdge1);
            }
        else if (pEdge1->curveIndex == RG_NULL_CURVEID)
            {
            ok = jmdlRG_linkToSegmentCurveIntersection (pRG, pIL,
                                        pEdge1, pEdge0);
            }
        else
            {
            ok = jmdlRG_linkToCurveCurveIntersection (pRG, pIL,
                                        pEdge1, pEdge0);
            }
        }
    return  ok;
    }


/*---------------------------------------------------------------------------------**//**
*
* Compute intersections between two edges described by the node ids.
* Add the intersections to a (growing) list for later sorting.
* @param    nodeAId =>
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool        jmdlRGMerge_cciNodeNode
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
MTGNodeId           nodeAId,
MTGNodeId           nodeBId
)
    {
    bool    ok = false;
    RG_EdgeData edge0, edge1;

    if  (   jmdlRG_getEdgeData (pRG, &edge0, nodeAId)
         && jmdlRG_getEdgeData (pRG, &edge1, nodeBId)
        )
        {
        if (nodeAId == nodeBId)
                ok = jmdlRGMerge_cciEdgeEdge (pRG, pIL, &edge0, &edge0);
            else
                ok = jmdlRGMerge_cciEdgeEdge (pRG, pIL, &edge0, &edge1);
        }
    return  ok;
    }

/*---------------------------------------------------------------------------------**//**
*
* Compute intersections between two edges described by the node ids.
* Add the intersections to a (growing) list for later sorting.
* @param    nodeAId =>
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool        jmdlRGMerge_cciNodeEdge
(
RG_Header           *pRG,
RG_IntersectionList *pIL,
MTGNodeId           node0Id,
RG_EdgeData         *pEdge1
)
    {
    bool    ok = false;
    RG_EdgeData edge0;

    if  (   jmdlRG_getEdgeData (pRG, &edge0, node0Id))
        {
        ok = jmdlRGMerge_cciEdgeEdge (pRG, pIL, &edge0, pEdge1);
        }
    return  ok;
    }

static int s_findAll = false;
/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in the graph.
* Return them as (huge!!!) array.
*
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRGMerge_findAllIntersections
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL
)
    {
    MTGGraph *pGraph = pRG->pGraph;

    /* Brute force quadratic comparison. */
    /* Each loop visits each edge twice -- use id order to
        limit to one time use. */
    MTGARRAY_SET_LOOP (nodeA0Id, pGraph)
        {
        if  (jmdlMTGGraph_getMask (pGraph, nodeA0Id, MTG_DIRECTED_EDGE_MASK))
            {
            MTGARRAY_SET_LOOP (nodeB0Id, pGraph)
                {
                if (   nodeA0Id <= nodeB0Id
                    && jmdlMTGGraph_getMask (pGraph, nodeB0Id, MTG_DIRECTED_EDGE_MASK)
                    && !jmdlRGMerge_cciNodeNode (pRG, pRGIL, nodeA0Id, nodeB0Id)
                   )
                    return          false;
                }
            MTGARRAY_END_SET_LOOP (nodeB0Id, pGraph)
            }
        }
    MTGARRAY_END_SET_LOOP (nodeA0Id, pGraph)

    return  true;
    }

typedef struct
    {
    int                     count0;
    int                     count1;
    RG_Header               *pRG;
    MTGNodeId               nodeAId;
    RG_IntersectionList     *pRGIL;
    } RangeTreeCCINodeContext;

typedef struct
    {
    RG_Header               *pRG;
    RG_IntersectionList     *pRGIL;
    RG_EdgeData             edgeData;
    } RangeTreeCCIEdgeContext;

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in the graph.
* Return them as (huge!!!) array.
*
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRGMerge_cb_cciFromRangeTreeHit
(
HideTreeRange   *pRange,
int             nodeBId,
RangeTreeCCINodeContext *pContext,
void            *pParent,
void            *pLeaf
)
    {
    if (pContext->nodeAId <= nodeBId)
        {
        jmdlRGMerge_cciNodeNode (pContext->pRG, pContext->pRGIL, pContext->nodeAId, nodeBId);
        pContext->count1++;
        }

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* Check self-intersection by an edge.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void     jmdlRGMerge_checkSelfIntersection
(
RangeTreeCCINodeContext *pContext,
MTGNodeId               nodeId
)
    {
    /* Full closure special case  ...*/
    RG_EdgeData edgeData;
    double tol = jmdlRG_getTolerance (pContext->pRG);
    double tol2 = tol * tol;

    if  (   jmdlRG_getEdgeData (pContext->pRG, &edgeData, nodeId)
        &&  edgeData.xyz[0].DistanceSquared (*(&edgeData.xyz[1]))
                    < tol2)
        {
        jmdlRGIL_declareVertexOnVertex  (
                                        pContext->pRG, pContext->pRGIL,
                                        nodeId, 0,
                                        nodeId, 1
                                        );
        /* Insert a break in the middle of the edge,
            so it is nominally two edges with distinct
            start and endvertices.
        */
        jmdlRGIL_declareIsolatedBreak (pContext->pRG,
                                        pContext->pRGIL,
                                        nodeId, 0.53423423
                                        );
        }
    }

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in the graph.
* Return them as (huge!!!) array.
*
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRGMerge_findAllIntersectionsFromRangeTree
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL
)
    {
    if (s_findAll)
        return jmdlRGMerge_findAllIntersections (pRG, pRGIL);

    MTGGraph *pGraph = pRG->pGraph;
    RangeTreeCCINodeContext context;
    HideTreeRange   nodeARange;
    DRange3d nodeADRange3d;
    int worstCase;
    static int s_printMergeCount = 0;

    context.pRG = pRG;
    context.pRGIL = pRGIL;
    context.count0 = context.count1 = 0;

    MTGARRAY_SET_LOOP (nodeA0Id, pGraph)
        {
        if  (   jmdlMTGGraph_getMask (pGraph, nodeA0Id, MTG_DIRECTED_EDGE_MASK)
             && jmdlRG_getEdgeRange (pRG, &nodeADRange3d, nodeA0Id)
            )
            {
            context.count0++;
            if (jmdlRG_checkAbort(pRG))
                return false;
            jmdlRGMerge_checkSelfIntersection (&context, nodeA0Id);
            rgXYRangeTree_initRangeFromDRange3d (&nodeARange, &nodeADRange3d);
            nodeARange.xmin -= pRG->tolerance;
            nodeARange.xmax += pRG->tolerance;
            nodeARange.ymin -= pRG->tolerance;
            nodeARange.ymax += pRG->tolerance;
            context.nodeAId = nodeA0Id;
            rgXYRangeTree_traverseTree (
                            pRG->pEdgeRangeTree,
                            (HideTree_NodeFunction)rgXYRangeTree_nodeFunction_testForRangeOverlap,
                            &nodeARange,
                            (HideTree_ProcessLeafFunction)jmdlRGMerge_cb_cciFromRangeTreeHit,
                            &context);
            }
        }
    MTGARRAY_END_SET_LOOP (nodeA0Id, pGraph)

    worstCase = context.count0 * context.count0 / 2;
    if (worstCase == 0)
        worstCase = 1;
    if (s_printMergeCount)
        GEOMAPI_PRINTF(" Merge counts: %d candidates, %d quadratic candidates, %d actual, %lf fractional\n",
                context.count0,
                worstCase,
                context.count1,
                (double)context.count1 / (double) worstCase
                );
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in the graph.
* Return them as (huge!!!) array.
*
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRGMerge_cb_edgeFaceIntersectionFromFaceRangeTreeHit
(
HideTreeRange   *pRange,
int             faceNodeId,
RangeTreeCCIEdgeContext *pContext,
void            *pParent,
void            *pLeaf
)
    {
    MTGGraph *pGraph = pContext->pRG->pGraph;
    int num0 = jmdlRGIL_getCount (pContext->pRGIL);
    int num1;

    MTGARRAY_FACE_LOOP (currNodeId, pGraph, faceNodeId)
        {
        jmdlRGMerge_cciNodeEdge (pContext->pRG, pContext->pRGIL, currNodeId, &pContext->edgeData);
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, faceNodeId)
    num1 = jmdlRGIL_getCount (pContext->pRGIL);

    jmdlRGIL_setSeeds (pContext->pRGIL, num0, num1, faceNodeId);
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in the graph.
* Return them as (huge!!!) array.
*
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRGIL_initTempEdge
(
RG_EdgeData                     *pEdgeData,
DRange3d                        *pRange,
const DPoint3d                  *pPoint0,
const DPoint3d                  *pPoint1,
int                             auxIndex
)
    {
    pEdgeData->nodeId[0] = pEdgeData->nodeId[1] = MTG_NULL_NODEID;
    pEdgeData->vertexIndex[0] = -1;
    pEdgeData->vertexIndex[1] = -1;
    pEdgeData->xyz[0] = *pPoint0;
    pEdgeData->xyz[1] = *pPoint1;
    pEdgeData->curveIndex = -1;
    pEdgeData->isReversed = false;
    pEdgeData->auxIndex = auxIndex;

    pRange->InitFrom(*pPoint0, *pPoint1);
    }

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in the graph.
* Return them as (huge!!!) array.
*
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRGIL_findPolylineIntersectionsFromFaceRangeTree
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL,
const DPoint3d                  *pPointArray,
int                             numPoint
)
    {
    RangeTreeCCIEdgeContext context;
    HideTreeRange   segmentSearchRange;
    DRange3d segmentRange;
    int         i0, i1;


    context.pRG = pRG;
    context.pRGIL = pRGIL;

    for (i1 = 1; i1 < numPoint; i1++)
        {
        i0 = i1 - 1;
        jmdlRGIL_initTempEdge (&context.edgeData, &segmentRange, pPointArray + i0, pPointArray + i1, i0);

        rgXYRangeTree_initRangeFromDRange3d (&segmentSearchRange, &segmentRange);

        rgXYRangeTree_traverseTree (
                        pRG->pFaceRangeTree,
                        (HideTree_NodeFunction)rgXYRangeTree_nodeFunction_testForRangeOverlap,
                        &segmentSearchRange,
                        (HideTree_ProcessLeafFunction)jmdlRGMerge_cb_edgeFaceIntersectionFromFaceRangeTreeHit,
                        &context);
        }

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in the graph.
* Return them as array.
*
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRGMerge_cb_edgeEdgeIntersectionFromEdgeRangeTreeHit
(
HideTreeRange   *pRange,
int             edgeNodeId,
RangeTreeCCIEdgeContext *pContext,
void            *pParent,
void            *pLeaf
)
    {
    int num0 = jmdlRGIL_getCount (pContext->pRGIL);
    int num1;

    jmdlRGMerge_cciNodeEdge (pContext->pRG, pContext->pRGIL, edgeNodeId, &pContext->edgeData);

    num1 = jmdlRGIL_getCount (pContext->pRGIL);

    jmdlRGIL_setSeeds (pContext->pRGIL, num0, num1, edgeNodeId);
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in the graph.
* Return them as (huge!!!) array.
*
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRGIL_findPolylineIntersectionsFromEdgeRangeTree
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL,
const DPoint3d                  *pPointArray,
int                             numPoint
)
    {
    RangeTreeCCIEdgeContext context;
    HideTreeRange   segmentSearchRange;
    DRange3d segmentRange;
    int         i0, i1;


    context.pRG = pRG;
    context.pRGIL = pRGIL;

    for (i1 = 1; i1 < numPoint; i1++)
        {
        i0 = i1 - 1;
        jmdlRGIL_initTempEdge (&context.edgeData, &segmentRange, pPointArray + i0, pPointArray + i1, i0);

        rgXYRangeTree_initRangeFromDRange3d (&segmentSearchRange, &segmentRange);

        rgXYRangeTree_traverseTree (
                        pRG->pEdgeRangeTree,
                        (HideTree_NodeFunction)rgXYRangeTree_nodeFunction_testForRangeOverlap,
                        &segmentSearchRange,
                        (HideTree_ProcessLeafFunction)jmdlRGMerge_cb_edgeEdgeIntersectionFromEdgeRangeTreeHit,
                        &context);
        }

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* Find all curve-curve intersections in a face
*
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRGIL_findPolylineIntersectionsFromFace
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL,
MTGNodeId                       faceStartNodeId,
const DPoint3d                  *pPointArray,
int                             numPoint
)
    {
    MTGGraph *pGraph = pRG->pGraph;
    int i0, i1;
    DRange3d segmentRange;
    RG_EdgeData edgeData;
    int num0, num1;

    for (i1 = 1; i1 < numPoint; i1++)
        {
        i0 = i1 - 1;
        jmdlRGIL_initTempEdge (&edgeData, &segmentRange, pPointArray + i0, pPointArray + i1, i0);

        MTGARRAY_FACE_LOOP (edgeNodeId, pGraph, faceStartNodeId)
            {
            num0 = jmdlRGIL_getCount (pRGIL);
            jmdlRGMerge_cciNodeEdge (pRG, pRGIL, edgeNodeId, &edgeData);

            num1 = jmdlRGIL_getCount (pRGIL);
            jmdlRGIL_setSeeds (pRGIL, num0, num1, edgeNodeId);
            }
        MTGARRAY_END_FACE_LOOP (edgeNodeId, pGraph, faceStartNodeId)
        }

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* Find all intersections of an edge with a polyline.
*
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRGIL_findPolylineIntersectionsFromEdge
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL,
MTGNodeId                       edgeNodeId,
const DPoint3d                  *pPointArray,
int                             numPoint
)
    {
    int i0, i1;
    DRange3d segmentRange;
    RG_EdgeData edgeData;
    int num0, num1;

    for (i1 = 1; i1 < numPoint; i1++)
        {
        i0 = i1 - 1;
        jmdlRGIL_initTempEdge (&edgeData, &segmentRange, pPointArray + i0, pPointArray + i1, i0);

        num0 = jmdlRGIL_getCount (pRGIL);
        jmdlRGMerge_cciNodeEdge (pRG, pRGIL, edgeNodeId, &edgeData);

        num1 = jmdlRGIL_getCount (pRGIL);
        jmdlRGIL_setSeeds (pRGIL, num0, num1, edgeNodeId);
        }

    return  true;
    }
/*---------------------------------------------------------------------------------**//**
* See if an intersection list (form a temporary edge ray cast??) indicates a clear parity change.
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRGIL_intersectionListHasClearParity
(
RG_IntersectionList             *pRGIL,
bool                            *pParityChanged
)
    {
    const RG_Intersection *pA;
    int     changeCount = 0;
    int     vertexCount = 0;
    *pParityChanged = false;

    for (size_t i = 0;
        NULL != (pA = pRGIL->GetIntersectionCPAt (i)); i++)
        {
        int geomType = RGI_GETMASK (pA->type, RGI_GEOMETRIC_BITS);
        if (geomType == RGI_MASK_START_POINT
            || geomType == RGI_MASK_END_POINT)
            {
            vertexCount++;
            return false;
            }
        else if (geomType == RGI_MASK_SIMPLE || geomType == RGI_MASK_CLOSE_APPROACH)
            {
            if (pA->nodeId != MTG_NULL_NODEID)
                changeCount++;
            }
        }

    *pParityChanged = changeCount & 0x01;
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @return index i1 such that all records i0<=i<i1 are have the same nodeId as i0.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public int             jmdlRGIL_findUpperIndexByNodeId
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL,
int                             i0
)
    {
    const RG_Intersection *pA, *pB;
    int i1 = i0;
    pA = pRGIL->GetIntersectionCPAt ((size_t)i0);
    if  (pA)
        {
        while (NULL != (pB = (pRGIL->GetIntersectionCPAt ((size_t)(++i1))))
                && pA->nodeId == pB->nodeId)
            {}
        }
    return  i1;
    }

/*---------------------------------------------------------------------------------**//**
* @return index i1 such that all records i0<=i<i1 are have the same clusterIndex as i0.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public int             jmdlRGIL_findUpperIndexByClusterIndex
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL,
int                             i0
)
    {
    const RG_Intersection *pA, *pB;
    int i1 = i0;
    pA = pRGIL->GetIntersectionCPAt ((size_t) i0);
    if  (pA)
        {
        while (NULL != (pB = pRGIL->GetIntersectionCPAt ((size_t)(++i1)))
                && pA->clusterIndex == pB->clusterIndex)
            {}
        }
    return  i1;
    }

/*---------------------------------------------------------------------------------**//**
* @return number of subedges defined by the record range
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public int             jmdlRGIL_countSubEdges
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL,
int                             i0,
int                             i1
)
    {
    const RG_Intersection *pA;
    int numEdge = 1;
    int currType;
    for (size_t i = (size_t)i0; i < (size_t)i1; i++)
        {
        pA = pRGIL->GetIntersectionCPAt (i);
        if ((currType = RGI_GETPROCESSFIELD(pA->type)) == RGI_MASK_PROCESSED_MASTER)
            numEdge++;
        }
    return  numEdge;
    }


/*---------------------------------------------------------------------------------**//**
* Scan the descriptor range i0 <= i < i1 and set the label paramter to the
* index of the descriptor which is its 'master' in the range.  Each master is the
* first of the contiguous subrange with common parameter.
* @return number of edges which will be present after subdivision.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public int             jmdlRGIL_markDistinctParameters
(
RG_Header                       *pRG,
RG_IntersectionList             *pRGIL,
int                             i0,
int                             i1,
double                          minParamStep,
double                          minGeomStep
)
    {
    RG_Intersection *pA;
    RG_EdgeData edgeData;
    int i;
    int numEdge = 1;
    double breakParam = minParamStep;
    double break0 = minParamStep;
    double break1 = 1.0 - minParamStep;
    double break0g = 0.5;
    double break1g = 0.5;
    double param;
    int    processedTypeMask;
    int master0 = i0 - 1;
    int master1 = i0 - 1;
    int master  = i0 - 1;
    double dist;
    DPoint3d currPoint, startPoint, endPoint, prevPoint;

    if (i1 < i0)
        return numEdge;

    pA = pRGIL->GetIntersectionPAt ((size_t)i0);
    if (    !jmdlRG_getEdgeData (pRG, &edgeData, pA->nodeId)
       ||   !jmdlRG_evaluateEdgeData (pRG, &startPoint, &edgeData, 0.0)
       ||   !jmdlRG_evaluateEdgeData (pRG, &endPoint, &edgeData, 1.0)
       )
       return numEdge;

    for (i = i0; i < i1; i++, prevPoint = currPoint)
        {
        pA = pRGIL->GetIntersectionPAt ((size_t)i);
        param = pA->param;
        jmdlRG_evaluateEdgeData (pRG, &currPoint, &edgeData, param);
        RGI_CLEARMASK (pA->type, RGI_MASK_ALL_PROCESSED_BITS);
        if (i > i0)
            {
            dist = currPoint.DistanceXY (prevPoint);
            /* If the current point AND THE PARAMETRIC MIDPOINT
                    are close to previous point,
                    force it to look like a match in parameter space.
                (Parametric midpoint test is important to allow self intersections)
            */
            if (dist < minGeomStep)
                {
                double d0, d1;
                DPoint3d midPoint;
                jmdlRG_evaluateEdgeData (pRG, &midPoint, &edgeData, 0.5 * (param + breakParam));
                d0 = midPoint.DistanceXY (prevPoint);
                d1 = midPoint.DistanceXY (currPoint);
                if (d0 < minGeomStep && d1 < minGeomStep)
                    breakParam = param + minParamStep;
                }
            }
        if  (param < break0
            || (param < break0g && (dist = startPoint.DistanceXY (currPoint)) < minGeomStep)
            )
            {

            if  (master0 < i0)
                master0 = i;
            else
                jmdlRGIL_mergeClustersOf (pRGIL, master0, i);

            RGI_SETMASK (pA->type, RGI_MASK_PROCESSED_START);
            }
        else if  (param >= break1
            || (param > break1g && (dist = endPoint.DistanceXY (currPoint)) < minGeomStep)
            )
            {

            if  (master1 < i0)
                master1 = i;
            else
                jmdlRGIL_mergeClustersOf (pRGIL, master1, i);

            RGI_SETMASK (pA->type, RGI_MASK_PROCESSED_END);
            }
        else
            {
            processedTypeMask = RGI_MASK_PROCESSED_SLAVE;
            if (master < i0 || param > breakParam)
                {
                master = i;
                processedTypeMask = RGI_MASK_PROCESSED_MASTER;
                }
            breakParam = param + minParamStep;

            if (master != i)
                jmdlRGIL_mergeClustersOf (pRGIL, master, i);

            RGI_SETMASK (pA->type, processedTypeMask);
            numEdge++;
            }
        }

    return  numEdge;
    }
/*---------------------------------------------------------------------------------**//**
* Create a chain of edges to replace parentEdgeNodeId.
* @param pChildNode0Id <= id of first child node.  MTG_NULL_NODEID if no replacements
* @param pChildNodeNId <= id of last child node.  MTG_NULL_NODEID if no replacements
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRGIL_createSubEdges
(
RG_Header                       *pRG,
RG_IntersectionList             *pIL,
MTGNodeId                       *pChildNode0Id,
MTGNodeId                       *pChildNodeNId,
int                             i0,
int                             i1,
int                             numSubEdge,
MTGNodeId                       parentEdgeNodeId,
bool                            noisy
)
    {
    int i, j;
    double param;
    MTGNodeId firstChildEdgeBaseId, currChildLeftId, currChildRightId, lastChildNodeId;
    MTGNodeId previousChildBaseId;
    int currVertexIndex;
    DPoint3d currPoint;
    RG_Intersection *pA = NULL;
    RG_EdgeData parentEdge;
    int masterIndex;
    int currType;
    double oldParam;
    int masterCurveIndex;
    RG_CurveId newCurveId;

    *pChildNode0Id = *pChildNodeNId = MTG_NULL_NODEID;

    if (numSubEdge == 1)
        {
        /* There should be RGI_MASK_PROCESSED_START and RGI_MASK_PROCESSED_END
            records (in that order).  Leave them all alone.
        */

        }
    else
        {
        /* Create the first subedge and remark all leading "start" nodes */
        jmdlRG_getEdgeData (pRG, &parentEdge, parentEdgeNodeId);
        masterCurveIndex = parentEdge.auxIndex;

        jmdlMTGGraph_createEdge (pRG->pGraph, &firstChildEdgeBaseId, &lastChildNodeId);

        if (noisy)
            GEOMAPI_PRINTF("    Replacing edge %d with new edge %d,%d vertices %d,%d\n",
                                parentEdge.nodeId[0],
                                firstChildEdgeBaseId, lastChildNodeId,
                                parentEdge.vertexIndex[0],
                                parentEdge.vertexIndex[1]);

        jmdlRG_setVertexIndex (pRG, firstChildEdgeBaseId,   parentEdge.vertexIndex[0]);
        jmdlRG_setVertexIndex (pRG, lastChildNodeId,        parentEdge.vertexIndex[1]);

        jmdlRG_setParentCurveIndex (pRG, firstChildEdgeBaseId,   masterCurveIndex);
        jmdlRG_setParentCurveIndex (pRG, lastChildNodeId,   masterCurveIndex);

        jmdlRG_setEdgeStartBit (pRG, firstChildEdgeBaseId, true);
        jmdlRG_setEdgeStartBit (pRG, lastChildNodeId, false);

        oldParam = 0.0;
        previousChildBaseId = firstChildEdgeBaseId;
        /* Reassign endpoint intersection records so they refer to the new edge */
        for (i = i0; i < i1 ; i++)
            {
            pA = pIL->GetIntersectionPAt ((size_t)i);
            if ((currType = RGI_GETPROCESSFIELD(pA->type)) == RGI_MASK_PROCESSED_START)
                {
                pA->param = 0.0;
                pA->mergedNodeId = firstChildEdgeBaseId;
                }
            else
                break;
            }
        /* Insert numSubEdge internal nodes */
        for (j = 1; j < numSubEdge; j++)
            {
            /* i,pA should be a master!! */
            if (pA && (currType = RGI_GETPROCESSFIELD(pA->type)) != RGI_MASK_PROCESSED_MASTER)
                return  false;

            if (pA == nullptr)
                {
                BeAssert(false && "NEEDS WORK STATIC ANALYSIS - This function is not prepared for pA to be null from here on.");
                return false;
                }

            param = pA->param;
            jmdlMTGGraph_splitEdge (pRG->pGraph, &currChildLeftId, &currChildRightId, previousChildBaseId);

            jmdlRG_findOrCreateVertexFromEdgeData (pRG,
                            &currPoint, &currVertexIndex,
                            &parentEdge,
                            param);

            if (noisy)
                {
                GEOMAPI_PRINTF("    Create vertex %d at param %lf from nodeId %d to %d, knuckle %d,%d\n",
                                currVertexIndex, param,
                                parentEdge.nodeId[0], parentEdge.nodeId[1],
                                currChildLeftId, currChildRightId);
                GEOMAPI_PRINTF("       xyz= %lf, %lf, %lf\n", currPoint.x, currPoint.y, currPoint.z);
                }

            jmdlRG_setVertexIndex (pRG, currChildLeftId, currVertexIndex);
            jmdlRG_setVertexIndex (pRG, currChildRightId, currVertexIndex);

            jmdlRG_setParentCurveIndex (pRG, currChildLeftId,  masterCurveIndex);
            jmdlRG_setParentCurveIndex (pRG, currChildRightId, masterCurveIndex);

            jmdlRG_setEdgeStartBit (pRG, currChildLeftId, true);
            jmdlRG_setEdgeStartBit (pRG, currChildRightId, false);

            if  (parentEdge.curveIndex != RG_NULL_CURVEID)
                {

                jmdlRG_linkToCreateSubcurve
                                        (
                                        pRG,
                                        &parentEdge,
                                        &newCurveId,
                                        oldParam,
                                        param
                                        );
                jmdlRG_setCurveIndex  (pRG, previousChildBaseId, newCurveId);
                jmdlRG_setCurveIndex  (pRG, currChildRightId,     newCurveId);
                }
            pA->mergedNodeId = currChildLeftId;
            masterIndex = i;
            previousChildBaseId = currChildLeftId;
            oldParam = param;

            /* Subsequent (slave) intersections as the same parameter are no longer involved */

            for (i++; i < i1 ;i++)
                {
                pA = pIL->GetIntersectionPAt ((size_t)i);
                if ((currType = RGI_GETPROCESSFIELD(pA->type)) == RGI_MASK_PROCESSED_SLAVE)
                    {
                    pA->mergedNodeId = MTG_NULL_NODEID;
                    jmdlRGIL_mergeClustersOf (pIL, masterIndex, i);
                    }
                else
                  break;
                }
            }

        param = 1.0;
        /* Point all trailing nodes at the new end */
        for (;i < i1 ;i++)
            {
            pA = pIL->GetIntersectionPAt ((size_t)i);
            if ((currType = RGI_GETPROCESSFIELD(pA->type)) == RGI_MASK_PROCESSED_END)
                {
                pA->mergedNodeId = lastChildNodeId;
                }
            else
                return  false;
            }

        /* Create and save subcurve for final segment */
        if  (parentEdge.curveIndex != RG_NULL_CURVEID)
            {

            jmdlRG_linkToCreateSubcurve
                                    (
                                    pRG,
                                    &parentEdge,
                                    &newCurveId,
                                    oldParam,
                                    param
                                    );
            jmdlRG_setCurveIndex  (pRG, previousChildBaseId, newCurveId);
            jmdlRG_setCurveIndex  (pRG, lastChildNodeId,     newCurveId);
            }

        *pChildNode0Id = firstChildEdgeBaseId;
        *pChildNodeNId = lastChildNodeId;
        }

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* Insert edge with given ends in place of the parent edge.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRGIL_replaceEdge
(
RG_Header                       *pRG,
RG_IntersectionList             *pIL,
MTGNodeId                       parentNode0Id,
MTGNodeId                       childNode0Id,
MTGNodeId                       childNode1Id
)
    {
    bool    funcStat = false;
    if (childNode0Id != MTG_NULL_NODEID && childNode1Id != MTG_NULL_NODEID)
        {
        /* Find the other end of the original edge */
        MTGNodeId parentNode1Id = jmdlMTGGraph_getEdgeMate (pRG->pGraph, parentNode0Id);
        /* Detach both ends of the original */
        MTGNodeId anchorNode0Id = jmdlMTGGraph_yankEdgeFromVertex (pRG->pGraph, parentNode0Id);
        MTGNodeId anchorNode1Id = jmdlMTGGraph_yankEdgeFromVertex (pRG->pGraph, parentNode1Id);
        /* Insert the replacement */
        jmdlMTGGraph_vertexTwist (pRG->pGraph, childNode0Id, anchorNode0Id);
        jmdlMTGGraph_vertexTwist (pRG->pGraph, childNode1Id, anchorNode1Id);
        funcStat = true;
        }
    return  funcStat;
    }

#define NUM_VERTEX_SORT_FUNC 5
#define NUM_SCALED_VERTEX_SORT_FUNC 3
typedef struct
    {
    MTGNodeId  nodeId;
    double      f[NUM_VERTEX_SORT_FUNC];
    } RG_VertexSort;

static int jmdlRGIL_compareAngles
(
const RG_VertexSort     *pA,
const RG_VertexSort     *pB
)
    {
    int i;
    double d;
    static double s_tolerance = 1.0e-8;
    static double s_angleTolerance = 1.0e-5;
    d = fabs (pA->f[0] - pB->f[0]);

    /* Special tolerancing for angles before
        proceeding to later lexical comparisons */
    if (d < s_angleTolerance
        || fabs (fabs (d) - msGeomConst_2pi) < s_angleTolerance)
        {
        /* Equal angles -- proceed to secondary */
        for (i = 1; i < NUM_VERTEX_SORT_FUNC; i++)
            {
            d = pA->f[i] - pB->f[i];

            if (d < -s_tolerance)
                return -1;
            if (d > s_tolerance)
                return  1;
            }
        return 0;
        }
    /* Fall out with clearly different angles -- simple algebraic comparison */
    if (pA->f[0] < pB->f[0])
        return -1;
    if (pA->f[0] > pB->f[0])
        return  1;

    return 0;
    }
void checkThisDerivative
(
int         i,
DPoint3d    *pDeriv,
DPoint3d    *pDiff,
double      step
)
    {
    DPoint3d approxDeriv, derivError;
    double mag = pDeriv->Magnitude ();
    double err;
    double relerr;
    approxDeriv.Scale (*pDiff, 1.0 / step);
    derivError.DifferenceOf (approxDeriv, *pDeriv);
    err = derivError.Magnitude ();
    if (err >= mag)
        relerr = 1.0;
    else
        relerr = err / mag;
    if (relerr > 5.0 * step)
        GEOMAPI_PRINTF("     D%d=%lf,%lf  delta=%lf,%lf  err = %lf\n", i,
                    pDeriv->x, pDeriv->y,
                    derivError.x, derivError.y,

                    relerr);
    }
void checkDerivatives
(
DPoint3d *pD0,
DPoint3d *pD1,
int         numDeriv,
double    epsilon
)
    {
    DPoint3d delta;
    int i, j;
    for (i = 1; i <= numDeriv; i++)
        {
        j = i - 1;
        delta.DifferenceOf (pD1[ j], pD0[ j]);
        checkThisDerivative (i, pD0 + i, &delta,  epsilon);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Set up sort parameters with a explicit sort function value.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         void        jmdlRG_initVertexSortData
(
RG_Header               *pRG,
RG_VertexSort           *pSortData,
MTGNodeId               nodeId,
double                  f0,
double                  f1,
double                  f2
)
    {
    RG_EdgeData edgeData;

    jmdlRG_getEdgeData (pRG, &edgeData, nodeId);

    pSortData->nodeId = nodeId;
    pSortData->f[0] = f0;
    pSortData->f[1] = f1;
    pSortData->f[2] = f2;
    if (edgeData.nodeId[0] < edgeData.nodeId[1])
        {
        pSortData->f[4] = (double)(edgeData.nodeId[0] + 1);
        }
    else
        {
        pSortData->f[4] = -(double)(edgeData.nodeId[1] + 1);
        }

    pSortData->f[3] = pSortData->f[4];
    }

/*---------------------------------------------------------------------------------**//**
* Set up the sort parameters for an edge from a vertex.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool    jmdlRGIL_setVertexSortParameters
(
RG_Header               *pRG,
RG_VertexSort           *pSortData,
RG_VertexSort           *pMagData,
int                     *pVertexIndex,
EmbeddedIntArray                *pNodeIdToClusterArray,
MTGNodeId               nodeId,
double                  paramOffset
)
    {
    DPoint3d dVec[4];
    double magT, magT2, magT3;
    double f, g, df, dg;
    int i;
    static double s_noisy = 0;
    bool    boolStat = false;
    RG_EdgeData edgeData;
    int farClusterIndex;
    static double s_tolFactor = 1.0e-3;
    double tangentTol = s_tolFactor * jmdlRG_getTolerance (pRG);

    pSortData->nodeId = nodeId;

    if (   jmdlRG_getVertexData (pRG, dVec, 3, pVertexIndex, nodeId, paramOffset)
        && jmdlRG_getEdgeData (pRG, &edgeData, nodeId))
        {
        pSortData->f[0] = Angle::Atan2 (dVec[1].y, dVec[1].x);
        magT2 = dVec[1].DotProductXY (dVec[1]);
        if (magT2 > tangentTol * tangentTol)
            {
            magT  = sqrt (magT2);
            magT3 = magT2 * magT;
            f = dVec[1].CrossProductXY (dVec[2]);

            pSortData->f[1] = f / magT3;
            df = dVec[1].CrossProductXY (dVec[3]);
            g  = magT3;
            dg = 3.0 * magT * dVec[1].DotProductXY (dVec[2]);

            pSortData->f[2] = (df * g - f * dg) / ( g * g * magT);

            /* If the derivatives all agree (as in identical edges)
               use far cluster id, then nodeId's and orientation
               to resolve.
            */
            if (jmdlEmbeddedIntArray_getInt (pNodeIdToClusterArray, &farClusterIndex, edgeData.nodeId[1]))
                    pSortData->f[3] = farClusterIndex;
            else
                    pSortData->f[3] = -1;

            if (edgeData.nodeId[0] < edgeData.nodeId[1])
                {
                pSortData->f[4] = (double)(edgeData.nodeId[0] + 1);
                }
            else
                {
                pSortData->f[4] = -(double)(edgeData.nodeId[1] + 1);
                }

            pSortData->f[3] = pSortData->f[4];
            boolStat = true;

            for (i = 0; i < 4; i++)
                {
                f = fabs (pSortData->f[i]);
                if (f > pMagData->f[i])
                    pMagData->f[i] = f;
                }
            }

#define CHECK_CURVATURE
#ifdef CHECK_CURVATURE
    if (s_noisy && fabs (pSortData->f[1]) > 0.0)
        {
        DPoint3d eVec[4];
        double theta, dThetaDS, rho;
        static double s_epsilon = 0.0001;
        double relErr;

        if (jmdlRG_getVertexData (pRG, eVec, 3, NULL, nodeId, s_epsilon))
            {
            checkDerivatives (dVec, eVec, 3, s_epsilon);
            theta = Angle::Atan2 (eVec[1].y, eVec[1].x);
            rho = sqrt (dVec[1].DotProductXY (dVec[1]));
            dThetaDS = (theta - pSortData->f[0]) / ( rho * s_epsilon);
            relErr = fabs (dThetaDS - pSortData->f[1])
                    / ( fabs (dThetaDS) + fabs (pSortData->f[1]));
            GEOMAPI_PRINTF(" node %d theta = %lf  cur = %lf dcur = %lf  %%relErr = %lf\n", nodeId,
                            pSortData->f[0],
                            pSortData->f[1],
                            dThetaDS,
                            relErr
                            );
            pSortData->f[1] = dThetaDS;

            }
    }
#endif
        }

    return boolStat;
    }
#ifdef CompileAll
/*---------------------------------------------------------------------------------**//**
* Set up the sort parameters from a vertex, using several guessed parameters to look
* for a nonzero tangent.
* Parameters are chosen as an initial grid of nTest evenly spaced parameters, then
* raised to a power to pull closer to 0.  For instance, with nTest = 3 and power 3 the initial
* parameters are 1/3, 2/3, 1.0, and these are cubed to produce parameters
* 1/81, 8/81, and 1, i.e. points bunced near zero.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool    jmdlRGIL_setDegenerateVertexSortParameters
(
RG_Header               *pRG,
RG_VertexSort           *pSortData,
RG_VertexSort           *pMagData,
int                     *pVertexIndex,
EmbeddedIntArray                *pNodeIdToClusterArray,
MTGNodeId               nodeId,
int                     nTest,
int                     testPower
)
    {
    double ds;
    double s, s0;
    int i, j;
    if (nTest < 2)
        nTest = 2;
    ds = 1.0 / nTest;
    for (i = 1; i <= nTest; i++)
        {
        s = s0 = i * ds;
        for (j = 1; j < testPower; j++)
            s *= s0;
        if (jmdlRGIL_setVertexSortParameters (pRG, pSortData,
                            pMagData, pVertexIndex,
                            pNodeIdToClusterArray,
                            nodeId, s))
            return true;
        }
    return false;
    }
 #endif
struct RGVertexSortArray : bvector <RG_VertexSort>
{

};

typedef RGVertexSortArray & RGVertexSortArrayR;
/*---------------------------------------------------------------------------------**//**
* Set up the sort parameters for an edge from a vertex.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         void    jmdlRGIL_scaleVertexSortParameters
(
RGVertexSortArrayR  vertexData,
RG_VertexSort           *pMagData
)
    {
    RG_VertexSort scaleData;

    for (size_t j = 0; j < NUM_SCALED_VERTEX_SORT_FUNC; j++)
        {
        scaleData.f[j] = pMagData->f[j] == 0.0 ? 1.0 : 1.0 / pMagData->f[j];
        }

    for (size_t i = 0; i < vertexData.size (); i++)
        {
        for (size_t j = 0; j < NUM_SCALED_VERTEX_SORT_FUNC; j++)
            vertexData[i].f[j] *= scaleData.f[j];
        }
    }


/*---------------------------------------------------------------------------------**//**
* @param smallAngleTol => there is a pair of outgoing edges within this tolerance
* @return a node at the vertex.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         MTGNodeId       jmdlRGIL_sortAndRebuildVertexLoop
(
RG_Header                       *pRG,
double                          *pSmallestAngleChange,
RGVertexSortArrayR              sortArray,
int                             consolidatedVertexIndex,
int                             noisy
)
    {
    MTGNodeId nodeId, outputNodeId, insertionNodeId = MTG_NULL_NODEID;
    RG_VertexSort currEdge;
    static int s_noisy = 0;
    double smallestAngle = 1.0e10;
    double previousAngle = 0.0;
    double deltaAngle;
    double currAngle;

    outputNodeId = MTG_NULL_NODEID;

    if (s_noisy > noisy)
        noisy = s_noisy;

    if (noisy)
        GEOMAPI_PRINTF("\n  VERTEX %d REASSEMBLY \n",
                consolidatedVertexIndex);

    size_t count = sortArray.size ();
    if (count > 0)
        {
        qsort (sortArray.data (), sortArray.size (), sizeof (RG_VertexSort), (VBArray_SortFunction)jmdlRGIL_compareAngles);

        /* Yank each edge from its current vertex and insert into the new one. */
        for (size_t i = 0; i < count; i++)
            {
            currEdge = sortArray[i];
            nodeId = currEdge.nodeId;
            currAngle = currEdge.f[0];
            jmdlRG_setVertexIndex (pRG, nodeId, consolidatedVertexIndex);
            jmdlMTGGraph_yankEdgeFromVertex (pRG->pGraph, nodeId);
            if (noisy)
                GEOMAPI_PRINTF("\n      %d %20.15le %lf %lf %lf %lf",
                        nodeId,
                        currEdge.f[0], currEdge.f[1],
                        currEdge.f[2], currEdge.f[3], currEdge.f[4]);
            if (i == 0)
                {
                outputNodeId = insertionNodeId = nodeId;
                }
            else
                {
                jmdlMTGGraph_vertexTwist (pRG->pGraph, nodeId, insertionNodeId);
                insertionNodeId = nodeId;
                deltaAngle = currAngle = previousAngle;
                if (i == 1 || deltaAngle < smallestAngle)
                    smallestAngle = deltaAngle;
                }
            }
        }
    if (pSmallestAngleChange)
        *pSmallestAngleChange = smallestAngle;
    return outputNodeId;
    }

/*---------------------------------------------------------------------------------**//**
* @return true if node has duplicate unresolvable tangents.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRGIL_checkTangentDegenerate
(
RG_Header                       *pRG,
RGVertexSortArrayR              sortArray,
int                             noisy
)
    {
    size_t n = sortArray.size ();
    /* f[0] is raw angle */
    for (size_t i = 0; i < n; i++)
        {
        for (size_t j = i + 1; j < n; j++)
            {
            if (bsiTrig_equalAngles (sortArray[i].f[0], sortArray[j].f[0]))
                return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Sort all the nodes in the given markset into a single vertex.
* @param pUnmergedNodeSet <=> evolving set showing nodes where local sorting failed.
* @param pNodesAtVertex <=> on input, contains ALL nodes being consolidated.
*       This markset is emptied during processing.
* @param pSortArray <=> scratch array for sort records.
*       Must be initialized and freed by caller.  Emptied and reused here.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         MTGNodeId       jmdlRGIL_sortAroundVertex
(
RG_Header                       *pRG,
MTG_MarkSet                     *pUnmergedNodeSet,
MTG_MarkSet                     *pNodesAtVertex,
RGVertexSortArrayR              sortArray,
EmbeddedIntArray               *pNodeIdToClusterArray,
int                             noisy
)
    {
    MTGNodeId nodeId;
    MTGNodeId outputNodeId = MTG_NULL_NODEID;
    int consolidatedVertexIndex = RG_NULL_VERTEXID;
    int currVertexIndex;
    RG_VertexSort   currEdge;
    RG_VertexSort   magEdge;
    int errors;
    static int s_alwaysDegenerate = 0;
    static int s_checkTangentDegenerate = 1;
    static double s_smallAngleTol = -1.0;
    double smallestAngle;
    static int s_noisy = 0;
    if (s_noisy > noisy)
        noisy = s_noisy;

    sortArray.clear ();
    errors = 0;
    memset (&magEdge, 0, sizeof(magEdge));
    for (;
        MTG_NULL_NODEID !=
            (nodeId = jmdlMTGMarkSet_chooseAndRemoveNode (pNodesAtVertex))
        ;)
        {
        if (!jmdlRGIL_setVertexSortParameters
                    (
                    pRG,
                    &currEdge,
                    &magEdge,
                    &currVertexIndex,
                    pNodeIdToClusterArray,
                    nodeId,
                    0.0)
            /* || jmdlRGIL_setDegenerateVertexSortParameters (pRG, &currEdge, &magEdge, &currVertexIndex, pNodeIdToClusterArray, nodeId, 20, 4) */
            )
            {
            errors ++;
            }

        sortArray.push_back (currEdge);

        if (consolidatedVertexIndex == RG_NULL_VERTEXID)
            consolidatedVertexIndex = currVertexIndex;
        }


    if  (   errors > 0
        ||  s_alwaysDegenerate
        ||  (   s_checkTangentDegenerate
             && jmdlRGIL_checkTangentDegenerate (pRG, sortArray, noisy))
        )
        {
        for (size_t i = 0; i < sortArray.size (); i++)
            {
            nodeId = sortArray[i].nodeId;
            jmdlMTGMarkSet_addNode (pUnmergedNodeSet, nodeId);
            }
        }
    else
        {
        jmdlRGIL_scaleVertexSortParameters (sortArray, &magEdge);
        outputNodeId = jmdlRGIL_sortAndRebuildVertexLoop
            (pRG, &smallestAngle, sortArray, consolidatedVertexIndex,
                        noisy > 1 ? noisy : 0);

        if (smallestAngle < s_smallAngleTol)
            {
            for (size_t i = 0; i < sortArray.size (); i++)
                {
                nodeId = sortArray[i].nodeId;
                jmdlMTGMarkSet_addNode (pUnmergedNodeSet, nodeId);
                }
            }
        }

    if (noisy)
        GEOMAPI_PRINTF ("\n    (end of vertex merge)\n");
    return  outputNodeId;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void            jmdlRGIL_print
(
RG_IntersectionList             *pIL,
const char                      *pTitle
)
        {
        RG_Intersection descr;
        int i;
        GEOMAPI_PRINTF("  Intersection Descriptors >>> %s\n", pTitle);
        GEOMAPI_PRINTF("    i  label type       nodeId param    clusterId mergedNodeId\n");
        for (i = 0; jmdlRGI_get (pIL, &descr, i);i++)
            {
            GEOMAPI_PRINTF( " %5d %5d %10x %5d %20.14lf %5d %5d\n",
                    i,
                    descr.label,
                    descr.type,
                    descr.nodeId,
                    descr.param,
                    descr.clusterIndex,
                    descr.mergedNodeId
                    );
            }
        }

/*---------------------------------------------------------------------------------**//**
* Primary step of jmdlRGMerge_mergeIntersections.  All memory allocation
* requiring cleanup are passed as parameters so this function can exit
* simply from any point.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRGMerge_mergeIntersections_go
(
RG_Header                       *pRG,
RG_IntersectionList             *pIL,
EmbeddedIntArray                        *pNodeIdToClusterArray,
RGVertexSortArrayR              sortArray,
MTG_MarkSet                     *pNodesAtVertex,
MTG_MarkSet                     *pUnmergedNodeSet,
MTGMask                         nullFaceMask,
double                          tolerance
)
    {
    bool    ok = true;
    int i0, i1;
    RG_Intersection descr0;
    static double minParamStep = 1.0e-12;   /* Yuck.  Geom tol is doing better than param lately. */
    int numEdges;
    int clusterIndex;
    MTGNodeId parentNodeId;
    MTGNodeId childNodeId[2];
    static int  s_noisy = 0;
    MTGNodeId nodeId;
    int s_mergeMethod = 1;

    if (s_noisy > 0)
        GEOMAPI_PRINTF(" ********* MERGE ********** \n");

    if (s_noisy > 0)
        jmdlRGIL_print(pIL, " pre merge ");

    jmdlRGIL_sortByNodeIdAndParameter (pIL);

    if (s_noisy > 1)
        jmdlRGIL_print(pIL, " after initial edge sort");
    /* Pass 1: Merge coincident intersection clusters within each edge */
    for (i0 = 0; jmdlRGI_get (pIL, &descr0, i0);)
        {
        if (jmdlRG_checkAbort (pRG))
            return false;
        parentNodeId = descr0.nodeId;
        /* Find the range of descriptors that apply to this edge */
        i1 = jmdlRGIL_findUpperIndexByNodeId (pRG, pIL, i0);
        /* Assign subdivision parameters */
        if (s_noisy > 2)
            GEOMAPI_PRINTF(" descr range: %d..%d node %d\n", i0, i1, descr0.nodeId);
        numEdges = jmdlRGIL_markDistinctParameters (pRG, pIL, i0, i1, minParamStep, tolerance);
        i0 = i1;
        }

    if (s_noisy > 1)
        jmdlRGIL_print(pIL, " after parameter consolidation (sorted by base edge");

    /* Pass 2: Split edges */
    for (i0 = 0; jmdlRGI_get (pIL, &descr0, i0);)
        {
        if (jmdlRG_checkAbort (pRG))
            return false;
        parentNodeId = descr0.nodeId;
        /* Find the range of descriptors that apply to this edge */
        i1 = jmdlRGIL_findUpperIndexByNodeId (pRG, pIL, i0);
        numEdges = jmdlRGIL_countSubEdges (pRG, pIL, i0, i1);
        /* Split the edge */
        if (!jmdlRGIL_createSubEdges (pRG, pIL, &childNodeId[0], &childNodeId[1], i0, i1, numEdges, parentNodeId, s_noisy > 2))
            return  false;
        if (   numEdges > 1
            && jmdlRGIL_replaceEdge (pRG, pIL, parentNodeId, childNodeId[0], childNodeId[1])
           )
            jmdlRG_dropEdge (pRG, parentNodeId);

        i0 = i1;
        }

    if (s_noisy > 1)
        jmdlRGIL_print(pIL, " after edge split (sorted by base edge");

    /* Pass 3: Get master cluster index for each intersection */
    for (i0 = 0; jmdlRGI_get (pIL, &descr0, i0); i0++)
        {
        if (jmdlRG_checkAbort (pRG))
            return false;
        jmdlRGIL_resolveClusterOf (pIL, i0);
        }

    if (s_noisy > 1)
        jmdlRGIL_print(pIL, " after cluster consolidation");

    if (s_mergeMethod == 1)
        {
        jmdlRG_clusterByXY (pRG, tolerance, nullFaceMask, s_noisy);
        }
    else
        {
        /* Pass 3a: Build an index from nodeid back to cluster index. */

        jmdlRGIL_sortByClusterId (pIL);
        jmdlEmbeddedIntArray_setConstant(pNodeIdToClusterArray, -1, jmdlMTGGraph_getNodeIdCount(pRG->pGraph));

        for (i0 = 0; jmdlRGI_get (pIL, &descr0, i0); i0++)
            {
            if (jmdlRG_checkAbort (pRG))
                return false;
            clusterIndex = descr0.clusterIndex;
            nodeId = descr0.mergedNodeId;
            if (nodeId != MTG_NULL_NODEID)
                jmdlEmbeddedIntArray_setInt(pNodeIdToClusterArray, clusterIndex, nodeId);
            }


        /* Pass 4: Merge clusters. */

        for (i0 = 0; jmdlRGI_get (pIL, &descr0, i0);)
            {
            int i;
            if (jmdlRG_checkAbort (pRG))
                return false;
            clusterIndex = descr0.clusterIndex;
            /* Find the range of descriptors that apply to this edge */
            i1 = jmdlRGIL_findUpperIndexByClusterIndex (pRG, pIL, i0);
            /* Sort all the edges of the cluster into a single vertex */
            if (s_noisy > 1)
                {
                GEOMAPI_PRINTF(" Cluster [%d,%d]\n", i0, i1);
                GEOMAPI_PRINTF("     i\t cluster\ttype\t\tnode\n");
                }
            jmdlMTGMarkSet_empty (pNodesAtVertex);

            for (i = i0; i < i1; i++)
                {
                RG_Intersection descr1;
                jmdlRGI_get (pIL, &descr1, i);
                if (s_noisy > 2)
                    GEOMAPI_PRINTF(" %5d\t%d\t%10x\t%d\n", i, descr1.clusterIndex, descr1.type, descr1.mergedNodeId);
                if  (   descr1.mergedNodeId == MTG_NULL_NODEID
                     && RGI_GETPROCESSFIELD(descr1.type) != RGI_MASK_PROCESSED_SLAVE
                    )
                    {
                    if (s_noisy > 0)
                        GEOMAPI_PRINTF(" **** ERROR: Merged node not assigned. NodeId %d type(hex) %x\n",

                                    descr1.mergedNodeId,
                                    RGI_GETPROCESSFIELD(descr1.type)
                                    );
                    }
                if (RGI_GETPROCESSFIELD(descr1.type) != RGI_MASK_PROCESSED_SLAVE)
                    {
                    MTGARRAY_VERTEX_LOOP (currNodeId, pRG->pGraph, descr1.mergedNodeId)
                        {
                        jmdlMTGMarkSet_addNode (pNodesAtVertex, currNodeId);
                        }
                    MTGARRAY_END_VERTEX_LOOP (currNodeId, pRG->pGraph, descr1.mergedNodeId)
                    }
                }

            jmdlRGIL_sortAroundVertex
                    (pRG,
                    pUnmergedNodeSet,
                    pNodesAtVertex,
                    sortArray,
                    pNodeIdToClusterArray,
                    s_noisy > 2
                    );

            i0 = i1;
            }

        if (s_noisy > 0)
            jmdlRGIL_print(pIL, " final");
        }
    return  ok;
    }

/*---------------------------------------------------------------------------------**//**
* Determine the parameter at which the node appears on its curve.
* In the param array, find the (index of the) nearest parameter.
* Remove all except that index from both the parameter and point arrays.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void filterMultipleIntersections
(
RG_Header           *pRG,
MTGNodeId           nodeId,
bvector<double> *pParamArray,
EmbeddedDPoint3dArray *pPointArray
)
    {
    size_t numIntersection = pParamArray->size();
    bool    isReversed = false;
    size_t outIndex, currIndex;
    DPoint3d outPoint;
    double minDelta, currDelta;
    double nodeParameter, currParameter;

//    static double paramOffset = 1.0e-6;
    if  (  numIntersection > 1
        && jmdlRG_getCurveData (pRG, NULL, &isReversed, NULL, NULL, nodeId)
        )
        {
        outIndex = -1;
        nodeParameter = isReversed ? 1.0 : 0.0;
        minDelta = 1.0e10;
        for (currIndex = 0; currIndex < numIntersection; currIndex++)
            {
            currParameter = pParamArray->at(currIndex);
            currDelta = fabs (currParameter - nodeParameter);
            if (currIndex == 0 || currDelta < minDelta)
                outIndex = currIndex;
            }

        currParameter = pParamArray->at (outIndex);
        pParamArray->clear();
        pParamArray->push_back (currParameter);

        outPoint = pPointArray->at(outIndex);
        pPointArray->clear ();
        pPointArray->push_back (outPoint);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Internal version -- parent performs all grab/drop ops.  This function can "return"
*   at any point.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRGMerge_mergeDegenerateIntersectionsGo
(
RG_Header               *pRG,
MTG_MarkSet             *pUnmergedNodes,
RGVertexSortArrayR      sortArray,
double                  rTest,
int                     noisy,
bvector<double>         *pParamArray,
EmbeddedDPoint3dArray   *pPointArray
)
    {
    double rr = rTest * rTest;
    bool    boolstat = true;
    MTGNodeId nodeId;
    DPoint3d xyz0, xyz1;
    double rr01, rCircle;
    RG_VertexSort   sortData;
    int vertexId;
    int consolidatedVertexId = -1;
    int iteratorIndex;
    MTGNodeId currNodeId;
    static double s_circleFactor = 1.000000001;
    //static double s_smallAngleTol = 0.0;
    static double angleTol = 1.0e-8;
    static int s_noisy = 0;
    MTG_MarkSet deleteSet (pRG->pGraph, MTG_ScopeEdge);

    if (s_noisy > noisy)
        noisy = s_noisy;

    if (noisy)
        GEOMAPI_PRINTF (" **********  Degenerate merge test radius %lf\n", rTest);
    rr = rTest * rTest;
    rCircle = s_circleFactor * rTest;

    for (;
        MTG_NULL_NODEID !=
            (nodeId = jmdlMTGMarkSet_chooseAndRemoveNode (pUnmergedNodes))
        ;)
        {
        if (jmdlRG_checkAbort(pRG))
            return false;
        if (jmdlMTGMarkSet_isNodeInSet (&deleteSet, nodeId))
            continue;


        jmdlRG_getVertexData (pRG, &xyz0, 0, &vertexId, nodeId, 0.0);
        currNodeId = nodeId;
        jmdlMTGMarkSet_initIteratorIndex (pUnmergedNodes, &iteratorIndex);
        if (noisy > 1)
            GEOMAPI_PRINTF ("\n ***** DEGENERATE VERTEX ***** \n");
        sortArray.clear ();
        consolidatedVertexId = -1;
        /* Linear scan of all remaining degenerates. */
        do
            {
            if (jmdlMTGMarkSet_isNodeInSet (&deleteSet, currNodeId))
                continue;

            jmdlRG_getVertexData (pRG, &xyz1, 0, &vertexId, currNodeId, 0.0);
            rr01 = xyz0.DistanceSquaredXY (xyz1);
            if (rr01 < rr)
                {
                double targetParam;
                DPoint3d targetPoint;
                DPoint3d targetVector;
                double angle;
                if (noisy > 1)
                    GEOMAPI_PRINTF
                    ("   node %d vertex %d %le,%le,%le r=%le\n",
                    currNodeId, vertexId, xyz1.x, xyz1.y, xyz1.z,
                    sqrt (rr01));
                jmdlMTGMarkSet_removeNode (pUnmergedNodes, currNodeId);
                jmdlRG_edgeCircleXYIntersection (pRG,
                        pParamArray, pPointArray, currNodeId, &xyz0, rCircle);
                filterMultipleIntersections (pRG, currNodeId, pParamArray, pPointArray);
                auto numIntersection = pParamArray->size();

                if (numIntersection != 1)
                    {
                    jmdlMTGMarkSet_addNode (&deleteSet, currNodeId);
                    if (noisy)
                        GEOMAPI_PRINTF (" ******** intersection count = %d\n", (int)numIntersection);
                    }
                else
                    {
                    targetPoint = pPointArray->front ();
                    targetParam = pParamArray->front ();
                    targetVector.DifferenceOf (targetPoint, xyz0);
                    angle = Angle::Atan2 (targetVector.y, targetVector.x);

                    if (fabs(fabs (angle) - msGeomConst_pi) < angleTol)
                        angle = msGeomConst_pi;

                    jmdlRG_initVertexSortData (pRG, &sortData, currNodeId, angle, 0.0, 0.0);
                    sortArray.push_back (sortData);
                    if (consolidatedVertexId < 0)
                        consolidatedVertexId = vertexId;
                    if (noisy > 1)
                        GEOMAPI_PRINTF("                    Target param %le angle %le\n",
                                        targetParam, angle);
                    }
                }
            } while (jmdlMTGMarkSet_getNextNode
                        (pUnmergedNodes, &iteratorIndex, &currNodeId));

            jmdlRGIL_sortAndRebuildVertexLoop
                    (pRG, NULL, sortArray, consolidatedVertexId,
                            noisy > 1 ? noisy : 0);

        }


    for (;
        MTG_NULL_NODEID !=
            (nodeId = jmdlMTGMarkSet_chooseAndRemoveNode (&deleteSet))
        ;)
        {
        jmdlMTGGraph_dropEdge (pRG->pGraph, nodeId);
        }


    return boolstat;
    }


/*---------------------------------------------------------------------------------**//**
* Merge degenerate intersection conditions
* @param pAllCandidateNodeIds <=> set of all nodes at degenerate vertices.
* @param pVertexSortArray <=> scratch array for sorting.
* @param rTest => radius at which sorting is carried out.
*
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRGMerge_mergeDegenerateIntersections
(
RG_Header               *pRG,
MTG_MarkSet             *pUnmergedNodes,
RGVertexSortArrayR      sortArray,
double                  rTest,
int                     noisy
)
    {
    bvector<DPoint3d> pointArray;
    bvector<double> paramArray;

    bool    result = jmdlRGMerge_mergeDegenerateIntersectionsGo
                        (pRG, pUnmergedNodes, sortArray, rTest, noisy, &paramArray, &pointArray);
    return result;
    }
/*---------------------------------------------------------------------------------**//**
* Merge precomputed array of curve-curve intersections in the graph.
*
* @param An array for efficient marking of active nodes.
* @return true if intersections computed successfully.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRGMerge_mergeIntersections
(
RG_Header                       *pRG,
RG_IntersectionList             *pIL,
MTGMask                         nullFaceMask
)
    {
    bool    boolstat = false;
    EmbeddedIntArray *pNodeIdToClusterArray = jmdlEmbeddedIntArray_grab ();
    RGVertexSortArray sortArray;
    static double s_degenerateNodeFactor = 1000.0;
    static double s_vertexToleranceFactor =  10.0;
    static int s_noisy = 0;
    double baseTolerance = jmdlRG_getTolerance (pRG);
    double rTest = s_degenerateNodeFactor * baseTolerance;
    double vertexTolerance = s_vertexToleranceFactor * baseTolerance;

    int noisy = jmdlRG_getNoisy ();

    if (s_noisy > noisy)
            noisy = s_noisy;

    MTG_MarkSet nodesAtVertex (pRG->pGraph, MTG_ScopeNode);
    MTG_MarkSet unmergedNodes (pRG->pGraph, MTG_ScopeNode);

    boolstat = jmdlRGMerge_mergeIntersections_go
                (
                pRG,
                pIL,
                pNodeIdToClusterArray,
                sortArray,
                &nodesAtVertex,
                &unmergedNodes,
                nullFaceMask,
                vertexTolerance
                );

    boolstat = jmdlRGMerge_mergeDegenerateIntersections
                (pRG, &unmergedNodes, sortArray, rTest, noisy);

    jmdlEmbeddedIntArray_drop (pNodeIdToClusterArray);


    jmdlRG_fixTopology (pRG);
    return boolstat;
    }
/* Triforma wants to remove V139 logic to get a build with V1.38-to-1.39 diffs.
   Undefine V139 to get that */
#define V139
#ifdef V139
/*---------------------------------------------------------------------------------**//**
* Add gap edges between a particular pair of nodes.  NO TESTING -- just do it.
* @param nodeId0 =>
* @param nodeId1 =>
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRGMerge_resolveVisibleSector
(
RG_Header       *pRG,
MTGNodeId       *pVisibleSectorNodeId,
MTGNodeId       seedNodeId,
const DPoint3d  *pXYZ
)
    {
    MTGNodeId previousNodeId = jmdlMTGGraph_getVPred (pRG->pGraph, seedNodeId);
    double paramOffset = 1.0e-8;
    static DPoint3d upVector = {0.0, 0.0, 1.0};
    DPoint3d derivatives[4];
    DPoint3d tangent0, tangent1;
    DPoint3d testVector;
    DPoint3d refPoint;
    *pVisibleSectorNodeId = MTG_NULL_NODEID;

    /* NEEDS WORK: test for ON edge case */
    if (seedNodeId == previousNodeId)
        {
        *pVisibleSectorNodeId = seedNodeId;
        return true;
        }

    if (   jmdlRG_getVertexData (pRG, derivatives, 3, NULL, previousNodeId, paramOffset))
        {
        tangent0 = derivatives[1];
        refPoint = derivatives[0];
        testVector.DifferenceOf (*pXYZ, refPoint);
        /* NEEDS WORK: consider curvature case */
        MTGARRAY_VERTEX_LOOP (currNodeId, pRG->pGraph, seedNodeId)
            {
            if (!jmdlRG_getVertexData (pRG, derivatives, 3, NULL, currNodeId, paramOffset))
                {
                break;
                }
            tangent1 = derivatives[1];
            if (testVector.IsVectorInCCWSector (tangent0, tangent1, upVector))
                {
                *pVisibleSectorNodeId = previousNodeId;
                return true;
                }
            previousNodeId = currNodeId;
            tangent0       = tangent1;
            }
        MTGARRAY_END_VERTEX_LOOP (currNodeId, pRG->pGraph, seedNodeId)
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Add gap edges between a particular pair of nodes.  NO TESTING -- just do it.
* @param nodeId0 =>
* @param nodeId1 =>
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRGMerge_joinNodes
(
RG_Header       *pRG,
MTGNodeId       nodeId0,
MTGNodeId       nodeId1
)
    {
    MTGNodeId nodeId2, nodeId3, resolvedNodeId0, resolvedNodeId1;
    RG_EdgeData edgeData0, edgeData1;

    if (   jmdlRG_getEdgeData (pRG, &edgeData0, nodeId0)
        && jmdlRG_getEdgeData (pRG, &edgeData1, nodeId1)
        )
        {
        if (   jmdlRGMerge_resolveVisibleSector (pRG, &resolvedNodeId0, nodeId0, &edgeData1.xyz[0])
            && jmdlRGMerge_resolveVisibleSector (pRG, &resolvedNodeId1, nodeId1, &edgeData0.xyz[0])
           )
            {
            jmdlMTGGraph_createEdge (pRG->pGraph, &nodeId2, &nodeId3);

            jmdlRG_setVertexIndex (pRG, nodeId2,            edgeData0.vertexIndex[0]);
            jmdlRG_setVertexIndex (pRG, nodeId3,            edgeData1.vertexIndex[0]);

            jmdlRG_setParentCurveIndex (pRG, nodeId2,   -1);
            jmdlRG_setParentCurveIndex (pRG, nodeId3,   -1);

            jmdlRG_setCurveIndex (pRG, nodeId2, -1);
            jmdlRG_setCurveIndex (pRG, nodeId3, -1);

            jmdlRG_setEdgeStartBit (pRG, nodeId2, true);
            jmdlRG_setEdgeStartBit (pRG, nodeId3, false);

            GEOMAPI_PRINTF (" Pre-twist face sizes: %d %d\n",
                        (int)pRG->pGraph->CountNodesAroundFace (resolvedNodeId0),
                        (int)pRG->pGraph->CountNodesAroundFace (resolvedNodeId1)
                        );

            jmdlMTGGraph_vertexTwist (pRG->pGraph, resolvedNodeId0, nodeId2);
            jmdlMTGGraph_vertexTwist (pRG->pGraph, resolvedNodeId1, nodeId3);
            GEOMAPI_PRINTF (" Post-twist face sizes: %d %d\n",
                        (int)pRG->pGraph->CountNodesAroundFace (nodeId2),
                        (int)pRG->pGraph->CountNodesAroundFace (nodeId3)
                        );
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Add gap edges between a particular pair of nodes.  Minimal testing
*   for validity conditions.
* @param nodeId0 =>
* @param nodeId1 =>
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRGMerge_joinNodesConditional
(
RG_Header       *pRG,
MTGNodeId       nodeId0,
MTGNodeId       nodeId1,
int             noisy
)
    {
    RG_EdgeData edgeData0, edgeData1;
    if (noisy)
        GEOMAPI_PRINTF (" gap candidates: %d %d\n", nodeId0, nodeId1);
    if (   jmdlRG_getEdgeData (pRG, &edgeData0, nodeId0)
        && jmdlRG_getEdgeData (pRG, &edgeData1, nodeId1))
        {
        double distance = edgeData0.xyz[0].DistanceXY (*(&edgeData1.xyz[0]));
        double tolerance        = jmdlRG_getTolerance (pRG);
        if (noisy)
            GEOMAPI_PRINTF ("  distance = %lf\n", distance);
        jmdlRG_debugSegment (pRG, &edgeData0.xyz[0], &edgeData1.xyz[0], 10, 4, RG_NORMALDRAW);
        if (distance <= tolerance)
            return false;
        }
    return jmdlRGMerge_joinNodes (pRG, nodeId0, nodeId1);
    }


/*---------------------------------------------------------------------------------**//**
* Add gap edges between clustered nodes.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static         bool            jmdlRGMerge_joinBlockedVertexClusters
(
RG_Header           *pRG,
EmbeddedIntArray    *pBlockedNodeIdArray,
int                 noisy
)
    {
    int i0, i1;
    int nodeId;
    for (i0 = i1 = 0;
        jmdlEmbeddedIntArray_getInt (pBlockedNodeIdArray, &nodeId, i1);
        i1++)
        {
        /* Isolate the special cases we are prepared to handle ... */
        if (nodeId < 0)
            {
            if (i1 - i0 == 2)
                {
                MTGNodeId nodeId0, nodeId1;
                jmdlEmbeddedIntArray_getInt (pBlockedNodeIdArray, &nodeId0, i0);
                jmdlEmbeddedIntArray_getInt (pBlockedNodeIdArray, &nodeId1, i0 + 1);
                jmdlRGMerge_joinNodesConditional (pRG, nodeId0, nodeId1, noisy);
                }
            i0 = i1 + 1;
            }
        }
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* Add gap edges to merged geometry.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRGMerge_closeGaps
(
RG_Header       *pRG,
double          vertexVertexTolerance,
double          vertexEdgeTolerance
)
    {
    EmbeddedIntArray *pNodeToVertexLoopArray = jmdlEmbeddedIntArray_grab ();
    EmbeddedIntArray *pBlockIndexArray = jmdlEmbeddedIntArray_grab ();
    EmbeddedDPoint3dArray *pXYZArray = jmdlEmbeddedDPoint3dArray_grab ();
    MTGGraph *pGraph = pRG->pGraph;
    DPoint3d xyz;
    int numSort;
    MTGNodeId nodeId;
    int numCluster;
    int i, k;
    static int s_noisy = 0;

    pGraph->CollectVertexLoops (*pNodeToVertexLoopArray);

    if (vertexVertexTolerance > 0.0)
        {
        for (numSort = 0;
               jmdlEmbeddedIntArray_getInt (pNodeToVertexLoopArray, &nodeId, numSort)
            && jmdlRG_getVertexDPoint3d (pRG, &xyz, nodeId)
            && jmdlEmbeddedDPoint3dArray_addDPoint3d (pXYZArray, &xyz);
            numSort++)
            {
            }

        numCluster = jmdlVArrayDPoint3d_identifyMatchedVertices (pXYZArray, NULL, pBlockIndexArray,
                        vertexVertexTolerance, 0.0);

        if (s_noisy)
            GEOMAPI_PRINTF  (
                    " Gap tol %lf Vertex count %d   cluster count %d\n",
                    vertexVertexTolerance,
                    jmdlEmbeddedDPoint3dArray_getCount (pXYZArray),
                    numCluster
                    );

        /* Renumber the block indices so they go directly to nodes */
        for (i = 0; jmdlEmbeddedIntArray_getInt (pBlockIndexArray, &k, i); i++)
            {
            if (k >= 0)
                {
                jmdlEmbeddedIntArray_getInt (pNodeToVertexLoopArray, &nodeId, k);
                jmdlEmbeddedIntArray_setInt (pBlockIndexArray, nodeId, i);
                }
            }

        jmdlRGMerge_joinBlockedVertexClusters (pRG, pBlockIndexArray, s_noisy);
        }

    jmdlEmbeddedDPoint3dArray_drop (pXYZArray);
    jmdlEmbeddedIntArray_drop (pNodeToVertexLoopArray);
    jmdlEmbeddedIntArray_drop (pBlockIndexArray);
    return true;
    }
#endif /* V139 compiler block */
/*---------------------------------------------------------------------------------**//**
* Search for connected components contained in others.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool            jmdlRGMerge_connectHolesToParents
(
RG_Header                       *pRG
)
    {
    return true;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
