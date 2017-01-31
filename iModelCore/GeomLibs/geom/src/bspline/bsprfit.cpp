/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bsprfit.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/BspPrivateApi.h>
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define     CALCULATE_NONE  0
#define     CALCULATE_HIGH  1
#define     CALCULATE_BOTH  2
#define     CALCULATE_ALL   3
#define     CALCULATE_ABC   4
#define     CALCULATE_D     5
#define     CALCULATE_E     6
#define     REV_LIMIT               10

#define     DENO3           1.0/fc_3
#define     DENO9           1.0/9.0

typedef struct surffuncinterface
    {
    int                 iso;
    double              value;
    int                 (*surfFunc)();
    void                *userDataP;
    } SurfFuncInterface;

/*----------------------------------------------------------------------+
|                                                                       |
|   Curve Routines                                                      |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    04/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprfit_approxWithLinestring
(
MSBsplineCurve*                     bspline,
BSplineCallback_AskCurvePoints      curveFunc,
CallbackArgP                        userDataP,
int                                 numPoints
)
    {
    int         status;
    double      u, incr;
    DPoint3d    *pP, *endP;

    memset (bspline, 0, sizeof(*bspline));

    if (numPoints == 0)
        numPoints = 24;
    bspline->display.curveDisplay = true;
    bspline->params.order = 2;
    bspline->params.numPoles = numPoints;

    if (SUCCESS != (status = bspcurv_allocateCurve (bspline)))
        return status;

    incr = 1.0 / (numPoints - 1);
    for (pP=endP=bspline->poles, endP += numPoints, u=0.0; pP < endP; pP++, u += incr)
        if (SUCCESS != (status = (*curveFunc) (pP, NULL, NULL, u, userDataP)))
            goto wrapup;
    status = bspknot_computeKnotVector (bspline->knots, &bspline->params, NULL);

wrapup:
    if (status)
        bspcurv_freeCurve (bspline);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    06/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprfit_cubicCalculatePoles
(
DPoint3dP                       poles,
double                          rangeLo,
double                          rangeHi,
BSplineCallback_AskCurvePoints  curveFunc,
CallbackArgP                    userDataP,
int                             numPoints,
double*                         bFuncArray,
DPoint3dP                       points,
DPoint3dP                       p0,
DPoint3dP                       p1,
int                             tangentIndex,
double*                         lamda
)
    {
    int         i, status;
    double      u, incr, matrix[2][2], rhs[2], *bfuncs, span;
    DPoint3d    delV0, delVn, di, delV0B, delVnB, *pP;

    memset (matrix, 0, sizeof(matrix));
    memset (rhs, 0, sizeof(rhs));

    poles[0] = p0[0]; delV0 = p0[tangentIndex];
    poles[3] = p1[0]; delVn = p1[tangentIndex];
    points[0] = p0[0];
    points[numPoints-1] = p1[0];
    span = rangeHi - rangeLo;

    /* Construct system of equations to be solved */
    incr = span / (numPoints - 1);

    for (i=2, pP=points+1, u=rangeLo+incr, bfuncs=bFuncArray + 4;
         i < numPoints; i++, pP++, u += incr, bfuncs += 4)
        {
        /* Get curve point */
        if (SUCCESS != (status = (*curveFunc) (pP, NULL, NULL, u, userDataP)))
            return status;
        di = *pP;

        /* Subtract off the 2 known poles */
        bsiDPoint3d_addScaledDPoint3d (&di, &di, poles,   -(bfuncs[0] + bfuncs[1]));
        bsiDPoint3d_addScaledDPoint3d (&di, &di, poles+3, -(bfuncs[2] + bfuncs[3]));

        /* Calculate the scaled derivatives */
        bsiDPoint3d_scale (&delV0B, &delV0, bfuncs[1]);
        bsiDPoint3d_scale (&delVnB, &delVn, bfuncs[2]);

        /* Add to the right hand side */
        rhs[0] += bsiDPoint3d_dotProduct (&di, &delV0B);
        rhs[1] += bsiDPoint3d_dotProduct (&di, &delVnB);

        /* Add to the matrix */
        matrix[0][0] += bsiDPoint3d_dotProduct (&delV0B, &delV0B);
        matrix[0][1] += bsiDPoint3d_dotProduct (&delV0B, &delVnB);
        matrix[1][1] += bsiDPoint3d_dotProduct (&delVnB, &delVnB);
        }

    /* Reflect matrix along diagonal */
    matrix[1][0] = matrix[0][1];

    /* Solve system of equations */
    if (SUCCESS != (status = (bsiLinAlg_solveLinearGaussPartialPivot ((double*)matrix, 2, rhs, 1) ? SUCCESS : ERROR)))
        return status;

    /* Set the inner poles */
    bsiDPoint3d_addScaledDPoint3d (poles+1, poles,   &delV0, rhs[0]);
    bsiDPoint3d_addScaledDPoint3d (poles+2, poles+3, &delVn, rhs[1]);

    if (lamda)
        {
        memcpy (lamda, rhs, 2*sizeof(double));
/*      lamda[0] = fc_3 * rhs[0];
        lamda[1] = fc_3 * rhs[1];
*/
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    02/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprfit_approxWithCubicBezier
(
MSBsplineCurve*                     bezier,
double*                             error,
DPoint2dP                           range,
BSplineCallback_AskCurvePoints      curveFunc,
CallbackArgP                        userDataP,
int                                 numPoints,
double*                             bFuncArray,
DPoint3dP                           points,
DPoint3dP                           p0,
DPoint3dP                           p1,
int                                 code
)
    {
    int         status=SUCCESS;
    double      u, incr, totalError;
    DPoint3d    tmp, *pP, *endP;

    memset (bezier, 0, sizeof(*bezier));
    bezier->params.numPoles = bezier->params.order = 4;

    /* Allocate memory */
    if (SUCCESS != (status = bspcurv_allocateCurve (bezier)))
        return status;
    memset (bezier->poles, 0, bezier->params.order * sizeof(DPoint3d));
    bezier->knots[0] = bezier->knots[1] = bezier->knots[2] = bezier->knots[3] = 0.0;
    bezier->knots[4] = bezier->knots[5] = bezier->knots[6] = bezier->knots[7] = 1.0;

    if (code == CALCULATE_BOTH)
        {
        if (SUCCESS != (status = (*curveFunc) (p0, p0+1, NULL, range->x, userDataP)) ||
            SUCCESS != (status = (*curveFunc) (p1, p1+1, NULL, range->y, userDataP)))
            goto wrapup;
        }
    else if (code == CALCULATE_HIGH)
        {
        if (SUCCESS != (status = (*curveFunc) (p1, p1+1, NULL, range->y, userDataP)))
            goto wrapup;
        }

    if (SUCCESS !=
        (status = bsprfit_cubicCalculatePoles (bezier->poles, range->x, range->y,
                                               curveFunc, userDataP, numPoints,
                                               bFuncArray, points, p0, p1, 1, NULL)))
        goto wrapup;

    /* Calculate error */
    incr = 1.0 / (numPoints - 1);
    for (pP=endP=points, endP+=numPoints, totalError=u=0.0; pP < endP; pP++, u += incr)
        {
        bspcurv_computeCurvePoint (&tmp, NULL, NULL, u, bezier->poles,
                bezier->params.order, bezier->params.numPoles, bezier->knots,
                bezier->weights, bezier->rational, bezier->params.closed);
        totalError += bsiDPoint3d_distance (pP, &tmp);
#if defined (debug_approxError)
        {
        DPoint3d        line[2];
        ElementUnion    elU;

        line[0] = tmp;
        line[1] = *pP;
        create_lineD (&elU, NULL, line, NULL);
        displayElement (&elU, 0, -1L);
        }
#endif
        }

    *error = totalError / numPoints;

wrapup:
    if (status)
        bspcurv_freeCurve (bezier);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    02/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprfit_approxWithQuinticBezier
(
MSBsplineCurve*                     bezier,
double*                             error,
DPoint2dP                           range,
BSplineCallback_AskCurvePoints      curveFunc,
CallbackArgP                        userDataP,
int                                 numPoints,
double*                             bFunc3Array,
double*                             bFunc5Array,
DPoint3dP                           points,
DPoint3dP                           p0,
DPoint3dP                           p1,
int                                 code
)
    {
    int         i, status;
    double      u, incr, matrix[2][2], rhs[2], *bfuncs, totalError, span,
                l1, l2, l1Sqd, l2Sqd;
    DPoint3d    tmp, delV0, delV1, delVnm1, delVn, delV0B, delVnB, di, *pP, *endP;

    memset (matrix, 0, sizeof(matrix));
    memset (rhs, 0, sizeof(rhs));

    if (code == CALCULATE_BOTH)
        {
        if (SUCCESS != (status = (*curveFunc) (p0, p0+1, p0+2, range->x, userDataP)) ||
            SUCCESS != (status = (*curveFunc) (p1, p1+1, p1+2, range->y, userDataP)))
            goto wrapup;
        }
    else if (code == CALCULATE_HIGH)
        {
        if (SUCCESS != (status = (*curveFunc) (p1, p1+1, p1+2, range->y, userDataP)))
            goto wrapup;
        }


    if (SUCCESS !=
        (status = bsprfit_approxWithCubicBezier (bezier, error, range, curveFunc, userDataP,
                                                 numPoints, bFunc3Array, points,
                                                 p0, p1, CALCULATE_NONE)))
        return status;

    if (SUCCESS != (status = bspcurv_elevateDegree (bezier, bezier, 5)))
        return status;

    l1 = fc_5 * bsiDPoint3d_distance (bezier->poles, bezier->poles+1) / bsiDPoint3d_magnitude (p0+1);
    l2 = fc_5 * bsiDPoint3d_distance (bezier->poles+5, bezier->poles+4) / bsiDPoint3d_magnitude (p1+1);
    l1Sqd = l1 * l1;
    l2Sqd = l2 * l2;

    delV0 = p0[1];
    delVn = p1[1];
    delV1 = p0[2];
    delVnm1 = p1[2];
    span = range->y - range->x;

    /* Construct system of equations to be solved */
    incr = span / (numPoints - 1);

    /* don't reclculate first point */
    points[0] = bezier->poles[0];
    bfuncs = bFunc5Array;
    bsiDPoint3d_scale (&di, points, (double) 20);
    bsiDPoint3d_scale (&delV0B, &delV0, bfuncs[2]);
    bsiDPoint3d_scale (&delVnB, &delVn, bfuncs[3]);
    bsiDPoint3d_addScaledDPoint3d (&di, &di, bezier->poles,   -20*(bfuncs[0] + bfuncs[1] + bfuncs[2]));
    bsiDPoint3d_addScaledDPoint3d (&di, &di, bezier->poles+5, -20*(bfuncs[5] + bfuncs[4] + bfuncs[3]));
    bsiDPoint3d_addScaledDPoint3d (&di, &di, &delV0, -fc_4 * l1 *(bfuncs[1] + 2.0 * bfuncs[2]));
    bsiDPoint3d_addScaledDPoint3d (&di, &di, &delVn, -fc_4 * l2 *(bfuncs[4] + 2.0 * bfuncs[3]));
    bsiDPoint3d_addScaledDPoint3d (&di, &di, &delV1,   -l1Sqd * bfuncs[2]);
    bsiDPoint3d_addScaledDPoint3d (&di, &di, &delVnm1, -l2Sqd * bfuncs[3]);
    rhs[0] += bsiDPoint3d_dotProduct (&di, &delV0B);
    rhs[1] += bsiDPoint3d_dotProduct (&di, &delVnB);
    matrix[0][0] += bsiDPoint3d_dotProduct (&delV0B, &delV0B);
    matrix[0][1] += bsiDPoint3d_dotProduct (&delV0B, &delVnB);
    matrix[1][1] += bsiDPoint3d_dotProduct (&delVnB, &delVnB);

    for (i=2, pP=points+1, u=range->x+incr, bfuncs=bFunc5Array + bezier->params.order;
         i < numPoints; i++, pP++, u += incr, bfuncs += bezier->params.order)
        {
        /* Add to equations */
        if (SUCCESS != (status = (*curveFunc) (pP, NULL, NULL, u, userDataP)))
            goto wrapup;

        bsiDPoint3d_scale (&di, pP, (double) 20);
        bsiDPoint3d_scale (&delV0B, &delV0, bfuncs[2]);
        bsiDPoint3d_scale (&delVnB, &delVn, bfuncs[3]);
        bsiDPoint3d_addScaledDPoint3d (&di, &di, bezier->poles,   -20*(bfuncs[0] + bfuncs[1] + bfuncs[2]));
        bsiDPoint3d_addScaledDPoint3d (&di, &di, bezier->poles+5, -20*(bfuncs[5] + bfuncs[4] + bfuncs[3]));
        bsiDPoint3d_addScaledDPoint3d (&di, &di, &delV0, -fc_4 * l1 *(bfuncs[1] + 2.0 * bfuncs[2]));
        bsiDPoint3d_addScaledDPoint3d (&di, &di, &delVn, -fc_4 * l2 *(bfuncs[4] + 2.0 * bfuncs[3]));
        bsiDPoint3d_addScaledDPoint3d (&di, &di, &delV1,   -l1Sqd * bfuncs[2]);
        bsiDPoint3d_addScaledDPoint3d (&di, &di, &delVnm1, -l2Sqd * bfuncs[3]);
        rhs[0] += bsiDPoint3d_dotProduct (&di, &delV0B);
        rhs[1] += bsiDPoint3d_dotProduct (&di, &delVnB);
        matrix[0][0] += bsiDPoint3d_dotProduct (&delV0B, &delV0B);
        matrix[0][1] += bsiDPoint3d_dotProduct (&delV0B, &delVnB);
        matrix[1][1] += bsiDPoint3d_dotProduct (&delVnB, &delVnB);
        }

    /* don't reclculate last point */
    *pP = bezier->poles[5];
    bsiDPoint3d_scale (&delV0B, &delV0, bfuncs[2]);
    bsiDPoint3d_scale (&delVnB, &delVn, bfuncs[3]);
    bsiDPoint3d_scale (&di, pP, (double) 20);
    bsiDPoint3d_addScaledDPoint3d (&di, &di, bezier->poles,   -20*(bfuncs[0] + bfuncs[1] + bfuncs[2]));
    bsiDPoint3d_addScaledDPoint3d (&di, &di, bezier->poles+5, -20*(bfuncs[5] + bfuncs[4] + bfuncs[3]));
    bsiDPoint3d_addScaledDPoint3d (&di, &di, &delV0, -fc_4 * l1 *(bfuncs[1] + 2.0 * bfuncs[2]));
    bsiDPoint3d_addScaledDPoint3d (&di, &di, &delVn, -fc_4 * l2 *(bfuncs[4] + 2.0 * bfuncs[3]));
    bsiDPoint3d_addScaledDPoint3d (&di, &di, &delV1,   -l1Sqd * bfuncs[2]);
    bsiDPoint3d_addScaledDPoint3d (&di, &di, &delVnm1, -l2Sqd * bfuncs[3]);
    rhs[0] += bsiDPoint3d_dotProduct (&di, &delV0B);
    rhs[1] += bsiDPoint3d_dotProduct (&di, &delVnB);
    matrix[0][0] += bsiDPoint3d_dotProduct (&delV0B, &delV0B);
    matrix[0][1] += bsiDPoint3d_dotProduct (&delV0B, &delVnB);
    matrix[1][1] += bsiDPoint3d_dotProduct (&delVnB, &delVnB);

    matrix[1][0] = matrix[0][1];

    /* Solve system of equations */
    if (SUCCESS != (status = (bsiLinAlg_solveLinearGaussPartialPivot ((double*)matrix, 2, rhs, 1) ? SUCCESS : ERROR)))
        goto wrapup;

    bsiDPoint3d_addScaledDPoint3d (bezier->poles+1, bezier->poles,   &delV0,   l1/fc_5);
    bsiDPoint3d_addScaledDPoint3d (bezier->poles+2, bezier->poles,   &delV0,   rhs[0]/20 + 0.4*l1);
    bsiDPoint3d_addScaledDPoint3d (bezier->poles+2, bezier->poles+2, &delV1,   l1Sqd/20);
    bsiDPoint3d_addScaledDPoint3d (bezier->poles+3, bezier->poles+5, &delVn,   rhs[1]/20 + 0.4*l2);
    bsiDPoint3d_addScaledDPoint3d (bezier->poles+3, bezier->poles+3, &delVnm1, l2Sqd/20);
    bsiDPoint3d_addScaledDPoint3d (bezier->poles+4, bezier->poles+5, &delVn,   -l2/fc_5);

    /* Calculate error */
    incr = 1.0 / (numPoints - 1);
    for (pP=endP=points, endP+=numPoints, totalError=u=0.0; pP < endP; pP++, u += incr)
        {
        bspcurv_computeCurvePoint (&tmp, NULL, NULL, u, bezier->poles,
                bezier->params.order, bezier->params.numPoles, bezier->knots,
                bezier->weights, bezier->rational, bezier->params.closed);
        totalError += bsiDPoint3d_distance (pP, &tmp);
#if defined (debug_approxError)
        {
        DPoint3d        line[2];
        ElementUnion    elU;

        line[0] = tmp;
        line[1] = *pP;
        create_lineD (&elU, NULL, line, NULL);
        displayElement (&elU, 0, -1L);
        }
#endif
        }

    *error = totalError / numPoints;

wrapup:
    if (status)
        bspcurv_freeCurve (bezier);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    02/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprfit_approxCurveRecurse
(
MSBsplineCurve*                     bspline,                /* <= Bspline curve */
DPoint2dP                           range,                  /* => parametric range to approximate */
BSplineCallback_AskCurvePoints      curveFunc,
CallbackArgP                        userDataP,              /* => passed through to curveFunc */
double                              tolerance,              /* => accuracy of approximation */
int                                 numPoints,              /* => density of approx pts for error calculation */
int                                 continuity,             /* => degree of continuity btw spline pieces */
double*                             bFunc3Array,
double*                             bFunc5Array,
DPoint3dP                           pointsArray,
DPoint3dP                           p0,
DPoint3dP                           p1,
int                                 code,
int                                 depth
)
    {
    int                 status;
    double              error;
    DPoint2d            range0, range1;
    DPoint3d            pMid[3];
    MSBsplineCurve      curve0, curve1;

    if (depth > REV_LIMIT)
        return ERROR;

    memset (bspline, 0, sizeof(*bspline));
    memset (&curve0, 0, sizeof(curve0));
    memset (&curve1, 0, sizeof(curve1));

    range0.x = range1.x = range->x;
    range0.y = range1.y = range->y;
    if (continuity == TANGENT_CONTINUITY)
        {
        if (SUCCESS !=
            (status = bsprfit_approxWithCubicBezier (bspline, &error, &range0, curveFunc,
                                                     userDataP, numPoints, bFunc3Array,
                                                     pointsArray, p0, p1, code)))
            goto wrapup;
        }
    else
        {
        if (SUCCESS !=
            (status = bsprfit_approxWithQuinticBezier (bspline, &error, &range0,
                                                       curveFunc, userDataP, numPoints,
                                                       bFunc3Array, bFunc5Array, pointsArray,
                                                       p0, p1, code)))
            goto wrapup;
        }

    if (error < tolerance)
        {
        bspline->display.curveDisplay = true;
        return SUCCESS;
        }

    bspcurv_freeCurve (bspline);

    /* Subdivide range and approximate in halves */
    range0.y = range1.x = (range0.x + range1.y) / 2.0;
    if (SUCCESS != (status = bsprfit_approxCurveRecurse (&curve0, &range0, curveFunc,
                                     userDataP, tolerance, numPoints, continuity,
                                     bFunc3Array, bFunc5Array, pointsArray,
                                     p0, pMid, CALCULATE_HIGH, depth+1)) ||
        SUCCESS != (status = bsprfit_approxCurveRecurse (&curve1, &range1, curveFunc,
                                     userDataP, tolerance, numPoints, continuity,
                                     bFunc3Array, bFunc5Array, pointsArray,
                                     pMid, p1, CALCULATE_NONE, depth+1)))
        goto wrapup;

    status = bspcurv_combineCurves (bspline, &curve0, &curve1, true, false);

wrapup:
    bspcurv_freeCurve (&curve0);
    bspcurv_freeCurve (&curve1);
    if (status)
        bspcurv_freeCurve (bspline);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* curveFunc : User supplied function to calculate curve position & tangent at a parameter
* @bsimethod                                                    Brian.Peters    02/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprfit_approximateCurve
(
MSBsplineCurve*                     bspline,                /* <= Bspline curve */
BSplineCallback_AskCurvePoints      curveFunc,              /* => curv function */
CallbackArgP                        userDataP,              /* => passed through to curveFunc */
double                              tolerance,              /* => accuracy of approximation */
int                                 density,                /* => density of approx pts for error calculation */
int                                 continuity              /* => degree of continuity btw spline pieces */
)
    {
    int         i, status, numPoints, left, order;
    double      *bP, *endP, *bFunc3Array, *bFunc5Array, knots[2*MAX_ORDER], u, incr;
    DPoint2d    range;
    DPoint3d    p0[3], p1[3], *points;

    if (continuity == TANGENT_CONTINUITY)
        {
        numPoints = density > 6 ? density : 6;
        order = 4;
        }
    else if (continuity == CURVATURE_CONTINUITY)
        {
        numPoints = density > 10 ? density : 10;
        order = 5;
        }
    else if (continuity == POSITION_CONTINUITY)
        return bsprfit_approxWithLinestring (bspline, curveFunc, userDataP, density);
    else
        return ERROR;

    if (tolerance < fc_1em15)
        return ERROR;


    range.x = 0.0;
    range.y = 1.0;



    /* Can save considerable time in the recursion by avoiding repeated calculation of
        the b-spline blending functions. Also allocate points scratch buffer here. */
    ScopedArray<double> scopedFunc3Array (numPoints * 4);          bFunc3Array = scopedFunc3Array.GetData ();
    ScopedArray<double> scopedFunc5Array (numPoints * 6);          bFunc5Array = scopedFunc5Array.GetData ();
    ScopedArray<DPoint3d> scopedPoints     (numPoints);              points = scopedPoints.GetData ();        
        

    memset (knots, 0, 4 * sizeof(double));
    for (bP=endP=knots+4, endP+=4; bP < endP; bP++)
        *bP = 1.0;

    incr = 1.0 / (numPoints - 1);
    for (bP=bFunc3Array, i=0, u=0.0; i < numPoints; bP += order, i++, u += incr)
        bsputil_computeBlendingFuncs (bP, NULL, &left, knots, u, 1.0, 4, false);
    if (continuity == CURVATURE_CONTINUITY)
        {
        memset (knots, 0, 6 * sizeof(double));
        for (bP=endP=knots+6, endP+=6; bP < endP; bP++)
            *bP = 1.0;
        for (bP=bFunc5Array, i=0, u=0.0; i < numPoints; bP += 6, i++, u += incr)
            bsputil_computeBlendingFuncs (bP, NULL, &left, knots, u, 1.0, 6, false);
        }

    status = bsprfit_approxCurveRecurse (bspline, &range, curveFunc, userDataP,
                                         tolerance, numPoints, continuity,
                                         bFunc3Array, bFunc5Array, points,
                                         p0, p1, CALCULATE_BOTH, 0);
    bspline->display.curveDisplay = true;


    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   Surface Routines                                                    |
|                                                                       |
+----------------------------------------------------------------------*/
#if defined (not_used)
/*---------------------------------------------------------------------------------**//**
* Recover the information needed to approximate an iso-curve from the user-supplied surfaceFunc
* @bsimethod                                                    Brian.Peters    06/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     surfFuncInterface
(
DPoint3d            *pt,
DPoint3d            *dPt,
DPoint3d            *ddPt,
double              uv,
SurfFuncInterface   *interfaceP
)
    {
    if (interfaceP->iso == BSSURF_V)
        return ((*(interfaceP->surfFunc)) (pt, dPt, NULL, ddPt, NULL, NULL,
                                           uv, interfaceP->value, interfaceP->userDataP));
    else
        {
        return ((*(interfaceP->surfFunc)) (pt, NULL, dPt, NULL, ddPt, NULL,
                                           interfaceP->value, uv, interfaceP->userDataP));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    06/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bsputil_completeScaledQuadrilateral
(
DPoint3d        *d,
DPoint3d        *a,
DPoint3d        *b,
DPoint3d        *c,
double          scale
)
    {
    DPoint3d    tmp;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&tmp, b, a);
    bsiDPoint3d_addScaledDPoint3d (d, a, &tmp, scale);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&tmp, c, a);
    bsiDPoint3d_addScaledDPoint3d (d, d, &tmp, scale);
    }
#endif

#if defined (twist_in_tangent_plane)
/*---------------------------------------------------------------------------------**//**
* Break with Hoschek algorithm and assign inner poles as linear combinations of the tangent vectors in the corners
* @bsimethod                                                    Brian.Peters    06/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprfit_calculateInnerPoles
(
DPoint3d        *poles,
DPoint2d        *rangeLo,
DPoint2d        *rangeHi,
int             (*surfaceFunc)(),
void            *userDataP,
int             numPoints,
double          *ubFuncArray,
double          *vbFuncArray,
DPoint3d        *points,
DPoint3d        *p00,
DPoint3d        *p01,
DPoint3d        *p10,
DPoint3d        *p11
)
    {
    int         i, j, k, l, status;
    double      u, v, uIncr, vIncr, uSpan, vSpan, matrix[8][8], rhs[8], *ubf, *vbf,
                prod11, prod12, prod21, prod22;
    DPoint3d    puv, dkl, partial[8], *rowP, *pP, *endP;

    memset (matrix, 0, sizeof(matrix));
    memset (rhs, 0, sizeof(rhs));

    uSpan = rangeHi->x - rangeLo->x;
    vSpan = rangeHi->y - rangeLo->y;

    /* Construct system of equations to be solved */
    uIncr = uSpan / (numPoints - 1);
    vIncr = vSpan / (numPoints - 1);

    ubf = ubFuncArray;
    vbf = vbFuncArray;
    for (l=2, v=rangeLo->y+vIncr, vbf=vbFuncArray+4, rowP=points+numPoints+1; l < numPoints;
         l++, v += vIncr, vbf += 4, rowP += numPoints)
    for (k=2, u=rangeLo->x+uIncr, ubf=ubFuncArray+4, pP=rowP; k < numPoints;
         k++, u += uIncr, ubf += 4, pP++)
        {
        /* Get surface point */
        if (SUCCESS !=
            (status = (*surfaceFunc) (pP, NULL, NULL, NULL, NULL, NULL, u, v, userDataP)))
            return status;
        dkl = *pP;

        /* Subtract off the 12 known poles */
        for (j=0; j < 4; j++)
            {
            bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+j,    -ubf[j]*vbf[0]);
            bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+12+j, -ubf[j]*vbf[3]);
            }
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+4,  -ubf[0]*vbf[1]);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+8,  -ubf[0]*vbf[2]);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+7,  -ubf[3]*vbf[1]);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+11, -ubf[3]*vbf[2]);

        prod11 = ubf[1]*vbf[1];
        prod12 = ubf[1]*vbf[2];
        prod21 = ubf[2]*vbf[1];
        prod22 = ubf[2]*vbf[2];

        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles,    -prod11);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+12, -prod12);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+3,  -prod21);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+15, -prod22);

        /* Calculate the scaled partials */
        bsiDPoint3d_scale (partial,   p00+2, prod11);
        bsiDPoint3d_scale (partial+1, p01+2, prod12);
        bsiDPoint3d_scale (partial+2, p00+1, prod11);
        bsiDPoint3d_scale (partial+3, p10+1, prod21);
        bsiDPoint3d_scale (partial+4, p10+2, prod21);
        bsiDPoint3d_scale (partial+5, p11+2, prod22);
        bsiDPoint3d_scale (partial+6, p01+1, prod12);
        bsiDPoint3d_scale (partial+7, p11+1, prod22);

        /* Add to the right hand side */
        for (j=0; j < 8; j++)
            rhs[j] += bsiDPoint3d_dotProduct (&dkl, partial+j);

        /* Add to the matrix */
        for (j=0; j < 8; j++)
            for (i=7; i >= j; i--)
                matrix[j][i] += bsiDPoint3d_dotProduct (partial+j, partial+i);
        }

    /* Reflect matrix along diagonal */
    for (j=7; j >= 0; j--)
        for (i=0; i < j; i++)
            matrix[j][i] = matrix[i][j];

    /* Solve system of equations */
    if (SUCCESS != (status = (bsiLinAlg_solveLinearGaussPartialPivot (matrix, 8, rhs, 1) ? SUCCESS : ERROR)))
        return status;

    /* Set the inner poles */
    bsiDPoint3d_addScaledDPoint3d (poles+5,  poles,    p00+2, rhs[0]);
    bsiDPoint3d_addScaledDPoint3d (poles+5,  poles+5,  p00+1, rhs[2]);
    bsiDPoint3d_addScaledDPoint3d (poles+9,  poles+12, p01+2, rhs[1]);
    bsiDPoint3d_addScaledDPoint3d (poles+9,  poles+9,  p01+1, rhs[6]);
    bsiDPoint3d_addScaledDPoint3d (poles+6,  poles+3,  p10+2, rhs[4]);
    bsiDPoint3d_addScaledDPoint3d (poles+6,  poles+6,  p10+1, rhs[3]);
    bsiDPoint3d_addScaledDPoint3d (poles+10, poles+15, p11+2, rhs[5]);
    bsiDPoint3d_addScaledDPoint3d (poles+10, poles+10, p11+1, rhs[7]);

    return status;
    }
