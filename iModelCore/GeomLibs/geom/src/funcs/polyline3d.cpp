/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
#define FIX_MINMAX(value, min, max)  FIX_MIN(value, min); else FIX_MAX(value, max)


/* MAP bsiDPoint3d_interpolatePolyline=Geom.interpolatePolyline ENDMAP */

/*-----------------------------------------------------------------*//**
* Interpolate within a polyline.  First and last points of polyline
* are at parameters 0 and 1.  Intermediate points have parameter
* spacing 1 / (n2), regardless of physical distance.
* (Note: This corresponds to a uniform knot order 2 bspline.
*
* @param pPoint <= interpolated point
* @param pTangent <= forward tangent vector
* @param pPointArray => points in polyline
* @param n => number of points in polyline
* @param param => parametric coordinate
* @see
* @return true if polyline has 1 or more points.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_interpolatePolyline

(
DPoint3dP pPoint,
DPoint3dP pTangent,
DPoint3dCP pPointArray,
int              n,
double           param
)
    {
    bool    result = false;
    if (n >= 2)
        {
        /* The usual case.*/
        double delta = 1.0 / (double) (n-1);
        double localParam;
        DPoint3d tangent;
        DPoint3d point0, point1;
        int iBase;
        /* Establish iBase = index of base point of segment where linear*/
        /* interpolation is applied.*/
        if (param >= 1.0)
            {
            // ensure exact ping of end point by basing fraction calculation there.
            localParam = (1.0 - param) / delta;     // this is 0 or negative.  point shuffle makes the interpolation accurate at 1.0
            point0 = pPointArray [n-1];
            point1 = pPointArray[n-2];
            tangent = DVec3d::FromStartEnd (point1, point0);        // this points forward

            }
        else
            {
            if (param < 0.0)
                {
                iBase = 0;
                }
            else
                {
                iBase = (int)(param / delta);
                // YES -- this can happen  !!! param = 0.9999999999999999, delta = 0.333333333333333 ==> iBase = n-1 !!!!
                if (iBase == n - 1)
                    iBase = n - 2;
                }
            
            localParam = (param - delta * iBase) / delta;
            point0 = pPointArray [iBase];
            point1 = pPointArray[iBase+1];
            tangent = DVec3d::FromStartEnd (point0, point1);
            }
        if (pPoint)
            pPoint->Interpolate (point0, localParam, point1);

        if (pTangent)
            pTangent->Scale (tangent, (double)(n-1));

        result = true;
        }
    else if (n == 1)
        {
        if (pPoint)
            *pPoint = pPointArray[0];

        if (pTangent)
            pTangent->Zero ();

        result = true;
        }
    else
        {
        if (pPoint)
            pPoint->Zero ();
        if (pTangent)
            pTangent->Zero ();
        result = true;
        }

    return result;
    }

