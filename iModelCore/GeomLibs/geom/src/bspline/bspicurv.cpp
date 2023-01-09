/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define MAX_DEPTH           10

/*---------------------------------------------------------------------------------**//**
* Estimate an integral by simpson's rule, and estimate the absolute value integral by 2-part trapezoid rule.
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
    int             status = SUCCESS;
    double          error, length;
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
* @bsimethod
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

    offset.DifferenceOf (point, *originP);
    if (selector == SweepArea)
        {
        *fP = 0.5 * offset.CrossProductXY (tangent);
        }
    else if (selector == SweepAngle)
        {
        double rr = offset.DotProductXY (offset);
        double tt = tangent.DotProductXY (tangent);
        /* Hmm .. how big is zero ? */
        if (rr <= squareDistTol * tt)
            return ERROR;
        *fP = offset.CrossProductXY (tangent) / rr;
        }
    else if (selector == ArcLength)
        {
        *fP = sqrt (tangent.DotProduct (tangent));
        }
    else if (selector == momentXdL)
        {
        *fP = (point.x - originP->x) * sqrt (tangent.DotProductXY (tangent));
        }
    else if (selector == momentYdL)
        {
        *fP = (point.y - originP->y) * sqrt (tangent.DotProductXY (tangent));
        }
    else if (selector == momentZdL)
        {
        *fP = (point.z - originP->z) * sqrt (tangent.DotProductXY (tangent));
        }

    return SUCCESS;

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
/*---------------------------------------------------------------------------------**//**
* @bsimethod
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


END_BENTLEY_GEOMETRY_NAMESPACE
