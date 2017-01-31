/*--------------------------------------------------------------------------------------+
|
|     $Source: vu/src/vufilter.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#include <math.h>
/* Small feature removal */

/*----------------------------------------------------------------------+
| name          mdlVec2d_magnitudeSquared                               |
| author        EarlinLutz                                      10/95   |
+----------------------------------------------------------------------*/
static double mdlVec2d_magnitudeSquared
(
DPoint2d        *vecP           /* vector whose length is to be computed */
)
    {
    return vecP->x * vecP->x + vecP->y * vecP->y;
    }

/*----------------------------------------------------------------------+
| name          mdlVec2d_crossProduct                                   |
| author        EarlinLutz                                      10/95   |
+----------------------------------------------------------------------*/
static double mdlVec2d_crossProduct
(
DPoint2d        *vec0P,         /* First vector */
DPoint2d        *vec1P          /* Second vector */
)
    {
    return vec0P->x * vec1P->y - vec0P->y * vec1P->x;
    }

/*----------------------------------------------------------------------+
| name          mdlVec2d_dotProduct                                     |
| author        EarlinLutz                                      10/95   |
+----------------------------------------------------------------------*/
static double mdlVec2d_dotProduct
(
DPoint2d        *vec0P,         /* First vector */
DPoint2d        *vec1P          /* Second vector */
)
    {
    return vec0P->x * vec1P->x + vec0P->y * vec1P->y;
    }

/*----------------------------------------------------------------------+
| name          mdlVec2d_distanceSquared                                |
| author        EarlinLutz                                      10/95   |
+----------------------------------------------------------------------*/
static double mdlVec2d_distanceSquared
(
DPoint2d        *vec1P,         /* vector whose length is to be computed */
DPoint2d        *vec2P
)
    {
    double dx = vec2P->x - vec1P->x;
    double dy = vec2P->y - vec1P->y;
    return dx * dx + dy * dy;
    }

/*----------------------------------------------------------------------+
| name          mdlVec2d_addScaledPoint                                 |
| author        EarlinLutz                                      10/95   |
+----------------------------------------------------------------------*/
static void mdlVec2d_addScaledPoint
(
DPoint2d        *resultP,               /* <= point0 + point1 * s */
DPoint2d        *point0P,               /* => start point */
DPoint2d        *point1P,               /* => offset vector */
double          s                       /* => scale factor */
)
    {
    resultP->x = point0P->x + point1P->x * s;
    resultP->y = point0P->y + point1P->y * s;
    }


/*----------------------------------------------------------------------+
| name          mdlVec2d_intersectRays                                  |
| author        EarlinLutz                                      10/95   |
+----------------------------------------------------------------------*/
static int mdlVec2d_intersectRays
(
DPoint2d        *intersectionP, /* <= intersection point */
double          *s0P,           /* <= parametric coordinate on ray 0 */
double          *s1P,           /* <= parametric coordinate on ray 1 */
DPoint2d        *start0P,       /* => start of first ray */
DPoint2d        *vec0P,         /* => first vector */
DPoint2d        *start1P,       /* => start of second ray */
DPoint2d        *vec1P,         /* => second vector */
double          bigParam        /* => A number beyond which parameters are meaningless */
)
    {
    double determinant  = mdlVec2d_crossProduct (vec0P, vec1P);
    double limit = fabs(determinant) * bigParam;
    double det0, det1;
    DPoint2d displacement;
    int status = ERROR;
    displacement.x = start1P->x - start0P->x;
    displacement.y = start1P->y - start0P->y;

    det0 = mdlVec2d_crossProduct (&displacement, vec1P);
    det1 = mdlVec2d_crossProduct (vec0P, &displacement);

    if (fabs(det0) < limit && fabs(det1) < limit)
        {
        status = SUCCESS;
        *s0P =   det0 / determinant;
        *s1P = - det1 / determinant;
        mdlVec2d_addScaledPoint (intersectionP, start0P, vec0P, *s0P);
        }
    return status;
    }