/*-----------------------------------------------------------------*//**
* Interpolate within a polyline.  First and last points of polyline
* are at parameters 0 and 1.  Intermediate points have parameter
* spacing 1 / (n2), regardless of physical distance.
* (Note: This corresponds to a uniform knot order 2 bspline.
*
* @param pPoint <= interpolated point
* @param tangentA <= incoming tangent vector
* @param tangentB <= outgoing tangent vector
* @param pPointArray => points in polyline
* @param n => number of points in polyline
* @param param => parametric coordinate
* @see
* @return true if polyline has 1 or more points.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3d_interpolatePolyline

(
DPoint3dR point,
DVec3dR tangentA,
DVec3dR tangentB,
DPoint3dCP pPointArray,
int              n,
double           param
)
    {
    bool    result = false;
    if (n >= 2)
        {
        /* The usual case.*/
        double delta = 1.0 / (double) (n-1);
        double localParam;
        DPoint3d point0, point1;
        int iBase;
        /* Establish iBase = index of base point of segment where linear*/
        /* interpolation is applied.*/
        if (param < 0.0)
            {
            localParam = param / delta;
            iBase = 0;
            }
        else if (param >= 1.0)
            {
            // treat the endpoint as the anchor
            iBase = n - 2;
            localParam = 1.0 + (param - 1.0) / delta;   // avoid near-one fuzz
            }
        else
            {

            iBase = (int)(param / delta);
            // YES -- this can happen  !!! param = 0.9999999999999999, delta = 0.333333333333333 ==> iBase = n-1 !!!!
            if (iBase == n - 1)
                iBase = n - 2;
            localParam = (param - delta * iBase) / delta;
            tangentA = DVec3d::FromStartEnd (point0, point1);
            }
        point0 = pPointArray[iBase];
        point1 = pPointArray[iBase + 1];
        tangentA = DVec3d::FromStartEnd (point0, point1);
        point.Interpolate (point0, localParam, point1);
        tangentB = tangentA;
        // Detect actual interior breakpoints . ..
        size_t k0 = SIZE_MAX;
        if (iBase > 0 && DoubleOps::AlmostEqualFraction (localParam, 0.0))
            k0 = iBase - 1;
        else if (iBase + 2 < n && DoubleOps::AlmostEqualFraction (localParam, 1.0))
            k0 = iBase;
        if (k0 != SIZE_MAX)
            {
            tangentA = DVec3d::FromStartEnd (pPointArray[k0], pPointArray[k0+1]);
            tangentB = DVec3d::FromStartEnd (pPointArray[k0+1], pPointArray[k0+2]);
            }
        tangentA.Scale ((double)(n-1));
        tangentB.Scale ((double)(n-1));
        result = true;
        }
    else if (n == 1)
        {
        point = pPointArray[0];
        tangentA.Zero ();
        tangentB.Zero ();
        result = true;
        }
    else
        {
        point.Zero ();
        tangentA.Zero ();
        tangentB.Zero ();
        result = true;
        }

    return result;
    }


/* MAP bsiGeom_polylineLength=Geom.polylineLength ENDMAP */

/*-----------------------------------------------------------------*//**
* Return the length of the polyline.
*
* @param pPointArray => points in polyline
* @param n => number of points in polyline
* @see
* @return double
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiGeom_polylineLength

(
DPoint3dCP pPointArray,
int              n
)

    {
    double sum = 0.0;
    int i;
    int numEdge = n - 1;
    for (i = 0; i < numEdge; i++)
        {
        sum += pPointArray[ i].Distance (pPointArray[ i + 1]);
        }
    return sum;
    }

/* MAP bsiGeom_polylineLength=Geom.polylineLength ENDMAP */

/*-----------------------------------------------------------------*//**
Return the length of the polyline.
@parma pLength <= computed length.
@param pPointArray => points in polyline
@param pWeights => optional weights
@param n => number of points in polyline
@return true if all weights are positive.  If false, returned length is zero.
@bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_positiveWeightedPolylineLength
(
double      *pLength,
DPoint3dCP pPointArray,
double     *pWeights,
int              n
)

    {
    static double sAbsTol = 1.0e-10;
    if (!pWeights)
        {
        if (pLength)
            *pLength = bsiGeom_polylineLength (pPointArray, n);
        return true;
        }

    double sum = 0.0;
    int i;
    DPoint3d xyz0, xyz1;

    if (pLength)
        *pLength = 0.0;

    if (n < 1)
        return true;

    if (pWeights[0] < sAbsTol)
        return false;

    xyz0.Scale (pPointArray[0], 1.0 / pWeights[0]);
    for (i = 1; i < n; i++, xyz0 = xyz1)
        {
        if (pWeights[i] < sAbsTol)
            return false;
        xyz1.Scale (pPointArray[i], 1.0 / pWeights[i]);
        sum += xyz0.Distance (xyz1);
        }
    if (pLength)
        *pLength = sum;
    return true;
    }

/* MAP bsiGeom_pointAtDistanceOnPolyline=Geom.pointAtDistanceOnPolyline ENDMAP */

