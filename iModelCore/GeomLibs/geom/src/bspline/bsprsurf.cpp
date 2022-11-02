/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/internal2/BspPrivateApi.h>
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#ifdef COUNT_CALLS

static int s_minDistSurface = 0;
static int s_boresiteSurface = 0;
static int s_anyBoresite = 0;
static int s_allBoresite = 0;
static int s_lineIntSurf = 0;
static int s_boxSurf = 0;
static int s_outputFrequency = 100;

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

#define     MAX_ITER                        5
#define     NALLOC                          256
#define     fc_coeffTol                     1.0e-8
#define     fc_tinyVal                      1.0e-14
#define     fc_newton                       1.0e-8

#define     MAX_POLY_ORDER                  20
#define     MAX_INTERSECTIONS               20
#define     TOLERANCE_ParameterDelta        1.0E-3
#define     MINIMUM_Parameter               -1.0E-6
#define     MAXIMUM_Parameter               1.000001
#define     MINIMUM_QuadraticDiscriminant   -1.0E-12

typedef struct appendsurfparams
    {
    bool            contiguous;
    bool            reparamSurface;
    int             midKnot;
    double          *avgKnots;
    } AppendSurfParams;

typedef struct blendsurfparams
    {
    double          param1;
    double          param2;
    int             degree;
    double          mag1;
    double          mag2;
    } BlendSurfParams;

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
|   Surface Routines - acting on single surface                         |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
Realloc the boundary array for the surface.
Insert the TrimCurve chain as the new boundary.
Note that the boundary does NOT yet have its linear image.
@param surfaceP IN OUT surface to receive additional boundary loop
@param trimLoopPP IN OUT handle for head of new curve chain.
+----------------------------------------------------------------------*/
StatusInt        bspsurf_transferTrimToSurface
(
MSBsplineSurface        *surfaceP,
TrimCurve               **trimLoopPP
)
    {
    StatusInt            status = ERROR;

    bspTrimCurve_breakCyclicList (trimLoopPP);
    if (NULL != *trimLoopPP)
        {
        int allocSize = (surfaceP->numBounds + 1) * sizeof(BsurfBoundary);
        if (NULL != (surfaceP->boundaries = surfaceP->numBounds
                                          ? (BsurfBoundary*) BSIBaseGeom::Realloc (surfaceP->boundaries, allocSize)
                                          : (BsurfBoundary*) BSIBaseGeom::Malloc (allocSize)))
            {
            BsurfBoundary newBoundary;
            memset (&newBoundary, 0, sizeof (newBoundary));
            newBoundary.pFirst = *trimLoopPP;
            surfaceP->boundaries[surfaceP->numBounds] = newBoundary;
            surfaceP->numBounds++;
            *trimLoopPP = NULL;
            status = SUCCESS;
            }
        }

    if (SUCCESS != status)
        bspTrimCurve_freeList (trimLoopPP);

    return status;
    }


MSBsplineSurfacePtr MSBsplineSurface::CreateTrimmedDisk (DEllipse3dCR ellipse)
    {
    TrimCurve           *trimLoopP = NULL;    
    double c = 0.5;
    double r = 0.5;
    DEllipse3d unitCircle = DEllipse3d::From (
            c, c, 0,
            r, 0, 0,
            0, r, 0,
            0.0, Angle::TwoPi ());

    MSBsplineCurve bcurve;
    bcurve.InitFromDEllipse3d (unitCircle);
    bspTrimCurve_allocateAndInsertCyclic (&trimLoopP, &bcurve);
    
    DPoint3d poles[4];
    poles[0].SumOf (ellipse.center, ellipse.vector0, -1.0, ellipse.vector90, -1.0);
    poles[1].SumOf (ellipse.center, ellipse.vector0,  1.0, ellipse.vector90, -1.0);
    poles[2].SumOf (ellipse.center, ellipse.vector0, -1.0, ellipse.vector90,  1.0);
    poles[3].SumOf (ellipse.center, ellipse.vector0,  1.0, ellipse.vector90,  1.0);
    MSBsplineSurfacePtr surface = MSBsplineSurface::CreatePtr ();
    surface->InitFromPointsAndOrder (2, 2, 2, 2, poles);
    bspsurf_transferTrimToSurface (surface.get (), &trimLoopP);
    bspsurf_restrokeTrimLoops (surface.get (), -1.0, -1.0);
    surface->holeOrigin = true;
    return surface;
    }
    
