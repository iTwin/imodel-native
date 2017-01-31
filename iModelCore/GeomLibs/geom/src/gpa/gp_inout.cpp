/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_inout.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static int s_debug = 0;

extern void mdlUtil_sortDoubles (double *, int, int);


/* Scan line classification context */
typedef struct
    {
    /* Boundaries of closed loops. */
    GraphicsPointArray *pBoundaryGPA;
    /* Points being classified. */
    GraphicsPointArray *pTestPointGPA;

    double hTolerance;
    double aTolerance;
    DMatrix4d matrix;
    /* Array containing blocks of 5 graphics points
    **  In the first 3 graphics points, the DPoint4d parts
    **  are the A, H ,K plane coefficients of a point being classified at a
    **  certain scan line angle, in the A graphics point, the userData field
    **  is the index of P in the original array.
    ** The final two graphics points are the closests points to left and right,
    **  with sort parameter in the a field.
    **
    ** During scan, the userData fields of the 5 points A, H, K, L, R, ON are as follows:
    **      A.userData = original test point index.
    **      H.userData = number of ON cases recognized -- boundaries passing
    **                      exactly through the test point.
    **      K.userData = number of degenerate scan line hits.  Boundaries with
    **                      endpont ON the scanline but not the test point.
    **      L.userData = simple crossings to left.
    **      R.userData = simple crossings to right.
    **      ON.userData = number of ON points recorded.  The final ON point encountered
    *               is saved.
    **
    */
    double theta;
    GraphicsPointArray *pAHKGPA;
    GraphicsPointArray *pScanDataGPA;
    } SLCContext;

#define OFFSET_A (0)
#define OFFSET_H (1)
#define OFFSET_K (2)
#define OFFSET_L (3)
#define OFFSET_R (4)
#define OFFSET_ON (5)
#define OFFSET_INDEX (0)
#define OFFSET_DEGENERATE_COUNT (1)
#define STATUS_BLOCK_SIZE 6

