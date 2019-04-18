/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"
#include <algorithm>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define MDLERR_BADPARAMETER ERROR
#define MDLERR_TOOFEWPOLES ERROR
#define MDLERR_TOOMANYPOLES ERROR
#define MDLERR_INSFMEMORY ERROR
#define MDLERR_BADORDER ERROR

#define BUFFER_SIZE     1024
#define NALLOC          256

#define FLAT_TOL        10
#define MAX_ITER        5
#define MINIMUM_BoundBox    fc_epsilon
#define MAX_CLIPBATCH_MINUS_1 (MAX_CLIPBATCH - 1)

#ifdef COUNT_CALLS

static int s_minDistToCurve = 0;
static int s_closeTan = 0;
static int s_allTans = 0;
static int s_intersectSegment = 0;
static int s_cusp = 0;
static int s_inflections = 0;
static int s_offset = 0;
static int s_parallelTans = 0;
static int s_planeIntesect = 0;
static int s_subdivideZ = 0;
static int s_outputFrequency = 1000;
static void reportCall
(
char *name,
int *pCounter
)
    {
    if ((*pCounter % s_outputFrequency) == 0)
        {
        printf (" %s %d\n", name, *pCounter);
        }
    *pCounter += 1;
    }
#else
#define reportCall(string,couter)
#endif

typedef struct strokeargs
    {
    DPoint3d        *point;
    double          *u;
    int             number;
    } StrokeArgs;


/*---------------------------------------------------------------------------------**//**
// edl june 2017 no callers in geomlibs.
* @bsimethod                                                    Lu.Han          02/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int      mdlBspline_setInterpolationTangents
(
DPoint3dP       pStartTangent,
DPoint3dP       pStartPoint,
DPoint3dP       pStartTangentPoint,
DPoint3dP       pEndTangent,
DPoint3dP       pEndPoint,
DPoint3dP       pEndTangentPoint
)
    {
    if (pStartTangent)
        {
        pStartTangent->DifferenceOf (*pStartTangentPoint, *pStartPoint);
        if (!pStartTangent->IsEqual (DPoint3d::FromZero (), fc_nearZero))
            pStartTangent->Normalize ();
        }

    if (pEndTangent)
        {
        pEndTangent->DifferenceOf (*pEndTangentPoint, *pEndPoint);
        if (!pEndTangent->IsEqual (DPoint3d::FromZero (), fc_nearZero))
            pEndTangent->Normalize ();
        }

    return  SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_validateCurveKnots                           |
|                                                                       |
| author        LuHan                                   09/02           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  StatusInt    mdlBspline_validateCurveKnots
(
double*         pKnots,         // <=> full knots
DPoint3d*       pPoles,         // <=> full weighted poles (optional)
double*         pWeights,       // <=> full weights (optional)
BsplineParam*   pParams         // <=> potentially modified only if poles given
)
    {
    bool        badFullKnots = false;
    int         divisor, numKnots, numInteriorKnots, i;
    double      *pP, *pStart, *pEnd, *pNext, accum, step;

    if (!pKnots || !pParams)
        return ERROR;

    numKnots = bspknot_numberKnots (pParams->numPoles, pParams->order, pParams->closed);

    pStart = pKnots + pParams->order - 1;        // last external knot at beginning of knot vector
    pEnd = pKnots + numKnots - pParams->order;   // first external knot at end of knot vector

    /* verify that the start knot is strictly less than the last order knots */
    for (i = 0; i < pParams->order; i++)
        {
        if (*pStart >= pEnd[i])
            {
            badFullKnots = true;
            break;
            }
        }

    if (badFullKnots)
        {
        /* the whole knot vector is bad; replace it with uniform knots */
        return bspknot_computeKnotVector (pKnots, pParams, NULL);
        }
    else
        {
        numInteriorKnots = numKnots - 2 * pParams->order;

        /* replace any decreasing interior knots with uniformly spaced knots */
        for (pP = pStart + 1; pP < pEnd; pP++)
            {
            if (*pP < *(pP-1))
                {
                /* find the next valid knot */
                divisor = 1;
                for (pNext = pP+1; pNext < pEnd; pNext++)
                    {
                    if (*pNext >= *(pP-1))
                        {
                        break;
                        }
                    divisor++;
                    }
                step = (*pNext - *(pP-1)) / (divisor+1);
                for (pNext = pP, accum = step; pNext < pP + divisor; pNext++)
                    {
                    *pNext = *(pP-1) + accum;
                    accum += step;
                    }
                pP += divisor - 1;
                }
            }
        }

    // check for oversaturated exterior knots in open vector (interior knots that equal an exterior knot)
    if (pPoles && !pParams->closed)
        {
        int nExcess0, nExcess1;

        double tol = (*pEnd - *pStart) * RELATIVE_BSPLINE_EXT_KNOT_TOLERANCE;

        for (i = pParams->order, nExcess0 = 0; i < numKnots; i++, nExcess0++)
            if (pKnots[i] - *pStart >= tol)
                break;

        for (i = numKnots - pParams->order - 1, nExcess1 = 0; i >= 0; i--, nExcess1++)
            if (*pEnd - pKnots[i] >= tol)
                break;

        // remove excess knots/poles
        if (nExcess0 > 0 || nExcess1 > 0)
            {
            // TR #170011: don't throw out the exterior knots and pole; they are likely golden!
            if (nExcess0 > 0)
                {
                memmove (pKnots + pParams->order, pKnots + pParams->order + nExcess0, (numKnots - pParams->order - nExcess0) * sizeof (*pKnots));
                memmove (pPoles + 1, pPoles + 1 + nExcess0, (pParams->numPoles - 1 - nExcess0) * sizeof (*pPoles));
                if (pWeights)
                    memmove (pWeights + 1, pWeights + 1 + nExcess0, (pParams->numPoles - 1 - nExcess0) * sizeof (*pWeights));

                numKnots          -= nExcess0;
                pParams->numKnots -= nExcess0;
                pParams->numPoles -= nExcess0;
                }
            if (nExcess1 > 0)
                {
                memmove (pKnots + numKnots - pParams->order - nExcess1, pKnots + numKnots - pParams->order, pParams->order * sizeof (*pKnots));
                memmove (pPoles + pParams->numPoles - 1 - nExcess1, pPoles + pParams->numPoles - 1, sizeof (*pPoles));
                if (pWeights)
                    memmove (pWeights + pParams->numPoles - 1 - nExcess1, pWeights + pParams->numPoles - 1, sizeof (*pWeights));

                numKnots          -= nExcess1;
                pParams->numKnots -= nExcess1;
                pParams->numPoles -= nExcess1;
                }
            }
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|									|
| name		bspproc_processSurfaceByCurves				|
|		Process a B-spline surface by operating on		|
|		each row/column of poles as a curve.			|
|		processFunc : receives a curve formed from each	    	|
|			      row/column of the pole array		|
|		direction   : if BSSURF_U process rows of pole array	|
|		    	      if BSSURF_V process columns of array	|
|									|
| author	BSI					8/91		|
|									|
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  int	bspproc_processSurfaceByCurves
(
MSBsplineSurface    	*surface,
PFBCurveVoidPInt       processFunc, /* => acts on row/column curves of surface */
void	    	    	*args, 	    	    	
int 	    	    	direction
)
    {
    int	    	    	i, numCurves, major = 1, minor = 1, status, numPoles = 0;
    double  	    	**knotP, *surfWtP = NULL, *srfWP, *crvWP, *endW;
    DPoint3d	    	*surfPoleP = NULL, *srfPP, *crvPP, *endP;
    MSBsplineCurve  	curve;
    MSBsplineSurface	surf;
    
    memset (&curve, 0, sizeof(curve));
    memset (&surf, 0, sizeof(surf));

    numCurves = direction == BSSURF_U ?
	    	surface->vParams.numPoles :
		surface->uParams.numPoles;

    for (i=0; i < numCurves; i++, surfPoleP += major, surfWtP += major)
	{
	if (SUCCESS != (status = bspconv_getCurveFromSurface (&curve, surface, direction, i)))
    	    goto wrapup;

	if (SUCCESS != (status = (*processFunc) (&curve, args, i)))
	    goto wrapup;

	if (i==0)
	    {
    	    surf = *surface;
	    surf.poles = NULL;
	    surf.weights = surf.uKnots = surf.vKnots = NULL;
	    surf.boundaries = NULL;
	    numPoles = curve.params.numPoles;
    	    if (direction == BSSURF_U)
    	    	{
	    	surf.uParams = curve.params;
	    	knotP = &surf.uKnots;
		major = surf.uParams.numPoles;
		minor = 1;
	    	}
    	    else 
    	    	{
	    	surf.vParams = curve.params;
	    	knotP = &surf.vKnots;
		major = 1;
		minor = surf.uParams.numPoles;
	    	}

	    if (SUCCESS != (status = bspsurf_allocateSurface (&surf)))
	    	goto wrapup;

	    surfPoleP = surf.poles;
	    surfWtP   = surf.weights;

	    /* Only copy new knot vector first time */
	    memcpy (*knotP, curve.knots,
		    bspknot_numberKnots (curve.params.numPoles, curve.params.order,
					 curve.params.closed) * sizeof(double));
	    }

	/* copy poles & weights to surf */
	for (crvPP = endP = curve.poles, srfPP=surfPoleP, endP += numPoles;
	     crvPP < endP; srfPP += minor, crvPP++)
	    *srfPP = *crvPP;
	if (surf.rational)
	    for (crvWP = endW = curve.weights, srfWP=surfWtP, endW += numPoles;
		 crvWP < endW; srfWP += minor, crvWP++)
	    	*srfWP = *crvWP;

	bspcurv_freeCurve (&curve);
	}

    /* Copy unchanged knot vector & boundaries */
    if (direction == BSSURF_U)
	{
	memcpy (surf.vKnots, surface->vKnots,
		bspknot_numberKnots (surf.vParams.numPoles, surf.vParams.order,
				     surf.vParams.closed) * sizeof(double));
	}
    else
	{
	memcpy (surf.uKnots, surface->uKnots,
		bspknot_numberKnots (surf.uParams.numPoles, surf.uParams.order,
				     surf.uParams.closed) * sizeof(double));
	}

    if (SUCCESS != (status = bspsurf_copyBoundaries (&surf, surface)))
	goto wrapup;

    bspsurf_freeSurface (surface);
    *surface = surf;

wrapup:
    if (status)
	bspsurf_freeSurface (&surf);
    bspcurv_freeCurve (&curve);
    return status;
    }


/*----------------------------------------------------------------------+
|									|
| name		bspconv_getCurveFromSurface				|
|									|
| author	BSI					8/91		|
|									|
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  int	bspconv_getCurveFromSurface
(
MSBsplineCurve	    *curve,
MSBsplineSurfaceCP   surface,
int 	    	    direction,
int 	    	    which	       /* => -1 mean last row/column */
)
    {
    int	    	offset, incr;
    double  	*knotPtr, *cvWP, *sfWP, *endW;
    DPoint3d	*cvPP, *sfPP, *endP;

    if (direction == BSSURF_U)
	{
	if (which == -1)
	    which = surface->vParams.numPoles - 1;
	else if (which < 0 || which > surface->vParams.numPoles - 1)
	    return (MDLERR_BADPARAMETER);
	curve->params = surface->uParams;
	knotPtr = surface->uKnots;
	offset = which * surface->uParams.numPoles;
	incr = 1;
	}
    else
	{
	if (which == -1)
	    which = surface->uParams.numPoles - 1;
	else if (which < 0 || which > surface->uParams.numPoles - 1)
	    return (MDLERR_BADPARAMETER);
	curve->params = surface->vParams;
	knotPtr = surface->vKnots;
	offset = which;
	incr = surface->uParams.numPoles;
	}

    curve->rational = surface->rational;
    curve->display = surface->display;

    if (bspcurv_allocateCurve (curve))
	return MDLERR_INSFMEMORY;

    /* Copy poles & weights if rational */
    for (cvPP=endP=curve->poles, sfPP=surface->poles + offset,
    	 endP += curve->params.numPoles;
         cvPP < endP;
	 cvPP++, sfPP += incr)
        *cvPP = *sfPP;

    if (curve->rational)
        for (cvWP=endW=curve->weights, sfWP=surface->weights + offset,
    	     endW += curve->params.numPoles;
             cvWP < endW; cvWP++, sfWP += incr)
	    *cvWP = *sfWP;

    /* Copy knot vector */
    memcpy (curve->knots, knotPtr,
	    bspknot_numberKnots (curve->params.numPoles, curve->params.order,
    	    	    	    	 curve->params.closed) * sizeof(double));
    return (SUCCESS);
    }