#ifdef Support_bsprsurf_blendSurface
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    addDummyBoundary
(
MSBsplineSurface    *pSurf
)
    {
    MSBsplineCurve      bCurve[4];
    DPoint3d            pnts[2];
    TrimCurve           *trimLoopP = NULL;
    memset (&bCurve, 0, sizeof(bCurve));

    pnts[0].Init ( 0.0, 0.0);
    pnts[1].Init ( 1.0, 0.0);
    mdlBspline_createBsplineFromPointsAndOrder (&bCurve[0], pnts, 2, 2);
    bspTrimCurve_allocateAndInsertCyclic (&trimLoopP, &bCurve[0]);

    pnts[0].Init ( 1.0, 0.0);
    pnts[1].Init ( 1.0, 1.0);
    mdlBspline_createBsplineFromPointsAndOrder (&bCurve[1], pnts, 2, 2);
    bspTrimCurve_allocateAndInsertCyclic (&trimLoopP, &bCurve[1]);

    pnts[0].Init ( 1.0, 1.0);
    pnts[1].Init ( 0.0, 1.0);
    mdlBspline_createBsplineFromPointsAndOrder (&bCurve[2], pnts, 2, 2);
    bspTrimCurve_allocateAndInsertCyclic (&trimLoopP, &bCurve[2]);

    pnts[0].Init ( 0.0, 1.0);
    pnts[1].Init ( 0.0, 0.0);
    mdlBspline_createBsplineFromPointsAndOrder (&bCurve[3], pnts, 2, 2);
    bspTrimCurve_allocateAndInsertCyclic (&trimLoopP, &bCurve[3]);

    bspsurf_transferTrimToSurface (pSurf, &trimLoopP);
    bspsurf_restrokeTrimLoops (pSurf, -1.0, -1.0);
    pSurf->holeOrigin = true;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   Surface Routines - acting on multiple surfaces                      |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprsurf_appendBnds
(
MSBsplineSurface    *output,
MSBsplineSurface    *surf1,
MSBsplineSurface    *surf2,
AppendSurfParams    *asp,
int                 edge
)
    {
    double          a1, a2;
    Transform transform1, transform2;

    bspsurf_freeBoundaries (output);

    if (surf1->numBounds + surf2->numBounds > 0)
        {
        if (surf1->numBounds == 0)
            addDummyBoundary (surf1);
        if (surf2->numBounds == 0)
            addDummyBoundary (surf2);
        }
    // Bad -- what if the trims have different hole sense?
    output->holeOrigin = surf1->holeOrigin;

    a1 = edge == BSSURF_U ?
              asp->avgKnots[asp->midKnot] / output->vParams.numPoles :
              asp->avgKnots[asp->midKnot] / output->uParams.numPoles;

    if (edge == BSSURF_U)
        transform1.InitFromRowValues (
                        a1,  0.0,  0.0, 0.0,
                       0.0,  1.0,  0.0, 0.0,
                       0.0,  0.0,  1.0, 0.0
                       );
    else
        transform1.InitFromRowValues (
                       1.0, 0.0, 0.0, 0.0,
                       0.0, a1,  0.0, 0.0,
                       0.0, 0.0, 1.0, 0.0
                       );

    bspsurf_appendCopyBoundaries (output, surf1, &transform1);

    a2 = 1.0 - a1;

    if (edge == BSSURF_U)
        transform2.InitFromRowValues (
                       a2,  0.0,  0.0, a1,
                       0.0, 1.0,  0.0, 0.0,
                       0.0, 0.0,  1.0, 0.0
                       );
    else
        transform2.InitFromRowValues (
                       1.0, 0.0,  0.0, 0.0,
                       0.0, a2,   0.0, a1,
                       0.0, 0.0,  1.0, 0.0
                       );

    bspsurf_appendCopyBoundaries (output, surf2, &transform2);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprsurf_appendSurfs
(
MSBsplineCurve      *append,
MSBsplineCurve      *crv1,
MSBsplineCurve      *crv2,
void                *vp,
int                 dummy
)
    {
    AppendSurfParams    *asp = (AppendSurfParams*)vp;
    int             status, allocSize;
    double          *kntP, *avgP, *endP;

    if (SUCCESS != (status = bspcurv_combineCurves (append, crv1, crv2,
                                                    asp->contiguous, asp->reparamSurface)))
        return status;

    if (asp->avgKnots == NULL)
        {
        allocSize = bspknot_numberKnots (append->params.numPoles, append->params.order,
                                         append->params.closed) * sizeof(double);
        if (NULL == (asp->avgKnots = (double*)msbspline_malloc (allocSize, HEAPSIG_BSRF)))
            return ERROR;
        memcpy (asp->avgKnots, append->knots, allocSize);
        }
    else
        {
        for (kntP = endP = append->knots, avgP = asp->avgKnots,
             endP += bspknot_numberKnots (append->params.numPoles, append->params.order,
                                          append->params.closed);
            kntP < endP;
            kntP++, avgP++)
            *avgP += *kntP;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprsurf_contiguousSurfaces
(
bool                *contiguous,
MSBsplineSurface    *surf1,
MSBsplineSurface    *surf2,
int                 edge
)
    {
    int             i, numKnots, index1, index2, delta1, delta2;
    double          *knotVector1, *knotVector2, weights1i, weights2i;
    BsplineParam    *params1, *params2;
    DPoint3d        tmp1, tmp2;

    if (edge == BSSURF_U)
        {
        params1 = &surf1->uParams;
        params2 = &surf2->uParams;
        knotVector1 = surf1->uKnots;
        knotVector2 = surf2->uKnots;
        index1 = surf1->uParams.numPoles * (surf1->vParams.numPoles - 1);
        index2 = 0;
        delta1 = delta2 = 1;
        }
    else
        {
        params1 = &surf1->vParams;
        params2 = &surf2->vParams;
        knotVector1 = surf1->vKnots;
        knotVector2 = surf2->vKnots;
        index1 = surf1->uParams.numPoles - 1;
        index2 = 0;
        delta1 = surf1->uParams.numPoles;
        delta2 = surf2->uParams.numPoles;
        }

    if (params1->closed != params2->closed)
        return ERROR;
    if (params1->order != params2->order)
        return ERROR;
    if (params1->numPoles != params2->numPoles)
        return ERROR;

    numKnots = bspknot_numberKnots (params1->numPoles, params1->order, params1->closed);
    for (i=0; i < numKnots; i++)
        if (!MSBsplineCurve::AreSameKnots (knotVector1[i], knotVector2[i]))
            return ERROR;

    *contiguous = true;
    for (i=0; i < params1->numPoles; i++, index1 += delta1, index2 += delta2)
        {
        weights1i = surf1->rational ? surf1->weights[index1] : 1.0;
        weights2i = surf2->rational ? surf2->weights[index2] : 1.0;
        tmp1.Scale (*(surf1->poles+index1), 1.0/weights1i);
        tmp2.Scale (*(surf2->poles+index2), 1.0/weights2i);
        if (false == (*contiguous = bsputil_isSamePoint (&tmp1, &tmp2)))
            break;
        }

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprsurf_combineSurfaces
(
MSBsplineSurface    *outSurface,
MSBsplineSurface    *surf1,
MSBsplineSurface    *surf2,
int                 edge,
bool                forceContinuity,
bool                reparamSurface
)
    {
    int                 status, numKnots, divisor;
    double              *kntP, *avgP, *endP;
    BsplineParam        *params1, *params2;
    AppendSurfParams    asp;
    MSBsplineSurface    surf0;

    memset (&surf0, 0, sizeof (surf0));
    asp.avgKnots = NULL;
    if (edge == BSSURF_U)
        {
        params1 = &surf1->vParams;
        params2 = &surf2->vParams;
        }
    else
        {
        params1 = &surf1->uParams;
        params2 = &surf2->uParams;
        }

    if (forceContinuity)
        asp.contiguous = forceContinuity;
    else
        {
        if (SUCCESS != (status = bsprsurf_contiguousSurfaces (&asp.contiguous,
                                                              surf1, surf2, edge)))
            goto wrapup;
        }

    asp.reparamSurface = reparamSurface;
    asp.midKnot = bspknot_numberKnots (params1->numPoles, params1->order,
                                       params1->closed) - params1->order;

    if (SUCCESS != (status = bspproc_doubleProcessSurfaceByCurves (&surf0, surf1, surf2,
                                                                   bsprsurf_appendSurfs,
                                                                   NULL, // boundary function.  We do it here.
                                                                   &asp, !edge)))
        goto wrapup;

    if (SUCCESS != (status = bsprsurf_appendBnds (&surf0, surf1, surf2, &asp, !edge)))
        goto wrapup;


    if (edge == BSSURF_U)
        {
        kntP = surf0.vKnots;
        numKnots = bspknot_numberKnots (surf0.vParams.numPoles, surf0.vParams.order,
                                        surf0.vParams.closed);
        divisor = surf0.uParams.numPoles;
        }
    else
        {
        kntP = surf0.uKnots;
        numKnots = bspknot_numberKnots (surf0.uParams.numPoles, surf0.uParams.order,
                                        surf0.uParams.closed);
        divisor = surf0.vParams.numPoles;
        }

    /* Average knot vector across curves */
    if (reparamSurface)
        {
        for (avgP = asp.avgKnots, endP = avgP + numKnots; avgP < endP; avgP++)
            *avgP /= divisor;

        memcpy (kntP, asp.avgKnots, numKnots * sizeof(double));
        }

    if (outSurface == surf1 || outSurface == surf2)
        bspsurf_freeSurface (outSurface);

    *outSurface = surf0;
    outSurface->uParams.numRules = outSurface->vParams.numPoles;
    outSurface->vParams.numRules = outSurface->uParams.numPoles;

wrapup:
    if (asp.avgKnots)       msbspline_free (asp.avgKnots);
    if (status)
        bspsurf_freeSurface (&surf0);

    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprsurf_appendSurfaces
(
MSBsplineSurface    *outSurface,
MSBsplineSurface    *inSurface1,
MSBsplineSurface    *inSurface2,
int                 edge,
bool                forceContinuity,
bool                reparamSurface
)
    {
    int                 status;
    BsplineParam        *params1, *params2;
    MSBsplineSurface    surf1, surf2;

    memset (&surf1, 0, sizeof (surf1));
    memset (&surf2, 0, sizeof (surf2));

    if (SUCCESS != (status = bspsurf_copySurface (&surf1, inSurface1)) ||
        SUCCESS != (status = bspsurf_copySurface (&surf2, inSurface2)))
        goto wrapup;

    if (edge == BSSURF_U)
        {
        params1 = &surf1.vParams;
        params2 = &surf2.vParams;
        }
    else
        {
        params1 = &surf1.uParams;
        params2 = &surf2.uParams;
        }

    if (params1->closed &&
        SUCCESS != (status = bsprsurf_openSurface (&surf1, &surf1, 0.0, !edge)))
        goto wrapup;
    if (params2->closed &&
        SUCCESS != (status = bsprsurf_openSurface (&surf2, &surf2, 0.0, !edge)))
        goto wrapup;

    if (params1->order < params2->order)
        {
        if (SUCCESS != (status = bsprsurf_elevateDegreeSurface (&surf1, &surf1,
                                                                params2->order-1, !edge)))
            goto wrapup;
        }
    else if (params2->order < params1->order)
        {
        if (SUCCESS != (status = bsprsurf_elevateDegreeSurface (&surf2, &surf2,
                                                                params1->order-1, !edge)))
            goto wrapup;
        }

    if (SUCCESS != (status = bspsurf_make2SurfacesCompatible (&surf1, &surf2, edge)))
        goto wrapup;

    if (SUCCESS != (status = bsprsurf_combineSurfaces (outSurface, &surf1, &surf2, edge,
                                                       forceContinuity, reparamSurface)))
        goto wrapup;

wrapup:
    bspsurf_freeSurface (&surf1);
    bspsurf_freeSurface (&surf2);

    return status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprsurf_blendSurfs
(
MSBsplineCurve  *blend,
MSBsplineCurve  *crv1,
MSBsplineCurve  *crv2,
void            *vp,
int             dummy
)
    {
    BlendSurfParams *bsp = (BlendSurfParams*)vp;
    return bspcurv_blendCurve (blend, crv1, crv2, bsp->param1, bsp->param2,
                               bsp->degree, bsp->mag1, bsp->mag2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprsurf_blendSurface
(
MSBsplineSurface    *outSurface,
MSBsplineSurface    *inSurface1,
MSBsplineSurface    *inSurface2,
double              param1,
double              param2,
int                 degree,
double              mag1,
double              mag2,
int                 direction
)
    {
    int                 status;
    MSBsplineSurface    surf0, surf1, surf2;
    BlendSurfParams     bsp;

    if (degree < 0 || degree > MAX_ORDER - 1)
        return ERROR;

    bsp.param1    = param1;
    bsp.param2    = param2;
    bsp.degree    = degree;
    bsp.mag1      = mag1;
    bsp.mag2      = mag2;

    memset (&surf0, 0, sizeof(surf0));
    memset (&surf1, 0, sizeof(surf1));
    memset (&surf2, 0, sizeof(surf2));
    if (SUCCESS != (status = bspsurf_copySurface (&surf1, inSurface1)) ||
        SUCCESS != (status = bspsurf_copySurface (&surf2, inSurface2)))
        goto wrapup;

    if (SUCCESS != (status = bspsurf_make2SurfacesCompatible (&surf1, &surf2, direction)) ||
        SUCCESS != (status = bspproc_doubleProcessSurfaceByCurves (&surf0, &surf1, &surf2,
                                     bsprsurf_blendSurfs, NULLFUNC, &bsp, ! direction)))
        goto wrapup;

    if (outSurface == inSurface1 || outSurface == inSurface2)
        bspsurf_freeSurface (outSurface);

    *outSurface = surf0;
    outSurface->uParams.numRules = outSurface->vParams.numPoles;
    outSurface->vParams.numRules = outSurface->uParams.numPoles;

wrapup:
    bspsurf_freeSurface (&surf1);
    bspsurf_freeSurface (&surf2);
    if (status)
        bspsurf_freeSurface (&surf0);
    return status;
    }
#endif
/*----------------------------------------------------------------------+
|                                                                       |
|   Recursive Processing On Single Surface Routines                     |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsprsurf_netPoles
(
DPoint3d            *poles,
double              *weights,
int                 uIndex,
int                 vIndex,
MSBsplineSurface    *surface
)
    {
    int         j, offset;
    double      *wP, *swP;
    DPoint3d    *pP, *spP;

    offset = vIndex * surface->uParams.numPoles + uIndex;

    pP = poles;
    wP = weights;
    spP = surface->poles + offset;
    swP = surface->weights + offset;

    if (surface->rational)
        {
        for (j=0; j < surface->vParams.order;
             j++,
             spP += surface->uParams.numPoles, swP += surface->uParams.numPoles,
             pP+=surface->uParams.order, wP+=surface->uParams.order)
            {
            memcpy (pP, spP, surface->uParams.order * sizeof(DPoint3d));
            memcpy (wP, swP, surface->uParams.order * sizeof(double));
            }
        }
    else
        {
        for (j=0; j < surface->vParams.order;
             j++, spP += surface->uParams.numPoles, pP+=surface->uParams.order)
            memcpy (pP, spP, surface->uParams.order * sizeof(DPoint3d));
        }
    }

#ifdef CompileBoresite
Public GEOMDLLIMPEXP int      bsprsurf_boresiteToSurface
(
DPoint3dP           borePt,
DPoint2dP           param,
DPoint3dCP          testPt,
DPoint3dCP          direction,
MSBsplineSurface*   surface,
double*             tolerance
)
    {
    bvector<DPoint3d> intersectionXYZ;
    bvector<DPoint2d> intersectionUV;
    bvector<double>   intersectionLambda;
    DRay3d ray = DRay3d::FromOriginAndVector (*testPt, *(DVec3d*)direction);
    surface->IntersectRay (intersectionXYZ, intersectionLambda, intersectionUV, ray);

    // ugh.  boresite only wants lambda >= 0....
    size_t acceptIndex = intersectionLambda.size ();
    double minParam = DBL_MAX;
    for (size_t i = 0; i < intersectionLambda.size (); i++)
        {
        if (intersectionLambda[i] >= 0.0 && intersectionLambda[i] < minParam)
            acceptIndex = i;
        }

    if (acceptIndex < intersectionLambda.size ())
        {
        if (NULL != borePt)
            *borePt = intersectionXYZ[acceptIndex];
        if (NULL != param)
            *param = intersectionUV[acceptIndex];
        return SUCCESS;
        }
    return ERROR;
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprsurf_allBoresiteToSurface
(
DPoint3d            **minPt,
DPoint2d            **param,
int                 *numPts,
DPoint3dCP          testPt,
DVec3dCP            direction,
MSBsplineSurfaceCP  surface,
double              *tolerance
)
    {
    bvector<DPoint3d> intersectionXYZ;
    bvector<DPoint2d> intersectionUV;
    bvector<double>   intersectionLambda;
    DRay3d ray = DRay3d::FromOriginAndVector (*testPt, *direction);
    DRange1d interval (0.0, DBL_MAX);
    surface->IntersectRay (intersectionXYZ, intersectionLambda, intersectionUV, ray, interval);
    size_t numAccept = intersectionLambda.size ();
    if (numAccept == 0)
        {
        if (NULL != minPt)
            *minPt = NULL;
        if (NULL != param)
            *param = NULL;
        if (NULL != numPts)
            *numPts = 0;
        return ERROR;
        }
    
    if (NULL != numPts)
        *numPts = (int)numAccept;


    if (NULL != minPt)
        *minPt = DPoint3dOps::MallocAndCopy (intersectionXYZ);
    if (NULL != param)
        *param = DPoint2dOps::MallocAndCopy (intersectionUV);

    return SUCCESS;
    }
    
Public GEOMDLLIMPEXP int      bsprsurf_lineXSurface
(
DPoint3d            **intPts,       /* <= all intersection points on surface */
DPoint2d            **param,        /* <= u, v parameters of intersections */
int                 *numPts,        /* <= number of intersections */
DSegment3d           *segmentP,          /* => segmentP->point[1] is the true end point */
MSBsplineSurface    *surfaceP       /* => surface */
)
    {
    bvector<DPoint3d> intersectionXYZ;
    bvector<DPoint2d> intersectionUV;
    bvector<double>   intersectionLambda;
    DRange1d interval (0.0, 1.0);
    DRay3d ray = DRay3d::From (*segmentP);
    surfaceP->IntersectRay (intersectionXYZ, intersectionLambda, intersectionUV, ray, interval);
    size_t numAccept = intersectionLambda.size ();
    if (numAccept == 0)
        {
        if (NULL != intPts)
            *intPts = NULL;
        if (NULL != param)
            *param = NULL;
        if (NULL != numPts)
            *numPts = 0;
        return ERROR;
        }
    
    if (NULL != numPts)
        *numPts = (int)numAccept;


    if (NULL != intPts)
        *intPts = DPoint3dOps::MallocAndCopy (intersectionXYZ);
    if (NULL != param)
        *param = DPoint2dOps::MallocAndCopy (intersectionUV);

    return SUCCESS;    
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static double  bsprsurf_determinant
(
DPoint3d        *vec1,
DPoint3d        *vec2,
DPoint3d        *vec3
)
    {
    return   (vec1->x * vec2->y * vec3->z + vec1->y * vec2->z * vec3->x +
              vec2->x * vec3->y * vec1->z - vec1->z * vec2->y * vec3->x -
              vec1->y * vec2->x * vec3->z - vec2->z * vec3->y * vec1->x);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprsurf_intersectOfThreePlanes
(
DPoint3d        *out,
DPoint3d        *norm1,
DPoint3d        *pt1,
DPoint3d        *norm2,
DPoint3d        *pt2,
DPoint3d        *norm3,
DPoint3d        *pt3
)
    {
    double      deno;
    DPoint3d    tmp1, tmp2, tmp3, left;

    left.x = norm1->DotProduct (*pt1);
    left.y = norm2->DotProduct (*pt2);
    left.z = norm3->DotProduct (*pt3);

    deno = bsprsurf_determinant (norm1, norm2, norm3);
    if (fabs(deno) < 1.0e-15)
        return false;

    tmp1.x = norm1->x;
    tmp1.y = norm2->x;
    tmp1.z = norm3->x;
    tmp2.x = norm1->y;
    tmp2.y = norm2->y;
    tmp2.z = norm3->y;
    tmp3.x = norm1->z;
    tmp3.y = norm2->z;
    tmp3.z = norm3->z;

    out->x = bsprsurf_determinant (&left, &tmp2, &tmp3) / deno;
    out->y = bsprsurf_determinant (&tmp1, &left, &tmp3) / deno;
    out->z = bsprsurf_determinant (&tmp1, &tmp2, &left) / deno;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bsprsurf_reverseNorm
(
DPoint3d        *out,
DPoint3d        *in,
DPoint3d        *pt1,
DPoint3d        *pt2
)
    {
    DPoint3d        tmp, plus, minus;

    plus.SumOf (*pt1, *in);
    tmp.Scale (*in, -1.0);
    minus.SumOf (*pt1, tmp);

    if (plus.Distance (*pt2) <= minus.Distance (*pt2))
        *out = *in;
    else
        *out = tmp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bsprsurf_oneThirdBtwPoints
(
DPoint3d        *pt,               /* <= one third from end */
DPoint3d        *org,              /* => */
DPoint3d        *end
)
    {
    pt->x = (2.0 * end->x + org->x)/fc_3;
    pt->y = (2.0 * end->y + org->y)/fc_3;
    pt->z = (2.0 * end->z + org->z)/fc_3;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprsurf_blendRails
(
MSBsplineSurface    *outSurf,
MSBsplineSurface    *inSurf1,
MSBsplineSurface    *inSurf2,
MSBsplineCurve      *railCv1,
MSBsplineCurve      *railCv2,
double              *tol,
bool                continuity
)
    {
    int                 i, itmp, numRows, numPoles, order, status, numPt;
    double              dist, step, value;
    DPoint2d            uv1, uv2;
    DPoint3d            *pt1, *pt2, clstPt, du1, du2, dv1, dv2,
                        norm1, norm2, normFst, diff, ptMid, intPt;
    MSBsplineSurface    blendSurf;

    memset (&blendSurf, 0, sizeof(blendSurf));

    /* Create rails from the given points and make them compatible */
    if (SUCCESS != (status = bspcurv_make2CurvesCompatible (railCv1, railCv2)))
        goto wrapup;

    if (!continuity)
        {
        if (SUCCESS != (status = bspsurf_ruledSurface (&blendSurf, railCv1, railCv2)))
            goto wrapup;
        }
    else
        {
        order = railCv1->params.order;
        numRows = 0;
        bspcurv_evaluateCurve (&pt1, tol, &numRows, railCv1);
        numRows = (numRows <= MAX_ORDER) ? MAX_ORDER+1 : numRows;

        /* Malloc the blending surface */
        blendSurf.type = BSSURF_GENERAL;
        blendSurf.rational = false;
        blendSurf.display.curveDisplay = true;
        blendSurf.display.polygonDisplay = false;
        blendSurf.uParams.order = 4;
        blendSurf.uParams.closed = false;
        blendSurf.uParams.numPoles = 4;
        blendSurf.uParams.numKnots = 0;
        blendSurf.uParams.numRules = 20;
        blendSurf.vParams.order = order;
        blendSurf.vParams.closed = false;
        blendSurf.vParams.numPoles = numPoles = numRows - order + 1;
        blendSurf.vParams.numKnots = 0;
        blendSurf.vParams.numRules = 4;
        blendSurf.numBounds = 0;
        bspsurf_allocateSurface (&blendSurf);

        /* Create control points net for blending surface from rails */
        numPt=1;
        step = 1.0 / (numRows - order);
        for (i=order - 1; i<numRows; i++)
            {
            value = (i - order + 1)*step;
            bspcurv_evaluateCurve (&pt1, &value, &numPt, railCv1);
            bspcurv_evaluateCurve (&pt2, &value, &numPt, railCv2);
            bsprsurf_minDistToSurface (&dist, &clstPt, &uv1, pt1, inSurf1, tol);
            bsprsurf_minDistToSurface (&dist, &clstPt, &uv2, pt2, inSurf2, tol);
            bspsurf_computePartials (pt1, NULL, &du1, &dv1, NULL, NULL, NULL,
                                    &norm1, uv1.x, uv1.y, inSurf1);
            bspsurf_computePartials (pt2, NULL, &du2, &dv2, NULL, NULL, NULL,
                                    &norm2, uv2.x, uv2.y, inSurf2);
            bsprsurf_reverseNorm (&norm1, &norm1, pt1, pt2);
            bsprsurf_reverseNorm (&norm2, &norm2, pt2, pt1);

#if defined (debug_draw)
    {
    DPoint3d    line1[2], tmp1;
    DPoint3d    line2[2], tmp2;
    ElementUnion u1;
    ElementUnion u2;

    tmp1.SumOf (norm1, *pt1);
    tmp2.SumOf (norm2, *pt2);
    line1[0] = tmp1;
    line1[1] = *pt1;
    line2[0] = tmp2;
    line2[1] = *pt2;
    mdlLine_create (&u1, NULL, line1);
    mdlLine_create (&u2, NULL, line2);
    u1.line_3d.dhdr.symb.b.color = 2;
    u2.line_3d.dhdr.symb.b.color = 2;
    u1.line_3d.dhdr.symb.b.weight = 1;
    u2.line_3d.dhdr.symb.b.weight = 1;
    displayElement (&u1, 0, -1L);
    displayElement (&u2, 0, -1L);
    }
#endif

            normFst.SumOf (norm1, norm2);
            diff.DifferenceOf (*pt1, *pt2);
            normFst.CrossProduct (normFst, diff);
            norm1.Normalize ();
            norm2.Normalize ();
            normFst.Normalize ();
            ptMid.Interpolate (*pt1, 0.5, *pt2);
            bsprsurf_intersectOfThreePlanes (&intPt, &norm1, pt1,
                                            &norm2, pt2, &normFst, &ptMid);

#if defined (debug_draw)
    {
    DPoint3d    line[2];
    ElementUnion u;

    line[0] = line[1] = intPt;
    mdlLine_create (&u, NULL, line);
    u.line_3d.dhdr.symb.b.color = 2;
    u.line_3d.dhdr.symb.b.weight = 5;
    displayElement (&u, 0, -1L);
    }
#endif

#if defined (nodebug_draw)
    {
    DPoint3d    line1[2];
    DPoint3d    line2[2];
    ElementUnion u1;
    ElementUnion u2;

    line1[0] = line1[1] = *pt1;
    line2[0] = line2[1] = *pt2;
    mdlLine_create (&u1, NULL, line1);
    mdlLine_create (&u2, NULL, line2);
    u1.line_3d.dhdr.symb.b.color = 2;
    u2.line_3d.dhdr.symb.b.color = 2;
    u1.line_3d.dhdr.symb.b.weight = 2;
    u2.line_3d.dhdr.symb.b.weight = 5;
    displayElement (&u1, 0, -1L);
    displayElement (&u2, 0, -1L);
    }
#endif

            itmp = 4*(i-order+1);
            blendSurf.poles[itmp] = *pt1;
            blendSurf.poles[itmp+3] = *pt2;
            bsprsurf_oneThirdBtwPoints (&blendSurf.poles[itmp+1], pt1, &intPt);
            bsprsurf_oneThirdBtwPoints (&blendSurf.poles[itmp+2], pt2, &intPt);
            }

        /* Assign the U-V knots */
        bspknot_computeKnotVector (blendSurf.uKnots, &blendSurf.uParams, NULL);
        bspknot_computeKnotVector (blendSurf.vKnots, &blendSurf.vParams, NULL);

        }

#if defined (nodebug_draw)
    {
    ElementDescr        *edp1 = NULL;
    ElementDescr        *edp2 = NULL;

    railCv1->display.curveDisplay = true;
    railCv2->display.curveDisplay = true;
    railCv1->display.polygonDisplay = true;
    railCv2->display.polygonDisplay = true;
    bspcurv_createCurve (&edp1, NULL, railCv1);
    bspcurv_createCurve (&edp2, NULL, railCv2);
    mdlElmdscr_display (edp1, MASTERFILE, NORMALDRAW);
    mdlElmdscr_display (edp2, MASTERFILE, NORMALDRAW);
    mdlElmdscr_freeAll (&edp1);
    mdlElmdscr_freeAll (&edp2);
    }
#endif

#if defined (debug_draw)
    {
    ElementDescr        *edp = NULL;

    blendSurf.display.curveDisplay = true;
    blendSurf.display.polygonDisplay = false;
    bspsurf_createSurface (&edp, NULL, &blendSurf);
    mdlElmdscr_display (edp, MASTERFILE, HILITE);
    mdlElmdscr_freeAll (&edp);
    }
#endif

    *outSurf = blendSurf;

wrapup:
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    bsprsurf_allPointsCoincide
(
DPoint3d        *points,        /* weighted points if weights != NULL */
double          *weights,       /* weights or NULL */
int             numPts,         /* number of points */
bool            *shift          /* if true, the coincident points are shifted */
)
    {
    double      shiftDist;
    DPoint3d    *pP, *endP;

    /* Unweight the points if rational */
    if (weights)
        bsputil_unWeightPoles (points, points, weights, numPts);

    shiftDist = fc_p0001;
    for (pP = endP = points, endP += numPts-1; pP < endP; pP++)
        if (pP->Distance (pP[1]) > fc_epsilon)
            {
            if (*shift)
                for (pP = endP = points, endP += numPts-1; pP < endP; pP++)
                    if (pP->Distance (pP[1]) < fc_epsilon)
                        {
                        pP->x -= shiftDist;
                        shiftDist /= 2.0;
                        *shift = false;
                        }

            if (weights)
                bsputil_weightPoles (points, points, weights, numPts);
            return (false);
            }

    if (weights)
        bsputil_weightPoles (points, points, weights, numPts);

    return (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int   bsprsurf_crossSectionSurface
(
MSBsplineSurface        *surface,
MSBsplineCurve          *curves,
int                     numCurves,
int                     processCurves,
int                     roundEnds,
double                  processTol
)
    {
    bool            shift, mixed = false, lastClosed = false, currentClosed;
    int             i, j, rational, status = SUCCESS, numU, numV, numPoints;
    double          *wPts = NULL, *wPoles = NULL, length, relativeTol, step, param;
    DPoint3d        *pPts = NULL, *pPoles = NULL, *pointsP;
    MSBsplineCurve  ruleCurves[2];

    /* If two curves pased in, use ruled surface */
    if (numCurves == 2)
        {
        if (processCurves)
            {
            length = 0.0; relativeTol = 0.005;
            if (fabs(processTol) < 1.0e-15)
                return  ERROR;
            else if (processTol < 0.0)
                processTol *= -1.0;

            numPoints = (int) (1.0 / processTol);
            if (numPoints < 2) numPoints = 2;
            if (NULL == (pointsP = (DPoint3d*)msbspline_malloc ((numPoints+2)*sizeof(DPoint3d), HEAPSIG_BSRF)))
                return ERROR;

            mixed = false;
            for (i = 0; i < numCurves; i++)
                {
                memset (&ruleCurves[i], 0, sizeof(MSBsplineCurve));
                length = curves[i].Length ();
                step = length / (numPoints - 1);
                double oldParam = 0.0;
                double actualStep;
                for (j = 0; j < numPoints; j++)
                    {
                    curves[i].FractionAtSignedDistance (oldParam, step, param, actualStep);
                    curves[i].FractionToPoint (pointsP[j], param);
                    oldParam = param;
                    }

                if (DPoint3dOps::AlmostEqual (pointsP[0], pointsP[numPoints-1]))
                    ruleCurves[i].params.closed = true;

                if (i == 0)
                    lastClosed = (0 != ruleCurves[i].params.closed);
                else if ((0 != ruleCurves[i].params.closed) != lastClosed)
                    mixed = true;

                bspcurv_c2CubicInterpolateCurve (&ruleCurves[i], pointsP, NULL, numPoints,
                    false, 1.0, NULL, mixed ? false : (0 != ruleCurves[i].params.closed));
                }
            msbspline_free (pointsP);
            pointsP = NULL;
            }
        else if (!processCurves || status != SUCCESS)
            {
            bspcurv_copyCurve (ruleCurves, curves);
            bspcurv_copyCurve (ruleCurves+1, curves+1);
            if (SUCCESS != (status = bspcurv_make2CurvesCompatible (
                &ruleCurves[0], &ruleCurves[1])))
                return status;
            }

        status = bspsurf_ruledSurface(surface, &ruleCurves[0], &ruleCurves[1]);
        bspcurv_freeCurve (&ruleCurves[0]);
        bspcurv_freeCurve (&ruleCurves[1]);
        }
    else
        {
        bvector<MSBsplineCurvePtr> inArray, cvArray;
        for (i = 0; i < numCurves; i++)
            {
            inArray.push_back (curves[i].CreateCopy ());
            }
        if (processCurves)
            {
            length = 0.0;
            relativeTol = 0.005;
            numPoints = (int) (1.0 / processTol);
            if (numPoints < 2) numPoints = 2;
            pointsP = (DPoint3d*)malloc ((numPoints+2) * sizeof(DPoint3d));
            mixed = false;
            for (i = 0; i < numCurves; i++)
                {
                length = curves[i].Length ();
                step = length / (numPoints - 1);
                double oldParam = 0.0;
                double actualStep;
                for (j = 0; j < numPoints; j++)
                    {
                    curves[i].FractionAtSignedDistance (oldParam, j * step, param, actualStep);
                    curves[i].FractionToPoint (pointsP[j], param);
                    oldParam = param;
                    }

                if (DPoint3dOps::AlmostEqual (pointsP[0], pointsP[numPoints-1]))
                    currentClosed = true;
                else
                    currentClosed = false;

                if (i == 0)
                    lastClosed = currentClosed;
                else if (!mixed)
                    {
                    if (currentClosed != lastClosed)
                        mixed = true;
                    }
                bspcurv_c2CubicInterpolateCurve (cvArray[i].get (), pointsP, NULL,
                    numPoints, false, 1.0, NULL, currentClosed);
                }
            free (pointsP);
            }

        if (!processCurves || mixed)
            {
            if (!MSBsplineCurve::CloneCompatibleCurves (cvArray, inArray, false, true))
                goto wrapup;
            lastClosed = false;
            }

        /* Create the surface from the cross sections */
        rational = cvArray[0]->rational;
        numU = cvArray[0]->params.numPoles;
        surface->rational = rational;
        surface->uParams = cvArray[0]->params;
        surface->vParams.closed = false;
        surface->vParams.order = 4;
        numV = surface->vParams.numPoles = numCurves + 2;
        surface->vParams.numKnots = numCurves - 2;

        if (SUCCESS != (status = bspsurf_allocateSurface (surface)))
            return status;

        /* Malloc some tmp variables */
        pPts = pPoles = NULL;
        wPts = wPoles = NULL;
        if (NULL == (pPts = (DPoint3d*)msbspline_malloc (numCurves*sizeof(DPoint3d), HEAPSIG_BSRF)) ||
            NULL == (pPoles=(DPoint3d*)msbspline_malloc (numV*sizeof(DPoint3d), HEAPSIG_BSRF)))
            {
            status = ERROR;
            goto wrapup;
            }

        if (rational)
            if (NULL == (wPts  =(double*)msbspline_malloc (numCurves*sizeof(double), HEAPSIG_BSRF)) ||
                NULL == (wPoles=(double*)msbspline_malloc (numV*sizeof(double), HEAPSIG_BSRF)))
                {
                status = ERROR;
                goto wrapup;
                }

        /* Compute the poles and weights if rational */
        for (i = 0; i < numU; i++)
            {
            for (j=0; j<numCurves; j++)
                {
                pPts[j] = cvArray[j]->poles[i];
                if (rational) wPts[j] = cvArray[j]->weights[i];
                }

            /* Check if all input data points conincide to one single point */
            shift = true;
            if (bsprsurf_allPointsCoincide (pPts, rational ? wPts : NULL, numCurves, &shift))
                {
                for (j=0; j<numV; j++)
                    {
                    surface->poles[j*numU+i] = pPts[0];
                    if (rational)
                        surface->weights[j*numU+i] = wPts[0];
                    }
                }
            else
                {
                if (SUCCESS != (status = bspcurv_c2CubicInterpolatePoles (
                                            pPoles,
                                            rational ?  wPoles : NULL, NULL, /* Knot */
                                            NULL, /* in params */
                                            pPts, NULL, rational ?  wPts : NULL,
                                            &surface->vParams,
                                            numCurves)))
                    goto wrapup;

                for (j=0; j<numV; j++)
                    {
                    surface->poles[j*numU+i] = pPoles[j];
                    if (rational)
                        surface->weights[j*numU+i] = wPoles[j];
                    }
                }
            }

        /* Compute the u knot vector */
        memcpy (surface->uKnots, cvArray[0]->knots,
                bspknot_numberKnots (numU, cvArray[0]->params.order,
                cvArray[0]->params.closed) * sizeof(double));

        /* Compute the v knot vector uniformly */
        surface->vParams.numKnots = 0;
        bspknot_computeKnotVector (surface->vKnots, &surface->vParams, NULL);

wrapup:
        if (pPts)
            msbspline_free (pPts);
        if (wPts)
            msbspline_free (wPts);
        if (pPoles)
            msbspline_free (pPoles);
        if (wPoles)
            msbspline_free (wPoles);
        }

    /* Swap surface U and V direction so that V direction has more poles */
    if (surface->uParams.numPoles > MAX_POLES &&
        surface->vParams.numPoles <= MAX_POLES)
        bspsurf_swapUV (surface, surface);

    surface->display.curveDisplay = true;
    surface->display.polygonDisplay = false;
    surface->uParams.numRules = surface->vParams.numPoles;
    surface->vParams.numRules = surface->uParams.numPoles;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bsprsurf_scaleCurveToPlane
(
MSBsplineCurve          *scaledCurveP,
MSBsplineCurve          *curveP,
DPoint3d                points[2],
DPoint3d                tangents[2],
DPoint3d                *sweepDirection,
bool                    rigidSweep
)
    {
    double              scale, diff;
    DPoint3d            start, end;
    DVec3d              xAxis, yAxis, zAxis;
    DPoint3d            center, *pP, *endP;
    RotMatrix           rotMatrix;

    /* Scale the input curve */
    bspcurv_extractEndPoints (&start, &end, curveP);
    if (start.Distance (end) < fc_epsilon)
        {
        ;
        }
    scale = points[0].Distance (points[1]) / start.Distance (end);
    bspcurv_copyCurve (scaledCurveP, curveP);
    if (0 != scaledCurveP->rational)
        bsputil_unWeightPoles (scaledCurveP->poles, scaledCurveP->poles,
            scaledCurveP->weights, scaledCurveP->params.numPoles);
    bsiDPoint3d_subtractDPoint3dArray (scaledCurveP->poles, &start, scaledCurveP->params.numPoles);
    xAxis.DifferenceOf (end, start);
    bspcurv_extractNormal (&zAxis, &center, &diff, curveP);
    if (zAxis.DotProduct (*sweepDirection) < 0.0)
        zAxis.Negate (zAxis);
    yAxis.CrossProduct (zAxis, xAxis);
    zAxis.CrossProduct (xAxis, yAxis);
    xAxis.Normalize ();
    yAxis.Normalize ();
    zAxis.Normalize ();
    rotMatrix.InitFromRowVectors (xAxis, yAxis, zAxis);
    rotMatrix.Multiply (scaledCurveP->poles, scaledCurveP->poles, scaledCurveP->params.numPoles);

    for (pP = endP = scaledCurveP->poles, endP += scaledCurveP->params.numPoles; pP < endP; pP++)
        {
        pP->x *= scale;
        pP->y *= scale;
        }

    /* Transform the scaled curve to the section plane */
    xAxis.DifferenceOf (points[1], points[0]);
    // point vector confusion .. do a copy to clarify
    DVec3d tangentVector0 = DVec3d::From (tangents[0]);
    DVec3d tangentVector1 = DVec3d::From (tangents[1]);
    if (!rigidSweep)
        zAxis.SumOf (tangentVector0, tangentVector1);
    yAxis.CrossProduct (zAxis, xAxis);
    zAxis.CrossProduct (xAxis, yAxis);
    xAxis.Normalize ();
    yAxis.Normalize ();
    zAxis.Normalize ();
    rotMatrix.InitFromRowVectors (xAxis, yAxis, zAxis);
    rotMatrix.MultiplyTranspose (scaledCurveP->poles, scaledCurveP->poles, scaledCurveP->params.numPoles);
    DPoint3d::AddToArray (scaledCurveP->poles, scaledCurveP->params.numPoles, points[0]);

#if defined (debug_sweepAlong)
        {
        ElementDescr    *edpDraw = NULL;

        scaledCurveP->display.curveDisplay = true;
        scaledCurveP->display.polygonDisplay = false;
        bspcurv_createCurve (&edpDraw, NULL, scaledCurveP);
        mdlElmdscr_display (edpDraw, MASTERFILE, NORMALDRAW);
        mdlElmdscr_freeAll (&edpDraw);
        }
#endif

    if (0 != scaledCurveP->rational)
        bsputil_weightPoles (scaledCurveP->poles, scaledCurveP->poles,
            scaledCurveP->weights, scaledCurveP->params.numPoles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprsurf_sweepAlongTwoTraces
(
MSBsplineSurface        *surfaceP,
MSBsplineCurve          *trace0P,
MSBsplineCurve          *trace1P,
MSBsplineCurve          *sectionP,
DPoint3d                *crossTangents,
double                  gapTolerance,
double                  fitTolerance,
int                     numSweepings,
bool                    rigidSweep,
bool                    useArcLength,
bool                    checkGaps
)
    {
    int                 i, status;
    double              step, param0, param1;
    DPoint3d            start[2], end[2], sweepDirection;
    MSBsplineCurve      *tmpCurveP=NULL;

    /* Change the directions of the input curves if necessary */
    if (checkGaps == true)
        {
        bspcurv_extractEndPoints (&start[0], &end[0], sectionP);
        bspcurv_extractEndPoints (&start[1], &end[1], trace0P);

        if (start[0].Distance (end[1]) < gapTolerance)
            bspcurv_reverseCurve (trace0P, trace0P);
        else if (end[0].Distance (start[1]) < gapTolerance)
            bspcurv_reverseCurve (sectionP, sectionP);
        else if (end[0].Distance (end[1]) < gapTolerance)
            {
            bspcurv_reverseCurve (trace0P, trace0P);
            bspcurv_reverseCurve (sectionP, sectionP);
            }
        else if (start[0].Distance (start[1]) > gapTolerance)
            return ERROR;

        bspcurv_extractEndPoints (&start[0], &end[0], sectionP);
        bspcurv_extractEndPoints (&start[1], &end[1], trace1P);
        if (end[0].Distance (end[1]) < gapTolerance)
            bspcurv_reverseCurve (trace1P, trace1P);
        else if (end[0].Distance (start[1]) > gapTolerance)
            return ERROR;
        }

    /* Make two traces compatible */
    if (SUCCESS != (status = bspcurv_make2CurvesCompatible (trace0P, trace1P)))
        return status;

    /* NEED WORK: use fitTolerance to determine the numSweepings */
    if (numSweepings <= 1)
        numSweepings = 20;

    /* Allocate an array of curves */
    if (NULL == (tmpCurveP = (MSBsplineCurve*)BSIBaseGeom::Malloc (numSweepings * sizeof(MSBsplineCurve))))
        return ERROR;

    /* Create sweeping cross sections */

    step = 1.0 / (numSweepings - 1);
    for (i = 0; i < numSweepings; i++)
        {
        /* NEED WORK: add option using arc length to compute param */
        param0 = param1 = i * step;
        bspcurv_evaluateCurvePoint (&start[0], &end[0], trace0P, param0);
        bspcurv_evaluateCurvePoint (&start[1], &end[1], trace1P, param1);
        if (i == 0)
            sweepDirection.SumOf (end[0], end[1]);
        bsprsurf_scaleCurveToPlane (&tmpCurveP[i], sectionP, start, end,
            &sweepDirection, rigidSweep);
        }

    /* Generate sweeping surface */
    status = bsprsurf_crossSectionSurface (surfaceP, tmpCurveP, numSweepings, false,
        true, 0.1);

    for (i = 0; i < numSweepings; i++)
        bspcurv_freeCurve (&tmpCurveP[i]);

    BSIBaseGeom::Free (tmpCurveP);
    return (status);
    }

END_BENTLEY_GEOMETRY_NAMESPACE