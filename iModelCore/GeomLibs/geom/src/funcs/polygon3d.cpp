/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/polygon3d.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <stdlib.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


static int bsiDVec3d_dominantComponentIndex
(
DVec3dCP pVector
)
    {
    int iMax = 0;
    double dMax = fabs (pVector->x);
    double di;
    if ((di = fabs (pVector->y)) > dMax)
        {
        iMax = 1;
        dMax = di;
        }
    if ((di = fabs (pVector->z)) > dMax)
        {
        iMax = 2;
        dMax = di;
        }
    return iMax;
    }

/*---------------------------------------------------------------------------------**//**
* @description Scan the edges of a polygon and determine the closest approach to a test point.
* @param pNearestPoint <= coordinates of closest point.
* @param pClassificationVector <= cross product of the vector along the closest
*               edge with the vector from the edge base to the test point.
*               If the polygon has a known orientation and return type is 2 or 3, the
*               dot product of this vector and the polygon normal is positive iff the
*               point is inside the polygon.
* @param pParam <= parametric coordinate along nearest edge
* @param pBaseVertexId <= index of base vertex of nearest edge
* @param pPointArray => polygon vertices
* @param nPoint => number of vertices
* @param pPoint => point to test
* @param tolDistSquared => squared absolute tolerance for deciding if the point is on an edge or vertex.
* @return 0 if the test point is on a vertex,
*         1 if along an edge interior,
*         2 if not on any edge and projects to an edge interior,
*         3 if not on any edge and projects to a vertex.
* @group Polygons
* @bsimethod                                                    EarlinLutz      01/96
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiPolygon_closestEdgeToPoint

(
DPoint3dP pNearestPoint,
DPoint3dP pClassificationVector,
double      *pParam,
int         *pBaseVertexId,
DPoint3dCP pPointArray,
int         nPoint,
DPoint3dCP pPoint,
double      tolDistSquared
)
    {
    int i0, i1;
    DPoint3d oldBasePoint;
    double roundoffRelTol = bsiTrig_smallAngle ();
    double      param;
    double      UdotU, UdotV, VdotV;
    double d2Min;
    double d2Normal;
    //int     firstTime = true;
    int     code = -1;
    double polygonLength = bsiGeom_polylineLength (pPointArray, nPoint);
    double edgeTol = roundoffRelTol * polygonLength;    // for closure test in the facet.
    double minTolSquared = edgeTol * edgeTol;
    if (minTolSquared > tolDistSquared)
        tolDistSquared = minTolSquared; // for hit comparisons


    while (nPoint > 1 && pPointArray[0].Distance (pPointArray[nPoint - 1]) <= edgeTol)
        nPoint--;
    if (nPoint == 0)
        return 0;
    if (nPoint == 1)
        {
        *pNearestPoint = pPointArray[0];
        pClassificationVector->Zero ();
        return 3;
        }

    oldBasePoint = pPointArray[nPoint - 1];
    d2Min = DBL_MAX;
    for (i0 =0; i0 < nPoint; i0++)
        {
        i1 = i0 + 1;
        if (i1 == nPoint)
            i1 = 0;

        DPoint3d basePoint = pPointArray[i0];
        DVec3d vectorU, vectorV;

        vectorV.DifferenceOf (*pPoint, basePoint);
        VdotV = vectorV.MagnitudeSquared ();
        vectorU.DifferenceOf (pPointArray[i1], basePoint);
        UdotU = vectorU.MagnitudeSquared ();
        UdotV = vectorU.DotProduct (vectorV);
        if (UdotU < tolDistSquared)
            continue;

        param = UdotV / UdotU;

        if (param <= 0.0)
            {
            /* point is 'before' this segment.  Check for a vertex hit or outside point */
            if (VdotV < tolDistSquared)
                {
                *pBaseVertexId = i0;
                *pParam = 0.0;
                *pNearestPoint = basePoint;
                return 0;
                }

            if (VdotV < d2Min)
                {
                DVec3d vectorU0;
                vectorU0.DifferenceOf (oldBasePoint, basePoint);
                if (vectorU0.DotProduct (vectorV) <= 0.0)
                    {
                    pClassificationVector->CrossProduct (vectorU0, vectorU);
                    *pBaseVertexId = i0;
                    *pNearestPoint = basePoint;
                    *pParam = 0.0;
                    d2Min = VdotV;
                    code = 3;
                    }
                }
            }
        else if (param <= 1.0)
            {
            /* Point projects within segement. */
            d2Normal = VdotV - param * param * UdotU;
            if (d2Normal < roundoffRelTol * VdotV)
                d2Normal = 0.0;
            if (d2Normal >= d2Min)
                {
                /* Ignore this segment */
                }
            else if (d2Normal < tolDistSquared)
                {
                if ( param <= 0.5)
                    {
                    if (VdotV < tolDistSquared)
                        {
                        *pNearestPoint = basePoint;
                        *pBaseVertexId = i0;
                        *pParam = 0.0;
                        return 0;
                        }
                    }
                else
                    {
                    /* Are we close to the end point? */
                    double reverseParam = 1.0 - param;
                    if (d2Normal + reverseParam * reverseParam * UdotU <= tolDistSquared)
                        {
                        *pNearestPoint = pPointArray[i1];
                        *pBaseVertexId = i1;
                        *pParam = 0.0;
                        return 0;
                        }
                    }
                /* Close to the edge, but not to either endpoint */
                *pBaseVertexId = i0;
                *pParam = param;
                pNearestPoint->SumOf (basePoint,vectorU, param);
                return 1;
                }
            else
                {
                /* We have found a new closest point */
                *pBaseVertexId = i0;
                *pParam        = param;
                pNearestPoint->SumOf (basePoint, vectorU, param);
                pClassificationVector->CrossProduct (vectorU, vectorV);
                d2Min = d2Normal;
                code = 2;
                }
            }
        // We get here when the current edge is nonzero length (good for next step)
        // zero length is skipped and oldBasePoint stays in place ...
        oldBasePoint = basePoint;
        }

    return code;
    }


/*---------------------------------------------------------------------------------**//**
* @description Return the point where a ray pierces a polygon, and classify it as interior, exterior, edge or vertex hit.
*
* @param pPierce        <= point where the ray pierces the plane
* @param pParam         <= parametric coordinate of nearest edge point
* @param pBaseVertexId  <= index of base vertex of edge
* @param pPointArray    => vertices of polygon
* @param nPoint         => number of polygon vertices
* @param pNormal        => polygon normal.  If null pointer is given, the normal is recomputed.
* @param pOrigin        => origin of ray
* @param pDirection     => ray direction
* @param tolDistSquared => squared absolute tolerance for on-edge decision
* @return -3 if ray passes through polygon exterior and projects to a vertex,
*         -2 if ray passes through polygon exterior and projects to the interior of an edge,
*         -1 if no intersection,
*          0 if ray passes through a vertex,
*          1 if ray passes through the interior of an edge,
*          2 if ray passes through polygon interior and projects to the interior of an edge, or
*          3 if ray passes through polygon interior and projects to a vertex.
* @group Polygons
* @bsimethod                                                    EarlinLutz      08/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiPolygon_piercePoint

(
DPoint3dP pPierce,
double      *pParam,
int         *pBaseVertexId,
DPoint3dCP pPointArray,
int         nPoint,
DPoint3dCP pNormal,
DPoint3dCP pOrigin,
DPoint3dCP pDirection,
double      tolDistSquared
)
    {
    DPoint3d    normal;
    DPoint3d    origin;
    DPoint3d    nearestPoint;
    DPoint3d    classificationVector;
    int         code;

    if (pNormal)
        {
        normal = *pNormal;
        }
    else
        {
        bsiGeom_polygonNormal (&normal,  &origin, pPointArray, nPoint);
        }

    if (bsiGeom_rayPlaneIntersection
                            (
                            NULL,
                            pPierce,
                            pOrigin,
                            pDirection,
                            &pPointArray[0],
                            &normal
                            ))
        {
        code = bsiPolygon_closestEdgeToPoint (
                                &nearestPoint,
                                &classificationVector,
                                pParam,
                                pBaseVertexId,
                                pPointArray, nPoint,
                                pPierce,
                                tolDistSquared);
        if (code == 2 || code == 3)
            {
            if (classificationVector.DotProduct (normal) < 0.0)
                {
                code = -code;
                }
            }
        }
    else
        {
        code = -1;
        }
    return code;
    }



/*----------------------------------------------------------------------+
|                                                                       |
| name          xyPolygonParityYTest                                    |
|                                                                       |
| author        EarlinLutz                              04/97           |
|                                                                       |
| Classify a point with respect to a polygon defined by the xy          |
| parts of the points, using only the y coordinate for the tests.       |
| Return false (failure, could not determine answer) if any polygon     |
| point has the same y coordinate as the test point.  Goal of code      |
| is to execute the simplest cases as fast as possible, and fail        |
| promptly for others.                                                  |
+----------------------------------------------------------------------*/
static bool    xyPolygonParityYTest

