/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/dpoint3darray.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <stdlib.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
#define FIX_MINMAX(value, min, max)  FIX_MIN(value, min); else FIX_MAX(value, max)


/*-----------------------------------------------------------------*//**
* @description Copy the given number of DPoint3d structures from the pSource array to the pDest array.
*
* @param pDest <= destination array
* @param pSource => source array
* @param n => number of points
* @group "DPoint3d Copy"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_copyArray

(
DPoint3dP pDest,
DPoint3dCP pSource,
int          n
)
    {
#if defined (__jmdl)
    int i;
    for (i = 0; i < n ;i++)
        {
        pDest[i]= pSource[i];
        }
#else
    memcpy (pDest, pSource, n*sizeof(DPoint3d) );
#endif
    }


/*-----------------------------------------------------------------*//**
* @description Reverse the order of points in the array.
* @param pXYZ => source array
* @param n => number of points
* @group "DPoint3d Copy"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_reverseArrayInPlace
(
DPoint3dP pXYZ,
int          n
)
    {
    int i, j;
    for (i= 0, j = n - 1; i < j; i++, j--)
        {
        DPoint3d xyz = pXYZ[i];
        pXYZ[i] = pXYZ[j];
        pXYZ[j] = xyz;
        }
    }



/*---------------------------------------------------------------------------------**//**
* @description Copy the given number of DPoint2d structures, setting all z-coordinates to zero.
* @param pDest <= destination array
* @param pSource => source array
* @param n => number of points
* @group "DPoint3d Copy"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_copyDPoint2dArray

(
DPoint3dP pDest,
DPoint2dCP pSource,
int          n
)
    {
    int i;
    for (i = 0; i < n ;i++)
        {
        pDest[i].x = pSource[i].x;
        pDest[i].y = pSource[i].y;
        pDest[i].z = 0.0;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Copy DPoint3d structures from the pSource array to pDest, using an index array to rearrange the copy order.
* @remarks The indexing assigns pDest[i] = pSource[indexP[i]], and is not necessarily 1 to 1.
* @remarks This function does not perform in-place rearrangement.
*
* @param pDest <= destination array (must be different from pSource)
* @param pSource => source array
* @param pIndex => array of indices into source array
* @param nIndex => number of points
* @group "DPoint3d Copy"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_copyIndexedArray

(
DPoint3dP pDest,
DPoint3dCP pSource,
int         *pIndex,
int          nIndex
)
    {
    int     i;
    int    *indP;

    for (i = 0, indP = pIndex; i < nIndex; i++, indP++)
        {
        pDest[i] = pSource[*indP];
        }
    }

/*-----------------------------------------------------------------*//**
* @description Find a disconnect point in the array after a given start index.
* @param pBuffer    IN      array of points
* @param i0         IN      start index
* @param n          IN      number of points
* @param value      IN      unused
* @return Index of disconnect point, or n if no disconnects.
* @see bsiDPoint3d_isDisconnect
* @group "DPoint3d Queries"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int  bsiDPoint3d_findDisconnectIndex

(
DPoint3dCP pBuffer,
int         i0,
int          n,
double      value
)
    {
    int i = i0;
    for (i = i0; i < n && !bsiDPoint3d_isDisconnect (&pBuffer[i]); i++)
        {
        }
    return i;
    }


/*-----------------------------------------------------------------*//**
* @description Add a given point to each of the points of an array.
*
* @param pArray <=> array whose points are to be incremented
* @param pDelta => point to add to each point of the array
* @param numPoints => number of points
* @group "DPoint3d Addition"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_addDPoint3dArray

(
DPoint3dP pArray,
DPoint3dCP pDelta,
int              numPoints
)
    {
    int         i;
    DPoint3d   *pPoint = pArray;
    double      x = pDelta->x;
    double      y = pDelta->y;
    double      z = pDelta->z;

    for (i=0; i < numPoints; i++)
        {
        pPoint->x += x;
        pPoint->y += y;
        pPoint->z += z;
        pPoint++;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Sets pOutVec[i] to pOrigin + scale*pInVec[i], for 0 &le; i &lt; numPoint.
* @param pOutVec <= output array
* @param pOrigin => origin for points
* @param pInVec => input array
* @param numPoint => number of points in arrays
* @param scale => scale
* @group "DPoint3d Addition"
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_addScaledDPoint3dArray

(
DPoint3dP pOutVec,
DPoint3dCP pOrigin,
DPoint3dCP pInVec,
int          numPoint,
double       scale
)
    {
        int         i;
        DPoint3d   *pDest;
const   DPoint3d   *pSource;
        double      x0, y0, z0;

    x0 = pOrigin->x;
    y0 = pOrigin->y;
    z0 = pOrigin->z;

    for (i = 0, pDest = pOutVec, pSource = pInVec ;
         i < numPoint; i++, pSource++, pDest++)
        {
        pDest->x = x0 + pSource->x * scale;
        pDest->y = y0 + pSource->y * scale;
        pDest->z = z0 + pSource->z * scale;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Subtract a given point from each of the points of an array.
*
* @param pArray <=> Array whose points are to be decremented
* @param pDelta => point to subtract from each point of the array
* @param numVerts => number of points
* @group "DPoint3d Addition"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_subtractDPoint3dArray

(
DPoint3dP pArray,
DPoint3dCP pDelta,
int              numVerts
)
    {
    int         i;
    DPoint3d   *pPoint = pArray;
    double      x = pDelta->x;
    double      y = pDelta->y;
    double      z = pDelta->z;

    for (i=0; i<numVerts; i++)
        {
        pPoint->x -= x;
        pPoint->y -= y;
        pPoint->z -= z;
        pPoint++;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Normalize an array of vectors in place.
* @param pArray <=> array of vectors to be normalized
* @param numVector => number of vectors
* @return number of zero length vectors encountered
* @group "DPoint3d Normalize"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDPoint3d_normalizeArray

(
DPoint3dP pArray,
int         numVector
)
    {
    double  magnitude;
    int numZero = 0;
    int i;
    DPoint3d *pVec;

    for (pVec = pArray, i = 0; i < numVector; i++, pVec++)
        {
        magnitude = sqrt(pVec->x*pVec->x + pVec->y*pVec->y + pVec->z*pVec->z);

        if (magnitude > 0.0)
            {
            double f = 1.0 / magnitude;
            pVec->x *= f;
            pVec->y *= f;
            pVec->z *= f;
            }
        else
            {
            numZero++;
            }
        }

    return  numZero;
    }


/*-----------------------------------------------------------------*//**
* @description Returns an upper bound for both the largest absolute value x, y or z
* coordinate and the greatest distance between any two x,y or z coordinates
* in an array of points.
*
* @param pPointArray => array of points to test
* @param numPoint => number of points
* @return upper bound as described above. or zero if no points
* @see bsiDPoint3d_getLargestCoordinateDifference, bsiDPoint3d_getLargestWeightedCoordinateDifference, bsiDPoint3d_getLargestXYCoordinate
* @group "DPoint3d Queries"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_getLargestCoordinate

(
DPoint3dCP pPointArray,
int         numPoint
)
    {
    if (pPointArray && numPoint > 0)
        {
        DRange3d tmpRange;
        bsiDRange3d_initFromArray(&tmpRange, pPointArray, numPoint);
        return bsiDRange3d_getLargestCoordinate(&tmpRange);
        }
    else
        {
        return 0.0;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Returns an upper bound for the greatest distance between any two x, y or z
* coordinates in an array of points.
*
* @param pPointArray => array of points to test
* @param numPoint => number of points
* @return upper bound as described above, or zero if no points
* @see bsiDPoint3d_getLargestCoordinate, bsiDPoint3d_getLargestWeightedCoordinateDifference, bsiDPoint3d_getLargestXYCoordinate
* @group "DPoint3d Queries"
* @bsihdr                                       DavidAssaf      04/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_getLargestCoordinateDifference

(
DPoint3dCP pPointArray,
int         numPoint
)
    {
    if (pPointArray && numPoint > 0)
        {
        DRange3d tmpRange;
        DPoint3d diagonal;
        bsiDRange3d_initFromArray (&tmpRange, pPointArray, numPoint);
        bsiDPoint3d_subtractDPoint3dDPoint3d (&diagonal, &tmpRange.high, &tmpRange.low);
        return bsiDPoint3d_maxAbs (&diagonal);
        }
    else
        {
        return 0.0;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Returns an upper bound for the greatest distance between any two x, y or z
* coordinates in an array of weighted points.
* @remarks Points with zero weight are ignored.
*
* @param pPointArray => array of weighted points to test
* @param pWeightArray => array of weights
* @param numPoint => number of points and weights
* @return upper bound as described above, or zero if no points
* @group "DPoint3d Queries"
* @see bsiDPoint3d_getLargestCoordinateDifference, bsiDPoint3d_getLargestCoordinate, bsiDPoint3d_getLargestXYCoordinate
* @bsihdr                                       DavidAssaf      04/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_getLargestWeightedCoordinateDifference

(
DPoint3dCP    pPointArray,
const   double*     pWeightArray,
int         numPoint
)
    {
    if (!pWeightArray)
        return bsiDPoint3d_getLargestCoordinateDifference (pPointArray, numPoint);

    if (pPointArray && numPoint > 0)
        {
        DRange3d    tmpRange;
        DPoint3d    diagonal;
        double      wRecip;
        int         i;

        bsiDRange3d_init (&tmpRange);
        for (i = 0; i < numPoint; i++)
            if (bsiTrig_safeDivide (&wRecip, 1.0, pWeightArray[i], 0.0))
                bsiDRange3d_extendByComponents (&tmpRange, pPointArray[i].x * wRecip, pPointArray[i].y * wRecip, pPointArray[i].z * wRecip);

        bsiDPoint3d_subtractDPoint3dDPoint3d (&diagonal, &tmpRange.high, &tmpRange.low);
        return bsiDPoint3d_maxAbs (&diagonal);
        }
    else
        {
        return 0.0;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Returns an upper bound for both the largest absolute value x or y coordinate
* and the greatest distance between any two x or y coordinates in an array of points.
*
* @param pPointArray => array of points to test
* @param numPoint => number of points
* @return upper bound as described above, or zero if no points
* @group "DPoint3d Queries"
* @see bsiDPoint3d_getLargestWeightedCoordinateDifference, bsiDPoint3d_getLargestCoordinateDifference, bsiDPoint3d_getLargestCoordinate
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_getLargestXYCoordinate

(
DPoint3dCP pPointArray,
int         numPoint
)
    {
    DRange3d tmpRange;
    bsiDRange3d_initFromArray(&tmpRange, pPointArray, numPoint);
    tmpRange.low.z = tmpRange.high.z = 0.0;
    return (bsiDRange3d_getLargestCoordinate(&tmpRange));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
static double fixPerspectiveFraction

(
double fraction
)
    {
    static double minFraction = 1.0e-10;
    if (fabs(fraction) <= minFraction)
        {
        if (fraction < 0.0)
            fraction = -minFraction;
        else
            fraction = minFraction;
        }
    return fraction;
    }


/*-----------------------------------------------------------------*//**
* @description Apply a perspective transformation to an array of 3D points.
* @remarks The transformation is defined by a single fraction which controls the taper rate of the view frustum.
* @remarks Some geometric effects of this perspective transformation are:
    <ul>
    <li>The eyepoint is at z = 1/(1-f)</li>
    <li>The z=0 plane is unchanged.</li>
    <li>The z=1 plane maps into itself; xy-components of such points are scaled by factor 1/f.
        That is, the unit square {(0,0,1), (1,0,1), (1,1,1), (0,1,1)} maps to {(0,0,1), (1/f,0,1), (1/f,1/f,1), (0,1/f,1)}.</li>
    <li>Original z-direction lines maintain their original z=0 points but now pass through the point (0,0,f/(f-1)).</li>
    <li>All planes map to planes.  This requires that z-coordinates vary in a not necessarily intuitive way.  The general behavior of
        mapped z for 0 < f < 1 as a point moves along a (premapped) z-direction line is:
        <ul>
        <li>Start at z=0.  Move toward z=1/(1-f).  Mapped z increases to infinity.</li>
        <li>Start at z=0.  Move in the negative z direction.  Mapped z decreases to -f/(f-1).</li>
        <li>Start at z=infinity and move back towards z=1/(f-1).  Mapped z starts at -f/(f-1) and decreases to -infinity.</li>
        <li>Start at z=-infinity and move forward towards z=1/(f-1).  Mapped z starts at -f/(f-1) and increases to infinity.</li>
        </ul></li>
    </ul>
* @param pOutArray <= points in perspective space
* @param pInArray => points in real space
* @param numPoint => number of points
* @param fraction => perspective effects parameter.
* @see bsiGeom_invertPerspective
* @group "DPoint3d Modification"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiGeom_applyPerspective


(
DPoint3dP pOutArray,
DPoint3dCP pInArray,
int      numPoint,
double   fraction
)
    {
    static double minWeight = 1.0e-6;
    double a;
    int i;

    fraction = fixPerspectiveFraction (fraction);
    a = fraction - 1.0;
    for (i = 0; i < numPoint; i++)
        {
        double w = 1.0 + pInArray[i].z * a;
        if (fabs(w) <= minWeight)
            {
            if (w >= 0.0)
                {
                w = minWeight;
                }
            else
                {
                w = -minWeight;
                }
            }
        pOutArray[i].x = pInArray[i].x / w;
        pOutArray[i].y = pInArray[i].y / w;
        pOutArray[i].z = pInArray[i].z * fraction / w;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Find two points (and their indices) in the given array of points that are relatively far from each other.
* @remarks The returned points are not guaranteed to be the points with farthest separation.
*
* @param pMinPoint  <= first of the two widely separated points (or null)
* @param pMinIndex  <= index of first point (or null)
* @param pMaxPoint  <= second of the two widely separated points (or null)
* @param pMaxIndex  <= index of second point (or null)
* @param pPoints    => array of points
* @param numPts     => number of points
* @return false if numPts < 2
* @group "DPoint3d Queries"
* @bsihdr                                       EarlinLutz      12/97
* @bsihdr                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiGeom_findWidelySeparatedPoints

(
DPoint3dP pMinPoint,
int                 *pMinIndex,
DPoint3dP pMaxPoint,
int                 *pMaxIndex,
const DPoint3d      *pPoints,
int                 numPts
)
    {
    double      *pArray;        /* TREAT DPOINT3D AS ARRAY OF 3 DOUBLES*/
    double      aMin[3];
    double      aMax[3];
    double      a;
    double      delta, deltaMax;
    int         iMin[3];
    int         iMax[3];
    int         kMax;
    int         minIndex, maxIndex;
    int         i, k;

    if (numPts < 2)
        return false;

    /* Find extrema on each axis, keeping track of their indices in the array.*/
    pArray = (double *)pPoints;
    for (k = 0; k < 3; k++)         /* init min/max vals/indices w/ 1st pt */
        {
        aMin[k] = aMax[k] = pArray[k];
        iMin[k] = iMax[k] = 0;
        }
    for (i = 1; i < numPts; i++)    /* compare min/max vals/indices w/ other pts */
        {
        pArray = (double *)(pPoints + i);
        for (k = 0; k < 3; k++)
            {
            a = pArray[k];
            if (a < aMin[k])
                {
                aMin[k] = a;
                iMin[k] = i;
                }
            else if (a > aMax[k])
                {
                aMax[k] = a;
                iMax[k] = i;
                }
            }
        }

    /* Find the axis (kMax) with largest range.*/
    kMax = 0;
    deltaMax = fabs (aMax[0] - aMin[0]);

    for (k = 1; k < 3; k++)
        {
        delta = fabs (aMax[k] - aMin[k]);
        if (delta > deltaMax)
            {
            deltaMax = delta;
            kMax = k;
            }
        }

    minIndex = iMin[kMax];
    maxIndex = iMax[kMax];

    if (pMinIndex)
        *pMinIndex = minIndex;
    if (pMaxIndex)
        *pMaxIndex = maxIndex;
    if (pMinPoint)
        *pMinPoint = pPoints[minIndex];
    if (pMaxPoint)
        *pMaxPoint = pPoints[maxIndex];

    return true;
    }