/*----------------------------------------------------------------------+
|									|
| name		bspconv_getEdgeFromSurface				|
|									|
|		1.0 +----------+ 					|
|		    |    V1    | 					|
|		    |	       | 					|
|		    |	       | 					|
|		 v  |U0      U1| 					|
|		    |	       | 					|
|		    |	       | 					|
|		    |    V0    |					|
|		0.0 +----------+					|
|		   0.0    u   1.0					|
|									|
| author	BrianPeters				6/93		|
|									|
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  int      bspconv_getEdgeFromSurface
(
MSBsplineCurve      *curve,
MSBsplineSurfaceCP  surf,
int		    edgeCode
)
    {
    int		direction = BSSURF_U, whichSide = 0;

    memset (curve, 0, sizeof(MSBsplineCurve));

    switch (edgeCode)
	{
	case U0_EDGE:
	    direction = BSSURF_V;
	    whichSide = 0;
	    break;
	case U1_EDGE:
	    direction = BSSURF_V;
	    whichSide = -1;
	    break;
	case V0_EDGE:
	    direction = BSSURF_U;
	    whichSide = 0;
	    break;
	case V1_EDGE:
	    direction = BSSURF_U;
	    whichSide = -1;
	    break;
	}

    return bspconv_getCurveFromSurface (curve, surf, direction, whichSide);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          09/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  void    bspconv_extractWeightValues
(
double          *weights,
double          *igdsWeights,
int             numPoles
)
    {
    static double s_minWeight = 1.0e-20;

    double      *lP, *wP, *endP;

    // The test for identity rules out QNAN floating point values.
    // Such a value has been observed in the third weight of a uniform B-spline curve header whose
    // children have had their "symbology" changed without regard to the ehdr.isGraphics bit.
    for (lP = igdsWeights, wP = endP = weights, endP += numPoles; wP < endP; lP++, wP++)
        {
        if (BeNumerical::BeIsnan (*lP) || *lP < s_minWeight)
            *wP = 1.0;
        else
            *wP = *lP;
        }
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bsprsurf_closeSurfaceU                                  |
|                                                                       |
| author        BFP                                     11/90           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bsprsurf_closeSurfaceU
(
MSBsplineSurface    *output,
MSBsplineSurfaceCP   input
)
#if defined (needs_work)
/* This could be speeded up substantially by not repeatedly calling
    bspcurv_closeCurve. Each call has the same uValue and knot vector.
    Keep track of needed info here and avoid all the repetition.
    This will do for now. */
#endif
    {
    int                 i, status;
    double              *inwP, *swP = NULL;
    DPoint3d            *inpP, *spP = NULL;
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

        if (SUCCESS != (status = bspcurv_closeCurve (&open, &curve)))
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

        if (surf.uParams.numPoles != open.params.numPoles)
            goto wrapup;

        memcpy (spP, open.poles, surf.uParams.numPoles * sizeof(DPoint3d));
        if (surf.rational)
            {
            memcpy (swP, open.weights, surf.uParams.numPoles * sizeof(double));
            inwP += input->uParams.numPoles;
            swP  += surf.uParams.numPoles;
            }
        bspcurv_freeCurve (&open);
        }

#if defined (NEEDS_WORK)
    /* Need to close the surface boundaries proprely.
        Now it will work if uValue == 0.0 */
#endif
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
| name          bsprsurf_closeSurfaceV                                  |
|                                                                       |
| author        BFP                                     11/90           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bsprsurf_closeSurfaceV
(
MSBsplineSurface    *output,
MSBsplineSurfaceCP   input
)
    {
    int                 i, status;
    double              *inwP, *swP, *cwP, *endW;
    DPoint3d            *inpP, *spP, *cpP, *endP;
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

        if (SUCCESS != (status = bspcurv_closeCurve (&open, &curve)))
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

        if (surf.vParams.numPoles != open.params.numPoles)
            goto wrapup;

        for (spP = surf.poles+i, cpP=endP=open.poles, endP += open.params.numPoles;
             cpP < endP; spP += surf.uParams.numPoles, cpP++)
            *spP = *cpP;

        if (surf.rational)
            for (swP = surf.weights+i, cwP=endW=open.weights,
                 endW += open.params.numPoles;
                 cwP < endW; swP += surf.uParams.numPoles, cwP++)
                *swP = *cwP;
        bspcurv_freeCurve (&open);
        }

#if defined (NEEDS_WORK)
    /* Need to close the surface boundaries proprely.
        Now it will work if uValue == 0.0 */
