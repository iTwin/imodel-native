/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bspv7.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
 FP equality test with fixed tolerance 0.0001.
* @bsimethod                                                    RBB             08/86
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     mdlMath_DEqual (double r1, double r2)
    {
    return (fabs (r1 - r2) < .0001);
    }



/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_relevantPoles_V7                                |
|                                                                       |
| author        BFP                                     11/90           |
|                                                                       |
| Copied from bsputil.c with V7 logic restored.                         |
|                                                                       |
+----------------------------------------------------------------------*/
static void    bsputil_relevantPoles_V7
(
DPoint3d    	*poles,		       /* <= poles that define curve at u */
double          *weights,              /* <= weights (if rational) */
int             *rght,                 /* <= kV[rght-1] <= u < kV[rght] */
int             *start,                /* <= index of first pole */
DPoint3d    	*fullPoles,	       /* => all poles of curve */
double          *fullWeights,          /* => all  weights (if rational) */
double          u,                     /* => parameter to find */
double          *knotVector,           /* => full knot vector (kV) */
int             numPoles,
int             order,
int             closed,
int             rational
)
    {
    double      *kP, *lastKnotP;

    lastKnotP = knotVector + bspknot_numberKnots (numPoles, order, closed);
    for (kP=knotVector; u >= *kP && kP < lastKnotP; kP++)
        ;

#ifdef TODO_EliminateAddressArithmetic
#endif
    *rght = *start = (int)(kP - knotVector);
    if (closed) *start -= order/2;
    *start -= order;

    bsputil_loadPoles (poles, weights, fullPoles, fullWeights, *start,
                       numPoles, order, closed, rational);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_insertKnot_V7                                   |
|                                                                       |
| author        BFP                                     3/90            |
|                                                                       |
| Copied from bsputil.c with V7 logic restored and call to              |
| relevantPoles changed.                                                |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bspknot_insertKnot_V7
(
double          u,
int             addMult,
int             currentMult,
DPoint3d	*oldPoles,
double          *oldKnots,
double          *oldWeights,
DPoint3d	*newPoles,
double          *newKnots,
double          *newWeights,
int             numPoles,
int             order,
int             closed,
int             rational
)
{
    int         i, j, degree, rght, start, numKnots, shift, end;
    double      *c, *left, *right, *wP0, *wP1, *wts,
                wtsBuffer[2*MAX_BSORDER], coef[MAX_BSORDER];
    DPoint3d	*pP0, *pP1, *pole, poleBuffer[2*MAX_BSORDER];
    
    degree = order - 1;
    pole   = poleBuffer + addMult;
    wts    = wtsBuffer + addMult;

    bsputil_relevantPoles_V7 (pole, wts, &rght, &start, oldPoles, oldWeights, u,
                           oldKnots, numPoles, order, closed, rational);

    for (i=addMult, end=degree; i > 0; i--, end--)
        {
        *(pole-i) = *pole;
        right = oldKnots + rght;
        left = right - end;
        for (j=0, pP0=pole, pP1=pP0+1, c=coef; j < end;
             j++, right++, left++, pP0++, pP1++, c++)
            {
            *c = (u - *left)/(*right - *left);
            pP0->x = *c * ((pP1->x) - (pP0->x)) + (pP0->x);
            pP0->y = *c * ((pP1->y) - (pP0->y)) + (pP0->y);
            pP0->z = *c * ((pP1->z) - (pP0->z)) + (pP0->z);
            }
        if (rational)
            {
            *(wts-i) = *wts;
            for (j=0, wP0=wts, wP1=wP0+1, c=coef;
                 j < end; j++, wP0++, wP1++, c++)
                {
                *wP0 = *c * (*wP1 - *wP0) + *wP0;
                }
            }
        }

    /* Copy to newPoles & newWeights */
    if (start < 0)
        {
        /* new poles wrap around old poles at left */
        shift = order + start;
	memcpy (newPoles, poleBuffer - start, (shift + addMult) * sizeof(DPoint3d));
        memcpy (newPoles + shift + addMult, oldPoles + shift,
		(numPoles - order) * sizeof(DPoint3d));
	memcpy (newPoles + numPoles + addMult + start, poleBuffer, (-start) * sizeof(DPoint3d));
        if (rational)
            {
            memcpy (newWeights, wtsBuffer - start, (shift + addMult) * sizeof(double));
            memcpy (newWeights + shift + addMult, oldWeights + shift,
                    (numPoles - order) * sizeof(double));
            memcpy (newWeights + numPoles + addMult + start, wtsBuffer,
                    (-start) * sizeof(double));
            }
        }
    else if (start > (numPoles - order))
        {
        /* new poles wrap around old poles at right */
        shift = start + order - numPoles;
	memcpy (newPoles, poleBuffer + order + addMult - shift, shift * sizeof(DPoint3d));
        memcpy (newPoles + shift, oldPoles + shift, (numPoles - order) * sizeof(DPoint3d));
        memcpy (newPoles + start, poleBuffer, (numPoles + addMult - start) * sizeof(DPoint3d));
        if (rational)
            {
            memcpy (newWeights, wtsBuffer + order + addMult - shift, shift * sizeof(double));
            memcpy (newWeights + shift, oldWeights + shift, (numPoles - order) * sizeof(double));
            memcpy (newWeights + start, wtsBuffer, (numPoles + addMult - start) * sizeof(double));
            }
        }
    else
        {
        /* new poles fit inside old poles */
        shift = start + order;
	memcpy (newPoles, oldPoles, start * sizeof(DPoint3d));
        memcpy (newPoles + start, poleBuffer, (order + addMult) * sizeof(DPoint3d));
        memcpy (newPoles + shift + addMult, oldPoles + start + order,
		(numPoles - shift) * sizeof(DPoint3d));
        if (rational)
            {
            memcpy (newWeights, oldWeights, start * sizeof(double));
            memcpy (newWeights + start, wtsBuffer, (order + addMult) * sizeof(double));
            memcpy (newWeights + shift + addMult, oldWeights + start + order,
                    (numPoles - shift) * sizeof(double));
            }
        }
    
    memcpy (newKnots, oldKnots, rght * sizeof(double));
    for (i=0, right=newKnots + rght; i < addMult; i++, right++)
        *right = u;
    numKnots = bspknot_numberKnots (numPoles, order, closed);
    memcpy (newKnots + rght + addMult, oldKnots + rght, (numKnots - rght) * sizeof(double));

    bspknot_normalizeKnotVector (newKnots, numPoles + addMult, order, closed);
    
    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_addKnot_V7                                      |
|                                                                       |
| author        BFP                                     3/90            |
|                                                                       |
| Copied from bsputil.c with call to insertKnot changed.                |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bspknot_addKnot_V7
(
MSBsplineCurve  *curve,
double          u,
double          knotTolerance,
int             newMult,
int             addToCurrent
)
    {
    int         i, status, degree, newNumPoles, numKnots, currentMult, addMult, allocSize;
    double      tempU, minKnot, maxKnot, *knots, *weights, *kP;
    DPoint3d	*poles;
    
    degree = curve->params.order - 1;
    
    numKnots = bspknot_numberKnots (curve->params.numPoles, curve->params.order,
                                    curve->params.closed);
    minKnot = curve->knots[degree];
    maxKnot = curve->knots[numKnots - curve->params.order];
    
    if (u < minKnot || u > maxKnot)
        return ERROR;

   /* determine the the multiplicity of u already in the knot sequence */
    
    currentMult = 0;
    tempU = u;
    for (i=0, kP=curve->knots; i < numKnots; i++, kP++)
        {
        if (fabs (*kP - tempU) <= knotTolerance)
            {
            tempU = *kP;
            currentMult += 1;
            }
        else
            {
            if (currentMult > 0) break;
            }
        }

    addMult = addToCurrent ? newMult : newMult - currentMult;

    /* done if addMult<=0. newMult = degree & currentMult = degree or order */
    if (addMult <= 0) return SUCCESS;
    if (addMult > curve->params.order)
        return ERROR;
        
    /* Malloc memory for the resulting curve structure */
    poles = NULL; knots = weights = NULL;
    newNumPoles = curve->params.numPoles + addMult;
    allocSize = newNumPoles * sizeof (DPoint3d);
    if (NULL == (poles = (DPoint3d *) BSIBaseGeom::Malloc (allocSize)))
        {
        status = ERROR;
        goto wrapup;
        }
    
    if (curve->rational)
        {
        allocSize = newNumPoles * sizeof (double);
        if (NULL == (weights = (double *) BSIBaseGeom::Malloc (allocSize)))
            {
            status = ERROR;
            goto wrapup;
            }
        }

    numKnots += addMult;
    allocSize = numKnots * sizeof (double);
    if (NULL == (knots = (double *) BSIBaseGeom::Malloc (allocSize)))
        {
        status = ERROR;
        goto wrapup;
        }
    
    /* Calculate new poles, knots, and weights from old */
    if (SUCCESS !=
        (status = bspknot_insertKnot_V7 (tempU, addMult, currentMult, curve->poles,
                                      curve->knots, curve->weights, poles, knots,
                                      weights, curve->params.numPoles, curve->params.order,
                                      curve->params.closed, curve->rational)))
        goto wrapup;

    curve->params.numPoles += addMult;
    BSIBaseGeom::Free (curve->poles);
    curve->poles = poles;
    
    curve->params.numKnots = bspknot_numberKnots (curve->params.numPoles,
                                                  curve->params.order, curve->params.closed)
                             - 2 * curve->params.order;
    BSIBaseGeom::Free (curve->knots);
    curve->knots = knots;

    if (curve->rational)
        {
        BSIBaseGeom::Free (curve->weights);
        curve->weights = weights;
        }
    poles = NULL; knots = weights = NULL;
    
wrapup:
    if (poles)      BSIBaseGeom::Free  (poles);
    if (knots)      BSIBaseGeom::Free  (knots);
    if (weights)    BSIBaseGeom::Free  (weights);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_openCurve_V7                                    |
|                                                                       |
| author        DavidAssaf                              12/01           |
|                                                                       |
| Copied from bspcurv.c with V7 logic restored.                         |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      mdlBspline_openCurve_V7
(
MSBsplineCurve  *outCurve,
MSBsplineCurve  *inCurve,
double          u
)
    {
    int             i, j, status, initKnot, maxKnotIndex, polesToEnd, numKnots;
    double          knotTolerance, *kP1, *end;
    MSBsplineCurve  curve;

    if (!inCurve->params.closed)
        return bspcurv_copyCurve (outCurve, inCurve);

    /* Copy the input curve so output may be same */
    if (SUCCESS != (status = bspcurv_copyCurve (&curve, inCurve)))
        return status;

    knotTolerance = bspknot_knotTolerance (&curve);
    if (SUCCESS !=
        (status = bspknot_addKnot_V7 (&curve, u, knotTolerance, curve.params.order, false)))
        goto wrapup;

    if (inCurve == outCurve)
        bspcurv_freeCurve (inCurve);
    *outCurve = curve;
    outCurve->params.closed = false;
    outCurve->params.numKnots = outCurve->params.numPoles - outCurve->params.order;

    if (SUCCESS != (status = bspcurv_allocateCurve (outCurve)))
        goto wrapup;

    numKnots = curve.params.numPoles + 2 * curve.params.order - 1;
    for (initKnot=0, kP1=end=curve.knots, end += numKnots; kP1 < end && (*kP1 - u <= knotTolerance);
         initKnot++, kP1++)
        ;
    /* point to first multiple of newly saturated knot */
    initKnot -= curve.params.order;

    maxKnotIndex = curve.params.numPoles + curve.params.order - 1;
    /* copy multiples of newly saturated knot up to end knot */
    for (i=initKnot, j=0; i < maxKnotIndex; i++, j++)
        outCurve->knots[j] = curve.knots[i];
    /* append old start knot on up, adding each to old end knot to complete new knot vector */
    for (i = outCurve->params.order - 1;
         j < outCurve->params.numPoles + outCurve->params.order;
         j++, i++)
        outCurve->knots[j] = curve.knots[i] + curve.knots[maxKnotIndex];

    bspknot_normalizeKnotVector (outCurve->knots, outCurve->params.numPoles,
                                 outCurve->params.order, outCurve->params.closed);

    /* when u=0, this points to start of old knots hacked up w/ closeCurve */
    initKnot -= curve.params.order / 2;

    /* Safety check, if first knot is zero, init knot could be negative. */
    if (initKnot < 0)
        initKnot = 0;
    polesToEnd = curve.params.numPoles - initKnot;

    memcpy (outCurve->poles, curve.poles + initKnot, polesToEnd * sizeof(DPoint3d));
    memcpy (outCurve->poles + polesToEnd, curve.poles, initKnot * sizeof(DPoint3d));

    if (outCurve->rational)
        {
        memcpy (outCurve->weights, curve.weights + initKnot, polesToEnd * sizeof(double));
        memcpy (outCurve->weights + polesToEnd, curve.weights, initKnot * sizeof(double));
        }

wrapup:
    bspcurv_freeCurve (&curve);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsprsurf_openSurfaceU_V7                                |
|                                                                       |
| author        DavidAssaf                              12/01           |
|                                                                       |
| Copied from bsprsurf.c with call to openCurve changed.                |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      mdlBspline_openSurfaceU_V7
(
MSBsplineSurface    *output,
MSBsplineSurface    *input,
double              uValue
)
    {
    int                 i, status;
    double              *inwP, *swP = NULL;
    DPoint3d	    	*inpP, *spP = NULL;
    MSBsplineCurve      curve, open;
    MSBsplineSurface    surf;

    if (!input->uParams.closed)
        return bspsurf_copySurface (output, input);

    memset (&curve, 0, sizeof(curve));
    memset (&open, 0, sizeof(open));
    memset (&surf, 0, sizeof(surf));

    curve.rational = input->rational;
    curve.params = input->uParams;
    if (SUCCESS != (status = bspcurv_allocateCurve (&curve)))
        return status;

    memcpy (curve.knots, input->uKnots,
            bspknot_numberKnots (curve.params.numPoles, curve.params.order,
                                 curve.params.closed) * sizeof(double));

    inpP = input->poles;
    inwP = input->weights;
    for (i=0; i < input->vParams.numPoles;
         i++, spP += surf.uParams.numPoles, inpP += input->uParams.numPoles)
        {
        memcpy (curve.poles, inpP, curve.params.numPoles * sizeof(DPoint3d));
        if (curve.rational)
            memcpy (curve.weights, inwP, curve.params.numPoles * sizeof(double));

        if (SUCCESS != (status = mdlBspline_openCurve_V7 (&open, &curve, uValue)))
            goto wrapup;

        if (i==0)
            {
            surf.rational = input->rational;
            surf.uParams  = open.params;
            surf.vParams  = input->vParams;
            surf.display  = input->display;
            if (SUCCESS != (status = bspsurf_allocateSurface (&surf)))
                goto wrapup;
            memcpy (surf.uKnots, open.knots,
                    bspknot_numberKnots (surf.uParams.numPoles, surf.uParams.order,
                                         surf.uParams.closed) * sizeof(double));
            memcpy (surf.vKnots, input->vKnots,
                    bspknot_numberKnots (surf.vParams.numPoles, surf.vParams.order,
                                         surf.vParams.closed) * sizeof(double));
            spP = surf.poles;
            swP = surf.weights;
            }
        memcpy (spP, open.poles, surf.uParams.numPoles * sizeof(DPoint3d));
        if (surf.rational)
            {
            memcpy (swP, open.weights, surf.uParams.numPoles * sizeof(double));
            inwP += input->uParams.numPoles;
            swP += surf.uParams.numPoles;
            }
        bspcurv_freeCurve (&open);

        }

    if (SUCCESS != (status = bspsurf_copyBoundaries (&surf, input)))
        goto wrapup;

    if (output == input)
        bspsurf_freeSurface (output);

    *output = surf;

wrapup:
    bspcurv_freeCurve (&curve);
    bspcurv_freeCurve (&open);
    if (status)
        bspsurf_freeSurface (&surf);

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsprsurf_openSurfaceV_V7                                |
|                                                                       |
| author        DavidAssaf                              12/01           |
|                                                                       |
| Copied from bsprsurf.c with call to openCurve changed.                |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      mdlBspline_openSurfaceV_V7
(
MSBsplineSurface    *output,
MSBsplineSurface    *input,
double              vValue
)
    {
    int                 i, status;
    double              *inwP, *swP, *cwP, *endW;
    DPoint3d	    	*inpP, *spP, *cpP, *endP;
    MSBsplineCurve      curve, open;
    MSBsplineSurface    surf;

    if (!input->vParams.closed)
        return bspsurf_copySurface (output, input);

    memset (&curve, 0, sizeof(curve));
    memset (&open, 0, sizeof(open));
    memset (&surf, 0, sizeof(surf));

    curve.rational = input->rational;
    curve.params = input->vParams;
    if (SUCCESS != (status = bspcurv_allocateCurve (&curve)))
        return status;

    memcpy (curve.knots, input->vKnots,
            bspknot_numberKnots (curve.params.numPoles, curve.params.order,
                                 curve.params.closed) * sizeof(double));

    for (i=0; i < input->uParams.numPoles; i++)
        {
        for (cpP=endP=curve.poles, inpP = input->poles+i, endP += curve.params.numPoles;
             cpP < endP; cpP++, inpP += input->uParams.numPoles)
            *cpP = *inpP;

        if (curve.rational)
            for (cwP=endW=curve.weights, inwP = input->weights+i,
                 endW += curve.params.numPoles;
                 cwP < endW; cwP++, inwP += input->uParams.numPoles)
                *cwP = *inwP;

        if (SUCCESS != (status = mdlBspline_openCurve_V7 (&open, &curve, vValue)))
            goto wrapup;

        if (i==0)
            {
            surf.rational = input->rational;
            surf.uParams  = input->uParams;
            surf.vParams  = open.params;
            surf.display  = input->display;
            if (SUCCESS != (status = bspsurf_allocateSurface (&surf)))
                goto wrapup;
            memcpy (surf.uKnots, input->uKnots,
                    bspknot_numberKnots (surf.uParams.numPoles, surf.uParams.order,
                                         surf.uParams.closed) * sizeof(double));
            memcpy (surf.vKnots, open.knots,
                    bspknot_numberKnots (surf.vParams.numPoles, surf.vParams.order,
                                         surf.vParams.closed) * sizeof(double));
            }

        for (spP = surf.poles+i, cpP=endP=open.poles, endP += open.params.numPoles;
             cpP < endP; spP += surf.uParams.numPoles, cpP++)
            *spP = *cpP;

        if (surf.rational)
            for (swP = surf.weights+i, cwP=endW=open.weights, endW += open.params.numPoles;
                cwP < endW; swP += surf.uParams.numPoles, cwP++)
                *swP = *cwP;
        bspcurv_freeCurve (&open);
        }

    if (SUCCESS != (status = bspsurf_copyBoundaries (&surf, input)))
        goto wrapup;

    if (output == input)
        bspsurf_freeSurface (output);

    *output = surf;

wrapup:
    bspcurv_freeCurve (&curve);
    bspcurv_freeCurve (&open);
    if (status)
        bspsurf_freeSurface (&surf);

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_closeCurve_V7                                   |
|                                                                       |
| author        DavidAssaf                              12/01           |
|                                                                       |
| Copied from bspcurv.c with V7 logic restored, with option to ignore   |
| the V7 linear case which doesn't mangle knots.  Treating linears      |
| just like other orders during import ensures that every V7 closed     |
| B-spline curve comes in with mangled knots---which V8 can recognize   |
| and so process accordingly.                                           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      mdlBspline_closeCurve_V7
(
MSBsplineCurve  *outCurve,
MSBsplineCurve  *inCurve,
bool            bUseLinearShift
)
    {
    int             status, numPoles;
    double          wt0, wt1;
    MSBsplineCurve  curve;

    if (inCurve->params.closed)
        return bspcurv_copyCurve (outCurve, inCurve);

    if (inCurve->params.order == 2 && inCurve->params.numPoles == 2)
        return bspcurv_copyCurve (outCurve, inCurve);
 
    numPoles = inCurve->params.numPoles;

    wt0 = inCurve->rational ? inCurve->weights[0] : 1.0;
    wt1 = inCurve->rational ? inCurve->weights[numPoles-1] : 1.0;
    
    if (! bsputil_isSamePoint (inCurve->poles, inCurve->poles+numPoles-1) ||
        ! mdlMath_DEqual (wt0, wt1))
        return (ERROR);

    if (SUCCESS != (status = bspcurv_copyCurve (&curve, inCurve)))
        return status;

    /* revert to v7 knots */
    mdlBspline_normalizeCurveKnots (NULL, NULL, &curve);

    if (bUseLinearShift && curve.params.order == 2)
        {
        /* set first/last knot intervals to penultimate/second knot intervals */ 
        curve.knots[0] = curve.knots[1] - (curve.knots[numPoles] - curve.knots[numPoles-1]);
        curve.knots[numPoles+1] = curve.knots[numPoles] + (curve.knots[2] - curve.knots[1]);

        curve.params.numPoles--;
        curve.params.closed = true;
        
        /* Shift the pole array by one to agree with the knot vector */
	memmove (curve.poles, curve.poles+1, curve.params.numPoles * sizeof(DPoint3d));
        }
    else
        {
        int     numKnots = bspknot_numberKnots (curve.params.numPoles, curve.params.order, curve.params.closed);
        double* pNormalizedKnots = (double*)_alloca (numKnots * sizeof (double));
        double* pKnots = NULL;

        /* need copy of old (normalized) full knot vector for bspknot_computeKnotVector */
        if (NULL == pNormalizedKnots)
            {
            status = ERROR;
            goto wrapup;
            }
        memcpy (pNormalizedKnots, curve.knots, numKnots * sizeof (double));

        /* Fake it!  This produces a problematic knot vector with order
        multiplicity start and end knots---both strictly *inside* the knot
        vector!  Use mdlBspline_curveShouldBeOpened to detect these curves. */
        curve.params.closed = true;
        curve.params.numKnots = curve.params.numPoles - 1;
        numKnots = bspknot_numberKnots (curve.params.numPoles, curve.params.order, curve.params.closed);

        if (NULL == (pKnots = (double*)BSIBaseGeom::Realloc (curve.knots, numKnots * sizeof (double))))
            {
            status = ERROR;
            goto wrapup;
            }
        curve.knots = pKnots;

        bspknot_computeKnotVector (curve.knots, &curve.params, pNormalizedKnots + (curve.params.order + 1) / 2);
        }

    if (outCurve == inCurve)
        bspcurv_freeCurve (outCurve);
    
    status = bspcurv_copyCurve (outCurve, &curve);

wrapup:
    bspcurv_freeCurve (&curve);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsprsurf_closeSurfaceU_V7                               |
|                                                                       |
| author        DavidAssaf                              12/01           |
|                                                                       |
| Copied from bsprsurf.c with call to closeCurve changed.               |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      mdlBspline_closeSurfaceU_V7
(
MSBsplineSurface    *output,
MSBsplineSurface    *input,
bool                bUseLinearShift
)
    {
    int                 i, status;
    double              *inwP, *swP = NULL;
    DPoint3d	    	*inpP, *spP = NULL;
    MSBsplineCurve      curve, open;
    MSBsplineSurface    surf;

    if (input->uParams.closed)
        return bspsurf_copySurface (output, input);

    memset (&curve, 0, sizeof(curve));
    memset (&open, 0, sizeof(open));
    memset (&surf, 0, sizeof(surf));

    curve.rational = input->rational;
    curve.params = input->uParams;
    if (SUCCESS != (status = bspcurv_allocateCurve (&curve)))
        return status;

    memcpy (curve.knots, input->uKnots,
            bspknot_numberKnots (curve.params.numPoles, curve.params.order,
                                 curve.params.closed) * sizeof(double));

    inpP = input->poles;
    inwP = input->weights;
    for (i=0; i < input->vParams.numPoles;
         i++, spP += surf.uParams.numPoles, inpP += input->uParams.numPoles)
        {
        memcpy (curve.poles, inpP, curve.params.numPoles * sizeof(DPoint3d));
        if (curve.rational)
            memcpy (curve.weights, inwP, curve.params.numPoles*sizeof(double));

        if (SUCCESS != (status = mdlBspline_closeCurve_V7 (&open, &curve, bUseLinearShift)))
            goto wrapup;

        if (i==0)
            {
            surf.rational = input->rational;
            surf.uParams  = open.params;
            surf.vParams  = input->vParams;
            surf.display  = input->display;
            if (SUCCESS != (status = bspsurf_allocateSurface (&surf)))
                goto wrapup;
            memcpy (surf.uKnots, open.knots,
                    bspknot_numberKnots (surf.uParams.numPoles, surf.uParams.order,
                                         surf.uParams.closed) * sizeof(double));
            memcpy (surf.vKnots, input->vKnots,
                    bspknot_numberKnots (surf.vParams.numPoles, surf.vParams.order,
                                         surf.vParams.closed) * sizeof(double));
            spP = surf.poles;
            swP = surf.weights;
            }
        memcpy (spP, open.poles, surf.uParams.numPoles * sizeof(DPoint3d));
        if (surf.rational)
            {
            memcpy (swP, open.weights, surf.uParams.numPoles * sizeof(double));
            inwP += input->uParams.numPoles;
            swP  += surf.uParams.numPoles;
            }
        bspcurv_freeCurve (&open);
        }

    if (SUCCESS != (status = bspsurf_copyBoundaries (&surf, input)))
        goto wrapup;

    if (output == input)
        bspsurf_freeSurface (output);

    *output = surf;

wrapup:
    bspcurv_freeCurve (&curve);
    bspcurv_freeCurve (&open);
    if (status)
        bspsurf_freeSurface (&surf);

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsprsurf_closeSurfaceV_V7                               |
|                                                                       |
| author        DavidAssaf                              12/01           |
|                                                                       |
| Copied from bsprsurf.c with call to closeCurve changed.               |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      mdlBspline_closeSurfaceV_V7
(
MSBsplineSurface    *output,
MSBsplineSurface    *input,
bool                bUseLinearShift
)
    {
    int                 i, status;
    double              *inwP, *swP, *cwP, *endW;
    DPoint3d	    	*inpP, *spP, *cpP, *endP;
    MSBsplineCurve      curve, open;
    MSBsplineSurface    surf;

    if (input->vParams.closed)
        return bspsurf_copySurface (output, input);

    memset (&curve, 0, sizeof(curve));
    memset (&open, 0, sizeof(open));
    memset (&surf, 0, sizeof(surf));

    curve.rational = input->rational;
    curve.params = input->vParams;
    if (SUCCESS != (status = bspcurv_allocateCurve (&curve)))
        return status;

    memcpy (curve.knots, input->vKnots,
            bspknot_numberKnots (curve.params.numPoles, curve.params.order,
                                 curve.params.closed) * sizeof(double));

    for (i=0; i < input->uParams.numPoles; i++)
        {
        for (cpP=endP=curve.poles, inpP = input->poles+i,
             endP += curve.params.numPoles;
             cpP < endP; cpP++, inpP += input->uParams.numPoles)
            *cpP = *inpP;

        if (curve.rational)
            for (cwP=endW=curve.weights, inwP = input->weights+i,
                 endW += curve.params.numPoles;
                 cwP < endW; cwP++, inwP += input->uParams.numPoles)
                *cwP = *inwP;

        if (SUCCESS != (status = mdlBspline_closeCurve_V7 (&open, &curve, bUseLinearShift)))
            goto wrapup;

        if (i==0)
            {
            surf.rational = input->rational;
            surf.uParams  = input->uParams;
            surf.vParams  = open.params;
            surf.display  = input->display;
            if (SUCCESS != (status = bspsurf_allocateSurface (&surf)))
                goto wrapup;
            memcpy (surf.uKnots, input->uKnots,
                    bspknot_numberKnots (surf.uParams.numPoles, surf.uParams.order,
                                         surf.uParams.closed) * sizeof(double));
            memcpy (surf.vKnots, open.knots,
                    bspknot_numberKnots (surf.vParams.numPoles, surf.vParams.order,
                                         surf.vParams.closed) * sizeof(double));
            }

        for (spP = surf.poles+i, cpP=endP=open.poles, endP += open.params.numPoles;
             cpP < endP; spP += surf.uParams.numPoles, cpP++)
            *spP = *cpP;

        if (surf.rational)
            for (swP = surf.weights+i, cwP=endW=open.weights,
                 endW += curve.params.numPoles;
                 cwP < endW; swP += surf.uParams.numPoles, cwP++)
                *swP = *cwP;
        bspcurv_freeCurve (&open);
        }

    if (SUCCESS != (status = bspsurf_copyBoundaries (&surf, input)))
        goto wrapup;

    if (output == input)
        bspsurf_freeSurface (output);

    *output = surf;

wrapup:
    bspcurv_freeCurve (&curve);
    bspcurv_freeCurve (&open);
    if (status)
        bspsurf_freeSurface (&surf);

    return status;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