/*-----------------------------------------------------------------*//**
* @description Compute a line segment approximating an array of points, with endpoints selected from the array.
* @remarks The returned distances are useful for testing if the points are colinear.  A typical followup test
*       would be to test if maxDist is less than a tolerance and dist01 is larger than (say) 1000 times the tolerance.
*       This test is implemented by ~mbsiGeom_isUnorderedDPoint3dArrayColinear.
* @remarks Note that the returned start and end points are selected from the given points; they are not points on
*       the least squares approximation that might have a smaller maxDist but not pass through any points.
* @param pPoint0 <= suggested starting point of the line segment.
* @param pPoint1 <= suggested end point.
* @param pDist01 <= distance from pPoint0 to pPoint1
* @param pMaxDist <= largest distance of any point to the (infinite) line
* @param pPointArray => array of points
* @param numPoint => number of points
* @group "DPoint3d Queries"
* @bsihdr                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void        bsiGeom_approximateLineThroughPoints

(
DPoint3dP pPoint0,
DPoint3dP pPoint1,
double      *pDist01,
double      *pMaxDist,
const   DPoint3d    *pPointArray,
int         numPoint
)
    {
    int             i;
    double          dist, maxDist = 0.0;
    DPoint3d        proj, p0, p1, direction;
    double          dist01;

    /* no pts or 1 pt: default to colinear */
    if (!bsiGeom_findWidelySeparatedPoints
            (&p0, NULL, &p1, NULL, pPointArray, numPoint))
        {
        dist01 = maxDist = 0.0;
        if (numPoint > 0)
            p0 = p1 = pPointArray[0];
        else
            {
            bsiDPoint3d_zero (&p0);
            p1 = p0;
            }
        }
    else
        {
        double s;
        bsiDPoint3d_subtractDPoint3dDPoint3d (&direction, &p1, &p0);
        dist01 = bsiDPoint3d_normalizeInPlace (&direction);

        for (i = 0; i < numPoint; i++)
            {
            s = bsiDPoint3d_dotDifference (&pPointArray[i], &p0, (DVec3d*) &direction);
            bsiDPoint3d_addScaledDPoint3d (&proj, &p0, &direction, s);
            if ((dist = bsiDPoint3d_distanceSquared (&proj, &pPointArray[i])) > maxDist)
                maxDist = dist;
            }
        }

    if (pMaxDist)
        *pMaxDist = sqrt (maxDist);
    if (pDist01)
        *pDist01 = dist01;
    if (pPoint0)
        *pPoint0 = p0;
    if (pPoint1)
        *pPoint1 = p1;
    }

