/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
| name          computeCirclePolynomialPoles                            |
|                                                                       |
| author        EarlinLutz                              09/98           |
|                                                                       |
+----------------------------------------------------------------------*/
static int computeXYCirclePolynomialPoles
(
        double  *pPolynomialPoleArray,
MSBsplineCurve  *pPreppedCurve,
int             *pStartIndex,
int             index,
double          x0,
double          y0,
double          r0
)
    {
    int i;
    int k = pStartIndex[index];
    int order = pPreppedCurve->params.order;
    double xDeltaPole[MAX_BEZIER_ORDER];
    double yDeltaPole[MAX_BEZIER_ORDER];
    double wrPole[MAX_BEZIER_ORDER];
    int productOrder = order + order - 1;
    double rr;

    DPoint3d const *pPole   = pPreppedCurve->GetPoleCP ()   + k;
    double   *pWeight = pPreppedCurve->weights + k;

    if (pPreppedCurve->rational)
        {
        /* Rationial function (x/w - x0)^2 + (y/w - y0)^2 = r^2
                Multiply by w^2 for simple polynomial (x-w*x0)^2+(y-w*y0)^2-r^2 w^2 = 0
        */
        for (i = 0; i < order; i++)
            {
            xDeltaPole[i] = pPole[i].x - pWeight[i]*x0;
            yDeltaPole[i] = pPole[i].y - pWeight[i]*y0;
            wrPole[i]     = r0 * pWeight[i];
            }
        bsiBezier_zeroPoles (pPolynomialPoleArray, productOrder, 1);
        bsiBezier_accumulateUnivariateProduct (pPolynomialPoleArray, 0, 1, 1.0,
                                xDeltaPole, order, 0, 1,
                                xDeltaPole, order, 0, 1);
        bsiBezier_accumulateUnivariateProduct (pPolynomialPoleArray, 0, 1, 1.0,
                                yDeltaPole, order, 0, 1,
                                yDeltaPole, order, 0, 1);
        bsiBezier_accumulateUnivariateProduct (pPolynomialPoleArray, 0, 1, -r0 * r0,
                                wrPole, order, 0, 1,
                                wrPole, order, 0, 1
                                );

        }
    else
        {
        for (i = 0; i < order; i++)
            {
            xDeltaPole[i] = pPole[i].x - x0;
            yDeltaPole[i] = pPole[i].y - y0;
            }
        bsiBezier_zeroPoles (pPolynomialPoleArray, productOrder, 1);
        bsiBezier_accumulateUnivariateProduct
                                (
                                pPolynomialPoleArray, 0, 1, 1.0,
                                xDeltaPole, order, 0, 1,
                                xDeltaPole, order, 0, 1);
        bsiBezier_accumulateUnivariateProduct
                                (
                                pPolynomialPoleArray, 0, 1, 1.0,
                                yDeltaPole, order, 0, 1,
                                yDeltaPole, order, 0, 1);
        rr = r0 * r0;
        for (i = 0; i < productOrder; i++)
            pPolynomialPoleArray[i] -= rr;
        }
    return productOrder;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_cb_accumulatePointAndLocalFraction              |
|                                                                       |
| author        EarlinLutz                              07/98           |
|                                                                       |
| Accumlate a parameter+point pair to arrays.  Accumulated parameter    |
| is normalized as a fraction of specified start-end pair.              |
+----------------------------------------------------------------------*/
Public void        bspcurv_cb_accumulatePointAndLocalFraction
(
MSBsplineCurve      *pCurve,
double              startFraction,
double              endFraction,
double              rootFraction,
void                *pUserData1,
void                *pUserData2
)
    {
    EmbeddedDPoint3dArray   *pPointArray    = (EmbeddedDPoint3dArray*)pUserData1;
    bvector<double>   *pParameterArray  = (bvector<double>*)  pUserData2;

    double localFraction = (rootFraction - startFraction) / (endFraction - startFraction);
    DPoint3d point;
    /*
    printf (" global, local root fractions %le %le\n", rootFraction, localFraction);
    */
    if (pPointArray)
        {
        bspcurv_evaluateCurvePoint (&point, NULL, pCurve, rootFraction);
        jmdlEmbeddedDPoint3dArray_addDPoint3d (pPointArray, &point);
        }
    if (pParameterArray)
        {
        pParameterArray->push_back (localFraction);
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_intersectXYCircle                               |
|                                                                       |
| author        EarlinLutz                              07/98           |
|                                                                       |
| Intersect with a circle in xy plane.                                  |
+----------------------------------------------------------------------*/
Public void                bspcurv_intersectXYCircle
(
MSBsplineCurve  *curveP,            /* => input curve */
double          startFraction,      /* => starting fractional parameter */
double          endFraction,        /* => ending fractional parameter */
double          x0,                 /* => circle center x */
double          y0,                 /* => circle center y */
double          r0,                 /* => circle radius */
double          absTol,             /* => absolute tolerance for double roots. */
MSBsplineCurve_AnnounceParameter F,
void            *pUserData0,
void            *pUserData1
)
    {
    int             i, status, numSegments, *start = NULL, j;
    double          upper, lower, diff;
    int             spanOrder;
    int             numRoot;
    double          rootParameter;
    double          polynomialPoles[MAX_BEZIER_ORDER];
    double          polynomialRoots[MAX_BEZIER_ORDER];
    double startParam = mdlBspline_fractionParameterToNaturalParameter (curveP, startFraction);
    double endParam   = mdlBspline_fractionParameterToNaturalParameter (curveP, endFraction);


    MSBsplineCurve  curve;


    /* Break curve to Bezier segments */
    if (SUCCESS != (status = bspproc_prepareCurve (&curve, &numSegments, &start, curveP)))
        goto wrapup;

    /* For each Bezier segment ... */
    for (i = 0; i < numSegments; i++)
        {
        lower = curve.knots[start[i]+1];
        upper = curve.knots[start[i]+curve.params.order];

        if (upper <= startParam || lower >= endParam)
            {

            }
        else
            {
            diff = upper - lower;
            spanOrder = computeXYCirclePolynomialPoles (polynomialPoles, &curve, start, i, x0, y0, r0);
            bsiBezier_univariateRoots (polynomialRoots, &numRoot, polynomialPoles, spanOrder);
            for (j = 0; j < numRoot; j++)
                {
                rootParameter = lower + polynomialRoots[j] * diff;
                if (rootParameter >= startParam && rootParameter <= endParam)
                    {
                    double rootFraction = mdlBspline_naturalParameterToFractionParameter (curveP, rootParameter);
                    F (curveP, startFraction, endFraction, rootFraction, pUserData0, pUserData1);
                    }
                }
            }
        }
wrapup:
    if (start)      bspsurf_freeArray ((void**)&start);
    bspcurv_freeCurve (&curve);
    }

static int s_noisy = 0;
#ifdef USE_ARTIFICIAL_GEOMETRY
/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_intersectXY_MSBsplineCurve_circle                |
|                                                                       |
| Author:   EarlinLutz                               9/21/00            |
|                                                                       |
+----------------------------------------------------------------------*/
static bool    jmdlRIMSBS_intersectXY_MSBsplineCurve_circle
(
RIMSBS_Context          *pContext,          /* => general context */
bvector<double>     *pParamArray,       /* <= intersection parameters on the ellipse */
EmbeddedDPoint3dArray   *pPointArray,       /* <= intersection points on the ellipse */
MSBsplineCurve          *pCurve,            /* => curve */
const DPoint3d          *pCenter,           /* => circle center */
double                  radius              /* => circle radius */
)
    {
    MSBsplineCurve circleAsCurve;
    RotMatrix z0Matrix;
    DEllipse3d circle;
    DPoint3d curvePoint;
    int i, numIntersection;
    double param;
    double *pCurveParam;
    static double s_relTol = 1.0e-8;
    double refScale = bsiDPoint3d_magnitude (pCenter) + fabs (radius);
    double tol = s_relTol * refScale;


    if (s_noisy)
        printf ("   MSBsplineCurve intersect circle \n");

    bsiRotMatrix_initFromScaleFactors (&z0Matrix, 1.0, 1.0, 0.0);

    circle.Init (
                    pCenter->x, pCenter->y, pCenter->z,
                    radius, 0.0, 0.0,
                    0.0, radius, 0.0,
                    0.0, msGeomConst_2pi
                    );
    bspconv_convertDEllipse3dToCurve (&circleAsCurve, &circle);

    bspcci_allIntersectsBtwCurves
                    (
                    NULL, &pCurveParam, NULL, &numIntersection,
                    pCurve, &circleAsCurve, &tol, &z0Matrix, false);

    if (numIntersection > 0)
        {
        for (i = 0; i < numIntersection; i++)
            {
            param = pCurveParam[i];

            if (pParamArray)
                pParamArray->push_back (param);

            if (pPointArray)
                {
                bspcurv_evaluateCurvePoint (&curvePoint, NULL, pCurve, param);
                jmdlEmbeddedDPoint3dArray_addDPoint3d
                            (pPointArray, &curvePoint);
                }
            }
        bsputil_free (pCurveParam);
        }

    bspcurv_freeCurve (&circleAsCurve);
    return true;
    }
#endif
    /*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_intersectXY_DEllipse3d_circle                    |
|                                                                       |
| Author:   EarlinLutz                               9/21/00            |
|                                                                       |
+----------------------------------------------------------------------*/
static bool    jmdlRIMSBS_intersectXY_DEllipse3d_circle
(
RIMSBS_Context          *pContext,          /* => general context */
bvector<double>     *pParameterArray,   /* <= intersection parameters on the ellipse */
EmbeddedDPoint3dArray   *pPointArray,       /* <= intersection points on the ellipse */
const DEllipse3d        *pEllipse,          /* => the ellipse */
const DPoint3d          *pCenter,           /* => circle center */
double                  radius              /* => circle radius */
)
    {
    DEllipse3d circle;
    DPoint3d trigPoint[4];
    double   ellipseAngle[4];
    double fraction;
    DPoint3d point;
    int numIntersection, i;

    if (s_noisy)
        printf ("   DEllipse3d intersect circle \n");

    if (pParameterArray)
        pParameterArray->clear();

    if (pPointArray)
        jmdlEmbeddedDPoint3dArray_empty (pPointArray);

    circle.Init (
                    pCenter->x, pCenter->y, pCenter->z,
                    radius, 0.0, 0.0,
                    0.0, radius, 0.0,
                    0.0, msGeomConst_2pi
                    );

#ifdef oldOrder
    numIntersection = pEllipse->IntersectXYDEllipse3dBounded
                    (
                    NULL,
                    trigPoint,
                    ellipseAngle,
                    NULL,
                    NULL,
                    circle
                    );
#else
    numIntersection = circle.IntersectXYDEllipse3dBounded
                    (
                    NULL,
                    NULL,
                    NULL,
                    trigPoint,
                    ellipseAngle,
                    *pEllipse
                    );
#endif
    for (i = 0; i < numIntersection; i++)
        {
        if (pParameterArray)
            {
            fraction = pEllipse->AngleToFraction (ellipseAngle[i]);
            pParameterArray->push_back (fraction);
            }
        if (pPointArray)
            {
            pEllipse->EvaluateTrigPairs (&point, (DPoint2d *)&trigPoint[i], 1);
            jmdlEmbeddedDPoint3dArray_addDPoint3d (pPointArray, &point);
            }
        }
    return true;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_curveCircleIntersectionXY                        |
|                                                                       |
| Author:   EarlinLutz                               9/21/00            |
|                                                                       |
+----------------------------------------------------------------------*/
bool RIMSBS_Context::TryIntersectXY_mappedCurve_circle
(
bvector<double> *pParameterArray,   /* <= curve parameters of intersections. */
bvector<DPoint3d>*pPointArray,       /* <= curve points of intersections */
int                     parentCurveId,
double                  s0,
double                  s1,
const DPoint3d          *pCenter,           /* => circle center */
double                  radius
)
    {
    bool    myResult = false;

    if (s_noisy)
        printf ("    mapped curve (%lf,%lf) circle \n", s0, s1);

    if (pParameterArray)
        pParameterArray->empty ();

    if (pPointArray)
        pPointArray->empty ();

    if (IsValidCurveIndex (parentCurveId))
        {
        RIMSBS_ElementHeader &desc = GetElementR (parentCurveId);
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d arc;
                TryGetArc (parentCurveId, arc);
                DEllipse3d partialEllipse;
                jmdlRIMSBS_initPartialEllipse (&partialEllipse, &arc, s0, s1);
                jmdlRIMSBS_intersectXY_DEllipse3d_circle
                        (
                        this,
                        pParameterArray, pPointArray,
                        &partialEllipse,
                        pCenter, radius
                        );
                myResult = true;
                break;
                }
            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurveP pCurve = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (parentCurveId));
#ifdef USE_ARTIFICIAL_GEOMETRY
                MSBsplineCurve partialCurve;
                bspcurv_segmentCurve (&partialCurve, pCurve, s0, s1);
                jmdlRIMSBS_intersectXY_MSBsplineCurve_circle
                            (
                            pContext,
                            pParameterArray, pPointArray,
                            &partialCurve,
                            pCenter, radius
                            );
                bspcurv_freeCurve (&partialCurve);
#elif defined (PROJECT_TO_GLOBAL_CURVE)
                bspcurv_intersectXYCircle (pCurve, s0, s1,
                                pCenter->x, pCenter->y, radius,
                                0.0, bspcurv_cb_accumulatePointAndLocalFraction,
                                pPointArray, pParameterArray);
#else
                MSBsplineCurve partialCurve;
                bspcurv_segmentCurve (&partialCurve, pCurve, s0, s1);
                bspcurv_intersectXYCircle (&partialCurve, 0.0, 1.0,
                                pCenter->x, pCenter->y, radius,
                                0.0, bspcurv_cb_accumulatePointAndLocalFraction,
                                pPointArray, pParameterArray);
                bspcurv_freeCurve (&partialCurve);
#endif
                myResult = true;
                break;
                }
            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = &desc.m_partialCurve;
                double dt = pInterval->s1 - pInterval->s0;
                double t0 = pInterval->s0 + s0 * dt;
                double t1 = pInterval->s0 + s1 * dt;
                myResult = TryIntersectXY_mappedCurve_circle
                        (
                        pParameterArray, pPointArray,
                        pInterval->parentId, t0, t1,
                        pCenter, radius
                        );
                break;
                }
            case RIMSBS_CurveChain:
                {
                RG_CurveId primaryCurveId = desc.m_chainData.m_primaryCurveId;
                myResult = TryIntersectXY_mappedCurve_circle (pParameterArray, pPointArray, primaryCurveId,
                          s0, s1, pCenter, radius);
                break;
                }
                break;
            }
        }
    return  myResult;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_curveCircleIntersectionXY                        |
