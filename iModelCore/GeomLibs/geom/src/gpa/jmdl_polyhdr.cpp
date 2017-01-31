/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/jmdl_polyhdr.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Mtg/jmdl_polyhdr.fdf>
#include <Mtg/jmdl_planeset.fdf>
#include "../memory/jmdl_iarray.fdf"
#include "../memory/jmdl_dpnt3.fdf"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define     TOP                     0
#define     FRONT                   1
#define     SIDE                    2

/*----------------------------------------------------------------------+
|                                                                       |
|   macro definitions                                                   |
|                                                                       |
+----------------------------------------------------------------------*/
#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
#define FIX_MINMAX(value, min, max)  FIX_MIN(value, min); FIX_MAX(value, max);


/* MAP jmdlPolyhedron_getBoundingBox=Geom.getBoundingBox ENDMAP */

/*----------------------------------------------------------------------+
|FUNC           mdl_polyhedronBoundingBox                               |
|AUTHOR         RaghavanKunigahalli                           5/96      |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void jmdlPolyhedron_getBoundingBox
(
DRange3d    *pRangeBox,
DPoint3d    *pPointArray
)

    {
    bsiDRange3d_initFromArray (pRangeBox, pPointArray, 8);
    }

/*----------------------------------------------------------------------+
|FUNC           jmdlPolyhedron_testBoundingBox                          |
|AUTHOR         RaghavanKunigahalli                           5/96      |
+----------------------------------------------------------------------*/
static int jmdlPolyhedron_testBoundingBox
(
const DRange3d  *pBoundBoxOne,
const DRange3d  *pBoundBoxTwo
)
    {  if (pBoundBoxOne->low.x > pBoundBoxTwo->high.x
        || pBoundBoxOne->low.y > pBoundBoxTwo->high.y
        || pBoundBoxOne->low.z > pBoundBoxTwo->high.z
        || pBoundBoxTwo->low.x > pBoundBoxOne->high.x
        || pBoundBoxTwo->low.y > pBoundBoxOne->high.y
        || pBoundBoxTwo->low.z > pBoundBoxOne->high.z
        )
        {
        return false;
        }
    return true;
    }


/*----------------------------------------------------------------------+
|FUNC           jmdlPolyhedron_frustumRangeBoxEnclosed                  |
|AUTHOR         RaghavanKunigahalli                           03/97     |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlPolyhedron_frustumRangeBoxEnclosed
(
DRange3d            *pFrustBoundBox,
DPlane3d_SmallSet   *pPlaneSetP,
const DRange3d      *pRangeBox
)

    {
    DPoint4d            hPlane;
    DPoint3d            origin, normal;
    DPoint3d            rangePoints[MAX_RANGE_POINTS];
    int                 i, status;
    double              minNeg, maxPos;

    if (false == (status = jmdlPolyhedron_testBoundingBox(pRangeBox, pFrustBoundBox)))
        return  false;

    jmdlPolyhedron_getRangeCorners(rangePoints, pRangeBox);

    for (i = 0; i < MAX_FRUST_PLANES; i++)
        {
        jmdlPlaneSet_getOriginAndNormal (&origin, &normal, pPlaneSetP, i);
        status = CHANGE_TO_BOOL(bsiDPoint4d_planeFromOriginAndNormal(&hPlane, &origin, &normal) ? SUCCESS : ERROR);
        jmdlPolyhedronHPlane_getExtremePoints(&minNeg, &maxPos, &hPlane, rangePoints);

        if ((minNeg > 0.0) || (maxPos > 0.0))
            return  false;
        }

     return true;
    }

/*----------------------------------------------------------------------+
| name          mdlPolyhdron_fixMinMax                                  |
| author        RaghavanKunigahalli                             5/96    |
+----------------------------------------------------------------------*/
static void jmdlPolyhedronHPlane_fixMinMax
(
double      *pMinNeg,
double      *pMaxPos,
DPoint4d    *pHPlane,
DPoint3d    *pTestPoint
)
    {
    DPoint4d    hPoint;
    double      dotValue;

    bsiDPoint4d_copyAndWeight(&hPoint, pTestPoint, 1.0);
    dotValue = bsiDPoint4d_dotProduct(pHPlane, &hPoint);
    FIX_MIN(dotValue, *pMinNeg);
    FIX_MAX(dotValue, *pMaxPos);
    }

