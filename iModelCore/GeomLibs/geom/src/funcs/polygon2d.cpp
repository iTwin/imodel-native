/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/polygon2d.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
#define FIX_MINMAX(value, min, max)  FIX_MIN(value, min); else FIX_MAX(value, max)

/*----------------------------------------------------------------------+
|                                                                       |
|   external variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|       local function definitions                                      |
|                                                                       |
+----------------------------------------------------------------------*/



/*---------------------------------------------------------------------------------**//**
* Test if a point is within a convex region.
*
* @param    pPoint      => point to test
* @param    pPointArray => boundary points
* @param    numPoint    => number of points
* @param    sense       =>   0 if sense not known, should be determined internally
*                                   by area calculation.
*                            1 if known to be counterclockwise
*                           -1 if known to be clockwise
* @return true if the point is in the region
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint2d_isPointInConvexPolygon

(
DPoint2dCP pPoint,
DPoint2dCP pPointArray,
int             numPoint,
int             sense
)

    {
    static int linearSearchLimit = 5;
    double areaFactor = (double)sense;
    double dot;
    int i1, i0;

    if (numPoint < 3)
        return  false;
    if (sense == 0)
        areaFactor = bsiDPoint2d_getPolygonArea (pPointArray, numPoint);

    if (numPoint < linearSearchLimit)
        {
        for (i0 = numPoint - 1, i1 = 0; i1 < numPoint; i0 = i1++)
            {
            dot = pPoint->CrossProductToPoints (pPointArray[i0], pPointArray[i1]);
            if (dot * areaFactor < 0.0)
                return false;
            }
        return true;
        }
    else
        {
        int iLeft, iRight, iMid;
        iLeft = 0;
        iRight = numPoint;
        /* We know  .... iLeft < iRight
                        iMid <= numPoint -1, so it can be used as an index without checking
            point is IN iff it is to the left of all edges from iRight to iLeft.
        */
        while (iLeft > iRight + 1)
            {
            iMid = (iRight + iLeft) >> 1;
            dot = pPoint->CrossProductToPoints (pPointArray[iLeft], pPointArray[iMid]);
            if (dot * areaFactor >= 0)
                iLeft = iMid;
            else
                iRight = iMid;
            }

        /* Known: iLeft == iRight + 1. This edge determines in/out */
        if (iRight == numPoint)
            iRight = 0;
        dot = pPoint->CrossProductToPoints (pPointArray[iLeft], pPointArray[iRight]);
        if (areaFactor * dot >= 0.0)
            {
            return true;
            }
        return false;
        }

    }


/*---------------------------------------------------------------------------------**//**
*
* @param    pPoint      => point to test
* @param    pPointArray => boundary points
* @param    numPoint    => number of points
* @param    sense       =>   0 if sense not known
*                            1 if known to be counterclockwise
*                           -1 if known to be clockwise
* @return true if the point is in the region
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_getPolygonArea

(
DPoint2dCP pPointArray,
int             numPoint
)
    {
    return PolygonOps::Area (pPointArray, numPoint);
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiDPoint2d_PolygonParity_yTest                 |
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
static bool    bsiDPoint2d_PolygonParity_yTest

(
int         *pParity,       /* <= parity result, if successful */
DPoint2dCP pPoint,          /* => point to test */
DPoint2dCP pPointArray,   /* => polygon points */
int       numPoint,         /* number of planes */
double      tol             /* tolerance for ON case detection */
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
| name          bsiDPoint2d_PolygonParity_xTest                 |
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
static bool    bsiDPoint2d_PolygonParity_xTest