|                                                                       |
| Author:   EarlinLutz                               9/21/00            |
|                                                                       |
+----------------------------------------------------------------------*/
Public bool    jmdlRIMSBS_curveCircleIntersectionXY
(
RIMSBS_Context          *pContext,          /* => general context */
bvector<double>     *pParameterArray,   /* <= curve parameters of intersections. */
EmbeddedDPoint3dArray   *pPointArray,       /* <= curve points of intersections */
RG_EdgeData             *pEdgeData,         /* => segment edge data */
const DPoint3d          *pCenter,           /* => circle center */
double                  radius
)
    {
    return pContext->TryCurveCircleIntersectionXY (pParameterArray, pPointArray, pEdgeData, pCenter, radius);
    }

bool    RIMSBS_Context::TryCurveCircleIntersectionXY
(
bvector<double> *pParameterArray,   /* <= curve parameters of intersections. */
bvector<DPoint3d>*pPointArray,       /* <= curve points of intersections */
RG_EdgeData             *pEdgeData,         /* => segment edge data */
const DPoint3d          *pCenter,           /* => circle center */
double                  radius
)
    {
    bool    myResult = false;
    int curveId = jmdlRGEdge_getCurveId (pEdgeData);

    if (pParameterArray)
        pParameterArray->empty ();

    if (pPointArray)
        pPointArray->empty ();

    if (s_noisy)
        printf (" (jmdlRIMSBS_curveCircleIntersectionXY\n");

    if (IsValidCurveIndex (curveId))
        {
        RIMSBS_ElementHeader &desc = GetElementR (curveId);
        switch  (desc.type)
            {
            case RIMSBS_DEllipse3d:
                {
                DEllipse3d arc;
                TryGetArc (curveId, arc);
                jmdlRIMSBS_intersectXY_DEllipse3d_circle
                        (
                        this,
                        pParameterArray, pPointArray,
                        &arc,
                        pCenter, radius
                        );
                myResult = true;
                break;
                }
            case RIMSBS_MSBsplineCurve:
                {
                MSBsplineCurveP pCurve = const_cast<MSBsplineCurveP> (GetMSBsplineCurveCP (curveId));
#ifdef USE_ARTIFICIAL_GEOMETRY
                jmdlRIMSBS_intersectXY_MSBsplineCurve_circle
                            (
                            pContext,
                            pParameterArray, pPointArray,
                            pCurve,
                            pCenter, radius
                            );
#else
                bspcurv_intersectXYCircle (pCurve, 0.0, 1.0,
                                pCenter->x, pCenter->y, radius,
                                0.0, bspcurv_cb_accumulatePointAndLocalFraction,
                                pPointArray, pParameterArray);
#endif
                myResult = true;
                break;
                }
            case RIMSBS_CurveInterval:
                {
                RIMSBS_CurveIntervalStruct *pInterval = &desc.m_partialCurve;
                myResult = TryIntersectXY_mappedCurve_circle
                        (
                        pParameterArray, pPointArray,
                        pInterval->parentId, pInterval->s0, pInterval->s1,
                        pCenter, radius
                        );
                break;
                }
            }
        }
    if (s_noisy)
        {
        if (pParameterArray)
            {
            for (double s : *pParameterArray)
                {
                printf ("    s= %le\n", s);
                }
            }
        printf (" )\n");
        }
    return  myResult;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
