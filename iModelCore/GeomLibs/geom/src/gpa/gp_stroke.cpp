/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_stroke.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
typedef struct
    {
    double                      chordTolerance;
    double                      angleTolerance;
    double                      maxEdgeLength;
    bool                        movePolesToCurves;
    bool                        xyOnly;
    GraphicsPointArray  *pDest;
    // Computed as needed...
    int                         numTestCalls;
    bool                        weighted;
    double                      minCosine;
    } StrokeOptions;

static void initStrokeOptions
(
StrokeOptions *pOptions,
GraphicsPointArray *pDest,
double chordTolerance,
double angleTolerance,
double maxEdgeLength,
bool    movePolesToCurves,
bool    xyOnly
)
    {
    memset (pOptions, 0, sizeof (*pOptions) );
    pOptions->pDest = pDest;
    pOptions->chordTolerance = chordTolerance;
    pOptions->maxEdgeLength  = maxEdgeLength;
    pOptions->movePolesToCurves = movePolesToCurves;
    pOptions->xyOnly = xyOnly;
    pOptions->numTestCalls = 0;
    pOptions->weighted = false;
    pOptions->minCosine = -2.0;
    if (angleTolerance > 0.0)
        {
        pOptions->minCosine = cos (angleTolerance);
        }
    pOptions->angleTolerance = angleTolerance;
    }

static bool    testStrokes
(
StrokeOptions *pOptions,
const DPoint4d    *pPoleArray,
int         order,
double      s0,
double      s1
)
    {
    if (order < 2)
        return false;
    DPoint3d *pXYZ = (DPoint3d*)_alloca (order * sizeof (DPoint3d));
    int i;
    DPoint3d vector0, vector1;
    DPoint3d vector2;
    DPoint3d projectionVector;
    DPoint3d unit0, unit1;
    double dotProduct;
    double a, a0, a1;
    double dot22, dot02;
    double s;
    if (pOptions->numTestCalls++ == 0)
        {
        pOptions->weighted = !bsiBezierDPoint4d_isUnitWeight (pPoleArray, order, 0.0);
        }

    for (i = 0; i < order; i++)
        {
        pXYZ[i].x = pPoleArray[i].x;
        pXYZ[i].y = pPoleArray[i].y;
        pXYZ[i].z = pPoleArray[i].z;
        }

    if (pOptions->weighted)
        {
        for (i = 0; i < order; i++)
            {
            if (pPoleArray[i].w == 0.0)
                return true;
            a = 1.0 / pPoleArray[i].w;
            pXYZ[i].x *= a;
            pXYZ[i].y *= a;
            pXYZ[i].z *= a;
            }
        }

    if (pOptions->xyOnly)
        {
        for (i = 0; i < order; i++)
            pXYZ[i].z = 0.0;
        }

    /* All sorts of ways to jump out with demand for more subdivision (true),
            have to get by all of them to accept (false). */
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector0, pXYZ+1, pXYZ);
    a0 = bsiDPoint3d_normalize (&unit0, &vector0);
    if (pOptions->maxEdgeLength > 0.0 && a0 > pOptions->maxEdgeLength)
        return true;

    for (i = 2; i < order; i++, vector0 = vector1, unit0 = unit1)
        {
        bsiDPoint3d_subtractDPoint3dDPoint3d (&vector1, pXYZ + i, pXYZ + i - 1);
        a1 = bsiDPoint3d_normalize (&unit1, &vector1);
        if (pOptions->maxEdgeLength > 0.0 && a1 > pOptions->maxEdgeLength)
            return true;
        dotProduct = bsiDPoint3d_dotProduct (&unit0, &unit1);
        if (  pOptions->minCosine >= 0.0
           && dotProduct < pOptions->minCosine)
                return true;
        if (pOptions->chordTolerance > 0.0)
            {
            bsiDPoint3d_subtractDPoint3dDPoint3d (&vector2, pXYZ + i, pXYZ + i - 2);
            dot22 = bsiDPoint3d_dotProduct (&vector2, &vector2);
            dot02 = bsiDPoint3d_dotProduct (&vector0, &vector2);
            if (!bsiTrig_safeDivide (&s, dot02, dot22, 0.0))
                return true;
            bsiDPoint3d_scale (&projectionVector, &vector2, s);
            a = bsiDPoint3d_distance (&projectionVector, &vector0);
            if (a > pOptions->chordTolerance)
                return true;
            }
        }

    return false;
    }