/*----------------------------------------------------------------------+
| name          mdlVec2d_projectPointToLine                             |
| author        EarlinLutz                                      10/95   |
+----------------------------------------------------------------------*/
static int mdlVec2d_projectPointToLine
(
DPoint2d        *projectionP,   /* <= projection of point on line */
double          *sP,            /* <= parametric coordinate along line */
DPoint2d        *pointP,        /* => point to project */
DPoint2d        *startP,        /* => line start */
DPoint2d        *endP           /* => line end */
)
    {
    double dot1, dot2;
    DPoint2d vec0, vec1;

    static double bigParam = 1.0e14;
    int status = ERROR;

    vec0.x = endP->x - startP->x;
    vec0.y = endP->y - startP->y;

    vec1.x = pointP->x - startP->x;
    vec1.y = pointP->y - startP->y;

    dot1 = mdlVec2d_dotProduct (&vec0, &vec1);
    dot2 = mdlVec2d_dotProduct (&vec0, &vec0);

    if (dot1 < bigParam * dot2)
        {
        *sP = dot1 / dot2;
        status = SUCCESS;
        }
    else
        {
        *sP = 0.0;
        }
    mdlVec2d_addScaledPoint (projectionP, startP, &vec0, *sP);
    return status;
    }

/*----------------------------------------------------------------------+
| name          mdlVec2d_interpolate                                    |
| author        EarlinLutz                                      10/95   |
| set resultP to the point interpolated between point0P and point1P at  |
| parametric (fractional) position s.                                   |
+----------------------------------------------------------------------*/
static void mdlVec2d_interpolate
(
DPoint2d        *resultP,               /* <= point0 + point1 * s */
const DPoint2d  *point0P,               /* => start point */
const double    s,                      /* => interpolation parameter */
const DPoint2d  *point1P                /* => end point */
)
    {
    double s0 = 1.0 - s;
    resultP->x = s0 * point0P->x + s * point1P->x;
    resultP->y = s0 * point0P->y + s * point1P->y;
    }