/*----------------------------------------------------------------------+
| name          jmdlPolyhedron_giftWrapGetTheta2D                       |
| author        RaghavanKunigahalli                 5/96                |
+----------------------------------------------------------------------*/
static double jmdlPolyhedron_giftWrapGetTheta
(
const DPoint3d    *pPoint0,
const DPoint3d    *pPoint1
)
    {
    double dx, dy, ax, ay;
    double t;

    dx = pPoint1->x - pPoint0->x;
    ax = fabs(dx);
    dy = pPoint1->y - pPoint0->y;
    ay = fabs(dy);

    t = (ax+ay == 0.0) ? 0.0 : (double) dy/(ax+ay);

    if (dx < 0.0)
        t = 2.0 - t;
    else if (dy < 0.0)
        t = 4.0 + t;

    return  t*90.0;
    }

/*----------------------------------------------------------------------+
| name          jmdlPolyhedron_giftWrap2D                               |
| author        RaghavanKunigahalli                             5/96    |
|DESC: convex hull algorithm from Algorithms in C by SedgeWick          |
|NOTE: The algorithm in Sedgewick has been modified to include some     |
|special cases.                                                         |
+----------------------------------------------------------------------*/
static int jmdlPolyhedron_giftWrap2D
(
EmbeddedDPoint3dArray    *pPointArray
)
    {
    int         i, j, currMin, min, status, pointCount;
    double      theta, sweepAngle, currAngle, currMaxDist = 0.0;
    const DPoint3d    *pTmpI, *pTmpMin;
    const DPoint3d *pIpoint, *pJpoint;
    DPoint3d    tmpMin;
    bool        horizontal = false;

    pointCount = jmdlVArrayDPoint3d_getCount(pPointArray);

    for (min = 0, i = 1; i < pointCount; i++)
        {
        pTmpI =  jmdlVArrayDPoint3d_getConstPtr(pPointArray, i);
        pTmpMin = jmdlVArrayDPoint3d_getConstPtr(pPointArray, min);

        if (pTmpI->y < pTmpMin->y)
            min = i;
        }

    jmdlVArrayDPoint3d_getDPoint3d (pPointArray, &tmpMin, min);
    jmdlVArrayDPoint3d_addPoint(pPointArray, &tmpMin);
    theta = 0.0;
    currMin = -1;
    for (j = 0; j < pointCount; j++)
        {
        status = jmdlVArrayDPoint3d_swapValues (pPointArray, j, min);

        min = j; currMin = min; sweepAngle = theta; theta = 360.0;
        pJpoint = jmdlVArrayDPoint3d_getConstPtr(pPointArray, j);

        for (i = j + 1; i <= pointCount; i++)
            {
            pIpoint = jmdlVArrayDPoint3d_getConstPtr(pPointArray, i);
            if (bsiDPoint3d_pointEqual (pIpoint, pJpoint))
                {
                i++;
                continue;
                }
            currAngle = jmdlPolyhedron_giftWrapGetTheta(pJpoint, pIpoint);

            if ((theta == 360.0) && (sweepAngle == currAngle) && (currAngle == 0.0))
                {
                double  maxDist;
                horizontal = true;
                maxDist =  bsiDPoint3d_distanceSquared (pIpoint, pJpoint);
                if (maxDist >= currMaxDist)
                    {
                    currMaxDist = maxDist;
                    min = i;
                    continue;
                    }
                }

            if (horizontal)
                {
                horizontal = false;
                theta = currMaxDist = 0.0;
                break;
                }

            if ((currAngle >= sweepAngle) && (currAngle < theta))
                {
                min = i;
                theta = currAngle;
                if (min == pointCount)
                    return  j;
                }

            }
        if (currMin == min)
            break;

        }
     return j;
    }

