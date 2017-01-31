/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_intersectxy.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#include <Geom/implicitbezier.fdf>

static double s_relTol = 1.0e-10;
static double s_maxFraction = 1.0e8;

// ExensionFlags carries a boolean for start and end of each of a pair of curves.
struct ExtensionFlags
    {
    // First index is curve selector
    // Second index is start/end selector
    bool bExtend[2][2];

    // Initialize with matched start/end on each of pair.
    ExtensionFlags (bool b0, bool b1)
        {
        bExtend[0][0] = b0;
        bExtend[0][1] = b0;
        bExtend[1][0] = b1;
        bExtend[1][1] = b1;
        }
    // Complete initialization
    ExtensionFlags (bool b00, bool b01, bool b10, bool b11)
        {
        bExtend[0][0] = b00;
        bExtend[0][1] = b01;
        bExtend[1][0] = b10;
        bExtend[1][1] = b11;
        }

    // For a linestring with index range i0<=k<=i1.
    // Given index i at the end of a segment.
    // Assume the ExtensionFlags was initialized with the overall extension condition.
    // Update (in place) to indicate extendability of the current segment.
    ExtensionFlags ApplyEndConditions (int select, int i0, int i, int i1)
        {
        ExtensionFlags newflags = ExtensionFlags (bExtend[0][0], bExtend[0][1], bExtend[1][0], bExtend[1][1]);
        if (select != 0)
            select = 1;
        bExtend[select][0] &= (i == i0 + 1);
        bExtend[select][1] &= (i == i1);
        return newflags;
        }

    // Update each end condition with boolean AND of current value and args.
    ExtensionFlags ApplyEndConditions (int select, bool b0, bool b1)
        {
        ExtensionFlags newflags = ExtensionFlags (bExtend[0][0], bExtend[0][1], bExtend[1][0], bExtend[1][1]);
        if (select != 0)
            select = 1;
        bExtend[select][0] &= b0;
        bExtend[select][1] &= b1;
        return newflags;
        }

    // Return flags with curves swapped.
    ExtensionFlags Reverse ()
        {
        return ExtensionFlags (bExtend[1][0], bExtend[1][1], bExtend[0][0], bExtend[0][1]);
        }

    // Return true if either end extends
    bool Extends (int select)
        {
        if (select != 0)
            select = 1;
        return bExtend[select][0] || bExtend[select][1];
        }
    // Test a fraction on a selected curve
    bool AcceptFraction (double f, int select)
        {
        if (select != 0)
            select = 1;
        if (fabs (f) > s_maxFraction)
            return false;
        if (!bExtend[select][0]
            && f < -s_relTol)
            return false;
        if (!bExtend[select][1]
            && f > 1.0 + s_relTol)
            return false;
        return true;
        }

    // Test two fractions
    bool AcceptFractions (double f0, double f1)
        {
        return AcceptFraction (f0, 0) && AcceptFraction (f1, 1);
        }
    };

bool IsExtensibleSource (GraphicsPointArrayCP pGeom, bool bExtendLines, bool bExtendArcs)
    {
    int i0, i1;
    int count = jmdlGraphicsPointArray_getCount (pGeom);
    i0 = 0;
    int curveType;
    if (!jmdlGraphicsPointArray_parseFragment
                            (
                            pGeom,
                            &i1,
                            NULL, NULL,
                            &curveType,
                            i0
                            ))
        return false;
    if (i1 != count - 1)
        return false;
    if (curveType == 0 && bExtendLines)
        return true;
    if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE && bExtendArcs)
        return true;
    return false;
    }


class XYIntersectParams
{
private:

public:
GraphicsPointArrayCP mpGeomA;
GraphicsPointArrayCP mpGeomB;
bool    mbSameArray;
bool    mbSinglePrimitiveA;
bool    mbSinglePrimitiveB;
bool    mbExtendLines;
bool    mbExtendConic;

XYIntersectParams (
    GraphicsPointArrayCP pGeomA,
    GraphicsPointArrayCP pGeomB,
    bool    bExtendLines,
    bool    bExtendConic
    )
    {
    mpGeomA = pGeomA;
    mpGeomB = pGeomB;
    mbExtendLines  = bExtendLines ? true : false;
    mbExtendConic  = bExtendConic ? true : false;
    mbSameArray    = jmdlGraphicsPointArray_identicalContents (pGeomA, pGeomB) ? true : false;
    //pGeomA         = pGeomA;
    //pGeomB         = pGeomB;
    }

ExtensionFlags GetExtensionFlags ()
    {
    return ExtensionFlags (IsExtensibleSource (mpGeomA, mbExtendLines, mbExtendConic),
                            IsExtensibleSource (mpGeomB, mbExtendLines, mbExtendConic));
    }
};