static bool    collectStrokes
(
StrokeOptions *pOptions,
const DPoint4d    *pPoleArray,
int         order,
double      s0,
double      s1
)
    {
    int degree = order - 1;
    int i;
    DPoint4d curvePoint;
    double fraction;
    /* Be careful not to re-issue first point */
    if (s0 <= 0.0)
        jmdlGraphicsPointArray_addDPoint4dArray (pOptions->pDest, pPoleArray, 1);

    if (pOptions->movePolesToCurves)
        {
        /* Send exact curve evaluations along the rest of the control polygon */
        for (i = 1; i < degree; i++)
            {
            fraction = (double)i / (double)degree;
            bsiBezierDPoint4d_evaluateDPoint4d (&curvePoint, NULL, pPoleArray, order, fraction);
            jmdlGraphicsPointArray_addDPoint4d (pOptions->pDest, &curvePoint);
            }

        jmdlGraphicsPointArray_addDPoint4dArray (pOptions->pDest, pPoleArray + degree, 1);
        }
    else
        {
        /* Send the rest of the control polygon as is */
        jmdlGraphicsPointArray_addDPoint4dArray (pOptions->pDest, pPoleArray + 1, degree);
        }
    return true;
    }

static void strokeEllipse
(
DEllipse4d      *pEllipse,
StrokeOptions   *pOptions
)
    {
    double theta0, theta1;
    DConic4d conic;
#define NUM_QUARTIC_POLE 5
    DPoint4d poleArray[NUM_QUARTIC_POLE];
    if (bsiDEllipse4d_getSector (pEllipse, &theta0, &theta1, 0))
        {
        bsiDConic4d_initFrom4dVectors (&conic,
                        &pEllipse->center, &pEllipse->vector0, &pEllipse->vector90, theta0, theta1 - theta0);
        bsiDConic4d_getQuarticBezierPoles (&conic, poleArray, NULL);
        bsiBezierDPoint4d_testAndSubdivide
                        (
                        poleArray,
                        NUM_QUARTIC_POLE,
                        (DPoint4dSubdivisionHandler)testStrokes,
                        pOptions,
                        (DPoint4dSubdivisionHandler)collectStrokes,
                        pOptions,
                        0
                        );
        }
    }


static void strokeBezier
(
GraphicsPointArray     *pDest,
DPoint4d *poleArray,
int numPole,
StrokeOptions   *pOptions,
double      toleranceXY
)
    {
     if (pOptions)
        {
        bsiBezierDPoint4d_testAndSubdivide
                        (
                        poleArray,
                        numPole,
                        (DPoint4dSubdivisionHandler)testStrokes,
                        pOptions,
                        (DPoint4dSubdivisionHandler)collectStrokes,
                        pOptions,
                        0
                        );
        }
    else
        {
        bsiBezierDPoint4d_stroke
                        (
                        poleArray,
                        numPole,
                        toleranceXY,
                        (DPoint4dArrayHandler)jmdlGraphicsPointArray_addDPoint4dArray,
                        pDest
                        );
        }
    }