/*---------------------------------------------------------------------------------**//**
* @description Disconnect small edge <em>features</em> from the graph and mark them.
* @remarks On return, the marked edges remain in the graph, but are disconnected from the graph.  This allows
*       the caller to mark more edges with this mask before deleting all masked edges at once.  Thus it is
*       assumed that on input, discardMask, if not cleared in the graph, has only been applied to both or neither
*       sides of each edge of the graph, and that any edges with this mask are disconnected from the rest of the graph.
* @remarks This function searches for successive nodes A, B, C and D in a face such that:
* <ul>
* <li>AB and CD have a well-defined intersection E.</li>
* <li>The triangle BEC is small.</li>
* <li>Cutting out BC and leaving edges AE and ED only changes the area by a small amount.</li>
* </ul>
* @param graphP         IN OUT  graph to examine
* @param tol            IN      absolute xy-distance tolerance for the perturbation of nodes B and C to E
* @param discardMask    IN      mask to apply to discarded edges
* @return number of edges marked and disconnected from the graph
* @group "VU Edges"
* @see vu_markAndExciseSmallEdges, vu_testAndExciseSmallEdges
* @bsimethod                                                    EarlinLutz      10/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_markAndExciseSliverEdges
(
VuSetP          graphP,
double          tol,
VuMask          discardMask
)
    {
    VuP mateP;
    DPoint2d vec12, vec23, vec10;
    DPoint2d vec11;
    double d01, d23;
    double s10, s23;
    double d1P, d2P;
    double dev2 = tol*tol;
    int deleteThisSegment;
    DPoint2d vecInt;            /* Intersection of two edges surrounding the
                                        candidate */
    DPoint2d vecProj;           /* Projection of vecInt on candidate */
    double sProj, distProj;
    int nDiscard = 0;

    VuMask visitMask     = vu_grabMask (graphP);

    VuMask skipMask = discardMask | visitMask;
    VuArrayP candidateArrayP = vu_grabArray (graphP);
    static double maxHitParam = 0.45;
    static double bigParam = 1000.0;
    VuP free1P, free2P;

    vec11.x = 0.0;
    vec11.y = 0.0;

    vu_clearMaskInSet (graphP, visitMask);

    VU_SET_LOOP (startP, graphP)
        {
        VuP node0P = VU_FPRED(startP);
        VuP node1P = startP;
        VuP node2P = VU_FSUCC(node1P);
        VuP node3P = VU_FSUCC(node2P);
        mateP = VU_EDGE_MATE(node1P);

        if (   !VU_GETMASK(node1P, skipMask)
            && !VU_GETMASK(mateP,  skipMask)
            && vu_countEdgesAroundVertex (node1P) == 2
            && vu_countEdgesAroundVertex (node2P) == 2
            )
            {
            VU_SETMASK (node0P, visitMask);
            VU_SETMASK (mateP, visitMask);

            vu_vector (&vec10, node1P, node0P);
            vu_vector (&vec12, node1P, node2P);
            vu_vector (&vec23, node2P, node3P);
            d01 = mdlVec2d_magnitudeSquared (&vec10);
            d23 = mdlVec2d_magnitudeSquared (&vec23);
            if (SUCCESS == mdlVec2d_intersectRays (
                                        &vecInt, &s10, &s23,
                                        &vec11, &vec10, &vec12, &vec23, bigParam))
                {
                d1P = mdlVec2d_distanceSquared (&vec11, &vecInt);
                d2P = mdlVec2d_distanceSquared (&vec12, &vecInt);
                mdlVec2d_projectPointToLine (&vecProj, &sProj, &vecInt, &vec11, &vec12);
                distProj = mdlVec2d_distanceSquared (&vecProj, &vecInt);

                if (   ((s10 >= 0.0 && s10 <= maxHitParam) && (s23 < 0.0 && d2P <= dev2))
                        || ((s23 >= 0.0 && s23 <= maxHitParam) && (s10 < 0.0 && d1P <= dev2))
                    )
                    {
                    deleteThisSegment = true;
                    }
                else if (( s10 >= 0.0 && s10 <= maxHitParam
                                && s23 >= 0.0 && s23 <= maxHitParam
                                && distProj < dev2 )
                        )
                    {
                    deleteThisSegment = true;
                    }
                else if ( s10 <= 0.0 && s23 <= 0.0 && d1P < dev2 && d2P < dev2)
                    {
                    deleteThisSegment = true;
                    }
                else
                    {
                    deleteThisSegment = false;
                    }

                vecInt.x += VU_U(node1P);
                vecInt.y += VU_V(node1P);

                if (deleteThisSegment)
                    {
                    free1P = VU_VPRED(node1P);
                    free2P = VU_VPRED(mateP);
                    vu_vertexTwist (graphP, free1P, node1P);
                    vu_vertexTwist (graphP, free2P, mateP);
                    vu_vertexTwist (graphP, free1P, free2P);
                    VU_SET_UV (free1P, vecInt.x, vecInt.y);
                    VU_SET_UV (free2P, vecInt.x, vecInt.y);
                    VU_SETMASK (node1P, discardMask);
                    VU_SETMASK (mateP, discardMask);
                    nDiscard++;
                    }
                }
            }
        VU_SETMASK (node1P, visitMask);
        VU_SETMASK (mateP, visitMask);
        }
    END_VU_SET_LOOP (startP, graphP)

    vu_returnArray (graphP, candidateArrayP);
    vu_returnMask (graphP, visitMask);
    return nDiscard;
    }

/*----------------------------------------------------------------------+
| name          vu_detachDuplicateEdgesAtVertex                         |
| author        EarlinLutz                                      10/95   |
+----------------------------------------------------------------------*/
static void vu_detachDuplicateEdgesAtVertex
(
VuSetP          graphP,
VuP             startP,
VuMask          discardMask
)
    {
    VuP saveP;
    int edgeCount = 0;
    int deleteCount = 0;
    if (startP)
        {
        /* Find a node that is guaranteed to remain */
        saveP = NULL;
        VU_VERTEX_LOOP (currP, startP)
            {
            if (VU_FSUCC(VU_FSUCC(currP)) != currP)
                {
                saveP = currP;
                }
            else
                {
                deleteCount++;
                }
            edgeCount++;
            }
        END_VU_VERTEX_LOOP (currP, startP)

        if (saveP)
            {
            /* No really easy way to stop guard this loop against
               the fact that parts of it (possibly the saveP!!)
               are disappearing under our feet.  Use some dumb counters.
            */
            edgeCount *= 3;
            VU_VERTEX_LOOP (currP, saveP)
                {
                VuP farP = VU_FSUCC(currP);
                if (VU_FSUCC(farP) == currP)
                    {
                    vu_detachEdge(graphP, farP);
                    vu_setMaskAroundFace (farP, discardMask);
                    saveP = currP;
                    deleteCount--;
                    }
                if (deleteCount <= 0 || edgeCount-- < 0)
                    break;
                }
            END_VU_VERTEX_LOOP (currP, saveP)
            }
        }
    }