/*----------------------------------------------------------------------+
| name           jmdlPolyhedron_getCollapsedPolygonRange                |
|AUTHOR         RaghavanKunigahalli                           5/96      |
+----------------------------------------------------------------------*/
static void     jmdlPolyhedron_getCollapsedPolygonRange
(
        DRange3d                *pRange,
const   EmbeddedDPoint3dArray        *pPoint
)
    {
    int i, numCount;
    DPoint3d *pMin, *pMax;
    const DPoint3d  *pIndexPtr;
    pMin = &pRange->low;
    pMax = &pRange->high;
    pIndexPtr = jmdlVArrayDPoint3d_getConstPtr (pPoint, 0);
    numCount = jmdlVArrayDPoint3d_getCount (pPoint);
    pMin->x = pMax->x = pIndexPtr->x;
    pMin->y = pMax->y = pIndexPtr->y;
    pMin->z = pMax->z = 0.0;
    for (i = 1; i < numCount; i++)
        {
        pIndexPtr = jmdlVArrayDPoint3d_getConstPtr (pPoint, i);
        FIX_MINMAX (pIndexPtr->x, pMin->x, pMax->x);
        FIX_MINMAX (pIndexPtr->y, pMin->y, pMax->y);
        }
    }

/*----------------------------------------------------------------------+
|FUNC           testDot                                                 |
|AUTHOR         RaghavanKunigahalli                          04/97      |
+----------------------------------------------------------------------*/
static void jmdlPolyhedron_testDot
(
int             *pCount,
const DPoint3d  *pVVec,
const DPoint3d  *pTestPoint,
const DPoint3d  *pCurrPoint
)
    {
    double epsilon   = -1.0e-12;

    DPoint3d    wVector;
    double      testDot;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&wVector, pTestPoint, pCurrPoint);
    testDot = bsiDPoint3d_dotProduct (pVVec, &wVector);
    if (testDot < epsilon)
        (*pCount)++;
    }

/*----------------------------------------------------------------------+
|FUNC           jmdlPolyhedronPolygon_checkCornerPoints                 |
|AUTHOR         RaghavanKunigahalli                           5/96      |
+----------------------------------------------------------------------*/
static bool    jmdlPolyhedronPolygon_checkCornerPoints
(
const DPoint3d  *pCurrent,
const DPoint3d  *pNext,
const DRange3d  *pRectangle
)
    {
    int         count = 0;
    DPoint3d    testPoint;
    DPoint3d    uVector, vVector;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&uVector, pNext, pCurrent);
    vVector.x = -uVector.y;
    vVector.y = uVector.x;
    vVector.z = 0.0;

    testPoint = pRectangle->low;

    jmdlPolyhedron_testDot(&count, &vVector, &testPoint, pCurrent);

    testPoint.y = pRectangle->high.y;
    jmdlPolyhedron_testDot(&count, &vVector, &testPoint, pCurrent);
    testPoint = pRectangle->high;
    jmdlPolyhedron_testDot(&count, &vVector, &testPoint, pCurrent);
    testPoint.y = pRectangle->low.y;
    jmdlPolyhedron_testDot(&count, &vVector, &testPoint, pCurrent);
    if (count == 4)
        return  false;

    return  true;
    }

/*----------------------------------------------------------------------+
|FUNC           jmdlPolyhedron_rectanglePolygonOverlap                  |
|AUTHOR         RaghavanKunigahalli                           5/96      |
+----------------------------------------------------------------------*/
static bool    jmdlPolyhedron_rectanglePolygonOverlap
(
const DRange3d              *pRectangle,
const EmbeddedDPoint3dArray       *pPolygon
)
    {
    DRange3d    polyRange;
    bool    status = true;
    jmdlPolyhedron_getCollapsedPolygonRange (&polyRange, pPolygon);
    if (!bsiDRange3d_checkOverlap (pRectangle, &polyRange))
        return  false;
    else
        {
        int i;
        const DPoint3d *pPivot, *pCurrent, *pNext;
        int numPoints = jmdlVArrayDPoint3d_getCount (pPolygon);
        pPivot =  jmdlVArrayDPoint3d_getConstPtr (pPolygon, 0);
        for (i = 0; i < numPoints - 1; i++)
            {
            pCurrent = jmdlVArrayDPoint3d_getConstPtr (pPolygon, i);
            pNext = jmdlVArrayDPoint3d_getConstPtr (pPolygon, i+1);
            status = jmdlPolyhedronPolygon_checkCornerPoints (pCurrent, pNext, pRectangle);
            if (status == false)
                return  status;

            }
        pCurrent = jmdlVArrayDPoint3d_getConstPtr (pPolygon, i);
        status = jmdlPolyhedronPolygon_checkCornerPoints (pCurrent, pPivot, pRectangle);
        if (status == false)
                return  status;
        }

    return  true;
    }