#elif defined (omegas_directly)
/*---------------------------------------------------------------------------------**//**
* Break with Hoschek algorithm and calculate for the poles int terms of the omega's.
* @bsimethod                                                    Brian.Peters    06/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprfit_calculateInnerPoles
(
DPoint3d        *poles,
DPoint2d        *rangeLo,
DPoint2d        *rangeHi,
int             (*surfaceFunc)(),
void            *userDataP,
int             numPoints,
double          *ubFuncArray,
double          *vbFuncArray,
double          *lamda,
DPoint3d        *points,
DPoint3d        *p00,
DPoint3d        *p01,
DPoint3d        *p10,
DPoint3d        *p11
)
    {
    int         i, j, k, l, status;
    double      u, v, uIncr, vIncr, uSpan, vSpan, matrix[4][4], rhs[4], *ubf, *vbf,
                prod11, prod12, prod21, prod22, factor, lamda2[8], *lP, *l2P, *endD,
                over9l1ml3, over9l2ml7, over9l5ml4, over9l6ml8;
    DPoint3d    puv, dkl, deriv[4], partial[8], *rowP, *pP, *endP;

    memset (matrix, 0, sizeof(matrix));
    memset (rhs, 0, sizeof(rhs));

    uSpan = rangeHi->x - rangeLo->x;
    vSpan = rangeHi->y - rangeLo->y;

    /* Construct system of equations to be solved */
    uIncr = uSpan / (numPoints - 1);
    vIncr = vSpan / (numPoints - 1);

    for (lP=endD=lamda, l2P=lamda2, endD+=8; lP < endD; lP++, l2P++)
        *l2P = fc_3 * *lP * *lP;

    /* Pre-calculate some common expressions to save time */
    over9l1ml3 = 1.0 / 9.0 / (lamda[0]-lamda[2]);
    over9l2ml7 = 1.0 / 9.0 / (lamda[1]-lamda[6]);
    over9l5ml4 = 1.0 / 9.0 / (lamda[4]-lamda[3]);
    over9l6ml8 = 1.0 / 9.0 / (lamda[5]-lamda[7]);

    ubf = ubFuncArray;
    vbf = vbFuncArray;
    for (l=2, v=rangeLo->y+vIncr, vbf=vbFuncArray+4, rowP=points+numPoints+1; l < numPoints;
         l++, v += vIncr, vbf += 4, rowP += numPoints)
    for (k=2, u=rangeLo->x+uIncr, ubf=ubFuncArray+4, pP=rowP; k < numPoints;
         k++, u += uIncr, ubf += 4, pP++)
        {
        /* Get surface point */
        if (SUCCESS !=
            (status = (*surfaceFunc) (pP, NULL, NULL, NULL, NULL, NULL, u, v, userDataP)))
            return status;
        dkl = *pP;

        /* Subtract off the 12 known poles */
        for (j=0; j < 4; j++)
            {
            bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+j,    -ubf[j]*vbf[0]);
            bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+12+j, -ubf[j]*vbf[3]);
            }
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+4,  -ubf[0]*vbf[1]);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+8,  -ubf[0]*vbf[2]);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+7,  -ubf[3]*vbf[1]);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+11, -ubf[3]*vbf[2]);

        /* Subtract off parts of inner poles that do not depend on the omegas */
        prod11 = ubf[1]*vbf[1];
        prod12 = ubf[1]*vbf[2];
        prod21 = ubf[2]*vbf[1];
        prod22 = ubf[2]*vbf[2];

        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, p00, -prod11);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, p01, -prod12);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, p10, -prod21);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, p11, -prod22);

        prod11 *= over9l1ml3;
        prod12 *= over9l2ml7;
        prod21 *= over9l5ml4;
        prod22 *= over9l6ml8;

        factor = lamda2[0] - 2.0*lamda[0]*lamda[2];
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, p00+2, -factor*prod11);
        factor = lamda2[2] - 2.0*lamda[0]*lamda[2];
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, p00+1,  factor*prod11);

        factor = lamda2[1] - 2.0*lamda[1]*lamda[6];
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, p01+2, -factor*prod12);
        factor = lamda2[6] - fc_4*lamda[1]*lamda[6];
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, p01+1,  factor*prod12);

        factor = lamda2[4] - fc_4*lamda[3]*lamda[4];
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, p10+2, -factor*prod21);
        factor = lamda2[3] - 2.0*lamda[3]*lamda[4];
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, p10+1,  factor*prod21);

        factor = lamda2[5] - fc_4*lamda[5]*lamda[7];
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, p11+2, -factor*prod22);
        factor = lamda2[7] - fc_4*lamda[5]*lamda[7];
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, p11+1,  factor*prod22);

        /* Calculate the scaled partials */
        bsiDPoint3d_scale (partial,   p00+1,  lamda[0]*prod11);
        bsiDPoint3d_scale (partial+1, p01+1, -lamda[1]*prod12);
        bsiDPoint3d_scale (partial+2, p00+2, -lamda[2]*prod11);
        bsiDPoint3d_scale (partial+3, p10+2,  lamda[3]*prod21);
        bsiDPoint3d_scale (partial+4, p10+1,  lamda[4]*prod21);
        bsiDPoint3d_scale (partial+5, p11+1, -lamda[5]*prod22);
        bsiDPoint3d_scale (partial+6, p01+2, -lamda[6]*prod12);
        bsiDPoint3d_scale (partial+7, p11+2,  lamda[7]*prod22);

        /* Add to the right hand side */
        for (j=i=0; j < 4; j++, i+=2)
            {
            bsiDPoint3d_addDPoint3dDPoint3d (deriv+j, partial+i, partial+i+1);
            rhs[j] += bsiDPoint3d_dotProduct (&dkl, deriv+j);
            }

        /* Add to the matrix */
        for (j=0; j < 4; j++)
            for (i=3; i >= j; i--)
                matrix[j][i] += bsiDPoint3d_dotProduct (deriv+j, deriv+i);
        }

    /* Reflect matrix along diagonal */
    for (j=3; j >= 0; j--)
        for (i=0; i < j; i++)
            matrix[j][i] = matrix[i][j];

    /* Solve system of equations */
    if (SUCCESS != (status = (bsiLinAlg_solveLinearGaussPartialPivot (matrix, 4, rhs, 1) ? SUCCESS : ERROR)))
        return status;

    /* Set the inner poles */
    bsiDPoint3d_scale (poles+5, p00,       1.0/over9l1ml3);
    factor = lamda2[0]-2.0*lamda[0]*lamda[2]-lamda[2]*rhs[1];
    bsiDPoint3d_addScaledDPoint3d (poles+5,  poles+5,   p00+2, factor);
    factor = lamda2[2]-2.0*lamda[0]*lamda[2]-lamda[0]*rhs[0];
    bsiDPoint3d_addScaledDPoint3d (poles+5,  poles+5,   p00+1, -factor);
    bsiDPoint3d_scale (poles+5, poles+5,   over9l1ml3);

    bsiDPoint3d_scale (poles+9, p01,          1.0/over9l2ml7);
    factor = lamda2[1]-2.0*lamda[1]*lamda[6]-lamda[6]*rhs[3];
    bsiDPoint3d_addScaledDPoint3d (poles+9,  poles+9,   p01+2, factor);
    factor = lamda2[6]-fc_4*lamda[1]*lamda[6]+lamda[1]*rhs[0];
    bsiDPoint3d_addScaledDPoint3d (poles+9,  poles+9,   p01+1, -factor);
    bsiDPoint3d_scale (poles+9, poles+9,   over9l2ml7);

    bsiDPoint3d_scale (poles+6, p10,          1.0/over9l5ml4);
    factor = lamda2[4]-fc_4*lamda[3]*lamda[4]+lamda[3]*rhs[1];
    bsiDPoint3d_addScaledDPoint3d (poles+6,  poles+6,   p10+2, factor);
    factor = lamda2[3]-2.0*lamda[3]*lamda[4]-lamda[4]*rhs[2];
    bsiDPoint3d_addScaledDPoint3d (poles+6,  poles+6,   p10+1, -factor);
    bsiDPoint3d_scale (poles+6, poles+6,   over9l5ml4);

    bsiDPoint3d_scale (poles+10, p11,      1.0/over9l6ml8);
    factor = lamda2[5]-fc_4*lamda[5]*lamda[7]+lamda[7]*rhs[3];
    bsiDPoint3d_addScaledDPoint3d (poles+10,  poles+10, p11+2, factor);
    factor = lamda2[7]-fc_4*lamda[5]*lamda[7]+lamda[5]*rhs[2];
    bsiDPoint3d_addScaledDPoint3d (poles+10,  poles+10, p11+1, -factor);
    bsiDPoint3d_scale (poles+10, poles+10, over9l6ml8);

    return status;
    }
