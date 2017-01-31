/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/dpoint2darray.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
* Copies n DPoint2d structures from the pSource array to the pDest
* array.
*
* @param pDest <= destination array
* @param pSource => source array
* @param n => number of points
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2dArray_copy

(
DPoint2dP pDest,
DPoint2dCP pSource,
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
    memcpy (pDest, pSource, n*sizeof(DPoint2d) );
#endif
    }


/*---------------------------------------------------------------------------------**//**
* Copies x and y parts from DPoint3d array.
* @param pDest <= destination array
* @param pSource => source array
* @param n => number of points
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_copyDPoint3dArray

(
DPoint2dP pDest,
DPoint3dCP pSource,
int          n
)
    {
    int i;
    for (i = 0; i < n ;i++)
        {
        pDest[i].x = pSource[i].x;
        pDest[i].y = pSource[i].y;
        }
    }


/*-----------------------------------------------------------------*//**
* Copies n DPoint2d structures from the pSource array to the pDest
* using an index array to rearrange (not necessarily 1to1) the order.
* The indexing assigns pDest[i] = pSource[indexP[i]].
* Does not do in-place rearrangement.
*
* @param pDest <= destination array
* @param pSource => source array
* @param pIndex => array of indices into source array
* @param nIndex => number of points
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2dArray_copyIndexed

(
DPoint2dP pDest,
DPoint2dCP pSource,
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
* Adds the (single) point pPoint to each of the numPoints points
*  of pArray.
*
* @param pArray <=> Array whose points are to be incremented
* @param pDelta => increment to add to each point
* @param numPoints => number of points
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2dArray_addDPoint2d

(
DPoint2dP pArray,
DPoint2dCP pDelta,
int              numPoints
)
    {
    int         i;
    DPoint2d   *pPoint = pArray;
    double      x = pDelta->x;
    double      y = pDelta->y;

    for (i=0; i < numPoints; i++)
        {
        pPoint->x += x;
        pPoint->y += y;
        pPoint++;
        }
    }


/*-----------------------------------------------------------------*//**
* Subtracts the (single) point pDelta from each of the numVerts points
* of pArray.
*
* @param pArray <=> Array whose points are to be incremented
* @param pDelta => increment to subtract from each point
* @param numVerts => number of points
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2dArray_subtractDPoint2d

(
DPoint2dP pArray,
DPoint2dCP pDelta,
int              numVerts
)
    {
    int         i;
    DPoint2d   *pPoint = pArray;
    double      x = pDelta->x;
    double      y = pDelta->y;

    for (i=0; i<numVerts; i++)
        {
        pPoint->x -= x;
        pPoint->y -= y;
        pPoint++;
        }
    }


/*-----------------------------------------------------------------*//**
* Normalize an array of vectors (inplace).  Return the count of the
* number of zerolength vectors encountered.
*
* @param pArray <=> array of vectors to be normalized
* @param numVector => number of vectors
* @see
* @return number of zero length vectors.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDPoint2dArray_normalize

(
DPoint2dP pArray,
int         numVector
)
    {
    double  magnitude;
    int numZero = 0;
    int i;
    DPoint2d *pVec;

    for (pVec = pArray, i = 0; i < numVector; i++, pVec++)
        {
        magnitude = sqrt(pVec->x*pVec->x + pVec->y*pVec->y);

        if (magnitude > 0.0)
            {
            double f = 1.0 / magnitude;
            pVec->x *= f;
            pVec->y *= f;
            }
        else
            {
            numZero++;
            }
        }

    return  numZero;
    }


/*-----------------------------------------------------------------*//**
* Returns an upper bound for both the largest absolute value x,y or z
* coordinate and the greatest distance between any two x,y or z coordinates
* in an array of points.
*
* @param pPointArray => array of points to test
* @param numPoint => number of points > 0
* @see
* @return double
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2dArray_getLargestCoordinate

(
DPoint2dCP pPointArray,
int         numPoint
)
    {
    DRange2d tmpRange;
    bsiDRange2d_initFromArray(&tmpRange, pPointArray, numPoint);
    return (bsiDRange2d_getLargestCoordinate(&tmpRange));
    }

/*-----------------------------------------------------------------*//**
* @description Returns an upper bound for the greatest distance between any two x or y
* coordinates in an array of points.
*
* @param pPointArray => array of points to test
* @param numPoint => number of points > 0
* @return double
* @bsihdr                                       DavidAssaf      06/06
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_getLargestCoordinateDifference

(
DPoint2dCP    pPointArray,
int         numPoint
)
    {
    if (pPointArray && numPoint > 0)
        {
        DRange2d tmpRange;
        DPoint2d diagonal;
        bsiDRange2d_initFromArray (&tmpRange, pPointArray, numPoint);
        bsiDPoint2d_subtractDPoint2dDPoint2d (&diagonal, &tmpRange.high, &tmpRange.low);
        return bsiDPoint2d_maxAbs (&diagonal);
        }

    return 0.0;
    }


/*-----------------------------------------------------------------*//**
* Find two points (and their indices) in the given array of points that are
* relatively far from each other.  The returned points are not guaranteed to be
* the points with farthest separation.
*
* @param pMinPoint  <= first of the two widely separated pts (or null)
* @param  pMinIndex  <= index of first pt (or null)
* @param pMaxPoint  <= second of the two widely separated pts (or null)
* @param pMaxIndex  <= index of second pt (or null)
* @param pPoints    => array of points
* @param numPts     => number of points
* @return false if numPts < 2
* @bsihdr                                       EarlinLutz      12/97
* @bsihdr                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDPoint2dArray_findWidelySeparatedPoints

(
DPoint2dP pMinPoint,
int                 *pMinIndex,
DPoint2dP pMaxPoint,
int                 *pMaxIndex,
DPoint2dCP pPoints,
int                 numPts
)
    {
    double      *pArray;        /* TREAT DPoint2d AS ARRAY OF 3 DOUBLES*/
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
* Comnpute an approximate line through (many) points.
* Return the two points that are the endpoints of the approximate line segment,
* and the length of the segment and max distance of any point from the segment.
* This is useful for testing if the points are collinear.  A typical followup test
* would be to test of maxDist is less than a tolerance and dist01 is larger than
* (say) 1000 times the tolerance.
*
* The returned start and endpoints are selected from the given points -- they
* are not points on some least sqaures approximation that might have a smaller
* maxDist but not pass through any points.
*
* @param pPoint0 <= suggested starting point of the line segment.
* @param pPoint1 <= suggested end point.
* @param pDist01 <= distance from pPoint0 to pPoint1
* @param pMaxDist <= largest distance of any point to the line.
* @param pPoints => array of points
* @param number => number of points