/*---------------------------------------------------------------------------------**//**
*
* Copy selected lines and curves from the source to the destination, stroking to given tolerance.
* @param        pDest <=> desination header.
* @param        pSource => source header.
* @param        i0 => first index to examine.
* @param        i1 => 1 past last index to examine.
* @param        pOptions => stroke options.  If present, toleranceXY is ignored and
*                       curve is passed to bsiBezierDPoint4d_testAndSubdivide, with max recursion from
*                       options.
* @param        toleranceXY => stroke tolerance, to be applied only to the xy values.  Only used if pOptions
*                       is missing -- final stroking is then via bsiBezierDPoint4d_stroke
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGraphicsPointArray_addStrokesFromSubarray
(
GraphicsPointArrayP pDest,
GraphicsPointArrayCP pSource,
int         i0,
int         i1,
StrokeOptions   *pOptions,
double      toleranceXY
)
    {
    int i;
    int curveType;
    int mask;
    static int maxEllipseStroke = 96;
    DEllipse4d ellipse;
    DPoint4d point;

    for (i = i0; i <= i1;)
        {
        bool isNullInterval;
        size_t iLast;
        curveType = jmdlGraphicsPointArray_getCurveType (pSource, i);
        if (curveType == 0)
            {
            jmdlGraphicsPointArray_getDPoint4dWithMask (pSource, &point, &mask, i);
            jmdlGraphicsPointArray_addDPoint4dWithMask (pDest, &point, mask);
            if (mask & HPOINT_MASK_MAJOR_BREAK)
                jmdlGraphicsPointArray_markMajorBreak (pDest);
            i++;
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            if (jmdlGraphicsPointArray_getDEllipse4d (pSource, &i, &ellipse, NULL, NULL, NULL, NULL))
                {
                jmdlGraphicsPointArray_getDPoint4dWithMask (pSource, &point, &mask, i - 1);
                if (pOptions)
                    {
                    strokeEllipse (&ellipse, pOptions);
                    }
                else
                    {
                    jmdlDEllipse4d_strokeToGraphicsPoints (pDest, &ellipse, maxEllipseStroke, toleranceXY);
                    }
                jmdlGraphicsPointArray_markBreak (pDest);
                if (mask & HPOINT_MASK_MAJOR_BREAK)
                    jmdlGraphicsPointArray_markMajorBreak (pDest);

                jmdlGraphicsPointArray_setArrayMask (pDest, HPOINT_ARRAYMASK_STROKED_DATA);
                }
            else
                {
                /* Unknown curvetype. Skip it. */
                i++;
                }
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE_CODED
            {
            DPoint4d poleArray[MAX_BEZIER_ORDER];
            int      numPole;
            if (jmdlGraphicsPointArray_getBezier (pSource, &i, poleArray, &numPole, MAX_BEZIER_ORDER))
                {
                jmdlGraphicsPointArray_getDPoint4dWithMask (pSource, &point, &mask, i - 1);
                strokeBezier  (pDest, poleArray, numPole, pOptions, toleranceXY);
                /* if (mask & HPOINT_MASK_BREAK) */
                jmdlGraphicsPointArray_markBreak (pDest);
                if (mask & HPOINT_MASK_MAJOR_BREAK)
                    jmdlGraphicsPointArray_markMajorBreak (pDest);

                jmdlGraphicsPointArray_setArrayMask (pDest, HPOINT_ARRAYMASK_STROKED_DATA);
                }
            else
                {
                /* Unknown curvetype. Skip it. */
                i++;
                }
            }
        else if (pSource->IsBsplineCurve ((size_t)i, iLast))
            {
            jmdlGraphicsPointArray_getDPoint4dWithMask (pSource, &point, &mask, i - 1);
            DPoint4d poleArray[MAX_BEZIER_ORDER];
            int      numPole;
            double knot0, knot1;
            for (size_t interval = 0;
                pSource->GetBezierSpanFromBsplineCurve ((size_t)i, interval, poleArray, numPole, MAX_BEZIER_ORDER,
                        isNullInterval, knot0, knot1);
                interval++)
                {
                if (!isNullInterval)
                    strokeBezier (pDest, poleArray, numPole, pOptions, toleranceXY);
                }
            jmdlGraphicsPointArray_markBreak (pDest);
            if (mask & HPOINT_MASK_MAJOR_BREAK)
                jmdlGraphicsPointArray_markMajorBreak (pDest);

            jmdlGraphicsPointArray_setArrayMask (pDest, HPOINT_ARRAYMASK_STROKED_DATA);
            i = (int)iLast + 1;
            }
        else
            {
            /* Unknown curvetype. Skip it. */
            i++;
            }
        }
    }



