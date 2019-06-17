/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*----------------------------------------------------------------------+
|                                                                       |
|   macro definitions                                                   |
|                                                                       |
+----------------------------------------------------------------------*/
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




/*-----------------------------------------------------------------*//**
* Test point a point is inside a convex region bounded by planes.
* @param pPoint => point to test.
* @param pPlaneArray => array of plane coefficients.  A zero or negative
*       value of the dot product (x,y,z,1) * (a,b,c,d)
*   is considered "in".
* @param numPlane => number of planes
* @return true if the point is in the region.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiHVec_isPointInConvexRegion

(
DPoint3dCP pPoint,          /* => point to test */
DPoint4dCP pPlaneArray,   /* => plane equations */
int         numPlane        /* number of planes */
)

    {
    int i;
    double dot;
    double x = pPoint->x;
    double y = pPoint->y;
    double z = pPoint->z;
    const DPoint4d *pPlane;
    for (i = 0; i < numPlane; i++)
        {
        pPlane = pPlaneArray + i;
        dot = pPlane->w + pPlane->x * x + pPlane->y * y + pPlane->z * z;
        if (dot > 0.0)
            return false;
        }
    return true;
    }



/*-----------------------------------------------------------------*//**
* Test points in an array to see if they are inside a convex
* region bounded by planes.
* @param pPointArray => array of points to test.
* @param numPoint => number of points
* @param pPlaneArray => array of plane coefficients.  A zero or negative
*       value of the dot product (x,y,z,1) * (a,b,c,d)
*   is considered "in".
* @param numPlane => number of planes
* @return the number of points which are "in" all the planar halfspaces.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiHVec_numLeadingPointsInConvexRegion

(
DPoint3dCP pPointArray,   /* => point to test */
int         numPoint,       /* => number of points in array */
DPoint4dCP pPlaneArray,   /* => plane equations */
int         numPlane        /* number of planes */
)

    {
    int n = 0;
    int i;
    for (i = 0;
         i < numPoint && bsiHVec_isPointInConvexRegion
                                    (
                                    pPointArray + i,
                                    pPlaneArray,
                                    numPlane
                                    );
         i++
        )
        {
        n++;
        }
    return n;
    }


/*-----------------------------------------------------------------*//**
* Search an array for the closest point, using only
* x and y components.  Useful for screenproximity
* tests between points at different depths.
*
* @param pPoint => fixed point for tests
* @param pArray => array of test points
* @param nPoint => number of points
* @see
* @return index of closest point
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiGeom_closestXYDPoint4d


(
DPoint3dCP pPoint,
DPoint4dCP pArray,
int             nPoint
)

    {
    int     iCurr, iMin;
    double  dCurr, dMin;

    iMin = -1;
    dMin = DBL_MAX;
    for (iCurr = 0; iCurr < nPoint; iCurr++)
        {
        if (bsiDPoint4d_realDistanceSquaredXY (&pArray[iCurr], &dCurr, pPoint)
            && (iMin == -1 || dCurr < dMin))
            {
            dMin = dCurr;
            iMin = iCurr;
            }
        }

    return iMin;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