/*------------------------------------------------------------------------------
*<ul>
*<li> M = [X Y Z W]^ be a (homogeneous, possibly perspective) transformation matrix.</li>
*<li> P = a (pretransform) reference point.</li>
*<li> Q = a (pretransform) point.</li>
*<li> (c,s) be the cosine and sine of a angle.</li>
*<li> C= cX + sY</li>
*<li> S=-sX + cY</li>
*</ul>
* We want to know the (post transformat, cartesian) coordinates of Q relative in an uv system with
* origin at P, u horizontal, v vertical, and also in an (a,h) system where u, v are rotated by the angle.
* (Mnemonics: h is height above the scan line.  "a" is the "a" field of a graphics point -- we will
*       sort along this line.)
*
* The homogeneous transformed points are M*P=[X^P Y^P Z^P W^P], M*Q=[X^Q Y^Q Z^Q W^Q].
* Then the uv and ah coordinates are
*
*<ul>
*<li>u=X^Q/W^Q-X^P/W^P = (X^Q W^P-X^P W^Q)/(W^PW^Q) = Q^(XW^-WX^)P/(P^WW^Q)</li>
*<li>v=Y^Q/W^Q-Y^P/W^P = (Y^Q W^P-Y^P W^Q)/(W^PW^Q) = Q^(YW^-WY^)P/(P^WW^Q)</li>
*<li>a=cu+sv = Q^((cX+sY)W^-W(cX+sY)^)P/(P^WW^Q)</li>
*<li>h=-su+cv = Q^((-sX+cY)W^-W(-sX+cY)^)P/(P^WW^Q)</li>
*</ul>
*
* Consider a scanline along the (a) axis (i.e. the scanline is h=0).  The condition h=0
* only requires then numerator of the (h) coordinate.  Define plane coefficients
*<ul>
*<li>A =((cX+sY)W^-W(cX+sY)^)P=(CW^-WC^)P=C(W^P)-W(C^P) </li>
*<li>H =((-sX+cY)W^-W(-sX+cY)^)P=(SW^-WS^)P=S(W^P)-W(S^P)</li>
*<li>K=WW^P</li>
*</ul>
* In computing a scan line crossing, we need h=0,i.e. Q^H=0, and it is not necessary to do
* the division. The sort coordinate is a=Q^A/Q^K (and the division is needed in order to compare
* different (a) values at various Q points).
*
* Sooooooooooo... the relation of Q to the scan line is determined by equations of 3 planes
* H, A, K extracted from the rows of the transformation matrix.
*
*
* Got it!!
*-------------------------------------------------------------------------------*/
static void    jmdlGPA_SLC_planeCoefficients
(
DPoint4d    *pA,
DPoint4d    *pH,
DPoint4d    *pK,
DMatrix4d   *pM,
DPoint4d    *pP,
double      theta
)
    {
    double cc = cos (theta);
    double ss = sin (theta);
    DPoint4d XX, YY, ZZ, WW;
    DPoint4d CC, SS;
    double WdotP, CdotP, SdotP;

    bsiDMatrix4d_getRowsDPoint4d (pM, &XX, &YY, &ZZ, &WW);
    bsiDPoint4d_add2ScaledDPoint4d (&CC, NULL, &XX,  cc, &YY, ss);
    bsiDPoint4d_add2ScaledDPoint4d (&SS, NULL, &XX, -ss, &YY, cc);

    CdotP = bsiDPoint4d_dotProduct (&CC, pP);
    SdotP = bsiDPoint4d_dotProduct (&SS, pP);
    WdotP = bsiDPoint4d_dotProduct (&WW, pP);

    bsiDPoint4d_add2ScaledDPoint4d (pA, NULL, &CC, WdotP, &WW, -CdotP);
    bsiDPoint4d_add2ScaledDPoint4d (pH, NULL, &SS, WdotP, &WW, -SdotP);
    bsiDPoint4d_scale (pK, &WW, WdotP);
    if (s_debug > 5)
        {
        printf (" Planes at angle %lf from point %lf %lf %lf %lf:\n",
                            theta,
                            pP->x, pP->y, pP->z, pP->w
                            );
        printf ("   A: %lf %lf %lf %lf\n", pA->x, pA->y, pA->z, pA->w);
        printf ("   H: %lf %lf %lf %lf\n", pH->x, pH->y, pH->z, pH->w);
        printf ("   K: %lf %lf %lf %lf\n", pK->x, pK->y, pK->z, pK->w);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlGPA_SLC_getPlaneCoffs
(
SLCContext *pContext,
        DPoint4d    *pA,
        DPoint4d    *pH,
        DPoint4d    *pK,
        int         testPointIndex
)
    {
    int gpIndex = STATUS_BLOCK_SIZE * testPointIndex;
    int gpCount = jmdlGraphicsPointArray_getCount (pContext->pAHKGPA);
    GraphicsPoint *pBuffer = jmdlGraphicsPointArray_getPtr (pContext->pAHKGPA, gpIndex);
    if (pBuffer && gpIndex + 2 < gpCount)
        {
        *pA = pBuffer[OFFSET_A].point;
        *pH = pBuffer[OFFSET_H].point;
        *pK = pBuffer[OFFSET_K].point;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_SLC_incrementCounts
(
SLCContext *pContext,
        int     numLeft,
        int     numRight,
        int     numOnBoundary,
        int     numDegenerate,
        int     testPointIndex,
        double  aa,
const DPoint4d  *pCrossingPoint
)
    {
    int gpIndex = STATUS_BLOCK_SIZE * testPointIndex;
    int gpCount = jmdlGraphicsPointArray_getCount (pContext->pAHKGPA);
    GraphicsPoint *pBuffer = jmdlGraphicsPointArray_getPtr (pContext->pAHKGPA, gpIndex);
    if (s_debug >= 10)
        {
        printf (" Point %d count changes: (left %d)  (right %d) (on %d) (error %d)\n",
                        testPointIndex, numLeft, numRight, numOnBoundary, numDegenerate);
        }

    if (pBuffer && gpIndex + 2 < gpCount)
        {
        pBuffer[OFFSET_ON].userData     += numOnBoundary;
        pBuffer[OFFSET_DEGENERATE_COUNT].userData += numDegenerate;
        pBuffer[OFFSET_L].userData += numLeft;
        pBuffer[OFFSET_R].userData += numRight;

        if (numOnBoundary + numDegenerate + numLeft + numRight == 1)
            {
            if (numLeft == 1)
                {
                if (pBuffer[OFFSET_L].userData == 1 || aa > pBuffer[OFFSET_L].a)
                    {
                    pBuffer[OFFSET_L].a     = aa;
                    pBuffer[OFFSET_L].point = *pCrossingPoint;
                    }
                }
            else if (numRight == 1)
                {
                if (pBuffer[OFFSET_R].userData == 1 || aa < pBuffer[OFFSET_R].a)
                    {
                    pBuffer[OFFSET_R].a     = aa;
                    pBuffer[OFFSET_R].point = *pCrossingPoint;
                    }
                }
            }
        else if (numOnBoundary > 0)
            {
            pBuffer[OFFSET_ON].point = *pCrossingPoint;
            }
        }
    }

/*------------------------------------------------------------------------------
* Compute point where a segment crosses a plane, given segment ends and
* altitudes at ends.
* @param pCrossing <= interpolated point.
* @param pPoint0 => start point.
* @param a0 => altitude at start.
* @param pPoint1 => end point.
* @param a1 => altitude at end.
* @bsimethod                                                    EarlinLutz      08/98
*-------------------------------------------------------------------------------*/
static void    jmdlGPA_SLC_interpolateCrossing
(
DPoint4d    *pCrossing,
DPoint4d    *pPoint0,
double      a0,
DPoint4d    *pPoint1,
double      a1
)
    {
    double da = a1 - a0;
    double w0, w1;

    bsiTrig_safeDivide (&w1, -a0, da, 0.0);
    w0 = 1.0 - w1;
    bsiDPoint4d_add2ScaledDPoint4d (pCrossing, NULL, pPoint0, w0, pPoint1, w1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlGPA_SLC_getCounts
(
SLCContext *pContext,
        int     *pRawPointIndex,
        int     *pNumLeft,
        int     *pNumRight,
        int     *pNumOn,
        int     *pNumDegenerate,
        DPoint4d *pInterpolatedPoint,
        bool    *pInterpolatedPointComputed,
        int     testPointIndex
)
    {
    int gpIndex = STATUS_BLOCK_SIZE * testPointIndex;
    int gpCount = jmdlGraphicsPointArray_getCount (pContext->pAHKGPA);
    GraphicsPoint *pBuffer = jmdlGraphicsPointArray_getPtr (pContext->pAHKGPA, gpIndex);
    if (pBuffer && gpIndex + STATUS_BLOCK_SIZE - 1 < gpCount)
        {
        *pRawPointIndex = pBuffer[OFFSET_INDEX].userData;
        *pNumLeft       = pBuffer[OFFSET_L].userData;
        *pNumRight      = pBuffer[OFFSET_R].userData;
        *pNumDegenerate = pBuffer[OFFSET_DEGENERATE_COUNT].userData;
        *pNumOn         = pBuffer[OFFSET_ON].userData;
        *pInterpolatedPointComputed = false;

        if (pInterpolatedPoint
            && *pNumLeft > 0
            && *pNumRight > 0)
            {
            jmdlGPA_SLC_interpolateCrossing
                    (
                    pInterpolatedPoint,
                    &pBuffer[OFFSET_L].point, pBuffer[OFFSET_L].a,
                    &pBuffer[OFFSET_R].point, pBuffer[OFFSET_R].a
                    );
            *pInterpolatedPointComputed = true;
            }

        if (s_debug >= 5)
            {
            printf (" Test Point %d Raw Point %d (counts %d %d %d %d)\n",
                        testPointIndex,
                        *pRawPointIndex,
                        *pNumLeft,
                        *pNumRight,
                        *pNumOn,
                        *pNumDegenerate
                        );
            }
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* Primary initialization of context -- memory allocation, to be followed by multiple
* sweeps with scans at various angles.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_SLC_initContext
(
SLCContext *pContext,
      GraphicsPointArray    *pBoundaryGPA,
      GraphicsPointArray    *pTestPointGPA,
      DMatrix4d                 *pMatrix,
      bool                      bCollectScanData
)
    {
    DRange3d range;
    static double s_defaultRelTol = 1.0e-12;
    double geomSize;

    memset (pContext, 0, sizeof (*pContext));
    bsiDRange3d_init (&range);
    jmdlGraphicsPointArray_extendDRange3d (pBoundaryGPA, &range);

    geomSize = bsiDRange3d_getLargestCoordinate (&range);

    pContext->aTolerance = s_defaultRelTol * geomSize;
    pContext->hTolerance = s_defaultRelTol * geomSize;

    pContext->pAHKGPA = jmdlGraphicsPointArray_grab ();
    if (bCollectScanData)
        pContext->pScanDataGPA  = jmdlGraphicsPointArray_grab ();

    jmdlGraphicsPointArray_clearAllInOutBits (pTestPointGPA);
    pContext->pBoundaryGPA  = pBoundaryGPA;
    pContext->pTestPointGPA = pTestPointGPA;
    pContext->matrix        = *pMatrix;
    }

/*---------------------------------------------------------------------------------**//**
* Primary initialization of context -- memory allocation, to be followed by multiple
* sweeps with scans at various angles.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_SLC_decomissionContext
(
SLCContext *pContext
)
    {
    jmdlGraphicsPointArray_drop (pContext->pAHKGPA);
    jmdlGraphicsPointArray_drop (pContext->pScanDataGPA);
    memset (pContext, 0, sizeof (*pContext));
    }

/*---------------------------------------------------------------------------------**//**
* For each unclassified point in the test point set, create the reference plane data
* for scan classification.
* @return number of unclassified points for which planes were prepared.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int     jmdlGPA_SLC_prepareScanPlanes
(
SLCContext *pContext,
double      theta
)
    {
    int i;
    DPoint4d AA, HH, KK;
    GraphicsPoint gp;
    int numPoint = 0;

    jmdlGraphicsPointArray_empty (pContext->pAHKGPA);
    pContext->theta = theta;

    for (i = 0;jmdlGraphicsPointArray_getGraphicsPoint
                        (
                        pContext->pTestPointGPA,
                        &gp,
                        i
                        );
                        i++)
        {
        if (!bsiGraphicsPoint_getInOut (&gp, NULL, NULL))
            {
            DPoint4d point = gp.point;
            jmdlGPA_SLC_planeCoefficients (&AA, &HH, &KK, &pContext->matrix, &gp.point, theta);
            jmdlGraphicsPointArray_addComplete (pContext->pAHKGPA,
                                    AA.x, AA.y, AA.z, AA.w, 0.0, 0, i);
            jmdlGraphicsPointArray_addComplete (pContext->pAHKGPA,
                                    HH.x, HH.y, HH.z, HH.w, 0.0, 0, 0);
            jmdlGraphicsPointArray_addComplete (pContext->pAHKGPA,
                                    KK.x, KK.y, KK.z, KK.w, 0.0, 0, 0);
            /* Left, right, and ON points, with nominal coordinates */
            jmdlGraphicsPointArray_addComplete (pContext->pAHKGPA,
                                    point.x, point.y, point.z, point.w, 0.0, 0, 0);
            jmdlGraphicsPointArray_addComplete (pContext->pAHKGPA,
                                    point.x, point.y, point.z, point.w, 0.0, 0, 0);
            jmdlGraphicsPointArray_addComplete (pContext->pAHKGPA,
                                    point.x, point.y, point.z, point.w, 0.0, 0, 0);
            numPoint++;
            }
        }
    return numPoint;
    }

static bool    jmdlGPA_SLC_transverseCoordinate
(
double *pa,
const DPoint4d *pPoint,
const DPoint4d *pACoff,
const DPoint4d *pKCoff
)
    {
    double xx = bsiDPoint4d_dotProduct (pPoint, pACoff);
    double ww = bsiDPoint4d_dotProduct (pPoint, pKCoff);
    return bsiTrig_safeDivide (pa, xx, ww, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* Record degeneracy for the given index.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    recordDegeneracy
(
SLCContext *pContext,
int testIndex
)
    {
    jmdlGPA_SLC_incrementCounts (pContext, 0, 0, 0, 1, testIndex, 0.0, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* Convert point coordinates to transverse coordinate.
* Increment left, right, or ON count as indicated by transverse coordinate.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    testAndRecordTransverseCoordinates
(
SLCContext *pContext,
int testIndex,
DPoint4d    *pPoint,
int         numPoint,
DPoint4d    *pACoff,
DPoint4d    *pKCoff
)
    {
    double aa;
    int i;
    double aTol = pContext->aTolerance;
    for (i = 0; i < numPoint; i++)
        {
        if  (  jmdlGPA_SLC_transverseCoordinate (&aa, &pPoint[i], pACoff, pKCoff))
            {
            if (aa > aTol)
                {
                /* Crossing to right */
                jmdlGPA_SLC_incrementCounts (pContext, 0, 1, 0, 0, testIndex, aa, &pPoint[i]);
                }
            else if (aa < -aTol)
                {
                /* Crossing to left */
                jmdlGPA_SLC_incrementCounts (pContext, 1, 0, 0, 0, testIndex, aa, &pPoint[i]);
                }
            else
                {
                /* crossing AT point */
                jmdlGPA_SLC_incrementCounts (pContext, 0, 0, 1, 0, testIndex, aa, &pPoint[i]);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Find scanline crossings for a single boundary segment.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int     cb_SLC_processDSegment4d
(
SLCContext *pContext,
GraphicsPointArrayCP pGPA,
int index,
DSegment4d *pSegment
)
    {
    double h0, h1, absh0, absh1;
    DPoint4d coffH, coffA, coffK;
    int testIndex;
    int numLeft, numRight, numOn;
    double param;
    DPoint4d intersectionPoint;
    double a0, a1;
    double aTol = pContext->aTolerance;
    double hTol = pContext->hTolerance;
    int incrementOnEdge, incrementDegenerate;

    for (testIndex = 0;
            jmdlGPA_SLC_getPlaneCoffs (pContext, &coffH, &coffA, &coffK, testIndex);
            testIndex++
            )
        {
        h0 = bsiDPoint4d_dotProduct (&coffH, &pSegment->point[0]);
        h1 = bsiDPoint4d_dotProduct (&coffH, &pSegment->point[1]);
        absh0 = fabs (h0);
        absh1 = fabs (h1);
        numLeft = numRight = numOn = 0;
        incrementOnEdge = incrementDegenerate = 0;

        if  (  absh0 < hTol
            || absh1 < hTol
            )
            {
            /* At least one vertex is near the scan line. */
            incrementDegenerate = 1;
            intersectionPoint = pSegment->point[0];
            if  (  jmdlGPA_SLC_transverseCoordinate (&a0, &pSegment->point[0], &coffA, &coffK)
                && jmdlGPA_SLC_transverseCoordinate (&a1, &pSegment->point[1], &coffA, &coffK)
                )
                {
                if (absh0 < hTol && absh1 < hTol)
                    {
                    /* Edge exactly on scan line. Test if it passes through the test point */
                    if (a0 * a1 < 0.0
                        || fabs (a0) < aTol
                        || fabs (a1) < aTol
                        )
                        {
                        incrementOnEdge = 1;
                        incrementDegenerate = 0;
                        jmdlGPA_SLC_interpolateCrossing (&intersectionPoint,
                                                    &pSegment->point[0], a0,
                                                    &pSegment->point[1], a1);
                        }
                    }
                else
                    {
                    bool    on0, on1;
                    on0 = absh0 < aTol && fabs (a0) < aTol;
                    on1 = absh1 < aTol && fabs (a1) < aTol;
                    if (on0 || on1)
                        {
                        incrementOnEdge = 1;
                        incrementDegenerate = 0;
                        if (on0)
                            intersectionPoint = pSegment->point[0];
                        if (on1)
                            intersectionPoint = pSegment->point[1];
                        }
                    }
                }
            jmdlGPA_SLC_incrementCounts (pContext, 0, 0,
                        incrementOnEdge, incrementDegenerate, testIndex, 0.0, &intersectionPoint);
            }
        else if (h1 * h0 < 0.0)
            {
            /* Simple crossing */
            param = -h0 / (h1 - h0);
            bsiDSegment4d_fractionParameterToDPoint4d (pSegment, &intersectionPoint, param);
            testAndRecordTransverseCoordinates (pContext, testIndex, &intersectionPoint, 1, &coffA, &coffK);
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Find scanline crossings for a single boundary conic.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int     cb_SLC_processDConic4d
(
SLCContext *pContext,
GraphicsPointArrayCP pGPA,
int index,
DConic4d    *pConic
)
    {
    DPoint4d coffH, coffA, coffK;
    int testIndex;
    int i, j;
    DPoint4d intersectionPoint[2];
    int numIntersection;
    int numRetained;
    DPoint3d trigPoints[2];

    for (testIndex = 0;
            jmdlGPA_SLC_getPlaneCoffs (pContext, &coffH, &coffA, &coffK, testIndex);
            testIndex++
            )
        {
        numIntersection = bsiDConic4d_intersectPlane
                                (pConic, trigPoints, &coffH);
        numRetained = 0;
        for (i = j = 0; i < numIntersection; i++)
            {
            if (bsiDConic4d_angleInSweep (pConic, trigPoints[i].z))
                {
                trigPoints[numRetained] = trigPoints[i];
                bsiDConic4d_trigParameterToDPoint4d
                                (
                                pConic,
                                &intersectionPoint[numRetained],
                                trigPoints[i].x,
                                trigPoints[i].y
                                );
                numRetained++;
                }
            }
        testAndRecordTransverseCoordinates (pContext, testIndex, intersectionPoint, numRetained, &coffA, &coffK);

        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Record scanline crossings for a single bezier.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int     cb_SLC_processBezierDPoint4dTagged
(
SLCContext *pContext,
TaggedBezierDPoint4d &bezier
)
    {
    DPoint4d coffH, coffA, coffK;
    int testIndex;
    int numIntersection;
    bool    allOn;
    DPoint4d *pIntersectionPoint = (DPoint4d*)_alloca (bezier.m_order * sizeof (DPoint4d));

    for (testIndex = 0;
            jmdlGPA_SLC_getPlaneCoffs (pContext, &coffH, &coffA, &coffK, testIndex);
            testIndex++
            )
        {
        if (bsiBezierDPoint4d_intersectPlane
                    (
                    pIntersectionPoint,
                    NULL,
                    NULL,
                    &numIntersection,
                    &allOn,
                    bezier.m_poles,
                    bezier.m_order,
                    &coffH
                    )
            )
            {
            if (allOn)
                recordDegeneracy (pContext, testIndex);
            else
                testAndRecordTransverseCoordinates
                            (
                            pContext,
                            testIndex,
                            pIntersectionPoint,
                            numIntersection,
                            &coffA,
                            &coffK);
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Classify points by scan lines.  May make multiple passes to resolve
* ambiguous points.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_SLC_computeScanPoints
(
SLCContext *pContext
)
    {
    if (pContext->pScanDataGPA)
        jmdlGraphicsPointArray_empty (pContext->pScanDataGPA);
    jmdlGraphicsPointArray_processPrimitives
                    (
                    pContext,
                    pContext->pBoundaryGPA,
                    (GPAFunc_DSegment4d)cb_SLC_processDSegment4d,
                    (GPAFunc_DConic4d)cb_SLC_processDConic4d,
                    (GPAFunc_BezierDPoint4dTagged)cb_SLC_processBezierDPoint4dTagged
                    );
    }

/*---------------------------------------------------------------------------------**//**
* Move classifications back from working arrays to primary test point array.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_SLC_updateOutput
(
SLCContext *pContext
)
    {
#define ODDNUMBER(n) (((n) % 2) == 1)
    int testIndex, rawPointIndex;
    int numLeft = 0, numRight = 0, numOn = 0, numDegenerate = 0;
    GraphicsPoint gp;
    DPoint4d interpolatedPoint;
    bool    interpolatedPointComputed;
    bool    inBit = false, outBit = false;

    for (testIndex = 0;
            jmdlGPA_SLC_getCounts
                    (
                    pContext,
                    &rawPointIndex,
                    &numLeft,
                    &numRight,
                    &numOn,
                    &numDegenerate,
                    &interpolatedPoint,
                    &interpolatedPointComputed,
                    testIndex
                    );
            testIndex++
            )
        {
        if (jmdlGraphicsPointArray_getGraphicsPoint
                    (
                    pContext->pTestPointGPA,
                    &gp,
                    rawPointIndex
                    ))
            {
            /* If there are identified degeneracies OR if there are
                an odd number of recorded crossings, mark the point
                unclassified.
            */
            inBit = outBit = false;
            if (numDegenerate > 0)
                {
                inBit = outBit = false;
                }
            else if (numOn > 0)
                {
                inBit = outBit = true;
                }
            else if (ODDNUMBER (numLeft + numRight))
                {
                inBit = outBit = false;
                }
            else if (ODDNUMBER (numLeft))
                {
                inBit = true;
                }
            else
                {
                outBit = true;
                }
            }

        bsiGraphicsPoint_setInOut (&gp, inBit, outBit);
        if (interpolatedPointComputed)
            {
            gp.point = interpolatedPoint;
            }
        jmdlGraphicsPointArray_setGraphicsPoint (pContext->pTestPointGPA, &gp, rawPointIndex);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Classify points by scan lines.  May make multiple passes to resolve
* ambiguous points.
* @return number of unclassified points.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int jmdlGPA_SLC_go
(
SLCContext *pContext,
int         numPass
)
    {
    int pass;
    double theta;
    static double s_baseAngle = 0.342487432;
    static double s_angleStep = 0.8324654246;
    int numUnclassified = 0;

    if (numPass < 1)
        numPass = 1;
    for (pass = 0; pass < numPass; pass++)
        {
        /* We want to scan at unobvious angles -- obvious things like horizontal
            and vertical invite special cases in the scan intersections, and
            there is no additional cost to computing at unobvious angles.
            At each pass, take an angle step which is a quadratic in the pass count
        */
        theta = s_baseAngle + pass * pass * s_angleStep;
        numUnclassified = jmdlGPA_SLC_prepareScanPlanes (pContext, theta);
        if (s_debug)
            {
            printf (" (Pass %d) (theta %lf) (numUnclassified %d)\n",
                            pass, theta, numUnclassified);
            }
        if (numUnclassified == 0)
            break;
        jmdlGPA_SLC_computeScanPoints (pContext);
        jmdlGPA_SLC_updateOutput (pContext);
        }
    return numUnclassified;
    }


/*---------------------------------------------------------------------------------**//**
*
* Compute an in/out classification for each point of the graphics point array, using
* scanline parity rules in the xy plane after applying the forward part of the given map.
* Return the classifications as the in/out mask bits of each point, and adjust
* each point to the apparent depth of intersection, where depth is determined by
* linear interpolation between boundary points encountered by a scan plane.
*
* The direction of the scan line is entirely arbitrary -- random, unpredictable,
* not for you to choose, chosen for geometric rather than aesthetic reasons.  If the
* geometry is truly planar, the z values of the returned point is not affected by
* the scan direction.  If the geometry is non-planar, since the boundary endpoints
* used for interpolation are unspecified externally, so also the interpolated point
* depth is unpredictable.
*
* @param pPoints <=> array of points to be classified. The points are modified as
*           follows:
*           1) inout bits of the mask are set.
*           2) test points which are "in" are replaced by interpolation between
*                   two boundary points.
* @param pBoundary => boundary data.
* @param pMatrix => transformation to apply.
*
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_testInOutMapped
(
        GraphicsPointArray      *pPoints,
        GraphicsPointArray      *pBoundary,
        DMatrix4d                   *pMatrix
)
    {
    SLCContext context;
    DMatrix4d matrix;
    if (pMatrix)
        matrix = *pMatrix;
    else
        bsiDMatrix4d_initIdentity (&matrix);

    jmdlGPA_SLC_initContext (&context, pBoundary, pPoints, &matrix, false);
    jmdlGPA_SLC_go (&context, 5);
    jmdlGPA_SLC_decomissionContext (&context);
    }

/*---------------------------------------------------------------------------------**//**
* Compute an in/out classification a single point, using only xy coordinates
* @param pBoundary => boundary data.
* @param pOutPoint <= test point with depth adjusted by interpolating
*           between boundary points.  If the boundary is planar this will
*           be the point on the plane of the "face" formed by the geometry.
* @param pPoint => point to test
* @return true if the point is on or inside the region (by parity rules)
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_isDPoint4dXYInOrOnExt
(
GraphicsPointArray      *pBoundary,
      DPoint4d              *pOutPoint,
const DPoint4d              *pPoint
)
    {
    GraphicsPoint   gp;
    int             inBit, outBit;
    DPoint4d        testPoint;
    GraphicsPointArray *pTestPointArray = jmdlGraphicsPointArray_grab ();
    bsiDPoint4d_initWithNormalizedWeight (&testPoint, pPoint);

    jmdlGraphicsPointArray_addDPoint4d (pTestPointArray, &testPoint);
    jmdlGraphicsPointArray_testInOutMapped (pTestPointArray, pBoundary, NULL);
    jmdlGraphicsPointArray_getGraphicsPoint (pTestPointArray, &gp, 0);

    if (pOutPoint)
        *pOutPoint = gp.point;
//bsiDPoint4d_normalize (&gp.point, pOutPoint);

    bsiGraphicsPoint_getInOut (&gp, &inBit, &outBit);
    jmdlGraphicsPointArray_drop (pTestPointArray);
    return inBit ? true : false;
    }


/*---------------------------------------------------------------------------------**//**
* Compute an in/out classification a single point, using only xy coordinates
* @param pBoundary => boundary data.
* @param pPoint => point to test
* @return true if the point is on or inside the region (by parity rules)
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_isDPoint3dXYInOrOn
(
GraphicsPointArray      *pBoundary,
const DPoint3d              *pPoint
)
    {
    DPoint4d tPt;
    bsiDPoint4d_initFromDPoint3dAndWeight (&tPt, pPoint, 1.0);
    return jmdlGraphicsPointArray_isDPoint4dXYInOrOnExt (pBoundary, NULL, &tPt);
    }
END_BENTLEY_GEOMETRY_NAMESPACE