#elif defined (not_used)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    06/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprfit_calculateInnerPoles
(
DPoint3d        *poles,
DPoint2d        *rangeLo,
DPoint2d        *rangeHi,
int             (*surfaceFunc)(),
void            *userDataP,
int             numPoints,
double          *ubFuncArray,
double          *vbFuncArray,
double          *lamdaIn,
DPoint3d        *points,
DPoint3d        *p00,
DPoint3d        *p01,
DPoint3d        *p10,
DPoint3d        *p11
)
    {
    int         i, j, k, l, status;
    double      u, v, uIncr, vIncr, uSpan, vSpan, matrix[4][4], rhs[4], *ubf, *vbf,
                prod11, prod12, prod21, prod22, *lamda,
                b02, b12, b22, b03, b13, b23, b33,
                a_e, b_e, c_e, a_f, b_f, c_f, a_a, b_b, c_c, a_b, a_c, b_c,
                c1, c2, c3, c4, denom, tmp1, tmp2, tmp3;
    DPoint3d    n[5], q[5], a, b, c, e, f, p05[3], p50[3], p15[3], p51[3],
                derivW11, derivOmega1, dkl, *rowP, *pP;

    memset (matrix, 0, sizeof(matrix));
    memset (rhs, 0, sizeof(rhs));

    /* Choose the free point (u0, v0) as (0.5, 0.5). Then the bfuncs are as follows
        for quadratic and cubic in both u and v directions. */
    b02 = b22 = 0.25;    b12 = 0.5;
    b03 = b33 = 0.125;   b13 = b23 = 0.375;

    /* Need some data from the surfaceFunc. */
    /* p05 = (0.0, 0.5) and ddU, ddV */
    /* p15 = (1.0, 0.5) and ddU, ddV */
    /* p50 = (0.5, 0.0) and ddU, ddV */
    /* p51 = (0.5, 1.0) and ddU, ddV */

    uSpan = rangeHi->x - rangeLo->x;
    vSpan = rangeHi->y - rangeLo->y;

    u = rangeLo->x;    v = rangeLo->y + 0.5 * vSpan;
    if (SUCCESS !=
        (status = (*surfaceFunc) (p05, p05+1, p05+2, NULL, NULL, NULL, u, v, userDataP)))
        return status;
    u = rangeHi->x;
    if (SUCCESS !=
        (status = (*surfaceFunc) (p15, p15+1, p15+2, NULL, NULL, NULL, u, v, userDataP)))
        return status;
    u = rangeLo->x + 0.5 * uSpan;    v = rangeLo->y;
    if (SUCCESS !=
        (status = (*surfaceFunc) (p50, p50+1, p50+2, NULL, NULL, NULL, u, v, userDataP)))
        return status;
    v = rangeHi->y;
    if (SUCCESS !=
        (status = (*surfaceFunc) (p51, p51+1, p51+2, NULL, NULL, NULL, u, v, userDataP)))
        return status;

    /* Calculate the n[i] and q[i] as per Hoschek's paper. Notice that the zero-th
        entry is not used in arrarys so that the indices match the paper. */
    lamda = lamdaIn - 1;

    bsiDPoint3d_scale (n+1, p05+1, b12);
    bsiDPoint3d_addScaledDPoint3d (q+1, p05, p05+1, lamda[3]*b02+lamda[7]*b22);
    bsiDPoint3d_addScaledDPoint3d (q+1, q+1, poles+1,  -b03);
    bsiDPoint3d_addScaledDPoint3d (q+1, q+1, poles+13, -b33);

    bsiDPoint3d_scale (n+2, p50+2, b12);
    bsiDPoint3d_addScaledDPoint3d (q+2, p50, p50+2, lamda[1]*b02+lamda[5]*b22);
    bsiDPoint3d_addScaledDPoint3d (q+2, q+2, poles+4,  -b03);
    bsiDPoint3d_addScaledDPoint3d (q+2, q+2, poles+7,  -b33);

    bsiDPoint3d_scale (n+3, p15+1, b12);
    bsiDPoint3d_addScaledDPoint3d (q+3, p15, p15+1, lamda[4]*b02+lamda[8]*b22);
    bsiDPoint3d_addScaledDPoint3d (q+3, q+3, poles+2,  -b03);
    bsiDPoint3d_addScaledDPoint3d (q+3, q+3, poles+14, -b33);

    bsiDPoint3d_scale (n+4, p51+2, b12);
    bsiDPoint3d_addScaledDPoint3d (q+4, p51, p51+2, lamda[2]*b02+lamda[6]*b22);
    bsiDPoint3d_addScaledDPoint3d (q+4, q+4, poles+8,  -b03);
    bsiDPoint3d_addScaledDPoint3d (q+4, q+4, poles+11, -b33);

    /* Calculate the vectors a,b,c,e,f and their dot products to make subsequent
        calculations easier. */
    bsiDPoint3d_scale (&a, n+2,  b13/fc_3);
    bsiDPoint3d_scale (&b, n+3, -b23/fc_3);
    bsiDPoint3d_scale (&c, n+4,  b23/fc_3);
    bsiDPoint3d_scale (&e, n+1,  b13/fc_3);

    bsiDPoint3d_scale (&f, q+1,     b13);
    bsiDPoint3d_addScaledDPoint3d (&f, &f, q+2, -b13);
    bsiDPoint3d_addScaledDPoint3d (&f, &f, q+3,  b23);
    bsiDPoint3d_addScaledDPoint3d (&f, &f, q+4, -b23);

    a_e = bsiDPoint3d_dotProduct (&a, &e);
    b_e = bsiDPoint3d_dotProduct (&b, &e);
    c_e = bsiDPoint3d_dotProduct (&c, &e);
    a_f = bsiDPoint3d_dotProduct (&a, &f);
    b_f = bsiDPoint3d_dotProduct (&b, &f);
    c_f = bsiDPoint3d_dotProduct (&c, &f);
    a_a = bsiDPoint3d_dotProduct (&a, &a);
    b_b = bsiDPoint3d_dotProduct (&b, &b);
    c_c = bsiDPoint3d_dotProduct (&c, &c);
    a_b = bsiDPoint3d_dotProduct (&a, &b);
    a_c = bsiDPoint3d_dotProduct (&a, &c);
    b_c = bsiDPoint3d_dotProduct (&b, &c);

    /* Calculate the constants c1,c2,c3,c4 to make subsequent calculations easier:
        omega[2] = omega[1]*c1 + c2,    omega[3] = omega[1]*c3 + c4 */
    tmp1 = b_b*c_c - b_c*b_c;
    tmp2 = a_b*c_c - b_c*a_c;
    tmp3 = a_b*b_c - b_b*a_c;
    denom = a_a*tmp1 - a_b*tmp2 + a_c*tmp3;

    c1 = (a_e*tmp1 - b_e*tmp2 + c_e*tmp3) / denom;
    c2 = (a_f*tmp1 - b_f*tmp2 + c_f*tmp3) / denom;

    tmp1 = tmp2;
    tmp2 = a_a*c_c - a_c*a_c;
    tmp2 = a_a*b_c - a_c*a_b;

    c3 = -(a_e*tmp1 - b_e*tmp2 + c_e*tmp3) / denom;
    c4 = -(a_f*tmp1 - b_f*tmp2 + c_f*tmp3) / denom;

    /* Calculate the error system to solve for the unknowns. */
    uIncr = uSpan / (numPoints - 1);
    vIncr = vSpan / (numPoints - 1);

    ubf = ubFuncArray;
    vbf = vbFuncArray;
    for (l=2, v=rangeLo->y+vIncr, vbf=vbFuncArray+4, rowP=points+numPoints+1; l < numPoints;
         l++, v += vIncr, vbf += 4, rowP += numPoints)
    for (k=2, u=rangeLo->x+uIncr, ubf=ubFuncArray+4, pP=rowP; k < numPoints;
         k++, u += uIncr, ubf += 4, pP++)
        {
        /* Get surface point */
        if (SUCCESS !=
            (status = (*surfaceFunc) (pP, NULL, NULL, NULL, NULL, NULL, u, v, userDataP)))
            return status;
        dkl = *pP;

        /* Subtract off the 12 known poles */
        for (j=0; j < 4; j++)
            {
            bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+j,    -ubf[j]*vbf[0]);
            bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+12+j, -ubf[j]*vbf[3]);
            }
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+4,  -ubf[0]*vbf[1]);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+8,  -ubf[0]*vbf[2]);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+7,  -ubf[3]*vbf[1]);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, poles+11, -ubf[3]*vbf[2]);

        /* Subtract off parts of inner poles that do not depend on W11 or omega[1] */
        prod11 = ubf[1]*vbf[1];
        prod12 = ubf[1]*vbf[2];
        prod21 = ubf[2]*vbf[1];
        prod22 = ubf[2]*vbf[2];

        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, q+1, -prod12/b23);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, q+2, -prod21/b23);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, n+2, -c2*prod21/(fc_3*b23));
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, q+3, -prod22/b23);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, q+2,  prod22/b23);
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, n+3, -c4*prod22/(fc_3*b23));
        bsiDPoint3d_addScaledDPoint3d (&dkl, &dkl, n+2,  c2*prod22/(fc_3*b23));

        /* Calculate the partials w.r.t. W11 and omega[1] */
        derivW11.x = derivW11.y = derivW11.z = prod11-prod12-prod21+prod22;

        bsiDPoint3d_scale (&derivOmega1, n+1, prod12/(fc_3*b23));
        bsiDPoint3d_addScaledDPoint3d (&derivOmega1, &derivOmega1, n+2, c1*(prod21-prod22)/(fc_3*b23));
        bsiDPoint3d_addScaledDPoint3d (&derivOmega1, &derivOmega1, n+3, c3*prod22/(fc_3*b23));

        /* Add to the right hand side */
        rhs[0] += dkl.x * derivW11.x;
        rhs[1] += dkl.y * derivW11.y;
        rhs[2] += dkl.z * derivW11.z;
        rhs[3] += bsiDPoint3d_dotProduct (&dkl, &derivOmega1);

        /* Add to the matrix */
        tmp1 = derivW11.x * derivW11.x;
        matrix[0][0] += tmp1;
        matrix[1][1] += tmp1;
        matrix[2][2] += tmp1;
        matrix[0][3] += derivW11.x * derivOmega1.x;
        matrix[1][3] += derivW11.y * derivOmega1.y;
        matrix[2][3] += derivW11.z * derivOmega1.z;
        matrix[3][3] += bsiDPoint3d_dotProduct (&derivOmega1, &derivOmega1);
        }

    /* Reflect matrix along diagonal */
    for (j=3; j >= 0; j--)
        for (i=0; i < j; i++)
            matrix[j][i] = matrix[i][j];

    /* Solve system of equations */
    if (SUCCESS != (status = (bsiLinAlg_solveLinearGaussPartialPivot (matrix, 4, rhs, 1) ? SUCCESS : ERROR)))
        return status;

    /* Set the inner poles */
    poles[5].x = rhs[0];
    poles[5].y = rhs[1];
    poles[5].z = rhs[2];

    bsiDPoint3d_addScaledDPoint3d (poles+9, q+1, poles+5, -b13);
    bsiDPoint3d_addScaledDPoint3d (poles+9, poles+9, n+1, rhs[3]/fc_3);
    bsiDPoint3d_scale (poles+9, poles+9, 1.0/b23);

    bsiDPoint3d_addScaledDPoint3d (poles+6, q+2, poles+5, -b13);
    bsiDPoint3d_addScaledDPoint3d (poles+6, poles+6, n+2, (rhs[3]*c1+c2)/fc_3);
    bsiDPoint3d_scale (poles+6, poles+6, 1.0/b23);

    bsiDPoint3d_addScaledDPoint3d (poles+10, q+3, poles+6, -b13);
    bsiDPoint3d_addScaledDPoint3d (poles+10, poles+10, n+3, (rhs[3]*c3+c4)/fc_3);
    bsiDPoint3d_scale (poles+10, poles+10, 1.0/b23);

    return status;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          02/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bsprfit_magnifyDerivatives