/*---------------------------------------------------------------------------------**//**
*
* Copy all lines and curves from the source to the destination, stroking to given tolerance.
* @param        pDest <=> desination header.
* @param        pSource => source header.
* @param        toleranceXY => stroke tolerance, to be applied only to the xy values.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addStrokes
(
GraphicsPointArray     *pDest,
GraphicsPointArray     *pSource,
double      toleranceXY
)
    {
    int nSource = jmdlGraphicsPointArray_getCount (pSource);
    jmdlGraphicsPointArray_addStrokesFromSubarray (pDest, pSource, 0, nSource - 1, NULL, toleranceXY);
    }


/*---------------------------------------------------------------------------------**//**
*
* Copy all lines and curves from the source to the destination, stroking to given tolerance.
* @param        pDest <=> desination header.
* @param        pSource => source header.
* @param        chordTolerance => allowed deviation between chord and curve.
* @param        angleTolerance => allowed turn between successive chords
* @param        movePolesToCurves => If true, all output points are required to be
*                       exactly on curves.  If false, bezier curve approximations may
*                       include poles that are within tolerance of the curve but
*                       are not "on" the curve.  Allowing poles-not-on-curve is
*                       faster and will not be visible if the tolerance is a screen
*                       pixel size.
* @param        xyOnly  => only use xy variation for tolerance.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addStrokesExt
(
GraphicsPointArrayP   pDest,
GraphicsPointArrayCP  pSource,
double      chordTolerance,
double      angleTolerance,
double      maxEdgeLength,
bool        movePolesToCurves,
bool        xyOnly
)
    {
    int nSource = jmdlGraphicsPointArray_getCount (pSource);
    StrokeOptions options;
    initStrokeOptions (&options, pDest, chordTolerance, angleTolerance, maxEdgeLength, movePolesToCurves, xyOnly);
    jmdlGraphicsPointArray_addStrokesFromSubarray (pDest, pSource, 0, nSource - 1, &options, 0.0);
    jmdlGraphicsPointArray_normalizeWeights (pDest);
    }

GEOMDLLIMPEXP void  GraphicsPointArray::AddStrokes
(
GraphicsPointArrayCR source,
double      chordTolerance,
double      angleTolerance,
double      maxEdgeLength,
bool        movePolesToCurves,
bool        xyOnly
)
    {
    jmdlGraphicsPointArray_addStrokesExt (this, &source, chordTolerance, angleTolerance, maxEdgeLength,
                    movePolesToCurves  ? true : false,
                    xyOnly ? true : false);
    }

/*---------------------------------------------------------------------------------**//**
* Search ahead of i0 for an explicit major break or end of array.
*
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlGraphicsPointArray_findMajorBreakAfter
(
GraphicsPointArrayCP pSource,
int                             i0
)
    {
    int nSource = jmdlGraphicsPointArray_getCount (pSource);
    const GraphicsPoint *pGPBuffer = jmdlGraphicsPointArray_getConstPtr (pSource, 0);
    int i = i0;
    for (; i < nSource; i++)
        {
        if (pGPBuffer[i].mask & HPOINT_MASK_MAJOR_BREAK)
            return i;
        }
    return nSource - 1;
    }

/*---------------------------------------------------------------------------------**//**
* In the (inclusive) subset of points i0<=i<=i1, clear all masks,
* and normalize all weights to 1.0.
* @param i0 => first point to examine.
* @param i1 => last point to examine.
* @return false if any index is out of bounds or index range is empty.
* @bsimethod                                                    EarlinLutz      01/01
+---------------+---------------+---------------+---------------+---------------+------*/
static double jmdlGraphicsPointArray_clearMaskAndNormalizeWeight
(
GraphicsPointArray  *pSource,
int                     i0,
int                     i1
)
    {
    int nSource = jmdlGraphicsPointArray_getCount (pSource);
    GraphicsPoint *pGPBuffer = jmdlGraphicsPointArray_getPtr (pSource, 0);
    double w, a;

    int i = i0;
    if (   i0 < 0
        || i1 < i0
        || nSource <= i1)
        return false;

    for (i = i0; i <= i1; i++)
        {
        pGPBuffer[i].mask = 0;
        w = pGPBuffer[i].point.w;
        if (w == 1.0)
            {
            /* Just leave it alone */
            }
        else if (w == 0.0)
            return false;
        else
            {
            a = 1.0 / w;
            pGPBuffer[i].point.x *= a;
            pGPBuffer[i].point.y *= a;
            pGPBuffer[i].point.z *= a;
            pGPBuffer[i].point.w = 1.0;
            }
        }

    pGPBuffer[i1].mask = HPOINT_MASK_BREAK | HPOINT_MASK_MAJOR_BREAK;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* In the (inclusive) subset of points i0<=i<=i1, clear all masks,
* and normalize all weights to 1.0.
* @param i0 => first point to examine.
* @param i1 => last point to examine.
* @return false if any index is out of bounds or index range is empty.
* @bsimethod                                                    EarlinLutz      01/01
+---------------+---------------+---------------+---------------+---------------+------*/
static double jmdlGraphicsPointArray_xyPolygonArea
(
GraphicsPointArrayCP pSource,
int                             i0,
int                             i1
)
    {
    int nSource = jmdlGraphicsPointArray_getCount (pSource);
    const GraphicsPoint *pGPBuffer = jmdlGraphicsPointArray_getConstPtr (pSource, 0);
    DPoint2d point0, vector01, vector02;
    double area;
    int i = i0;
    if (   i0 < 0
        || i1 < i0
        || nSource <= i1)
        return 0.0;

    point0.x = pGPBuffer[i0].point.x;
    point0.y = pGPBuffer[i0].point.y;
    vector01.x = pGPBuffer[i0+1].point.x - point0.x;
    vector01.y = pGPBuffer[i0+1].point.y - point0.y;
    area = 0.0;
    for (i = i0 + 2; i <= i1; i++)
        {
        vector02.x = pGPBuffer[i].point.x - point0.x;
        vector02.y = pGPBuffer[i].point.y - point0.y;
        area += vector01.x * vector02.y - vector01.y * vector02.x;
        }
    area *= 0.5;
    return area;
    }

/*---------------------------------------------------------------------------------**//**
*
* Copy all lines and curves from the source to the destination, modified by...
*<ul>
*<li>Stroke to tolerance</li>
*<li>normalize to unit weight</li>
*<li>compute loop areas</li>
*<li>insert disconnect points between successive parent and child loops</li>
*</ul>
* A loop is a child if its area has sign opposite the sign of the very first loops.
* @param        pDest <=> desination header.
* @param        pSource => source header.
* @param        disconnectCoordinate => special coordinate value to use for disconnect points.
*                   If 0, loops are left in original sequence without area analysis.
* @param        toleranceXY => stroke tolerance, to be applied only to the xy values.
* @return       numLoop = number of loops
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int  jmdlGraphicsPointArray_addSimpleStrokesAndDisconnects
(
GraphicsPointArray     *pDest,
GraphicsPointArray     *pSource,
double                     disconnectCoordinate,
double                     toleranceXY
)
    {
    int nSource = jmdlGraphicsPointArray_getCount (pSource);
    int i0, i1;
    int k0, k1;
    int numLoop = 0;
    double refArea = 1.0;
    GraphicsPoint disconnectGP;
    GraphicsPoint *pGP;
    double area;

    memset (&disconnectGP, 0, sizeof (disconnectGP));

    bsiDPoint4d_setComponents
            (&disconnectGP.point, disconnectCoordinate, disconnectCoordinate, disconnectCoordinate, 1.0);

    for (i0 = 0; i0 < nSource; i0 = i1 + 1)
        {
        i1 = jmdlGraphicsPointArray_findMajorBreakAfter (pSource, i0);
        if (i1 <= i0)
            break;
        k0 = jmdlGraphicsPointArray_getCount (pDest);
        jmdlGraphicsPointArray_addStrokesFromSubarray (pDest, pSource, i0, i1, NULL, toleranceXY);
        k1 = jmdlGraphicsPointArray_getCount (pDest) - 1;
        if (jmdlGraphicsPointArray_clearMaskAndNormalizeWeight (pDest, k0, k1))
            {
            jmdlGraphicsPointArray_markBreak (pDest);
            jmdlGraphicsPointArray_markMajorBreak (pDest);
            if (disconnectCoordinate == 0.0)
                {
                /* Leave the loop alone. */
                }
            else
                {
                area = jmdlGraphicsPointArray_xyPolygonArea (pDest, k0, k1);
                if (numLoop == 0)
                    {
                    refArea = area;
                    }
                else
                    {
                    if (area * refArea < 0.0)
                        {
                        /* SUPPRESS the previous indication of a major break  -- instead insert a
                            disconnect before the loop that was just added.
                        */
                        pGP = jmdlGraphicsPointArray_getPtr (pDest, k0 - 1);
                        pGP->mask &= ~HPOINT_MASK_MAJOR_BREAK;
                        jmdlGraphicsPointArray_insertGraphicsPoint (pDest, &disconnectGP, k0);
                        }
                    }
                }
            }
        numLoop++;
        }
    return numLoop;
    }


