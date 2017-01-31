/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_hatch.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#ifdef HATCH_DEBUG
static int s_noisy = 0;
#endif

/*----------------------------------------------------------------------+
|                                                                       |
|   Local defines                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Local type definitions                                              |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Private Global variables                                            |
|                                                                       |
+----------------------------------------------------------------------*/
/* Internal nominal limit on number of scan lines
   over entire boundary range.
   Due to rounding of scale factors, anything up to twice
   this number goes through.

   This number changes horrible performance to merely bad performance.
   For complex patterns created by multiple, interrelated patterns, applying this
    limit independently creates havoc in the patterns.
   A significantly smaller limit -- say, 1000 or 2000 -- should be applied
    by the application itself and applied consistently.  Because transverse and
    tangential spacings is coupled, this is very tricky!!!!
*/
static int s_maxLines = 20000;
/*----------------------------------------------------------------------+
|                                                                       |
|   Public GEOMDLLIMPEXP Global variables                                             |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   External variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/

/*======================================================================+
|                                                                       |
|   Private Utility Routines                                            |
|                                                                       |
+======================================================================*/


static int jmdlGPA_roundUp
(
double value
)
    {
    return (int)ceil (value);
    }

static int jmdlGPA_roundDown
(
double value
)
    {
    return (int)floor (value);
    }

#ifdef CompileAll
static double jmdlGPA_roundUpDouble
(
double value
)
    {
    return ceil (value);
    }
#endif
static double jmdlGPA_roundDownDouble
(
double value
)
    {
    return floor (value);
    }


/*====================================================================================
* Hatch generation notes:
* 1) Collect all crossings of the hatch plane with boundaries.  Each is recorded
*   as (integer) plane number and (double) distance in arbitrary orthogonal direction.
* 2) Sort by plane number and distance.
* 3) Ignoring vertex-on-plane conditions, use alternating points on each plane
*       as in/out pairs.
* 4) Mark vertex-on-plane points.   After sorting, each interval of the paired points requires
*       explicit in-out testing.
*====================================================================================*/

typedef struct
    {
    GraphicsPointArrayCP pOriginalBoundary;
    GraphicsPointArray          *pBoundary;
    GraphicsPointArray      *pHatchCollector;
    GraphicsPointArrayCP pClipPlanes;
    Transform    transform;
    Transform    inverse;
    /* Plane k in the cross hatch system is officially
            fixedPlane + k * incrementalPlane.
       BUT ... We are willing to have the knowledge that the incremental plane is
            0,0,0,-1
            used explicitly
    */
    DPoint4d        fixedPlane;
    DPoint4d        incrementalPlane;
    bool            zRangeLimits;
    int             minPlane;
    int             maxPlane;
    double          altitudeTolerance;
    } GPA_TransformedHatchContext;



/*---------------------------------------------------------------------------------**//**
* Lexical comparison of two hatch candidates.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int jmdlGPA_compareHatchCandidates
(
const GraphicsPoint   *pPoint0,
const GraphicsPoint   *pPoint1
)
    {
    if (pPoint1->userData > pPoint0->userData)
        return -1;

    if (pPoint1->userData < pPoint0->userData)
        return 1;

    if (pPoint1->a > pPoint0->a)
        return -1;

    if (pPoint1->a < pPoint0->a)
        return 1;

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
*
* Compute the overall period of a dash pattern.
*
* @param pDashLengths => array of dash lengths.  Negative lengths are "off".
*   (Remark: It seems obvious that alternate entries will have alternate signs.
*   If not, we just do as we're told.)
* @param numDashLength => number of dash lengths in pattern.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double jmdlGPA_computeDashPeriod
(
const   double                  *pDashLengths,
        int                     numDashLength
)
    {
    double sum;
    int i;
    for (sum = 0.0, i = 0; i < numDashLength; i++)
        sum += fabs (pDashLengths[i]);
    return sum;
    }
/*---------------------------------------------------------------------------------**//**
*
* Add a single dash to the output.  Parameter values are assumed to be ordered, and
* only the dash portion within a0<a<a1 will be output.
*
* @param pGP0   => line start point.
* @param pGP1   => line end point.
* @param a0     => interpolation knot for start
* @param a1     => interpolation knot for end
* @param b0     => interpolated start parameter
* @param b1     => interpolated end parameter
* @param divDeltaA => 1 over (a1 - a0), to save the divide.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_insertDash
(
        GraphicsPointArray      *pHatchCollector,
        GraphicsPoint           *pGP0,
        GraphicsPoint           *pGP1,
        double                  a0,
        double                  a1,
        double                  b0,
        double                  b1,
        double                  divDeltaA
)
    {
    DPoint4d point0, point1;
    if (b1 > a0)
        {
        if (b1 > a1)
            b1 = a1;
        if (b0 < a0)
            b0 = a0;

        bsiDPoint4d_add2ScaledDPoint4d (&point0, NULL,
                                &pGP0->point, (a1 - b0) * divDeltaA,
                                &pGP1->point, (b0 - a0) * divDeltaA
                                );

        bsiDPoint4d_add2ScaledDPoint4d (&point1, NULL,
                                &pGP0->point, (a1 - b1) * divDeltaA,
                                &pGP1->point, (b1 - a0) * divDeltaA
                                );

        jmdlGraphicsPointArray_addDPoint4d (pHatchCollector, &point0);
        jmdlGraphicsPointArray_addDPoint4d (pHatchCollector, &point1);

        jmdlGraphicsPointArray_markBreak (pHatchCollector);
        }
    }
/*---------------------------------------------------------------------------------**//**
*
* Apply dash patterning to a single line.
*
* @param pGP0 => line start point.
* @param pGP1 => line end point.
* @param pDashLengths => array of dash lengths.  Negative lengths are "off".
*   (Remark: It seems obvious that alternate entries will have alternate signs.
*   If not, we just do as we're told.)
* @param numDashLength => number of dash lengths in pattern.
* @param dashPeriod => total of all dash lengths.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGPA_expandSingleLineDashPattern
(
        GraphicsPointArray      *pHatchCollector,
        GraphicsPoint               *pGP0,
        GraphicsPoint               *pGP1,
const   double                      *pDashLengths,
        int                         numDashLength,
        double                      dashPeriod,
        int                         maxCollectorPoints
)
    {
    if (dashPeriod <= 0.0)
        dashPeriod = jmdlGPA_computeDashPeriod (pDashLengths, numDashLength);
    if (dashPeriod <= 0.0)
        return;

    if (pGP0->a > pGP1->a)
        {
        jmdlGPA_expandSingleLineDashPattern (pHatchCollector,
                                    pGP1, pGP0,
                                    pDashLengths, numDashLength, dashPeriod, maxCollectorPoints);
        }
    else
        {
        double b0 = jmdlGPA_roundDownDouble (pGP0->a / dashPeriod) * dashPeriod;
        double db;
        double a0 = pGP0->a;
        double a1 = pGP1->a;
        double b1;

        double delta = ((a1 - a0) == 0.0)? 1.0 : (a1 - a0);
        double divDeltaA = 1.0 / delta;
        //double divDeltaA = 1.0 / (a1 - a0);
        int i;

        for (i = 0; b0 < a1 && jmdlGraphicsPointArray_getCount (pHatchCollector) < maxCollectorPoints;)
            {
            db = pDashLengths[i];
            b1 = b0 + fabs (db);
            if (db >= 0.0)
                jmdlGPA_insertDash
                            (
                            pHatchCollector,
                            pGP0, pGP1,
                            a0, a1, b0, b1,
                            divDeltaA);
            b0 = b1;
            if (++i >= numDashLength)
                i = 0;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
*
* Copy lines from one array to another, applying dash logic to replace each
*   line by a dash pattern.
*
* @param pSource => array of line segments.  Each point is assumed to be
*   marked with a dash-space coordinate as its "a" value.
* @param pDashLengths => array of dash lengths.  Negative lengths are "off".
*   (Remark: It seems obvious that alternate entries will have alternate signs.
*   If not, we just do as we're told.)
* @param numDashLength => number of dash lengths in pattern.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_expandDashPattern
(
        GraphicsPointArray      *pHatchCollector,
GraphicsPointArrayCP pSource,
const   double                      *pDashLengths,
        int                         numDashLength
)
    {
    int i0, i1, i;
    DPoint4d point0, point1;
    int curveType;
    GraphicsPoint gp0, gp1;
    static int s_maxCollectorPoint = 100000;
    double dashPeriod = jmdlGPA_computeDashPeriod (pDashLengths, numDashLength);

    for (i0 = 0;
         jmdlGraphicsPointArray_parseFragment (pSource, &i1, &point0, &point1, &curveType, i0)
         && jmdlGraphicsPointArray_getCount(pHatchCollector) < s_maxCollectorPoint;
         i0 = i1 + 1)
        {
        if (curveType == 0)
            {
            jmdlGraphicsPointArray_getGraphicsPoint (pSource, &gp0, i0);
            for (i = i0 + 1;
                i <= i1 && jmdlGraphicsPointArray_getCount(pHatchCollector) < s_maxCollectorPoint;
                i++, gp0 = gp1)
                {
                jmdlGraphicsPointArray_getGraphicsPoint (pSource, &gp1, i);
                jmdlGPA_expandSingleLineDashPattern
                                (
                                pHatchCollector,
                                &gp0,
                                &gp1,
                                pDashLengths,
                                numDashLength,
                                dashPeriod,
                                s_maxCollectorPoint
                                );
                }
            }
        else
            {
            jmdlGraphicsPointArray_appendFragment (pHatchCollector, pSource, i0, i1, 0);
            jmdlGraphicsPointArray_markBreak (pHatchCollector);
            }
        }
    }




/*---------------------------------------------------------------------------------**//**
* Split a double into integer and fraction parts, specifically for use in identifying
*   parallel cutting planes, hence possibly requiring referene to the context for
*   spacing information.
* @param pIndex     <= integer part.  Always algebraically less than or equal to height.
* @param pFraction  <= fractional part.  At least zero, less than one.
* @param height     => floating point height.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_splitHeight
(
GPA_TransformedHatchContext     *pContext,
int                             *pIndex,
double                          *pFraction,
double                          height
)
    {
    int index;
    if (height >= 0)
        {
        index = (int)height;
        }
    else
        {
        index = -1 - (int) (-height);
        }

    *pIndex = index;
    *pFraction = height - index;
    }

/*---------------------------------------------------------------------------------**//**
* Find the extremal, integerized values of k such that f + k * g = 0, where f and g are
*   bezier polynomials.
* @return true if indices were calculated.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlGPA_crossingExtrema
(
int         *pk0,
int         *pk1,
double      *pF,
double      *pG,
int         numPole
)
    {
    int k0, k1, i;
    int m0, m1;
    double a;

    if (numPole <= 0)
        {
        k0 = 0;
        k1 = -1;
        }
    else
        {
        k0 = k1 = 0;    // Will be overwritten on first pass.
        for (i = 0; i < numPole; i++)
            {
            if (bsiTrig_safeDivide (&a, -pF[i], pG[i], 0.0))
                {
                m0 = jmdlGPA_roundDown (a);
                m1 = jmdlGPA_roundUp   (a);
                if (i == 0)
                    {
                    k0 = m0;
                    k1 = m1;
                    }
                else
                    {
                    if (m0 < k0)
                        k0 = m0;
                    if (m1 > k1)
                        k1 = m1;
                    }
                }
            }
        }
    if (pk0)
        *pk0 = k0;
    if (pk1)
        *pk1 = k1;
    return k1 >= k0;
    }



/*---------------------------------------------------------------------------------**//**
* Apply the context's inverse transform to points.
* @param pOut <= transformed points
* @param pIn  => original points
* @param numPoint => number of points.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_applyInverse
(
GPA_TransformedHatchContext     *pContext,
        DPoint4d                *pOut,
const   DPoint4d                *pIn,
int                             numPoint
)
    {
    bsiTransform_multiplyDPoint4dArray (&pContext->inverse, pOut, pIn, numPoint);
    }

/*---------------------------------------------------------------------------------**//**
* Get the height of the hatch plane which would pass through the point.
* @param point => local coordinates of point.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static double  jmdlGPA_TCH_getPointHeight
(
GPA_TransformedHatchContext     *pContext,
const DPoint4d                  *pPoint
)
    {
    return pPoint->z / pPoint->w;
    }

/*---------------------------------------------------------------------------------**//**
* Get the height of the hatch plane which would pass through the point.
* @param point => local coordinates of point.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_planeIndexToPlane
(
GPA_TransformedHatchContext     *pContext,
        DPoint4d                *pPlane,
        int                     index
)
    {
    *pPlane = pContext->fixedPlane;
    pPlane->w -= (double)index;
    }

/*---------------------------------------------------------------------------------**//**
* Get the height of the hatch plane which would pass through the point.
* @param point => local coordinates of point.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static double  jmdlGPA_TCH_pointToSortCoordinate
(
GPA_TransformedHatchContext     *pContext,
        DPoint4d                *pPoint
)
    {
    double localX =
              pContext->inverse.form3d[0][0] * pPoint->x
            + pContext->inverse.form3d[0][1] * pPoint->y
            + pContext->inverse.form3d[0][2] * pPoint->z
            + pContext->inverse.form3d[0][3] * pPoint->w;
    if (pPoint->w == 1.0)
        return localX;

    return localX / pPoint->w;
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void        jmdlGPA_TCH_cutLine
(
GPA_TransformedHatchContext *pContext,
const   DPoint4d            *pPlane,
const   DPoint4d            *pLineStart,
const   DPoint4d            *pLineEnd,
const   DPoint4d            *pLocalStart,
const   DPoint4d            *pLocalEnd,
        int                 index
)
    {
    double h0, h1, ah;
    double f0, f1;
    double a0, a1;
    DPoint4d hPoint, hLocalPoint;
    double hTol = pContext->altitudeTolerance;

    GraphicsPoint gPoint;
    h0 = bsiDPoint4d_dotProduct (pPlane, pLineStart);
    h1 = bsiDPoint4d_dotProduct (pPlane, pLineEnd);

    a0 = fabs (h0);
    a1 = fabs (h1);

    if (a0 < hTol)
        a0 = 0.0;
    if (a1 < hTol)
        a1 = 0.0;

    /* Ignore dead horizontal */
    if (a0 <= hTol && a1 <= hTol)
        return;

    /* Ignore local minimum */
    if (a0 <= hTol && h1 > hTol)
        return;

    if (a1 <= hTol && h0 > hTol)
        return;

    /* Ignore segment completely to one side of plane */
    if (a0 * a1 != 0.0 && h0 * h1 > 0.0)
        return;

    ah = 1.0 / (h1 - h0);
    f0 = h1 * ah;
    f1 = - h0 * ah;
    bsiDPoint4d_add2ScaledDPoint4d (&hPoint, NULL, pLineStart, f0, pLineEnd, f1);
    bsiDPoint4d_add2ScaledDPoint4d (&hLocalPoint, NULL, pLocalStart, f0, pLocalEnd, f1);

    gPoint.point    = hPoint;
    gPoint.a        = hLocalPoint.x / hLocalPoint.w;
    gPoint.userData = index;
    gPoint.mask     = 0;