/*-----------------------------------------------------------------*//**
* Interpolate within a polyline.  First point is at arc length 0.
* Last point is at arc length as returned by bsiGeom_polylineLength.
* Out of bounds cases are extrapolated.
*
* @param pPoint <= interpolated point
* @param pTangent <= forward tangent vector unit
* @param pPointArray => points in polyline
* @param n => number of points in polyline
* @param dist => distance from start
* @see
* @return true if polyline has 1 or more points.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_pointAtDistanceOnPolyline

(
DPoint3dP pPoint,
DPoint3dP pTangent,
DPoint3dCP pPointArray,
int              n,
double           dist
)
    {
    double sum, newSum, segmentLength = 0.0;
    DPoint3d tangent;
    double fraction;
    int i;
    int numSeg = n - 1;
    if (n >= 2)
        {
        sum = 0.0;
        /* Dist < 0 will trigger backwards extrapolation from first non-null segment*/
        for (i = 0; i < numSeg; i++)
            {
            tangent.DifferenceOf (pPointArray[ i + 1], pPointArray[ i]);
            segmentLength = tangent.Magnitude ();
            if (segmentLength > 0.0)
                {
                newSum = sum + segmentLength;
                if (dist < newSum)
                    {
                    fraction = (dist - sum) / segmentLength;
                    if (pPoint)
                        pPoint->SumOf (pPointArray[ i], tangent, fraction);
                    if (pTangent)
                        pTangent->Scale (tangent, 1.0 / segmentLength);
                    return true;
                    }
                sum = newSum;
                }
            }
        /* Is there an exact hit at the last point?*/
        /* If so take the last point, and use the left-over tangent*/
        if (sum > 0.0)
            {
            fraction = (dist - sum) / segmentLength;
            if (pPoint)
                pPoint->SumOf (pPointArray[ n - 1], tangent, fraction);
            if (pTangent)
                pTangent->Scale (tangent, 1.0 / segmentLength);
            return  true;
            }
        }

    /* Fall out if point never found.*/
    if (pPoint)
        pPoint->Zero ();
    if (pTangent)
        pTangent->Zero ();
    return false;
    }

/* MAP bsiGeom_pointsAlongPolyline=Geom.pointsAlongPolyline ENDMAP */

/*-----------------------------------------------------------------*//**
* @description Construct points at equal arc distances along a prior (possibly
* unevenly spaced) polyline.
* Note that distance between the new points is measured along the old
* polyline, not directly between new points.
*
* @param pNewPoint <= interpolated point
* @param maxCount => max point to be returned
* @param pPointArray => points in polyline
* @param n => number of points in polyline
* @param dist => distance between points
* @return number of points actually placed
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiGeom_pointsAlongPolyline

(
DPoint3dP pNewPoint,
int             maxCount,
DPoint3dCP pPointArray,
int              n,
double           dist
)
    {
    int count = 0;
    double sum, newSum, segmentLength;
    int numSeg = n - 1;
    double relTol = 1.0e-10;
    double nextDist;
    DPoint3d tangent, basePoint;
    double fraction;
    int i;
    if (n >= 2 && maxCount > 0 && dist > 0.0)
        {
        sum = newSum = 0.0;
        pNewPoint[0] = pPointArray[0];
        nextDist = dist;
        count = 1;
        for (i = 0; i < numSeg && count < maxCount; i++)
            {
            basePoint = pPointArray[i];
            tangent.DifferenceOf (pPointArray[ i + 1], basePoint);
            segmentLength = tangent.Magnitude ();
            if (segmentLength > 0.0)
                {
                newSum = sum + segmentLength;
                while (nextDist < newSum && count < maxCount)
                    {
                    fraction = (nextDist - sum) / segmentLength;
                    pNewPoint[ count].SumOf (basePoint, tangent, fraction);
                    count++;
                    nextDist = count * dist;
                    }
                sum = newSum;
                }
            }
        /* Make sure the last point gets in*/
        if (count < maxCount && (sum - ((count - 1) * dist)) > relTol * dist)
            {
            pNewPoint[count++] = pPointArray[n - 1];
            }
        }
    return count;
    }