/*----------------------------------------------------------------------+
|FUNC           jmdlPolyhedron_getSilhouette                            |
|AUTHOR         RaghavanKunigahalli                           07/96     |
|NOTE: setting VArray count is in general not allowed.  In this case    |
|the gift wrapping algorithm is in-memory algorithm and speed is        |
|very important in skewed range searches that utilizes giftwrapping     |
|algorithm.  Hence, in order to avoid slow down due to unnecessary      |
|copying of the elements in array returned by giftwrapping, the count   |
| in the varray is set directly.                                        |
+----------------------------------------------------------------------*/
static void jmdlPolyhedronVArrayDPoint3d_setCount
(
EmbeddedDPoint3dArray     *pArray,
int                 count
)
    {
    (pArray->vbArray).count = count;
    }

/* MAP jmdlPolyhedron_getSilhouette=Geom.getSilhouette ENDMAP */

/*----------------------------------------------------------------------+
|FUNC           jmdlPolyhedron_getSilhouette                            |
|AUTHOR         RaghavanKunigahalli                           5/96      |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPolyhedron_getSilhouette
(
EmbeddedDPoint3dArray  *pPointArray,    // z-coord will be disregarded
int             projType
)

    {
    int i, pointCount, numHullPoints = 0;
    DPoint3d    *pCurrPoint;

    if (pPointArray)
        {
        pointCount = jmdlVArrayDPoint3d_getCount (pPointArray);

        switch (projType)
            {
            case TOP:
                break;

            case FRONT:
                for (i = 0; i < pointCount; i++)
                    {
                    pCurrPoint = jmdlVArrayDPoint3d_getPtr (pPointArray, i);

                    pCurrPoint->y = pCurrPoint->z;
                    pCurrPoint->z = 0.0;
                    }
                break;

            case SIDE:
                for (i = 0; i < pointCount; i++)
                    {
                    pCurrPoint = jmdlVArrayDPoint3d_getPtr (pPointArray, i);
                    pCurrPoint->x = pCurrPoint->y;
                    pCurrPoint->y = pCurrPoint->z;
                    pCurrPoint->z = 0.0;
                    }
                break;
            }

        numHullPoints =  jmdlPolyhedron_giftWrap2D(pPointArray);
        jmdlPolyhedronVArrayDPoint3d_setCount(pPointArray, ++numHullPoints);
        }
     return numHullPoints;
    }

/* MAP jmdlPolyhedron_getRangeCorners=Geom.getRangeCorners ENDMAP */

/*----------------------------------------------------------------------+
| name          mdlPolyhdron_getRangeBox                                |
| author        RaghavanKunigahalli                             5/96    |
|DESC: will return corner points of a range box in a specific order     |
|the returned points 0,..., 3 will correspond to the plane that         |
|contains the lower range point and points 4,...7 will correspond to    |
|the plane that contains higher range point                             |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void jmdlPolyhedron_getRangeCorners
(
DPoint3d        *pOutPoints, // an array of just eight points
                             // returned points are ordered
const DRange3d  *pRangeBox    // in z-up right handed coord. system
)

    {
    if (pOutPoints)
        {
        pOutPoints[0] = pRangeBox->low;

        pOutPoints[1].x = pRangeBox->high.x;
        pOutPoints[1].y = pRangeBox->low.y;
        pOutPoints[1].z = pRangeBox->low.z;

        pOutPoints[2].x = pRangeBox->low.x;
        pOutPoints[2].y = pRangeBox->high.y;
        pOutPoints[2].z = pRangeBox->low.z;

        pOutPoints[3].x = pRangeBox->high.x;
        pOutPoints[3].y = pRangeBox->high.y;
        pOutPoints[3].z = pRangeBox->low.z;

        pOutPoints[4].x = pRangeBox->low.x;
        pOutPoints[4].y = pRangeBox->low.y;
        pOutPoints[4].z = pRangeBox->high.z;

        pOutPoints[5].x = pRangeBox->high.x;
        pOutPoints[5].y = pRangeBox->low.y;
        pOutPoints[5].z = pRangeBox->high.z;

        pOutPoints[6].x = pRangeBox->low.x;
        pOutPoints[6].y = pRangeBox->high.y;
        pOutPoints[6].z = pRangeBox->high.z;

        pOutPoints[7] = pRangeBox->high;
        }
    }

/* MAP jmdlPolyhedronHPlane_getExtremePoints=Geom.getExtremePoints ENDMAP */