/*-----------------------------------------------------------------*//**
* @description Test if an array of points is effectively a straight line from the first to the last.
* @param pOnLine <= true if all points are all within tolerance of the (bounded) line segment from the first point to the last point.
* @param pPointArray => array of points
* @param numPoint => number of points
* @param tolerance => absolute tolerance for distance test (or nonpositive to compute minimal tolerance)
* @return same as pOnLine
* @group "DPoint3d Queries"
* @bsihdr                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiGeom_pointArrayColinearTest

(
bool        *pOnLine,
const   DPoint3d    *pPointArray,
int         numPoint,
double      tolerance
)
    {
    DPoint3d    vector;
    double d01, s;
    int i;
    DPoint3d p0, p1, proj;
    static double relTol = 1.0e-12;
    double myTol = bsiDPoint3d_getLargestCoordinate (pPointArray, numPoint) * relTol;
    double myTol2;

    if (tolerance > myTol)
        myTol = tolerance;

    myTol2 = myTol * myTol;

    *pOnLine = false;

    if (numPoint < 2)
        return *pOnLine;

    p0 = pPointArray[0];
    p1 = pPointArray[numPoint - 1];
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector, &p1, &p0);
    d01 = bsiDPoint3d_normalizeInPlace (&vector);

    if (d01 <= myTol)
        return *pOnLine;

    for (i = 1; i < numPoint - 1; i++)
        {
        s = bsiDPoint3d_dotDifference (&pPointArray[i], &p0, (DVec3d*) &vector);
        bsiDPoint3d_addScaledDPoint3d (&proj, &p0, &vector, s);
        if (bsiDPoint3d_distanceSquared (&proj, &pPointArray[i]) > myTol2)
            return *pOnLine;
        if (s < 0.0)
            {
            /* projected before line segment's start */
            if (-s * d01 > myTol)
                return *pOnLine;
            }
        else if (s > d01)
            {
            /* projected after line segment's end */
            if ((s - d01) * d01 > myTol)
                return *pOnLine;
            }
        }
    *pOnLine = true;
    return *pOnLine;
    }