(
double              *ratiox,
double              *ratioy,
double              *ratioxy,
DRange2d           *range,
DPoint3d            *offP00,
DPoint3d            *offP01,
DPoint3d            *offP10,
void                *userData,
int                 isOffset
)
    {
    DPoint3d        tmp00, tmp01, tmp10;
    double          dist0, dist1;

    if (!(isOffset && userData))
        {
        *ratiox = *ratioy = *ratioxy = 1.0;
        }
    else
        {
        bspsurf_evaluateSurfacePoint (&tmp00, NULL, NULL, NULL,
                                  range->low.x, range->low.y,
                                  ((LiftEdgeData *)userData)->surface);
        bspsurf_evaluateSurfacePoint (&tmp01, NULL, NULL, NULL,
                                  range->low.x, range->high.y,
                                  ((LiftEdgeData *)userData)->surface);
        bspsurf_evaluateSurfacePoint (&tmp10, NULL, NULL, NULL,
                                  range->high.x, range->low.y,
                                  ((LiftEdgeData *)userData)->surface);

        dist0 = bsiDPoint3d_distance (&tmp00, &tmp10);
        dist1 = bsiDPoint3d_distance (&tmp00, &tmp01);

        if (dist0 < fc_epsilon && dist1 < fc_epsilon)
            {
            *ratiox = *ratioy = *ratioxy = 1.0;
            }
        else
            {
            if (dist0 > fc_epsilon && dist1 < fc_epsilon)
                {
                *ratiox = bsiDPoint3d_distance (offP00, offP10)/dist0;
                *ratioy = 1.0;
                }
            else if (dist0 < fc_epsilon && dist1 > fc_epsilon)
                {
                *ratiox = 1.0;
                *ratioy = bsiDPoint3d_distance (offP00, offP01)/dist1;
                }
            else
                {
                *ratiox = bsiDPoint3d_distance (offP00, offP10)/dist0;
                *ratioy = bsiDPoint3d_distance (offP00, offP01)/dist1;
                }
            *ratioxy = *ratiox * *ratioy;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          02/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprfit_approxWithBiHermite
(
MSBsplineSurface*                   patch,          /* <= returned patch */
double*                             error,          /* <=> error of approximation */
DRange2d*                           range,          /* => uv range of this sub-patch */
BSplineCallback_AskSurfacePoints    surfaceFunc,    /* => supplied function to evaluate surface */
CallbackArgP                        userDataP,      /* => data passed through to surfaceFunc */
int                                 numPoints,      /* => to use to calculate error */
double*                             bFuncArray,     /* => precalculated blending funcs */
DPoint3dP                           points,         /* => from surfaceFunc */
DPoint3dP                           p00,            /* => the corners of this sub-patch */
DPoint3dP                           p01,            /*    These may already be calculated */
DPoint3dP                           p10,            /*    depending on the value of 'code' */
DPoint3dP                           p11,
int                                 code,           /* => indicates which corners are already known */
int                                 isOffset        /* => True for offset surface */
)
    {
    int                 i, j, status;
    double              u, v, uIncr, vIncr, maxError, tmpError,
                        diffx, diffy, factor_3x, factor_3y, factor_9,
                        ratiox, ratioy, ratioxy;
    DPoint3d            tmp0, *pP, tmp00[3], tmp01[3], tmp10[3], tmp11[3];

    /* Malloc the surface */
    memset (patch, 0, sizeof(*patch));
    patch->uParams.order = patch->uParams.numPoles =
    patch->vParams.order = patch->vParams.numPoles = 4;
    patch->uParams.numKnots = patch->vParams.numKnots = 0;
    if (SUCCESS != (status = bspsurf_allocateSurface (patch)))
        return status;

    /* Assign the knot vectors */
    memset (patch->uKnots, 0, patch->uParams.order * sizeof(double));
    memset (patch->vKnots, 0, patch->vParams.order * sizeof(double));
    patch->uKnots[4] = patch->uKnots[5] = patch->uKnots[6] = patch->uKnots[7] =
    patch->vKnots[4] = patch->vKnots[5] = patch->vKnots[6] = patch->vKnots[7] = 1.0;

    /* Evaluate function at corners if necessary */
    if (code == CALCULATE_ALL)
        {
        if (SUCCESS != (status = (*surfaceFunc) (p00, p00+1, p00+2, NULL, NULL, p00+5,
                                                 range->low.x, range->low.y, userDataP)) ||
            SUCCESS != (status = (*surfaceFunc) (p01, p01+1, p01+2, NULL, NULL, p01+5,
                                                 range->low.x, range->high.y, userDataP)) ||
            SUCCESS != (status = (*surfaceFunc) (p10, p10+1, p10+2, NULL, NULL, p10+5,
                                                 range->high.x, range->low.y, userDataP)) ||
            SUCCESS != (status = (*surfaceFunc) (p11, p11+1, p11+2, NULL, NULL, p11+5,
                                                 range->high.x, range->high.y, userDataP)))
        goto wrapup;
        }
    else if (code == CALCULATE_ABC)
        {
        if (SUCCESS != (status = (*surfaceFunc) (p01, p01+1, p01+2, NULL, NULL, p01+5,
                                                 range->low.x, range->high.y, userDataP)) ||
            SUCCESS != (status = (*surfaceFunc) (p10, p10+1, p10+2, NULL, NULL, p10+5,
                                                 range->high.x, range->low.y, userDataP)) ||
            SUCCESS != (status = (*surfaceFunc) (p11, p11+1, p11+2, NULL, NULL, p11+5,
                                                 range->high.x, range->high.y, userDataP)))
            goto wrapup;
        }
    else if (code == CALCULATE_D || code == CALCULATE_E)
        {
        if (SUCCESS != (status = (*surfaceFunc) (p11, p11+1, p11+2, NULL, NULL, p11+5,
                                                 range->high.x, range->high.y, userDataP)))
            goto wrapup;
        }


#if defined (debug_draw)
            {
            DPoint3d            line0[2], line1[2], line2[2], line3[2];
            ElementUnion        elU0, elU1, elU2, elU3;

            line0[0] = line0[1] = *p00;
            line1[0] = line1[1] = *p01;
            line2[0] = line2[1] = *p10;
            line3[0] = line3[1] = *p11;
            create_lineD (&elU0, NULL, line0, NULL);
            create_lineD (&elU1, NULL, line1, NULL);
            create_lineD (&elU2, NULL, line2, NULL);
            create_lineD (&elU3, NULL, line3, NULL);
            elU0.line_3d.dhdr.symb.b.color = 10;
            elU1.line_3d.dhdr.symb.b.color = 10;
            elU2.line_3d.dhdr.symb.b.color = 10;
            elU3.line_3d.dhdr.symb.b.color = 10;
            elU0.line_3d.dhdr.symb.b.weight = 10;
            elU1.line_3d.dhdr.symb.b.weight = 10;
            elU2.line_3d.dhdr.symb.b.weight = 10;
            elU3.line_3d.dhdr.symb.b.weight = 10;
            displayElement (&elU0, 0, -1L);
            displayElement (&elU1, 0, -1L);
            displayElement (&elU2, 0, -1L);
            displayElement (&elU3, 0, -1L);
            }
#endif

    /* Calculate the magnifying factor for derivatives of offset surf */
    bsprfit_magnifyDerivatives (&ratiox, &ratioy, &ratioxy, range,
                                p00, p01, p10, userDataP, isOffset);

    /* Scale the derivatives by the length of the interval */
    diffx = range->high.x - range->low.x;
    diffy = range->high.y - range->low.y;
    factor_3x  = diffx * ratiox * DENO3;
    factor_3y  = diffy * ratioy * DENO3;
    factor_9  = diffx * diffy * ratioxy * DENO9;

    bsiDPoint3d_scale (tmp00,   p00+1, factor_3x);
    bsiDPoint3d_scale (tmp00+1, p00+2, factor_3y);
    bsiDPoint3d_scale (tmp00+2, p00+5, factor_9);

    bsiDPoint3d_scale (tmp01,   p01+1, factor_3x);
    bsiDPoint3d_scale (tmp01+1, p01+2, factor_3y);
    bsiDPoint3d_scale (tmp01+2, p01+5, factor_9);

    bsiDPoint3d_scale (tmp10,   p10+1, factor_3x);
    bsiDPoint3d_scale (tmp10+1, p10+2, factor_3y);
    bsiDPoint3d_scale (tmp10+2, p10+5, factor_9);

    bsiDPoint3d_scale (tmp11,   p11+1, factor_3x);
    bsiDPoint3d_scale (tmp11+1, p11+2, factor_3y);
    bsiDPoint3d_scale (tmp11+2, p11+5, factor_9);

    /* Assign the poles according to the bicubic Hermite interpolation */
    patch->poles[0] = *p00;
    bsiDPoint3d_addDPoint3dDPoint3d (patch->poles+1, p00, tmp00);
    bsiDPoint3d_subtractDPoint3dDPoint3d (patch->poles+2, p10, tmp10);
    patch->poles[3] = *p10;
    bsiDPoint3d_addDPoint3dDPoint3d (patch->poles+4, p00, tmp00+1);

    bsiDPoint3d_addDPoint3dDPoint3d (patch->poles+5, tmp00, tmp00+1);
    bsiDPoint3d_addDPoint3dDPoint3d (patch->poles+5, p00, patch->poles+5);
    bsiDPoint3d_addDPoint3dDPoint3d (patch->poles+5, patch->poles+5, tmp00+2);

    bsiDPoint3d_subtractDPoint3dDPoint3d (patch->poles+6, tmp10+1, tmp10);
    bsiDPoint3d_addDPoint3dDPoint3d (patch->poles+6, p10, patch->poles+6);
    bsiDPoint3d_subtractDPoint3dDPoint3d (patch->poles+6, patch->poles+6, tmp10+2);

    bsiDPoint3d_addDPoint3dDPoint3d (patch->poles+7, p10, tmp10+1);
    bsiDPoint3d_subtractDPoint3dDPoint3d (patch->poles+8, p01, tmp01+1);

    bsiDPoint3d_subtractDPoint3dDPoint3d (patch->poles+9, tmp01, tmp01+1);
    bsiDPoint3d_addDPoint3dDPoint3d (patch->poles+9, p01, patch->poles+9);
    bsiDPoint3d_subtractDPoint3dDPoint3d (patch->poles+9, patch->poles+9, tmp01+2);

    bsiDPoint3d_addDPoint3dDPoint3d (patch->poles+10, tmp11, tmp11+1);
    bsiDPoint3d_subtractDPoint3dDPoint3d (patch->poles+10, p11, patch->poles+10);
    bsiDPoint3d_addDPoint3dDPoint3d (patch->poles+10, patch->poles+10, tmp11+2);

    bsiDPoint3d_subtractDPoint3dDPoint3d (patch->poles+11, p11, tmp11+1);
    patch->poles[12] = *p01;
    bsiDPoint3d_addDPoint3dDPoint3d (patch->poles+13, p01, tmp01);
    bsiDPoint3d_subtractDPoint3dDPoint3d (patch->poles+14, p11, tmp11);
    patch->poles[15] = *p11;

    /* Calculate error */
    maxError = 0.0;
    uIncr = vIncr = 1.0 / (numPoints-1);

    for (j=0, v = 0.0, pP=points; j < numPoints; j++, v += vIncr)
        for (i=0, u = 0.0; i < numPoints; i++, u += uIncr, pP++)
            {
            bspsurf_evaluateSurfacePoint (&tmp0, NULL, NULL, NULL, u, v,
                                            patch);

            status = (*surfaceFunc) (pP,
                                     NULL, NULL, NULL, NULL, NULL,
                                     diffx*u+range->low.x,
                                     diffy*v+range->low.y,
                                     userDataP);

            tmpError = bsiDPoint3d_distance (&tmp0, pP);
            if (tmpError > maxError )
                maxError = tmpError;

#if defined (nodebug_draw)
            {
            DPoint3d            line[2];
            ElementUnion        elU;

            line[0] = tmp0;
            line[1] = *pP;
            create_lineD (&elU, NULL, line, NULL);
            displayElement (&elU, 0, -1L);
            }
#endif
            }

    *error = maxError;

wrapup:
    if (status)
        bspsurf_freeSurface (patch);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    06/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprfit_approxWithQuinticPatch
(
MSBsplineSurface*                   patch,
double*                             error,
DRange2d*                           range,
BSplineCallback_AskSurfacePoints    surfaceFunc,
void*                               userDataP,
int                                 numPoints,
double*                             bFunc3Array,
double*                             bFunc5Array,
DPoint3dP                           points,
DPoint3dP                           p00,
DPoint3dP                           p01,
DPoint3dP                           p10,
DPoint3dP                           p11,
int                                 code
)
    {
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    06/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprfit_approxSurfaceRecurse
(
MSBsplineSurface*                   bspline,            /* <= Bspline surface */
DRange2d*                           fullRange,          /* => parametric range to approximate */
BSplineCallback_AskSurfacePoints    surfaceFunc,        /* => surface function */
void*                               userDataP,          /* => passed through to surfFunc */
double                              tolerance,          /* => accuracy of approximation */
int                                 numPoints,          /* => density of approx pts for error calculation */
int                                 continuity,         /* => degree of continuity btw spline pieces */
double*                             bFunc3Array,        /* => blending funcs */
double*                             bFunc5Array,        /* => blending funcs for curvature */
DPoint3dP                           pointsArray,
DPoint3dP                           p00,
DPoint3dP                           p01,
DPoint3dP                           p10,
DPoint3dP                           p11,
int                                 code,
int                                 depth,
int                                 isOffset
)
    {
    int                 status;
    double              error, midU, midV;
    DRange2d            range[4];
    DPoint3d            ptA[6], ptB[6], ptC[6], ptD[6], ptE[6];
    MSBsplineSurface    surf[4];

    if (depth > REV_LIMIT)
        return ERROR;

    memset (bspline, 0, sizeof(*bspline));
    memset (surf, 0, sizeof(surf));

    if (continuity == TANGENT_CONTINUITY)
        {
        if (SUCCESS != (status = bsprfit_approxWithBiHermite (
                                        bspline, &error, fullRange,
                                        surfaceFunc, userDataP,
                                        numPoints, bFunc3Array, pointsArray,
                                        p00, p01, p10, p11, code, isOffset)))
            goto wrapup;
        }
    else
        {
        if (SUCCESS != (status = bsprfit_approxWithQuinticPatch (
                                        bspline, &error, fullRange,
                                        surfaceFunc, userDataP, numPoints,
                                        bFunc3Array, bFunc5Array, pointsArray,
                                        p00, p01, p10, p11, code)))
            goto wrapup;
        }

    if (error < tolerance || depth > 3)
        {
        bspline->display.curveDisplay = true;
        return SUCCESS;
        }

    bspsurf_freeSurface (bspline);

    /* Subdivide range and approximate in quarters */
    /* surf[0] 0 <= u <= 1/2 : 0 <= v <= 1/2     */
    /* surf[1] 1/2 <= u <= 1 : 0 <= v <= 1/2     */
    /* surf[2] 0 <= u <= 1/2 : 1/2 <= v <= 1     */
    /* surf[3] 1/2 <= u <= 1 : 1/2 <= v <= 1     */

    midU = (fullRange->high.x + fullRange->low.x) / 2.0;
    midV = (fullRange->high.y + fullRange->low.y) / 2.0;

    range[0].low.x = range[2].low.x = fullRange->low.x;
    range[0].low.y = range[1].low.y = fullRange->low.y;

    range[0].high.x = range[1].low.x = range[2].high.x = range[3].low.x = midU;
    range[0].high.y = range[1].high.y = range[2].low.y = range[3].low.y = midV;

    range[1].high.x = range[3].high.x = fullRange->high.x;
    range[2].high.y = range[3].high.y = fullRange->high.y;


    /* Avoid recalculating the following points and their derivatives: */
    /* ptA = (0.5, 0.5) and dAdU, dAdV, dAdUU, dAdVV */
    /* ptB = (0.0, 0.5) and dBdU, dBdV, dBdUU, dBdVV */
    /* ptC = (0.5, 0.0) and dCdU, dCdV, dCdUU, dCdVV */
    /* ptD = (0.5, 1.0) and dDdU, dDdV, dDdUU, dDdVV */
    /* ptE = (1.0, 0.5) and dEdU, dEdV, dEdUU, dEdVV */

    if (SUCCESS != (status = bsprfit_approxSurfaceRecurse (
                                    surf, range, surfaceFunc, userDataP,
                                    tolerance, numPoints, continuity,
                                    bFunc3Array, bFunc5Array, pointsArray,
                                    p00, ptB, ptC, ptA, CALCULATE_ABC,
                                    depth+1, isOffset)) ||
        SUCCESS != (status = bsprfit_approxSurfaceRecurse (
                                    surf+1, range+1, surfaceFunc, userDataP,
                                    tolerance, numPoints, continuity,
                                    bFunc3Array, bFunc5Array, pointsArray,
                                    ptC, ptA, p10, ptE, CALCULATE_E,
                                    depth+1, isOffset)) ||
        SUCCESS != (status = bsprfit_approxSurfaceRecurse (
                                    surf+2, range+2, surfaceFunc, userDataP,
                                    tolerance, numPoints, continuity,
                                    bFunc3Array, bFunc5Array, pointsArray,
                                    ptB, p01, ptA, ptD, CALCULATE_D,
                                    depth+1, isOffset)) ||
        SUCCESS != (status = bsprfit_approxSurfaceRecurse (
                                    surf+3, range+3, surfaceFunc, userDataP,
                                    tolerance, numPoints, continuity,
                                    bFunc3Array, bFunc5Array, pointsArray,
                                    ptA, ptD, ptE, p11, CALCULATE_NONE,
                                    depth+1, isOffset)))
        goto wrapup;

    if (SUCCESS != (status = bsprsurf_appendSurfaces (surf, surf, surf+1, BSSURF_V,
                                           true, false)) ||
        SUCCESS != (status = bsprsurf_appendSurfaces (surf+2, surf+2, surf+3, BSSURF_V,
                                           true, false)) ||
        SUCCESS != (status = bsprsurf_appendSurfaces (bspline, surf, surf+2, BSSURF_U,
                                           true, false)))
        goto wrapup;

wrapup:
    bspsurf_freeSurface (surf);
    bspsurf_freeSurface (surf+1);
    bspsurf_freeSurface (surf+2);
    bspsurf_freeSurface (surf+3);
    if (status)
        bspsurf_freeSurface (bspline);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* surfaceFunc : User supplied function to calculate curve position & tangent at a parameter
* @bsimethod                                                    Brian.Peters    06/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int        bsprfit_approximateSurface
(
MSBsplineSurface*                   bspline,            /* <= Bspline surface */
BSplineCallback_AskSurfacePoints    surfaceFunc,        /* => surface function */
CallbackArgP                        userDataP,          /* => passed through to surf func */
double                              tolerance,          /* => accuracy of approximation */
int                                 density,            /* => density of approx pts for error calculation */
int                                 continuity,         /* => degree of continuity btw spline pieces */
int                                 isOffset            /* => True for offset surface */
)
    {
    int         i, status, numPoints, left, order;
    double      *bP, *endP, *bFunc3Array, *bFunc5Array,
                knots[2*MAX_ORDER], u, incr;
    DRange2d   range;
    DPoint3d    p00[6], p01[6], p10[6], p11[6], *points;

    if (continuity == TANGENT_CONTINUITY)
        {
        numPoints = density > 6 ? density : 6;
        order = 4;
        }
    else if (continuity == CURVATURE_CONTINUITY)
        {
        numPoints = density > 10 ? density : 10;
        order = 5;
        }
    else if (continuity == POSITION_CONTINUITY)
        return ERROR;
    else
        return ERROR;

    if (tolerance < fc_1em15)
        return ERROR;

    points = NULL;      bFunc3Array = bFunc5Array = NULL;
    range.low.x = range.low.y = 0.0;
    range.high.x = range.high.y = 1.0;

    /* Can save considerable time in the recursion by avoiding repeated
       calculation of the b-spline blending functions. Also allocate
       points scratch buffer here. */
    ScopedArray<double> scopedFunc3Array (numPoints * 4);          bFunc3Array = scopedFunc3Array.GetData ();
    ScopedArray<double> scopedFunc5Array (numPoints * 6);          bFunc5Array = scopedFunc5Array.GetData ();
    ScopedArray<DPoint3d> scopedPoints     (numPoints * numPoints);  points = scopedPoints.GetData ();       

    memset (knots, 0, order * sizeof(double));
    for (bP=endP=knots+order, endP+=order; bP < endP; bP++)
        *bP = 1.0;

    incr = 1.0 / (numPoints - 1);
    for (bP=bFunc3Array, i=0, u=0.0; i < numPoints;
                                         bP += 4, i++, u += incr)
        bsputil_computeBlendingFuncs (bP, NULL, &left, knots, u, 1.0,
                                      order, false);

    if (continuity == CURVATURE_CONTINUITY)
        {
        memset (knots, 0, 6 * sizeof(double));
        for (bP=endP=knots+6, endP+=6; bP < endP; bP++)
            *bP = 1.0;
        for (bP=bFunc5Array, i=0, u=0.0; i < numPoints;
                bP += 6, i++, u += incr)
            bsputil_computeBlendingFuncs (bP, NULL, &left, knots, u,
                1.0, 6, false);
        }

    status = bsprfit_approxSurfaceRecurse (bspline, &range, surfaceFunc,
                                           userDataP, tolerance, numPoints,
                                           continuity, bFunc3Array,
                                           bFunc5Array, points, p00, p01,
                                           p10, p11, CALCULATE_ALL, 0,
                                           isOffset);

    bspline->display.curveDisplay = true;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          01/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bsprfit_catmullGetCorners
(
DPoint3d        *corners,
DPoint3d        *poles,
int             index,
int             numPoles
)
    {
    DPoint3d    vec1, vec2, inners[5];

    inners[0] = poles[index];
    inners[1] = poles[index-1];
    inners[2] = poles[index+1];
    inners[3] = poles[index-numPoles];
    inners[4] = poles[index+numPoles];

    bsiDPoint3d_subtractDPoint3dDPoint3d (&vec1, inners+1, inners);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vec2, inners+2, inners);
    bsiDPoint3d_addScaledDPoint3d (corners,   inners+3, &vec1, 1.0);
    bsiDPoint3d_addScaledDPoint3d (corners+2, inners+4, &vec1, 1.0);
    bsiDPoint3d_addScaledDPoint3d (corners+1, inners+3, &vec2, 1.0);
    bsiDPoint3d_addScaledDPoint3d (corners+3, inners+4, &vec2, 1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          01/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprfit_catmullRomSurface
(
MSBsplineSurface    *surface,       /* <=  resulting surface */
DPoint3dCP           inPts,         /*  => data points */
int                 inNumU,         /*  => number of points in each row */
int                 inNumV,         /*  => number of rows */
DPoint2dCP          inValues       /*  => uv values or NULL */
)
    {
    int             i, j, status, numPolesU, numPolesV,
                    total, numU, numV, numKnotsU, numKnotsV, index;
    DPoint3d        corners[4], vec1, vec2;
    DPoint3dCP     pColU; // points into input (const)
    DPoint3dP      pColV, polesU, polesV;

    if (inNumU < 3 || inNumV < 3)
        return ERROR;

    /* Allocate memory */
    surface->Zero ();
    surface->rational=surface->uParams.closed=surface->vParams.closed=false;
    surface->uParams.order = surface->vParams.order = 4;
    numPolesU = surface->uParams.numPoles = 3*inNumU-2;
    numPolesV = surface->vParams.numPoles = 3*inNumV-2;
    surface->uParams.numKnots = surface->vParams.numKnots = 0;
    numKnotsU = numPolesU+4;
    numKnotsV = numPolesV+4;

    if (SUCCESS != (status = bspsurf_allocateSurface (surface)))
        return status;

    /* Allocate tmp variable */
    polesU = polesV = NULL;
    ScopedArray<DPoint3d> scoped_pColV (inNumV);          pColV = scoped_pColV.GetData ();
    ScopedArray<DPoint3d> scoped_polesU (numPolesU);      polesU = scoped_polesU.GetData ();
    ScopedArray<DPoint3d> scoped_polesV (numPolesV);      polesV = scoped_polesV.GetData ();    

    /* Assign the poles in rows */
    for (j=0, pColU=inPts; j<inNumV; j++, pColU+=inNumU)
        {
        if (SUCCESS != (status = bspcurv_catmullRomPoles(polesU, NULL,
                                                         j == 0 ? surface->uKnots : NULL,
                                                         (DPoint3d*)pColU, NULL, NULL, numPolesU,
                                                         surface->uParams.numKnots, inNumU)))
            goto wrapup;

        for (i=0; i<numPolesU; i++)
            surface->poles[3*j*numPolesU+i] = polesU[i];
        }

    for (i=0; i<inNumU; i++)
        {
        for (j=0; j<inNumV; j++)
            pColV[j]=inPts[j*inNumU+i];

        if (SUCCESS != (status = bspcurv_catmullRomPoles(polesV, NULL,
                                                         i == 0 ? surface->vKnots : NULL,
                                                         pColV, NULL, NULL, numPolesV,
                                                         surface->vParams.numKnots, inNumV)))
            goto wrapup;

        for (j=0; j<numPolesV; j++)
            surface->poles[j*numPolesU+3*i]=polesV[j];
        }

    /* Compute the inner poles */
    numU=inNumU-1;
    numV=inNumV-1;
    for (j=1; j<numV; j++)
        {
        for (i=1; i<numU; i++)
            {
            index = 3*(j*numPolesU+i);
            bsprfit_catmullGetCorners(corners, surface->poles,
                                      index, numPolesU);
            surface->poles[index-numPolesU-1] = corners[0];
            surface->poles[index-numPolesU+1] = corners[1];
            surface->poles[index+numPolesU-1] = corners[2];
            surface->poles[index+numPolesU+1] = corners[3];
            }
        }

    /* First row */
    for (i=1; i<numU; i++)
        {
        index=3*i;
        bsiDPoint3d_subtractDPoint3dDPoint3d(&vec1, surface->poles+index-1, surface->poles+index);
        bsiDPoint3d_subtractDPoint3dDPoint3d(&vec2, surface->poles+index+1, surface->poles+index);
        bsiDPoint3d_addScaledDPoint3d(surface->poles+index+numPolesU-1,
                    surface->poles+index+numPolesU, &vec1, 1.0);
        bsiDPoint3d_addScaledDPoint3d(surface->poles+index+numPolesU+1,
                    surface->poles+index+numPolesU, &vec2, 1.0);
        }

    /* Last row */
    total = surface->uParams.numPoles * (surface->vParams.numPoles-1);
    for (i=1; i<numU; i++)
        {
        index=3*i+total;
        bsiDPoint3d_subtractDPoint3dDPoint3d(&vec1, surface->poles+index-1, surface->poles+index);
        bsiDPoint3d_subtractDPoint3dDPoint3d(&vec2, surface->poles+index+1, surface->poles+index);
        bsiDPoint3d_addScaledDPoint3d(surface->poles+index-numPolesU-1,
                    surface->poles+index-numPolesU, &vec1, 1.0);
        bsiDPoint3d_addScaledDPoint3d(surface->poles+index-numPolesU+1,
                    surface->poles+index-numPolesU, &vec2, 1.0);
        }

    /* Fiest colunm */
    for (j=1; j<numV; j++)
        {
        index=3*j*numPolesU;
        bsiDPoint3d_subtractDPoint3dDPoint3d(&vec1, surface->poles+index+numPolesU,
                        surface->poles+index);
        bsiDPoint3d_subtractDPoint3dDPoint3d(&vec2, surface->poles+index-numPolesU,
                        surface->poles+index);
        bsiDPoint3d_addScaledDPoint3d(surface->poles+index+numPolesU+1,
                    surface->poles+index+1, &vec1, 1.0);
        bsiDPoint3d_addScaledDPoint3d(surface->poles+index-numPolesU+1,
                    surface->poles+index+1, &vec2, 1.0);
        }

    /* Second colunm */
    for (j=1; j<numV; j++)
        {
        index=3*j*numPolesU+numPolesU-1;
        bsiDPoint3d_subtractDPoint3dDPoint3d(&vec1, surface->poles+index+numPolesU,
                        surface->poles+index);
        bsiDPoint3d_subtractDPoint3dDPoint3d(&vec2, surface->poles+index-numPolesU,
                        surface->poles+index);
        bsiDPoint3d_addScaledDPoint3d(surface->poles+index+numPolesU-1,
                    surface->poles+index-1, &vec1, 1.0);
        bsiDPoint3d_addScaledDPoint3d(surface->poles+index-numPolesU-1,
                    surface->poles+index-1, &vec2, 1.0);
        }

    /* Four corners*/
    bsiDPoint3d_subtractDPoint3dDPoint3d(&vec1, surface->poles+1, surface->poles);
    bsiDPoint3d_subtractDPoint3dDPoint3d(&vec2, surface->poles+numPolesU-2,
                    surface->poles+numPolesU-1);
    bsiDPoint3d_addScaledDPoint3d(surface->poles+numPolesU+1, surface->poles+numPolesU,
                &vec1, 1.0);
    bsiDPoint3d_addScaledDPoint3d(surface->poles+2*numPolesU-2, surface->poles+2*numPolesU-1,
                &vec2, 1.0);

    bsiDPoint3d_subtractDPoint3dDPoint3d(&vec1, surface->poles+total+1, surface->poles+total);
    bsiDPoint3d_subtractDPoint3dDPoint3d(&vec2, surface->poles+total+numPolesU-2,
                    surface->poles+total+numPolesU-1);
    bsiDPoint3d_addScaledDPoint3d(surface->poles+total-numPolesU+1,
                surface->poles+total-numPolesU, &vec1, 1.0);
    bsiDPoint3d_addScaledDPoint3d(surface->poles+total-2, surface->poles+total-1,       &vec2, 1.0);

    surface->display.curveDisplay = true;

wrapup:
    return status;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
