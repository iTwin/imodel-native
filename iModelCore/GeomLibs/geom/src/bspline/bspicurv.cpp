/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bspicurv.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define MAX_DEPTH           10

/*---------------------------------------------------------------------------------**//**
* Estimate an integral by simpson's rule, and estimate the absolute value integral by 2-part trapezoid rule.
* @bsimethod                                                    Earlin.Lutz     08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int s_maxDepth;
static void simpQuad
(
double          *sumP,      /* <= simpson's rule estimate of integral */
double          *absP,      /* <= trapezoidal estimate of integral of absolute value */
double          f0,         /* => integrand at 0 */
double          fM,         /* => integrand at midpoint */
double          f1,         /* => integrand at 1 */
double          delta       /* => interval width */
)
    {
    *sumP = delta * (f0 + 4.0 * fM + f1) / 6.0;
    *absP = delta * 0.25 * ( fabs (f0) + 2.0 * fabs (fM) + fabs (f1));
    }

static double s_relTolFactor = 1.0;
static double s_absTolFactor = 0.707;

typedef StatusInt  (*MSBsplineScalarFunc)
(
double  *pValue,        /* <= function value */
DPoint3dCR point,
DVec3dCR tangent,
DPoint3dCP origin,
int     userData1       /* => context data */
);


typedef struct
    {
    int             numDeriv;
    DPoint3d        origin;
    int             userInt;
    MSBsplineScalarFunc F;
    BCurveSegment   segment;
    int             count;
    } TangentialFunctionThunk;

typedef int (*SimpsonEvaluationFunction)(double*, double, TangentialFunctionThunk*);