/*-----------------------------------------------------------------*//**
* @description Test if an array of points is effectively a straight line from the first to the last.
* @param pPointArray => array of points
* @param numPoint => number of points
* @param tolerance => absolute tolerance for distance test (or nonpositive to compute minimal tolerance)
* @return true if all points are within tolerance of the (bounded) line segment from the first point to the last point.
* @group "DPoint3d Queries"
* @bsihdr                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiGeom_isDPoint3dArrayColinear

(
const   DPoint3d    *pPointArray,
int         numPoint,
double      tolerance
)
    {
    bool    onLine;
    return bsiGeom_pointArrayColinearTest (&onLine, pPointArray, numPoint, tolerance);
    }


/*-----------------------------------------------------------------*//**
* @description Test if the points are colinear.
* @param pPointArray    => array of points
* @param numPoint       => number of points
* @param relativeTol    => fraction of the distance <i>d</i> between extremal points to use as colinearity threshhold
* @return true if all points are within the distance relativeTol * <i>d</i> of the line through the extremal points.
* @group "DPoint3d Queries"
* @bsihdr                                       DavidAssaf      01/2004
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiGeom_isUnorderedDPoint3dArrayColinear

(
DPoint3dCP    pPointArray,
int         numPoint,
double      relativeTol
)
    {
    double chordalDistance, maxDeviation;

    if (!pPointArray)
        return false;

    bsiGeom_approximateLineThroughPoints (NULL, NULL, &chordalDistance, &maxDeviation, pPointArray, numPoint);

    return maxDeviation <= chordalDistance * relativeTol;
    }


/*-----------------------------------------------------------------*//**
* @description Approximate a plane through a set of points.
* @remarks The method used is:
    <ul>
    <li>Find the bounding box.</li>
    <li>Choose the axis with greatest range.</li>
    <li>Take two points that are on the min and max of this axis.</li>
    <li>Also take as a third point the point that is most distant from the line connecting the two extremal points.</li>
    <li>Form plane through these 3 points.</li>
    </ul>
* @param pNormal <= plane normal
* @param pOrigin <= origin for plane
* @param pPoint => point array
* @param numPoint => number of points
* @param tolerance => max allowable deviation from colinearity (or nonpositive to compute minimal tolerance)
* @return true if the points define a clear plane; false if every point lies on the line (within tolerance) joining the two extremal points.
* @group "DPoint3d Queries"
* @bsihdr                                       DavidAssaf      06/01
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiGeom_planeThroughPointsTol

(
DPoint3dP pNormal,
DPoint3dP pOrigin,
DPoint3dCP pPoint,
int             numPoint,
double          tolerance
)
    {
    int i;
    int i0, i1;
    bool    result = false;

    DPoint3d point0;
    DPoint3d point1;
    DPoint3d vector;

    DPoint3d currVector;
    DPoint3d currNormal;
    DPoint3d maxNormal;

    double delta, deltaMax;
    static double relTol = 1.0e-12;
    double myTol = bsiDPoint3d_getLargestCoordinateDifference (pPoint, numPoint) * relTol;
    double myTol2;

    if (tolerance > myTol)
        myTol = tolerance;

    myTol2 = myTol * myTol;

    if (numPoint > 2)
        {
        bsiGeom_findWidelySeparatedPoints
            (&point0, &i0, &point1, &i1, pPoint, numPoint);

        bsiDPoint3d_computeNormal (&vector, &point1, &point0);

        deltaMax = 0.0;
        for (i = 0; i < numPoint; i++)
            {
            if ( i != i0 && i != i1)
                {
                bsiDPoint3d_subtractDPoint3dDPoint3d (&currVector, pPoint + i, &point0);
                bsiDPoint3d_addScaledDPoint3d (
                                            &currNormal,
                                            &currVector,
                                            &vector,
                                            -bsiDPoint3d_dotProduct (&currVector, &vector));
                delta = bsiDPoint3d_dotProduct (&currNormal, &currNormal);
                if (delta > deltaMax)
                    {
                    maxNormal = currNormal;
                    deltaMax = delta;
                    }
                }
            }


        if (deltaMax > myTol2)
            {
            *pOrigin = point1;
            bsiDPoint3d_crossProduct (pNormal, &vector, &maxNormal);
            result = true;
            }
        }
    return result;
    }


/*-----------------------------------------------------------------*//**
* @description Approximate a plane through a set of points.
* @remarks This function calls ~mbsiGeom_planeThroughPointsTol with tolerance = 0.0 to force usage of smallest colinearity tolerance.
* @param pNormal <= plane normal
* @param pOrigin <= origin for plane
* @param pPoint => point array
* @param numPoint => number of points
* @return true if the points define a clear plane; false if every point lies on the line joining the two extremal points.
* @group "DPoint3d Queries"
* @bsihdr                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiGeom_planeThroughPoints

(
DPoint3dP pNormal,
DPoint3dP pOrigin,
DPoint3dCP pPoint,
int             numPoint
)
    {
    return bsiGeom_planeThroughPointsTol (pNormal, pOrigin, pPoint, numPoint, 0.0);
    }


/*-----------------------------------------------------------------*//**
* @description Remove the effects of a perspective transformation of 3D points.
* @remarks The transformation is as described in ~mbsiGeom_applyPerspective.
*
* @param pOutArray <= points in real space
* @param pInArray => points in perspective space
* @param numPoint => number of points
* @param fraction => perspective effects parameter
* @see bsiGeom_applyPerspective
* @group "DPoint3d Modification"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiGeom_invertPerspective

(
DPoint3dP pOutArray,
DPoint3dCP pInArray,
int      numPoint,
double   fraction
)
    {
    double a;
    static double minWeight = 1.0e-6;
    int i;

    fraction = fixPerspectiveFraction (fraction);
    a = (1.0 - fraction) / fraction;
    for (i = 0; i < numPoint; i++)
            {
            double w = 1.0 + pInArray[i].z * a;
            if (fabs(w) <= minWeight)
                {
                if (w >= 0.0)
                    {
                    w = minWeight;
                    }
                else
                    {
                    w = -minWeight;
                    }
                }
            pOutArray[i].x = pInArray[i].x / w;
            pOutArray[i].y = pInArray[i].y / w;
            pOutArray[i].z = pInArray[i].z / (fraction * w);
            }
    }


/*-----------------------------------------------------------------*//**
* @description Find the closest point in an array to the given point.
* @param pDist2 <= squared distance of closest point to test point (or NULL)
* @param pPointArray => point array
* @param n => number of points
* @param pTestPoint => point to test
* @return index of nearest point, or negative if n is nonpositive
* @group "DPoint3d Queries"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiGeom_nearestPointinDPoint3dArray

(
double          *pDist2,
DPoint3dCP pPointArray,
int              n,
DPoint3dCP pTestPoint
)
    {
    double minDist2, dist2;
    int    minIndex, i;
    if (n <= 0)
        {
        minDist2= 0.0;
        minIndex = -1;
        }
    else

        {
        minDist2 = bsiDPoint3d_distanceSquared (pTestPoint, pPointArray);
        minIndex = 0;
        for (i = 1; i < n; i++)
            {
            dist2 = bsiDPoint3d_distanceSquared (pTestPoint, pPointArray + i);
            if (dist2 < minDist2)
                {
                minDist2 = dist2;
                minIndex = i;
                }
            }
        }
    if (pDist2)
        *pDist2 = minDist2;
    return minIndex;
    }


/*-----------------------------------------------------------------*//**
* @description Find the closest point in an array to the given point, using only xy-coordinates for distance calculations.
* @param pPoint => point to test
* @param pArray => point array
* @param nPoint => number of points
* @return index of nearest point, or negative if nPoint is nonpositive
* @group "DPoint3d Queries"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiGeom_closestXYPoint

(
DPoint3dCP pPoint,
DPoint3dCP pArray,
int             nPoint
)
    {
    double      xdist, ydist;
    int iCurr, iMin;
    double dMin, dCurr;
    const DPoint3d *pCurr = pArray;
    if ( nPoint < 0)
        return -1;

    iMin = iCurr = 0;
    dMin = bsiDPoint3d_distanceSquaredXY (pPoint, pCurr);

    while (pCurr++, iCurr++, iCurr < nPoint)
        {
        xdist = (pCurr->x - pPoint->x);
        ydist = (pCurr->y - pPoint->y);
        dCurr = xdist * xdist + ydist * ydist;
        if (dCurr < dMin)
            {
            dMin = dCurr;
            iMin = iCurr;
            }
        }

    return iMin;
    }


/*-----------------------------------------------------------------*//**
* @description Evaluate trig functions at regularly spaced angles and store the results in the xy-coordinates of a point array.
* @param pPointArray <= filled array, z-coordinates untouched
* @param numPoint => number of points to evaluate
* @param theta0 => initial radian angle
* @param theta1 => end radian angle
* @group Trigonometry
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiEllipse_evaluateCosSin

(
DPoint3dP pPointArray,
int             numPoint,
double          theta0,
double          theta1
)
    {
    int i;
    double dtheta = (theta1 - theta0) / ( numPoint > 1 ? numPoint - 1 : 1 );
    double theta;
    double c0 = cos(theta0);
    double s0 = sin(theta0);
    double c1 = cos(theta0 + dtheta);
    double s1 = sin(theta0 + dtheta);
    double c2,s2;
    double twok = 2.0*cos(dtheta);
    DPoint3d *pPoint = pPointArray;

    for ( i = 0, pPoint = pPointArray;
          i < numPoint;
          i++, pPoint++ )
        {
        theta = theta0 + (double)i * dtheta;
        pPoint->x = c0;
        pPoint->y = s0;

        /* Recurrence relations for advancing sine and cosine */
        /* These maintain the unit-vector property to 14 digits through 4000
           steps.  Angles may be a bit off.
        */
        c2 = twok*c1 - c0;
        s2 = twok*s1 - s0;
        c0 = c1;
        s0 = s1;
        c1 = c2;
        s1 = s2;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Compute the simple average of points in the array.