static bool    rangesOverlapXY
(
DRange3dCP pRange1,
DRange3dCP pRange2
)
    {
    if (   pRange1->low.x > pRange2->high.x
        || pRange1->low.y > pRange2->high.y
        || pRange2->low.x > pRange1->high.x
        || pRange2->low.y > pRange1->high.y
        )
        return  false;
    else
        return  true;
    }

/*---------------------------------------------------------------------------------**//**
* Append a single intersection point to the graphics point array.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    appendPoint
(
        GraphicsPointArray  *pArray,
const   DPoint4d                *pPoint,
        double                  parameter,
        int                     index
)
    {
    GraphicsPoint gp;
    if (pArray)
        {
        bsiGraphicsPoint_initFromDPoint4d (&gp, pPoint, parameter, HPOINT_MASK_POINT, index);
        jmdlGraphicsPointArray_addGraphicsPoint (pArray, &gp);
        }
    }


static void    appendPoint
(
        GraphicsPointArray  *pArray,
const   DPoint4d                *pPoint,
        double                  fraction,
        double                  globalParameter0,
        double                  globalParameter1,
        int                     index
)
    {
    appendPoint (pArray, pPoint,
            globalParameter0 + fraction * (globalParameter1 - globalParameter0),
                    index);
    }
/*---------------------------------------------------------------------------------**//**
* Append intersection data for coordinate line from A, conic from B.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void xyIntersect_lineConic
(
        XYIntersectParams       *pParams,
        GraphicsPointArray  *pSplitA,
        GraphicsPointArray  *pSplitB,
const   DSegment4d              *pSegmentA,
        int                     iA,
const   DConic4d                *pConicB,
        int                     iB,
        ExtensionFlags          &flags
)
    {
    int numRoot, root;
    DPoint4d linePoint[2];
    DPoint4d conicPoint[2];
    double   angle[2];
    double   lineFraction[2];
    double   angleFraction;

    numRoot = bsiDConic4d_intersectDSegment4dXYW (pConicB,
                        conicPoint, angle, linePoint, lineFraction, pSegmentA);
    for (root = 0; root < numRoot; root++)
        {
        angleFraction = bsiDConic4d_angleParameterToFraction (pConicB, angle[root]);
        if (flags.AcceptFractions (lineFraction[root], angleFraction))
            {
            appendPoint (pSplitA, &linePoint[root], lineFraction[root], iA);
            appendPoint (pSplitB, &conicPoint[root], angleFraction, iB);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Append intersection data for coordinate conics.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void xyIntersect_conicConic
(
        XYIntersectParams       *pParams,
        GraphicsPointArray  *pSplitA,
        GraphicsPointArray  *pSplitB,
const   DConic4d                *pConicA,
        int                     iA,
const   DConic4d                *pConicB,
        int                     iB,
        ExtensionFlags          &flags
)
    {
    int numRoot, root;
    DPoint4d pointA[4];
    DPoint4d pointB[4];
    double   angleA[4];
    double   angleB[4];
    double   fractionA, fractionB;

    numRoot = bsiDConic4d_intersectDConic4dXYW (pConicA, pointA, angleA, pointB, angleB, pConicB);

    for (root = 0; root < numRoot; root++)
        {
        fractionA = bsiDConic4d_angleParameterToFraction (pConicA, angleA[root]);
        fractionB = bsiDConic4d_angleParameterToFraction (pConicB, angleB[root]);
        if (flags.AcceptFractions (fractionA, fractionB))
            {
            appendPoint (pSplitA, &pointA[root], fractionA, iA);
            appendPoint (pSplitB, &pointB[root], fractionB, iB);
            }
        }
    }

static void showPoint
(
char const *pName,
int  index,
double f,
DPoint4dR point
)
    {
    double w = point.w;
    if (w == 1.0 || w == 0.0)
        printf ("    %s:%d (f %lg) (%lg %lg %lg @ w = %lg)\n", pName,
                    index, f,
                    point.x, point.y, point.z, w);
    else
        printf ("    %s:%d (f %lg) (%lg %lg %lg @ w = %lg)\n", pName,
                    index, f,
                    point.x / w, point.y / w, point.z / w, w);

    }


// Return true iff fractional positions on a pair of beziers are "head to tail"
// joining points for successvie bezier fragments in the same array -- i.e. successive
//  bezier fragments from a parent bspline.
static bool    headToTail
(
XYIntersectParams   *pParams,
int                 orderA,
int                 iA,
double              fractionA,
int                 orderB,
int                 iB,
double              fractionB
)
    {
    if (pParams->mbSameArray
        && iA + orderA == iB
        && fabs (fractionA - 1.0) <= s_relTol
        && fabs (fractionB)       <= s_relTol
        )
        return true;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Append intersection data for coordinate conics.
* typeMask0 and typeMask1 allow caller to indicate non-bezier parent geometry (e.g. lines, conics)
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void xyIntersect_bezierBezier
(
        XYIntersectParams       *pParams,
        GraphicsPointArray  *pSplitA,
        GraphicsPointArray  *pSplitB,
const   DPoint4d                *pPoleArrayA,
        int                     orderA,
        int                     iA,
        double                  knotA0,
        double                  knotA1,
        DRange3dCP              pRangeA,
const   DPoint4d                *pPoleArrayB,
        int                     orderB,
        int                     iB,
        double                  knotB0,
        double                  knotB1,
        DRange3dCP              pRangeB,
        ExtensionFlags          &flags
)
    {
    static int sNoisy = 0;
    DPoint4d pointA[MAX_BEZIER_ORDER];
    DPoint4d pointB[MAX_BEZIER_ORDER];
    double   paramA[MAX_BEZIER_ORDER];
    double   paramB[MAX_BEZIER_ORDER];
    int numRoot;
    int maxRoot = MAX_BEZIER_ORDER;
    int root;
    double fractionA, fractionB;
    bool    bStat = false;
    double condition;
    static double sConditionLimit = 0.00001;
    int numExtra;

    if (!rangesOverlapXY (pRangeA, pRangeB))
        return;

    /* Hmm... This really only returns roots in 0..1 ??????*/
    bStat = bsiBezierDPoint4d_intersectXYExt
            (pointA, paramA, pointB, paramB, &numRoot, &condition, maxRoot,
                        pPoleArrayA, orderA, pPoleArrayB, orderB, sConditionLimit);

    if (sNoisy)
        {
        printf (" Implicit (iA=%d, iB=%d)(b=%d) (c=%7.2le) (numRoot %d)\n", iA, iB, bStat, condition, numRoot);
        for (root = 0; root < numRoot; root++)
            {
            showPoint ("A", root, paramA[root], pointA[root]);
            showPoint ("B", root, paramB[root], pointB[root]);
            }
        }

    if (!bStat || condition < sConditionLimit)
        {
        bStat = bsiBezierDPoint4d_intersectXY_chordal
            (pointA, paramA, pointB, paramB, &numRoot, &numExtra, maxRoot,
                        pPoleArrayA, orderA, pPoleArrayB, orderB);
        if (sNoisy)
            {
            printf (" Chordal (b=%d) (numRoot %d)\n", bStat, numRoot);
            for (root = 0; root < numRoot; root++)
                {
                showPoint ("A", root, paramA[root], pointA[root]);
                showPoint ("B", root, paramB[root], pointB[root]);
                }
            }
        }

    if (bStat)
        {
        for (root = 0; root < numRoot; root++)
            {
            fractionA = paramA[root];
            fractionB = paramB[root];
            if (headToTail (pParams, orderA, iA, fractionA, orderB, iB, fractionB))
                {
                // Ignore simple joining of adjacent bezier fragments of same curve
                }
            else if (flags.AcceptFractions (fractionA, fractionB))
                {
                appendPoint (pSplitA, &pointA[root], fractionA, knotA0, knotA1, iA);
                appendPoint (pSplitB, &pointB[root], fractionB, knotB0, knotB1, iB);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Append intersection data for coordinate conics.
* typeMask0 and typeMask1 allow caller to indicate non-bezier parent geometry (e.g. lines, conics)
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void xyIntersect_segmentBezier
(
        XYIntersectParams       *pParams,
        GraphicsPointArray  *pSplitA,
        GraphicsPointArray  *pSplitB,
const   DSegment4d              *pSegment,
        int                     iA,
const   DPoint4d                *pPoleArrayB,
        int                     orderB,
        int                     iB,
        double                  knotB0,
        double                  knotB1,
        DRange3dCP              pRangeB,
        ExtensionFlags          &flags
)
    {
    DPoint4d planeCoffs;
    double paramB[MAX_BEZIER_ORDER];
    double paramA;
    int numIntersection, i;
    DMatrix4d worldToLocal;
    DSegment4d localSegment;
    DPoint4d   localPoleArrayB[MAX_BEZIER_ORDER];
    DPoint4d    localPointB, worldPointB;
    DPoint4d    localPointA, worldPointA;

    if (!flags.Extends (0))
        {
        DRange3d rangeA;
        bsiDRange3d_init (&rangeA);
        bsiDRange3d_extendByDPoint4dArray (&rangeA, pSegment->point, 2);
        // we only look at xy - force z overlap.
        rangeA.low.z  = pRangeB->low.z  - 1.0;
        rangeA.high.z = pRangeB->high.z + 1.0;
        if (!rangesOverlapXY (&rangeA, pRangeB))
            return;
        }

    /* Translate to local origin of segment */
    bsiDMatrix4d_initForDPoint4dOrigin (&worldToLocal, NULL, &pSegment->point[0]);
    bsiDSegment4d_transformDMatrix4d (&localSegment, &worldToLocal, pSegment);
    bsiDMatrix4d_multiply4dPoints
                (
                &worldToLocal,
                localPoleArrayB,
                pPoleArrayB,
                orderB
                );

    bsiDSegment4d_getXYWImplicitDPoint4dPlane (&localSegment, &planeCoffs);
    bsiBezierDPoint4d_allDPlane4dIntersections
                (
                paramB, NULL, &numIntersection, MAX_BEZIER_ORDER,
                localPoleArrayB, orderB, &planeCoffs, 2,  false  // EDL -- This was flags.Extends (1).  But we NEVER extend beziers.
                );

    for (i =0; i < numIntersection; i++)
        {
        bsiBezierDPoint4d_evaluateDPoint4d (&localPointB, NULL, localPoleArrayB, orderB, paramB[i]);

        if (bsiDSegment4d_projectDPoint4dCartesianXYW
                            (
                            &localSegment,
                            &localPointA,
                            &paramA,
                            &localPointB
                            ))
            {
            if (flags.AcceptFractions (paramA, paramB[i]))
                {
                bsiDSegment4d_fractionParameterToDPoint4d (pSegment, &worldPointA, paramA);
                bsiBezierDPoint4d_evaluateDPoint4d (&worldPointB, NULL, pPoleArrayB, orderB, paramB[i]);
                appendPoint (pSplitA, &worldPointA, paramA, iA);
                appendPoint (pSplitB, &worldPointB, paramB[i], knotB0, knotB1, iB);
                }
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* Append intersection data for coordinate conic and coordinate bezier.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void xyIntersect_bezierConic
(
        XYIntersectParams       *pParams,
        GraphicsPointArray  *pSplitA,
        GraphicsPointArray  *pSplitB,
const   DPoint4d                *pPoleArrayA,
        int                     orderA,
        int                     iA,
        double                  knotA0,
        double                  knotA1,
        DRange3dCP              pRangeA,
const   DConic4d                *pConicB,
        int                     iB,
        ExtensionFlags          &flags
)
    {
    DPoint4d pointA[MAX_BEZIER_ORDER];
    DPoint4d pointB[MAX_BEZIER_ORDER];
    double   paramA[MAX_BEZIER_ORDER];
    double   paramB[MAX_BEZIER_ORDER];
    int numRoot;
    int maxRoot = MAX_BEZIER_ORDER;
    int root;
    double angleFraction;
    bool    extendConic = flags.bExtend[1][0] ? true : false;   // conic extension is all or nothing.

    if (bsiBezierDPoint4d_intersectDConic4dXYExt
            (pointA, paramA, pointB, paramB, &numRoot, maxRoot,
                        pPoleArrayA, orderA, pConicB, extendConic ))
        {
        for (root = 0; root < numRoot; root++)
            {
            angleFraction = bsiDConic4d_angleParameterToFraction (pConicB, paramB[root]);
            appendPoint (pSplitA, &pointA[root], paramA[root], knotA0, knotA1, iA);
            appendPoint (pSplitB, &pointB[root], angleFraction, iB);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Append intersection data for coordinate line from A, fragment from B.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    xyIntersect_lineFragment
(
        XYIntersectParams       *pParams,
        GraphicsPointArray  *pSplitA,
        GraphicsPointArray  *pSplitB,
const   DSegment4d              *pSegmentA,
        int                     iA,
GraphicsPointArrayCP pGeomB,
        int                     j0,
        int                     j1,
        int                     curveType,
        ExtensionFlags          &flags
)
    {
    DConic4d conic;
    DPoint4d poleArray[MAX_BEZIER_CURVE_ORDER];
    int order;
    DPoint4d        pointA, pointB;
    DSegment4d      segmentB;
    double paramA, paramB;
    int mask;
    int j;

    if (curveType == 0)
        {
        for (j = j0; j <= j1; j++)
            {
            jmdlGraphicsPointArray_getDPoint4dWithMask (pGeomB, &segmentB.point[1], &mask, j);
            if (j > j0)
                {
                ExtensionFlags segmentFlags = flags;
                segmentFlags.ApplyEndConditions (1, j0, j, j1);
                if (bsiDSegment4d_intersectXYDSegment4dDSegment4d
                                        (
                                        &pointA,
                                        &paramA,
                                        &pointB,
                                        &paramB,
                                        pSegmentA,
                                        &segmentB
                                        )
                    && segmentFlags.AcceptFractions (paramA, paramB)
                    )
                    {
                    appendPoint (pSplitA, &pointA, paramA, iA);
                    appendPoint (pSplitB, &pointB, paramB, j - 1 );
                    }
                }
            segmentB.point[0] = segmentB.point[1];
            }
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
        {
        j = j0;
        if (jmdlGraphicsPointArray_getDConic4d (pGeomB, &j, &conic, NULL, NULL, NULL, NULL))
            {
            xyIntersect_lineConic (pParams, pSplitA, pSplitB, pSegmentA, iA, &conic, j0, flags);
            }
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) //BSPLINE_CODED
        {
        j = j0;
        if (jmdlGraphicsPointArray_getBezier
                    (pGeomB, &j, poleArray, &order, MAX_BEZIER_CURVE_ORDER))
            {
            DRange3d rangeB;
            bsiBezierDPoint4d_getDRange3d (&rangeB, poleArray, order);
            xyIntersect_segmentBezier (pParams, pSplitA, pSplitB,
                            pSegmentA, iA,
                            poleArray, order, j0, 0.0, 1.0, &rangeB, flags);
            }
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
        {
        double knot0, knot1;
        bool isNullInterval;
        for (size_t spanIndex = 0; pGeomB->GetBezierSpanFromBsplineCurve (j0, spanIndex,
                                    poleArray, order, MAX_BEZIER_CURVE_ORDER, isNullInterval, knot0, knot1);
                    spanIndex++)
            {
            DRange3d rangeB;
            bsiBezierDPoint4d_getDRange3d (&rangeB, poleArray, order);
            xyIntersect_segmentBezier (pParams, pSplitA, pSplitB,
                            pSegmentA, iA,
                            poleArray, order, j0, knot0, knot1, &rangeB, flags);
            }
        }

    }
/*---------------------------------------------------------------------------------**//**
* Append intersection data for coordinate conic from A, fragment from B.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    xyIntersect_bezierFragment
(
        XYIntersectParams       *pParams,
        GraphicsPointArray  *pSplitA,
        GraphicsPointArray  *pSplitB,
const   DPoint4d                *pPoleArrayA,
        int                     orderA,
        int                     iA,
        double                  knotA0,
        double                  knotA1,
        DRange3dCP              pRangeA,
GraphicsPointArrayCP pGeomB,
        int                     j0,
        int                     j1,
        int                     curveType,
        ExtensionFlags          &flags
)
    {
    DConic4d conicB;
    DPoint4d poleArrayB[MAX_BEZIER_CURVE_ORDER];
    int orderB;
    DSegment4d      segmentB;
    int mask;
    int j;

    if (curveType == 0)
        {
        for (j = j0; j <= j1; j++)
            {
            jmdlGraphicsPointArray_getDPoint4dWithMask (pGeomB, &segmentB.point[1], &mask, j);
            if (j > j0)
                {
                xyIntersect_segmentBezier (pParams, pSplitB, pSplitA,
                                &segmentB, j - 1,
                                pPoleArrayA, orderA, iA, knotA0, knotA1, pRangeA,
                                flags
                                );
                }
            segmentB.point[0] = segmentB.point[1];
            }
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
        {
        j = j0;
        if (jmdlGraphicsPointArray_getDConic4d (pGeomB, &j, &conicB, NULL, NULL, NULL, NULL))
            {
            xyIntersect_bezierConic (pParams, pSplitA, pSplitB, pPoleArrayA, orderA, iA, knotA0, knotA1, pRangeA, &conicB, j0, flags);
            }
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) //BSPLINE_CODED
        {
        DRange3d rangeB;
        j = j0;
        if (jmdlGraphicsPointArray_getBezierAndDRange3d
                    (pGeomB, &j, poleArrayB, &orderB, &rangeB, MAX_BEZIER_CURVE_ORDER))
            {
            xyIntersect_bezierBezier (pParams, pSplitA, pSplitB,
                            pPoleArrayA, orderA, iA, knotA0, knotA1, pRangeA,
                            poleArrayB, orderB, j0, 0.0, 1.0, &rangeB, flags);
            }
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
        {
        double knotB0, knotB1;
        bool isNullInterval;
        for (size_t spanIndex = 0; pGeomB->GetBezierSpanFromBsplineCurve (j0, spanIndex,
                                    poleArrayB, orderB, MAX_BEZIER_CURVE_ORDER, isNullInterval, knotB0, knotB1);
                    spanIndex++)
            {
            DRange3d rangeB;
            bsiBezierDPoint4d_getDRange3d (&rangeB, poleArrayB, orderB);
            xyIntersect_bezierBezier (pParams, pSplitA, pSplitB,
                            pPoleArrayA, orderA, iA, knotA0, knotA1, pRangeA,
                            poleArrayB, orderB, j0, knotB0, knotB1, &rangeB, flags);
            }        
        }
    }

/*---------------------------------------------------------------------------------**//**
* Append intersection data for coordinate conic from A, fragment from B.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    xyIntersect_conicFragment
(
        XYIntersectParams       *pParams,
        GraphicsPointArray  *pSplitA,
        GraphicsPointArray  *pSplitB,
const   DConic4d                *pConicA,
        int                     iA,
GraphicsPointArrayCP pGeomB,
        int                     j0,
        int                     j1,
        int                     curveType,
        ExtensionFlags          flags
)
    {
    DConic4d conicB;
    DPoint4d poleArray[MAX_BEZIER_CURVE_ORDER];
    int order;
    DSegment4d      segmentB;
    int mask;
    int j;

    if (curveType == 0)
        {
        for (j = j0; j <= j1; j++)
            {
            jmdlGraphicsPointArray_getDPoint4dWithMask (pGeomB, &segmentB.point[1], &mask, j);
            if (j > j0)
                {
                ExtensionFlags segmentFlags = flags.Reverse ();
                segmentFlags.ApplyEndConditions (0, j0, j, j1);
                xyIntersect_lineConic (pParams, pSplitB, pSplitA,
                            &segmentB, j - 1,
                            pConicA, iA, segmentFlags);
                }
            segmentB.point[0] = segmentB.point[1];
            }
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
        {
        j = j0;
        if (jmdlGraphicsPointArray_getDConic4d (pGeomB, &j, &conicB, NULL, NULL, NULL, NULL))
            {
            xyIntersect_conicConic (pParams, pSplitA, pSplitB, pConicA, iA, &conicB, j0, flags);
            }
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE_CODED
        {
        DRange3d range;
        j = j0;
        if (jmdlGraphicsPointArray_getBezierAndDRange3d
                    (pGeomB, &j, poleArray, &order, &range, MAX_BEZIER_CURVE_ORDER))
            {
            ExtensionFlags segmentFlags = flags.Reverse ();
            xyIntersect_bezierConic (pParams, pSplitB, pSplitA, poleArray, order, j0, 0.0, 1.0, &range, pConicA, iA, segmentFlags);
            }
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
        {
        double knot0, knot1;
        bool isNullInterval;
        for (size_t spanIndex = 0; pGeomB->GetBezierSpanFromBsplineCurve (j0, spanIndex,
                                    poleArray, order, MAX_BEZIER_CURVE_ORDER, isNullInterval, knot0, knot1);
                                    spanIndex++)
            {
            DRange3d rangeB;
            bsiBezierDPoint4d_getDRange3d (&rangeB, poleArray, order);
            ExtensionFlags segmentFlags = flags.Reverse ();
            xyIntersect_bezierConic (pParams, pSplitB, pSplitA, poleArray, order, j0, knot0, knot1, &rangeB, pConicA, iA, segmentFlags);
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* Append intersection data for coordinate line from A, all of B.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    xyIntersect_lineArray
(
        XYIntersectParams       *pParams,
        GraphicsPointArray  *pSplitA,
        GraphicsPointArray  *pSplitB,
const   DSegment4d              *pSegmentA,
        int                     iA,
GraphicsPointArrayCP pGeomB,
        int                     iB0,
        ExtensionFlags          &flags
)
    {
    int j0, j1;
    int curveType;

    for (j0 = iB0;
         jmdlGraphicsPointArray_parseFragment
                (pGeomB, &j1, NULL, NULL, &curveType, j0);
         j0 = j1 + 1)
        {
        xyIntersect_lineFragment
                (
                pParams,
                pSplitA, pSplitB,
                pSegmentA, iA,
                pGeomB, j0, j1, curveType, flags
                );
        }
    }

/*---------------------------------------------------------------------------------**//**
* Append intersection data for coordinate line from A, all of B.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    xyIntersect_conicArray
(
        XYIntersectParams       *pParams,
        GraphicsPointArray  *pSplitA,
        GraphicsPointArray  *pSplitB,
const   DConic4d                *pConicA,
        int                     iA,
GraphicsPointArrayCP pGeomB,
        int                     iB0,
        ExtensionFlags          &flags
)
    {
    int j0, j1;
    int curveType;

    for (j0 = iB0;
         jmdlGraphicsPointArray_parseFragment
                (pGeomB, &j1, NULL, NULL, &curveType, j0);
         j0 = j1 + 1)
        {
        xyIntersect_conicFragment
                (pParams, pSplitA, pSplitB, pConicA, iA, pGeomB, j0, j1, curveType, flags);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Append intersection data for coordinate bezier from A, all of B.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    xyIntersect_bezierArray
(
        XYIntersectParams       *pParams,
        GraphicsPointArray  *pSplitA,
        GraphicsPointArray  *pSplitB,
const   DPoint4d                *pPoleArrayA,
        int                     orderA,
        int                     iA,
        double                  knotA0,
        double                  knotA1,
GraphicsPointArrayCP pGeomB,
        int                     iB0,
        ExtensionFlags          flags
)
    {
    int j0, j1;
    int curveType;
    DRange3d rangeA;
    bsiBezierDPoint4d_getDRange3d (&rangeA, pPoleArrayA, orderA);

    for (j0 = iB0;
         jmdlGraphicsPointArray_parseFragment
                (pGeomB, &j1, NULL, NULL, &curveType, j0);
         j0 = j1 + 1)
        {
        xyIntersect_bezierFragment
                (pParams, pSplitA, pSplitB, pPoleArrayA, orderA, iA, knotA0, knotA1, &rangeA, pGeomB, j0, j1, curveType, flags);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Append intersection data for indexed fragment from A, all of B after specified start.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    xyIntersect_fragmentArray
(
        XYIntersectParams       *pParams,
        GraphicsPointArray  *pSplitA,
        GraphicsPointArray  *pSplitB,
GraphicsPointArrayCP pGeomA,
        int                     i0,
        int                     i1,
        int                     curveType,
GraphicsPointArrayCP pGeomB,
        int                     iB0
)
    {
    DConic4d conic;
    DPoint4d poleArray[MAX_BEZIER_CURVE_ORDER];
    int order;
    DSegment4d segment;
    int mask;
    ExtensionFlags flags = pParams->GetExtensionFlags ();

    if (curveType == 0)
        {
        int i;
        for (i = i0; i <= i1; i++)
            {
            jmdlGraphicsPointArray_getDPoint4dWithMask (pGeomA, &segment.point[1], &mask, i);
            if (i > i0)
                {
                ExtensionFlags segmentFlags = pParams->GetExtensionFlags ();
                segmentFlags.ApplyEndConditions (0, i0, i, i1);
                if (!pParams->mbSameArray || iB0 != i0)
                    {
                    xyIntersect_lineArray (pParams, pSplitA, pSplitB, &segment, i - 1, pGeomB, iB0, segmentFlags);
                    }
                else
                    {
                    /* When computing self intersections, pGeomB has same indices as pGeomA.
                        Only do pGeomB for remainder of this linestring.
                        Don't intersect with immediate successor ... */
                    if (i < i1 -1 )
                        {
                        // There are still pieces of this linestring out there starting at i.
                        // Suppress possible backward extension in the tail linestring ..
                        ExtensionFlags tailFlags = segmentFlags;
                        tailFlags.ApplyEndConditions (1, false, true);
                        xyIntersect_lineArray (pParams, pSplitA, pSplitB, &segment, i - 1, pGeomB, i + 1, tailFlags);
                        }
                    else
                        {
                        xyIntersect_lineArray (pParams, pSplitA, pSplitB, &segment, i - 1, pGeomB, i1 + 1, segmentFlags);
                        }

                    }
                }
            segment.point[0] = segment.point[1];
            }
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
        {
        int i = i0;
        if (jmdlGraphicsPointArray_getDConic4d (pGeomA, &i, &conic, NULL, NULL, NULL, NULL))
            {
            xyIntersect_conicArray (pParams, pSplitA, pSplitB, &conic, i0, pGeomB, iB0, flags);
            }
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER)  // BSPLINE_CODED
        {
        int i = i0;
        if (jmdlGraphicsPointArray_getBezier
                    (pGeomA, &i, poleArray, &order, MAX_BEZIER_CURVE_ORDER))
            {
            xyIntersect_bezierArray (pParams, pSplitA, pSplitB, poleArray, order, i0, 0.0, 1.0, pGeomB, iB0, flags);
            }
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
        {
        double knot0, knot1;
        bool isNullInterval;
        for (size_t spanIndex = 0; pGeomA->GetBezierSpanFromBsplineCurve (i0, spanIndex,
                                    poleArray, order, MAX_BEZIER_CURVE_ORDER, isNullInterval, knot0, knot1);
                    spanIndex++)
            {
            xyIntersect_bezierArray (pParams, pSplitA, pSplitB, poleArray, order, i0, knot0, knot1, pGeomB, iB0, flags);
            }
        }

    }


/*---------------------------------------------------------------------------------**//**
* Append intersection data for all of A, all of B.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    xyIntersect_arrayArray
(
        GraphicsPointArray  *pSplitA,
        GraphicsPointArray  *pSplitB,
GraphicsPointArrayCP pGeomA,
GraphicsPointArrayCP pGeomB,
        bool                    extendLines,
        bool                    extendConics
)
    {
    XYIntersectParams params (pGeomA, pGeomB, extendLines, extendConics);
    int i0, i1;
    int curveType;

    for (i0 = 0;
         jmdlGraphicsPointArray_parseFragment
                            (
                            pGeomA,
                            &i1,
                            NULL, NULL,
                            &curveType,
                            i0
                            );
        i0 = i1 + 1)
        {
        if (params.mbSameArray)
            {
            if (curveType == 0)
                {
                /* Special case -- allow linestring self intersection */
                xyIntersect_fragmentArray
                    (&params, pSplitA, pSplitB, pGeomA, i0, i1, curveType, pGeomB, i0);
                }
            else
                {
                /* Only consider following geometry */
                xyIntersect_fragmentArray
                    (&params, pSplitA, pSplitB, pGeomA, i0, i1, curveType, pGeomB, i1 + 1);
                }
            }
        else
            xyIntersect_fragmentArray
                (&params, pSplitA, pSplitB, pGeomA, i0, i1, curveType, pGeomB, 0);
        }
    }


/*---------------------------------------------------------------------------------**//**
* Compute split points where geometry from pGeomA intersects with pGeomB, using
*   apparent intersection in XY plane.
* Each split point is represented by a graphics point.
* @param pSplitA <= array of intersection point, primitive index, and primitive parameter. May be NULL.
* @param pSplitB <= array of intersection point, primitive index, and primitive parameter. May be NULL.
* @param pGeomA => candidate geoemtry.
* @param pGeomB => candidate geometry.
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_xyIntersectionPoints
(
        GraphicsPointArray  *pSplitA,
        GraphicsPointArray  *pSplitB,
GraphicsPointArrayCP pGeomA,
GraphicsPointArrayCP pGeomB
)
    {
    xyIntersect_arrayArray (pSplitA, pSplitB, pGeomA, pGeomB, false, false);
    }


/*---------------------------------------------------------------------------------**//**
* Compute split points where geometry from pGeomA intersects with pGeomB, using
*   apparent intersection in XY plane.
* Each split point is represented by a graphics point.
* @param pSplitA <= array of intersection point, primitive index, and primitive parameter. May be NULL.
* @param pSplitB <= array of intersection point, primitive index, and primitive parameter. May be NULL.
* @param pGeomA => candidate geoemtry.
* @param pGeomB => candidate geometry.
* @param extendLines => true for unbounded lines
* @param extendConics => true for unbounded conics
* @bsimethod                                                    EarlinLutz      07/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_xyIntersectionPointsExt
(
        GraphicsPointArray  *pSplitA,
        GraphicsPointArray  *pSplitB,
GraphicsPointArrayCP pGeomA,
GraphicsPointArrayCP pGeomB,
        bool                    extendLines,
        bool                    extendConics
)
    {
    jmdlGraphicsPointArray_empty (pSplitA);
    jmdlGraphicsPointArray_empty (pSplitB);

    xyIntersect_arrayArray (pSplitA, pSplitB, pGeomA, pGeomB, extendLines, extendConics);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    realDistanceSquaredXY
(
double          *pDist2,
DPoint4d        *pPointA,
DPoint4d        *pPointB
)
    {
    DPoint3d    xyzA, xyzB;

    if (bsiDPoint4d_normalize (pPointA, &xyzA) && bsiDPoint4d_normalize (pPointB, &xyzB))
        {
        *pDist2 = bsiDPoint3d_distanceSquaredXY (&xyzA, &xyzB);

        return true;
        }

    return false;
    }


/*---------------------------------------------------------------------------------**//**
* Find tangent discontinuities (essentially, vertices) in curvesA.
* Compute apparent (xy) distance from each to geometry in curvesB.
* Collect those that are within tolerance.
* @param outPointA OUT accumulating points from inCurvesA.
* @param outPointB OUT accumulating points from inCurvesB
* @param curvesA IN source geometry.for discontinuities.
* @param curvesB IN source geometry
* @param maxDist IN max distance to record.
* @bsifunction                                                  EarlinLutz 10/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addDiscontinuityContacts
(
GraphicsPointArray          *pointsA,
GraphicsPointArray          *pointsB,
GraphicsPointArrayCP curvesA,
GraphicsPointArrayCP curvesB,
double                          maxDist
)
    {
    int             i, j, workdim = 2;
    bool            extend = false;
    double          currDist2, maxDist2, relTol = bsiTrig_smallAngle ();
    static double   angleTol = 1.0e-8;
    GraphicsPoint   gpA, gpB;
    GraphicsPointArray  *discontinuityPoints, *nearPoints;

    discontinuityPoints = jmdlGraphicsPointArray_grab ();
    nearPoints = jmdlGraphicsPointArray_grab ();

    maxDist2 = maxDist * maxDist;

    jmdlGraphicsPointArray_addDiscontinuityPoints (discontinuityPoints, curvesA, NULL, 0.0, workdim, -relTol, angleTol);

    // for each "vertex" from curvesA ....
    for (i = 0; jmdlGraphicsPointArray_getGraphicsPoint (discontinuityPoints, &gpA, i); i++)
        {
        // find nearby stuff on curvesB ...
        jmdlGraphicsPointArray_empty (nearPoints);
        jmdlGraphicsPointArray_addPerpendicularsFromDPoint4dExt (nearPoints, curvesB, &gpA.point, workdim, extend);

        for (j = 0; jmdlGraphicsPointArray_getGraphicsPoint (nearPoints, &gpB, j); j++)
            {
            if (realDistanceSquaredXY (&currDist2, &gpA.point, &gpB.point) && currDist2 < maxDist2)
                {
                jmdlGraphicsPointArray_addGraphicsPoint (pointsA, &gpA);
                jmdlGraphicsPointArray_addGraphicsPoint (pointsB, &gpB);
                }
            }
        }

    jmdlGraphicsPointArray_drop (discontinuityPoints);
    jmdlGraphicsPointArray_drop (nearPoints);
    }

END_BENTLEY_GEOMETRY_NAMESPACE