(
int         *pParity,       /* <= parity result, if successful */
DPoint2dCP pPoint,          /* => point to test */
DPoint2dCP pPointArray,   /* => polygon points */
int       numPoint,         /* number of planes */
double      tol             /* tolerance for ON case detection */
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

static double bsiDPoint2d_dotProductToPoint

(
DPoint2dCP pVector,
DPoint2dCP pBasePoint,
DPoint2dCP pTargetPoint
)
    {
    return    pVector->x * (pTargetPoint->x - pBasePoint->x)
            + pVector->y * (pTargetPoint->y - pBasePoint->y);
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiDPoint2d_PolygonParity_xTest                 |
|                                                                       |
| author        EarlinLutz                              04/97           |
|                                                                       |
| Classify a point with respect to a polygon defined by the xy          |
| parts of the points, using a given ray cast direction.                |
| Return false (failure, could not determine answer) if any polygon     |
| point is on the ray.                                                  |
+----------------------------------------------------------------------*/
static bool    bsiDPoint2d_PolygonParity_vectorTest

(
int         *pParity,       /* <= parity result, if successful */
DPoint2dCP pPoint,          /* => point to test */
double      theta,          /* => angle for ray cast */
DPoint2dCP pPointArray,     /* => polygon points */
int       numPoint,         /* number of planes */
double      tol             /* tolerance for ON case detection */
)
    {
    DPoint2d tangent, normal;
    double v0, v1;
    double u0, u1;
    double u;
    double s;
    int numLeft = 0, numRight = 0;

    int i, i0;

    tangent.x = cos (theta);
    tangent.y = sin (theta);
    normal.x = - tangent.y;
    normal.y =   tangent.x;

    v0 = bsiDPoint2d_dotProductToPoint (&normal, pPoint, &pPointArray[numPoint - 1]);

    if (fabs (v0) <= tol)
        return false;
    for (i = 0; i < numPoint; i++, v0 = v1)
        {
        v1 = bsiDPoint2d_dotProductToPoint (&normal, pPoint, &pPointArray[i]);
        if (fabs (v1) <= tol)
            return  false;
        if (v0 * v1 < 0.0)
            {
            s = -v0 / (v1 - v0);
            i0 = i - 1;
            if (i0 < 0)
                i0 = numPoint - 1;
            u0 = bsiDPoint2d_dotProductToPoint (&tangent, pPoint, &pPointArray[i0]);
            u1 = bsiDPoint2d_dotProductToPoint (&tangent, pPoint, &pPointArray[i]);
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


/*---------------------------------------------------------------------------------**//**
* Classify a point with respect to a polygon.
* @param pPoint => point to test
* @param pPointArray => polygon points.  Last point does not need to be duplicated.
* @param numPoint => number of points.
* @param tol => tolerance for "on" detection. May be zero.
* @return 1 if point is "in" by parity, 0 if "on", -1 if "out", -2 if nothing worked.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDPoint2d_PolygonParity

(
DPoint2dCP pPoint,
DPoint2dCP pPointArray,
int       numPoint,
double      tol
)
    {
    int i, parity;
    double x = pPoint->x, y = pPoint->y, theta, dTheta, maxTheta;

    if (numPoint < 2)
        return (fabs (x - pPointArray[0].x) <= tol && fabs (y - pPointArray[0].y) <= tol) ? 0 : -1;

    /* Try really easy ways first ... */
    if ( bsiDPoint2d_PolygonParity_yTest (&parity, pPoint, pPointArray, numPoint, tol))
        return  parity;
    if ( bsiDPoint2d_PolygonParity_xTest (&parity, pPoint, pPointArray, numPoint, tol))
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
        if (bsiDPoint2d_PolygonParity_vectorTest (&parity, pPoint, theta, pPointArray, numPoint, tol))
            return parity;
        theta += dTheta;
        }
    return -2;
    }

/*---------------------------------------------------------------------------------**//**
* Construct a by point and perpendicular.
* Find intersections with polygon edges.
* If intersections are all trivial, return an "in" point by parity rules.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
static  bool    bsiDPoint2d_findAnyInteriorPointOnPerpendicular

(
DPoint2dP pPoint,
DPoint2dCP pPointArray,
int       numPoint,
DPoint2dCP pBasePoint,
DPoint2dCP pPerp
)
    {
    DPoint2d sortVector, crossingPoint;
    int i0, i1;
    double sortParameter[3];
    double lineFraction;
    double currSortParameter;
    int numSortParameter = 0;
    double h0, h1, product;
    int k;

    bsiDPoint2d_rotate90 (&sortVector, pPerp);
    if (bsiDPoint2d_normalize (&sortVector) == 0.0)
        return false;

    h0 = bsiDPoint2d_dotProductToPoint (pPerp, pBasePoint, pPointArray + numPoint - 1);
    for (i0 = numPoint - 1, i1 = 0; i1 < numPoint; i0 = i1++, h0 = h1)
        {
        h1 = bsiDPoint2d_dotProductToPoint (pPerp, pBasePoint, pPointArray + i1);
        product = h0 * h1;
        if (product == 0.0)
            {
            /* The perpendicular line cuts directly through the vertex.  Give up. */
            return false;
            }
        else if (product < 0.0)
            {
            /* simple crossing. */
            lineFraction = -h0 / (h1 - h0);
            bsiDPoint2d_interpolate
                                (
                                &crossingPoint,
                                pPointArray + i0,
                                lineFraction,
                                pPointArray + i1
                                );
            currSortParameter = bsiDPoint2d_dotProductToPoint (&sortVector, pBasePoint, &crossingPoint);
            /* Sort paramter Array contains up to 2 left most (algebraically minimal)
                crossing parameters.  See if the new sort parameter goes in the array.
            */
            sortParameter[numSortParameter] = currSortParameter;
            for (k = 0;
                k < numSortParameter && sortParameter[k] < currSortParameter;
                k++
                )
                {
                }

            /* Insert new paramter as second lowest or lowest parameter. */
            if (k == 1)
                {
                sortParameter[1] = currSortParameter;
                numSortParameter = 2;
                }
            else if (k == 0)
                {
                if (numSortParameter > 0)
                    sortParameter[1] = sortParameter[0];
                sortParameter[0] = currSortParameter;
                if (numSortParameter < 2)
                    numSortParameter ++;
                }
            }
        }

    if (numSortParameter == 2 && sortParameter[0] < sortParameter[1])
        {
        bsiDPoint2d_addScaledDPoint2d (pPoint,
                        pBasePoint, &sortVector,
                        0.5 * (sortParameter[0] + sortParameter[1])
                        );
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Find any point interior to the polygon.
* @param pPoint <= computed point
* @param pPointArray => polygon points.  Last point does not need to be duplicated.
* @param numPoint => number of points.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint2d_findAnyInteriorPoint

(
DPoint2dP pPoint,
DPoint2dCP pPointArray,
int       numPoint
)
    {
    int i1, i0;
    DPoint2d tangent, edgePoint;
    double fraction0 = 0.4157623462;
    double fractionDelta = 0.0712878768796;
    double fraction;

    /* Try shootings rays sideways from points a nontrivial mid-edge positions.
       This will nearly always end on the first try with a nonzero tangent;
       Failures can occur if the polygon has overlapping boundary lines.
    */
    for (i0 = numPoint - 1, i1 = 0; i1 < numPoint; i0 = i1++)
        {
        bsiDPoint2d_subtractDPoint2dDPoint2d (&tangent, pPointArray + i1, pPointArray + i0);
        if (bsiDPoint2d_maxAbs (&tangent) != 0.0)
            {
            fraction = fraction0 + (fractionDelta * (double)i1) / (double)numPoint;
            bsiDPoint2d_addScaledDPoint2d (&edgePoint, pPointArray + i0, &tangent, fraction);
            if (bsiDPoint2d_findAnyInteriorPointOnPerpendicular (pPoint, pPointArray, numPoint, &edgePoint, &tangent))
                return true;
            }
        }
    return false;
    }END_BENTLEY_GEOMETRY_NAMESPACE