(
int         *pParity,         /* <= parity result, if successful */
DPoint3dCP pPoint,          /* => point to test */
DPoint3dCP pPointArray,   /* => polygon points */
int         numPoint,         /* number of planes */
double      tol               /* tolerance for ON case detection */
)
    {
    /* Var names h, crossing to allow closest code correspondence between x,y code */
    double crossing0 = pPoint->x;
    double h = pPoint->y;
    double h0 = h - pPointArray[numPoint - 1].y;
    double h1;
    double crossing;
    double s;
    int numLeft = 0, numRight = 0;

    int i, i0;
    if (fabs (h0) <= tol)
        return false;
    for (i = 0; i < numPoint; i++, h0 = h1)
        {
        h1 = h - pPointArray[i].y;
        if (fabs (h1) <= tol)
            return  false;
        if (h0 * h1 < 0.0)
            {
            s = -h0 / (h1 - h0);
            i0 = i - 1;
            if (i0 < 0)
                i0 = numPoint - 1;
            crossing = pPointArray[i0].x + s * (pPointArray[i].x - pPointArray[i0].x);
            if (fabs (crossing - crossing0) <= tol)
                {
                *pParity = 0;
                return  true;
                }
            else if ( crossing < crossing0 )
                {
                numLeft++;
                }
            else
                {
                numRight++;
                }
            }
        }

    *pParity = (numLeft & 0x01) ? 1 : -1;
    return  true;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          xyPolygonParityXTest                                    |
|                                                                       |
| author        EarlinLutz                              04/97           |
|                                                                       |
| Classify a point with respect to a polygon defined by the xy          |
| parts of the points, using only the x coordinate for the tests.       |
| Return false (failure, could not determine answer) if any polygon     |
| point has the same x coordinate as the test point.  Goal of code      |
| is to execute the simplest cases as fast as possible, and fail        |
| promptly for others.                                                  |
+----------------------------------------------------------------------*/
static bool    xyPolygonParityXTest

(
int         *pParity,         /* <= parity result, if successful */
DPoint3dCP pPoint,          /* => point to test */
DPoint3dCP pPointArray,   /* => polygon points */
int         numPoint,         /* number of planes */
double      tol               /* tolerance for ON case detection */
)
    {
    /* Var names h, crossing to allow closest code correspondence between x,y code */
    double crossing0 = pPoint->y;
    double h = pPoint->x;
    double h0 = h - pPointArray[numPoint - 1].x;
    double h1;
    double crossing;
    double s;
    int numLeft = 0, numRight = 0;

    int i, i0;
    if (fabs (h0) <= tol)
        return false;
    for (i = 0; i < numPoint; i++, h0 = h1)
        {
        h1 = h - pPointArray[i].x;
        if (fabs (h1) <= tol)
            return  false;
        if (h0 * h1 < 0.0)
            {
            s = -h0 / (h1 - h0);
            i0 = i - 1;
            if (i0 < 0)
                i0 = numPoint - 1;
            crossing = pPointArray[i0].y + s * (pPointArray[i].y - pPointArray[i0].y);
            if (fabs (crossing - crossing0) <= tol)
                {
                *pParity = 0;
                return  true;
                }
            else if ( crossing < crossing0 )
                {
                numLeft++;
                }
            else
                {
                numRight++;
                }
            }
        }

    *pParity = (numLeft & 0x01) ? 1 : -1;
    return  true;
    }

static double dotProductToPointXY

(
DVec3dCP pVector,
DPoint3dCP pBasePoint,
DPoint3dCP pTargetPoint
)
    {
    return    pVector->x * (pTargetPoint->x - pBasePoint->x)
            + pVector->y * (pTargetPoint->y - pBasePoint->y);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          xyPolygonParityVectorTest                               |
|                                                                       |
| author        EarlinLutz                              04/97           |
|                                                                       |
| Classify a point with respect to a polygon defined by the xy          |
| parts of the points, using a given ray cast direction.                |
| Return false (failure, could not determine answer) if any polygon     |
| point is on the ray.                                                  |
+----------------------------------------------------------------------*/
static bool    xyPolygonParityVectorTest

(
int         *pParity,         /* <= parity result, if successful */
DPoint3dCP pPoint,          /* => point to test */
double      theta,            /* => angle for ray cast */
DPoint3dCP pPointArray,     /* => polygon points */
int         numPoint,         /* number of planes */
double      tol               /* tolerance for ON case detection */
)
    {
    DVec3d tangent, normal;
    double v0, v1;
    double u0, u1;
    double u;
    double s;
    int numLeft = 0, numRight = 0;

    int i, i0;

    tangent.x = cos (theta);
    tangent.y = sin (theta);
    tangent.z = normal.z = 0.0;
    normal.x = - tangent.y;
    normal.y =   tangent.x;

    v0 = dotProductToPointXY (&normal, pPoint, &pPointArray[numPoint - 1]);

    if (fabs (v0) <= tol)
        return false;
    for (i = 0; i < numPoint; i++, v0 = v1)
        {
        v1 = dotProductToPointXY (&normal, pPoint, &pPointArray[i]);
        if (fabs (v1) <= tol)
            return  false;
        if (v0 * v1 < 0.0)
            {
            s = -v0 / (v1 - v0);
            i0 = i - 1;
            if (i0 < 0)
                i0 = numPoint - 1;
            u0 = dotProductToPointXY (&tangent, pPoint, &pPointArray[i0]);
            u1 = dotProductToPointXY (&tangent, pPoint, &pPointArray[i]);
            u = u0 + s * (u1 - u0);
            if (fabs (u) <= tol)
                {
                *pParity = 0;
                return  true;
                }
            else if ( u < 0.0 )
                {
                numLeft++;
                }
            else
                {
                numRight++;
                }
            }
        }

    *pParity = (numLeft & 0x01) ? 1 : -1;
    return  true;
    }
#ifdef CompileSweptPolygonCrossingCounts
/*---------------------------------------------------------------------------------**//**
* @description Look for intersections of a ray with a swept polygon.
* @remarks In most common usage, the caller expects the polygon to be planar, and passes the plane normal as the sweep direction.
*   Defining the results in terms of a swept polygon volume clarifies what the calculation means if the caller unexpectedly has a
*   non-planar polygon.
* @param pNumPositiveRayCrossings OUT number of edges which cross the (strictly) negative half-ray
* @param pNumNegativeRayCrossings OUT number of edges which cross the (strictly) positive half-ray
* @param pbAnyEdgePassesThroughPoint OUT true if one or more edges pass exactly throught the point
* @param pPoint IN start point of ray
* @param pRayDirection IN direction of ray
* @param pPointArray IN array of polygon vertices
* @param numPoint IN number of vertices in polygon
* @param pSweepDirection IN direction of sweep, typically the normal to the polygon
* @return true if the ray and sweep directions are are non-parallel
* @group Polygons
* @bsimethod                                                    EarlinLutz      08/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_sweptPolygonCrossingCounts

(
int *pNumPositiveRayCrossings,
int *pNumNegativeRayCrossings,
bool    *pbAnyEdgePassesThroughPoint,
DPoint3dCP pPoint,
DPoint3dCP pRayDirection,
DPoint3dCP pPointArray,
int       numPoint,
DPoint3dCP pSweepDirection
)
    {
    DPoint3d xDir;
    DPoint3d yDir;
    Transform localToWorld, worldToLocal;
    int i;
    bool    bExactOnEdge;
    int numPositiveCrossing;
    int numNegativeCrossing;
    bool    bStat = false;

    /* Normal vector for cut plane containing the ray and sweep directions */
    xDir.Normalize (*pRayDirection);
    yDir.NormalizedCrossProduct (*pRayDirection, *pSweepDirection);

    localToWorld.InitFromOriginAndVectors(*pPoint, *pRayDirection, yDir, *pSweepDirection);

    numPositiveCrossing = numNegativeCrossing = 0;
    bExactOnEdge = false;

    if (!worldToLocal.InverseOf (localToWorld))
        {
        bStat = false;
        }
    else if (numPoint <= 0)
        {
        /* Zero points might not make sense in typical polygon, but the counts are all clearly zero. */
        bStat = true;
        }
    else
        {
        DPoint3d *pXYZ =(DPoint3d*)_alloca (sizeof (DPoint3d) * (numPoint+1));

        bStat = true;
        /* transform to local system where x is "along the ray" and y is "above the plane of the
                ray and sweep direction" */
        worldToLocal.Multiply (pXYZ, pPointArray, numPoint);

        /* Force duplicate first/last point (even if it creates a bogus zero-length edge at end)
            The counting logic will not be affected by a duplicate point.
        */
        pXYZ[numPoint] = pXYZ[0];
        /* Count crossings.
           When an exact zero is encountered at one end of an edge,
           treat it as if shifted slightly negative.
        */
        for (i = 0; i < numPoint; i++)
            {
            double yProduct = pXYZ[i].y * pXYZ[i+1].y;
            if (yProduct > 0.0)
                {
                /* Both ends on same side.   Nothing to count */
                }
            else if (yProduct < 0.0)
                {
                /* Simple crossing in strict interior of polygon edge. */
                double fraction = - pXYZ[i].y / (pXYZ[i+1].y - pXYZ[i].y);
                double x = pXYZ[i].x + fraction * (pXYZ[i+1].x - pXYZ[i].x);
                if (x > 0.0)
                    {
                    numPositiveCrossing++;
                    }
                else if (x < 0.0)
                    {
                    numNegativeCrossing++;
                    }
                else
                    {
                    bExactOnEdge = true;
                    }
                }
            else if (pXYZ[i+1].y != 0.0)
                {
                /* point i is ON the x axis */
                if (pXYZ[i+1].y > 0.0)
                    {
                    if (pXYZ[i].x > 0.0)
                        numPositiveCrossing++;
                    else if (pXYZ[i].x < 0.0)
                        numNegativeCrossing++;
                    else
                        bExactOnEdge = true;
                    }
                }
            else if (pXYZ[i].y != 0.0)
                {
                /* point i+1 is ON the x axis */
                if (pXYZ[i].y > 0.0)
                    {
                    if (pXYZ[i+1].x > 0.0)
                        numPositiveCrossing++;
                    else if (pXYZ[i+1].x < 0.0)
                        numNegativeCrossing++;
                    else
                        bExactOnEdge = true;
                    }
                }
            else
                {
                /* Both ends are ON.
                   Record an ON hit if the x values straddle or touch zero.
                   Otherwise both points on same side, don't count it at all.
                */
                if (pXYZ[i].x * pXYZ[i+1].x <= 0.0)
                    {
                    bExactOnEdge = true;
                    }
                }
            }
        }

    if (pNumPositiveRayCrossings)
        *pNumPositiveRayCrossings = numPositiveCrossing;
    if (pNumNegativeRayCrossings)
        *pNumNegativeRayCrossings = numNegativeCrossing;
    if (pbAnyEdgePassesThroughPoint)
        *pbAnyEdgePassesThroughPoint = bExactOnEdge;

    return bStat;
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @description Classify a point with respect to a polygon, ignoring z-coordinates.
* @param pPoint => point to test
* @param pPointArray => polygon vertices
* @param numPoint => number of polygon vertices
* @param tol => absolute tolerance for ON classification
* @return 0 if pPoint is on the polygon within tolerance, 1 if in, -1 if out, -2 if nothing worked
* @group Polygons
* @bsimethod                                                    EarlinLutz      08/99
+---------------+---------------+---------------+---------------+---------------+------*/
static int bsiGeom_XYPolygonParity_oneLoop

(
DPoint3dCP pPoint,
DPoint3dCP pPointArray,
int         numPoint,
double      tol
)
    {
    int i, parity;
    double x = pPoint->x, y = pPoint->y, theta, dTheta, maxTheta;

    if (numPoint < 2)
        return (fabs (x - pPointArray[0].x) <= tol && fabs (y - pPointArray[0].y) <= tol) ? 0 : -1;

    /* Try really easy ways first ... */
    if ( xyPolygonParityYTest (&parity, pPoint, pPointArray, numPoint, tol))
        return  parity;
    if ( xyPolygonParityXTest (&parity, pPoint, pPointArray, numPoint, tol))
        return  parity;

    // Is test point within tol of one of the polygon points in x and y?
    for (i = 0; i < numPoint; i++)
        if (fabs (x - pPointArray[i].x) <= tol && fabs (y - pPointArray[i].y) <= tol)
            return 0;

    /* Nothing easy worked. Try some ray casts */
    maxTheta = 10.0;
    theta = dTheta = 0.276234342921378;
    while (theta < maxTheta)
        {
        if (xyPolygonParityVectorTest (&parity, pPoint, theta, pPointArray, numPoint, tol))
            return parity;
        theta += dTheta;
        }
    return -2;
    }


/*---------------------------------------------------------------------------------**//**
* @description Classify a point with respect to a polygon, ignoring z-coordinates.
*     Holes are passed separated by DISCONNECT points.
* @param pPoint => point to test
* @param pPointArray => polygon vertices
* @param numPoint => number of polygon vertices
* @param tol => absolute tolerance for ON classification
* @return 0 if pPoint is on the polygon within tolerance, 1 if in, -1 if out, -2 if nothing worked
* @group Polygons
* @bsimethod                                                    EarlinLutz      08/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiGeom_XYPolygonParity

(
DPoint3dCP pPoint,
DPoint3dCP pPointArray,
int         numPoint,
double      tol
)
    {
    int i0 = 0;
    int numIn = 0;
    int numOut = 0;
    for (int i1 = 0; i1 <= numPoint;)
        {
        if (i1 >= numPoint || pPointArray[i1].IsDisconnect ())
            {
            int numThisLoop = i1 - i0;
            if (numThisLoop > 1)
                {
                int loopStat = bsiGeom_XYPolygonParity_oneLoop (pPoint, pPointArray+i0, i1 - i0, tol);
                if (loopStat == 0)
                    return 0;
                if (loopStat == 1)
                    {
                    numIn++;
                    }
                else if (loopStat == -  1)
                    {
                    numOut++;
                    }
                else
                    {
                    return loopStat;
                    }
                }
            i0 = i1 = i1+1;
            }
        else
            {
            i1++;
            }
        }
    if (1 == (numIn & 0x01))
        return 1;
    else
        return -1;
    }


/*---------------------------------------------------------------------------------**//**
* @description Compute the signed area of the polygon, using only xy-coordinates.
* @remarks Positive area is a counterclocwise polygon, negative is clockwise.
* @param    pPointArray => polygon vertices
* @param    numPoint    => number of vertices
* @return signed area of polygon
* @group Polygons
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiGeom_getXYPolygonArea

(
DPoint3dCP pPointArray,
int             numPoint
)
    {
    return PolygonOps::AreaXY (pPointArray, numPoint);
    }


/*---------------------------------------------------------------------------------**//**
* @description Copy vertices of a polygon into the output array, suppressing immediately
*       adjacent duplicate points.
* @remarks This function also processes a given integer array in a parallel fashion.
* @remarks The first point/integer of a run of duplicates is saved in the compressed output arrays.
* @remarks Input/output arrays may be the same.
* @remarks Working tolerance is abstol + reltol * maxCoordinate, where maxCoordinate is the maximum coordinate found in any point.
* @param pXYZOut    <= compressed point array (or NULL)
* @param pIntOut    <= compressed int array (or NULL)
* @param pIntOut2   <= copy of pIntIn with duplicate entries corresponding to redundant points (or NULL)
* @param pNumOut    <= compressed point/int count (or NULL)
* @param pXYZIn     => point array
* @param pIntIn     => integer array (or NULL)
* @param numIn      => point/int count
* @param abstol     => absolute tolerance for identical point test.
* @param reltol     => relative tolerance for identical point test.
* @return true if compression was successful
* @group Polygons
* @bsimethod                                                    DavidAssaf      03/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiPolygon_compressDuplicateVertices

(
DPoint3dP pXYZOut,
int             *pIntOut,
int             *pIntOut2,
int             *pNumOut,
DPoint3dCP pXYZIn,
const int       *pIntIn,
int             numIn,
double          abstol,
double          reltol
)
    {
    bool    bInts = pIntOut && pIntIn;
    bool    bInts2 = pIntOut2 && pIntIn;
    int i, j, k;
    double tol;

    if (!pXYZIn || numIn < 1 || (pIntIn && pIntOut && pIntOut2 && (pIntOut == pIntOut2)))
        return false;

    /* l-infinity norm (max{abs(coordinate difference)}) */
    tol = abstol;
    if (tol < 0.0)
        tol = 0.0;
    if (reltol > 0.0)
        tol += reltol * bsiDPoint3d_getLargestCoordinateDifference (pXYZIn, numIn);

    /* copy full ints */
    if (bInts2)
        memmove (pIntOut2, pIntIn, numIn * sizeof (*pIntOut2));

    /* Eliminate trailing points which duplicate first point */
    while (1 < numIn && pXYZIn->IsEqual (pXYZIn[ numIn - 1], tol))
        {
        numIn--;

        if (bInts2)
            pIntOut2[numIn] = pIntIn[0];    /* set trailing index equal to first index */
        }

    /* Strip internal redundant runs (k <= j <= i) */
    for (i = j = k = 0; i < numIn; j = i, k++)
        {
        /* store the first vertex/int of this run */
        if (pXYZOut)
            pXYZOut[k] = pXYZIn[i];

        if (bInts)
            pIntOut[k] = pIntIn[i];

        /* skip past dups in this run (j points to 1st vertex/int of this run) */
        while (++i < numIn && pXYZIn[ j].IsEqual (pXYZIn[ i], tol))
            {
            if (bInts2)
                pIntOut2[i] = pIntIn[j];    /* duplicate the int corresp to the 1st vertex in this run */
            }
        }

    if (pNumOut)
        *pNumOut = k;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description Test if pXYZ0 is "below" pXYZ1 in xy-lexical order.
* @param pXYZ0  IN  point to test
* @param pXYZ1  IN  comparison point
* @return true iff (a) the y-coordinate of pXYZ1 is strictly greater than the y-coordinate of pXYZ0, or
*                  (b) the y-coordinates are identical but x-coordinate of pXYZ1 is strictly greater than the x-coordinate of pXYZ0.
* @group "DPoint3d Queries"
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_lexicalXYBelow

(
DPoint3dCP pXYZ0,
DPoint3dCP pXYZ1
)
    {
    double dy = pXYZ1->y - pXYZ0->y;
    return dy > 0 || (dy == 0 && pXYZ1->x > pXYZ0->x);
    }

/*---------------------------------------------------------------------------------**//**
* @description Test if pXYZ0 is "to the left of" pXYZ1 in xy-lexical order.
* @param pXYZ0  IN  point to test
* @param pXYZ1  IN  comparison point
* @return true iff (a) the x-coordinate of pXYZ1 is strictly greater than the x-coordinate of pXYZ0, or
*                  (b) the x coordinates are identical but the y-coordinate of pXYZ1 is strictly greater than the y-coordinate of pXYZ0.
* @group "DPoint3d Queries"
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_lexicalXYLeftOf

(
DPoint3dCP pXYZ0,
DPoint3dCP pXYZ1
)
    {
    double dx = pXYZ1->x - pXYZ0->x;
    return dx > 0 || (dx == 0 && pXYZ1->y > pXYZ0->y);
    }

/*---------------------------------------------------------------------------------**//**
* @description Test if a polygon is has 3 or more points and all cross products from pPointArray[0]
* * to other points are in the same direction as largest cross product.
* @param pPointArray => polygon vertices
* @param numPoint    => number of vertices.  Trailing duplicates of the first vertex are ignored.
* @return true if polygon is convex.
* @group Polygons
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool bsiGeom_testPolygonConvex
(
DPoint3dCP pPointArray,
int             numPoint
)
    {
    DVec3d vecA, vecB, maxCross, cross;
    static double s_areaRelTol = 1.0e-12;
    DVec3d unitNormal;

    double positiveArea = 0.0;
    double negativeArea = 0.0;

    while (numPoint > 2 && pPointArray[numPoint - 1].IsEqual (pPointArray[0]))
        numPoint--;

    if (numPoint < 3)
        return FALSE;
    double a2 = 0.0;
    maxCross.Zero ();
    // Find largest cross product as reference vector.
    vecA.DifferenceOf (pPointArray[1], pPointArray[0]);
    for (int i = 2; i < numPoint; i++, vecA = vecB)
        {
        vecB.DifferenceOf (pPointArray[i], pPointArray[0]);
        cross.CrossProduct (vecA, vecB);
        double b2 = cross.MagnitudeSquared ();
        if (b2 > a2)
            {
            a2 = b2;
            maxCross = cross;
            }
        }

    unitNormal.Normalize (maxCross);
    // 
    vecA.DifferenceOf (pPointArray[0], pPointArray[numPoint - 1]);
    for (int i = 1; i <= numPoint; i++, vecA = vecB)
        {
        vecB.DifferenceOf (pPointArray[i % numPoint], pPointArray[i-1]);
        cross.CrossProduct (vecA, vecB);
        double b = cross.DotProduct (unitNormal);
        if (b >= 0.0)
            positiveArea += b;
        else
            negativeArea += b;
        }

    return fabs (negativeArea) < s_areaRelTol * positiveArea;
    }

/*---------------------------------------------------------------------------------**//**
* @description Test if a polygon is convex, ignoring z-coordinates.

* @param pPointArray => polygon vertices
* @param numPoint    => number of vertices.  Trailing duplicates of the first vertex are ignored.
* @return 0 if polygon is not convex, 1 if convex with all turns to left, -1 if convex with all turns to right.
* @group Polygons
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiGeom_testXYPolygonConvex

(
DPoint3dCP pPointArray,
int             numPoint
)
    {
    int iLast, i1, i0;
    bool    below0, below1, left0, left1;

    int numXMin = 0;
    int numXMax = 0;
    int numYMin = 0;
    int numYMax = 0;

    // Reduce count by trailing duplicates; leaves iLast at final index.
    iLast = numPoint - 1;
    while (iLast > 1
            && pPointArray[iLast].x == pPointArray[0].x
            && pPointArray[iLast].y == pPointArray[0].y
            )
        {
        numPoint = iLast--;
        }

    if (numPoint <= 2)
        return 0;

    below0 = bsiDPoint3d_lexicalXYBelow (pPointArray + iLast, pPointArray);
    left0  = bsiDPoint3d_lexicalXYLeftOf (pPointArray + iLast, pPointArray);
    for (i0 = 0; i0 < numPoint; i0++, left0 = left1, below0 = below1)
        {
        i1 = i0 + 1;
        if (i1 >= numPoint)
            i1 = 0;
        below1 = bsiDPoint3d_lexicalXYBelow (pPointArray + i0, pPointArray + i1);
        left1  = bsiDPoint3d_lexicalXYLeftOf (pPointArray + i0, pPointArray + i1);
        if (left0 && !left1)
            numXMax++;
        else if (left1 && !left0)
            numXMin++;

        if (below0 && !below1)
            numYMax++;
        else if (below1 && !below0)
            numYMin++;
        }
    if (numXMin == 1 && numYMin == 1 && numXMax == 1 && numYMax ==1)
        {
        return bsiGeom_testXYPolygonTurningDirections (pPointArray, numPoint);
        }
    else
        return 0;
    }


/*---------------------------------------------------------------------------------**//**
* @description Test the direction of turn at the vertices of the polygon, ignoring z-coordinates.
* @remarks For a polygon without self intersections, this is a convexity and orientation test:
*       all positive is convex and counterclockwise, all negative is convex and clockwise.
*       Beware that a polygon which turns through more than a full turn can cross itself
*       and close, but is not convex.
* @param pPointArray => polygon vertices
* @param numPoint    => number of vertices.  Trailing duplicates of the first vertex are ignored.
* @return 1 if all turns are to the left, -1 if all to the right, and 0 if there are any zero turns
*       (successive colinear edges) or a mixture of right and left.
* @group Polygons
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiGeom_testXYPolygonTurningDirections

(
DPoint3dCP pPointArray,
int             numPoint
)
    {
    double baseArea, currArea;

    int iLast, i1;
    DPoint2d vector0, vector1;

    // Reduce count by trailing duplicates; leaves iLast at final index.
    iLast = numPoint - 1;
    while (iLast > 1
            && pPointArray[iLast].x == pPointArray[0].x
            && pPointArray[iLast].y == pPointArray[0].y
            )
        {
        numPoint = iLast--;
        }

    if (numPoint > 2)
        {
        vector0.x = pPointArray[iLast].x - pPointArray[iLast-1].x;
        vector0.y = pPointArray[iLast].y - pPointArray[iLast-1].y;
        vector1.x = pPointArray[0].x - pPointArray[iLast].x;
        vector1.y = pPointArray[0].y - pPointArray[iLast].y;
        baseArea = vector0.x * vector1.y - vector0.y * vector1.x;
        // In a convex polygon, all successive-vector cross products will
        // have the same sign as the base area, hence all products will be
        // positive.
        for (i1 = 1; i1 < numPoint; i1++)
            {
            vector0 = vector1;
            vector1.x = pPointArray[i1].x - pPointArray[i1 - 1].x;
            vector1.y = pPointArray[i1].y - pPointArray[i1 - 1].y;
            currArea = vector0.x * vector1.y - vector0.y * vector1.x;
            if (currArea * baseArea <= 0.0)
                return 0;
            }
        // Fall out with all signs same as base area.
        return baseArea > 0.0 ? 1 : -1;
        }

    return  0;
    }


/*---------------------------------------------------------------------------------**//**
* @description Compute the normal of the polygon.
* @param pNormal <= polygon normal, or approximate normal if nonplanar points (or NULL)
* @param pOrigin <= origin for plane (or NULL)
* @param pVert => vertex array
* @param numPoints => number of vertices
* @return true if the polygon defines a clear plane
* @group Polygons
* @bsimethod                                                    EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_polygonNormal

(
DPoint3dP pNormal,
DPoint3dP pOrigin,
DPoint3dCP pVert,
int              numPoints
)
    {
    double magnitude = bsiPolygon_polygonNormalAndArea ((DVec3d*) pNormal, pOrigin, pVert, numPoints);
    // Ugh. No good notion of zero.
    return magnitude > 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @description Computes the (unnormalized) normal of the triangle whose vertices are given
*       in ccw order.  Works best when first vertex is at the largest angle of the
*       triangle.
* @param pNormal    <= normal (or NULL)
* @param pXYZ       => array of 3 vertices
* @param dot1       => squared side01 length
* @param dot2       => squared side02 length
* @param eps2       => minval of sin^2(angle) allowable in mag^2 of cross prod
* @return squared magnitude of normal
* @bsimethod                                                    DavidAssaf      10/99
+---------------+---------------+---------------+---------------+---------------+------*/
static double  computeTriangleNormalAtMaxAngle

(
DPoint3dP pNormal,
DPoint3dCP pXYZ,
double              dot1,
double              dot2,
double              eps2
)
    {
    DPoint3d    normal;
    double      mag2;

    normal.CrossProductToPoints (*pXYZ, pXYZ[1], pXYZ[2]);
    mag2 = normal.MagnitudeSquared ();

    /*
    If the angle at first vertex is too large, split it with the median and choose
    the cross product which yields the larger sine in the magnitude.
    */
    if (mag2 <= eps2 * dot1 * dot2)
        {
        double m1, m2;
        DPoint3d v, n1, n2;

        v.Interpolate (pXYZ[1], 0.5, pXYZ[2]);

        n1.CrossProductToPoints (*pXYZ, pXYZ[1], v);
        m1 = n1.MagnitudeSquared ();

        n2.CrossProductToPoints (*pXYZ, v, pXYZ[2]);
        m2 = n2.MagnitudeSquared ();

        // compare sin^2 of subangles
        if (m1 * dot2 >= m2 * dot1)
            {
            normal = n1;
            mag2 = m1;
            }
        else
            {
            normal = n2;
            mag2 = m2;
            }
        }

    if (pNormal)
        *pNormal = normal;

    return mag2;
    }

/*---------------------------------------------------------------------------------**//**
* @description Computes the normal of the given triangle at its largest angle.
*
* @param pNormal    <= unnormalized normal (or NULL)
* @param pMaxIndex  <= index of vertex at largest angle (or NULL)
* @param pXYZ       => array of 3 vertices
* @param eps2       => minimum value of sin^2(angle) allowable in squared magnitude of cross product
* @return squared magnitude of normal, or zero if triangle is degenerate
* @group Polygons
* @bsimethod                                                    DavidAssaf      11/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bsiGeom_triangleNormal

(
DPoint3dP pNormal,
int                 *pMaxIndex,
DPoint3dCP pXYZ,
double              eps2
)
    {
    DPoint3d        normal, xyz[5];
    double          mag2, dot01, dot02, dot12;
    int             maxIndex;

    // get squared side lengths
    dot01 = pXYZ->DistanceSquared (pXYZ[1]);
    dot02 = pXYZ->DistanceSquared (pXYZ[2]);
    dot12 = pXYZ[1].DistanceSquared (pXYZ[2]);

    // cyclically extend the array
    xyz[0] = xyz[3] = pXYZ[0];
    xyz[1] = xyz[4] = pXYZ[1];
    xyz[2] =          pXYZ[2];

    // dot01, angle at pXYZ[2] largest
    if ((dot01 > dot02) && (dot02 > dot12))
        {
        maxIndex = 2;
        mag2 = computeTriangleNormalAtMaxAngle (&normal, &xyz[2], dot02, dot12, eps2);
        }

    // dot02, angle at pXYZ[1] largest
    else if (dot02 > dot12)
        {
        maxIndex = 1;
        mag2 = computeTriangleNormalAtMaxAngle (&normal, &xyz[1], dot12, dot01, eps2);
        }

    // dot12, angle at pXYZ[0] largest
    else
        {
        maxIndex = 0;
        mag2 = computeTriangleNormalAtMaxAngle (&normal, &xyz[0], dot01, dot02, eps2);
        }

    if (pNormal)
        *pNormal = normal;

    if (pMaxIndex)
        *pMaxIndex = maxIndex;

    return mag2;
    }

/*---------------------------------------------------------------------------------**//**
* @description Computes the circumcenter of the given triangle.
*
* @param pCenter        <= circumcenter (or NULL)
* @param pBaryCenter    <= barycentric coordinates of the circumcenter (or NULL)
* @param pXYZ           => array of 3 vertices
* @param eps2           => minimum value of sin^2(angle) allowable in squared magnitude of cross product
* @return squared circumradius, or zero if triangle is degenerate
* @group Polygons
* @bsimethod                                                    DavidAssaf      09/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bsiGeom_triangleCircumcenter

(
DPoint3dP    pCenter,
DPoint3dP    pBaryCenter,
DPoint3dCP    pXYZ,
double      eps2
)
    {
    DPoint3d    norm;               // unnormalized normal of tri at a
    DPoint3d    ba, ca;             // offsets to side points (b-a, c-a)
    DPoint3d    ctra;               // offest to center (ctr-a)
    double      denom;
    double      l2norm, l2ba, l2ca; // squared len of normal, side vectors
    int         a;                  // index of vertex with largest angle
    int         b, c;               // indices of other vertices in order

    l2norm = bsiGeom_triangleNormal (&norm, &a, pXYZ, eps2);

    if (!l2norm)
        return 0.0;

    b = (a+1) % 3;
    c = (a+2) % 3;
    denom = 0.5 / l2norm;

    ba.DifferenceOf (pXYZ[b], pXYZ[a]);
    ca.DifferenceOf (pXYZ[c], pXYZ[a]);

    l2ba = ba.MagnitudeSquared ();
    l2ca = ca.MagnitudeSquared ();

    // calculate offset (from pXYZ[a]) of circumcenter
    ctra.x = ((l2ba * ca.y - l2ca * ba.y) * norm.z - (l2ba * ca.z - l2ca * ba.z) * norm.y) * denom;
    ctra.y = ((l2ba * ca.z - l2ca * ba.z) * norm.x - (l2ba * ca.x - l2ca * ba.x) * norm.z) * denom;
    ctra.z = ((l2ba * ca.x - l2ca * ba.x) * norm.y - (l2ba * ca.y - l2ca * ba.y) * norm.x) * denom;

    if (pCenter)
        pCenter->SumOf (pXYZ[a], ctra);

    if (pBaryCenter)
        {
        // use Cramer's rule, with denom computed from largest abs norm coordinate
        if (((norm.x >= norm.y) || (-norm.x > norm.y)) && ((norm.x >= norm.z) || (-norm.x > norm.z)))
            {
            denom = 1.0 / norm.x;
            pBaryCenter->y = (ctra.y * ca.z - ctra.z * ca.y) * denom;
            pBaryCenter->z = (ctra.z * ba.y - ctra.y * ba.z) * denom;
            }
        else if ((norm.y >= norm.z) || (-norm.y > norm.z))
            {
            denom = 1.0 / norm.y;
            pBaryCenter->y = (ctra.z * ca.x - ctra.x * ca.z) * denom;
            pBaryCenter->z = (ctra.x * ba.z - ctra.z * ba.x) * denom;
            }
        else
            {
            denom = 1.0 / norm.z;
            pBaryCenter->y = (ctra.x * ca.y - ctra.y * ca.x) * denom;
            pBaryCenter->z = (ctra.y * ba.x - ctra.x * ba.y) * denom;
            }

        pBaryCenter->x = 1.0 - pBaryCenter->y - pBaryCenter->z;
        }

    return ctra.MagnitudeSquared ();
    }

/*---------------------------------------------------------------------------------**//**
* @description Prepare an array for use as a polygon.
* 1) Eliminate duplicate points
* 2) Add wraparound point.
* 3) Compute transform to plane
* 4) apply transform
* Caller must allocate arrays for up to numXYZ+1 points (to allow for wraparound.)
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    prepPolygon

(
DPoint3dP pXYZWorld,
DPoint3dP pXYZLocal,
int         *pNumOut,
TransformP pWorldToLocal,
TransformP pLocalToWorld,
DRange3dP pRange,
DPoint3dCP pXYZ,
int numXYZ
)
    {
    int i, j;
    int numOut;

    *pNumOut = 0;
    if (numXYZ <= 1)
        return false;

    pXYZWorld[0] = pXYZ[0];
    // i is destination index of last accepted point.
    // j is source index of next candidate
    for (i = 0, j = 1; j < numXYZ; j++)
        {
        if (   pXYZ[j].x != pXYZWorld[i].x
            || pXYZ[j].y != pXYZWorld[i].y
            || pXYZ[j].z != pXYZWorld[i].z
            )
            {
            i++;
            pXYZWorld[i] = pXYZ[j];
            }
        }
    // Eliminate trailing duplicates of point 0

    while (   i > 0
           && pXYZWorld[i].x == pXYZWorld[0].x
           && pXYZWorld[i].y == pXYZWorld[0].y
           && pXYZWorld[i].z == pXYZWorld[0].z
            )
            {
            i--;
            }

    // Add a wraparound point.
    pXYZWorld[++i] = pXYZWorld[0];
    numOut = i + 1;

    if (!bsiDPoint3dArray_transformToPlane
                (pXYZLocal, pWorldToLocal, pLocalToWorld, pRange, pXYZWorld, numOut))
        return false;

    *pNumOut = numOut;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description Test if a polyline pierces a polygon at a point interior to the polygon, as viewed in the xy-plane.
* @remarks A point in or on the polygon as viewed in the xy-plane is a pierce point if it is either:
*   <ul>
*   <li>a simple intersection of an edge of the polyline with the z=0 plane, or</li>
*   <li>either end of an edge that is entirely on the z=0 plane.</li>
*   </ul>
* @remarks Note that the polygon is treated as if all z-coordinates are 0.  This function is intended to be used
*       when both the polyline and polygon have been transformed into the coordinate system whose xy-plane completely
*       contains the polygon, while the polyline is out of plane.
* @remarks Use ~mbsiDPoint3dArray_polylineClashPolygonXYZ if the plane of the polygon has not yet been determined; this
*       will compute the plane and then call this function with properly transformed data.
*
* @param pXYZA IN points of polyline
* @param int numA IN number of points for polyline
* @param bAddClosureA IN whether to use an additional edge from the end to the start of the polyline.
*         Note that this closure edge does not make the polyline bound area for clash purposes.
* @param pXYZB IN points of polygon
* @param int numB IN number of points for polygon
* @param tol tolerance for on-plane tests and parity edge tests (cf. ~mbsiGeom_XYPolygonParity)
* @return true if polyline and polygon clash in xy-plane
* @bsimethod                                                    EarlinLutz      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    polylineClashPolygonXY

(
DPoint3dCP pXYZA,
int numA,
bool    bAddClosureA,
DPoint3dCP pXYZB,
int numB,
double tol
)
    {
    int iA0, iA1;
    double z0, z1;
    DPoint3d xyz;
    bool    bAddClosureB;

    bAddClosureB = !pXYZB[0].IsEqual (pXYZB[numB-1]);

    if (bAddClosureA)
        {
        iA0 = numA - 1;
        iA1 = 0;
        }
    else
        {
        iA0 = 0;
        iA1 = 1;
        }

    if (    !bAddClosureA
        && fabs (pXYZA[0].z) <= tol
        && bsiGeom_XYPolygonParity (&pXYZA[0], pXYZB, numB, tol) >= 0)
        return true;

    for (; iA1 < numA; iA0 = iA1++)
        {
        z0 = pXYZA[iA0].z;
        z1 = pXYZA[iA1].z;
        if (fabs (z0) <= tol && bsiGeom_XYPolygonParity (&pXYZA[iA1], pXYZB, numB, tol) >= 0)
            return true;

        if (fabs (z0) <= tol && fabs (z1) <= tol)
            {
            if (bsiDPoint3dArray_polylineClashXY (pXYZA + iA0, 2, false,
                            pXYZB, numB, bAddClosureB))
                return true;
            }
        else if (z0 * z1 < 0.0)
            {
            double s = z0 / (z0 - z1);
            xyz.Interpolate (pXYZA[iA0], s, pXYZA[iA1]);
            if (bsiGeom_XYPolygonParity (&xyz, pXYZB, numB, tol) >= 0)
                return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @description Test if a polyline pierces the plane of a polygon at a point interior to the polygon.
* @param pPointArrayA IN points of polyline.
* @param numA IN number of points for polyline.
* @param bAddClosureEdgeA IN whether to use an additional edge from the end to the start of the polyline.
*         Note that this closure edge does not make the polyline bound area for clash purposes.
* @param pPointArrayB IN points of polygon
* @param numB IN number of points for polygon
* @return true if polyline and polygon clash
* @group Polygons
* @bsimethod                                                    EarlinLutz      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      bsiDPoint3dArray_polylineClashPolygonXYZ

(
DPoint3dCP   pPointArrayA,
int numA,
bool    bAddClosureEdgeA,
const   DPoint3d *  pPointArrayB,
int numB
)
    {
    Transform worldToLocalB;
    Transform localToWorldB;
    DRange3d rangeA_localB, rangeB_localB;
    double largeCoordinate;
    double tol;
    DVec3d zvecB;


    DPoint3d *pXYZA_localB = (DPoint3d *)_alloca ((numA + 1) * sizeof (DPoint3d));
    DPoint3d *pXYZB_localB = (DPoint3d *)_alloca ((numB + 1) * sizeof (DPoint3d));
    DPoint3d *pXYZB_world = (DPoint3d *)_alloca ((numB + 1) * sizeof (DPoint3d));

    if (!prepPolygon (pXYZB_world, pXYZB_localB, &numB,
                    &worldToLocalB, &localToWorldB, &rangeB_localB,
                    pPointArrayB, numB))
        return false;

    worldToLocalB.Multiply (pXYZA_localB, pPointArrayA, numA);

    rangeA_localB.InitFrom(pXYZA_localB, numA);
    rangeB_localB.InitFrom(pXYZB_localB, numB);

    zvecB = worldToLocalB.GetMatrixColumn (2);

    largeCoordinate = rangeA_localB.LargestCoordinate ()
                    + rangeB_localB.LargestCoordinate ();

    if (!rangeA_localB.IntersectsWith (rangeB_localB))
        return false;

    tol = bsiTrig_smallAngle() * largeCoordinate;

    if (polylineClashPolygonXY
                (
                pXYZA_localB, numA, bAddClosureEdgeA,
                pXYZB_localB, numB, tol))
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @description Test if there is any point of contact between two polygons on arbitrary planes.
* @remarks If polygons are coplanar, look for any edge intersection or total containment;
*       if they are not coplanar, look for an edge of one polygon piercing the plane of the
*       other polygon at an interior point of the other polygon.
* @param pPointArrayA IN points of polygon A
* @param numA IN number of points for polygon A
* @param pPointArrayB IN points of polygon B
* @param numB IN number of points for polygon B
* @return true if polygons clash
* @group Polygons
* @bsimethod                                                    EarlinLutz      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      bsiDPoint3dArray_polygonClashXYZ

(
DPoint3dCP   pPointArrayA,
int numA,
const   DPoint3d *  pPointArrayB,
int numB
)
    {
    Transform worldToLocalA, worldToLocalB;
    Transform localToWorldA, localToWorldB;
    DRange3d rangeA_localA, rangeA_localB, rangeB_localA, rangeB_localB;
    DRange3d rangeAB_localA;
    double largeCoordinate;
    double tol;
    DVec3d zvecA, zvecB;

    DPoint3d *pXYZA_localA = (DPoint3d *)_alloca ((numA + 1) * sizeof (DPoint3d));
    DPoint3d *pXYZB_localB = (DPoint3d *)_alloca ((numB + 1) * sizeof (DPoint3d));

    DPoint3d *pXYZA_world = (DPoint3d *)_alloca ((numA + 1) * sizeof (DPoint3d));
    DPoint3d *pXYZB_world = (DPoint3d *)_alloca ((numB + 1) * sizeof (DPoint3d));

    DPoint3d *pXYZA_localB = (DPoint3d *)_alloca ((numA + 1) * sizeof (DPoint3d));
    DPoint3d *pXYZB_localA = (DPoint3d *)_alloca ((numB + 1) * sizeof (DPoint3d));

    if (!prepPolygon (pXYZA_world, pXYZA_localA, &numA,
                    &worldToLocalA, &localToWorldA, &rangeA_localA,
                    pPointArrayA, numA))
        return false;

    if (!prepPolygon (pXYZB_world, pXYZB_localB, &numB,
                    &worldToLocalB, &localToWorldB, &rangeB_localB,
                    pPointArrayB, numB))
        return false;

    worldToLocalB.Multiply (pXYZA_localB, pXYZA_world, numA);
    worldToLocalA.Multiply (pXYZB_localA, pXYZB_world, numB);

    rangeA_localB.InitFrom(pXYZA_localB, numA);
    rangeB_localA.InitFrom(pXYZB_localA, numB);

    zvecA = worldToLocalA.GetMatrixColumn (2);
    zvecB = worldToLocalB.GetMatrixColumn (2);

    largeCoordinate = rangeA_localA.LargestCoordinate ()
                    + rangeB_localA.LargestCoordinate ();

    if (!rangeA_localA.IntersectsWith (rangeB_localA))
        return false;
    if (!rangeA_localB.IntersectsWith (rangeB_localB))
        return false;

    tol = bsiTrig_smallAngle () * largeCoordinate;
    rangeAB_localA = DRange3d::FromUnion (rangeA_localA, rangeB_localA);

    if (zvecA.IsParallelTo(zvecB))
        {
        if (rangeAB_localA.high.z - rangeAB_localA.low.z < tol)
            {
            // Is first point of B inside A?
            if (bsiGeom_XYPolygonParity (&pXYZB_localA[0], pXYZA_localA, numA, tol) >= 0)
                return true;
            // Is first point of A inside B?

            if (bsiGeom_XYPolygonParity (&pXYZA_localB[0], pXYZB_localB, numB, tol) >= 0)
                return true;

            // Is there direct clash between edges?
            if (bsiDPoint3dArray_polylineClashXY
                        (
                        pXYZA_localA, numA, false,
                        pXYZB_localA, numB, false
                        ))
                return true;
            }
        }
    else
        {
        // Does any edge of B pierce A's plane within A?
        if (polylineClashPolygonXY
                    (
                    pXYZB_localA, numB, false,
                    pXYZA_localA, numA, tol))
            return true;
        // and vice versa
        if (polylineClashPolygonXY
                    (
                    pXYZA_localB, numA, false,
                    pXYZB_localB, numB, tol))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
@description For a space point and polygon, find the polygon vertex, edge, or interior point closest to the space point.
@param pClosestPoint    OUT     closest point
@param pXYZ             IN      polygon vertices
@param numXYZ           IN      number of polygon vertices
@param pSpacePoint      IN      point to project to polygon
@return Indicator of where the closest point lies:
    <ul>
    <li>-1 no data</li>
    <li>0 at polygon vertex</li>
    <li>1 on polygon edge</li>
    <li>2 in polygon interior</li>
    </ul>
@group Polygons
@bsimethod                                                    EarlinLutz      06/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiPolygon_closestPointExt
(
DPoint3dR closestPoint,
ptrdiff_t &edgeIndex,
double   &edgeFraction,
DPoint3dCP pXYZ,
int         numXYZ,
DPoint3dCR spacePoint
)
    {
    DVec3d normal;
    DPoint3d origin;
    DPoint3d planePoint;
    DPoint3d nearestPoint;
    DVec3d classificationVector;
    double area = bsiPolygon_polygonNormalAndArea (&normal, &origin, pXYZ, numXYZ);
    double h;
    int code;
    double edgeParam;
    int    edgeSelect;
    edgeIndex = -1;
    edgeFraction = 0.0;
    if (area <= 0.0)
        {
        closestPoint.Zero ();
        return -1;
        }

    h = spacePoint.DotDifference(origin, normal);
    planePoint.SumOf (spacePoint, normal, -h);

    code = bsiPolygon_closestEdgeToPoint (
                            &nearestPoint,
                            (DPoint3d*)&classificationVector,
                            &edgeParam,
                            &edgeSelect,
                            pXYZ, numXYZ,
                            &planePoint,
                            0.0);
    edgeIndex = (ptrdiff_t)edgeSelect;
    edgeFraction = edgeParam;
    if (code == 0 || code == 1)
        {
        closestPoint = nearestPoint;
        if (code == 0)
            edgeFraction = 0.0;
        }
    else if (code == 2 || code == 3)
        {
        if (classificationVector.DotProduct (normal) < 0.0)
            {
            // "outside".   Take the closest edge or vertex
            closestPoint = nearestPoint;
            code = 1;
            }
        else
            {
            closestPoint = planePoint;
            edgeIndex = -1;
            code = 2;
            }
        }
    return code;
    }

Public GEOMDLLIMPEXP int bsiPolygon_closestPoint
(
DPoint3dP pClosestPoint,
DPoint3dCP pXYZ,
int         numXYZ,
DPoint3dCP pSpacePoint
)
    {
    ptrdiff_t edgeIndex;
    double edgeFraction;
    DPoint3d closestPoint;
    int code = bsiPolygon_closestPointExt (closestPoint, edgeIndex, edgeFraction, pXYZ, numXYZ, *pSpacePoint);
    if (pClosestPoint)
        *pClosestPoint = closestPoint;
    return code;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     11/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static int cyclicStep

(
int i0,
int step,
int num
)
    {
    int i1 = i0 + step;
    while (i1 >= num)
        i1 -= num;
    while (i1 < 0)
        i1 += num;
    return i1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     11/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    savePoint

(
DPoint4dP pArray,
int      *pCount,
int      max,
DPoint3dCP pXYZ,
DRay3dCP pParameterizationRay
)
    {
    double param = pXYZ->DotDifference(pParameterizationRay->origin, pParameterizationRay->direction);

    if (*pCount < max)
        {
        pArray[*pCount].InitFrom (*pXYZ, param);
        *pCount += 1;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     11/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static int qsort_compareDPoint4dw

(
void const *vpA,
void const *vpB
)
    {
    DPoint4d const*pA = (DPoint4d const*)vpA;
    DPoint4d const*pB = (DPoint4d const*)vpB;
    if (pA->w < pB->w)
        return -1;
    if (pA->w > pB->w)
        return 1;
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley 06/2009.
+---------------+---------------+---------------+---------------+---------------+------*/
double      tolerancedDotDiff (DPoint3dCP point, DPoint3dCP origin, DVec3dCP direction, double *tolerance)
    {
    double  dot = point->DotDifference(*origin, *direction);
    return (NULL != tolerance && fabs (dot) < *tolerance)  ? 0.0 : dot;
    }

/*---------------------------------------------------------------------------------**//**
@description Clip an unbounded ray to a polygon as viewed perpendicular to a given vector.
@param pClipPoints      OUT     array of pairs of points, alternating between start and end of "in" segments.  MUST BE ALLOCATED BY CALLER TO HOLD maxClipPoints.
@param pClipParams      OUT     array of ray parameters of intersection points.  MUST BE ALLOCATED BY CALLER TO HOLD maxClipPoints.
@param pNumClipPoints   OUT     number of clip points returned
@param maxClipPoints    IN      size of preallocated output arrays
@param pXYZArray        IN      polygon points
@param numXYZ           IN      number of polygon points
@param pPolygonPerp     IN      normal of plane in which (virtual) intersections are computed
@param pRay             IN      unbounded ray to clip to polygon in plane
@param onTolerance      IN      if not NULL a tolerance to allow grazing rays to be output (used only by bsiPolygon_transverseIntersection).
@return false if ray has zero magnitude or if maxClipPoints is too small to house all computed clip points.
@group Polygons
@bsimethod                                                      EarlinLutz      04/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    polygonClipDRay3d
(
DPoint3dP   pClipPoints,
double      *pClipParams,
int         *pNumClipPoints,
int         maxClipPoints,
DPoint3dCP  pXYZArray,
int         numXYZ,
DVec3dCP    pPolygonPerp,
DRay3dCP    pRay,
double*     onTolerance
)
    {
    if (numXYZ < 2)
        return false;
    DVec3d      rayVector = pRay->direction;
    DRay3d      scaledRay = *pRay;
    DPoint3d    rayOrg = pRay->origin;
    DVec3d      rayPerp;
    bool        ok = true;
    double      amax, amin;
    double      a0;
    int         imin, imax, i, i0, i1, istart;
    double     *pAltitude = (double*)_alloca (numXYZ * sizeof (double));
    DPoint4d   *pCrossing = (DPoint4d *)_alloca (numXYZ * sizeof (DPoint4d));    // w = line parameter.
    int         numCrossing = 0;

    *pNumClipPoints = 0;

    if (!scaledRay.direction.SafeDivide (rayVector, rayVector.MagnitudeSquared ()))
        return false;

    rayPerp.CrossProduct (*pPolygonPerp, rayVector);

    amax = amin = pAltitude[0] = tolerancedDotDiff (&pXYZArray[0], &rayOrg, &rayPerp, onTolerance);
    imax = imin = 0;
    for (i = 1; i < numXYZ; i++)
        {
        double a;
        a = pAltitude[i] = tolerancedDotDiff (&pXYZArray[i], &rayOrg, &rayPerp, onTolerance);
        if (a > amax)
            {
            amax = a;
            imax = i;
            }
        else if (a < amin)
            {
            amin = a;
            imin = i;
            }
        }

    if (NULL == onTolerance)
        {
        if (amin >= 0.0 || amax <= 0.0)
            return true;
        }
    else
        {
        if (amin > 0.0 || amax < 0.0 || (amin == 0.0 && amax == 0.0))
            return true;
        }

    // We know sd polygon3d.cthat there are nonzeros.  If you take steps, you will find a nonzero.
    istart = i0 = fabs (amax) > fabs (amin) ? imax : imin;
    for (ok = true; ok;)
        {
        // a0 cannot be zero !!!
        a0 = pAltitude[i0];
        ok = true;
        i1 = cyclicStep (i0, 1, numXYZ);
        // walk to zero or positive....
        if (a0 < 0.0)
            while (pAltitude[i1] < 0.0 && i1 != istart)
                i1 = cyclicStep (i0 = i1, 1, numXYZ);
        else
            while (pAltitude[i1] > 0.0 && i1 != istart)
                i1 = cyclicStep (i0 = i1, 1, numXYZ);

        if (pAltitude[i1] == 0.0)
            {
            int     iStart = i1;
            i0 = i1;
            i1 = cyclicStep (i0, 1, numXYZ);
            // Walk along until not zero
            while (pAltitude[i1] == 0.0)
                i1 = cyclicStep (i0 = i1, 1, numXYZ);
            // i1 is finally off the axis.  Record i0 if i1 is a sign change from a0.
            if (pAltitude[i1] * a0 < 0.0)
                ok = savePoint (pCrossing, &numCrossing, numXYZ, &pXYZArray[i0], &scaledRay);
            else if (iStart != i0 && NULL != onTolerance)
                {
                ok = savePoint (pCrossing, &numCrossing, numXYZ, &pXYZArray[iStart], &scaledRay) &&
                     savePoint (pCrossing, &numCrossing, numXYZ, &pXYZArray[i0], &scaledRay);
                }
            }
        else if (pAltitude[i0] * pAltitude[i1] < 0.0)   // On return to istart, there might not be a crossing.
            {
            double s = -pAltitude[i0] / (pAltitude[i1] - pAltitude[i0]);
            DPoint3d xyz;
            xyz.Interpolate (pXYZArray[i0], s, pXYZArray[i1]);
                ok = savePoint (pCrossing, &numCrossing, numXYZ, &xyz, &scaledRay);
            }
        // Next pass will look forward from i1 ...
        i0 = i1;
        if (i0 == istart)
            break;
        }

    if (ok && numCrossing > 0)
        {
    qsort (pCrossing, numCrossing, sizeof (DPoint4d), qsort_compareDPoint4dw);
        ok = numCrossing <= maxClipPoints;
        if (numCrossing > maxClipPoints)
            numCrossing = maxClipPoints;
        *pNumClipPoints = numCrossing;
        if (pClipPoints)
            for (i = 0; i < numCrossing; i++)
                pClipPoints[i].Init (
                        pCrossing[i].x, pCrossing[i].y, pCrossing[i].z);
        if (pClipParams)
            for (i = 0; i < numCrossing; i++)
                pClipParams[i] = pCrossing[i].w;
        }
    return ok;
    }

/*---------------------------------------------------------------------------------**//**
@description Clip an unbounded ray to a polygon as viewed perpendicular to a given vector.
@param pClipPoints      OUT     array of pairs of points, alternating between start and end of "in" segments.  MUST BE ALLOCATED BY CALLER TO HOLD maxClipPoints.
@param pClipParams      OUT     array of ray parameters of intersection points.  MUST BE ALLOCATED BY CALLER TO HOLD maxClipPoints.
@param pNumClipPoints   OUT     number of clip points returned
@param maxClipPoints    IN      size of preallocated output arrays
@param pXYZArray        IN      polygon points
@param numXYZ           IN      number of polygon points
@param pPolygonPerp     IN      normal of plane in which (virtual) intersections are computed
@param pRay             IN      unbounded ray to clip to polygon in plane
@return false if ray has zero magnitude or if maxClipPoints is too small to house all computed clip points.
@group Polygons
@bsimethod                                                      EarlinLutz      04/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiPolygon_clipDRay3d

(
DPoint3dP   pClipPoints,
double      *pClipParams,
int         *pNumClipPoints,
int         maxClipPoints,
DPoint3dCP  pXYZArray,
int         numXYZ,
DVec3dCP    pPolygonPerp,
DRay3dCP    pRay
)
    {
    return polygonClipDRay3d (pClipPoints, pClipParams, pNumClipPoints, maxClipPoints, pXYZArray, numXYZ, pPolygonPerp, pRay, NULL);
    }

/*---------------------------------------------------------------------------------**//**
Stash parameter pair as x part of a new segment.
Increment index by TWO.
* @bsimethod                                    Earlin.Lutz                     11/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    acceptParamInterval

(
DSegment3dP pSegmentArray,
int        *pNumSegment,
int        maxSegment,
double     u0,
double     u1,
int         *pIndex
)
    {
    if (*pNumSegment >= maxSegment)
        return false;
    if (pIndex)
        *pIndex += 2;

    if (pSegmentArray)
        pSegmentArray[*pNumSegment].Init (u0, 0.0, 0.0, u1, 0.0, 0.0);
    *pNumSegment += 1;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
@description Compute the intersection segment(s) between two non-coplanar polygons.

@param pSegmentArray OUT array (ALLOCATED BY CALLER) of intersection segments
@param pNumSegment OUT number of intersection segments returned
@param pbParallelPlanes OUT true if polygon planes are parallel.  INTERSECTION NOT COMPUTED IN THIS CASE.
@param pNormalA OUT normal to plane of polygon A
@param pNormalB OUT normal to plane of polygon B
@param maxClipSegments IN allocated size of segment array
@param pXYZArrayA IN polygon A
@param numXYZA IN number of points on polygon A
@param pXYZArrayB IN polygon B
@param numXYZB IN number of points on polygon B
@return false if maxClipPoints is too small to house all computed intersections.
@group Polygons
@bsimethod                                                      EarlinLutz      04/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiPolygon_transverseIntersection

(
DSegment3dP pSegmentArray,
int         *pNumSegment,
bool        *pbParallelPlanes,
DVec3dP pNormalA,
DVec3dP pNormalB,
int         maxClipSegments,
DPoint3dCP pXYZArrayA,
int             numXYZA,
DPoint3dCP pXYZArrayB,
int             numXYZB
)
    {
    DPoint3d originA, originB;
    DVec3d   normalA, normalB;
    double   areaA,   areaB;
    int numParamA, numParamB;
    if (numXYZA < 2 || numXYZB < 2)
        return false;
    double *pParamA = (double*)_alloca (numXYZA * sizeof (double));
    double *pParamB = (double*)_alloca (numXYZB * sizeof (double));
    int iA, iB;
    int numSegment = 0;
    bool    ok = true;
    double      onTolerance = 1.0E-8;       // Small tolerance to accept grazing intersection with ray. Else adjacent polygons may not produce intersection with
                                            // a polygon intersecting at their commone edge. -TR# 274510.

    DRay3d ray;
    areaA = bsiPolygon_polygonNormalAndArea (&normalA, &originA, pXYZArrayA, numXYZA);
    areaB = bsiPolygon_polygonNormalAndArea (&normalB, &originB, pXYZArrayB, numXYZB);

    if (pNormalA)
        *pNormalA = normalA;
    if (pNormalB)
        *pNormalB = normalB;

    if (pNumSegment)
        *pNumSegment = 0;
    if (pbParallelPlanes)
        *pbParallelPlanes = false;

    if (normalA.IsParallelTo (normalB)
        || !bsiGeom_planePlaneIntersection (&ray.origin, &ray.direction,
                        &originA, &normalA, &originB, &normalB))
        {
        if (pbParallelPlanes)
            *pbParallelPlanes = true;

        return true;
        }

    if (!polygonClipDRay3d (NULL, pParamA, &numParamA, numXYZA, pXYZArrayA, numXYZA, &normalA, &ray, &onTolerance)
        || numParamA == 0)
        return true;

    if (!polygonClipDRay3d (NULL, pParamB, &numParamB, numXYZB, pXYZArrayB, numXYZB, &normalB, &ray, &onTolerance)
        || numParamB == 0)
        return true;


    // iA, iB are upper index of a segment pair.
    // Each loop pass advances exactly one of these indices.
    for (iA = iB = 1; ok && iA < numParamA && iB < numParamB && iA < numXYZA && iB < numXYZB;)
        {
        double uA0 = pParamA[iA-1];
        double uA1 = pParamA[iA];
        double uB0 = pParamB[iB-1];
        double uB1 = pParamB[iB];
        if (uA0 < uB0)
            {
            if (uA1 <= uB0)
                iA += 2;
            else if (uA1 <= uB1)
                ok = acceptParamInterval (pSegmentArray, &numSegment, maxClipSegments, uB0, uA1, &iA);
            else    // uA1 >= uB1.
                ok = acceptParamInterval (pSegmentArray, &numSegment, maxClipSegments, uB0, uB1, &iB);
            }
        else
            {
            if (uB1 <= uA0)
                iB += 2;
            else if (uB1 <= uA1)
                ok = acceptParamInterval (pSegmentArray, &numSegment, maxClipSegments, uA0, uB1, &iB);
            else
                ok = acceptParamInterval (pSegmentArray, &numSegment, maxClipSegments, uA0, uA1, &iA);
            }
        }

    // Convert parameters to points.
    if (pSegmentArray)
        {
        int i, j;
        for (i = 0; i < numSegment; i++)
            {
            for (j = 0; j < 2; j++)
                pSegmentArray[i].point[j] = ray.FractionParameterToPoint (pSegmentArray[i].point[j].x);
            }
        }
    if (pNumSegment)
        *pNumSegment = numSegment;
    return ok;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     12/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    buildTransformToLocal

(
TransformP pLocalToWorld,
TransformP pWorldToLocal,
DPoint3dCP pOrigin,
DVec3dCP pNormal
)
    {
    DVec3d unitX, unitY, unitZ;
    if (!pNormal->GetNormalizedTriad (unitX, unitY, unitZ))
        {
        pWorldToLocal->InitIdentity ();
        pLocalToWorld->InitIdentity ();
        return false;
        }

    pLocalToWorld->InitFromOriginAndVectors(*pOrigin, unitX, unitY, unitZ);
    pWorldToLocal->InvertRigidBodyTransformation (*pLocalToWorld);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
Look for vertex of B projecting within A.
* @bsimethod                                    Earlin.Lutz                     12/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static void    updateMinDistanceByVertexProjection

(
DPoint3dP pPointA,
DPoint3dP pPointB,
double   *pMinDistSquared,
DPoint3dCP pPolygonA,
DPoint3dP pLocalA,
int numA,
DVec3dCP pNormalA,
DPoint3dCP pPolygonB,
DPoint3dP pLocalB,
int numB
)
    {
    Transform   worldToLocalA, localToWorldA;
    int         iB;

    if (!buildTransformToLocal (&localToWorldA, &worldToLocalA, &pPolygonA[0], pNormalA))
        return;
    worldToLocalA.Multiply (pLocalA, pPolygonA, numA);
    worldToLocalA.Multiply (pLocalB, pPolygonB, numB);

    for (iB = 0; iB < numB; iB++)
        {
        double dzLeft, dzRight, z = pLocalB[iB].z;
        int iBLeft = iB - 1;
        int iBRight = iB + 1;

        if (iBLeft < 0)
            iBLeft += numB;
        if (iBRight >= numB)
            iBRight = 0;

        if (z * z > *pMinDistSquared)
            continue;

        dzLeft  = pLocalB[iBLeft].z - z;
        dzRight = pLocalB[iBRight].z - z;

        if (  (z > 0.0 && dzLeft >= 0.0 && dzRight >= 0.0)
           || (z < 0.0 && dzLeft <= 0.0 && dzRight <= 0.0)
           )
            {
            // Distance to plane is a local minimum.
            if (bsiGeom_XYPolygonParity (&pLocalB[iB], pLocalA, numA, 0.0) >= 0)
                {
                DPoint3d planePoint = pLocalB[iB];

                *pMinDistSquared = z * z;
                *pPointB = pPolygonB[iB];
                planePoint.z = 0.0;
                localToWorldA.Multiply (*pPointA, planePoint);
                }
            }
        }
    }

bool SetupLoopIndexing
(
DPoint3dCP source,
int n,
bool closed,
DPoint3dR point0,   //< OUT start point for first edge
int &index1         //< OUT end index for first edge
)
    {
    if (n < 2)
        return false;
    if (closed)
        {
        point0 = source[n- 1];
        index1 = 0;
        }
    else
        {
        point0 = source[0];
        index1 = 1;
        }
    return true;
    }
/*---------------------------------------------------------------------------------**//**
@description look for a closest approach between edges.
* @bsimethod                                    Earlin.Lutz                     12/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static void    updateMinDistanceByEdges

(
DPoint3dP pPointA,
DPoint3dP pPointB,
double   *pMinDistSquared,
DPoint3dCP pPolygonA,
int numA,
bool closedA,
DPoint3dCP pPolygonB,
int numB,
bool closedB
)
    {
    DSegment3d  segmentA, segmentB;
    int iAStart, iBStart;
    DPoint3d pointA0, pointB0;
    int iA, iB;
    if (!SetupLoopIndexing (pPolygonA, numA, closedA, pointA0, iAStart)
        || !SetupLoopIndexing (pPolygonB, numB, closedB, pointB0, iBStart))
        return;
    
    for (iA = iAStart, segmentA.point[0] = pointA0;
        iA < numA;
        iA++, segmentA.point[0] = segmentA.point[1]
        )
        {
        segmentA.point[1] = pPolygonA[iA];
        for (iB = iBStart, segmentB.point[0] = pointB0;
            iB < numB;
            iB++, segmentB.point[0] = segmentB.point[1]
            )
            {
            double  paramA, paramB;

            segmentB.point[1] = pPolygonB[iB];
            DPoint3d pointAA, pointBB;
            if (DSegment3d::ClosestApproachUnbounded (paramA, paramB, pointAA, pointBB, segmentA, segmentB))
                {
                double currDistSquared;

                // apply bounds
                if (paramA < 0.0)
                    pointAA = segmentA.point[0];
                if (paramA > 1.0)
                    pointAA = segmentA.point[1];

                if (paramB < 0.0)
                    pointBB = segmentB.point[0];
                if (paramB > 1.0)
                    pointBB = segmentB.point[1];

                currDistSquared = pointAA.DistanceSquared (pointBB);
                if (currDistSquared < *pMinDistSquared)
                    {
                    *pPointA = pointAA;
                    *pPointB = pointBB;
                    *pMinDistSquared = currDistSquared;
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
@description Search for the points where a pair of polygons makes closest approach.
@param pPointA OUT point on polygon A
@param pPointB OUT point on polygon B
@param pPolygonA IN vertices of polygon A
@param numA IN number of vertices of polygon A
@param pPolygonB IN vertices of polygon B
@param numB IN number of vertices of polygon B
@return true if polygon vertex counts are positive
@group Polygons
@bsimethod                                    Earlin.Lutz                     12/2005
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiPolygon_closestApproachBetweenPolygons

(
DPoint3dP pPointA,
DPoint3dP pPointB,
DPoint3dCP pPolygonA,
int numA,
DPoint3dCP pPolygonB,
int numB
)
    {
    DVec3d normalA, normalB;
    return bsiPolygon_closestApproachBetweenPolygons (pPointA, pPointB, &normalA, &normalB, pPolygonA, numA, pPolygonB, numB);
    }
/*---------------------------------------------------------------------------------**//**
@description Search for the points where a pair of polygons makes closest approach.
@param pPointA OUT point on polygon A
@param pPointB OUT point on polygon B
@param pNormalA OUT normal to polygon A
@param pNormalB OUT normal to polygon B
@param pPolygonA IN vertices of polygon A
@param numA IN number of vertices of polygon A
@param pPolygonB IN vertices of polygon B
@param numB IN number of vertices of polygon B
@return true if polygon vertex counts are positive
@group Polygons
@bsimethod                                    Earlin.Lutz                     12/2005
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiPolygon_closestApproachBetweenPolygons

(
DPoint3dP pPointA,
DPoint3dP pPointB,
DVec3dP pNormalA,
DVec3dP pNormalB,
DPoint3dCP pPolygonA,
int numA,
DPoint3dCP pPolygonB,
int numB
)
    {
    // Closest approach can be:
    //   1) Direct intersection.
    //      1a) Detect by finding transverse intersection segment.
    //      1b) Detect by finding edge pierce.
    //   2) EdgeA EdgeB
    //   3)Vertex of A over interior of B, or vice vera.
    //
    // If coplanar, there will be a case of 2 or 3.

    DSegment3d  segment;
    DPoint3d*   pLocalA, *pLocalB;
    double      minDistSquared;
    int         numSegment;
    bool        bParallel;

    if (numA <= 0 || numB <= 0)
        return false;

    *pPointA = pPolygonA[0];
    *pPointB = pPolygonB[0];

    if (numA == 1 && numB == 1)
        return true;

    // We only need one intersection segment.
    #define MAX_CLIP_SEGMENT 1

    // Direct intersection wins immediately:
    bsiPolygon_transverseIntersection (&segment, &numSegment, &bParallel, pNormalA, pNormalB, MAX_CLIP_SEGMENT, pPolygonA, numA, pPolygonB, numB);
    if (numSegment > 0)
        {
        *pPointA = *pPointB = segment.point[0];
        return true;
        }

    if (numA > 1 && pPolygonA[0].IsEqual (pPolygonA[numA-1]))
        numA -= 1;

    if (numB > 1 && pPolygonB[0].IsEqual (pPolygonB[numB-1]))
        numB -= 1;

    minDistSquared = pPointA->DistanceSquared (*pPointB);
    // Vertex projection
    pLocalA = (DPoint3d*)_alloca ( numA * sizeof (DPoint3d));
    pLocalB = (DPoint3d*)_alloca ( numB * sizeof (DPoint3d));

    updateMinDistanceByVertexProjection (pPointA, pPointB, &minDistSquared,
                pPolygonA, pLocalA, numA, pNormalA,
                pPolygonB, pLocalB, numB
                );

    updateMinDistanceByVertexProjection (pPointB, pPointA, &minDistSquared,
                pPolygonB, pLocalB, numB, pNormalB,
                pPolygonA, pLocalA, numA
                );

    updateMinDistanceByEdges (pPointA, pPointB, &minDistSquared, pPolygonA, numA, true, pPolygonB, numB, true);
    return true;
    }




Public GEOMDLLIMPEXP bool    bsiPolygon_closestApproachBetweenPolygonAndLineString

(
DPoint3dR pointA,
DPoint3dR pointB,
DPoint3dCP pPolygonA,
int numA,
DPoint3dCP pLinestringB,
int numB
)
    {
    // Closest approach can be:
    //   1) Direct intersection.
    //      1a) Detect by finding transverse intersection segment.
    //      1b) Detect by finding edge pierce.
    //   2) EdgeA EdgeB
    //   3)Vertex of A over interior of B, or vice vera.
    //
    // If coplanar, there will be a case of 2 or 3.

    double      minDistSquared;
    DVec3d normalA;
    if (numA <= 0 || numB <= 0)
        return false;

    pointA = pPolygonA[0];
    pointB = pLinestringB[0];

    if (numA == 1 && numB == 1)
        return true;

    // We only need one intersection segment.
    #define MAX_CLIP_SEGMENT 1
#ifdef abc
    DSegment3d  segment;
    DVec3d normalB;
    // Direct intersection wins immediately:
    int         numSegment;
    bool        bParallel;
    bsiPolygon_transverseIntersection (&segment, &numSegment, &bParallel, &normalA, &normalB, MAX_CLIP_SEGMENT, pPolygonA, numA, pPolygonB, numB);
    if (numSegment > 0)
        {
        pointA = pointB = segment.point[0];
        return true;
        }
#endif
    if (numA > 1 && pPolygonA[0].IsEqual (pPolygonA[numA-1]))
        numA -= 1;


    minDistSquared = pointA.DistanceSquared (pointB);
    // Vertex projection
    ScopedArray<DPoint3d> localA (numA);   DPoint3d * pLocalA = localA.GetData ();
    ScopedArray<DPoint3d> localB (numB);   DPoint3d * pLocalB = localB.GetData ();

    updateMinDistanceByVertexProjection (&pointA, &pointB, &minDistSquared,
                pPolygonA, pLocalA, numA, &normalA,
                pLinestringB, pLocalB, numB
                );
#ifdef abc
    updateMinDistanceByVertexProjection (pPointB, pointA, &minDistSquared,
                pPolygonB, pLocalB, numB, &normalB,
                pPolygonA, pLocalA, numA
                );
#endif
    updateMinDistanceByEdges (&pointA, &pointB, &minDistSquared, pPolygonA, numA, true, pLinestringB, numB, false);
    return true;
    }




/*---------------------------------------------------------------------------------**//**
Find transverse plane polygon intersections via paritiy crossings.
Call bsiPolygon_intersectDPlane3dExt to additional determine the all-on case.
@param    pXYZOut    OUT intersection segments, in start end pairs.
@param    pNumXYZOut OUT number of points (2x number of segments)
@param pbAllOn OUT true if all points are on the plane.   Points are NOTE copied to the output buffer.
@param    maxOut OUT max number of points (including disconnects) that can be returned.
@param    pXYZ        IN polygon vertices.
@param    numXYZ      IN number of vertices.
@return false if output buffer too small.  Note that zero crossings is a true return.
@param    pPlane      IN      plane
@group Polygons
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiPolygon_intersectDPlane3d
(
DPoint3d        *pXYZOut,
int             *pNumXYZOut,
int             maxOut,
DPoint3d        *pXYZ,
int             numXYZ,
DPlane3dCP      pPlane
)
    {
    double abstol;
    bool    bAllOn;
    static double sLocalRelTol  = 1.0e-10;
    static double sGlobalRelTol = 1.0e-14;
    abstol = sGlobalRelTol * (1.0 + bsiDPoint3d_getLargestCoordinate (pXYZ, numXYZ))
           + sLocalRelTol * bsiDPoint3d_getLargestCoordinateDifference (pXYZ, numXYZ);
    return bsiPolygon_intersectDPlane3dExt (pXYZOut, pNumXYZOut, &bAllOn,
                        maxOut, pXYZ, numXYZ, pPlane, abstol);
    }

/*---------------------------------------------------------------------------------**//**
Find plane polygon intersections via paritiy crossings.
@param    pXYZOut    OUT intersection segments, in start end pairs.
@param    pNumXYZOut OUT number of points (2x number of segments)
@param pbAllOn OUT true if all points are on the plane.   Points are NOT copied to the output buffer.
@param    maxOut OUT max number of points (including disconnects) that can be returned.
@param    pXYZ        IN polygon vertices.
@param    numXYZ      IN number of vertices.
@param tolerance IN tolerance for deciding if vertices are all on plane.
@return false if output buffer too small.  Note that zero crossings is a true return.
@param    pPlane      IN      plane
@group Polygons
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiPolygon_intersectDPlane3dExt
(
DPoint3d        *pXYZOut,
int             *pNumXYZOut,
bool            *pbAllOn,
int             maxOut,
DPoint3d        *pXYZ,
int             numXYZ,
DPlane3dCP      pPlane,
double          tolerance
)
    {
    int numCrossing = 0;
    bool    bOK = true;
    if (numXYZ < 2)
        return false;
    double    *pAltitude  = (double*  )_alloca (sizeof (double)   * (numXYZ));

    double hMin, hMax, h;
    int    iMin, iMax;

    if (pbAllOn)
        *pbAllOn = false;

    if (numXYZ < 1)
        {
        // Hmmm.. let it go out true+0
        }
    else
        {
        // Record all altitudes.  Find extremal altitudes.
        iMin = iMax = 0;
        hMin = hMax = pAltitude[0] = pXYZ[0].DotDifference (pPlane->origin, pPlane->normal);
        for (int i = 1; i < numXYZ; i++)
            {
            h = pAltitude[i] = pXYZ[i].DotDifference (pPlane->origin, pPlane->normal);
            if (h < hMin)
                {
                iMin = i;
                hMin = h;
                }
            if (h > hMax)
                {
                iMax = i;
                hMax = h;
                }
            }

        if (hMax <= tolerance && hMin >= -tolerance)
            {
            if (pbAllOn)
                *pbAllOn = true;
            }
        else if (hMin > tolerance)
            {
            // all above
            }
        else if (hMax < -tolerance)
            {
            // all below
            }
        else if (fabs (hMin) < tolerance || fabs (hMax) < tolerance)
            {
            // Just look for ON edges and record the start/end pairs.  If you double back and overlap, you get overlap.
            int i0, i1;
            for (i0 = numXYZ -1, i1 = 0; i1 < numXYZ; i0 = i1++)
                {
                if (fabs (pAltitude[i0]) <= tolerance && fabs (pAltitude[i1]) <= tolerance)
                    {
                    if (numCrossing + 2 <= maxOut)
                        {
                        pXYZOut[numCrossing++] = pXYZ[i0];
                        pXYZOut[numCrossing++] = pXYZ[i1];
                        }
                    }
                }
            }
        else if (hMin < 0.0 && hMax > 0.0)
            {
            // There are true crossings.
            //int side = -1;  // Start below
            double h0 = hMin;
            double h1;
            int i0 = iMin;
            int i1;
            numCrossing = 0;
            for (int i = 1; i <= numXYZ; i++, i0 = i1, h0 = h1)
                {
                i1 = i0 + 1;
                if (i1 >= numXYZ)
                    i1 = 0;
                h1 = pAltitude[i1];
                if (h0 * h1 <= 0.0 && h1 != 0.0)
                    {
                    if (numCrossing < maxOut)
                        {
                        // A crossing occured in this edge.
                        double s = -h0 / (h1 - h0);
                        pXYZOut[numCrossing++].Interpolate (pXYZ[i0], s, pXYZ[i1]);
                        }
                    else
                        bOK = false;
                    }
                }

            if (numCrossing > 2)
                bsiDPoint3dArray_sortAlongAnyDirection (pXYZOut, numCrossing);
            }
        }
    *pNumXYZOut = numCrossing;
    return bOK;
    }


class XYZCaptureBuffer
{
    int &mNumXYZ;
    int mMaxXYZ;
    DPoint3d *mpXYZ;
public:
XYZCaptureBuffer (DPoint3d *pXYZ, int &numXYZ, int maxXYZ)
    : mpXYZ(pXYZ), mNumXYZ(numXYZ), mMaxXYZ(maxXYZ)
    {
    mNumXYZ = 0;
    }
bool Add (DPoint3dCR xyz)
    {
    if (mNumXYZ < mMaxXYZ)
        mpXYZ[mNumXYZ++] = xyz;
    return true;
    }
bool AddDisconnect ()
    {
    DPoint3d xyz;
    xyz.InitDisconnect ();
    return Add (xyz);
    }
};

struct ClipCrossing
    {
public:
    DPoint3d xyz;
    double sortKey;
    int mId;               // original index (position) of this crossing record in the crossing array.
    int loopBaseIndex;     // base index of loop in global xyz array.
    int loopCount;         // number of points in this loop.
    int pointIndex0;       // First off-plane point index, counted from the base index !!!
                           // Direction and count to be determined at mate.
    int nextCrossing;      // After sorting, these define the loops.
    unsigned int mask;     // bits.

void init
(
DPoint3dCR _xyz,
int id,
int i0,
int baseIndex,
int count
)
    {
    xyz = _xyz;
    mId = id;
    pointIndex0 = i0;
    loopBaseIndex = baseIndex;
    loopCount = count;
    }

    };


int cb_compareCrossings
(
const void *vpCrossing0,
const void *vpCrossing1
)
    {
    const ClipCrossing *pCrossing0 = (const ClipCrossing*)vpCrossing0;
    const ClipCrossing *pCrossing1 = (const ClipCrossing*)vpCrossing1;
    double a = pCrossing0->sortKey - pCrossing1->sortKey;
    if (a < 0.0)
        return -1;
    else if (a > 0.0)
        return 1;
    else
        {
        // hm.  Not sure what should happen here.
        return 0;
        }
    }

#define MAX_CLIP_CROSSING 1000
struct ClipCrossingSorter
{
private:
    ClipCrossing mCrossing[MAX_CLIP_CROSSING];
    int          mCrossingIndex[MAX_CLIP_CROSSING];
    int         mNumCrossing;


    DPoint3dP mpXYZOut;     // CAPTURED pointer to output buffer
    int &mNumXYZOut;        // CAPTURED reference to caller's point count.
    int &mNumLoop;          // CAPTURED reference to caller's loop count.
    int mMaxOut;            // buffer size limit, from caller
    DPoint3dP mpXYZ;        // CAPTURED pointer to input buffer.
    int mNumXYZ;            // input size, from caller
    DPlane3d    mPlane;     // plane, normal pointing outwards.

void AddCrossing
(
DPoint3dCR xyz,
int i0,
int baseIndex,
int count
)
    {
    if (mNumCrossing < MAX_CLIP_CROSSING)
        {
        mCrossing[mNumCrossing].init (xyz, mNumCrossing, i0, baseIndex, count);
        mNumCrossing++;
        }
    }


// Define crossings for a new fragment ...
void AddFragment
(
DPoint3dCR xyz0,    // OnPlane start coordinate
int i0,             // first above-plane index
int i1,             // last above-plane index
DPoint3dCR xyz1,     // OnPlane end coordinate
int baseIndex,
int count
)
    {
    // Even-odd pairings will distinguish start from end !!!
    AddCrossing (xyz0, i0, baseIndex, count);
    AddCrossing (xyz1, i1, baseIndex, count);
    }

// Evaluate altitude above the plane ...
double altitude (DPoint3dCR xyz)
    {
    return xyz.DotDifference(mPlane.origin, mPlane.normal);
    }

void GetCrossingCoordinates
(
DPoint3dR xyz,
DPoint3dCR xyz0,
double h0,
DPoint3dCR xyz1,
double h1
)
    {
    double s;
    DoubleOps::SafeDivide (s, h0, h0 - h1, 0.0);
    xyz.Interpolate (xyz0, s, xyz1);
    }

// Load loop from index 0 to count-1 and back to 0.  Precheck for trailing dups.
void LoadOneLoop
(
DPoint3d *pXYZ,
int baseIndex,
int count
)
    {
    // strip trailing duplicates ...
    for (;count > 1 && pXYZ[0].IsEqual (pXYZ[count-1]); count--)
        {
        }
    if (count < 2)
        return;

    // Find a negative place to start ...
    int startIndex = -1;
    int numNegative = 0;
    int numPositive = 0;
    for (int i = 0; i < count; i++)
        {
        double h = -altitude(pXYZ[i]);
        if (h < 0.0)
            {
            startIndex = i;
            numNegative++;
            }
        else if (h > 0.0)
            {
            numPositive++;
            }
        }

    if (numNegative == 0)
        {
        // All on or above ...
        for (int i = 0; i < count; i++)
            AddVertex (pXYZ[i]);
        AddVertex (pXYZ[0]);
        AddDisconnect ();
        return;
        }
    else if (numPositive == 0)
        {
        // All on or below ...
        return;
        }


    // Look for crossing from 0 or negative to strictly positive ..
    int i0 = startIndex, i1;
    double h0 = -altitude (pXYZ[i0]);
    double h1;

    // i0 is always at a zero or negative point !!!
    // there is always a strictly negative point "ahead" to stop it all
    // i0 and i1 are properly restrained to period.
    for (int numTested = 0;
            numTested < count;
            i0 = i1, h0 = h1)
        {
        i1 = (i0 + 1) % count;
        h1 = -altitude (pXYZ[i1]);
        numTested++;

        if (h1 > 0.0)
            {
            // [h0..h1) has a crossing
            DPoint3d xyzA, xyzB;
            GetCrossingCoordinates (xyzA, pXYZ[i0], h0, pXYZ[i1], h1);
            // Walk forward (with wraparound) for a down crossing.
            // guaranteed to terminate -- eventually has to hit the strictly start index with strictly negative h1.
            int fragmentStartIndex = i1;
            for (;;)
                {
                i0 = i1;
                h0 = h1;
                i1 = (i0 + 1) % count;
                h1 = -altitude(pXYZ[i1]);
                numTested++;
                if (h1 < 0.0)
                    {
                    // i1A..i0 is the fragment, in increasing index order (needs to be rewrapped)
                    GetCrossingCoordinates (xyzB, pXYZ[i0], h0, pXYZ[i1], h1);
                    AddFragment (xyzA, fragmentStartIndex, i0, xyzB, baseIndex, count);
                    while (h1 == 0.0)
                        {
                        i0 = i1;
                        h0 = h1;
                        i1 = (i0 + 1) % count;
                        h1 = -altitude(pXYZ[i1]);
                        numTested++;
                        }
                    break;
                    }
                }
            }
        }
    }

public:
ClipCrossingSorter
(
DPoint3d        *pXYZOut,
int             &numXYZOut,
int             &numLoop,
int             maxOut,
DPoint3d        *pXYZ,
int             numXYZ,
DPlane3dCR      plane
) :
    mpXYZOut(pXYZOut),
    mNumXYZOut(numXYZOut),
    mNumLoop(numLoop),
    mMaxOut(maxOut),
    mpXYZ(pXYZ),
    mNumXYZ(numXYZ),
    mPlane(plane)
    {
    mNumCrossing = 0;
    mNumXYZOut = mNumLoop = 0;
    }

void LoadLoops
(
)
    {
    // For each loop ...
    int i1;     // index that seeks a spot "after a loop" -- disconnect or out of range.
    int i0 = 0; // "start" of loop
    for (i1 = 0; i0 <= mNumXYZ; i1++)
        {
        if (i1 == mNumXYZ || mpXYZ[i1].IsDisconnect ())
            {
            LoadOneLoop (mpXYZ + i0, i0, i1 - i0);
            i0 = i1 + 1;
            }
        }
    }

// Prior to call ...
//   Each crossing contains its coordinates.
//   Each crossing contains its positional index.  (Probably 0,1, etc, but any permutation is ok)
bool SortCrossings ()
    {
    if (mNumCrossing < 2)
        return false;
    DRange3d range;
    range.Init ();
    for (int i = 0; i < mNumCrossing; i++)
        {
        range.Extend (mCrossing[i].xyz);
        }
    DVec3d diagonal;
    diagonal.DifferenceOf (range.high, range.low);
    int iMax = bsiDVec3d_dominantComponentIndex (&diagonal);
    for (int i = 0; i < mNumCrossing; i++)
        {
        mCrossing[i].sortKey = mCrossing[i].xyz.GetComponent (iMax);
        }
    qsort (mCrossing, mNumCrossing, sizeof (ClipCrossing), cb_compareCrossings);
    // Let id be a pre-sort position.
    // Make mCrossingIndex[id] = sorted position of that mCrossing.
    for (int i = 0; i < mNumCrossing; i++)
        mCrossingIndex [i] = -1;

    for (int i = 0; i < mNumCrossing; i++)
        {
        mCrossing[i].mask = 0;
        int j = mCrossing[i].mId;
        mCrossing[i].nextCrossing = -1;
        if (0 <= j && j < mNumCrossing && mCrossingIndex[j] == -1)
            mCrossingIndex[j] = i;
        else
            return false;  // Bad indices -- not a permutation
        }

    // relink original pairs ...
    for (int i = 0; i < mNumCrossing; i += 2)
        {
        int k0 = mCrossingIndex[i];
        int k1 = mCrossingIndex[i+1];
        mCrossing[k0].nextCrossing = k1;
        mCrossing[k1].nextCrossing = k0;
        }

    // Walk from left.
    // At each unvisited crossing i0 with (necessarily unvisited) mate i1 ..
    //   jump forward to unvisited
    //   walk backward, pulling crossing pairs j0,j1 that go backwards.
    //   Skip over any oddities
    for (int i0 = 0; i0 < mNumCrossing; i0++)
        {
        if (mCrossing[i0].mask != 0)
            continue;
        int i1 = mCrossing[i0].nextCrossing;    // must be > i0 -- prior visit to i0 would have marked it.
        mCrossing[i0].mask = mCrossing[i1].mask = 1;
        int j0 = i1 - 1;
        while (j0 > i0)
            {
            int j1 = mCrossing[j0].nextCrossing;
            if (mCrossing[j0].mask != 0 || j1 > j0)
                j0--;
            else
                {
                mCrossing[i1].nextCrossing = j0;
                mCrossing[j1].nextCrossing = i0;
                mCrossing[j0].mask = mCrossing[j1].mask = 1;
                j0 = j1 - 1;
                i1 = j1;
                }
            }
        }
    return true;
    }

void AddDisconnect
(
)
    {
    if (mNumXYZOut < mMaxOut)
        {
        mpXYZOut [mNumXYZOut++].InitDisconnect ();
        mNumLoop++;
        }
    }

void AddVertex
(
DPoint3dCR  xyz
)
    {
    if (mNumXYZOut < mMaxOut)
        mpXYZOut [mNumXYZOut++] = xyz;
    }


void ExtractLoops
(
)
    {
    mNumLoop = 0;
    for (int i = 0; i < mNumCrossing; i++)
        mCrossing[i].mask = 0;

    for (int i = 0; i < mNumCrossing; i++)
        {
        if (mCrossing[i].mask == 0)
            {
            int k0 = i;
            //DPoint3d xyz0 = mCrossing[k0].xyz;
            while (mCrossing[k0].mask == 0)
                {
                int k1 = mCrossing[k0].nextCrossing;
                mCrossing[k0].mask = mCrossing[k1].mask = 1;
                AddVertex (mCrossing[k0].xyz);
                int m0 = mCrossing[k0].pointIndex0;
                int m1 = mCrossing[k1].pointIndex0;
                int mBase = mCrossing[k0].loopBaseIndex;
                int mPeriod = mCrossing[k0].loopCount;
                // REMARK -- the two crossings came from the same loop -- must have same base and period.
                int count, step;

                if ((mCrossing[k0].mId & 0x01) == 0)
                    {
                    count = m1 - m0 + 1;
                    step = 1;
                    }
                else
                    {
                    count = m0 - m1 + 1;
                    step = -1;
                    }

                if (count < 0)
                    count += mPeriod;

                for (int nStep = 0; nStep < count; nStep++)
                    {
                    int m = m0 + nStep * step;
                    while (m >= mPeriod)
                        m -= mPeriod;
                    while (m < 0)
                        m += mPeriod;
                    AddVertex (mpXYZ[mBase + m]);
                    }

                AddVertex (mCrossing[k1].xyz);

                mCrossing[k0].mask = 1;
                mCrossing[k1].mask = 1;

                k0 = mCrossing[k1].nextCrossing;
                }
            // Closure vertex ...
            AddVertex (mCrossing[k0].xyz);
            AddDisconnect ();
            }
        }
    }
};

/*---------------------------------------------------------------------------------**//**
* Clip loops to a plane.  NO OPTIONAL POINTERS.
@param    pXYZOut    OUT "interior" loops, separated by disconnects.
@param    pNumXYZOut OUT number of points (including disconnects)
@param    pNumLoop OUT number of loops
@param    maxOut OUT max number of points (including disconnects) that can be returned.
@param    pXYZ        IN polygon vertices.  Separate multiple loops with disconnect points.
@param    numXYZ      IN number of vertices.
@param    pPlane      IN plane with normal pointing to outside.
@group Polygons
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiPolygon_clipToPlane
(
DPoint3d        *pXYZOut,
int             *pNumXYZOut,
int             *pNumLoop,
int             maxOut,
DPoint3d        *pXYZ,
int             numXYZ,
DPlane3dCP      pPlane
)
    {
    ClipCrossingSorter context(pXYZOut, *pNumXYZOut, *pNumLoop, maxOut,
                pXYZ, numXYZ,
                *pPlane);
    *pNumXYZOut = 0;
    context.LoadLoops ();
    context.SortCrossings ();
    context.ExtractLoops ();
    }

#ifdef abc
/*---------------------------------------------------------------------------------**//**
@description Clip linestrings to a plane.
@param    pXYZOut    OUT interior linestrings, separated by disconnects.
@param    pNumXYZOut OUT number of points (2x number of segments)
@param    pNumStringOut OUT number of distinct linestrings in return buffer.
@param    maxOut OUT max number of points (including disconnects) that can be returned.
@param    pXYZ        IN polygon vertices.  Separate multiple loops with disconnect points.
@param    numXYZ      IN number of vertices.
@param    pPlane      IN plane with normal pointing to outside.
@group Polygons
+---------------+---------------+---------------+---------------+---------------+------*/
Publicx GEOMDLLIMPEXP void bsiPolyline_clipToPlane
(
DPoint3d        *pXYZOut,
int             *pNumXYZOut,
int             *pNumStringOut,
int             maxOut,
DPoint3d        *pXYZ,
int             numXYZ,
DPlane3dCP      pPlane
)
    {
    XYZCaptureBuffer buffer(pXYZOut, *pNumXYZOut, maxOut);
    double h0, h1;
    int i0, i1, iA;
    for (int i0 = 0; i0 < numXYZ;)
        {
        if (pXYZ[i]->IsDisconnect ())
            {
            i0++;
            }
        else
            {
            h0 = pXYZ[i0].DotDifference(pPlane->origin, pPlane->normal);
            iA = i0;
            if (h0 > 0.0
                {
                while (
                }
            else
                {
                buffer.Add (pXYZ[i0];
                for (i1 = i0 + 1; i1 < numXYZ && !pXYZ[i1].IsDisconnect (); h0 = h1, i0 = i1++)
                    {
                    h1 = pXYZ[i1].DotDifference(pPlane->origin, pPlane->normal);
                    if (h1 <= 0.0)
                        buffer.Add (pXYZ[i1]);
                    else
                        {
                        DPoint3d xyzI;
                        double s;
                        DoubleOps::SafeDivide (s, -h0, h1 - h0, 0.0);
                        xyzI.Interpolate (pXYZ[i0], s, pXYZ[i1]);
                        buffer.Add (xyzI);
                        i0 = i1;
                        break;
                        }
                    }
                }
            }
        }
    }
#endif

END_BENTLEY_GEOMETRY_NAMESPACE