/*---------------------------------------------------------------------------------**//**
* Replace the (possibly curved) contents of an GraphicsPointArray array by strokes with weight 1.
*
* @param        ppHeader    <=> pointer to pointer to header.   If necessary, this routine may
*                       replace the header by another one.  In this case the original header will
*                       be "dropped" to the cache, and the original allocator may drop the returned
*                       header.
* @param        toleranceXY => stroke tolerance, to be applied only to the xy values.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_dropToStrokes
(
GraphicsPointArray     *pInstance,
double      toleranceXY
)
    {
    if (jmdlGraphicsPointArray_getArrayMask (pInstance, HPOINT_ARRAYMASK_CURVES))
        {
        GraphicsPointArray *pHeader1 = jmdlGraphicsPointArray_grab ();
        jmdlGraphicsPointArray_swap (pInstance, pHeader1);
        jmdlGraphicsPointArray_copyArrayMask (pInstance, pHeader1);
        jmdlGraphicsPointArray_clearArrayMask (pInstance, HPOINT_ARRAYMASK_CURVES);
        jmdlGraphicsPointArray_setArrayMask (pInstance, HPOINT_ARRAYMASK_STROKED_DATA);
        jmdlGraphicsPointArray_addStrokes (pInstance, pHeader1, toleranceXY);
        jmdlGraphicsPointArray_drop (pHeader1);
        }
    else
        {
        /* Just normalize the weights */
        jmdlGraphicsPointArray_normalizeWeights (pInstance);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Add beziers to the array at (possibly) higher degree.
