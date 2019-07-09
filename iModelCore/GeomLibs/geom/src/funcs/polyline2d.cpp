/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE



/*-----------------------------------------------------------------*//**
* @description Compute the nearest point on the polyline.
* @param pPoint <= interpolated point
* @param pParam <= global parameter at nearest point
* @param pNearSegment <= index of nearest segment
* @param pNearSegmentParam <= local parameter at nearest point
* @param pPointArray => points in polyline
* @param n => number of points in polyline
* @param pTestPoint => space point being projected
* @param isClosed => polyline is closed
* @return true if polyline has 1 or more points.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            bsiVector2d_nearestPointOnPolyline

(
DPoint2dP pPoint,
double          *pParam,
int             *pNearSegment,
double          *pNearSegmentParam,
DPoint2dCP pPointArray,
int              n,
DPoint2dCP pTestPoint,
bool             isClosed
)
    {
    bool    result = false;
    int i;
    if (n >= 1)
        {
        DPoint2d vectorU, vectorV, point0, nearPoint, vectorW;
        double d2Min, d2;
        double nearParam;
        double paramStep = n > 1 ? 1.0 / (double)(n - 1) : 1.0;
        double dotUV, dotUU;
        //int    pastPriorSegment = 0;

        vectorV.DifferenceOf (*pTestPoint, *pPointArray);
        d2Min = vectorV.DotProduct (vectorV);
        nearParam = 0.0;
        point0 = nearPoint = pPointArray[0];
        if (pNearSegment)
            {
            *pNearSegment = 0;
            }

        /* Each step considers projection to a single line segment and to the segement endpoint*/
        /* For n==1, the loop setup gives the final result also.*/
        for (i = 1; i < n + isClosed; i++)
            {
            if (i != n)
                {
                vectorU.DifferenceOf (pPointArray[ i], point0);
                }
            else
                {
                vectorU.DifferenceOf (*pPointArray, point0);
                }
            dotUV = vectorU.DotProduct (vectorV);
            dotUU = vectorU.DotProduct (vectorU);

            /* Does the point project strictly interior to the segment?*/
            /* Strict less than tests protect against dividing by zero on a */
            /* zero length segment.*/
            if (0.0 < dotUV && dotUV < dotUU)
                {
                double localParam = dotUV / dotUU;
                vectorW.SumOf (vectorV, vectorU, -localParam);
                d2 = vectorW.DotProduct (vectorW);
                if (d2 < d2Min)
                    {
                    d2Min = d2;
                    nearParam = (i - 1) * paramStep + localParam * paramStep;
                    nearPoint.SumOf (point0, vectorU, localParam);
                    if (pNearSegment)
                        {
                        *pNearSegment = i;
                        }
                    if (pNearSegmentParam)
                        {
                        *pNearSegmentParam = localParam;
                        }
                    }
                }

            /* Move point0 and vectorV up to the next point, and test direct distance.*/
            if (i != n)
                {
                point0 = pPointArray[i];
                }
            else
                {
                point0 = pPointArray[0];
                }

            vectorV.DifferenceOf (*pTestPoint, point0);
            d2 = vectorV.DotProduct (vectorV);
            if (d2 < d2Min)
                {
                d2Min = d2;
                nearParam = i * paramStep;
                nearPoint = point0;
                if (pNearSegment)
                    {
                    *pNearSegment = i;
                    }
                }
            }

        if (pPoint)
            *pPoint = nearPoint;

        if (pParam)
            *pParam = nearParam;

        result = true;
        }
    else
        {
        if (pPoint)
            pPoint->Zero ();

        if (pParam)
            *pParam = 0.0;

        result = false;
        }

    return result;
    }


/*---------------------------------------------------------------------------------**//**
* @description Compute the length of the polyline.
* @param pPointArray    IN      points in polyline
* @param numPoints      IN      number of points in polyline
* @return length of polyline
* @bsimethod                                                    DavidAssaf      01/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bsiDPoint2d_polylineLength

(
DPoint2dCP  pPointArray,
int             numPoints
)
    {
    double  sum = 0.0;
    int     i, numEdges = numPoints - 1;

    for (i = 0; i < numEdges; i++)
        sum += pPointArray[ i].Distance (pPointArray[ i + 1]);

    return sum;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