* @param pAverage <= average of points
* @param pPoint => point array
* @param numPoint => number of points
* @return true if 1 or more points in the array
* @group "DPoint3d Queries"
* @bsihdr                                       DavidAssaf      06/01
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDPoint3d_averageDPoint3dArray

(
DPoint3dP pAverage,
DPoint3dCP pPoint,
int             numPoint
)
    {
    int i;
    double a;
    bsiDPoint3d_zero (pAverage);
    if (numPoint <= 0)
        return false;
    for (i = 0; i < numPoint; i++)
        {
        bsiDPoint3d_addDPoint3dInPlace (pAverage, pPoint + i);
        }
    a = 1.0 / numPoint;
    bsiDPoint3d_scaleInPlace (pAverage, a);
    return true;
    }

typedef struct
    {
        DPoint3d *pOut;
        int numOut;
        double *pError;
        const DPoint3d *pIn;
        int numIn;
        double tol2;
        } CLContext;

/*-----------------------------------------------------------------*//**
* Copy indexed input point to output.
* @bsihdr                                       EarlinLutz 08/02
+---------------+---------------+---------------+---------------+------*/
static void cl_output

(
CLContext *pContext,
int index
)
    {
        if (0 <= index && index < pContext->numIn
        && pContext->numOut < pContext->numIn)
        {
                pContext->pOut[pContext->numOut++] = pContext->pIn[index];
                }
        }

/*-----------------------------------------------------------------*//**
* Distance from pointpXYZ to nearest point on a bounded line segment,
* using only xy components.
* @bsihdr                                       EarlinLutz 08/02
+---------------+---------------+---------------+---------------+------*/
static double squaredDistanceToBoundedXYSegment