#ifdef HATCH_DEBUG
    if (s_noisy)
        printf ("Line cut (%d,%lf)\n", gPoint.userData, gPoint.a);
#endif
    jmdlGraphicsPointArray_addGraphicsPoint (pContext->pHatchCollector, &gPoint);
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_applyPlaneLimits
(
GPA_TransformedHatchContext     *pContext,
int                             *pPlane0,
int                             *pPlane1
)
    {
    int plane0, plane1;
    if (pContext->zRangeLimits)
        {
        if (*pPlane0 < pContext->minPlane)
            *pPlane0 = pContext->minPlane;
        if (*pPlane1 > pContext->maxPlane)
            *pPlane1 = pContext->maxPlane;
        }

    /* Watch for integer overflow case!!
        If plane1 is at maxint, "next" integer wraps to negative.
    */
    plane0 = *pPlane0;
    plane1 = *pPlane1;
    plane1++;
    if (plane1 < *pPlane1)
        {
        *pPlane1 -= 1;
        }
    }
/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_collectPolylineCrossings
(
GPA_TransformedHatchContext     *pContext,
int                             i0,
int                             i1
)
    {
    GraphicsPoint gp0, gp1;
    DPoint4d    p0, p1;
    int     index0, index1;
    int     plane0, plane1, plane;
    DPoint4d    currPlane;
    double  h0 = 0, h1;
    double f0, f1;
    int i;
    double nearOne = 1.0 - 1.0e-8;
    for (i = i0; i <= i1; i++)
        {
        jmdlGraphicsPointArray_getGraphicsPoint (pContext->pBoundary, &gp1, i);
        jmdlGPA_TCH_applyInverse (pContext, &p1, &gp1.point, 1);

        h1 = jmdlGPA_TCH_getPointHeight (pContext, &p1);
        if (i > i0)
            {
            if (h0 < h1)
                {
                jmdlGPA_TCH_splitHeight (pContext, &index0, &f0, h0);
                jmdlGPA_TCH_splitHeight (pContext, &index1, &f1, h1);
                }
            else
                {
                jmdlGPA_TCH_splitHeight (pContext, &index0, &f0, h1);
                jmdlGPA_TCH_splitHeight (pContext, &index1, &f1, h0);
                }

            plane0 = index0;
            if (f0 > 0.0)       /* Tolerance ?? */
                plane0++;
            plane1 = index1;    /* Tolerance ?? */
            if (f1 > nearOne)
                plane1++;

            jmdlGPA_TCH_applyPlaneLimits (pContext, &plane0, &plane1);
#ifdef HATCH_DEBUG
            if (s_noisy)
                printf ("(Line (%12.2lf,%12.2lf) (%12.2lf,%12.2lf) (delta %12.2lf,%12.2lf) planes %d %d)\n",
                            gp0.point.x, gp0.point.y, gp1.point.x, gp1.point.y,
                            gp1.point.x - gp0.point.x,
                            gp1.point.y - gp0.point.y,
                            plane0, plane1);
#endif
            for (plane = plane0; plane <= plane1; plane++)
                {
                jmdlGPA_TCH_planeIndexToPlane (pContext, &currPlane, plane);
                jmdlGPA_TCH_cutLine (pContext, &currPlane, &gp0.point, &gp1.point,
                                                        &p0, &p1, plane);
                }
            }
        gp0     = gp1;
        p0      = p1;
        h0      = h1;
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    closeToAngle
(
double theta,
double theta0,
bool    bExpandTolerance,
double expansionFactor
)
    {
    if (bExpandTolerance)
        {
        // This is a +-PI normalization...
        double alpha = bsiTrig_getNormalizedAngle (theta - theta0);
        return fabs (alpha) < expansionFactor * bsiTrig_smallAngle ();
        }
    else
        return bsiTrig_equalAngles (theta, theta0); // Start crossing -- need to find negative points nearby
    }

/*---------------------------------------------------------------------------------**//**
* Generate intersections of a plane with an ellipse
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void        jmdlGPA_TCH_cutDEllipse4d
(
        GPA_TransformedHatchContext    *pContext,
        DEllipse4d          *pEllipse,
        double              theta0,
        double              sweep,
        DPoint4d            *pPlane,
        int                 index
)
    {
    DPoint3d trigPoint[2];
    DPoint4d arcPoints[2];
    bool     endAngleIsNearPlane [2];
    double endAngle[2];
    GraphicsPoint gPoint;
    double hTol = pContext->altitudeTolerance;
    int i;
    double h;
    int numUnbounded;
    bool    bAccept;
    double theta;
    double hCenter;
    static double s_expansionFactor = 1.0e6;
    hCenter = bsiDPoint4d_dotProduct (pPlane, &pEllipse->center);

    numUnbounded = bsiDEllipse4d_intersectPlane (
                trigPoint,
                &pEllipse->center,
                &pEllipse->vector0,
                &pEllipse->vector90,
                pPlane
                );

    endAngle[0] = theta0;
    endAngle[1] = theta0 + sweep;
    for (i = 0; i < 2; i++)
        {
        bsiDEllipse4d_evaluateDPoint4d (pEllipse, &arcPoints[i], endAngle[i]);
        h = bsiDPoint4d_dotProduct (&arcPoints[i], pPlane);
        endAngleIsNearPlane[i] = false;
        if (fabs (h) < hTol)
            endAngleIsNearPlane[i] = true;
        }

    for (i = 0; i < numUnbounded; i++)
        {
        double dHdTheta, cc, ss, hCenter;
        DPoint4d derivVector;
        bAccept = false;
#ifdef libraryOK
        theta = trigPoint[i].z;
        cc = cos (theta);
        ss = sin (theta);
#else
        // library code has x,y flipped in atan call. Recompute it ...
        cc = trigPoint[i].x;
        ss = trigPoint[i].y;
        theta = bsiTrig_atan2 (ss, cc);
        trigPoint[i].z = theta;
#endif
        bsiDPoint4d_add2ScaledDPoint4d (&derivVector, NULL, &pEllipse->vector0, -ss, &pEllipse->vector90, cc);
        dHdTheta = sweep * bsiDPoint4d_dotProduct (&derivVector, pPlane);
        hCenter = bsiDPoint4d_dotProduct (&pEllipse->center, pPlane);

        if (closeToAngle (theta, endAngle[0], endAngleIsNearPlane[0], s_expansionFactor))
            {
            if (dHdTheta < -hTol)
                bAccept = true;
            else if (   dHdTheta < hTol
                    &&  hCenter < 0.0)
                {
                bAccept = true;
                }
            }
        else if (closeToAngle (theta, endAngle[1], endAngleIsNearPlane[1], s_expansionFactor)) // End crossing -- need to find positivve points nearby
            {
            if (dHdTheta > hTol)
                bAccept = true;
            else if (   dHdTheta > -hTol
                    && hCenter < 0.0)
                {
                bAccept = true;
                }
            }
        else if (bsiTrig_angleInSweep (trigPoint[i].z, theta0, sweep))
            {
            // Only accept simple crossings.
            if (fabs (dHdTheta) > hTol)
                bAccept = true;
            }

        //printf ("%s (%d,%d) q=%10.4lf (q0,dq)=(%10.4lf,%10.4lf)\n", bAccept ? "+  " : "  -", i, numUnbounded, theta, theta0, sweep);
        if (bAccept)
            {
            bsiDEllipse4d_evaluateDPoint4d (pEllipse, &gPoint.point, trigPoint[i].z);
            gPoint.a     = jmdlGPA_TCH_pointToSortCoordinate (pContext, &gPoint.point);
            gPoint.userData = index;
            gPoint.mask     = 0;
            jmdlGraphicsPointArray_addGraphicsPoint (pContext->pHatchCollector, &gPoint);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void            jmdlGPA_TCH_getHomogeneousPlaneTerms
(
const   GPA_TransformedHatchContext    *pContext,
        DPoint4d                *pH0,
        DPoint4d                *pH1
)
    {
    *pH0 = pContext->fixedPlane;
    *pH1 = pContext->incrementalPlane;
    }

/*---------------------------------------------------------------------------------**//**
*
* @return true if indices were calculated.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlGPA_TCH_ellipseExtrema
(
GPA_TransformedHatchContext    *pContext,
int         *pPlane0,
int         *pPlane1,
const   DEllipse4d  *pEllipse,
const   DPoint4d    *pStartPoint,
const   DPoint4d    *pEndPoint,
        double      theta0,
        double      sweep
)
    {
#define MAX_SORT_DIST 4
#define MAX_CLIPPED_SORT_DIST 6
    double alpha0, alpha1, beta0, beta1, gamma0, gamma1;
    double alpha, beta, gamma;
    double sortDist[4];
    double clippedSortDist[6];
    int numSortDist, numClippedSort;
    double theta;
    DPoint4d H0, H1;
    double a;
    int i;
    bool    result = false;

    jmdlGPA_TCH_getHomogeneousPlaneTerms (pContext, &H0, &H1);

    alpha0 = bsiDPoint4d_dotProduct (&H0, &pEllipse->center);
    alpha1 = bsiDPoint4d_dotProduct (&H1, &pEllipse->center);

    beta0 = bsiDPoint4d_dotProduct (&H0, &pEllipse->vector0);
    beta1 = bsiDPoint4d_dotProduct (&H1, &pEllipse->vector0);

    gamma0 = bsiDPoint4d_dotProduct (&H0, &pEllipse->vector90);
    gamma1 = bsiDPoint4d_dotProduct (&H1, &pEllipse->vector90);

    alpha = alpha0 * alpha0 - beta0 * beta0 - gamma0 * gamma0;
    beta  = 2.0 * (alpha0 * alpha1 - beta0 * beta1 - gamma0 * gamma1);
    gamma = alpha1 * alpha1 - beta1 * beta1 - gamma1 * gamma1;

    numSortDist = bsiMath_solveQuadratic (sortDist, gamma, beta, alpha);

    if (numSortDist < 2)
        {
        /* ugh. Badly conditioned data.  Just evaluate a bunch of points
           in the ellipse range and find out where they hit. */
        DPoint4d currPoint;
        int numPoint = 16;
        int numOK = 0;
        double df = 1.0 / (numPoint - 1);
        double hMin = DBL_MAX, hMax = - DBL_MAX;
        for (i = 0; i < numPoint; i++)
            {
            bsiDEllipse4d_evaluateDPoint4d (pEllipse, &currPoint, theta0 + i * df * sweep);
            if (bsiTrig_safeDivide (&a,
                                -bsiDPoint4d_dotProduct (&H0, &currPoint),
                                bsiDPoint4d_dotProduct (&H1, &currPoint),
                                0.0))
                {
                if (numOK == 0)
                    hMin = hMax = a;
                else if (a < hMin)
                    hMin = a;
                else if (a > hMax)
                    hMax = a;
                numOK++;
                }
            }
        if (numOK > 0)
            {
            *pPlane0 = jmdlGPA_roundDown (hMin);
            *pPlane1 = jmdlGPA_roundUp (hMax);
            return true;
            }
        }

    /* Needs work: hyperbolic case */

    if (bsiTrig_isAngleFullCircle (sweep))
        {
        /* Just accept the tangent distances as limits */
        if (numSortDist == 2)
            {
            DoubleOps::Sort (sortDist, numSortDist, true);
            *pPlane0 = jmdlGPA_roundDown (sortDist[0]);
            *pPlane1 = jmdlGPA_roundUp (sortDist[1]);
            result = true;
            }
        }
    else
        {
        numClippedSort = 0;
        for (i = 0; i < numSortDist; i++)
            {
            a = sortDist[i];
            alpha = alpha0 + a * alpha1;
            beta  = beta0  + a * beta1;
            gamma = gamma0 + a * gamma1;
            theta = bsiTrig_atan2 (gamma, beta);
            /* HACK .. a little confused on signs here. Only one of these
                angles matters, but really cheap to test both. */
            if (bsiTrig_angleInSweep (theta, theta0, sweep))
                bsiDoubleArray_append (clippedSortDist, &numClippedSort, MAX_CLIPPED_SORT_DIST, a);
            theta += msGeomConst_pi;
            if (bsiTrig_angleInSweep (theta, theta0, sweep))
                bsiDoubleArray_append (clippedSortDist, &numClippedSort, MAX_CLIPPED_SORT_DIST, a);
            }

        if (bsiTrig_safeDivide (&a,
                            -bsiDPoint4d_dotProduct (&H0, pStartPoint),
                            bsiDPoint4d_dotProduct (&H1, pStartPoint),
                            0.0))
            {
            bsiDoubleArray_append (clippedSortDist, &numClippedSort, MAX_CLIPPED_SORT_DIST, a);
            }

        if (bsiTrig_safeDivide (&a,
                            -bsiDPoint4d_dotProduct (&H0, pEndPoint),
                            bsiDPoint4d_dotProduct (&H1, pEndPoint),
                            0.0))
            {
            bsiDoubleArray_append (clippedSortDist, &numClippedSort, MAX_CLIPPED_SORT_DIST, a);
            }

        if (numClippedSort >= 1)
            {
            DoubleOps::Sort (clippedSortDist, numClippedSort, true);
            *pPlane0 = jmdlGPA_roundDown (clippedSortDist[0]);
            *pPlane1 = jmdlGPA_roundUp (clippedSortDist[numClippedSort - 1]);
            result = true;
            }

        }

    return result;
    }