/*-----------------------------------------------------------------*//**
*
* @param pPoint <= interpolated point
* @param pTangent <= forward tangent at interpolated point.
* @param pParam <= parameter at nearest point
* @param pPointArray => points in polyline
* @param n => number of points in polyline
* @param pTestPoint => space point being projected
* @see
* @return true if polyline has 1 or more points.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_nearestPointOnPolylineExt

(
DPoint3dP pPoint,
DPoint3dP pTangent,
double          *pParam,
DPoint3dCP pPointArray,
int              n,
DPoint3dCP pTestPoint
)
    {
    bool    result = false;
    int i;
    if (n >= 1)
        {
        DPoint3d vectorU, vectorV, point0, nearPoint, vectorW, scaledTangent;
        double d2Min, d2;
        double nearParam;
        double paramStep = n > 1 ? 1.0 / (double)(n - 1) : 1.0;
        double dotUV, dotUU;
        int endPointIndex;
        scaledTangent.Zero ();
        vectorV.DifferenceOf (*pTestPoint, *pPointArray);
        d2Min = vectorV.DotProduct (vectorV);
        nearParam = 0.0;
        endPointIndex = 0;
        point0 = nearPoint = pPointArray[0];

        /* Each step considers projection to a single line segment and to the segement endpoint*/
        /* For n==1, the loop setup gives the final result also.*/
        for (i = 1; i < n; i++)
            {
            vectorU.DifferenceOf (pPointArray[ i], point0);
            dotUV = vectorU.DotProduct (vectorV);
            dotUU = vectorU.DotProduct (vectorU);

            /* Does the point project strictly interior to the segment?*/
            /* Strict less than tests protect against dividing by zero on a */
            /* zero length segment.*/
            if (   0.0 < dotUV && dotUV < dotUU)
                {
                double localParam = dotUV / dotUU;
                vectorW.SumOf (vectorV, vectorU, -localParam);
                d2 = vectorW.DotProduct (vectorW);
                if (d2 < d2Min)
                    {
                    d2Min = d2;
                    nearParam = (i - 1) * paramStep + localParam * paramStep;
                    endPointIndex = -1;
                    scaledTangent.Scale (vectorU, (double)(n - 1));
                    nearPoint.SumOf (point0, vectorU, localParam);
                    }
                }

            /* Move point0 and vectorV up to the next point, and test direct distance.*/
            point0 = pPointArray[i];
            vectorV.DifferenceOf (*pTestPoint, point0);
            d2 = vectorV.DotProduct (vectorV);
            if (d2 < d2Min)
                {
                d2Min = d2;
                nearParam = i * paramStep;
                endPointIndex = i;
                nearPoint = point0;
                }
            }

        if (pPoint)
            *pPoint = nearPoint;

        if (pTangent)
        {
        if (n <= 1)
            {
            // Not enough points for a tangent
            scaledTangent.Zero ();
            }
        else if (endPointIndex == -1)
            {
            // Tangent was saved at closest segment.
            }
        else if (endPointIndex == 0)
            {
            // First segment gives tangent.
            vectorU.DifferenceOf (pPointArray[1], pPointArray[0]);
            scaledTangent.Scale (vectorU, (double)(n - 1));
            }
        else if (endPointIndex == n - 1)
            {
            // Last segment gives tangent.
            vectorU.DifferenceOf (pPointArray[n - 1], pPointArray[n - 2]);
            scaledTangent.Scale (vectorU, (double)(n - 1));
            }
        else
            {
            // One of the two nearby segments gives a tangent.
            DPoint3d leftSegment, rightSegment, planeNormal;
            DPoint3d leftUnit, rightUnit;

            // Setup up the forward segement and unit vectors on both incoming and outgoing edges.
            leftSegment.DifferenceOf (pPointArray[endPointIndex], pPointArray[endPointIndex - 1]);
            leftUnit.Normalize (leftSegment);

            rightSegment.DifferenceOf (pPointArray[endPointIndex + 1], pPointArray[endPointIndex]);
            rightUnit.Normalize (rightSegment);

            planeNormal.SumOf (rightUnit, leftUnit);
            vectorV.DifferenceOf (*pTestPoint, pPointArray[endPointIndex]);
            if (vectorV.DotProduct (planeNormal) <= 0.0)
                vectorU = leftSegment;
            else
                vectorU = rightSegment;
            scaledTangent.Scale (vectorU, (double)(n - 1));
            }
        *pTangent = scaledTangent;
        }

        if (pParam)
            *pParam = nearParam;

        result = true;
        }
    else
        {
        if (pPoint)
            pPoint->Zero ();
        if (pTangent)
            pTangent->Zero ();

        if (pParam)
            *pParam = 0.0;

        result = false;
        }

    return result;
    }