(
DPoint3dCP pXYZ,
DPoint3dCP pXYZStart,
DPoint3dCP pXYZEnd
)
    {
    DPoint3d vectorU, vectorV, vectorW;
    double s;
    double dotUU, dotUV;
    double dd;
    vectorV.x = pXYZ->x - pXYZStart->x;
    vectorV.y = pXYZ->y - pXYZStart->y;
    vectorU.x = pXYZEnd->x - pXYZStart->x;
    vectorU.y = pXYZEnd->y - pXYZStart->y;
    dotUU = vectorU.x * vectorU.x + vectorU.y * vectorU.y;
    dotUV = vectorU.x * vectorV.x + vectorU.y * vectorV.y;
    if (dotUV <= 0.0)
        {
        vectorW.x = vectorV.x;
        vectorW.y = vectorV.y;
        }
    else if (dotUV >= dotUU)
        {
        vectorW.x = pXYZ->x - pXYZEnd->x;
        vectorW.y = pXYZ->x - pXYZEnd->x;
        }
    else
        {
        s = dotUV / dotUU;  /* Safe divide, we know result is between 0 and 1 */
        vectorW.x = vectorV.x - s * vectorU.x;
        vectorW.y = vectorV.y - s * vectorU.y;
        }

    dd = vectorW.x * vectorW.x + vectorW.y * vectorW.y;
    return dd;
    }

/*-----------------------------------------------------------------*//**
* Consider line segment from i0 to i1.  Return true if any
*   point i0+1...i1-1 is more than tolerance away from the (bounded) segment.
* @bsihdr                                       EarlinLutz 08/02
+---------------+---------------+---------------+---------------+------*/
static bool    cl_errorWithinSequenceExceedsTol

(
CLContext *pContext,
int i0,
int i1
)
    {
    int i;
    DPoint3d xyz0 = pContext->pIn[i0];
    DPoint3d xyz1 = pContext->pIn[i1];
    double e;
    /* Linear search.  Bah humbug.  Some sort of binary test would "probably" hit big errors
        sooner, but it's probably not worth the effort for first try.
    */
    for (i = i0 + 1; i < i1; i++)
        {
        e = squaredDistanceToBoundedXYSegment (&pContext->pIn[i], &xyz0, &xyz1);
        if (e > pContext->tol2)
            return true;
        }
    return false;
    }

/*-----------------------------------------------------------------*//**
* Output segments within a sequence of points, such that all points skipped
*   are within tolerance of their segment.
* It is assumed that each point is within tolerance of the segment between
*   immediate predecessor and successor.
* If the complete segment from i0 to i1 cannot be used, binary subdivision is used
*   within the interval.
* @bsihdr                                       EarlinLutz 08/02
+---------------+---------------+---------------+---------------+------*/
static void cl_subdivideAndOutput

(
CLContext *pContext,
int i0,
int i1
)
    {
    int iMid;
    /* ASSUME any interval with only 3 points i0, i0+1, i0+2 is ok from higher level test.
    Hence only consider subdivision of longer sequences. */
    if (i1 > i0 + 2 && cl_errorWithinSequenceExceedsTol (pContext, i0, i1))
        {
        iMid = (i0 + i1) / 2;
        cl_subdivideAndOutput (pContext, i0, iMid);
        cl_subdivideAndOutput (pContext, iMid, i1);
        }
    else
        {
        cl_output (pContext, i1);
        }
    }

/*-----------------------------------------------------------------*//**
* @description Generate a compressed linestring, with points removed if they are within tolerance of the output linestring.
* @remarks For a closed polygon, the caller must ensure the first/last points are equal; this
*       point will also be the first/last of the output.
* @param pOut OUT output point buffer (must be allocated by caller)
* @param pNumOut OUT number of output points (at most numIn)
* @param pIn IN input point buffer
* @param numIn IN number of input points
* @param tol IN absolute distance tolerance from input point to output segment
* @group Polylines
* @bsihdr                                       EarlinLutz      08/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3dArray_compressXYLinestring

(
DPoint3dP pOut,
int     *pNumOut,
DPoint3dCP pIn,
int     numIn,
double tol
)
    {
    int i, i0;
    int iLast = numIn - 1;
    CLContext context;
    context.pOut = pOut;
    context.pIn  = pIn;
    context.numOut = 0;
    context.numIn  = numIn;
    context.tol2    = tol * tol;
    *pNumOut = 0;
    i0 = 0;
    cl_output (&context, 0);

    /* Scan for contiguous blocks in which point 1 of each
    3 consecutive point 0,1,2 is within epsilon of the segment
    from 0 to 2.
    Try to eliminate points within each such block.
    */
    for (i = 1; i < numIn; i++)
        {
        if (   i == iLast
            || squaredDistanceToBoundedXYSegment (&pIn[i], &pIn[i-1], &pIn[i+1]) > context.tol2)
            {
            cl_subdivideAndOutput (&context, i0, i);
            i0 = i;
            }
        }
    *pNumOut = context.numOut;
    }

/*-----------------------------------------------------------------*//**
* qsort comparator.  Compare w parts of pA, pB; if equal comapre (squared)
*       distance from origin.
* @bsihdr                                       EarlinLutz 08/02
+---------------+---------------+---------------+---------------+------*/
static int compareW

(
DPoint4dCP pA,
DPoint4dCP pB
)
    {
    double rrA, rrB;
    if (pA->w < pB->w)
        return -1;
    if (pA->w > pB->w)
        return 1;
    rrA = pA->x * pA->x + pA->y * pA->y;
    rrB = pB->x * pB->x + pB->y * pB->y;

    if (rrA < rrB)
        return -1;
    if (rrA > rrB)
        return 1;
    return 0;
    }

/*-----------------------------------------------------------------*//**
* @description Trim points from the tail of an evolving hull if they
*   are covered by segment from new point back to earlier point.
* @param pXYZA IN evolving hull array.
* @param num IN number of points in initial hull.
* @param pXYZNew = new point being added to hull.
* @return number of points in hull array after removals.
* @bsihdr                                       EarlinLutz 08/02
+---------------+---------------+---------------+---------------+------*/
static int trimHull

(
DPoint4dP pXYZA,
int     num,
DPoint4dCP pXYZNew
)
    {
    int i0, i1;
    double dx0, dy0, dx1, dy1;
    double cross;
    while (num >= 2)
        {
        i0 = num - 1;
        i1 = i0 - 1;
        dx0 = pXYZA[i0].x - pXYZNew->x;
        dy0 = pXYZA[i0].y - pXYZNew->y;
        dx1 = pXYZA[i1].x - pXYZNew->x;
        dy1 = pXYZA[i1].y - pXYZNew->y;
        cross = dx0 * dy1 - dy0 * dx1;
        if (cross < 0.0)
            break;
        num--;
        }
    return num;
    }