/*----------------------------------------------------------------------+
| name          mdlPolyhdronHPlane_getExtremePoints                     |
| author        RaghavanKunigahalli                             5/96    |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void jmdlPolyhedronHPlane_getExtremePoints
(
double          *pMinNeg,
double          *pMaxPos,
DPoint4d        *pHPlane,
DPoint3d        *pRangePoints
)

    {
    int         i;

    *pMinNeg = 1.0E200;
    *pMaxPos = -1.0E200;

    for (i = 0; i < MAX_RANGE_POINTS;i++)
        {
        jmdlPolyhedronHPlane_fixMinMax(pMinNeg, pMaxPos, pHPlane, &pRangePoints[i]);
        }
    }


/*----------------------------------------------------------------------+
|FUNC           jmdl_planeSetFromFrustumCorners                         |
|AUTHOR         RaghavanKunigahalli                           5/96      |
|DESC loads the plane sets of a view frustum from a given point array   |
|ordered according to the standard corner point indexing                |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt jmdlPolyhedron_planeSetFromFrustumCorners
(
DPlane3d_SmallSet   *planeSetP,
DPoint3d            *pointArray
)

    {
    DPoint3d    normal;
    double      magnitude;
    int         status;

    /* load back plane */
    bsiDPoint3d_crossProduct3DPoint3d(&normal, &pointArray[0], &pointArray[2], &pointArray[1]);
    magnitude = bsiDPoint3d_normalizeInPlace(&normal);

    if (SUCCESS == (status = jmdlPlaneSet_clear(planeSetP)))
        {
        status = jmdlPlaneSet_addByOriginAndNormal(planeSetP, &pointArray[0], &normal);
        }

    /*load front plane */
    bsiDPoint3d_crossProduct3DPoint3d(&normal, &pointArray[4], &pointArray[5], &pointArray[6]);
    magnitude = bsiDPoint3d_normalizeInPlace(&normal);
    status = jmdlPlaneSet_addByOriginAndNormal(planeSetP, &pointArray[4], &normal);

    /*load right plane */
    bsiDPoint3d_crossProduct3DPoint3d(&normal, &pointArray[1], &pointArray[3], &pointArray[5]);
    magnitude = bsiDPoint3d_normalizeInPlace(&normal);
    status = jmdlPlaneSet_addByOriginAndNormal(planeSetP, &pointArray[1], &normal);

    /*load left plane */
    bsiDPoint3d_crossProduct3DPoint3d(&normal, &pointArray[0], &pointArray[4], &pointArray[2]);
    magnitude = bsiDPoint3d_normalizeInPlace(&normal);
    status = jmdlPlaneSet_addByOriginAndNormal(planeSetP, &pointArray[0], &normal);

    /*load top plane */
    bsiDPoint3d_crossProduct3DPoint3d(&normal, &pointArray[2], &pointArray[6], &pointArray[3]);
    magnitude = bsiDPoint3d_normalizeInPlace(&normal);
    status = jmdlPlaneSet_addByOriginAndNormal(planeSetP, &pointArray[2], &normal);

    /*load bottom plane */
    bsiDPoint3d_crossProduct3DPoint3d(&normal, &pointArray[0], &pointArray[1], &pointArray[4]);
    magnitude = bsiDPoint3d_normalizeInPlace(&normal);

    status = jmdlPlaneSet_addByOriginAndNormal(planeSetP, &pointArray[0], &normal);

    return  status;

    }