typedef StatusInt   (*IntegrationControlFunction)
(
double          *integralP,         /* <= resulting integration */
double          *errorP,            /* <= achived absolute error */
double          x0,                 /* => lower limit for integration */
double          x1,                 /* => upper limit for integration */
double          absTol,             /* => relative tolerance required */
double          relTol,             /* => relative tolerance required */
SimpsonEvaluationFunction evaluateFunc,  /* => evaluate function for integrand */
TangentialFunctionThunk *thunkP     /* => user data */
);




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/95
+---------------+---------------+---------------+---------------+---------------+------*/
static int     recursiveSubdivisionExt
(
double          *integralP,         /* <= resulting integration */
double          *errorP,            /* <= achived absolute error */
double          x0,                 /* => lower limit for integration */
double          f0,                 /* => integrand at lower limit */
double          xM,                 /* => midpoint of interval */
double          fM,                 /* => integrand at midpoint */
double          x1,                 /* => upper limit for integration */
double          f1,                 /* => integrand at lower limit */
double          g01,                /* => simpson's rule estimate for overall integral */
double          absTol,             /* => absolute tolerance required */
double          relTol,             /* => relative tolerance required */
SimpsonEvaluationFunction evaluateFunc,  /* => evaluate function for integrand */
TangentialFunctionThunk* userDataP,         /* => passed through to evaluateFunc */
int             depth
)
    {
    double      error;
    double      x2 = 0.5 * (x0 + xM);
    double      x3 = 0.5 * (xM + x1);
    double      f2, f3;
    double      g0, g1, g;
    double      a0, a1;
    double      h   = 0.5 * (x1 - x0);


    (*evaluateFunc) (&f2, x2, userDataP);
    (*evaluateFunc) (&f3, x3, userDataP);

    simpQuad (&g0, &a0, f0, f2, fM, h);
    simpQuad (&g1, &a1, fM, f3, f1, h);

    g = g0 + g1;

    error = fabs (g01 - g);

    if (depth > MAX_DEPTH || error < absTol + relTol * (fabs (a0) + fabs (a1)) )
        {
        *integralP += g;
        *errorP += error;
        return SUCCESS;
        }

    depth++;
    if (depth > s_maxDepth)
        s_maxDepth = depth;

    recursiveSubdivisionExt (integralP, errorP,
            x0, f0, x2, f2, xM, fM, g0,
            s_absTolFactor * absTol, s_relTolFactor * relTol,
            evaluateFunc, userDataP, depth);

    recursiveSubdivisionExt (integralP, errorP,
            xM, fM, x3, f3, x1, f1, g1,
            s_absTolFactor * absTol, s_relTolFactor * relTol,
            evaluateFunc, userDataP, depth);

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/95
+---------------+---------------+---------------+---------------+---------------+------*/
static int     recursiveSimpsonIntegrationExt
(
double          *integralP,         /* <= resulting integration */
double          *errorP,            /* <= achived absolute error */
double          x0,                 /* => lower limit for integration */
double          x1,                 /* => upper limit for integration */
double          absTol,             /* => relative tolerance required */
double          relTol,             /* => relative tolerance required */
SimpsonEvaluationFunction evaluateFunc,  /* => evaluate function for integrand */
TangentialFunctionThunk* userDataP          /* => passed through to evaluateFunc */
)
    {
    double f0, f1, fM, g01, a01;
    double xM;

    xM = 0.5 * (x0 + x1);
    (*evaluateFunc) (&f0, x0, userDataP);
    (*evaluateFunc) (&fM, xM, userDataP);
    (*evaluateFunc) (&f1, x1, userDataP);
    simpQuad (&g01, &a01, f0, fM, f1, x1 - x0);
    *integralP = *errorP = 0.0;
    return (recursiveSubdivisionExt (integralP, errorP,
        x0, f0, xM, fM, x1, f1, g01,
        absTol, relTol, evaluateFunc,
        userDataP, 0));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/98
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt evaluateTangentialFunction
(
double          *valueP,    /* <= value evaluated at param */
double          param,      /* => parameter to evaluate at */
TangentialFunctionThunk *thunkP     /* => user data */
)
    {
    StatusInt       status = ERROR;
    double          value = 0.0;
    thunkP->count ++;

    DPoint3d point;
    DVec3d tangent;
    thunkP->segment.FractionToPoint (point, tangent, param, false);

    status = thunkP->F (&value, point, tangent, &thunkP->origin, thunkP->userInt);

    if (SUCCESS == status)
        {
        *valueP = value;
        }
    else
        {
        *valueP = 0.0;
        }
    return status;

    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/98
+---------------+---------------+---------------+---------------+---------------+------*/
static         int     bspcurv_integrateTangentialFunction
(
double          *fP,                /* <= integral */
double          *errorAchievedP,     /* <= the absolute error achieved, i.e.,
                                          abs(arcLengthP - trueArcLength) */
double          startFraction,      /* => starting fraction parameter */
double          endFraction,        /* => ending fraction parameter */
MSBsplineCurve  *curveP,            /* => input curve */
int             numDeriv,           /* => number of derivatives required by function.
                                            This must be 2 or less. */
MSBsplineScalarFunc F,              /* => function returning integrand */
IntegrationControlFunction  G,      /* => function to carry out local integrations */
DPoint3d        *pOrigin,
int             userInt,
double          absTol,             /* absolute tolerance on integration */
double          relTol              /* => relative tolerance for integration */
)
    {
    //bool            startIn = false;
    int             status = SUCCESS;
    double          error, length, errorFactor;
    double startParam = mdlBspline_fractionParameterToNaturalParameter (curveP, startFraction);
    double endParam   = mdlBspline_fractionParameterToNaturalParameter (curveP, endFraction);
    TangentialFunctionThunk thunk;
    thunk.numDeriv   = numDeriv;
    thunk.origin     = *pOrigin;
    thunk.F          = F;
    thunk.userInt    = userInt;
    thunk.count      = 0;

    /* Loop each Bezier segment */
    *fP = 0.0;
    *errorAchievedP = 0.0;
    int numSegments = 1;    // NEEDS WORK -- get true segment count.
    errorFactor = relTol / numSegments;
    s_maxDepth = 0;
    DRange1d interval = DRange1d::From (startParam, endParam);
    for (size_t i = 0; curveP->AdvanceToBezierInKnotInterval  (thunk.segment, i, interval);)
        {
        G (&length, &error,
            0.0, 1.0,       // Use the entire bezier
            absTol, relTol,
            evaluateTangentialFunction, &thunk);

        *fP += length;
        *errorAchievedP += error;

        }
#ifdef PRINT_INTEGRATION_STATISTICS
    printf(" Integration %le (+-) %le # %d depth %d\n",
                *fP,
                *errorAchievedP,
                thunk.count, s_maxDepth
                );
#endif
    return status;
    }

typedef enum
    {
    SweepArea   = 0,
    SweepAngle  = 1,
    ArcLength   = 2,
    momentXdL   = 3,
    momentYdL   = 4,
    momentZdL   = 5
    } SweepSelector;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspcurv_sweepFunc
(
double          *fP,                /* <= integrand */
DPoint3dCR      point,              // Point on curve
DVec3dCR        tangent,            // derivative
DPoint3dCP      originP,           /* => fixed point for sweep */
int             selector            /* => selects integrand */
)
    {
    DPoint3d offset;
    static double squareDistTol = 1.0e-20;
    *fP = 0.0;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&offset, &point, originP);
    if (selector == SweepArea)
        {
        *fP = 0.5 * bsiDPoint3d_crossProductXY (&offset, &tangent);
        }
    else if (selector == SweepAngle)
        {
        double rr = bsiDPoint3d_dotProductXY (&offset, &offset);
        double tt = bsiDPoint3d_dotProductXY (&tangent, &tangent);
        /* Hmm .. how big is zero ? */
        if (rr <= squareDistTol * tt)
            return ERROR;
        *fP = bsiDPoint3d_crossProductXY (&offset, &tangent) / rr;
        }
    else if (selector == ArcLength)
        {
        *fP = sqrt (bsiDPoint3d_dotProduct (&tangent, &tangent));
        }
    else if (selector == momentXdL)
        {
        *fP = (point.x - originP->x) * sqrt (bsiDPoint3d_dotProductXY (&tangent, &tangent));
        }
    else if (selector == momentYdL)
        {
        *fP = (point.y - originP->y) * sqrt (bsiDPoint3d_dotProductXY (&tangent, &tangent));
        }
    else if (selector == momentZdL)
        {
        *fP = (point.z - originP->z) * sqrt (bsiDPoint3d_dotProductXY (&tangent, &tangent));
        }

    return SUCCESS;

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bspcurv_fixRelTol
(
double          relTol
)
    {
    static double s_minRelTol = 1.0e-14;
    return relTol < s_minRelTol ? s_minRelTol : relTol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_sweptArea
(
double          *fP,                /* <= integral */
double          *errorAchievedP,     /* <= the absolute error achieved, i.e.,
                                          abs(arcLengthP - trueArcLength) */
double          startFraction,      /* => strating parameter */
double          endFraction,        /* => ending parameter */
MSBsplineCurve  *curveP,            /* => input curve */
const DPoint3d  *originP,           /* => origin for sweeping ray */
double          relativeTol         /* => relative tolerance for integration
                                          using Simpson's Rule, i.e.,
                                          abs(errorAchivedP) / arcLengthP <=
                                          relativeTol */
)
    {
    DPoint3d origin = *originP;
    static double s_minimumArea = 1.0;
    relativeTol = bspcurv_fixRelTol (relativeTol);
    return bspcurv_integrateTangentialFunction (fP, errorAchievedP, startFraction, endFraction, curveP,
                                                1,
                                                bspcurv_sweepFunc,
                                                recursiveSimpsonIntegrationExt,
                                                &origin,
                                                SweepArea,
                                                relativeTol * s_minimumArea,
                                                relativeTol);
    }

/*-----------------------------------------------------------------*//**
* @param arclengthP <= arc length
* @param centroidP <= centroid
* @param startParam => start parameter of effective interval.
* @param endParam => end parameter of effective interval.
* @param curveP => curve
* @param relativeTol relative tolerance for integrations.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_pathCentroid
(
double          *arcLengthP,
DPoint3d        *centroidP,
double          startFraction,
double          endFraction,
MSBsplineCurve  *curveP,
double          relativeTol
)
    {
    DPoint3d origin;
    double integral_dL, integral_XdL, integral_YdL, integral_ZdL;
    DPoint3d centroid;
    DPoint3d localCentroid;
    double s_refLength = bspcurv_polygonLength (curveP);
    double s_refArea = s_refLength * s_refLength;
    StatusInt status = ERROR;
    double error = 0.0;
    //double startParam = mdlBspline_fractionParameterToNaturalParameter (curveP, startFraction);
    //double endParam = mdlBspline_fractionParameterToNaturalParameter (curveP, endFraction);
    relativeTol = bspcurv_fixRelTol (relativeTol);



    bspcurv_evaluateCurvePoint (&origin, NULL, curveP, 0.5);

    centroid = origin;

    if (    SUCCESS == bspcurv_integrateTangentialFunction
                        (&integral_dL, &error, startFraction, endFraction,
                        curveP, 1, bspcurv_sweepFunc, recursiveSimpsonIntegrationExt,
                        &origin, ArcLength, relativeTol * s_refLength, relativeTol)
        && SUCCESS == bspcurv_integrateTangentialFunction
                        (&integral_XdL, &error, startFraction, endFraction,
                        curveP, 1, bspcurv_sweepFunc, recursiveSimpsonIntegrationExt,
                        &origin, momentXdL, relativeTol * s_refArea, relativeTol)
        && SUCCESS == bspcurv_integrateTangentialFunction
                        (&integral_YdL, &error, startFraction, endFraction,
                        curveP, 1, bspcurv_sweepFunc, recursiveSimpsonIntegrationExt,
                        &origin, momentYdL, relativeTol * s_refArea, relativeTol)
        && SUCCESS == bspcurv_integrateTangentialFunction
                        (&integral_ZdL, &error, startFraction, endFraction,
                        curveP, 1, bspcurv_sweepFunc, recursiveSimpsonIntegrationExt,
                        &origin, momentZdL, relativeTol * s_refArea, relativeTol)
        && bsiTrig_safeDivide (&localCentroid.x, integral_XdL, integral_dL, 0.0)
        && bsiTrig_safeDivide (&localCentroid.y, integral_YdL, integral_dL, 0.0)
        && bsiTrig_safeDivide (&localCentroid.z, integral_ZdL, integral_dL, 0.0)
        )
        {
        bsiDPoint3d_addDPoint3dDPoint3d (&centroid, &localCentroid, &origin);
        status = SUCCESS;
        }
    if (arcLengthP)
        *arcLengthP = integral_dL;
    if (centroidP)
        *centroidP = centroid;
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_sweptAngle
(
double          *fP,                /* <= integral */
double          *errorAchievedP,     /* <= the absolute error achieved, i.e.,
                                          abs(arcLengthP - trueArcLength) */
double          startFraction,      /* => strating parameter */
double          endFraction,        /* => ending parameter */
MSBsplineCurve  *curveP,            /* => input curve */
const DPoint3d  *originP,           /* => origin for sweeping ray */
double          relativeTol         /* => relative tolerance for integration
                                          using Simpson's Rule, i.e.,
                                          abs(errorAchivedP) / arcLengthP <=
                                          relativeTol */
)
    {
    DPoint3d origin = *originP;
    static double s_minimumAngle = 1.0e-5;
    relativeTol = bspcurv_fixRelTol (relativeTol);
    return bspcurv_integrateTangentialFunction (fP, errorAchievedP, startFraction, endFraction, curveP,
                                                1,
                                                bspcurv_sweepFunc,
                                                recursiveSimpsonIntegrationExt,
                                                &origin,
                                                SweepAngle,
                                                relativeTol * s_minimumAngle,
                                                relativeTol);

    }


typedef struct
    {
    DPoint3d xyz[3];
    bool    bEdgeNormalsComputed;
    DPoint4d planeCoffs;
    DVec3d planeNormal;
    DVec3d edgeNormal[3];
    MSBsplineCurve_AnnounceParameter *F;
    void *pUserData0;
    void *pUserData1;
    } MSBsplineCurveTriangleIntersectionContext;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void cb_cti_announcePlaneIntersection
(
MSBsplineCurve *curveP,
double          startFraction,
double          endFraction,
double          rootFraction,
MSBsplineCurveTriangleIntersectionContext *pContext,
void            *pUnused
)
    {
    DPoint3d curvePoint;
    int i;
    bspcurv_evaluateCurvePoint (&curvePoint, NULL, curveP, rootFraction);
    if (!pContext->bEdgeNormalsComputed)
        {
        for (i = 0; i < 3; i++)
            {
            int j = (i + 1) % 3;
            DVec3d edgeVector;
            bsiDVec3d_subtractDPoint3dDPoint3d
                        (
                        &edgeVector,
                        &pContext->xyz[j],
                        &pContext->xyz[i]
                        );
            bsiDVec3d_normalizedCrossProduct
                        (
                        &pContext->edgeNormal[i],
                        &pContext->planeNormal,
                        &edgeVector
                        );
            }
        pContext->bEdgeNormalsComputed = true;
        }

    // The "inside" has positive dots with all normals...
    for (i = 0; i < 3; i++)
        {
        double a = bsiDPoint3d_dotDifference (&curvePoint, &pContext->xyz[i], &pContext->edgeNormal[i]);
        if (a < 0.0)
            return;
        }
    // Still alive to announce ....
    pContext->F (curveP, startFraction, endFraction, rootFraction,
                pContext->pUserData0,
                pContext->pUserData1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    cti_init
(
MSBsplineCurveTriangleIntersectionContext *pContext,
DPoint3d const  *pXYZ0,
DPoint3d const *pXYZ1,
DPoint3d const *pXYZ2,
MSBsplineCurve_AnnounceParameter F,
void            *pUserData0,
void            *pUserData1
)
    {
    pContext->xyz[0] = *pXYZ0;
    pContext->xyz[1] = *pXYZ1;
    pContext->xyz[2] = *pXYZ2;
    pContext->bEdgeNormalsComputed = false;
    pContext->pUserData0 = pUserData0;
    pContext->pUserData1 = pUserData1;
    pContext->F = F;
    bsiDVec3d_crossProduct3DPoint3d (&pContext->planeNormal, pXYZ0, pXYZ1, pXYZ2);
    bsiDVec3d_normalizeInPlace (&pContext->planeNormal);
    if (bsiDPoint4d_planeFromOriginAndNormal (&pContext->planeCoffs, pXYZ0, &pContext->planeNormal))
        {
        return true;
        }
    else
        {
        return false;
        }
    }

/*--------------------------------------------------------------------*//**
@description Compute the intersection of a bspline curve with a triangle.
@param curveP IN curve
@param startFraction IN start fraction for curve portion to be considered
@param endFraction IN end fraction for curve portion to be considered
@param pXYZ0 IN triangle vertex
@param pXYZ1 IN triangle vertex
@param pXYZ2 IN triangle vertex
@param F IN function to call to accumulate results
        Call is F (curveP, startFraction, endFraction, rootFraction, pUserData0, pUserData1);
@param pUserData0 IN arg for F
@param pUserData1 IN arg for F
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void  bspcurv_intersectTriangle
(
MSBsplineCurve  *curveP,            /* => input curve */
double          startFraction,      /* => starting fractional parameter */
double          endFraction,        /* => ending fractional parameter */
DPoint3d        *pXYZ0,
DPoint3d        *pXYZ1,
DPoint3d        *pXYZ2,
MSBsplineCurve_AnnounceParameter F,
void            *pUserData0,
void            *pUserData1
)
    {
    MSBsplineCurveTriangleIntersectionContext context;
    if (cti_init (&context, pXYZ0, pXYZ1, pXYZ2, F, pUserData0, pUserData1))
        {
        bspcurv_intersectPlane
                (
                curveP, startFraction, endFraction, &context.planeCoffs, 0.0,
                (MSBsplineCurve_AnnounceParameter*)cb_cti_announcePlaneIntersection, &context, NULL
                );
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void cb_collectFractionAndXYZ
(
MSBsplineCurve *curveP,
double          startFraction,
double          endFraction,
double          rootFraction,
EmbeddedDoubleArray *pFractionArray,
EmbeddedDPoint3dArray *pXYZArray
)
    {
    if (pXYZArray)
        {
        DPoint3d curvePoint;
        bspcurv_evaluateCurvePoint (&curvePoint, NULL, curveP, rootFraction);
        jmdlEmbeddedDPoint3dArray_addDPoint3d (pXYZArray, &curvePoint);
        }

    if (pFractionArray)
        jmdlEmbeddedDoubleArray_addDouble (pFractionArray, rootFraction);
    }

/*--------------------------------------------------------------------*//**
@description Compute the intersection of a bspline curve with a triangle.
@param curveP IN curve
@param startFraction IN start fraction for curve portion to be considered
@param endFraction IN end fraction for curve portion to be considered
@param pXYZ0 IN triangle vertex
@param pXYZ1 IN triangle vertex
@param pXYZ2 IN triangle vertex
@param pFractionArray IN OUT intersect parameters wrt curve are appended to this array.
@param pXYZArray IN OUT intersection xyz appended to this array.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void  bspcurv_collectTriangleIntersections
(
MSBsplineCurve  *curveP,            /* => input curve */
double          startFraction,      /* => starting fractional parameter */
double          endFraction,        /* => ending fractional parameter */
DPoint3d        *pXYZ0,
DPoint3d        *pXYZ1,
DPoint3d        *pXYZ2,
EmbeddedDoubleArray *pFractionArray,
EmbeddedDPoint3dArray *pXYZArray
)
    {
    bspcurv_intersectTriangle (curveP, startFraction, endFraction, pXYZ0, pXYZ1, pXYZ2,
                    (MSBsplineCurve_AnnounceParameter*)cb_collectFractionAndXYZ, pFractionArray, pXYZArray);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_convertDPoint4dBezierToCurve
(
MSBsplineCurve      *pCurve,
const DPoint4d      *pPoles,
int                 order
)
    {
    return pCurve->InitFromDPoint4dArray (pPoles, order, order);
    }

/*---------------------------------------------------------------------------------**//**
@description Sweep a base curve along a vector until it is on a target plane.
    Optionally determine the parameter intervals at which projected curve is within the
    bounded sweep.
@param pBaseCurve IN base curve.
@parma pTargetCurve OUT curve on target plane.
@param pStartEndParams OUT array of start-end pairs for pTargetCurve segments that are within the sweep.
@param pRuleLineParams OUT array of base curve parameters at which the plane exactly passes through the rule
        line.  (i.e. the sweep direction is in the plane)
@param pSweepVector IN (non-unit) vector with complete extent of sweep.
@param pPlane IN target plane.
@returns  SUCCESS if results computed.
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspcurv_sweepToPlane
(
MSBsplineCurve const *pBaseCurve,
MSBsplineCurve  *pTargetCurve,
EmbeddedDoubleArray *pStartEndParams,
EmbeddedDoubleArray *pRuleLineParams,
DVec3dCP        pSweepVector,
DPlane3dCP      pPlane
)
    {
    static double sDuplicateParameterTol = 1.0e-7;
    DVec3d sweepVector = pSweepVector ? *pSweepVector : pPlane->normal;
    int i;


    if (bsiDVec3d_arePerpendicular (&sweepVector, &pPlane->normal))
        {
        // Just return the parameters of intersection with the plane ...
        memset (pTargetCurve, 0, sizeof (MSBsplineCurve));
        if (pRuleLineParams)
            bspcurv_curvePlaneIntersects (pRuleLineParams, &pPlane->origin, &pPlane->normal,
                    const_cast<MSBsplineCurve*>(pBaseCurve), 0.0);
        }
    else
        {
        // Project the curve points into the plane ....
        bspcurv_copyCurve (pTargetCurve, const_cast<MSBsplineCurve*>(pBaseCurve));
        if (pTargetCurve->rational)
            {
            for (i = 0; i < pTargetCurve->params.numPoles; i++)
                bsiDPoint3d_sweepWeightedToPlane (&pBaseCurve->poles[i], &pTargetCurve->poles[i], NULL, pBaseCurve->weights[i],
                                    pSweepVector, pPlane);
            }
        else
            {
            for (i = 0; i < pTargetCurve->params.numPoles; i++)
                bsiDPoint3d_sweepToPlane (&pBaseCurve->poles[i], &pTargetCurve->poles[i], NULL, pSweepVector, pPlane);
            }

        if (pStartEndParams)
            {
            double a0, a1;
            //  We need parameters (but not points) for intersection of the plane with the shifted curve.
            //  Rather than shifting the curve up, just make a plane shifted down.
            DPlane3d shiftPlane = *pPlane;
            bsiDPoint3d_subtractDVec3d (&shiftPlane.origin, &sweepVector);
            EmbeddedDoubleArray *pBaseParams = jmdlEmbeddedDoubleArray_grab ();
            EmbeddedDoubleArray *pTopParams = jmdlEmbeddedDoubleArray_grab ();
            bspcurv_curvePlaneIntersects (pBaseParams,   &pPlane->origin, &pPlane->normal, const_cast<MSBsplineCurve*>(pBaseCurve), 0.0);
            bspcurv_curvePlaneIntersects (pTopParams, &shiftPlane.origin, &shiftPlane.normal, const_cast<MSBsplineCurve*>(pBaseCurve), 0.0);

            jmdlEmbeddedDoubleArray_addDoubleArray (pBaseParams,
                        jmdlEmbeddedDoubleArray_getConstPtr (pTopParams, 0),
                        jmdlEmbeddedDoubleArray_getCount (pTopParams));

            jmdlEmbeddedDoubleArray_addDouble (pBaseParams, 0.0);
            jmdlEmbeddedDoubleArray_addDouble (pBaseParams, 1.0);
            jmdlEmbeddedDoubleArray_sort (pBaseParams);
            jmdlEmbeddedDoubleArray_getDouble (pBaseParams, &a0, 0);
            for (int i = 1; jmdlEmbeddedDoubleArray_getDouble (pBaseParams, &a1, i); i++)
                {
                if (a1 > a0 + sDuplicateParameterTol)
                    {
                    double aMid;
                    DPoint3d xyzMid;
                    double fraction;
                    if (a1 >= 1.0 - sDuplicateParameterTol)
                        a1 = 1.0;
                    aMid = 0.5 * (a0 + a1);
                    bspcurv_evaluateCurvePoint (&xyzMid, NULL, pBaseCurve, aMid);
                    if (bsiDPoint3d_sweepToPlane (&xyzMid, NULL, &fraction, pSweepVector, pPlane)
                        && 0.0 <= fraction && fraction <= 1.0)
                        {
                        jmdlEmbeddedDoubleArray_addDouble (pStartEndParams, a0);
                        jmdlEmbeddedDoubleArray_addDouble (pStartEndParams, a1);
                        }
                    a0 = a1;
                    }
                }
            jmdlEmbeddedDoubleArray_drop (pBaseParams);
            jmdlEmbeddedDoubleArray_drop (pTopParams);
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  void                bspcurv_intersectPlane
(
MSBsplineCurve  *curveP,            /* => input curve */
double          startFraction,      /* => starting fractional parameter */
double          endFraction,        /* => ending fractional parameter */
DPoint4d        *pPlaneCoff,        /* => plane coefficients */
double          absTol,             /* => absolute tolerance for double roots. */
MSBsplineCurve_AnnounceParameter F,
void            *pUserData0,
void            *pUserData1
)
    {
    BCurveSegment segment;
    DPoint4d intersectionPoints[MAX_ORDER];
    double  intersectionFractions[MAX_ORDER];
    int numIntersection;
    DRange1d globalFractionInterval = DRange1d::From (startFraction, endFraction);
    for (size_t spanIndex = 0; curveP->AdvanceToBezier (segment, spanIndex);)
        {
        bsiBezierDPoint4d_allDPlane4dIntersections (intersectionFractions, intersectionPoints, &numIntersection, MAX_ORDER,
                segment.GetPoleP (), (int)segment.GetOrder (), pPlaneCoff, 3, false);
        for (int i = 0; i < numIntersection; i++)
            {
            //double localFraction = intersectionFractions[i];
            double globalKnot = segment.FractionToKnot (intersectionFractions[i]);
            double globalFraction = curveP->KnotToFraction (globalKnot);
            if (globalFractionInterval.Contains (globalFraction))
                {
                F (curveP, startFraction, endFraction, globalFraction, pUserData0, pUserData1);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public void   cb_collectParameter
(
MSBsplineCurveP         pCurve,
double                  param0,
double                  param1,
double                  currParam,
//bvector<double>&    pParamArray,
EmbeddedDoubleArray *pParamArray,
void*                   pVoid
)
    {
    jmdlEmbeddedDoubleArray_addDouble  (pParamArray, currParam);
    //pParamArray.push_back (currParam);
    }

// eliminate params that have (a) very close to predecessor, (b) both have same sign dot product with normal.
// (these occur when a plane cut hits at a knot break)
static void FilterDuplicateIntersections (MSBsplineCurveCR curve, bvector<double> &params, DVec3dCR normal)
    {
    if (params.size () < 2)
        return;
    size_t numAccept = 1;
    static double s_duplicateParamTolerance = 1.0e-8;
    static double s_shift = 3.0e-7;
    double lastAcceptedParam = params[0];
    for (size_t i = 1; i < params.size (); i++)
        {
        bool accept = true;
        if (fabs (lastAcceptedParam - params[i]) <= s_duplicateParamTolerance)
            {
            double u0 = params[i] - s_shift;
            double u1 = params[i] + s_shift;
            DPoint3d xyz0, xyz1;
            DVec3d tangent0, tangent1;
            bspcurv_evaluateCurvePoint (&xyz0, &tangent0, &curve, u0);
            bspcurv_evaluateCurvePoint (&xyz1, &tangent1, &curve, u1);
            double dot0 = normal.DotProduct (tangent0);
            double dot1 = normal.DotProduct (tangent1);
            if (dot0 * dot1 <= 0.0)
                accept = true;
            else
                accept = false;
            }
        if (accept)
            {
            params[numAccept++] = lastAcceptedParam = params[i];
            }
        }
    params.resize (numAccept);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  StatusInt     bspcurv_curvePlaneIntersects
(
//bvector<double>&    pParamArray,
EmbeddedDoubleArray     *pParamArray,
DPoint3dCP              pOrigin,
DPoint3dCP              pNormal,
MSBsplineCurveP         pCurve,
double                  tolerance
)
    {
    DPoint3d unitNormal = *pNormal;
    DPoint4d planeCoffs;

    if (0.0 == bsiDPoint3d_normalize (&unitNormal, pNormal))
        return ERROR;
    DVec3d normal = DVec3d::From (*pNormal);  // ugh. old DPoint3d code
    bsiDPoint4d_setComponents (&planeCoffs, unitNormal.x, unitNormal.y, unitNormal.z,
                                    -bsiDPoint3d_dotProduct (&unitNormal, pOrigin));
    bspcurv_intersectPlane (pCurve, 0.0, 1.0, &planeCoffs, tolerance,
                (MSBsplineCurve_AnnounceParameter*)cb_collectParameter, pParamArray, NULL);
    FilterDuplicateIntersections (*pCurve, *pParamArray, normal);
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  void                bspcurv_intersectPlaneExt
(
MSBsplineCurve  *curveP,            /* => input curve */
double          startFraction,      /* => starting fractional parameter */
double          endFraction,        /* => ending fractional parameter */
DPoint4d        *pPlaneCoff,        /* => plane coefficients */
double          absTol,             /* => absolute tolerance for double roots. */
MSBsplineCurve_AnnounceParameter F,
void            *pUserData0,
void            *pUserData1
)
    {
    bspcurv_intersectPlane (curveP, startFraction, endFraction, pPlaneCoff, absTol, F, pUserData0, pUserData1);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             12/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprcurv_relaxToCurve
(
DPoint3d        *nearPt,               /* <= closest point on curve */
DPoint3d        *tangent,              /* <= tangent @ closest point on curve */
double          *u,                    /* <=> initial guess -> param of pt */
DPoint3d        *testPt,               /* => want closest pt to this pt */
MSBsplineCurve  *curve,
int             iterations,            /* => number of iterations to try */
double          convTol                /* => convergence tolerance in UV space */
)
    {
    int             count;
    double          mag, difference;
    DPoint3d        diff, pt;
    int             timesConverged = 0;

    difference = 0.0;
    pt = *testPt;
    /* Allow extra iterations for double check */
    iterations += 2;

    for (count=0; count < iterations; count++)
        {
        *u += difference;

        if (*u < 0.0)       *u = 0.0;
        else if (*u > 1.0)          *u = 1.0;

        bspcurv_computeCurvePoint (nearPt, tangent, NULL, *u, curve->poles,
                                   curve->params.order, curve->params.numPoles, curve->knots,
                                   curve->weights, curve->rational, curve->params.closed);

        if ((mag = bsiDPoint3d_magnitude (tangent)) < fc_epsilon)
            return STATUS_DIVERGING;

        bsiDPoint3d_subtractDPoint3dDPoint3d (&diff, &pt, nearPt);
        difference = bsiDPoint3d_dotProduct (&diff, tangent) / (mag * mag);

        if (fabs (difference) < convTol)
            {
            timesConverged++;
            if (timesConverged > 2 || count >= iterations - 1)
                    return STATUS_CONVERGED;
            }
        else
            timesConverged = 0;
        }

    /* Failed to converge within iterations */
    return STATUS_NONCONVERGED;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