/*-----------------------------------------------------------------*//**
* @description Compute a convex hull of given points.  Each output point
*       is one of the inputs, including its z part.
* @param pOutBuffer OUT Convex hull points.  First/last point NOT duplicated.
*       This must be allocated to the caller, large enough to contain numIn points.
* @param pNumOut OUT number of points on hull
* @param pInBuffer IN input points.
* @param numIn IN number of input points.
* @param iMax IN index of point at maximal radius, i.e. guaranteed to be on hull.
* @bsihdr                                       EarlinLutz 08/02
+---------------+---------------+---------------+---------------+------*/
static bool    bsiDPoint3dArray_convexHullXY_go

(
DPoint3dP pOutBuffer,
int         *pNumOut,
DPoint4dP pXYZA,
int         numIn,
int         iMax
)
    {
    int i;
    int numOut;
    double shift = 8.0 * atan (1.0); /* 2pi */
    double theta0 = pXYZA[iMax].w;
    for (i = 0; i < numIn; i++)
        {
        if (pXYZA[i].w < theta0)
            pXYZA[i].w += shift;
        }
    qsort (pXYZA, numIn, sizeof (DPoint4d),
            (int (*)(const void *, const void *))compareW);
    numOut = 1;
    for (i = 1; i < numIn; i++)
        {
        numOut = trimHull (pXYZA, numOut, &pXYZA[i]);
        pXYZA[numOut++] = pXYZA[i];
        }
    if (numOut > 2)
        numOut = trimHull (pXYZA, numOut, &pXYZA[0]);
    *pNumOut = numOut;
    return true;
    }

/*-----------------------------------------------------------------*//**
* @description Compute a convex hull of a point array, ignoring z-coordinates.
* @remarks Each output point is one of the inputs, including its z-coordinate.
* @param pOutBuffer OUT convex hull points, first/last point <em>not</em> duplicated.
*                       This must be allocated by the caller, large enough to contain numIn points.
* @param pNumOut    OUT number of points on hull
* @param pInBuffer  IN  input points
* @param numIn      IN  number of input points
* @return false if numin is nonpositive or memory allocation failure
* @group "DPoint3d Queries"
* @bsihdr                                       EarlinLutz      08/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3dArray_convexHullXY

(
DPoint3dP pOutBuffer,
int         *pNumOut,
DPoint3dP pInBuffer,
int         numIn
)
    {
    int i;
    double xsum, ysum;
    double dx, dy, rr, rrMax;
    int iMax, k;
    double x0, y0;
    int numOut = 0;
    bool    boolstat = false;
    int numSort;

    numOut = 0;
    if (pNumOut)
        *pNumOut = numOut;

    if (numIn <= 0)
        return false;

    bvector<DPoint4d>sortXYZA ((size_t)(numIn + 1));

    if (numIn == 1)
        {
        numOut = 1;
        pOutBuffer[0] = pInBuffer[0];
        boolstat = true;
        }
    else
        {
        /* Compute centroid of all points, relative to first point. */
        xsum = ysum = 0.0;
        x0 = pInBuffer[0].x;
        y0 = pInBuffer[0].y;
        for (i = 1; i < numIn; i++)
            {
            xsum += pInBuffer[i].x - x0;
            ysum += pInBuffer[i].y - y0;
            }
        x0 += xsum / numIn;
        y0 += ysum / numIn;
        /* Set up work array with x,y,i,angle in local coordinates around centroid. */
        iMax = -1;
        rrMax = 0.0;
        numSort = 0;
        for (i = 0; i < numIn; i++)
            {
            dx = sortXYZA[numSort].x = pInBuffer[i].x - x0;
            dy = sortXYZA[numSort].y = pInBuffer[i].y - y0;
            sortXYZA[numSort].z = (double)i;
            sortXYZA[numSort].w = bsiTrig_atan2 (dy, dx);
            rr = dx * dx + dy * dy;
            if (rr > 0.0)
                {
                if (rr > rrMax)
                    {
                    iMax = numSort;
                    rrMax = rr;
                    }
                numSort++;
                }
            }

        if (numSort == 0)
            {
            /* All points are at the centroid. Copy the first one out. */
            numOut = 1;
            if (pOutBuffer)
                pOutBuffer[0] = pInBuffer[0];
            boolstat = true;
            }
        else
            {
            boolstat = bsiDPoint3dArray_convexHullXY_go (pOutBuffer, &numOut, &sortXYZA[0], numSort, iMax);
            if (boolstat)
                {
                for (i = 0; i < numOut; i++)
                    {
                    k = (int)sortXYZA[i].z;
                    pOutBuffer[i] = pInBuffer[k];
                    }
                }
            else
                numOut = 0;
            }
        }
    if (pNumOut)
        *pNumOut = numOut;
    return boolstat;
    }