*
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void jmdlGraphicsPointArray_addRaisedDegreeDPoint4dBezier
(
GraphicsPointArray  *pDest,
const DPoint4d *pPoleArray,
int     inputOrder,
int     numSpan,
bool    sharedPoles,
int     targetOrder
)
    {
    DPoint4d poleArray[MAX_BEZIER_CURVE_ORDER];
    int maxCurveOrder = MAX_BEZIER_CURVE_ORDER;
    int span, spanStart;
    int currOrder;

    if (inputOrder == targetOrder)
        {
        jmdlGraphicsPointArray_addDPoint4dBezier (pDest, pPoleArray, inputOrder, numSpan, sharedPoles);
        }
    else if (inputOrder < targetOrder && targetOrder <= maxCurveOrder)
        {
        int spanStep = sharedPoles ? inputOrder - 1 : inputOrder;
        for (span = 0, spanStart = 0; span < numSpan; span++, spanStart += spanStep)
            {
            bsiDPoint4d_copyArray (poleArray, maxCurveOrder, pPoleArray + spanStart, inputOrder);
            currOrder = inputOrder;
            while (currOrder < targetOrder)
                {
                bsiBezier_raiseDegree ((double*)poleArray, (double*)poleArray, currOrder, 4);
                currOrder++;
                }
            jmdlGraphicsPointArray_addDPoint4dBezier (pDest, poleArray, currOrder, 1, false);
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* Copy primitives from pSource to pDest, converting to bezier curves of the
* highest order found in the source.
* @return the order used for the copied geometry.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlGraphicsPointArray_appendAsHighestOrderBezier
(
GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource
)
    {
    int curr0, curr1;
    int targetOrder = jmdlGraphicsPointArray_getHighestBezierOrder (pSource);
    int currOrder = 0;
    DSegment4d segment;
    DConic4d conic;
    DPoint4d poleArray[MAX_BEZIER_CURVE_ORDER];
    int maxCurveOrder = MAX_BEZIER_CURVE_ORDER;
    int readIndex;
    int curveType;
    static int s_fullCircleSpans = 4;

    if (targetOrder >= 2 && targetOrder <= maxCurveOrder)
        {
        for (curr0 = curr1 = -1;
            jmdlGraphicsPointArray_parsePrimitiveAfter (pSource, &curr0, &curr1, NULL, NULL, &curveType, curr1);
            )
            {
            readIndex = curr0;
            if (curveType == 0)
                {
                jmdlGraphicsPointArray_getDSegment4d (pSource, &readIndex, &segment);
                jmdlGraphicsPointArray_addRaisedDegreeDPoint4dBezier
                                (
                                pDest,
                                segment.point,
                                2,
                                1,
                                false,
                                targetOrder
                                );
                }
            else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
                {
                int numPole = 0;
                int numSpan = 0;
                jmdlGraphicsPointArray_getDConic4d (pSource, &readIndex, &conic,
                                    NULL, NULL, NULL, NULL);
                bsiDConic4d_getQuadricBezierPoles (&conic, poleArray, NULL,
                            &numPole, &numSpan, 1 + 2 * s_fullCircleSpans);
                jmdlGraphicsPointArray_addRaisedDegreeDPoint4dBezier
                                    (
                                    pDest,
                                    poleArray,
                                    3,
                                    numSpan,
                                    true,
                                    targetOrder
                                    );
                }
            else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE_CODED
                {
                jmdlGraphicsPointArray_getBezier (pSource, &readIndex, poleArray, &currOrder, maxCurveOrder);
                jmdlGraphicsPointArray_addRaisedDegreeDPoint4dBezier
                                    (
                                    pDest,
                                    poleArray,
                                    currOrder,
                                    1,
                                    false,
                                    targetOrder
                                    );
                }
            else if (pSource->IsBsplineCurve ((size_t)readIndex))
                {
                double knot0, knot1;
                bool isNullInterval;
                for (size_t interval = 0;
                    pSource->GetBezierSpanFromBsplineCurve ((size_t)readIndex, interval, poleArray, currOrder,
                                maxCurveOrder,
                            isNullInterval, knot0, knot1);
                    interval++)
                    {
                    if (!isNullInterval)
                        jmdlGraphicsPointArray_addRaisedDegreeDPoint4dBezier
                                    (
                                    pDest,
                                    poleArray,
                                    currOrder,
                                    1,
                                    false,
                                    targetOrder
                                    );
                    }
                }

            }
        }
    return targetOrder;
    }


/*---------------------------------------------------------------------------------**//**
* Convert all geometry to bezier curves of the highest order found in the
* input.  Append to the output as linestrings tracing the bezier control
* polygons.  Optionally eliminate duplicate endpoints.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlGraphicsPointArray_appendAsHighestOrderBezierPolygons
(
GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource,
bool    eliminateDuplicateEndpoints
)
    {
    /* Copy as beziers */
    int targetOrder = jmdlGraphicsPointArray_appendAsHighestOrderBezier
                        (pDest, pSource);
    GraphicsPoint *pBuffer = jmdlGraphicsPointArray_getPtr (pDest, 0);
    GraphicsPoint gp0, gp1;
    int n0 = jmdlGraphicsPointArray_getCount (pDest);
    int n1;
    int readIndex, lastWriteIndex;
    int i;
    double xyzTol, wTol;
    static double s_xyzAbsTol = 0.0;
    static double s_xyzRelTol = 1.0e-10;
    static double s_wAbsTol   = 1.0e-10;
    static double s_wRelTol   = 1.0e-10;
    jmdlGraphicsPointArray_getTolerances (pSource,
                        &xyzTol, &wTol, s_xyzAbsTol, s_xyzRelTol, s_wAbsTol, s_wRelTol);

   bsiGraphicsPoint_init (&gp0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0);
    n1 = 0;
    readIndex = 0;
    lastWriteIndex   = -1;
    while (readIndex < n0)
        {
        if (   eliminateDuplicateEndpoints
            && lastWriteIndex >= 0
            && bsiDPoint4d_pointEqualMixedTolerance
                            (&pBuffer[lastWriteIndex].point, &pBuffer[readIndex].point, xyzTol, wTol))
            {
            /* Get rid of the previous (already marked) endpoint */
            lastWriteIndex--;
            }

        for (i = 0; i < targetOrder; i++)
            {
            gp1 = pBuffer[readIndex++];
            gp1.mask = 0;
            pBuffer[++lastWriteIndex] = gp1;
            }
        /* Close off the current polygon -- this will be replace if successor matches. */
        pBuffer[lastWriteIndex].mask = HPOINT_MASK_BREAK;
        }
    jmdlGraphicsPointArray_trim (pDest, lastWriteIndex + 1);
    return targetOrder;
    }


/*---------------------------------------------------------------------------------**//**
Copy primitives from pSource to pDest, converting arcs to bezier form.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      jmdlGraphicsPointArray_appendWithArcsAsBezier
(
GraphicsPointArray  *pDest,
GraphicsPointArrayCP pSource
)
    {
    int curr0, curr1;
    DConic4d conic;
    DPoint4d poleArray[MAX_BEZIER_CURVE_ORDER];
    int readIndex;
    int curveType;

    for (curr0 = 0;
        jmdlGraphicsPointArray_parseFragment
                    (pSource, &curr1, NULL, NULL, &curveType, curr0);
        curr0 = curr1 + 1
        )
        {
        readIndex = curr0;
        // Identify and convert conics.  Everything else copies as GP...
        if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            int numPole = 0;
            int numSpan = 0;
            jmdlGraphicsPointArray_getDConic4d (pSource, &readIndex, &conic,
                                NULL, NULL, NULL, NULL);
            bsiDConic4d_getQuadricBezierPoles (&conic, poleArray, NULL,
                        &numPole, &numSpan, MAX_BEZIER_CURVE_ORDER);
            jmdlGraphicsPointArray_addRaisedDegreeDPoint4dBezier
                                (
                                pDest,
                                poleArray,
                                3,
                                numSpan,
                                true,
                                3
                                );
            }
        else
            {
            GraphicsPoint gp;
            int i;
            for (i = curr0; i <= curr1; i++)
                {
                jmdlGraphicsPointArray_getGraphicsPoint (pSource, &gp, i);
                jmdlGraphicsPointArray_addGraphicsPoint (pDest, &gp);
                }
            }
        if (jmdlGraphicsPointArray_isMajorBreak (pSource, curr1))
            jmdlGraphicsPointArray_markMajorBreak (pDest);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     04/2006
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    closePointsXY
(
DPoint4d const *pPointA,
DPoint4d const *pPointB,
double      tol
)
    {
    return fabs (pPointA->x-pPointB->x) < tol
        && fabs (pPointA->y-pPointB->y) < tol;
    }


/*---------------------------------------------------------------------------------**//**
Close small gaps between adjacent endpoints.  Only ends of non-conics are moved.
Gaps LARGER THAN maxGapTol are NOT closed.
(Expected to be used after elimininating conics.)
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      jmdlGraphicsPointArray_forceBezierAndLinestringEndsToNeighbors
(
GraphicsPointArray  *pSource,
double maxGapTol
)
    {
    int indexA, indexB, indexC;
    int curveType;
    bool    bIsMajorBreak;
    GraphicsPoint gpB, gpC;
    int numPoints = jmdlGraphicsPointArray_getCount (pSource);
    int majorBreakIndex;

    for (indexA = majorBreakIndex = 0;
        jmdlGraphicsPointArray_parseFragment
                    (pSource, &indexB, NULL, NULL, &curveType, indexA);
        indexA = indexB + 1
        )
        {
        bIsMajorBreak = jmdlGraphicsPointArray_isMajorBreak (pSource, indexB);
        // Transfer from geometrically next point back to curren tpoint...
        indexC = bIsMajorBreak ? majorBreakIndex : indexB + 1;
        if (indexC < numPoints && curveType != HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            jmdlGraphicsPointArray_getGraphicsPoint (pSource, &gpB, indexB);
            jmdlGraphicsPointArray_getGraphicsPoint (pSource, &gpC, indexC);
            if (closePointsXY (&gpB.point, &gpC.point, maxGapTol))
                gpB.point = gpC.point;
            jmdlGraphicsPointArray_setGraphicsPoint (pSource, &gpB, indexB);
            }

        if (bIsMajorBreak)
            majorBreakIndex = indexB + 1;
        }
    }
END_BENTLEY_GEOMETRY_NAMESPACE