/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_collectDEllipse4dCrossings
(
GPA_TransformedHatchContext     *pContext,
int                     i0,
int                     i1
)
    {
    DPoint4d H0, H1, H;
    DEllipse4d ellipse;
    double theta0, sweep;
    DPoint4d point0, point1;
    int plane, plane0, plane1;

    if (jmdlGraphicsPointArray_getDEllipse4d (pContext->pBoundary, &i0,
                    &ellipse,
                    &theta0, &sweep, &point0, &point1))
        {
        jmdlGPA_TCH_getHomogeneousPlaneTerms (pContext, &H0, &H1);
        if (jmdlGPA_TCH_ellipseExtrema (pContext, &plane0, &plane1, &ellipse, &point0, &point1, theta0, sweep))
            {
            jmdlGPA_TCH_applyPlaneLimits (pContext, &plane0, &plane1);
#ifdef HATCH_DEBUG
            if (s_noisy)
                printf ("(Ellipse (%12.2lf,%12.2lf) (U %12.2lf,%12.2lf) (V %12.2lf,%12.2lf) planes %d %d)\n",
                            ellipse.center.x, ellipse.center.y,
                            ellipse.vector0.x, ellipse.vector0.y,
                            ellipse.vector90.x, ellipse.vector90.y,
                            plane0, plane1);
            //printf ("\n\n Planes %d to %d\n", plane0, plane1);
#endif
            for (plane = plane0; plane <= plane1; plane++)
                {
                //printf ("              -------------------------------------------- PLANE %d\n", plane);
                bsiDPoint4d_addScaledDPoint4d (&H, &H0, &H1, (double)plane);
                jmdlGPA_TCH_cutDEllipse4d (pContext, &ellipse, theta0, sweep, &H, plane);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Generate intersections of a plane with a bezier, where the plane intersections are
* defined by a univariate bezier which is a linear combination of two others.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void        jmdlGPA_TCH_cutCombinationBezier
(
        GPA_TransformedHatchContext    *pContext,
        DPoint4d            *pPoleArray,
        double              *pHeight0Array,
        double              *pHeight1Array,
        double              a,
        int                 numPole,
        int                 index
)
    {
    int i;
    int numRoot;
    if (numPole < 1)
        return;
    double heightPole[MAX_BEZIER_CURVE_ORDER];
    double rootArray[MAX_BEZIER_CURVE_ORDER];

    DPoint4d    rootPointArray[MAX_BEZIER_CURVE_ORDER];
    GraphicsPoint gPoint;
    DPoint3d      cPoint;
    // Tolerance for identifying endpoints.
    // Endpoints with near-zero altitude according to global
    //   toleranced will be forced to exact zero, and we
    //   expect the root finder to generate an exact 0 or 1
    //   parameter. So the parametric tolerance for endpoint
    //   is tight ...
    static double s_paramRelTol = 1.0e-14;
    static double s_derivativeTol = 1.0e-8;
    double poleTol = pContext->altitudeTolerance;
    bool    bAccept;

    for (i = 0; i < numPole; i++)
        {
        heightPole[i] = pHeight0Array[i] + a * pHeight1Array[i];
        }

    if (fabs(heightPole[0]) < poleTol)
        heightPole[0] = 0.0;

    if (fabs (heightPole[numPole-1]) < poleTol)
        heightPole[numPole - 1] = 0.0;

    bsiBezier_univariateRoots (rootArray, &numRoot, heightPole, numPole);

    if (numRoot > 0)
        {
        bsiBezierDPoint4d_evaluateArray
                (rootPointArray, NULL, pPoleArray, numPole, rootArray, numRoot);
        for (i = 0; i < numRoot; i++)
            {
            double u = rootArray[i];
            double f, df, ddf;
            int k;
            bsiBezier_functionAndDerivativeExt (&f, &df, &ddf, heightPole, numPole, 1, rootArray[i]);
            bAccept = false;
            // We want a simple crossing or a local MAX
            // At start or end, look for a nonzero pole to decide
            if (u < s_paramRelTol)
                {
                for (k = 1; k < numPole; k++)
                    {
                    if (heightPole[k] > s_derivativeTol)
                        break;  // Heading positive ....
                    else if (heightPole[k] < -s_derivativeTol)
                        {
                        bAccept = true;
                        break;
                        }
                    }
                }
            else if (u > 1.0 - s_paramRelTol)
                {
                for (k = numPole - 2; k >= 0; k--)
                    {
                    if (heightPole[k] < -s_derivativeTol)
                        {
                        bAccept = true;
                        break;
                        }
                    }
                }
            else if (fabs (df) > s_derivativeTol)
                {
                bAccept = true; // simple crossing.
                }
            else
                {
                // local min. In usual case we see both sides of the hit and can skip it.
                // Hope it's not an inflection too!!
                // bAccept = ddf < 0.0;
                }

            if (bAccept)
                {
                gPoint.point = rootPointArray[i];
                bsiDPoint4d_cartesianFromHomogeneous (&gPoint.point, &cPoint);
                gPoint.a     = jmdlGPA_TCH_pointToSortCoordinate (pContext, &gPoint.point);
                gPoint.userData = index;
                gPoint.mask     = 0;
#ifdef HATCH_DEBUG
                if (s_noisy)
                    printf ("Bezier cut (%d,%lf)\n", gPoint.userData, gPoint.a);
#endif
                jmdlGraphicsPointArray_addGraphicsPoint (pContext->pHatchCollector, &gPoint);
                }
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_collectBezierCrossings_go
(
GPA_TransformedHatchContext     *pContext,
int                     i0,
int                     i1,
DPoint4dP               poleArray,
int                     numPole
)
    {
    DPoint4d H0, H1;
    double   poleArray0[MAX_BEZIER_CURVE_ORDER];
    double   poleArray1[MAX_BEZIER_CURVE_ORDER];
    int plane, plane0, plane1;
    jmdlGPA_TCH_getHomogeneousPlaneTerms (pContext, &H0, &H1);
    for (int i = 0; i <  numPole; i++)
        {
        poleArray0[i] = bsiDPoint4d_dotProduct (&H0, &poleArray[i]);
        poleArray1[i] = bsiDPoint4d_dotProduct (&H1, &poleArray[i]);
        }
    if (jmdlGPA_crossingExtrema (&plane0, &plane1, poleArray0, poleArray1, numPole))
        {
        jmdlGPA_TCH_applyPlaneLimits (pContext, &plane0, &plane1);
        for (plane = plane0; plane <= plane1; plane++)
            {
            jmdlGPA_TCH_cutCombinationBezier
                    (pContext, poleArray, poleArray0, poleArray1, (double)plane, numPole, plane);
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_collectBezierCrossings
(
GPA_TransformedHatchContext     *pContext,
int                     i0,
int                     i1
)
    {
    DPoint4d poleArray[MAX_BEZIER_CURVE_ORDER];
    int numPole;

    if (jmdlGraphicsPointArray_getBezier (pContext->pBoundary, &i0, poleArray, &numPole, MAX_BEZIER_CURVE_ORDER))
        jmdlGPA_TCH_collectBezierCrossings_go (pContext, i0, i1, poleArray, numPole);
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_collectBsplineCrossings
(
GPA_TransformedHatchContext     *pContext,
int                     i0,
int                     i1
)
    {
    DPoint4d poleArray[MAX_BEZIER_CURVE_ORDER];
    int numPole;
    double knot0, knot1;
    bool isNullInterval;
    for (size_t i = 0; pContext->pBoundary->GetBezierSpanFromBsplineCurve (i0, i,
                                poleArray, numPole, MAX_BEZIER_CURVE_ORDER, isNullInterval, knot0, knot1); ++i)
        {
        jmdlGPA_TCH_collectBezierCrossings_go (pContext, i0, i1, poleArray, numPole);        
        }
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_collectCrossings
(
GPA_TransformedHatchContext     *pContext
)
    {
    int i0, i1;
    DPoint4d point0, point1;
    int curveType;

    for (i0 = 0;
         jmdlGraphicsPointArray_parseFragment (
                        pContext->pBoundary, &i1,
                        &point0, &point1,
                        &curveType, i0);
         i0 = i1 + 1)
        {
        if (curveType == 0)
            {
            jmdlGPA_TCH_collectPolylineCrossings (pContext, i0, i1);
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE_CODED
            {
            jmdlGPA_TCH_collectBezierCrossings (pContext, i0, i1);
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            jmdlGPA_TCH_collectDEllipse4dCrossings (pContext, i0, i1);
            }
        else if (curveType == HPOINT_MASK_CURVETYPE_BSPLINE)
            {
            jmdlGPA_TCH_collectBsplineCrossings (pContext, i0, i1);
            }
        }
    }

typedef enum
    {
    TCH_Parity  = 0,
    TCH_All     = 1,
    TCH_Outer   = 2
    } TCH_SelectMode;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      01/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_addStroke
(
GPA_TransformedHatchContext     *pContext,
GraphicsPointArray          *pDest,
const GraphicsPoint             *pGP0,
const GraphicsPoint             *pGP1,
bool                            normalizeOutput,
double                          daMin
)
    {
    GraphicsPoint gp0, gp1;

    gp0 = *pGP0;
    gp1 = *pGP1;
    gp0.mask = gp1.mask = 0;

    if (fabs (gp1.a - gp0.a) < daMin)
        return;

    if (pContext->pClipPlanes)
        {
        const GraphicsPoint *pClipPlaneBuffer = jmdlGraphicsPointArray_getConstPtr (pContext->pClipPlanes, 0);
        int numClipPlanes = jmdlGraphicsPointArray_getCount (pContext->pClipPlanes);
        int i;
        double sMin, sMax, h0, h1;
        double s;
        sMin = 0.0;
        sMax = 1.0;
        /* Find the parameter limits sMin and sMax of the active segment.
            (0 <= sMin <= sMax <= 1) is the fraction parameter range of active points.
        */
        for (i = 0; i < numClipPlanes; i++)
            {
            h0 = bsiDPoint4d_dotProduct (&gp0.point, &pClipPlaneBuffer[i].point);
            h1 = bsiDPoint4d_dotProduct (&gp1.point, &pClipPlaneBuffer[i].point);
            if (h1 >= h0)
                {
                if (h0 > 0.0)
                    return;
                if (h1 > 0.0)
                    {
                    s = -h0 / (h1 - h0);
                    /* Clip away fractional parameters larger than s */
                    if (s < sMin)
                        return;
                    if (s < sMax)
                        sMax = s;
                    }
                }
            else
                {
                if (h1 > 0.0)
                    return;
                if (h0 > 0.0)
                    {
                    s = -h0 / (h1 - h0);
                    /* Clip away fractional paramters smaller than s */
                    if (s > sMax)
                        return;
                    if (s > sMin)
                        sMin = s;
                    }
                }
            }

        if (sMin > 0.0)
            {
            bsiDPoint4d_interpolate (&gp0.point, &pGP0->point, sMin, &pGP1->point);
            gp0.a = (1.0 - sMin) * pGP0->a  + sMin * pGP1->a;
            }

        if (sMax < 1.0)
            {
            bsiDPoint4d_interpolate (&gp1.point, &pGP0->point, sMax, &pGP1->point);
            gp1.a = (1.0 - sMax) * pGP0->a  + sMax * pGP1->a;
            }

        /* Check one more time for short strokes ... */
        if (fabs (gp1.a - gp0.a) < daMin)
            return;
        }


    if (normalizeOutput)
        {
        if (gp0.point.w != 1.0)
            bsiDPoint4d_initWithNormalizedWeight (&gp0.point, &gp0.point);
        if (gp1.point.w != 1.0)
            bsiDPoint4d_initWithNormalizedWeight (&gp1.point, &gp1.point);
        }

    jmdlGraphicsPointArray_addGraphicsPoint (pDest, &gp0);
    jmdlGraphicsPointArray_addGraphicsPoint (pDest, &gp1);
    jmdlGraphicsPointArray_markBreak (pDest);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_collectOutput
(
GPA_TransformedHatchContext     *pContext,
GraphicsPointArray          *pDest,
int                             selectMode,
bool                            normalizeOutput
)
    {
    GraphicsPoint *pSourceArray = jmdlGraphicsPointArray_getPtr (pContext->pHatchCollector, 0);
    int sourceCount = jmdlGraphicsPointArray_getCount (pContext->pHatchCollector);
    int i0, i1, i, numCut;
    double aMin, aMax;
    static double s_minFraction = 1.0e-10;
    static double s_minDelta    = 1.0e-14;
    double daMin;

    if (pSourceArray && sourceCount > 0)
        {

        aMin = aMax = pSourceArray[0].a;
        for (i0 = 1; i0 < sourceCount; i0++)
            {
            if (pSourceArray[i0].a < aMin)
                aMin = pSourceArray[i0].a;
            if (pSourceArray[i0].a > aMax)
                aMax = pSourceArray[i0].a;
            }
        daMin = s_minFraction * (aMax - aMin);
        if (daMin < s_minDelta)
            daMin = s_minDelta;

        for (i0 = 0; i0 < sourceCount; i0 = i1)
            {
            /* Find index range for points at same altitute */
            for (  i1 = i0 + 1;
                   i1 < sourceCount
                && pSourceArray[i1].userData == pSourceArray[i0].userData;
                   i1++)
                {
                /* Nothing to do -- we're just counting them up */
                }

            numCut = i1 - i0;
            if (numCut <= 1)
                {
                }
            else if (selectMode == TCH_All || numCut <= 3)
                {
                jmdlGPA_TCH_addStroke
                        (
                        pContext,
                        pDest,
                        pSourceArray + i0,
                        pSourceArray + i1 - 1,
                        normalizeOutput,
                        daMin
                        );
                }
            else if (selectMode == TCH_Outer)
                {
                jmdlGPA_TCH_addStroke
                        (
                        pContext,
                        pDest,
                        pSourceArray + i0,
                        pSourceArray + i0 + 1,
                        normalizeOutput,
                        daMin
                        );
                jmdlGPA_TCH_addStroke
                        (
                        pContext,
                        pDest,
                        pSourceArray + i1 - 2,
                        pSourceArray + i1 - 1,
                        normalizeOutput,
                        daMin
                        );
                }
            else
                {
                /* Parity .. left side wins if odd crossing count */
                for (i = i0; i < i1 - 1; i += 2)
                    {
                    jmdlGPA_TCH_addStroke
                        (
                        pContext,
                        pDest,
                        pSourceArray + i,
                        pSourceArray + i + 1,
                        normalizeOutput,
                        daMin
                        );
                    }
                }
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_sort
(
GPA_TransformedHatchContext     *pContext
)
    {
    GraphicsPoint *pSourceArray = jmdlGraphicsPointArray_getPtr (pContext->pHatchCollector, 0);
    int sourceCount = jmdlGraphicsPointArray_getCount (pContext->pHatchCollector);

    if (pSourceArray && sourceCount > 1)
        {
        qsort (pSourceArray, sourceCount, sizeof (GraphicsPoint), (int (*)(const void *, const void *))jmdlGPA_compareHatchCandidates);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Crosshatch elements come in with coordinate system set up to assure that x is a good sort direction.
* Single-plane section cuts do not -- all the cut points might have identical x coordinates.
* For a planar cuttee, the cut will produce colinear points.  A vector between any reasonably well separated
*   pair of cut points will be a good sort direction.  To get well separated, find the point farthest
*   from point 0 -- this will be at least half the cut length.
* REMARK:  Earlier code here assumed the cut points were in the local system, so only xy variation was
*    possible. This is wrong.  The cut points are global.  Must use true vectors to get sortable coordinate.
* @bsimethod                                                    EarlinLutz      11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_chooseSortCoordinate
(
GPA_TransformedHatchContext     *pContext
)
    {
    GraphicsPoint *pSourceArray = jmdlGraphicsPointArray_getPtr (pContext->pHatchCollector, 0);
    int sourceCount = jmdlGraphicsPointArray_getCount (pContext->pHatchCollector);
    DPoint3d xyzBase, xyzCurr, xyzFar;
    DVec3d lineVector;
    double d, dMax;
    int i;
    if (sourceCount <= 1)
        return;

    // Find the point farthest from point 0 ...
    pSourceArray[0].point.GetProjectedXYZ (xyzBase);
    dMax = 0.0;
    for (i = 1; i < sourceCount; i++)
        {
        GraphicsPoint *pCurr = &pSourceArray[i];
        pCurr->point.GetProjectedXYZ (xyzCurr);
        d = xyzBase.DistanceSquared (xyzCurr);
        if (d > dMax)
            {
            dMax = d;
            xyzFar = xyzCurr;
            }
        }

    // Assign sort coordinate along maximal direction ..
    lineVector.NormalizedDifference(xyzFar, xyzBase);
    for (i = 0; i < sourceCount; i++)
        {
        GraphicsPoint *pCurr = &pSourceArray[i];
        pCurr->point.GetProjectedXYZ (xyzCurr);
        pCurr->a = xyzCurr.DotDifference(xyzBase, lineVector);
        }
    }


#ifdef CompileAll
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPA_TCH_setClassification
(
GraphicsPoint   *pGP,
bool            bInBit,
bool            bOutBit
)
    {
    if (pGP)
        {

        }
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @description Return a scale factor to be applied to the z-axis of the hatch transform so that
* at most maxLines scan planes are defined within the combined range of two GPAs.
* @param pTransform => proposed hatch transform.  xy plane is hatch plane.  z direction
*       is advance vector between successive planes.
* @param pGPA0 => first geometry source.  May be NULL. For instance, send boundary geometry here.
* @param pGPA1 => second geometry source.  May be NULL, for instance, send corners of clip planes.
* @param maxLines => max number of lines allowed in specified ranges. If 0 or negative, a default
*               is applied.
* @return scale factor to apply to z vector. Any error condition returns 1.0.  Example error
*       conditions are (a) no geometry to determine range, and (b) singular transform.
*
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlGPA_hatchDensityScaleExt
(
        double                          *pScale,
const   Transform                    *pTransform,
GraphicsPointArrayCP pGPA0,
GraphicsPointArrayCP pGPA1,
        int                             maxLines
)
    {
    DRange3d worldRange, localRange;
    Transform inverse;
    double dNumLines;
    double dMaxLines = (double)maxLines;
    double bigNum = (double)(0x7FffFFff);

    double zScale = 1.0;
    if (pScale)
        *pScale = 1.0;

    if (!inverse.InverseOf (*pTransform))
        return false;

    bsiDRange3d_init (&worldRange);
    if (pGPA0)
        jmdlGraphicsPointArray_extendDRange3d (pGPA0, &worldRange);
    if (pGPA1)
        jmdlGraphicsPointArray_extendDRange3d (pGPA1, &worldRange);

    if (bsiDRange3d_isNull (&worldRange))
        return false;

    bsiTransform_multiplyRange (&inverse, &localRange, &worldRange);

    /* Converted V7 files sometimes ask for 1 UOR spacing.
       Just blow these off
    */
    if (fabs (localRange.high.z) > bigNum)
        return false;
    if (fabs (localRange.low.z) > bigNum)
        return false;

    if (dMaxLines <= 0)
        dMaxLines = (double)s_maxLines;

    dNumLines = localRange.high.z - localRange.low.z;
    if (dNumLines > dMaxLines)
        {
        double exactScale = dNumLines / dMaxLines;
        int integerScale = jmdlGPA_roundUp (exactScale);
        zScale = (double)integerScale;
        }

    if (pScale)
        *pScale = zScale;
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @description Return a scale factor to be applied to the z-axis of the hatch transform so that
* at most maxLines scan planes are defined within the combined range of two GPAs.
* @param pTransform => proposed hatch transform.  xy plane is hatch plane.  z direction
*       is advance vector between successive planes.
* @param pGPA0 => first geometry source.  May be NULL. For instance, send boundary geometry here.
* @param pGPA1 => second geometry source.  May be NULL, for instance, send corners of clip planes.
* @param maxLines => max number of lines allowed in specified ranges. If 0 or negative, a default
*               is applied.
* @return scale factor to apply to z vector. Any error condition returns 1.0.  Example error
*       conditions are (a) no geometry to determine range, and (b) singular transform.
*
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double jmdlGPA_hatchDensityScale
(
const   Transform                      *pTransform,
GraphicsPointArrayCP pGPA0,
GraphicsPointArrayCP pGPA1,
        int                             maxLines
)
    {
    double scale;
    jmdlGPA_hatchDensityScaleExt (&scale, pTransform, pGPA0, pGPA1, maxLines);
    return scale;
    }

/*---------------------------------------------------------------------------------**//**
* Initialize the transform, inverse, and plane fields.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void jmdlGPA_TCH_teardown
(
GPA_TransformedHatchContext     *pContext
)
    {
    jmdlGraphicsPointArray_drop (pContext->pBoundary);
    }


/*---------------------------------------------------------------------------------**//**
* Initialize the transform, inverse, and plane fields.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlGPA_TCH_initTransform
(
        GPA_TransformedHatchContext     *pContext,
GraphicsPointArrayCP pBoundary,
        GraphicsPointArray          *pHatchCollector,
const   Transform                    *pTransform,
GraphicsPointArrayCP pClipRangePoints,
GraphicsPointArrayCP pClipPlanes,
        int                             maxLines
)
    {
    bool    boolstat;
    int i;
    DPoint4d clipPoint;
    DPoint3d worldPoint, localPoint;
    double zMin, zMax;
    static double s_endTolerance = 1.0e-8;
    double s_maxGapRelTol = 1.0e-6;
    double maxGapTol = 1.0;

    DRange3d range;
    bsiDRange3d_init (&range);
    jmdlGraphicsPointArray_extendDRange3d (pBoundary, &range);
    maxGapTol = s_maxGapRelTol * range.low.DistanceXY (range.high);

    pContext->pHatchCollector = pHatchCollector;
    pContext->pOriginalBoundary  = pBoundary;
    pContext->pBoundary = jmdlGraphicsPointArray_grab ();
    jmdlGraphicsPointArray_appendWithArcsAsBezier (pContext->pBoundary, pBoundary);
    jmdlGraphicsPointArray_forceBezierAndLinestringEndsToNeighbors (pContext->pBoundary, maxGapTol);
    pContext->transform = *pTransform;
    pContext->zRangeLimits = false;
    pContext->pClipPlanes = pClipPlanes;
    pContext->altitudeTolerance = s_endTolerance;
    boolstat = pContext->inverse.InverseOf (*pTransform);

    if (boolstat)
        {

        if (maxLines > 0)
            {
            double zScale;
            RotMatrix scaleMatrix1;
            if (!jmdlGPA_hatchDensityScaleExt
                        (&zScale, pTransform, pContext->pBoundary, pClipRangePoints, maxLines))
                return false;
            // Be sure the saved index for each line reflects the scaling.....
            //  in a grouped hole, saved index must correspond to actual plane shared across holes.
            scaleMatrix1.InitFromScaleFactors (1.0, 1.0, zScale);
            pContext->transform.InitProduct (pContext->transform, scaleMatrix1);
            if (!pContext->inverse.InverseOf (pContext->transform))
                return false;
            }

        bsiDPoint4d_setComponents
                        (
                        &pContext->fixedPlane,
                        pContext->inverse.form3d[2][0],
                        pContext->inverse.form3d[2][1],
                        pContext->inverse.form3d[2][2],
                        pContext->inverse.form3d[2][3]
                        );
        bsiDPoint4d_setComponents
                        (
                        &pContext->incrementalPlane,
                        0.0,
                        0.0,
                        0.0,
                        -1.0
                        );
        if (pClipRangePoints)
            {
            zMin = zMax = 0.0;  // Will be overwritten first pass?
            for (i = 0; jmdlGraphicsPointArray_getDPoint4dWithMask (pClipRangePoints, &clipPoint, NULL, i); i++)
                {
                if (clipPoint.GetProjectedXYZ (worldPoint))
                    {
                    bsiTransform_multiplyDPoint3d (&pContext->inverse, &localPoint, &worldPoint);
                    if (!pContext->zRangeLimits || localPoint.z < zMin)
                        zMin = localPoint.z;
                    if (!pContext->zRangeLimits || localPoint.z > zMax)
                        zMax = localPoint.z;
                    pContext->zRangeLimits = true;
                    }
                }
            if (pContext->zRangeLimits)
                {
                pContext->minPlane = jmdlGPA_roundDown (zMin);
                pContext->maxPlane = jmdlGPA_roundUp (zMax);
                }
            }
        }
    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlGPA_TCH_initSinglePlaneIntersection
(
        GPA_TransformedHatchContext     *pContext,
GraphicsPointArrayCP pBoundary,
        GraphicsPointArray          *pHatchCollector,
DPlane3dCP                              pPlane
)
    {
    DVec3d uVec, vVec, wVec;
    bool    boolstat = false;
    double s_maxGapRelTol = 1.0e-6;
    double maxGapTol = 1.0;
    DRange3d range;

    pContext->pHatchCollector = pHatchCollector;
    pContext->pOriginalBoundary  = pBoundary;
    pContext->pBoundary = jmdlGraphicsPointArray_grab ();

    if (0 == jmdlGraphicsPointArray_getCount(pBoundary))
        return false;

    bsiDRange3d_init (&range);
    jmdlGraphicsPointArray_extendDRange3d (pBoundary, &range);

    if (bsiDRange3d_isNull (&range))
        return false;

    maxGapTol = s_maxGapRelTol * range.low.DistanceXY (range.high);

    jmdlGraphicsPointArray_appendWithArcsAsBezier (pContext->pBoundary, pBoundary);
    jmdlGraphicsPointArray_forceBezierAndLinestringEndsToNeighbors (pContext->pBoundary, maxGapTol);
    bsiDVec3d_getNormalizedTriad (&pPlane->normal, &uVec, &vVec, &wVec);
    bsiTransform_initFromOriginAndVectors (&pContext->transform, &pPlane->origin, &uVec, &vVec, &wVec);
    boolstat = pContext->inverse.InverseOf (pContext->transform);
    pContext->pClipPlanes = NULL;

    bsiDPoint4d_setComponents
                    (
                    &pContext->fixedPlane,
                    pContext->inverse.form3d[2][0],
                    pContext->inverse.form3d[2][1],
                    pContext->inverse.form3d[2][2],
                    pContext->inverse.form3d[2][3]
                    );
    bsiDPoint4d_setComponents
                    (
                    &pContext->incrementalPlane,
                    0.0,
                    0.0,
                    0.0,
                    -1.0
                    );

    pContext->zRangeLimits = true;
    pContext->minPlane = 0;
    pContext->maxPlane = 0;
    pContext->altitudeTolerance = 2.0 * maxGapTol;
    return boolstat;
    }


/*---------------------------------------------------------------------------------**//**
*
* Generate crosshatch in the loops of the boundary array.  Add the crosshatch to the
* instance array.
*
* On return the cross hatch is a sequence of alternating start/stop graphics points.
* On each graphics point of the hatch, the user int is the hatch line index and the
* double value is the sort coordinate along the hatch line.
*
* @param pTranfsorm         => transform to hatch plane space.
*                               Hatch is created on every plane parallel to xy and through
*                               integer z.   In/out is determined by parity rules
*                               after x-direction sort.
* @param pClipRangePoints   => any number of points which are vertices of the clip box.
*                                   Hatch planes which fall outside the range of these points
*                                   in hatch space (after transformation) are not considered.
* @param pClipPlanes        => any number of clip planes, expressed as DPoint4d plane coefficients.
*                                   Negative points of each halfspace are IN.  The convex
*                                   clip region is accepted.
* @param selectRule         => selects inside/outside rule.  On each scan line,
*
*                               0 -- simple parity -- alternating in out
*                               1 -- maximal stroke -- only first and last crossings matter
*                               2 -- outer -- if 4 or more crossings, only first and last
*                                       pairs matter.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addTransformedCrossHatchClipped
(
        GraphicsPointArray      *pHatchCollector,
GraphicsPointArrayCP pBoundary,
const   Transform                *pTransform,
GraphicsPointArrayCP pClipRangePoints,
GraphicsPointArrayCP pClipPlanes,
int     selectRule
)
    {
    GPA_TransformedHatchContext context;
    GraphicsPointArray  *pScratchArray = jmdlGraphicsPointArray_grab ();

#ifdef HATCH_DEBUG
    if (s_noisy)
        printf("\n ADD TRANSFORMED CROSSHATCH\n");
#endif
    if (jmdlGPA_TCH_initTransform (&context, pBoundary, pScratchArray, pTransform,
                pClipRangePoints, pClipPlanes, s_maxLines))
        {
        jmdlGPA_TCH_collectCrossings (&context);
        jmdlGPA_TCH_sort (&context);
        jmdlGPA_TCH_collectOutput (&context, pHatchCollector, selectRule, true);
        }
    jmdlGraphicsPointArray_drop (pScratchArray);
    jmdlGPA_TCH_teardown (&context);
    }


/*---------------------------------------------------------------------------------**//**
Generate the intersection (sticks) of a plane with the area bounded by loops in the boundary GPA.
On return the intersection is a sequence of alternating start/stop graphics points.

@param pPlane   IN plane to intersect.
@param selectRule           => selects inside/outside rule.  On each scan line,
@bsimethod                                                      EarlinLutz      03/07
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_intersectDPlane3d
(
        GraphicsPointArray      *pHatchCollector,
GraphicsPointArrayCP pBoundary,
DPlane3dCP                          pPlane
)
    {
    GPA_TransformedHatchContext context;
    GraphicsPointArray  *pScratchArray = jmdlGraphicsPointArray_grab ();

#ifdef HATCH_DEBUG
    if (s_noisy)
        printf("\n ADD TRANSFORMED CROSSHATCH\n");
#endif
    if (jmdlGPA_TCH_initSinglePlaneIntersection (&context, pBoundary, pScratchArray, pPlane))
        {
        jmdlGPA_TCH_collectCrossings (&context);
        jmdlGPA_TCH_chooseSortCoordinate (&context);
        jmdlGPA_TCH_sort (&context);
        jmdlGPA_TCH_collectOutput (&context, pHatchCollector, 0, true);
        }
    jmdlGraphicsPointArray_drop (pScratchArray);
    jmdlGPA_TCH_teardown (&context);
    }


/*---------------------------------------------------------------------------------**//**
@description Inspect the ranges of multiple arrays.   Determine an appropriate scale factor
    to apply to the transform to limit line counts.
@param pBoundaryArray1 IN array of boundaries
@param numBoundary1 IN number of boundareis in pBoundaryArray1
@param pBoundaryArray2 IN array of boundaries
@param numBoundary2 IN number of boundaries in pBoundaryArray2
@param pTransformIn IN nominal transform (from hatch space to world.  integer z is a hatch plane index)
@param pClipRangePoints IN range of points in clippers.
@param pTransformOut OUT transform with z scale applied to limit lines.
@param pScael OUT applied scale factor.
@returns false if data just looks horrible.
@param
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGraphicsPointArray_applyLineLimitsToTransform
(
GraphicsPointArrayP      *pBoundaryArray1,
int                      numBoundary1,
GraphicsPointArrayP      *pBoundaryArray2,
int                      numBoundary2,
const   Transform     *pTransformIn,
GraphicsPointArrayP      pClipRangePoints,
int                      maxHatchLines,
Transform             *pTransformOut,
double                   *pScale
)
    {
    double zCurr;
    double zMax = 1.0;
    int i;

    if (pScale)
        *pScale = 1.0;
    if (pTransformOut)
        *pTransformOut = *pTransformIn;

    for (i = 0; i < numBoundary1; i++)
        {
        if (!jmdlGPA_hatchDensityScaleExt
                        (&zCurr, pTransformIn, pBoundaryArray1[i], pClipRangePoints, maxHatchLines))
            return false;
        if (zCurr > zMax)
            zMax = zCurr;
        }

    for (i = 0; i < numBoundary2; i++)
        {
        if (!jmdlGPA_hatchDensityScaleExt
                        (&zCurr, pTransformIn, pBoundaryArray2[i], pClipRangePoints, maxHatchLines))
            return false;
        if (zCurr > zMax)
            zMax = zCurr;
        }

    if (pScale)
        *pScale = zMax;
    if (zMax > 1.0 && pTransformIn != NULL)
        {
        RotMatrix scaleMatrix;
        scaleMatrix.InitFromScaleFactors (1.0, 1.0, zMax);
        pTransformOut->InitProduct (*pTransformIn, scaleMatrix);
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
*
* Generate crosshatch in the loops of the boundary array.  Add the crosshatch to the
* instance array.
*
* On return the cross hatch is a sequence of alternating start/stop graphics points.
* On each graphics point of the hatch, the user int is the hatch line index and the
* double value is the sort coordinate along the hatch line.
*
* @param pTranfsorm         => transform to hatch plane space.
*                               Hatch is created on every plane parallel to xy and through
*                               integer z.   In/out is determined by parity rules
*                               after x-direction sort.
* @param selectRule         => selects inside/outside rule.  On each scan line,
*
*                               0 -- simple parity -- alternating in out
*                               1 -- maximal stroke -- only first and last crossings matter
*                               2 -- outer -- if 4 or more crossings, only first and last
*                                       pairs matter.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addTransformedCrossHatchExt
(
        GraphicsPointArray      *pHatchCollector,
GraphicsPointArrayCP pBoundary,
const   Transform                *pTransform,
int     selectRule
)
    {
    jmdlGraphicsPointArray_addTransformedCrossHatchClipped (pHatchCollector, pBoundary, pTransform, NULL, NULL, selectRule);
    }


/*---------------------------------------------------------------------------------**//**
*
* Generate crosshatch in the loops of the boundary array.  Add the crosshatch to the
* instance array.
*
* On return the cross hatch is a sequence of alternating start/stop graphics points.
* On each graphics point of the hatch, the user int is the hatch line index and the
* double value is the sort coordinate along the hatch line.
*
* @param pTranfsorm         => transform to hatch plane space.
*                               Hatch is created on every plane parallel to xy and through
*                               integer z.   In/out is determined by parity rules
*                               after x-direction sort.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addTransformedCrossHatch
(
        GraphicsPointArray      *pHatchCollector,
GraphicsPointArrayCP pBoundary,
const   Transform                *pTransform
)
    {
    jmdlGraphicsPointArray_addTransformedCrossHatchExt (pHatchCollector, pBoundary, pTransform, 0);
    }


/*---------------------------------------------------------------------------------**//**
*
* Generate all points that might be endpoints of crosshatch in a certain direction.
*
* The returned hatch contains every point where the boundary geometry cuts a plane
* in the transformed coordinate system parallel to the xy plane and with integer z.
* In each candidate graphics point, the "point" field contains global coordinates, the
* "a" field contains the distance along the x direction, and the "userData" field contains the
* z (height) index.
*
* The cut points are sorted by userData and a.
*
* @param pTransformm        => transform to hatch plane space.
*                               Hatch is created on every plane parallel to xy and through
*                               integer z.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_addTransformedCrossHatchCutPoints
(
        GraphicsPointArray      *pHatchCollector,
GraphicsPointArrayCP pBoundary,
const   Transform                *pTransform
)
    {
    GPA_TransformedHatchContext context;
    if (jmdlGPA_TCH_initTransform (&context, pBoundary, pHatchCollector, pTransform, NULL, NULL, s_maxLines))
        {
        jmdlGPA_TCH_collectCrossings (&context);
        jmdlGPA_TCH_sort (&context);
        }
    jmdlGPA_TCH_teardown (&context);
    }


/*---------------------------------------------------------------------------------**//**
* Compute point set difference (GPA0)-(GPA1)
* where each cross hatch set has
*<ul>
*<li>userData = scanline number</li>
*<li>a = horizontal position on scanline</li>
*</ul>
* and each is lexically sorted by to cluster by scan line, and sorted by a within
*   common scan line.
* @param pGPAOut <= result cross hatch.
* @param pGPA0 => outer boundary cross hatch
* @param pGPA1 => inner boundary cross hatch
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_crossHatchDifference
(
        GraphicsPointArray  *pGPAOut,
GraphicsPointArrayCP pGPA0,
GraphicsPointArrayCP pGPA1
)
    {
    int i0 = 0;
    int i1 = 0;
    /* We really expect y values to be on integer levels */
    int n0 = jmdlGraphicsPointArray_getCount (pGPA0);
    int n1 = jmdlGraphicsPointArray_getCount (pGPA1);
    const GraphicsPoint *pGP0base = jmdlGraphicsPointArray_getConstPtr (pGPA0, 0);
    const GraphicsPoint *pGP1base = jmdlGraphicsPointArray_getConstPtr (pGPA1, 0);
    const GraphicsPoint *pGP0 = NULL, *pGP1 = NULL;
    GraphicsPoint gpStart, gpEnd;
    bool    feed0, feed1;
    /* If odd counts, ignore the last.  Odd count is caller error, don't worry about preserving it. */
    int m0 = n0 & ~0x01;
    int m1 = n1 & ~0x01;
    jmdlGraphicsPointArray_empty (pGPAOut);

    if (m0 >= 2)
        {
        feed0 = true;
        feed1 = true;
        i0 = -2;
        i1 = -2;

        for (; i0 < m0 && i1 < m1;)
            {
            if (feed0)
                {
                i0 += 2;
                if (i0 >= m0)
                    break;
                pGP0 = pGP0base + i0;
                gpStart = pGP0[0];
                gpEnd   = pGP0[1];
                feed0 = false;
                }

            if (feed1)
                {
                i1 += 2;
                if (i1 >= m1)
                    break;
                pGP1 = pGP1base + i1;
                feed1 = false;
                }

            if (i0 >= m0 || i1 >= m1)
                break;
            if (gpStart.userData < pGP1[0].userData)
                {
                jmdlGraphicsPointArray_addGraphicsPointSegment (pGPAOut, &gpStart, &gpEnd);
                feed0 = true;
                }
            else if (gpStart.userData > pGP1[0].userData)
                {
                feed1 = true;
                }
            else /* (gpStart.userData == pGP1[0].userData) */
                {
                if (pGP1[0].a >= gpEnd.a)
                    {
                    /*  ----------00000000000000------------
                        --------------------------1111111111 */
                    jmdlGraphicsPointArray_addGraphicsPointSegment (pGPAOut, &gpStart, &gpEnd);
                    feed0 = true;
                    }
                else if (pGP1[0].a < gpStart.a)
                    {
                    if (pGP1[1].a <= gpStart.a)
                        {
                        /*  -------------00000000000000-------
                            -----111--------------------------
                        */
                        feed1 = true;
                        }
                    else if (pGP1[1].a <= gpEnd.a)
                        {
                        /*  -------------00000000000000-------
                            -----1111111111111????????????????  */
                        feed1 = true;
                        gpStart = pGP1[1];
                        }
                    else /* if (pGP1[1].a >= gpEnd.a) */
                        {
                        /*  -------------00000000000000-------
                            -----111111111111111111111111111??? */
                        feed0 = true;
                        }
                    }
                else
                    {
                    /*  ----------00000000000000------------
                        ------------------?????????????????? */
                    jmdlGraphicsPointArray_addGraphicsPointSegment (pGPAOut, &gpStart, &pGP1[0]);
                    if (pGP1[1].a >= gpEnd.a)
                        {
                        /*  ----------00000000000000------------
                            ------------------1111111111???????? */
                        feed0 = true;
                        }
                    else
                        {
                        /*  ----------00000000000000------------
                            ------------------111???????????????
                        */
                        gpStart = pGP1[1];
                        feed1 = true;
                        }
                    }
                }
            }
        }
    /* Add all trailing segments ... */
    if (i0 < m0)
        {
        /* Force the current segment out with the (possibly modified)
            local copy of the start. */
        jmdlGraphicsPointArray_addGraphicsPointSegment (pGPAOut, &gpStart, pGP0 + 1);
        pGP0 += 2;
        i0 += 2;
        for (; i0 < m0; i0 += 2, pGP0 += 2)
            {
            jmdlGraphicsPointArray_addGraphicsPointSegment (pGPAOut, pGP0, pGP0 + 1);
            }
        }
    }

typedef void (*BooleanSweepFunction)
    (
    GraphicsPointArray      *pGPAOut,
                int         *pOldSource,        /* Initialized to -1 by sweeper. Subsequent values changed only by the callback. */
            GraphicsPoint   *pOld,
    const   GraphicsPoint   *pNew,
                int         newSource   /* 0 if from pGPA0, 1 if from pGPA1, -1 if cleanup at end */
    );


/*---------------------------------------------------------------------------------**//**
* Boolean union logic for an old and new segment, forcing the old on out if no overlap.
* Assumes old start at or behind new start.
* @param pOldActive <=> flag to say if old data is alive.
* @param pOld <=> saved segment for merging.  TWO graphics points.
* @param pNew => new segment.  TWO graphics points.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    mergeFunc_union
(
GraphicsPointArray      *pGPAOut,
            int         *pOldSource,
        GraphicsPoint   *pOld,
const   GraphicsPoint   *pNew,
            int         newSource
)
    {
    if (!pNew)
        {
        if (*pOldSource >= 0)
            jmdlGraphicsPointArray_addGraphicsPointSegment (pGPAOut, pOld, pOld + 1);
        *pOldSource = -1;
        }
    else if (*pOldSource < 0)
        {
        /* Just save */
        pOld[0] = pNew[0];
        pOld[1] = pNew[1];
        *pOldSource = newSource;
        }
    else if (jmdlGPA_compareHatchCandidates (pOld + 1, pNew) < 0)
        {
        jmdlGraphicsPointArray_addGraphicsPointSegment (pGPAOut, pOld, pOld + 1);
        pOld[0] = pNew[0];
        pOld[1] = pNew[1];
        *pOldSource = newSource;
        }
    else
        {
        if (jmdlGPA_compareHatchCandidates (pOld + 1, pNew + 1) < 0)
                pOld[1] = pNew[1];
        *pOldSource = newSource;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Boolean intersection an old and new segment, forcing the old on out if no overlap.
* Assumes old start at or behind new start.
* @param pOldActive <=> flag to say if old data is alive.
* @param pOld <=> saved segment for merging.  TWO graphics points.
* @param pNew => new segment.  TWO graphics points.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    mergeFunc_intersection
(
GraphicsPointArray  *pGPAOut,
            int         *pOldSource,
        GraphicsPoint   *pOld,
const   GraphicsPoint   *pNew,
            int         newSource
)
    {
    if (!pNew)
        {
        *pOldSource = -1;
        }
    else if (*pOldSource < 0 || *pOldSource == newSource)
        {
        /* Just save */
        pOld[0] = pNew[0];
        pOld[1] = pNew[1];
        *pOldSource = newSource;
        }
    else if (jmdlGPA_compareHatchCandidates (pOld + 1, pNew) < 0)
        {
        pOld[0] = pNew[0];
        pOld[1] = pNew[1];
        *pOldSource = newSource;
        }
    else
        {
        if (jmdlGPA_compareHatchCandidates (pOld + 1, pNew + 1) < 0)
            {
            jmdlGraphicsPointArray_addGraphicsPointSegment (pGPAOut, pNew, pOld + 1);
            pOld[0] = pOld[1];
            pOld[1] = pNew[1];
            *pOldSource = newSource;
            }
        else
            {
            jmdlGraphicsPointArray_addGraphicsPointSegment (pGPAOut, pNew, pNew + 1);
            pOld[0] = pNew[1];
            }
        }
    }



/*---------------------------------------------------------------------------------**//**
* Compute point set booleans for a pair of hatch sets
* where each cross hatch set has
*<ul>
*<li>userData = scanline number</li>
*<li>a = horizontal position on scanline</li>
*</ul>
* and each is lexically sorted by to cluster by scan line, and sorted by a within
*   common scan line.
* @param pGPAOut <= result cross hatch.
* @param pGPA0 => outer boundary cross hatch
* @param pGPA1 => inner boundary cross hatch
* @param mergeFunc0 => function to progress to a segment in pGPA0, and for the residual
*                   (with no new data).
* @param mergeFunc1 => function to progress to a segment in pGAP1
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGraphicsPointArray_booleanSweep
(
        GraphicsPointArray  *pGPAOut,
GraphicsPointArrayCP pGPA0,
GraphicsPointArrayCP pGPA1,
BooleanSweepFunction    mergeFunc
)
    {
    int i0 = 0;
    int i1 = 0;
    int n0 = jmdlGraphicsPointArray_getCount (pGPA0);
    int n1 = jmdlGraphicsPointArray_getCount (pGPA1);
    const GraphicsPoint *pGP0base = jmdlGraphicsPointArray_getConstPtr (pGPA0, 0);
    const GraphicsPoint *pGP1base = jmdlGraphicsPointArray_getConstPtr (pGPA1, 0);
    GraphicsPoint gpOld[2];
    GraphicsPoint const *pGP0;
    GraphicsPoint const *pGP1;
    int oldSource = -1;

    /* If odd counts, ignore the last.  Odd count is caller error, don't worry about preserving it. */
    int m0 = n0 & ~0x01;
    int m1 = n1 & ~0x01;
    jmdlGraphicsPointArray_empty (pGPAOut);
    for (i0 = i1 = 0; i0 < m0 || i1 < m1;)
        {
        pGP0 = pGP0base + i0;
        pGP1 = pGP1base + i1;
        if (i0 < m0 && i1 < m1)
            {
            if (jmdlGPA_compareHatchCandidates (pGP0, pGP1) < 0)
                {
                mergeFunc (pGPAOut, &oldSource, gpOld, pGP0, 0);
                i0 += 2;
                }
            else
                {
                mergeFunc (pGPAOut, &oldSource, gpOld, pGP1, 1);
                i1 += 2;
                }
            }
        else if (i0 < m0)
            {
            mergeFunc (pGPAOut, &oldSource, gpOld, pGP0, 0);
            i0 += 2;
            }
        else
            {
            mergeFunc (pGPAOut, &oldSource, gpOld, pGP1, 1);
            i1 += 2;
            }
        }
    mergeFunc (pGPAOut, &oldSource, gpOld, NULL, -1);
    }



/*---------------------------------------------------------------------------------**//**
* Compute point set union (GPA0) + (GPA1)
* where each cross hatch set has
*<ul>
*<li>userData = scanline number</li>
*<li>a = horizontal position on scanline</li>
*</ul>
* and each is lexically sorted by to cluster by scan line, and sorted by a within
*   common scan line.
* @param pGPAOut <= result cross hatch.
* @param pGPA0 => outer boundary cross hatch
* @param pGPA1 => inner boundary cross hatch
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_crossHatchUnion
(
        GraphicsPointArray  *pGPAOut,
GraphicsPointArrayCP pGPA0,
GraphicsPointArrayCP pGPA1
)
    {
    jmdlGraphicsPointArray_booleanSweep (pGPAOut, pGPA0, pGPA1, mergeFunc_union);
    }


/*---------------------------------------------------------------------------------**//**
* Compute point set intersection (GPA0) + (GPA1)
* where each cross hatch set has
*<ul>
*<li>userData = scanline number</li>
*<li>a = horizontal position on scanline</li>
*</ul>
* and each is lexically sorted by to cluster by scan line, and sorted by a within
*   common scan line.
* @param pGPAOut <= result cross hatch.
* @param pGPA0 => outer boundary cross hatch
* @param pGPA1 => inner boundary cross hatch
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_crossHatchIntersection
(
        GraphicsPointArray  *pGPAOut,
GraphicsPointArrayCP pGPA0,
GraphicsPointArrayCP pGPA1
)
    {
    jmdlGraphicsPointArray_booleanSweep (pGPAOut, pGPA0, pGPA1, mergeFunc_intersection);
    }





END_BENTLEY_GEOMETRY_NAMESPACE