/* MAP jmdlPolyhedron_frustumRangeBoxOverlap=Geom.frustumRangeBoxOverlap ENDMAP */

/*----------------------------------------------------------------------+
|FUNC           jmdlPolyhedron_frustumRangeBoxOverlap                   |
|AUTHOR         RaghavanKunigahalli                       5/96          |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP RangeTestResult  jmdlPolyhedron_frustumRangeBoxOverlap
(
RangeTest           *pRangeTest,
const DRange3d      *pRangeBox
)
    {
    DPoint3d            rangePoints[MAX_RANGE_POINTS];
    int                 i, status;
    double              minNeg, maxPos;
    int                 count = 0;

    if (pRangeTest->planeState == DRange_null || pRangeTest->rangeState == DRange_null)
        return  Range_Error;

    if (false == (status = jmdlPolyhedron_testBoundingBox(pRangeBox, &pRangeTest->range)))
        return  Range_Out;

    jmdlPolyhedron_getRangeCorners(rangePoints, pRangeBox);

    for (i = 0; i < MAX_FRUST_PLANES; i++)
        {
        jmdlPolyhedronHPlane_getExtremePoints (&minNeg, &maxPos, &pRangeTest->plane[i], rangePoints);
        if ((minNeg > 0.0) && (maxPos > 0.0))
            return  Range_Out;
        else if ((minNeg < 0.0) && (maxPos < 0.0))
            count++;
        }
    if (count == MAX_FRUST_PLANES)
        return  Range_In;

    DRange3d            rect;
    EmbeddedDPoint3dArray     *pSilhouettes;
    int numPoints;
    pSilhouettes = jmdlVArrayDPoint3d_grab();
    status = jmdlVArrayDPoint3d_addArray (pSilhouettes, pRangeTest->cornerPoints, MAX_RANGE_POINTS);

    for (int k = 0; k < MAX_RANGE_POINTS; k++)
        {
        DPoint3d    *pIndexPtr = jmdlVArrayDPoint3d_getPtr (pSilhouettes, k);
        pIndexPtr->z = 0.0;
        }

    numPoints = jmdlPolyhedron_getSilhouette (pSilhouettes, TOP);
    rect.low = rangePoints[0];
    rect.high = rangePoints[7];
    rect.low.z = rect.high.z = 0.0;

    if (!jmdlPolyhedron_rectanglePolygonOverlap(&rect, pSilhouettes))
        {
        pSilhouettes = jmdlVArrayDPoint3d_drop (pSilhouettes);
        return  Range_Out;
        }

    else
        {
        jmdlVArrayDPoint3d_empty (pSilhouettes);
        status = jmdlVArrayDPoint3d_addArray (pSilhouettes, pRangeTest->cornerPoints, MAX_RANGE_POINTS);
        numPoints = jmdlPolyhedron_getSilhouette (pSilhouettes, FRONT);
        rect.low.y = rangePoints[0].z;
        rect.high.y = rangePoints[5].z;
        if (!jmdlPolyhedron_rectanglePolygonOverlap(&rect, pSilhouettes))
            {
            pSilhouettes = jmdlVArrayDPoint3d_drop (pSilhouettes);
            return  Range_Out;
            }

        else
            {
            jmdlVArrayDPoint3d_empty (pSilhouettes);
            status = jmdlVArrayDPoint3d_addArray (pSilhouettes, pRangeTest->cornerPoints, MAX_RANGE_POINTS);
            numPoints = jmdlPolyhedron_getSilhouette (pSilhouettes, SIDE);
            rect.low.x = rangePoints[0].y;
            rect.low.y = rangePoints[0].z;
            rect.high.x = rangePoints[7].y;
            rect.high.y = rangePoints[7].z;
            if (!jmdlPolyhedron_rectanglePolygonOverlap(&rect, pSilhouettes))
                {
                pSilhouettes = jmdlVArrayDPoint3d_drop (pSilhouettes);
                return Range_Out;
                }
            }
        }
    pSilhouettes = jmdlVArrayDPoint3d_drop (pSilhouettes);
    return Range_Partial;
    }



END_BENTLEY_GEOMETRY_NAMESPACE