/*-----------------------------------------------------------------*//**
*
* @param pPoint <= interpolated point
* @param pParam <= parameter at nearest point
* @param pPointArray => points in polyline
* @param n => number of points in polyline
* @param pTestPoint => space point being projected
* @see
* @return true if polyline has 1 or more points.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_nearestPointOnPolyline

(
DPoint3dP pPoint,
double          *pParam,
DPoint3dCP pPointArray,
int              n,
DPoint3dCP pTestPoint
)
    {
    return bsiGeom_nearestPointOnPolylineExt
                (pPoint, NULL, pParam, pPointArray, n, pTestPoint);
    }

/*-----------------------------------------------------------------*//**
* @param pPointArray IN OUT array to sort
* @param numPoint IN number of points
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3dArray_sortToChainFromSeed

(
DPoint3dP pPointArray,
int         numPoint,
DPoint3dCP pSeedPoint
)
    {
    DPoint3d seedPoint = *pSeedPoint;
    int baseIndex, closePointIndex;
    double distanceSquared;

    for (baseIndex = 0; baseIndex < numPoint - 1; baseIndex++)
        {
        /* In baseIndex..end, find the closest point ... */
        closePointIndex = baseIndex + bsiGeom_nearestPointinDPoint3dArray (&distanceSquared,
                        pPointArray + baseIndex,
                        numPoint - baseIndex,
                        &seedPoint);
        /* swap it back to the base index */
        seedPoint = pPointArray [closePointIndex];
        pPointArray [closePointIndex] = pPointArray [baseIndex];
        pPointArray [baseIndex] = seedPoint;
        /* and contine with new base point as seed */
        }
    }