/*-----------------------------------------------------------------*//**
* @description Compute a transformation which, if the points are coplanar in 3D, transforms all to the z=0 plane.
* @remarks Optionally returns post-transform range data so the caller can assess planarity.   If non-coplanar points are given,
    the plane will be chosen to pass through 3 widely separated points.   If the points are "close" to coplanar, the choice of
    "widely separated" will give an intuitively reasonable plane, but is not a formal "best" plane by any particular condition.
* @param pTransformedPoints OUT the points after transformation.  May be NULL.
* @param pWorldToPlane OUT transformation from world to plane.  May be NULL.
* @param pPlaneToWorld OUT transformation from plane to world.  May be NULL.
* @param pRange OUT range of the points in the transformed system.  May be NULL.
* @param pPoints IN pretransformed points
* @param numPoint IN number of points
* @return true if a plane was computed.  This does not guarantee that the points are coplanar.
    The false condition is for highly degenerate (colinear or single point) data, not
    an assessment of deviation from the proposed plane.
* @group "DPoint3d Modification"
* @bsihdr                                       EarlinLutz      08/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3dArray_transformToPlane

(
DPoint3dP pTransformedPoints,
TransformP pWorldToPlane,
TransformP pPlaneToWorld,
DRange3dP pRange,
DPoint3dCP pPoints,
int numPoint
)
    {
    Transform worldToPlane, planeToWorld;
    DVec3d normal;
    DPoint3d origin;
    DVec3d xVec, yVec, zVec;
    bool    boolstat = false;
    if (   !bsiGeom_planeThroughPoints (&normal, &origin, pPoints, numPoint)
        || !bsiDVec3d_getNormalizedTriad (&normal, &xVec, &yVec, &zVec)
       )
        {
        if (pWorldToPlane)
            bsiTransform_initIdentity (pWorldToPlane);
        if (pPlaneToWorld)
            bsiTransform_initIdentity (pPlaneToWorld);
        if (pRange)
            bsiDRange3d_init (pRange);
        if (pTransformedPoints)
            bsiDPoint3d_copyArray (pTransformedPoints, pPoints, numPoint);
        }
    else
        {
        bsiTransform_initFromOriginAndVectors (&planeToWorld, &origin, &xVec, &yVec, &zVec);
        bsiTransform_invertAsRotation (&worldToPlane, &planeToWorld);

        if (pWorldToPlane)
            *pWorldToPlane = worldToPlane;
        if (pPlaneToWorld)
            *pPlaneToWorld = planeToWorld;

        if (pTransformedPoints)
            {
            bsiTransform_multiplyDPoint3dArray (&worldToPlane, pTransformedPoints, pPoints, numPoint);
            if (pRange)
                bsiDRange3d_initFromArray (pRange, pTransformedPoints, numPoint);
            }
        else
            {
            // If range requested but no buffer given, have to do it one by one ourselves.
            if (pRange)
                {
                DPoint3d xyz;
                int i;

                bsiDRange3d_init (pRange);
                for (i = 0; i < numPoint; i++)
                    {
                    bsiTransform_multiplyDPoint3d (&worldToPlane, &xyz, &pPoints[i]);
                    bsiDRange3d_extendByDPoint3d (pRange, &xyz);
                    }
                }
            }
        boolstat = true;
        }
    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
* @description Compute the linear combination of the given points and scalars.
*
* @param pPoint         <= linear combination of points
* @param pPoints        => array of points
* @param pScalars       => array of scalars to multiply each point
* @param numPoints      => number of points = number of scalars
* @group "DPoint3d Addition"
* @bsimethod                                                    DavidAssaf      04/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDPoint3dArray_linearCombination

(
DPoint3dP        pPoint,
DPoint3dCP        pPoints,
const   double*         pScalars,
int             numPoints
)
    {
    DPoint3d    sum = {0.0, 0.0, 0.0};
    int         i;

    for (i = 0; i < numPoints; i++)
        {
        sum.x += pScalars[i] * pPoints[i].x;
        sum.y += pScalars[i] * pPoints[i].y;
        sum.z += pScalars[i] * pPoints[i].z;
        }

    *pPoint = sum;
    }

/*---------------------------------------------------------------------------------**//**
* @description Compute the linear combination of the given points (XY only) and scalars.
*
* @param pPoint         <= linear combination of points
* @param pPoints        => array of points
* @param pScalars       => array of scalars to multiply each point
* @param numPoints      => number of points = number of scalars
* @group "DPoint3d Addition"
* @bsimethod                                                    DavidAssaf      04/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3dArray_linearCombinationXY

(
DPoint2dP        pPoint,
DPoint3dCP        pPoints,
const   double*         pScalars,
int             numPoints
)
    {
    DPoint2d    sum = {0.0, 0.0};
    int         i;

    for (i = 0; i < numPoints; i++)
        {
        sum.x += pScalars[i] * pPoints[i].x;
        sum.y += pScalars[i] * pPoints[i].y;
        }

    *pPoint = sum;
    }

/*---------------------------------------------------------------------------------**//**
@description Return the range of dot product values in the expression (XYZ[i] - Origin) dot Vector.
@param pOrigin IN origin for vectors
@param pVector IN fixed vector
@param pXYZ IN array of points
@param numXYZ IN number of points
@param pAMin OUT minimum dot product
@param pAMax OUT minimum dot product
@return true if numXYZ is positive
@group "DPoint3d Dot and Cross"
@bsimethod                                                      EarlinLutz      04/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3dArray_dotDifferenceRange

(
DPoint3dCP pOrigin,
DVec3dCP pVector,
DPoint3dCP pXYZ,
int         numXYZ,
double      *pAMin,
double      *pAMax
)
    {
    int j;
    if (numXYZ <= 0)
        {
        *pAMin = *pAMax = 0.0;
        return false;
        }

    *pAMin = *pAMax = bsiDPoint3d_dotDifference (pXYZ, pOrigin, pVector);
    for (j = 1; j < numXYZ; j++)
        {
        double a = bsiDPoint3d_dotDifference (pXYZ + j, pOrigin, pVector);
        if (a > *pAMax)
            *pAMax = a;
        else if (a < *pAMin)
            *pAMin = a;
        }
    return true;
    }


/*-----------------------------------------------------------------*//**
* qsort comparator.  Compare x parts of pA, pB
* @bsihdr                                       EarlinLutz 08/02
+---------------+---------------+---------------+---------------+------*/
static int compareX
(
const DPoint3d *pA,
const DPoint3d *pB
)
    {
    if (pA->x < pB->x)
        return -1;
    if (pA->x > pB->x)
        return 1;
    return 0;
    }

/*-----------------------------------------------------------------*//**
* qsort comparator.  Compare y parts of pA, pB
* @bsihdr                                       EarlinLutz 08/02
+---------------+---------------+---------------+---------------+------*/
static int compareY
(
const DPoint3d *pA,
const DPoint3d *pB
)
    {
    if (pA->y < pB->y)
        return -1;
    if (pA->y > pB->y)
        return 1;
    return 0;
    }

/*-----------------------------------------------------------------*//**
* qsort comparator.  Compare x parts of pA, pB
* @bsihdr                                       EarlinLutz 08/02
+---------------+---------------+---------------+---------------+------*/
static int compareZ
(
const DPoint3d *pA,
const DPoint3d *pB
)
    {
    if (pA->z < pB->z)
        return -1;
    if (pA->z > pB->z)
        return 1;
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
@description Sort points along any direction with clear variation.
@param pXYZ IN OUT points to sort.
@param numXYZ IN number of points.
@group "DPoint3d Sorting"
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3dArray_sortAlongAnyDirection
(
DPoint3d *pXYZ,
int      numXYZ
)
    {
    if (numXYZ < 2)
        return;
    DRange3d range;
    range.Init ();
    range.Extend (pXYZ, numXYZ);
    DVec3d diagonal;
    diagonal.DifferenceOf (range.high, range.low);

    if (diagonal.x >= diagonal.y && diagonal.x >= diagonal.z)
        qsort (pXYZ, numXYZ, sizeof (DPoint3d),
            (int (*)(const void *, const void *))compareX);
    else if (diagonal.y >= diagonal.z)
        qsort (pXYZ, numXYZ, sizeof (DPoint3d),
            (int (*)(const void *, const void *))compareY);
    else
        qsort (pXYZ, numXYZ, sizeof (DPoint3d),
            (int (*)(const void *, const void *))compareZ);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