* @return
* @bsihdr                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void        bsiDPoint2dArray_approximateLineThroughPoints

(
DPoint2dP pPoint0,
DPoint2dP pPoint1,
double      *pDist01,
double      *pMaxDist,
DPoint2dCP pPointArray,
int         numPoint
)
    {
    int             i;
    double          dist, maxDist = 0.0;
    DPoint2d        proj, p0, p1, direction;
    double          dist01;

    /* no pts or 1 pt: default to colinear */
    if (!bsiDPoint2dArray_findWidelySeparatedPoints
            (&p0, NULL, &p1, NULL, pPointArray, numPoint))
        {
        dist01 = maxDist = 0.0;
        if (numPoint > 0)
            p0 = p1 = pPointArray[0];
        else
            {
            bsiDPoint2d_zero (&p0);
            p1 = p0;
            }
        }
    else
        {
        double s;
        direction.DifferenceOf (p1, p0);
        dist01 = bsiDPoint2d_normalize (&direction);

        for (i = 0; i < numPoint; i++)
            {
            s = bsiDPoint2d_dotDifference (&pPointArray[i], &p0, &direction);
            proj.SumOf (p0, direction, s);
//              bsiDPoint2d_addScaledPoint (&proj, &p0, &direction, s);
            if ((dist = bsiDPoint2d_distanceSquared (&proj, &pPointArray[i])) > maxDist)
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
* Test if an array of points is effectively a straight line from the first to the last.
*
* @param pOnLine <= true if the points are all within tolerance of the (bounded)
*   line segment from the first point to the last point.
* @param pPoints => array of points
* @param number => number of points
* @return same as pOnLine
* @bsihdr                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDPoint2dArray_arePointsColinear

(
bool        *pOnLine,
DPoint2dCP pPointArray,
int         numPoint,
double      tolerance
)
    {
    DPoint2d    vector;
    double d01, s;
    int i;
    DPoint2d p0, p1, proj;
    static double relTol = 1.0e-12;
    double myTol = bsiDPoint2dArray_getLargestCoordinate (pPointArray, numPoint) * relTol;
    double myTol2;

    if (tolerance > myTol)
        myTol = tolerance;

    myTol2 = myTol * myTol;

    *pOnLine = false;

    if (numPoint < 2)
        return *pOnLine;

    p0 = pPointArray[0];
    p1 = pPointArray[numPoint - 1];
    vector.DifferenceOf (p1, p0);
//      bsiDPoint2d_subtractDPoint2d (&vector, &p1, &p0);
    d01 = bsiDPoint2d_normalize (&vector);

    if (d01 <= myTol)
        return *pOnLine;

    for (i = 1; i < numPoint - 1; i++)
        {
        s = bsiDPoint2d_dotDifference (&pPointArray[i], &p0, &vector);
        proj.SumOf (p0, vector, s);
//        bsiDPoint2d_addScaledPoint (&proj, &p0, &vector, s);
        if (bsiDPoint2d_distanceSquared (&proj, &pPointArray[i]) > myTol2)
            return *pOnLine;
        if (s < 0.0)
            {
            if (-s * d01 > myTol)
                return *pOnLine;
            }
        else if (s > 1.0)
            {
            if ((s - 1.0) * d01 > myTol)
                return *pOnLine;
            }
        }
    *pOnLine = true;
    return *pOnLine;
    }



/*-----------------------------------------------------------------*//**
* Computes the squared distance between two points, using only the
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
Public GEOMDLLIMPEXP int bsiDPoint2dArray_closestPoint

(
DPoint2dCP pPoint,
DPoint2dCP pArray,
int             nPoint
)

    {
    double      xdist, ydist;
    int iCurr, iMin;
    double dMin, dCurr;
    const DPoint2d *pCurr = pArray;
    if ( nPoint < 0)
        return -1;

    iMin = iCurr = 0;
    dMin = bsiDPoint2d_distanceSquared (pPoint, pCurr);

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

typedef struct
    {
        DPoint2d *pOut;
        int numOut;
        double *pError;
        const DPoint2d *pIn;
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
DPoint2dCP pXYZ,
DPoint2dCP pXYZStart,
DPoint2dCP pXYZEnd
)
    {
    DPoint2d vectorU, vectorV, vectorW;
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
    DPoint2d xyz0 = pContext->pIn[i0];
    DPoint2d xyz1 = pContext->pIn[i1];
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
* @description Generate a compressed linestring, with points removed if they are within
* tolerance of the output linestring.
* @param pOut OUT output point buffer.
* @param *pNumOut OUT number of output points.  (At most numIn)
* @param pIn IN input point buffer.
* @param numIn IN number of input points.
* @param tol IN distance tolerance from input point to output segment.
* @remarks For closed polygon, caller must double the first/last point, and this
*       point will also be the first/last of the output.
* @bsihdr                                       EarlinLutz 08/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2dArray_compressLinestring

(
DPoint2dP pOut,
int     *pNumOut,
DPoint2dCP pIn,
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

/*---------------------------------------------------------------------------------**//**
* @description Compute the linear combination of the given points and scalars.
*
* @param pPoint         <= linear combination of points
* @param pPoints        => array of points
* @param pScalars       => array of scalars to multiply each point
* @param numVertices    => number of points = number of scalars
* @bsimethod                                                    DavidAssaf      04/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDPoint2dArray_linearCombination

(
DPoint2dP        pPoint,
DPoint2dCP        pPoints,
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

END_BENTLEY_GEOMETRY_NAMESPACE