/*-----------------------------------------------------------------*//**
* Sort points in place to a heuristically-driven short chain.
* The algorithm is to choose a random point (actually, the first) and
* walk to closest neighbors as first pass.   In the first pass sort order,
* find the longest edge and repeat the closest neighbor logic using one end of
* that long edge as seed.
* @param pPointArray IN OUT array to sort
* @param numPoint IN number of points
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3dArray_sortToChain

(
DPoint3dP pPointArray,
int         numPoint
)
    {
    DPoint3d seedPoint;
    double maxDist;
    double currDist;
    int    maxHead;
    int    currHead;

    if (numPoint <= 2)
        return;

    seedPoint = pPointArray[0];
    bsiDPoint3dArray_sortToChainFromSeed (pPointArray, numPoint, &seedPoint);
    maxDist = pPointArray[0].Distance (pPointArray[numPoint - 1]);
    maxHead = 0;

    for (currHead = 1; currHead < numPoint; currHead++)
        {
        currDist = pPointArray[currHead - 1].Distance (pPointArray[currHead]);
        if (currDist > maxDist)
            {
            maxDist = currDist;
            maxHead = currHead;
            }
        }

    if (maxHead != 0)
        {
        seedPoint = pPointArray[maxHead];
        bsiDPoint3dArray_sortToChainFromSeed (pPointArray, numPoint, &seedPoint);
        }
    }

static double sRelTol = 1.0e-12;
/*-----------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
static bool    bsiDPoint3dArray_firstXYIntersectionWithSegment
(
DPoint3dCP pXYZArray,
int i0,
int i1,
bool    bWrapPolyline,
DPoint3dCR point0,
DPoint3dCR point1,
int &iEdgeBase,
double &edgeFraction,
double &segmentFraction
)
    {
    DVec3d vectorA, vectorB, vectorC;
    vectorA.DifferenceOf (point1, point0);
    int iLast = i1 - 1;
    if (bWrapPolyline)
        iLast = i1;
    for (int i = i0; i <= iLast; i++)
        {
        int iNext = i + 1;
        if (i == i1)
            iNext = i0;
        vectorB.DifferenceOf (pXYZArray[iNext], pXYZArray[i]);
        vectorC.DifferenceOf (pXYZArray[i], point0);

        double crossAB = vectorA.CrossProductXY (vectorB);
        double crossCA = vectorC.CrossProductXY (vectorA);
        double crossCB = vectorC.CrossProductXY (vectorB);

        if (crossAB < 0.0)
            {
            crossAB = -crossAB;
            crossCA = -crossCA;
            crossCB = -crossCB;
            }

        if (crossAB == 0.0)
            {
            // Ignore parallel case.
            }
        else
            {
            double e = sRelTol * crossAB;
            if (  crossCB >= -e&& crossCB <= crossAB + e
                && crossCA >= -e && crossCA <= crossAB + e)
                {
                segmentFraction = crossCB / crossAB;
                edgeFraction    = crossCA / crossAB;
                iEdgeBase       = i;
                return true;
                }
            }
        }
    return false;
    }

// Check if a non-trivial closure edge is needed.
// Coordinates of "final" edge are always copied.
// Return false if (a) closure not requested or (b) closure already implied by coordinates.
static bool    needClosureEdge
(
DPoint3dCP pXYZ,
int        numXYZ,
bool       bClosed,
DPoint3dP  xyzEdge
)
    {
    xyzEdge[0] = pXYZ[0];
    xyzEdge[1] = pXYZ[numXYZ - 1];
    if (!bClosed)
        return false;
    if (xyzEdge[0].IsEqual (xyzEdge[1]))
        return false;
    return true;
    }

/*-----------------------------------------------------------------*//**
@description Search for any intersection among line segments in two polylines.
@param pPointArrayA IN points on polyline A
@param numA IN number of points in polyline A
@param bCloseA IN true to force additional closure segment on polyline A
@param pPointArrayB IN points on polyline B
@param numB IN number of points in polyline B
@param bCloseB IN true to force additional closure segment on polyline B
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3dArray_polylineClashXY

(
DPoint3dCP pPointArrayA,
int         numA,
bool        bCloseA,
DPoint3dCP pPointArrayB,
int         numB,
bool        bCloseB
)
    {
    int edgeBase = 0;
    double edgeFraction, segmentFraction;
    DPoint3d finalEdgeA[2];
    DPoint3d finalEdgeB[2];

    bool    bNeedClosureEdgeA = needClosureEdge (pPointArrayA, numA, bCloseA, finalEdgeA);
    bool    bNeedClosureEdgeB = needClosureEdge (pPointArrayB, numB, bCloseB, finalEdgeB);

    for (int i0 = 0; i0 < numA - 1; i0++)
        {
        if (bsiDPoint3dArray_firstXYIntersectionWithSegment (pPointArrayB, 0, numB - 1, bNeedClosureEdgeB,
                    pPointArrayA[i0], pPointArrayA[i0+1], edgeBase, edgeFraction, segmentFraction))
            return true;
        }

    if (bNeedClosureEdgeA)
        {
        if (bsiDPoint3dArray_firstXYIntersectionWithSegment (pPointArrayB, 0, numB - 1, bNeedClosureEdgeB,
                    finalEdgeA[0], finalEdgeA[1], edgeBase, edgeFraction, segmentFraction))
            return true;
        }

    return false;
    }
// Record data into output arrays; indices and fractions are just copies; xyz values are interpolated.
static bool    recordInterpolation
(
DPoint3dP   pXYZA,
int         *pIndexA,
double      *pFractionA,
DPoint3dP   pXYZB,
int         *pIndexB,
double      *pFractionB,
int         &numOut,
int         &numExtra,
int         maxOut,
DPoint3dCR  xyzA0,
DPoint3dCR  xyzA1,
int         indexA,
double      fractionA,
DPoint3dCR  xyzB0,
DPoint3dCR  xyzB1,
int         indexB,
double      fractionB
)
    {
    if (numOut < maxOut)
        {
        if (pIndexA)
            pIndexA[numOut] = indexA;
        if (pFractionA)
            pFractionA[numOut] = fractionA;
        if (pXYZA)
            pXYZA[numOut].Interpolate (xyzA0, fractionA, xyzA1);

        if (pIndexB)
            pIndexB[numOut] = indexB;
        if (pFractionB)
            pFractionB[numOut] = fractionB;
        if (pXYZB)
            pXYZB[numOut].Interpolate (xyzB0, fractionB, xyzB1);
        numOut++;
        return true;
        }
    else
        {
        numExtra++;
        return false;
        }
    }

/*-----------------------------------------------------------------*//**
@description Search for all xy intersection among line segments in two polylines.

@param pIntersectionA OUT intersection points on A.
@param pIndexA OUT segment indices on A.
@param pFractionA OUT segment fractions on A.

@param pIntersectionB OUT intersection points on B.
@param pIndexB OUT segment indices on B.
@param pFractionB OUT segment fractions on B.

@param pNumOut OUT number of intersections reported.
@param pNumExtraIntersection OUT number of intersections not reported.
@param maxIntersection IN max allowed in arrays.
@param pPointArrayA IN points on polyline A
@param numA IN number of points in polyline A
@param bCloseA IN true to force additional closure segment on polyline A
@param pPointArrayB IN points on polyline B
@param numB IN number of points in polyline B
@param bCloseB IN true to force additional closure segment on polyline B
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3dArray_polylineIntersectXY
(
DPoint3dP pIntersectionA,
int       *pIndexA,
double    *pFractionA,
DPoint3dP pIntersectionB,
int       *pIndexB,
double    *pFractionB,
int       *pNumOut,
int         *pNumExtraIntersection,
int         maxIntersection,
DPoint3dCP pPointArrayA,
int         numA,
bool        bCloseA,
DPoint3dCP pPointArrayB,
int         numB,
bool        bCloseB
)
    {
    int numOut = 0;
    int numExtra = 0;
    double fractionA, fractionB;
    for (int iA = 0; iA < numA - 1; iA++)
        {
        DPoint3d xyzA0 = pPointArrayA[iA];
        DPoint3d xyzA1 = pPointArrayA[iA+1];
        for (int iB0 = 0; iB0 < numB - 1; )
            {
            int iB1 = numB - 1;
            int iB = 0;
            if (bsiDPoint3dArray_firstXYIntersectionWithSegment (pPointArrayB, iB0, iB1, false,
                        xyzA0, xyzA1, iB, fractionB, fractionA))
                {
                recordInterpolation (
                            pIntersectionA, pIndexA, pFractionA,
                            pIntersectionB, pIndexB, pFractionB,
                            numOut, numExtra, maxIntersection,
                            xyzA0, xyzA1, iA, fractionA,
                            pPointArrayB[iB], pPointArrayB[iB+1], iB, fractionB);
                // Next pass in B loop continues from here.
                iB0 = iB + 1;
                }
            else
                {
                iB0 = numB;
                }
            }
        }
    if (pNumOut)
        *pNumOut = numOut;
    if (pNumExtraIntersection)
        *pNumExtraIntersection = numExtra;
    }


/*-----------------------------------------------------------------*//**
@description Test if a linestring or polygon is self intersecting.
@param pXYZArray IN points on polyline
@param numPoints IN number of points in polyline or polygon
@param bClosed IN true to treat as polygon.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3dArray_isSelfIntersectingXY
(
DPoint3dCP pXYZArray,
int         numXYZ,
bool        bClosed
)
    {
    int edgeBase = 0;
    double edgeFraction, segmentFraction;
    int iLast = numXYZ - 1;
    int jLast = numXYZ - 1;
    DPoint3d xyzClose[2];
    bool    bNeedClosureEdge = needClosureEdge (pXYZArray, numXYZ, bClosed, xyzClose);
    if (bNeedClosureEdge)
        {
        // Check the closure edge against everything except first, last.
        // Remainder acts as simple polyline line.
        if (bsiDPoint3dArray_firstXYIntersectionWithSegment (pXYZArray, 1, numXYZ - 2, false,
                    xyzClose[0], xyzClose[1], edgeBase, edgeFraction, segmentFraction))
            return true;
        }
    else if (bClosed)
        {
        // Closure from coordinates.  Prevent intersection calculation for first, last.
        jLast = numXYZ - 2;
        }

    // Main sweep
    for (int i0 = 0; i0 < iLast; i0++)
        {
        if (bsiDPoint3dArray_firstXYIntersectionWithSegment (pXYZArray, i0 + 2, jLast, false,
                    pXYZArray[i0], pXYZArray[i0+1], edgeBase, edgeFraction, segmentFraction))
            return true;
        // In close-by-coordinate case, jLast kept final segment out of view of first segment.
        // This restores it for later segments ...
        jLast = iLast;
        }
    return false;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