/*----------------------------------------------------------------------+
| name          vu_checkNewVertexCoordinate                             |
| author        EarlinLutz                                      10/95   |
| Check if a proposed vertex coordinate violates geometry.              |
+----------------------------------------------------------------------*/
static int vu_checkNewVertexCoordinate         /* => SUCCESS if coordiante ok */
(
VuP             nodeP,          /* => any node at vertex being moved */
DPoint2d        *uvP            /* => tentative new coordiantes */
)
    {
    int status = SUCCESS;
    double area1, area2;
    VuP predP, succP;

    if (VU_VSUCC(nodeP) == nodeP)
        {
        /* Allow a dangler to move anywhere (at least at this end --
                the other end might object!!)
        */
        }
    else
        {
        VU_VERTEX_LOOP (currP, nodeP)
            {
            predP = VU_FPRED(nodeP);
            succP = VU_FSUCC(nodeP);
            area1 = vu_cross (succP, predP, nodeP);
            area2 = vu_crossPoint (succP, predP, uvP);
            if (area1 * area2 < 0.0)
                return ERROR;
            }
        END_VU_VERTEX_LOOP (currP, nodeP)
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @description Disconnect small edges from the graph and mark them.
* @remarks On return, the marked edges remain in the graph, but are disconnected from the graph.  This allows
*       the caller to mark more edges with this mask before deleting all masked edges at once.  Thus it is
*       assumed that on input, discardMask, if not cleared in the graph, has only been applied to both or neither
*       sides of each edge of the graph, and that any edges with this mask are disconnected from the rest of the graph.
* @remarks The incident edges to an excised edge have their incident vertices moved to the midpoint of the excised edge.
* @param graphP             IN OUT  graph to examine
* @param tol                IN      absolute xy-distance tolerance for edge size
* @param protectEdgeMask    IN      mask for edges that may not be disconnected
* @param fixedVertexMask    IN      mask for vertices that may not be moved
* @param discardMask        IN      mask to apply to discarded edges
* @return number of edges marked and disconnected from the graph
* @group "VU Edges"
* @see vu_markAndExciseSliverEdges, vu_testAndExciseSmallEdges
* @bsimethod                                                    EarlinLutz      10/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_markAndExciseSmallEdges
(
VuSetP          graphP,
double          tol,
VuMask          protectEdgeMask,
VuMask          fixedVertexMask,
VuMask          discardMask
)
    {
    int nDiscard = 0;
    int n1, n2;
    VuP endP;
    VuP startVertP, endVertP;
    DPoint2d startPoint, endPoint, newPoint;
    double tol2 = tol * tol;
    double dist2;
    int deleteThisEdge;

    VuMask visitMask     = vu_grabMask (graphP);

    VuMask skipMask = discardMask | visitMask;

    vu_clearMaskInSet (graphP, visitMask);

    VU_SET_LOOP (startP, graphP)
        {
        if (!VU_GETMASK (startP, skipMask))
            {
            endVertP = VU_FSUCC(startP);
            endP = VU_VSUCC(endVertP);
            startVertP = VU_FSUCC(endP);

            vu_setMask (startP, visitMask);
            vu_setMask (endP, visitMask);

            vu_getDPoint2d (&startPoint, startP);
            vu_getDPoint2d (&endPoint, endP);

            dist2 = mdlVec2d_distanceSquared (&startPoint, &endPoint);
            deleteThisEdge =
                (dist2 <= tol2)
                && ! vu_getMask (startP, protectEdgeMask)
                && ! vu_getMask (endP, protectEdgeMask)
                ;

            if (deleteThisEdge)
                {
                n1 = vu_countMaskAroundVertex (startP, fixedVertexMask);
                n2 = vu_countMaskAroundVertex (endP, fixedVertexMask);

                if (n1 == 0 && n2 == 0)
                    {
                    mdlVec2d_interpolate (&newPoint, &startPoint, 0.5, &endPoint);
                    deleteThisEdge = (SUCCESS == vu_checkNewVertexCoordinate (startP, &newPoint));
                    deleteThisEdge = (SUCCESS == vu_checkNewVertexCoordinate (endP, &newPoint));
                    }
                else if (n1 == 0 && n2 > 0)
                    {
                    newPoint = endPoint;
                    deleteThisEdge = (SUCCESS == vu_checkNewVertexCoordinate (startP, &newPoint));
                    deleteThisEdge = false;
                    }
                else if (n1 > 0 && n2 == 0)
                    {
                    newPoint = startPoint;
                    deleteThisEdge = (SUCCESS == vu_checkNewVertexCoordinate (endP, &newPoint));
                    deleteThisEdge = false;
                    }
                else
                    {
                    deleteThisEdge = false;
                    }

                if (deleteThisEdge)
                    {
                    /* Excise and mark the edge */
                    vu_vertexTwist (graphP, startVertP, startP);
                    vu_vertexTwist (graphP, endVertP, endP);
                    vu_setMask (startP, discardMask);
                    vu_setMask (endP, discardMask);

                    /* Rejoin the dangling vertices */
                    vu_vertexTwist (graphP, startVertP, endVertP);
                    vu_setDPoint2dAroundVertex (startVertP, &newPoint);
                    vu_detachDuplicateEdgesAtVertex (graphP, startVertP, discardMask);
                    }

                }
            }
        }
    END_VU_SET_LOOP (startP, graphP)

    vu_returnMask (graphP, visitMask);
    return nDiscard;
    }

/*---------------------------------------------------------------------------------**//**
* @description Split all sliver triangles in the graph.
* @remarks For the purposes of this function, a "sliver triangle" is defined to be a triangle ABC where vertex C is "near" edge AB,
*       but all three edges are "long".  Such a triangle is split along a new edge from C to its projection onto AB.
* @param graphP         IN OUT  graph to examine
* @param minHeight      IN      absolute minimum xy-altitude of a non-sliver triangle
* @param legFactor      IN      absolute minimum xy-length of sliver triangle legs, as fraction of xy-altitude
* @return number of sliver triangles split
* @group "VU Edges"
* @bsimethod                                                    EarlinLutz      10/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_splitSliverTriangles
(
VuSetP          graphP,
double          minHeight,
double          legFactor               /* => require all legs to be larger than this multiple
                                        of the height */
)
    {

    double minHeight2 = minHeight * minHeight;
    double legFactor2    = legFactor * legFactor;
    DPoint2d pointA, pointB, pointC, pointD;
    double minLeg2;
    double dist2AB, dist2BC, dist2CA, dist2CD, paramD;
    VuP nodeAP, nodeBP, nodeDP, nodeD1P, nodeD2P, nodeC1P;

    int changes = 0;

    VU_SET_LOOP (nodeCP, graphP)
        {
        nodeAP = vu_fsucc(nodeCP);
        nodeBP = vu_fsucc(nodeAP);

        vu_getDPoint2d (&pointA, nodeAP);
        vu_getDPoint2d (&pointB, nodeBP);
        vu_getDPoint2d (&pointC, nodeCP);

        if (    vu_fsucc(nodeBP) == nodeCP /* make sure it's a triangle !!! */)
            {
            if (SUCCESS == mdlVec2d_projectPointToLine (&pointD, &paramD,
                                                &pointC, &pointA, &pointB)
                && (dist2CD = mdlVec2d_distanceSquared (&pointC, &pointD)) < minHeight2
                && 0.0 < paramD
                && paramD < 1.0
                )
                {

                dist2AB = mdlVec2d_distanceSquared (&pointA, &pointB);
                dist2BC = mdlVec2d_distanceSquared (&pointB, &pointC);
                dist2CA = mdlVec2d_distanceSquared (&pointC, &pointA);

                minLeg2 = legFactor2 * dist2CD;

                if  (    dist2AB > minLeg2
                     && dist2BC > minLeg2
                     && dist2CA > minLeg2
                    )
                    {
                    changes++;
                    vu_splitEdge (graphP, nodeAP, &nodeDP, &nodeD2P);
                    vu_setDPoint2dAroundVertex (nodeDP, &pointD);
                    vu_join(graphP, nodeCP, nodeDP, &nodeC1P, &nodeD1P);
                    }
                }
            }

        }
    END_VU_SET_LOOP (nodeCP, graphP)

    return changes;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