#endif
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
| name          bsprsurf_closeSurface                                   |
|                                                                       |
| author        BFP                                     10/90           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  int      bsprsurf_closeSurface
(
MSBsplineSurface    *output,
MSBsplineSurfaceCP   input,
int                 edge
)
    {
    if (edge == BSSURF_U)
        return bsprsurf_closeSurfaceU (output, input);
    else
        return bsprsurf_closeSurfaceV (output, input);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsprsurf_openSurfaceU                                   |
|                                                                       |
| author        BFP                                     11/90           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bsprsurf_openSurfaceU
(
MSBsplineSurface    *output,
MSBsplineSurfaceCP   input,
double              uValue
)
#if defined (needs_work)
/* This could be speeded up substantially by not repeatedly calling
    bspcurv_openCurve. Each call has the same uValue and knot vector.
    Keep track of needed info here and avoid all the repetition.
    This will do for now. */
#endif
    {
    int                 i, status;
    double              *inwP, *swP = NULL;
    DPoint3d            *inpP, *spP = NULL;
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

        if (SUCCESS != (status = bspcurv_openCurve (&open, &curve, uValue)))
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

#if defined (NEEDS_WORK)
    /* Need to open the surface boundaries proprely.
        Now it will work if uValue == 0.0 */
#endif
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
| name          bsprsurf_openSurfaceV                                   |
|                                                                       |
| author        BFP                                     11/90           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bsprsurf_openSurfaceV
(
MSBsplineSurface    *output,
MSBsplineSurfaceCP   input,
double              vValue
)
    {
    int                 i, status;
    double              *inwP, *swP, *cwP, *endW;
    DPoint3d            *inpP, *spP, *cpP, *endP;
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

        if (SUCCESS != (status = bspcurv_openCurve (&open, &curve, vValue)))
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

#if defined (NEEDS_WORK)
    /* Need to open the surface boundaries proprely.
        Now it will work if uValue == 0.0 */
#endif
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
| name          bsprsurf_openSurface                                    |
|                                                                       |
| author        BFP                                     10/90           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  int      bsprsurf_openSurface
(
MSBsplineSurface    *output,
MSBsplineSurfaceCP   input,
double              uvValue,
int                 edge
)
    {
    if (edge == BSSURF_U)
        return bsprsurf_openSurfaceU (output, input, uvValue);
    else
        return bsprsurf_openSurfaceV (output, input, uvValue);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsprsurf_makeRowBezier                                  |
|                                                                       |
| author        BFP                                     11/90           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bsprsurf_makeRowBezier
(
MSBsplineSurface    *output,           /* <= already allocated */
MSBsplineSurfaceCP   input,            /* => original surface */
double              *distinctKnots,
int                 *knotMultiplicity,
int                 numDistinct,
double              knotTolerance
)
    {
    int                 i, row, status, numKnots, degree;
    double              *inwP, *outwP;
    DPoint3d            *inpP, *outpP;
    MSBsplineCurve      curve;

    memset (&curve, 0, sizeof (curve));
    curve.rational = input->rational;
    curve.params = input->uParams;
    degree = curve.params.order - 1;

    if (SUCCESS != (status = bspcurv_allocateCurve (&curve)))
        return status;
    numKnots = bspknot_numberKnots (input->uParams.numPoles, input->uParams.order,
                                    input->uParams.closed);

    inpP = input->poles;    inwP = input->weights;
    outpP = output->poles;  outwP = output->weights;
    for (row=0; row < input->vParams.numPoles;
         row++,
         inpP += input->uParams.numPoles, inwP += input->uParams.numPoles,
         outpP += curve.params.numPoles,  outwP += curve.params.numPoles)
        {
        curve.params = input->uParams;
        memcpy (curve.poles, inpP, input->uParams.numPoles * sizeof(DPoint3d));
        memcpy (curve.knots, input->uKnots, numKnots * sizeof(double));
        if (curve.rational)
            memcpy (curve.weights, inwP, input->uParams.numPoles * sizeof (double));

        for (i=0; i < numDistinct; i++)
            {
            if (knotMultiplicity[i] < degree)
                {
                if (SUCCESS != (status = bspknot_addKnot (&curve, distinctKnots[i],
                                                          knotTolerance, degree, false)))
                    goto wrapup;
                }
            }

        memcpy (outpP, curve.poles, curve.params.numPoles * sizeof(DPoint3d));
        if (curve.rational)
            memcpy (outwP, curve.weights, curve.params.numPoles * sizeof (double));
        }

    numKnots = bspknot_numberKnots (curve.params.numPoles, curve.params.order,
                                    curve.params.closed);
    memcpy (output->uKnots, curve.knots, numKnots * sizeof (double));
    output->uParams = curve.params;

    numKnots = bspknot_numberKnots (input->vParams.numPoles, input->vParams.order,
                                    input->vParams.closed);
    memcpy (output->vKnots, input->vKnots, numKnots * sizeof (double));
    output->vParams = input->vParams;

wrapup:
    bspcurv_freeCurve (&curve);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsprsurf_makeColumnBezier                               |
|                                                                       |
| author        BFP                                     11/90           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bsprsurf_makeColumnBezier
(
MSBsplineSurface    *output,           /* <= already Bezier in rows */
BsplineParam  const *origVParams,     /* => original vParams */
double              *distinctKnots,
int                 *knotMultiplicity,
int                 numDistinct,
double              knotTolerance
)
    {
    int                 i, col, status, numKnots, degree;
    double              *wp, *outwP, *cwP, *endW;
    DPoint3d            *pP, *outpP, *cpP, *endP;
    MSBsplineCurve      curve;

    memset (&curve, 0, sizeof (curve));
    curve.rational = output->rational;
    curve.params = *origVParams;
    degree = curve.params.order - 1;

    if (SUCCESS != (status = bspcurv_allocateCurve (&curve)))
        return status;
    numKnots = bspknot_numberKnots (origVParams->numPoles,  origVParams->order,
                                    origVParams->closed);

    for (col=0; col < output->uParams.numPoles; col++)
        {
        curve.params = *origVParams;
        for (cpP=endP=curve.poles, pP=output->poles+col, endP += curve.params.numPoles;
             cpP < endP; cpP++, pP += output->uParams.numPoles)
            *cpP = *pP;

        memcpy (curve.knots, output->vKnots, numKnots * sizeof(double));

        if (curve.rational)
            for (cwP=endW=curve.weights, wp=output->weights+col,
                 endW += curve.params.numPoles;
                 cwP < endW; cwP++, wp += output->uParams.numPoles)
                *cwP = *wp;

        for (i=0; i < numDistinct; i++)
            {
            if (knotMultiplicity[i] < degree)
                {
                if (SUCCESS != (status = bspknot_addKnot (&curve, distinctKnots[i],
                                                          knotTolerance, degree, false)))
                    goto wrapup;
                }
            }

        for (cpP=endP=curve.poles, outpP=output->poles+col,
             endP += curve.params.numPoles;
             cpP < endP; cpP++, outpP += output->uParams.numPoles)
            *outpP = *cpP;
        if (curve.rational)
            for (cwP=endW=curve.weights, outwP=output->weights+col,
                 endW += curve.params.numPoles;
                 cwP < endW; cwP++, outwP += output->uParams.numPoles)
                *outwP = *cwP;
        }

    numKnots = bspknot_numberKnots (curve.params.numPoles, curve.params.order,
                                    curve.params.closed);
    memcpy (output->vKnots, curve.knots, numKnots * sizeof (double));
    output->vParams = curve.params;

wrapup:
    bspcurv_freeCurve (&curve);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsprsurf_makeBezierSurface                              |
|                                                                       |
| author        BFP                                     11/90           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  int      bsprsurf_makeBezierSurface
(
MSBsplineSurface    *outSurface,
MSBsplineSurfaceCP   inSurface
)
    {
    int                 i, uBezier, vBezier, uNumDist, vNumDist,
                        uAddNum, vAddNum, degree, status=SUCCESS,
                        *uKnotMult, *vKnotMult;
    double              uKnotTol, vKnotTol, *uDistKnots, *vDistKnots;
    MSBsplineCurve      curve;
    MSBsplineSurface    surf;

    memset (&surf, 0, sizeof (surf));
    memset (&curve, 0, sizeof (curve));
    uKnotMult = vKnotMult = NULL;
    uDistKnots = vDistKnots = NULL;

    /* Check if already simple Bezier */
    uBezier = (inSurface->uParams.order == 2)  ||
              ((inSurface->uParams.numPoles == inSurface->uParams.order) &&
              !inSurface->uParams.closed);
    vBezier = (inSurface->vParams.order == 2)  ||
              ((inSurface->vParams.numPoles == inSurface->vParams.order) &&
              !inSurface->vParams.closed);

    if (uBezier && vBezier)
        return bspsurf_copySurface (outSurface, inSurface);

    uNumDist = bspknot_numberKnots (inSurface->uParams.numPoles,inSurface->uParams.order,
                                    inSurface->uParams.closed);
    if (NULL == (uKnotMult  = (int*)msbspline_malloc (uNumDist * sizeof(int), HEAPSIG_BSRF)) ||
        NULL == (uDistKnots = (double*)msbspline_malloc (uNumDist * sizeof(double), HEAPSIG_BSRF)))
        {
        status = MDLERR_INSFMEMORY;
        goto wrapup;
        }

    vNumDist = bspknot_numberKnots (inSurface->vParams.numPoles, inSurface->vParams.order,
                                    inSurface->vParams.closed);
    if (NULL == (vKnotMult  = (int*)msbspline_malloc (vNumDist * sizeof(int), HEAPSIG_BSRF)) ||
        NULL == (vDistKnots = (double*)msbspline_malloc (vNumDist * sizeof(double), HEAPSIG_BSRF)))
        {
        status = MDLERR_INSFMEMORY;
        goto wrapup;
        }

    /* Figure out size of finished surface */
    curve.params = inSurface->uParams;
    curve.knots = inSurface->uKnots;
    uKnotTol = fc_epsilon;
    bspknot_getKnotMultiplicity (uDistKnots, uKnotMult, &uNumDist, inSurface->uKnots,
                                 inSurface->uParams.numPoles, inSurface->uParams.order,
                                 inSurface->uParams.closed, uKnotTol);
    for (i=uAddNum=0, degree=inSurface->uParams.order-1; i < uNumDist-1; i++)
        if (uKnotMult[i] < degree) uAddNum += (degree - uKnotMult[i]);

    curve.params = inSurface->vParams;
    curve.knots = inSurface->vKnots;
    vKnotTol = fc_epsilon;
    bspknot_getKnotMultiplicity (vDistKnots, vKnotMult, &vNumDist, inSurface->vKnots,
                                 inSurface->vParams.numPoles, inSurface->vParams.order,
                                 inSurface->vParams.closed, vKnotTol);
    for (i=vAddNum=0, degree=inSurface->vParams.order-1; i < vNumDist-1; i++)
        if (vKnotMult[i] < degree) vAddNum += (degree - vKnotMult[i]);

    if (uAddNum+vAddNum)
        {
        surf = *inSurface;
        surf.uParams.numPoles += uAddNum;
        surf.vParams.numPoles += vAddNum;

        if (SUCCESS != (status = bspsurf_allocateSurface (&surf)))
             goto wrapup;

        if (uAddNum)
            {
            if (SUCCESS != (status = bsprsurf_makeRowBezier (&surf, inSurface, uDistKnots,
                                                             uKnotMult, uNumDist, uKnotTol)))
                goto wrapup;
            }
        else
            {
            /* surf is already Bezier in rows so just copy data */
            memcpy (surf.poles, inSurface->poles,
                   (inSurface->uParams.numPoles * inSurface->vParams.numPoles)
                   * sizeof(DPoint3d));
            memcpy (surf.uKnots, inSurface->uKnots,
                    bspknot_numberKnots (inSurface->uParams.numPoles,
                    inSurface->uParams.order, inSurface->uParams.closed)
                    * sizeof(double));
            memcpy (surf.vKnots, inSurface->vKnots,
                    bspknot_numberKnots (inSurface->vParams.numPoles,
                    inSurface->vParams.order, inSurface->vParams.closed)
                    * sizeof(double));
            if (surf.rational)
                memcpy (surf.weights, inSurface->weights,
                        (inSurface->uParams.numPoles * inSurface->vParams.numPoles)
                       * sizeof(double));
            }

        if (vAddNum)
            {
            if (SUCCESS != (status = bsprsurf_makeColumnBezier (&surf, &inSurface->vParams,
                                              vDistKnots, vKnotMult, vNumDist, vKnotTol)))
                goto wrapup;
            }

        /* Need to copy boundary information */
        bspsurf_copyBoundaries (&surf, inSurface);

        if (outSurface == inSurface)
            bspsurf_freeSurface (outSurface);
        *outSurface = surf;
        }
    else
        {
        status = bspsurf_copySurface (outSurface, inSurface);
        }

wrapup:
    if (uKnotMult)          msbspline_free (uKnotMult);
    if (uDistKnots)         msbspline_free (uDistKnots);
    if (vKnotMult)          msbspline_free (vKnotMult);
    if (vDistKnots)         msbspline_free (vDistKnots);
    if (status)
        bspsurf_freeSurface (&surf);
    return status;
    }

typedef struct elevateParams
    {
    int             order;
    int             numPoles;
    int             numKnots;
    double          *knots;
    double          *knotsMat;
    } ElevateParams;

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsprsurf_matrixElevateDegree                            |
|                                                                       |
| author        LuHan                                   1/93            |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bsprsurf_matrixElevateDegree
(
ElevateParams   *eP,
MSBsplineCurve  *inCurve,
int             newDegree
)
    {
    int             i,j,k, newN, newOrder, newSquare, f, temp, status, itmp;
    double          *symi=NULL, symiMem[NALLOC], delta[MAX_ORDER],
                    *newKnotVector=NULL, knotTolerance;
    bool            symiAlloc = false;
    MSBsplineCurve  curve;

    if (SUCCESS != (status = bspcurv_copyCurve (&curve, inCurve)))
        return status;

    if (curve.params.closed)
        {
        if (SUCCESS != (status = bspcurv_openCurve (&curve, &curve, 0.0)))
            goto wrapup;
        }

    knotTolerance = bspknot_knotTolerance (&curve);

    if (SUCCESS != (status = bspknot_increaseKnotDegree (&newKnotVector, &newN,
                                                         newDegree, curve.knots,
                                                         curve.params.numPoles,
                                                         curve.params.order-1,
                                                         curve.params.closed,
                                                         knotTolerance)))
        goto wrapup;

    newOrder = newDegree + 1;

    newSquare = newOrder*newOrder;
    temp = newSquare * (newN + 1);
    if (temp > NALLOC)
        {
        symiAlloc = true;
        symi = (double*)msbspline_malloc ((unsigned)(temp * sizeof(double)), HEAPSIG_BSRF);
        if (!symi)
            {
            status = MDLERR_INSFMEMORY;
            goto wrapup;
            }
        }
    else
        {
        symi = &symiMem[0];
        }

    for (i=0 ; i <= newN; i++)
        {
        /* we now form a real valued array from the new knot values */
        for (j=0; j < newDegree; j++)
            delta[j] = newKnotVector[i+j] - newKnotVector[i+newDegree];

        if (SUCCESS != (status = bspcurv_generateSymmetricFunctions (&symi[i*newSquare],
                                                                     delta, newDegree)))
            goto wrapup;
        }

    eP->order = newOrder;
    eP->numPoles = newN+1;
    eP->numKnots = bspknot_numberKnots (eP->numPoles, eP->order,
                                        curve.params.closed) -  2*newOrder;


    temp = newOrder * newN * sizeof(double);
    if (NULL == (eP->knotsMat = (double*)msbspline_malloc (temp,HEAPSIG_BSRF)))
        {
        status = MDLERR_INSFMEMORY;
        goto wrapup;
        }

    for (i=1; i <= newN; i++)
        {
        temp = (i-1)*newOrder;
        itmp = (i+1)*newSquare-1;
        for (j=0; j <= newDegree; j++)
            {
            f =  (j%2 == 1) ? -1 : 1;
            eP->knotsMat[temp+j] = f * symi[itmp-j];
            for (k=j+1; k <= newDegree; k++)
                eP->knotsMat[temp+j] /= k;
            }
        }

    temp = bspknot_numberKnots (newN+1, newOrder, curve.params.closed) *
                                sizeof(double);

    if (NULL == (eP->knots = (double*)msbspline_malloc (temp,HEAPSIG_BSRF)))
        {
        status = MDLERR_INSFMEMORY;
        goto wrapup;
        }

    memcpy (eP->knots, newKnotVector, temp);

wrapup:
    bspcurv_freeCurve (&curve);

    if (symiAlloc && symi)
        msbspline_free (symi);

    if (newKnotVector)
        msbspline_free (newKnotVector);

    return status;
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          bsprsurf_elevateDegreeProcess                           |
|                                                                       |
| author        LuHan                                   1/93            |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bsprsurf_elevateDegreeProcess
(
MSBsplineCurve          *inCurve,
void                    *voidP, // Really ElevateParams
int                     dummy
)
    {
    ElevateParams           *eP = (ElevateParams*)voidP;
    int         i, j, f, closed, status, temp, numDerivatives, newDegree,
                iitmp, jtmp, tmp;
    double      firstWeight  = 1.0, sumWts, *dervWts=NULL, dervWtsMem[NALLOC];
    bool        dervAlloc = false;
    DPoint3d    firstPoint, sumPoles, *dervPoles=NULL, dervPolesMem[NALLOC];
    MSBsplineCurve      curve;

    newDegree = eP->order-1;
    if (SUCCESS != (status = bspcurv_copyCurve (&curve, inCurve)))
        return status;

    closed = inCurve->params.closed;
    if (curve.params.closed)
        {
        if (SUCCESS != (status = bspcurv_openCurve (&curve, &curve, 0.0)))
            goto wrapup;
        }

    firstPoint = curve.poles[0];
    if (curve.rational)
        firstWeight = curve.weights[0];

    temp = eP->numPoles * eP->order;
    if (temp > NALLOC)
        {
        dervAlloc = true;
        dervPoles = (DPoint3d*)msbspline_malloc ((unsigned)(temp*sizeof(DPoint3d)), HEAPSIG_BSRF);
        dervWts =   (double*)msbspline_malloc ((unsigned)(temp * sizeof(double)), HEAPSIG_BSRF);
        if (!dervPoles || !dervWts)
            {
            status = MDLERR_INSFMEMORY;
            goto wrapup;
            }
        }
    else
        {
        dervPoles = &dervPolesMem[0];
        dervWts = &dervWtsMem[0];
        }

    numDerivatives = newDegree;

    for (i=0 ; i < eP->numPoles; i++)
        if (SUCCESS != (status = bspcurv_computeDerivatives (dervPoles+i*eP->order,
                                                             dervWts + i*eP->order,
                                                             &curve, numDerivatives,
                                                             eP->knots[i + eP->order-1],
                                                             true)))
            goto wrapup;

    bspcurv_freeCurve(inCurve);

    *inCurve = curve;
    inCurve->params.order = eP->order;
    inCurve->params.numPoles = eP->numPoles;
    inCurve->params.numKnots = eP->numKnots;

    if (SUCCESS != (status = bspcurv_allocateCurve (inCurve)))
        goto wrapup;

    inCurve->poles[eP->numPoles-1] = curve.poles[curve.params.numPoles-1];
    if (inCurve->rational)
        inCurve->weights[eP->numPoles-1] =
                  curve.weights[curve.params.numPoles-1];

    for (i=1; i < eP->numPoles; i++)
        {
        tmp = i*eP->order;
        temp = tmp-eP->order;

        /* Sum the poles and weights (if rational) */
        sumPoles.x = sumPoles.y = sumPoles.z = sumWts = 0.0;
        for (j=0; j <= newDegree; j++)
            {
            f = ((newDegree - j) % 2 == 1) ? -1 : 1;
            jtmp = temp+newDegree - j;
            iitmp = tmp+j;
            sumPoles.x += f * eP->knotsMat[jtmp] * dervPoles[iitmp].x;
            sumPoles.y += f * eP->knotsMat[jtmp] * dervPoles[iitmp].y;
            sumPoles.z += f * eP->knotsMat[jtmp] * dervPoles[iitmp].z;
            if (inCurve->rational)
                sumWts += f * eP->knotsMat[jtmp] * dervWts[iitmp];
            }

        inCurve->poles[i-1] = sumPoles;
        if (inCurve->rational)
            inCurve->weights[i-1] = sumWts;
        }

    /*Reassige original first pole */
    if (!inCurve->params.closed)
        {
        inCurve->poles[0] = firstPoint;
        if (inCurve->rational)
            inCurve->weights[0] = firstWeight;
        }

    memcpy (inCurve->knots, eP->knots,
            bspknot_numberKnots (inCurve->params.numPoles,
                                 inCurve->params.order,
                                 inCurve->params.closed) * sizeof(double));

    if (closed)
        status = bspcurv_closeCurve (inCurve, inCurve);

wrapup:
    bspcurv_freeCurve (&curve);

    if (dervAlloc && dervPoles)
        msbspline_free (dervPoles);

    if (dervAlloc && dervWts)
        msbspline_free (dervWts);

    return status;
    }
    
/*----------------------------------------------------------------------+
|                                                                       |
| name          bsprsurf_elevateDegreeU                                 |
|                                                                       |
| author        BFP, LuHan                             11/90, 1/93      |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bsprsurf_elevateDegreeU
(
MSBsplineSurface    *output,
MSBsplineSurfaceCP   input,
int                 newDegree
)
    {
    int                 numKnots, status;
    MSBsplineCurve      curve;
    MSBsplineSurface    surf;
    ElevateParams       eP;

    if (newDegree > MAX_BSORDER-1)
        return MDLERR_BADORDER;

    if (newDegree <= input->uParams.order - 1)
        {
        if (newDegree == input->uParams.order - 1)
            return bspsurf_copySurface (output, input);
        else
            return MDLERR_BADORDER;
        }

    memset (&curve,0, sizeof(curve));
    memset (&surf,0, sizeof(surf));

    curve.rational = input->rational;
    curve.params = input->uParams;
    if (SUCCESS != (status = bspcurv_allocateCurve (&curve)))
        return status;

    memcpy (curve.poles, input->poles,
            curve.params.numPoles * sizeof(DPoint3d));

    if (curve.rational)
        memcpy (curve.weights, input->weights,
                curve.params.numPoles * sizeof(double));

    numKnots = bspknot_numberKnots (curve.params.numPoles,
                                    curve.params.order,
                                    curve.params.closed);
    memcpy (curve.knots, input->uKnots, numKnots * sizeof(double));

    if (SUCCESS != (status = bsprsurf_matrixElevateDegree (&eP, &curve, newDegree)))
        goto wrapup;

    /* Make a copy of input surface */
    if (SUCCESS != (status = bspsurf_copySurface (&surf, input)))
        return status;

    /* Use process func to elevate degree of each row */
    if (SUCCESS != (status = bspproc_processSurfaceByCurves (&surf,
                                                             bsprsurf_elevateDegreeProcess,
                                                             &eP, BSSURF_U)))
        goto wrapup;

    if (output == input)
        bspsurf_freeSurface (output);

    *output = surf;

wrapup:
    bspcurv_freeCurve (&curve);
    if (status)
        bspsurf_freeSurface (&surf);
    if (NULL != eP.knotsMat)
        msbspline_free (eP.knotsMat);
    if (NULL != eP.knots)
        msbspline_free (eP.knots);

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsprsurf_elevateDegreeV                                 |
|                                                                       |
| author        BFP, LuHan                             11/90, 1/93      |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bsprsurf_elevateDegreeV
(
MSBsplineSurface    *output,
MSBsplineSurfaceCP   input,
int                 newDegree
)
    {
    int                 numKnots, status;
    double              *inwP, *cwP, *endW;
    DPoint3d            *inpP, *cpP, *endP;
    MSBsplineCurve      curve;
    MSBsplineSurface    surf;
    ElevateParams       eP;

    if (newDegree > MAX_BSORDER-1)
        return MDLERR_BADORDER;

    if (newDegree <= input->vParams.order - 1)
        {
        if (newDegree == input->vParams.order - 1)
            return bspsurf_copySurface (output, input);
        else
            return MDLERR_BADORDER;
        }

    memset (&curve,0, sizeof(curve));
    memset (&surf,0, sizeof(surf));

    curve.rational = input->rational;
    curve.params = input->vParams;
    if (SUCCESS != (status = bspcurv_allocateCurve (&curve)))
        return status;

    for (cpP=endP=curve.poles, inpP = input->poles, endP += curve.params.numPoles;
         cpP < endP; cpP++, inpP += input->uParams.numPoles)
        *cpP = *inpP;

    if (curve.rational)
        for (cwP=endW=curve.weights, inwP = input->weights, endW += curve.params.numPoles;
             cwP < endW; cwP++, inwP += input->uParams.numPoles)
        *cwP = *inwP;

    numKnots = bspknot_numberKnots (curve.params.numPoles,
                                    curve.params.order,
                                    curve.params.closed);
    memcpy (curve.knots, input->vKnots, numKnots * sizeof(double));

    if (SUCCESS != (status = bsprsurf_matrixElevateDegree (&eP, &curve, newDegree)))
        goto wrapup;

    /* Make a copy of input surface */
    if (SUCCESS != (status = bspsurf_copySurface (&surf, input)))
        return status;

    /* Use process func to elevate degree of each row */
    if (SUCCESS != (status = bspproc_processSurfaceByCurves (&surf,
                                                             bsprsurf_elevateDegreeProcess,
                                                             &eP, BSSURF_V)))
        goto wrapup;

    if (output == input)
        bspsurf_freeSurface (output);

    *output = surf;

wrapup:
    bspcurv_freeCurve (&curve);
    if (status)
        bspsurf_freeSurface (&surf);

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsprsurf_elevateDegreeSurface                           |
|                                                                       |
| author        BFP                                     10/90           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  int      bsprsurf_elevateDegreeSurface
(
MSBsplineSurface    *output,
MSBsplineSurfaceCP   input,
int                 newDegree,
int                 edge
)
    {
    if (edge == BSSURF_U)
        return bsprsurf_elevateDegreeU (output, input, newDegree);
    else
        return bsprsurf_elevateDegreeV (output, input, newDegree);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_removeRedundantVKnotsFromSurface             |
|                                                                       |
| author        LuHan                                   6/99            |
|               DavidAssaf                              12/05           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  int      mdlBspline_removeRedundantVKnotsFromSurface
(
MSBsplineSurface    *out,
MSBsplineSurfaceCP   in
)
    {
    double*     pDistinctKnots = NULL;
    int         i, j, iWrite, iRead, nMove, numKnots, numDistinct, numRemove, index;
    int*        pKnotMultiplicities = NULL, *pRemoveMults = NULL, *pRemoveKnotIndices = NULL, *pRemovePoleRowIndices = NULL;
    StatusInt   status = SUCCESS;

    if (SUCCESS != (status = bsprsurf_openSurface (out, in, 0.0, BSSURF_V)))
        return status;

    numKnots = bspknot_numberKnots (out->vParams.numPoles, out->vParams.order, false);
    if (NULL == (pDistinctKnots         = (double*)dlmSystem_mdlMalloc (numKnots * sizeof (double))) ||
        NULL == (pKnotMultiplicities    = (int*)dlmSystem_mdlMalloc (numKnots * sizeof (int)))    ||
        NULL == (pRemoveMults           = (int*)dlmSystem_mdlMalloc (numKnots * sizeof (int)))    ||
        NULL == (pRemoveKnotIndices     = (int*)dlmSystem_mdlMalloc (numKnots * sizeof (int)))    ||
        NULL == (pRemovePoleRowIndices  = (int*)dlmSystem_mdlMalloc (numKnots * sizeof (int))))
        {
        status = MDLERR_INSFMEMORY;
        goto wrapup;
        }

    if (SUCCESS != (status = bspknot_getKnotMultiplicity (pDistinctKnots, pKnotMultiplicities, &numDistinct, out->vKnots,
                                                             out->vParams.numPoles, out->vParams.order, false, KNOT_TOLERANCE_BASIS)))
        goto wrapup;

    // fill arrays with excess multiplicities and indices to starts of excess knot groups
    for (i = 0, numRemove = 0, index = 0; i < numDistinct; i++)
        {
        index += pKnotMultiplicities[i];

        // The first and last knots should have no more than order multiplicity, but interior knots no more than degree multiplicity
        if (i == 0)
            {
            if (pKnotMultiplicities[i] > out->vParams.order)
                {
                pRemoveMults[numRemove]          = pKnotMultiplicities[i] - out->vParams.order;
                pRemovePoleRowIndices[numRemove] = index - pKnotMultiplicities[i];
                pRemoveKnotIndices[numRemove]    = index - pRemoveMults[numRemove]; // remove rightmost excess knot(s) to preserve start knots
                numRemove++;
                }
            }
        else if (i == numDistinct - 1)
            {
            if (pKnotMultiplicities[i] > out->vParams.order)
                {
                pRemoveMults[numRemove]          = pKnotMultiplicities[i] - out->vParams.order;
                pRemovePoleRowIndices[numRemove] = index - pKnotMultiplicities[i];
                pRemoveKnotIndices[numRemove]    = index - pKnotMultiplicities[i];  // remove leftmost excess knot(s) to preserve end knots
                numRemove++;
                }
            }
        else
            {
            if (pKnotMultiplicities[i] >= out->vParams.order)
                {
                pRemoveMults[numRemove]          = pKnotMultiplicities[i] - out->vParams.order + 1;
                pRemovePoleRowIndices[numRemove] = index - pKnotMultiplicities[i];
                pRemoveKnotIndices[numRemove]    = index - pKnotMultiplicities[i];  // remove leftmost excess knot(s)
                numRemove++;
                }
            }
        }

    // Remove redundant v-knots and their associated rows of poles
    for (i = 0; i < numRemove; i++)
        {
        iWrite = pRemoveKnotIndices[i];
        iRead  = pRemoveKnotIndices[i] + pRemoveMults[i];
        nMove  = numKnots - iRead;

        memmove (out->vKnots + iWrite, out->vKnots + iRead, nMove * sizeof (*out->vKnots));

        iWrite = out->uParams.numPoles * pRemovePoleRowIndices[i];
        iRead  = out->uParams.numPoles * (pRemovePoleRowIndices[i] + pRemoveMults[i]);
        nMove  = out->uParams.numPoles * out->vParams.numPoles - iRead;

        memmove (out->poles + iWrite, out->poles + iRead, nMove * sizeof (*out->poles));
        if (out->rational)
            memmove (out->weights + iWrite, out->weights + iRead, nMove * sizeof (*out->weights));

        // reset remaining excess indices
        for (j = i + 1; j < numRemove; j++)
            {
            pRemoveKnotIndices[j] -= pRemoveMults[i];
            pRemovePoleRowIndices[j] -= pRemoveMults[i];
            }

        numKnots              -= pRemoveMults[i];
        out->vParams.numKnots -= pRemoveMults[i];
        out->vParams.numPoles -= pRemoveMults[i];
        }

wrapup:
    if (pRemovePoleRowIndices)
        dlmSystem_mdlFree (pRemovePoleRowIndices);
    if (pRemoveKnotIndices)
        dlmSystem_mdlFree (pRemoveKnotIndices);
    if (pRemoveMults)
        dlmSystem_mdlFree (pRemoveMults);
    if (pKnotMultiplicities)
        dlmSystem_mdlFree (pKnotMultiplicities);
    if (pDistinctKnots)
        dlmSystem_mdlFree (pDistinctKnots);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_removeRedundantUKnotsFromSurface             |
|                                                                       |
| author        LuHan                                   6/99            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  int      mdlBspline_removeRedundantUKnotsFromSurface
(
MSBsplineSurface    *out,
MSBsplineSurfaceCP   in
)
    {
    int     status;

    if (SUCCESS == (status = bspsurf_swapUV (out, in)) &&
        SUCCESS == (status = mdlBspline_removeRedundantVKnotsFromSurface (out, in)))
        status = bspsurf_swapUV (out, out);
    return  status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_removeRedundantKnotsFromSurface              |
|                                                                       |
| author        LuHan                                   6/99            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  int      mdlBspline_removeRedundantKnotsFromSurface
(
MSBsplineSurface    *out,
MSBsplineSurfaceCP   in,
bool                uvDir   /* => true: u knots, false: v knots */
)
    {
    if (uvDir == true)
        return mdlBspline_removeRedundantUKnotsFromSurface (out, in);
    else
        return mdlBspline_removeRedundantVKnotsFromSurface (out, in);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_cleanSurfaceKnots                            |
|                                                                       |
| author        VenkatKalyan/LuHan                      01/98           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP  int      mdlBspline_cleanSurfaceKnots
(
MSBsplineSurface    *out,
MSBsplineSurfaceCP   in
)
    {
    int     status;

    if (SUCCESS == (status = mdlBspline_removeRedundantKnotsFromSurface(out, in, 0)))
        status = mdlBspline_removeRedundantKnotsFromSurface (out, out, 1);

    return  status;
    }


#ifdef Compile_bspsurf_intersectBoundariesWithUVLineExt

/*----------------------------------------------------------------------+
|         Copy of util_compareDoubles                                   |
+----------------------------------------------------------------------*/
static int     compareDoubles
(
double  y1,
double  y2
)
    {
    if (y1 == y2)
        return (0);
    else if (y1<y2)
        return (1);
    else
        return(-1);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             09/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  void bspsurf_intersectBoundariesWithUVLineExt
(
bvector<double>&    pFractionArray,
double                  value,
MSBsplineSurfaceCP      pSurface,
int                     horizontal
)
    {
    int             i, prev, curr, next, numSpans, bufSize, numPoints;
    double          scanHeight, near1;
    DPoint2d        *p;
    BsurfBoundary   *currBound, *endB;
    int numLinear, numPCurve;
    static double sParamTol = 1.0e-8;

    scanHeight = value;
    near1 = 1.0 - fc_epsilon;
    if (scanHeight < fc_epsilon)
            scanHeight = fc_epsilon;
    if (scanHeight > near1)
            scanHeight = near1;

    //pFractionArray->clear();
    bvector<double>().swap (pFractionArray);

    bspsurf_countLoops (pSurface, &numLinear, &numPCurve);
    if (   numPCurve > 0)
        {
        DPoint3d origin;
        DPoint3d normal;
        //EmbeddedDoubleArray *pCurveParamArray = jmdlEmbeddedDoubleArray_grab ();
        bvector<double> pCurveParamArray;

        if (!pSurface->holeOrigin)
            {
            //jmdlEmbeddedDoubleArray_addDouble (pFractionArray, 0.0);
            //jmdlEmbeddedDoubleArray_addDouble (pFractionArray, 1.0);
            pFractionArray.push_back (0.0);
            pFractionArray.push_back (1.0);
            }
        if (horizontal)
            {
            origin.Init ( 0.0, scanHeight, 0.0);
            normal.Init ( 0.0, 1.0, 0.0);
            }
        else
            {
            origin.Init ( scanHeight,  0.0, 0.0);
            ((DVec3d *) &normal)->Init ( 1.0,       0.0, 0.0);
            }

        for (int i = 0; i < pSurface->numBounds; i++)
            {
            for (TrimCurve *pCurr = pSurface->boundaries[i].pFirst;
                    NULL != pCurr;
                    pCurr = pCurr->pNext)
                {
                //pCurveParamArray->clear();
                bvector<double>().swap (pCurveParamArray);
                bspcurv_curvePlaneIntersectsExt (pCurveParamArray, &origin, &normal, &pCurr->curve, sParamTol);
                //int numCurveParam = pCurveParamArray->size();
                size_t numCurveParam = pCurveParamArray.size ();
                for (size_t k = 0; k < numCurveParam; k++)
                    {
                    double u;
                    DPoint3d xyz;
                    //jmdlEmbeddedDoubleArray_getDouble (pCurveParamArray, &u, k);
                    u = pCurveParamArray.at (k);
                    bspcurv_evaluateCurvePoint (&xyz, NULL, &pCurr->curve, u);
                    //jmdlEmbeddedDoubleArray_addDouble (pFractionArray, horizontal ? xyz.x : xyz.y);
                    pFractionArray.push_back (horizontal ? xyz.x : xyz.y);
                    }
                }
            }
        //jmdlEmbeddedDoubleArray_drop (pCurveParamArray);

        //jmdlEmbeddedDoubleArray_sort (pFractionArray);
        std::sort (pFractionArray.begin (), pFractionArray.end ());
        }
    else if (pSurface->numBounds > 0)
        {
        numSpans = bufSize = 0;
        near1 = 1.0 - fc_epsilon;
        if (!pSurface->holeOrigin)
            {
            //jmdlEmbeddedDoubleArray_addDouble (pFractionArray, 0.0);
            //jmdlEmbeddedDoubleArray_addDouble (pFractionArray, 1.0);
            pFractionArray.push_back (0.0);
            pFractionArray.push_back (1.0);
            }
        for (currBound=endB=pSurface->boundaries, endB += pSurface->numBounds;
             currBound < endB; currBound++)
            {
            p = currBound->points;
            numPoints = currBound->numPoints;

            /* Ignore trailing duplicates */
            for (;numPoints > 1 && p[numPoints-1].x == p[numPoints - 2].x &&
                                   p[numPoints-1].y == p[numPoints - 2].y; numPoints--);

            /* Ignore leading duplicates */
            for (;numPoints > 1 && p[0].x == p[1].x &&
                                   p[0].y == p[1].y; numPoints--, p++);

            if (numPoints < 2)
                {
                }
            else if (horizontal)
                {
                curr = compareDoubles (scanHeight, p[0].y);
                next = compareDoubles (scanHeight, p[1].y);
                for (i=1; i < numPoints; i++)
                    {
                    prev = curr;
                    curr = next;
                    next = compareDoubles (scanHeight,
                                                p[(i+1) % (numPoints-1)].y);
                    if (curr)
                        {
                        if (curr*prev < 0)
                            {
                            double f;
                            double g;
                            if (DoubleOps::SafeDivide (f, p[i].x - p[i-1].x, p[i].y - p[i-1].y, 0.0))
                                {
                                g = p[i-1].x + (scanHeight - p[i-1].y) * f;
                                //jmdlEmbeddedDoubleArray_addDouble (pFractionArray, g);
                                pFractionArray.push_back (g);
                                }
                            }
                        }
                    else
                        {
                        if (prev*next < 0)
                            {
                            //jmdlEmbeddedDoubleArray_addDouble (pFractionArray, p[i].x);
                            pFractionArray.push_back (p[i].x);
                            }
                        else
                            {
                            curr = prev;
                            }
                        }
                    }
                }
             else
                {
                curr = compareDoubles (scanHeight, p[0].x);
                next = compareDoubles (scanHeight, p[1].x);
                for (i=1; i < numPoints; i++)
                    {
                    prev = curr;
                    curr = next;
                    next = compareDoubles (scanHeight,
                                                p[(i+1) % (numPoints-1)].x);
                    if (curr)
                        {
                        if (curr*prev < 0)
                            {
                            double f;
                            DoubleOps::SafeDivide (f, p[i].y - p[i-1].y, p[i].x - p[i-1].x, 0.0);
                            double g = p[i-1].y + (scanHeight - p[i-1].x) * f;
                            //jmdlEmbeddedDoubleArray_addDouble (pFractionArray, g);
                            pFractionArray.push_back (g);
                            }
                        }
                    else
                        {
                        if (prev*next < 0)
                            {
                            //jmdlEmbeddedDoubleArray_addDouble (pFractionArray, p[i].y);
                            pFractionArray.push_back (p[i].y);
                            }
                        else
                            {
                            curr = prev;
                            }
                        }
                    }
                }
            }

        //jmdlEmbeddedDoubleArray_sort (pFractionArray);
        std::sort (pFractionArray.begin (), pFractionArray.end ());
        }
    else
        {
        //jmdlEmbeddedDoubleArray_addDouble (pFractionArray, 0.0);
        //jmdlEmbeddedDoubleArray_addDouble (pFractionArray, 1.0);
        pFractionArray.push_back (0.0);
        pFractionArray.push_back (1.0);
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
 FP equality test with fixed tolerance 0.0001.
* @bsimethod                                                    RBB             08/86
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     math_DEqual (double r1, double r2)
    {
    return (fabs (r1 - r2) < .0001);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt    bspproc_getBoundaryCurve
(
MSBsplineCurve          *curveP,
const MSBsplineSurface  *surfaceP,
int                     iBound
)
    {
    int             numKnots;
    BsurfBoundary   *boundP;

    if (NULL == curveP || NULL == surfaceP || iBound < 0 || iBound > surfaceP->numBounds-1)
        return ERROR;

    memset (curveP, 0, sizeof (*curveP));

    curveP->params.order = 2;
    curveP->display      = surfaceP->display;

    boundP = &surfaceP->boundaries[iBound];
    bspsurf_evaluateSurface (&curveP->poles, boundP->points, &boundP->numPoints, surfaceP);

    curveP->params.numPoles = boundP->numPoints;
    numKnots = bspknot_numberKnots (curveP->params.numPoles, curveP->params.order, curveP->params.closed);

    if (NULL != (curveP->knots = (double *) dlmSystem_mdlMalloc (numKnots * sizeof (double))))
        {
        bspknot_computeKnotVector (curveP->knots, &curveP->params, NULL);

        return SUCCESS;
        }

    bspcurv_freeCurve (curveP);

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* Process B-spline surface boundaries.
* @bsimethod                                                    Ray.Bentley     06/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public int      bspproc_processBoundaryCurves
(
const MSBsplineSurface          *pSurface,          /* => Bspline surface to process */
ProcessFuncSurfaceWireFrame     processFunc,        /* => acts on rule lines */
void                            *pProcessFuncArg,   /* => argument passed to processFunction */
bool                            processEdge         /* => true to process parameter edges if unbounded */
)
    {
    StatusInt   status = SUCCESS;

    if (0 != pSurface->numBounds)
        {
        int     iBound;

        for (iBound = 0; iBound < pSurface->numBounds; iBound++)
            {
            MSBsplineCurve      curve;

            if (SUCCESS == bspproc_getBoundaryCurve (&curve, pSurface, iBound))
                {
                double      boundNum = (double) - iBound;

                status = (processFunc) (pProcessFuncArg, &curve, boundNum, boundNum, boundNum, boundNum);

                bspcurv_freeCurve (&curve);

                if (SUCCESS != status)
                    break;
                }
            }
        }

    return status;
    }

bool CheckFractions (bvector<double> &fractions)
    {
    size_t numFraction = fractions.size ();
    if ((numFraction & 0x01) == 0)
        return true;
    // ?? remove small gap -- hope it is a repeated start/end?
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* Process a B-spline surface by operating on the wireframe as curves processFunc : receives a curve formed from each isoparametric and
* boundary curve
* @bsimethod                                                    Ray.Bentley     03/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int      bspproc_surfaceWireframeByCurves
(
const MSBsplineSurface          *pSurface,          /* => Bspline surface to process */
ProcessFuncSurfaceWireFrame     processFunc,        /* => acts on rule lines */
void                            *pProcessFuncArg,   /* => argument passed to processFunction */
bool                            processBounds       /* => process boundaries as curves? */
)
    {
    int                 i, j, status = SUCCESS, size, rule;
    bvector<double> cutFractions;
    double              delta, u, v, *weightP=NULL;
    DPoint3d            *poleP;
    MSBsplineCurve      curve, isoCurve, segCurve;

    memset (&isoCurve, 0, sizeof (isoCurve));
    memset (&curve, 0, sizeof (curve));
    memset (&segCurve, 0, sizeof (segCurve));

    if (pSurface->vParams.numRules > 1)
        {
        /* Process V rule lines (iso-u-curves) */
        delta = 1.0/(double) (pSurface->uParams.closed ?
                               pSurface->vParams.numRules :
                               pSurface->vParams.numRules - 1);
        isoCurve.rational = pSurface->rational;
        isoCurve.params   = pSurface->vParams;
        isoCurve.display  = pSurface->display;
        if (SUCCESS != (status = bspcurv_allocateCurve (&isoCurve)))
            return status;

        size = bspknot_numberKnots (pSurface->vParams.numPoles, pSurface->vParams.order, pSurface->vParams.closed);
        memcpy (isoCurve.knots, pSurface->vKnots, size * sizeof(double));

        for (rule=0, u=0.0; rule < pSurface->vParams.numRules; rule++, u += delta)
            {
            for (i=0; i < pSurface->vParams.numPoles; i++)
                {
                poleP = pSurface->poles + i*pSurface->uParams.numPoles;
                if (pSurface->rational)
                    weightP = pSurface->weights + i*pSurface->uParams.numPoles;

                bspcurv_computeCurvePoint (isoCurve.poles+i, NULL,
                                            pSurface->rational ? isoCurve.weights + i : NULL,
                                            u, poleP, pSurface->uParams.order, pSurface->uParams.numPoles,
                                            pSurface->uKnots, weightP, pSurface->rational,
                                    pSurface->uParams.closed);
                if (pSurface->rational)
                    {
                    isoCurve.poles[i].x *= isoCurve.weights[i];
                    isoCurve.poles[i].y *= isoCurve.weights[i];
                    isoCurve.poles[i].z *= isoCurve.weights[i];
                    }
                }
            bspsurf_intersectBoundariesWithUVLine (&cutFractions, u, pSurface, false);
            CheckFractions (cutFractions);
            for (size_t k = 0; k +1 < cutFractions.size (); k += 2)
                {
                double a0 = cutFractions[k];
                double a1 = cutFractions[k+1];
                if (!math_DEqual (a0, a1))
                    {
                    if (SUCCESS ==
                        bspcurv_segmentCurve (&segCurve, &isoCurve, a0, a1))
                        {
                        if (SUCCESS != (status = (processFunc) (pProcessFuncArg, &segCurve, u, u, a0, a1)))
                            goto wrapup;

                        bspcurv_freeCurve (&segCurve);
                        }
                    }
                }
            }

        bspcurv_freeCurve (&isoCurve);
        }

    /* Process U rule lines (iso-v-curves) */
    if (pSurface->uParams.numRules > 1)
         {
        delta = 1.0/(double) (pSurface->vParams.closed ?
                               pSurface->uParams.numRules :
                               pSurface->uParams.numRules - 1);
        isoCurve.rational = pSurface->rational;
        isoCurve.params   = pSurface->uParams;

        curve.params   = pSurface->vParams;
        curve.rational = pSurface->rational;

        if (SUCCESS != (status = bspcurv_allocateCurve (&isoCurve)) ||
            SUCCESS != (status = bspcurv_allocateCurve (&curve)))
            return status;

        size = bspknot_numberKnots (pSurface->vParams.numPoles, pSurface->vParams.order,
                                    pSurface->vParams.closed);
        memcpy (curve.knots, pSurface->vKnots, size * sizeof (double));

        size = bspknot_numberKnots (pSurface->uParams.numPoles, pSurface->uParams.order,
                                    pSurface->uParams.closed);
        memcpy (isoCurve.knots, pSurface->uKnots, size * sizeof (double));

        for (rule=0, v=0.0; rule < pSurface->uParams.numRules; rule++, v += delta)
            {
            /* Use curve to hold poles & weights from bspline */
            for (i=0; i < pSurface->uParams.numPoles; i++)
                {
                for (j=0; j < pSurface->vParams.numPoles; j++)
                    curve.poles[j] = pSurface->poles[i + j * pSurface->uParams.numPoles];

                if (curve.rational)
                    for (j=0; j < pSurface->vParams.numPoles; j++)
                        curve.weights[j] = pSurface->weights[i + j * pSurface->uParams.numPoles];

                bspcurv_computeCurvePoint (isoCurve.poles+i, NULL,
                                           curve.rational ? isoCurve.weights + i : NULL,
                                           v, curve.poles, curve.params.order,
                                           curve.params.numPoles, curve.knots, curve.weights,
                                           curve.rational, curve.params.closed);

                if (curve.rational)
                    {
                    isoCurve.poles[i].x *= isoCurve.weights[i];
                    isoCurve.poles[i].y *= isoCurve.weights[i];
                    isoCurve.poles[i].z *= isoCurve.weights[i];
                    }
                }

            bspsurf_intersectBoundariesWithUVLine (&cutFractions, v, pSurface, true);
            CheckFractions (cutFractions);
            for (size_t k = 0; k +1 < cutFractions.size (); k += 2)
                {
                double a0 = cutFractions[k];
                double a1 = cutFractions[k+1];
                if (!math_DEqual (a0, a1))
                    {
                    if (SUCCESS ==
                        bspcurv_segmentCurve (&segCurve, &isoCurve, a0, a1))
                        {
                        if (SUCCESS != (status = (processFunc) (pProcessFuncArg, &segCurve, v, v, a0, a1)))
                            goto wrapup;

                        bspcurv_freeCurve (&segCurve);
                        }
                    }
                }
            }
        bspcurv_freeCurve (&isoCurve);
        }

    if (processBounds)
        status = bspproc_processBoundaryCurves (pSurface, processFunc, pProcessFuncArg, false);

wrapup:

    bspcurv_freeCurve (&curve);
    bspcurv_freeCurve (&segCurve);
    bspcurv_freeCurve (&isoCurve);

    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  bool        bspcurv_closestXYPoint
(
        DPoint3d    *pClosePoint,
        double      *pCloseFraction,
        double      *pCloseDist2,
MSBsplineCurve      *curveP,        /* => input curve */
double              startFraction,  /* => starting fraction parameter */
double              endFraction,    /* => ending fraction parameter */
double              xx,             /* => fixed point x */
double              yy              /* => fixed point y */
)
    {
    BCurveSegment segment;
    bool            nearPointComputed = false;
    double          currDist2, currParam;
    DPoint4d        currClosePoint;
    double          minDist2 = DBL_MAX, minKnot = DBL_MAX;
    DPoint4d        minPoint;
    double startKnot = curveP->FractionToKnot (startFraction);
    double endKnot   = curveP->FractionToKnot (endFraction);
    DRange1d activeRange = DRange1d::From (startKnot, endKnot);
    /* For each Bezier segment ... */
    for (size_t spanIndex = 0; curveP->AdvanceToBezierInKnotInterval (segment, spanIndex, activeRange);)
        {
        bsiBezierDPoint4d_closestXYPoint (&currClosePoint, &currParam, &currDist2,
                            segment.GetPoleP (), (int)segment.GetOrder (), xx, yy, 0.0, 1.0);

        if (!nearPointComputed || currDist2 < minDist2)
            {
            minPoint            = currClosePoint;
            minDist2            = currDist2;
            minKnot            = segment.FractionToKnot (currParam);
            nearPointComputed   = true;
            }
        }

    if (nearPointComputed)
        {
        if (pCloseDist2)
            *pCloseDist2 = minDist2;
        if (pCloseFraction)
            *pCloseFraction = mdlBspline_naturalParameterToFractionParameter (curveP, minKnot);
        if (pClosePoint)
            minPoint.GetProjectedXYZ (*pClosePoint);

        }

    return nearPointComputed;
    }

/*----------------------------------------------------------------------+
Return larger of
 (1) abstol
 (2) larger of relTol or RELATIVE_RESOLUTION times
          larger of dataSize or SMALLEST_ALLOWED_REFERENCE_SIZE
+----------------------------------------------------------------------*/
static     double  sizeToResolution
(
double dataSize,
double absTol,
double relTol
)
    {
    double tol;
    dataSize = fabs (dataSize);
    if (dataSize < SMALLEST_ALLOWED_REFERENCE_SIZE)
        dataSize = SMALLEST_ALLOWED_REFERENCE_SIZE;
    if (relTol < RELATIVE_RESOLUTION)
        relTol = RELATIVE_RESOLUTION;
    tol = dataSize * relTol;
    if (tol < absTol)
        tol = absTol;
    return tol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     01/01
+---------------+---------------+---------------+---------------+---------------+------*/
static double  expandMaxAbsDoubleArray
(
double *pData,
int     count,
double  dMax
)
    {
    int  i;
    double dCurr;
    for (i = 0; i < count; i++)
        {
        dCurr = fabs (pData[i]);
        if (dCurr > dMax)
            dMax = dCurr;
        }
    return dMax;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     01/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  double  bspcurv_getResolutionExt
(
MSBsplineCurve const *pCurve,                /* => b-spline curve */
double absTol,
double relTol
)
    {
    return sizeToResolution (
                expandMaxAbsDoubleArray
                    (
                    (double *)pCurve->poles,
                    3 * pCurve->params.numPoles,
                    0.0
                    ),
                absTol,
                relTol);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     01/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  double  bspsurf_getResolutionExt
(
MSBsplineSurface const *pSurface,          /* => b-spline surface */
double              absTol,
double              relTol
)
    {
    return sizeToResolution (
                expandMaxAbsDoubleArray
                    (
                    (double*)pSurface->poles,
                    3 * pSurface->uParams.numPoles * pSurface->vParams.numPoles,
                    SMALLEST_ALLOWED_REFERENCE_SIZE
                    ),
                absTol,
                relTol);
    }

/*---------------------------------------------------------------------------------**//**
* @description Validate surface knots
* @bsimethod                                                    DavidAssaf      12/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  StatusInt    mdlBspline_validateSurfaceKnots
(
double*         pUKnots,        // <=> full u-knots
double*         pVKnots,        // <=> full v-knots
DPoint3d*       pPoles,         // <=> full weighted poles (u-major order, optional)
double*         pWeights,       // <=> full weights (u-major order, optional)
BsplineParam*   pUParams,       // <=> potentially modified only if poles given
BsplineParam*   pVParams        // <=> potentially modified only if poles given
)
    {
    StatusInt   status;

    if (!pUKnots || !pVKnots || !pUParams || !pVParams)
        return ERROR;

    // check for nondecreasing knots
    if (SUCCESS != (status = mdlBspline_validateCurveKnots (pUKnots, NULL, NULL, pUParams)) ||
        SUCCESS != (status = mdlBspline_validateCurveKnots (pVKnots, NULL, NULL, pVParams)))
        return status;

    // check for oversaturated exterior knots in open vector (interior knots that equal an exterior knot)
    if (pPoles)
        {
        double  tol, *pStart, *pEnd;
        int     nExcess0, nExcess1, numKnots, i, iWrite, iRead, nMove;

        if (!pUParams->closed)
            {
            numKnots = bspknot_numberKnots (pUParams->numPoles, pUParams->order, pUParams->closed);
            pStart   = pUKnots + pUParams->order - 1;           // last external knot at beginning of knot vector
            pEnd     = pUKnots + numKnots - pUParams->order;    // first external knot at end of knot vector
            tol      = (*pEnd - *pStart) * RELATIVE_BSPLINE_EXT_KNOT_TOLERANCE;

            for (i = pUParams->order, nExcess0 = 0; i < numKnots; i++, nExcess0++)
                if (pUKnots[i] - *pStart >= tol)
                    break;

            for (i = numKnots - pUParams->order - 1, nExcess1 = 0; i >= 0; i--, nExcess1++)
                if (*pEnd - pUKnots[i] >= tol)
                    break;

            // remove excess u-knots and pole columns (in the interior)
            if (nExcess0 > 0 || nExcess1 > 0)
                {
                int iRow, newNumUPoles;

                if (nExcess0 > 0)
                    {
                    memmove (pUKnots + pUParams->order, pUKnots + pUParams->order + nExcess0, (numKnots - pUParams->order - nExcess0) * sizeof (*pUKnots));

                    newNumUPoles = pUParams->numPoles - nExcess0;
                    nMove = newNumUPoles - 1;
                    for (iRow = iWrite = iRead = 0; iRow < pVParams->numPoles; iRow++, iWrite += nMove, iRead += nMove)
                        {
                        pPoles[iWrite] = pPoles[iRead];
                        if (pWeights)
                            pWeights[iWrite] = pWeights[iRead];

                        iWrite += 1;
                        iRead  += 1 + nExcess0;
                        memmove (pPoles + iWrite, pPoles + iRead, nMove * sizeof (*pPoles));
                        if (pWeights)
                            memmove (pWeights + iWrite, pWeights + iRead, nMove * sizeof (*pWeights));
                        }

                    numKnots           -= nExcess0;
                    pUParams->numKnots -= nExcess0;
                    pUParams->numPoles -= nExcess0;
                    }

                if (nExcess1 > 0)
                    {
                    memmove (pUKnots + numKnots - pUParams->order - nExcess1, pUKnots + numKnots - pUParams->order, pUParams->order * sizeof (*pUKnots));

                    newNumUPoles = pUParams->numPoles - nExcess1;
                    nMove = newNumUPoles - 1;
                    for (iRow = iWrite = iRead = 0; iRow < pVParams->numPoles; iRow++, iWrite++, iRead++)
                        {
                        memmove (pPoles + iWrite, pPoles + iRead, nMove * sizeof (*pPoles));
                        if (pWeights)
                            memmove (pWeights + iWrite, pWeights + iRead, nMove * sizeof (*pWeights));

                        iWrite += nMove;
                        iRead  += nMove + nExcess1;
                        pPoles[iWrite] = pPoles[iRead];
                        if (pWeights)
                            pWeights[iWrite] = pWeights[iRead];
                        }

                    numKnots           -= nExcess1;
                    pUParams->numKnots -= nExcess1;
                    pUParams->numPoles -= nExcess1;
                    }
                }
            }

        if (!pVParams->closed)
            {
            numKnots = bspknot_numberKnots (pVParams->numPoles, pVParams->order, pVParams->closed);
            pStart   = pVKnots + pVParams->order - 1;           // last external knot at beginning of knot vector
            pEnd     = pVKnots + numKnots - pVParams->order;    // first external knot at end of knot vector
            tol      = (*pEnd - *pStart) * RELATIVE_BSPLINE_EXT_KNOT_TOLERANCE;

            for (i = pVParams->order, nExcess0 = 0; i < numKnots; i++, nExcess0++)
                if (pVKnots[i] - *pStart >= tol)
                    break;

            for (i = numKnots - pVParams->order - 1, nExcess1 = 0; i >= 0; i--, nExcess1++)
                if (*pEnd - pVKnots[i] >= tol)
                    break;

            // remove excess u-knots and pole columns (in the interior)
            if (nExcess0 > 0 || nExcess1 > 0)
                {
                if (nExcess0 > 0)
                    {
                    memmove (pVKnots + pVParams->order, pVKnots + pVParams->order + nExcess0, (numKnots - pVParams->order - nExcess0) * sizeof (*pVKnots));

                    iWrite = pUParams->numPoles;
                    iRead  = pUParams->numPoles * (1 + nExcess0);
                    nMove  = pUParams->numPoles * (pVParams->numPoles - 1 - nExcess0);
                    memmove (pPoles + iWrite, pPoles + iRead, nMove * sizeof (*pPoles));
                    if (pWeights)
                        memmove (pWeights + iWrite, pWeights + iRead, nMove * sizeof (*pWeights));

                    numKnots           -= nExcess0;
                    pVParams->numKnots -= nExcess0;
                    pVParams->numPoles -= nExcess0;
                    }

                if (nExcess1 > 0)
                    {
                    memmove (pVKnots + numKnots - pVParams->order - nExcess1, pVKnots + numKnots - pVParams->order, pVParams->order * sizeof (*pVKnots));

                    iWrite = pUParams->numPoles * (pVParams->numPoles - 2 - nExcess0);
                    iRead  = pUParams->numPoles * (pVParams->numPoles - 1);
                    nMove  = pUParams->numPoles;
                    memmove (pPoles + iWrite, pPoles + iRead, nMove * sizeof (*pPoles));
                    if (pWeights)
                        memmove (pWeights + iWrite, pWeights + iRead, nMove * sizeof (*pWeights));

                    numKnots           -= nExcess1;
                    pVParams->numKnots -= nExcess1;
                    pVParams->numPoles -= nExcess1;
                    }
                }
            }
        }

    return SUCCESS;
    }
#ifdef Compile_bspproc_processSurfacePolygon
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             12/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int      bspproc_processSurfacePolygon
(
const MSBsplineSurface              *surfaceP,         /* => curve to process */
ProcessFuncSurfaceControlPolygon    processFunc,       /* => process function */
void                                *args              /* => arguments */
)
    {
    int                 numPoles, status, i, uNumPoles, vNumPoles, maxPoles, nPoints;
    DPoint3d            *pPoles, *pPoints;

    uNumPoles = surfaceP->uParams.numPoles;
    vNumPoles = surfaceP->vParams.numPoles;
    numPoles = uNumPoles * vNumPoles;
    maxPoles = uNumPoles > vNumPoles ? uNumPoles : vNumPoles;

    if (surfaceP->rational)
        bsputil_unWeightPoles (pPoles = (DPoint3d*)_alloca (numPoles * sizeof(DPoint3d)), surfaceP->poles, surfaceP->weights, numPoles);
    else
        pPoles = surfaceP->poles;

    pPoints = (DPoint3d*)_alloca ((1 + maxPoles) * sizeof(DPoint3d));
    nPoints = uNumPoles + (surfaceP->uParams.closed ? 1 : 0);

    for (i=0; i < vNumPoles; i++)
        {
        double      v = (double) i / (double) (vNumPoles - 1);
        memcpy (pPoints, pPoles + uNumPoles * i, uNumPoles * sizeof(DPoint3d));

        if (surfaceP->uParams.closed)
            pPoints[uNumPoles] = pPoints[0];

        if (SUCCESS != (status = processFunc (args, pPoints, nPoints, surfaceP, 0.0, 1.0, v, v)))
            return status;
        }
    nPoints = vNumPoles + (surfaceP->vParams.closed ? 1 : 0);

    for (i=0; i < uNumPoles; i++)
        {
        int         j;
        double      u = (double) i / (double) (uNumPoles - 1);

        for (j=0; j<vNumPoles; j++)
            pPoints[j] = pPoles[i + j * uNumPoles];

        if (surfaceP->vParams.closed)
            pPoints[vNumPoles] = pPoints[0];

        if (SUCCESS != (status = processFunc (args, pPoints, nPoints, surfaceP, u, u, 0.0, 1.0)))
            return status;
        }

    return SUCCESS;
    }
#endif


    
typedef int (*PFFlushDPoint3dPDoublePIntVoidP)(DPoint3d*,double*,int,void*);
typedef int (*PFFlushDPoint3dPIntVoidP)(DPoint3d*,int,void*);

#ifdef Compile_bspcurve_curvePlaneIntersectsExt    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void   cb_collectParameterExt
(
MSBsplineCurveP         pCurve,
double                  param0,
double                  param1,
double                  currParam,
bvector<double>&    pParamArray,
void*                   pVoid
)
    {
    //jmdlEmbeddedDoubleArray_insertDouble  (pParamArray, currParam, -1);
    pParamArray.push_back (currParam);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  StatusInt     bspcurv_curvePlaneIntersectsExt
(
bvector<double>&    pParamArray,
DPoint3dCP              pOrigin,
DPoint3dCP              pNormal,
MSBsplineCurveP         pCurve,
double                  tolerance
)
    {
    DPoint3d unitNormal = *pNormal;
    DPoint4d planeCoffs;

    if (0.0 == unitNormal.Normalize (*pNormal))
        return ERROR;

    bsiDPoint4d_setComponents (&planeCoffs, unitNormal.x, unitNormal.y, unitNormal.z,
                                    -unitNormal.DotProduct (*pOrigin));
    bspcurv_intersectPlaneExt (pCurve, 0.0, 1.0, &planeCoffs, tolerance,
                (MSBsplineCurve_AnnounceParameter*)cb_collectParameterExt, &pParamArray[0], NULL);
    return SUCCESS;
    }
#endif
#ifdef CompileUpdated_getNumBoundPointsCurve
/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_getNumBoundPointsCurve                          |
|                                                                       |
| author        NikolayShulga                           6/95            |
|               Cut Ray's code out of igesin.mc and modified            |
|               as needed (trivial changes mostly)                      |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspsurf_getNumBoundPointsCurve
(
int                 *numPointsP,
MSBsplineCurve      *curveP,
MSBsplineSurface    *surfaceP,
DRange2d           *surfUVRange,
double              tolerance
)
    {
    double surfaceUVPerimeter = surfUVRange->XLength () + surfUVRange->YLength ();
    DRange3d curveRange = curveP->GetPoleRange ();
    double curveUVPerimeter = curveRange.XLength () + curveRange.YLength ();
    DRange3d surfaceRange = surfaceP->GetPoleRange ();
    double surfaceDiagonal = surfaceRange.low.Distance (surfaceRange.high);
    /* perimeter ratio is the APPROX ratio of bound curve to surface */
    perimeterRatio = curveUVPerimeter / surfaceUVPerimeter;

    /*
        NumPoints is calculated based on the linear distance around
        the bounding curve divided by a user specified value. The
        linear distance value is approximated by appling the ratio
        of bound curveuv to surface uv to the linear distance of the
        surface edges. This is attempting to get a resonable number
        of points to evaluate the bound curve at.
    */
    *numPointsP = (int)((perimeterRatio * surfaceDiagonal) / (tolerance * fc_100));

    return  (SUCCESS);
    }
#else
/*----------------------------------------------------------------------+
|                                                                       |
| name          bspsurf_getNumBoundPointsCurve                          |
|                                                                       |
| author        NikolayShulga                           6/95            |
|               Cut Ray's code out of igesin.mc and modified            |
|               as needed (trivial changes mostly)                      |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspsurf_getNumBoundPointsCurve
(
int                 *numPointsP,
MSBsplineCurve      *curveP,
MSBsplineSurface    *surfaceP,
DRange2d           *surfUVRange,
double              tolerance
)
    {
    DPoint3d    *tmpCurvPolesP = NULL,
                *tmpSurfPolesP = NULL,
                deltaSpaceSurf,
                deltaUVCurve, deltaUVSurf;
    int         numSurfPoles;
    double      perimeterRatio, diagonal;
    DRange3d   box;

    numSurfPoles = surfaceP->uParams.numPoles * surfaceP->vParams.numPoles;
    /* malloc memory for tmp copy of poles from surface and curve */
    tmpCurvPolesP = (DPoint3d *)malloc (curveP->params.numPoles * sizeof (DPoint3d));
    if (NULL == tmpCurvPolesP)
        {
        return  ERROR;
        }
    else
        {
        tmpSurfPolesP = (DPoint3d *)malloc (numSurfPoles * sizeof (DPoint3d));
        if (NULL == tmpSurfPolesP)
            {
            free (tmpCurvPolesP);
            return ERROR;
            }
        }

    /* copy/unweight poles from surface and curve */
    if (0 != curveP->rational)
        bsputil_unWeightPoles (tmpCurvPolesP, curveP->poles, curveP->weights,
                                  curveP->params.numPoles);
    else
        memcpy (tmpCurvPolesP, curveP->poles,
                (curveP->params.numPoles * sizeof (DPoint3d)));

    if (0 != surfaceP->rational)
        bsputil_unWeightPoles (tmpSurfPolesP, surfaceP->poles,
                                  surfaceP->weights, numSurfPoles);
    else
        memcpy (tmpSurfPolesP, surfaceP->poles, numSurfPoles * sizeof (DPoint3d));

    /*
        find curve parameter space delta, the curve poles are in uv space
        by definition as they are 142 which are required to be in uv space
    */
    bound_rectangleCompute(&box, tmpCurvPolesP, curveP->params.numPoles);

    deltaUVCurve.x = box.high.x - box.low.x;
    deltaUVCurve.y = box.high.y - box.low.y;
    deltaUVCurve.z = box.high.z - box.low.z;

    /* find surface parameter space delta */
    box.high.z = box.low.z = 0.0;
    memcpy (&box.high, &surfUVRange->high, sizeof (DPoint2d));
    memcpy (&box.low, &surfUVRange->low, sizeof (DPoint2d));

    deltaUVSurf.x = box.high.x - box.low.x;
    deltaUVSurf.y = box.high.y - box.low.y;
    deltaUVSurf.z = box.high.z - box.low.z;

    /* perimeter ratio is the APPROX ratio of bound curve to surface */
    perimeterRatio = (deltaUVCurve.x + deltaUVCurve.y) /
                     (deltaUVSurf.x + deltaUVSurf.y);

    /*  Find surface space range diagonal. It is used to APPROX the
        linear distance of the surface edges    */
    bound_rectangleCompute(&box, tmpSurfPolesP, numSurfPoles);

    deltaSpaceSurf.x = box.high.x - box.low.x;
    deltaSpaceSurf.y = box.high.y - box.low.y;
    deltaSpaceSurf.z = box.high.z - box.low.z;

    diagonal = /*deltaSpaceSurf.Magnitude ();*/
        sqrt( deltaSpaceSurf.x * deltaSpaceSurf.x +
              deltaSpaceSurf.y * deltaSpaceSurf.y +
              deltaSpaceSurf.z * deltaSpaceSurf.z);

    /*
        NumPoints is calculated based on the linear distance around
        the bounding curve divided by a user specified value. The
        linear distance value is approximated by appling the ratio
        of bound curveuv to surface uv to the linear distance of the
        surface edges. This is attempting to get a resonable number
        of points to evaluate the bound curve at.
    */
    *numPointsP = (int)((perimeterRatio * diagonal) / (tolerance * fc_100));

    free (tmpCurvPolesP);
    free (tmpSurfPolesP);
    return  (SUCCESS);
    }
#endif
#ifdef CompileBoresite
Public GEOMDLLIMPEXP int      bspsurf_imposeBoundary
(
MSBsplineSurface    *surface,
MSBsplineCurve      *curve,
double              tolerance,
DRange3d            *directionPoints,
DPoint3d            **surfPoints,       /* <= points on surface, or NULL */
int                 *numSurfPts
)
    {
    DVec3d direction;
    direction.DifferenceOf (directionPoints->high, directionPoints->low);
    return bspsurf_imposeBoundaryBySweptCurve (surface, curve, tolerance, &direction, surfPoints, numSurfPts);
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bound_rectangleCompute
(
DRange3d    *rectP,             /* <= bounding rectangle */
DPoint3d    *pointP,            /* => points */
int         nPoints             /* => number of points */
)
    {
    DPoint3d    *endP;

    endP = pointP + nPoints;
    rectP->low = rectP->high= *pointP++;

    for (; pointP < endP; pointP++)
        {
        if (pointP->x < rectP->low.x)    rectP->low.x = pointP->x;
        if (pointP->y < rectP->low.y)    rectP->low.y = pointP->y;
        if (pointP->z < rectP->low.z)    rectP->low.z = pointP->z;
        if (pointP->x > rectP->high.x)    rectP->high.x = pointP->x;
        if (pointP->y > rectP->high.y)    rectP->high.y = pointP->y;
        if (pointP->z > rectP->high.z)    rectP->high.z = pointP->z;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bspproc_setSubPatchParameter
(
DRange2d       *out,                  /* <= corners of sub-patch */
int             code,                  /* => sub-patch code */
DRange2d        *in                    /* => corners of parent patch */
)
    {
    DPoint2d    mid;

    mid.x = (in->low.x + in->high.x) * 0.5;
    mid.y = (in->low.y + in->high.y) * 0.5;

    switch (code)
        {
        case 0:                        /* 0 <= u <= 1/2 : 0 <= v <= 1/2 */
            out->low   = in->low;
            out->high   = mid;
            break;
        case 1:                        /* 1/2 <= u <= 1 : 0 <= v <= 1/2 */
            out->low.x = mid.x;
            out->low.y = in->low.y;
            out->high.x = in->high.x;
            out->high.y = mid.y;
            break;
        case 2:                        /* 0 <= u <= 1/2 : 1/2 <= v <= 1 */
            out->low.x = in->low.x;
            out->low.y = mid.y;
            out->high.x = mid.x;
            out->high.y = in->high.y;
            break;
        case 3:                        /* 1/2 <= u <= 1 : 1/2 <= v <= 1 */
            out->low   = mid;
            out->high   = in->high;
            break;
        }
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 04/09
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsputil_selectQuarterPatch
(
DRange2dP   out,                  /* <= corners of sub-patch */
int         code,                  /* => sub-patch code */
DRange2dCP  in                    /* => corners of parent patch */
)
    {
    double xmid = 0.5 * (in->low.x + in->high.x);
    double ymid = 0.5 * (in->low.y + in->high.y);
    *out = *in;
    // 0,1
    if (code == 0 || code == 1)
        out->high.y = ymid;
    else
        out->low.y = ymid;

    if (code == 0 || code == 2)
        out->high.x = xmid;
    else
        out->low.x = xmid;    
    }


#ifdef ModernizeWireframeByCurves
// April 1, 2014
// Almost complete replacement using newer queries for segment curves
// 1) u,v intervals of segment are lost.
// 2) This has logic to skip first, last isolines if they will "obviously" be duplicated by boundaries.
//     This does NOT corrrect duplication where a boundary curve matches an interior wireline.
// To fix #1, go down into the GetIsoUCurveSegments functions and pass the transverse limits back up through an added argument.
// "Even more modern" replacement would be a function that just returns all the wires, to be paired (by the caller)
//   with GetUnstructuredBoundaryCurves
// Set bounds limits for numRule' lines, optionally avoiding first/last
// ruleA is first index to process.
// ruleB is "one more than last" for usual "less than" test in loop.
static void GetBoundsLimits (int numRule, int &ruleA, int &ruleB, bool avoidFirstAndLast)
    {
    if (avoidFirstAndLast)
        {
        ruleA = 1;
        ruleB = numRule - 1;
        }
    else
        {
        ruleA = 0;
        ruleB = numRule;
        }
    }
/*---------------------------------------------------------------------------------**//**
* Process a B-spline surface by operating on the wireframe as curves processFunc : receives a curve formed from each isoparametric and
* boundary curve
* @bsimethod                                                    Ray.Bentley     03/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int      bspproc_surfaceWireframeByCurves
(
const MSBsplineSurface          *pSurface,          /* => Bspline surface to process */
ProcessFuncSurfaceWireFrame     processFunc,        /* => acts on rule lines */
void                            *pProcessFuncArg,   /* => argument passed to processFunction */
bool                            processBounds       /* => process boundaries as curves? */
)
    {
    int status = SUCCESS;
    int ruleA, ruleB;

    static bool s_suppress01 = true;
    bool suppress01 = s_suppress01 && processBounds && pSurface->numBounds > 0 && pSurface->holeOrigin;
    if (pSurface->vParams.numRules > 1)
        {
        /* Process V rule lines (iso-u-curves) */
        double delta = 1.0/(double) (pSurface->uParams.closed ?
                               pSurface->vParams.numRules :
                               pSurface->vParams.numRules - 1);

        GetBoundsLimits (pSurface->vParams.numRules, ruleA, ruleB, suppress01);
        for (int rule=ruleA; SUCCESS == status && rule < ruleB; rule++)
            {
            double u = rule * delta;
            bvector<MSBsplineCurvePtr> segments;
            pSurface->GetIsoUCurveSegments (u, segments);
            for (size_t i = 0;SUCCESS == status &&  i < segments.size (); i++)
                {
                if (SUCCESS != (status = (processFunc) (pProcessFuncArg, segments[i].get (), u, u, 0.0, 1.0)))    // WRONG -- v limits
                    return status;
                }
            }
        }

    /* Process U rule lines (iso-v-curves) */
    if (pSurface->uParams.numRules > 1)
         {
        double delta = 1.0/(double) (pSurface->vParams.closed ?
                               pSurface->uParams.numRules :
                               pSurface->uParams.numRules - 1);

        GetBoundsLimits (pSurface->vParams.numRules, ruleA, ruleB, suppress01);
        for (int rule=ruleA; SUCCESS == status && rule < ruleB; rule++)
            {
            double v = rule * delta;
            bvector<MSBsplineCurvePtr> segments;
            pSurface->GetIsoVCurveSegments (v, segments);
            for (size_t i = 0; SUCCESS == status && i < segments.size (); i++)
                {
                if (SUCCESS != (status = (processFunc) (pProcessFuncArg, segments[i].get (), v, v, 0.0, 1.0)))    // WRONG -- u limits
                    return status;
                }
            }
        }

    if (processBounds)
        status = bspproc_processBoundaryCurves (pSurface, processFunc, pProcessFuncArg, false);

    return status;
    }
#endif

END_BENTLEY_GEOMETRY_NAMESPACE
